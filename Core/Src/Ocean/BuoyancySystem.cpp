#include "BuoyancySystem.h"

#include "spdlog/spdlog.h"
#include "Components/WaterEffectorComponent.h"
#include "glad/glad.h"
#include "Math/Math.h"
#include "Meshes/MeshManager.h"
#include "Rendering/Shaders/ShaderBuilder.h"
#include "Scene/TransformComponent.h"

namespace Core
{
    namespace
    {
        constexpr int32_t WorkGroupDim1D = 64;
        constexpr size_t DefaultTriangleBufferSize = 4096;
        constexpr GLenum TriangleBufferUsage = GL_DYNAMIC_COPY;
        constexpr int32_t AggregatorElementsPerThread = 4;
        constexpr int32_t AggregatorElementsPerGroup = WorkGroupDim1D * AggregatorElementsPerThread;
        constexpr float WaterDensity = 1024.8103f;
    }

    BuoyancySystem::BuoyancySystem()
        : _triangleBuffer(CreateTriangleBuffers()),
          _triangleProcessorComputeShader(
              ShaderBuilder().SetStage(
                  "Assets/Shaders/Water/Buoyancy/TriangleProcessor.comp",
                  ShaderStage::ComputeShader).Build()),
          _triangleAggregationComputeShader(
              ShaderBuilder().SetStage(
                  "Assets/Shaders/Water/Buoyancy/TriangleAggregator.comp",
                  ShaderStage::ComputeShader).Build()),
          _outputComputeShader(
              ShaderBuilder().SetStage(
                  "Assets/Shaders/Water/Buoyancy/TriangleOutput.comp",
                  ShaderStage::ComputeShader).Build()),
          _meshDataUbo(CreateMeshDataUbo()),
          _windVelocityLocation(_triangleProcessorComputeShader.GetUniformLocation("WindVelocity")),
          _triangleCountLocation(_triangleAggregationComputeShader.GetUniformLocation("TriangleCount")),
          _partialsCountLocation(_outputComputeShader.GetUniformLocation("PartialsCount"))
    {
    }

    BuoyancySystem::~BuoyancySystem()
    {
        glDeleteBuffers(1, &_triangleBuffer.MainBufferId);
        glDeleteBuffers(1, &_triangleBuffer.PartialSumsBufferId);
        glDeleteBuffers(1, &_triangleBuffer.OutputBufferId);
        glDeleteBuffers(1, &_meshDataUbo);
    }

    void BuoyancySystem::ProcessMeshes(Scene& Scene, PhysicsSystem& PhysicsSystem, const uint32_t DisplacementTextures,
                                       const glm::vec2 WindVelocity, const uint32_t VelocityTextures,
                                       const uint32_t CascadeSettingsBuffer)
    {
        entt::registry* entityRegistry = Scene.GetEntityRegistry();
        const auto view = entityRegistry->view<WaterEffectorComponent, TransformComponent, RigidbodyComponent>();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, DisplacementTextures);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D_ARRAY, VelocityTextures);

        _triangleProcessorComputeShader.Use();
        Shader::SetUniform(_windVelocityLocation, WindVelocity);

        for (const auto [entity,waterEffector, transform, rigidbody] : view.each())
        {
            ProcessMesh(PhysicsSystem, waterEffector.Mesh, transform, rigidbody, CascadeSettingsBuffer);
        }
    }

    void BuoyancySystem::ProcessMesh(PhysicsSystem& PhysicsSystem, const Mesh& Mesh,
                                     const TransformComponent& Transform,
                                     const RigidbodyComponent& Rigidbody,
                                     const uint32_t CascadeSettingsBuffer)
    {
        const uint32_t triangleCount = Mesh.GetTriangleCount();
        if (triangleCount > _triangleBuffer.Capacity)
        {
            ResizeTriangleBuffers(triangleCount);
        }

        glm::vec3 meshVelocity = PhysicsSystem.GetLinearVelocity(Rigidbody);

        const glm::mat4& localToWorld = Transform.GetLocalToWorldMatrix();

        const ShaderMeshData meshData(
            localToWorld, meshVelocity,
            _dragCoefficient,
            Mesh.GetVerticesIndex(), Mesh.GetIndicesIndex(), triangleCount);

        _triangleProcessorComputeShader.Use();

        glBindBuffer(GL_UNIFORM_BUFFER, _meshDataUbo);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ShaderMeshData), &meshData);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, _meshDataUbo);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, CascadeSettingsBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _triangleBuffer.MainBufferId);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, MeshManager::GetVertexBufferId());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, MeshManager::GetElementBufferId());

        Shader::DispatchCompute(static_cast<int32_t>(Math::IntegerDivCeilPositive(triangleCount, WorkGroupDim1D)),
                                1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        _triangleAggregationComputeShader.Use();
        Shader::SetUniform(_triangleCountLocation, static_cast<int32_t>(triangleCount));
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _triangleBuffer.MainBufferId);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _triangleBuffer.PartialSumsBufferId);
        const int32_t aggregationGroups = static_cast<int32_t>(Math::IntegerDivCeilPositive(
            triangleCount, AggregatorElementsPerGroup));

        Shader::DispatchCompute(aggregationGroups, 1, 1);

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        _outputComputeShader.Use();
        Shader::SetUniform(_partialsCountLocation, static_cast<uint32_t>(aggregationGroups));
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _triangleBuffer.PartialSumsBufferId);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _triangleBuffer.OutputBufferId);
        Shader::DispatchCompute(1, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);

        GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
        glDeleteSync(fence);

        constexpr int32_t bufferSize = sizeof(TriangleProperties);
        TriangleProperties properties{};
        memcpy(&properties, _triangleBuffer.MappedOutputBuffer, bufferSize);
        if (properties.SubmergedVolume <= 1e-3f)
        {
            PhysicsSystem.AddForce(Rigidbody, properties.Drag, Transform.GetWorldPosition());
            return;
        }
        const glm::vec3 buoyancy = glm::vec3(0.0f, 9.80665f * properties.SubmergedVolume * WaterDensity, 0.0f);
        const glm::vec3 centerOfImmersionWorldSpace = glm::vec3(
            localToWorld * glm::vec4(properties.CenterOfImmersion, 1.0f));
        PhysicsSystem.AddForce(Rigidbody, buoyancy + properties.Drag, centerOfImmersionWorldSpace);
    }

    void BuoyancySystem::ResizeTriangleBuffers(const uint32_t TriangleCount)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _triangleBuffer.MainBufferId);
        glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<int64_t>(sizeof(TriangleProperties) * TriangleCount),
                     nullptr,
                     TriangleBufferUsage);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _triangleBuffer.PartialSumsBufferId);
        const int32_t elementCount = static_cast<int32_t>(Math::IntegerDivCeilPositive(
            TriangleCount, AggregatorElementsPerGroup));
        glBufferData(
            GL_SHADER_STORAGE_BUFFER,
            static_cast<int64_t>(sizeof(TriangleProperties) * elementCount),
            nullptr,
            TriangleBufferUsage);

        _triangleBuffer.Capacity = TriangleCount;
    }

    BuoyancySystem::TriangleBuffer BuoyancySystem::CreateTriangleBuffers()
    {
        uint32_t ids[3]{};
        glGenBuffers(3, ids);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ids[0]);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(TriangleProperties) * DefaultTriangleBufferSize, nullptr,
                     TriangleBufferUsage);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ids[1]);
        glBufferData(
            GL_SHADER_STORAGE_BUFFER,
            static_cast<int64_t>(Math::IntegerDivCeilPositive(
                sizeof(TriangleProperties) * DefaultTriangleBufferSize,
                AggregatorElementsPerGroup)),
            nullptr, TriangleBufferUsage);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ids[2]);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(TriangleProperties), nullptr,
                        GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);

        TriangleProperties* ptr = static_cast<TriangleProperties*>(glMapBufferRange(
            GL_SHADER_STORAGE_BUFFER, 0, sizeof(TriangleProperties),
            GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT));

        return TriangleBuffer{ids[0], ids[1], ids[2], DefaultTriangleBufferSize, ptr};
    }

    uint32_t BuoyancySystem::CreateMeshDataUbo()
    {
        uint32_t id = 0;
        glGenBuffers(1, &id);
        glBindBuffer(GL_UNIFORM_BUFFER, id);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(ShaderMeshData), nullptr, GL_DYNAMIC_DRAW);
        return id;
    }
}
