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
        int32_t i = 0;
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
          _meshDataUbo(CreateMeshDataUbo())
    {
    }

    void BuoyancySystem::ProcessMeshes(Scene& Scene, PhysicsSystem& PhysicsSystem, const uint32_t DisplacementTextures,
                                       const uint32_t CascadeSettingsBuffer)
    {
        entt::registry* entityRegistry = Scene.GetEntityRegistry();
        const auto view = entityRegistry->view<WaterEffectorComponent, TransformComponent, RigidbodyComponent>();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, DisplacementTextures);
        i = 0;
        for (const auto [entity,waterEffector, transform, rigidbody] : view.each())
        {
            ProcessMesh(PhysicsSystem, waterEffector.GetMesh(), transform, rigidbody, CascadeSettingsBuffer);
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

        const ShaderMeshData meshData{
            Transform.GetLocalToWorldMatrix(),
            Mesh.GetVerticesIndex(), Mesh.GetIndicesIndex(), triangleCount,
            0.0f
        };

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

        // // Debug: Check if triangle processor wrote any data
        // GLsync fence1 = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        // glClientWaitSync(fence1, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
        // glDeleteSync(fence1);
        // glBindBuffer(GL_SHADER_STORAGE_BUFFER, _triangleBuffer.MainBufferId);
        // TriangleProperties* triangleData = static_cast<TriangleProperties*>(
        //     glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(TriangleProperties),
        //                      GL_MAP_READ_BIT));
        //
        // spdlog::info("{} BuoyancySystem: First triangle SubmergedVolume: {} SubmergedArea: {}", i,
        //              triangleData[0].SubmergedVolume,
        //              triangleData[0].SubmergedArea);
        // glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

        _triangleAggregationComputeShader.Use();
        _triangleAggregationComputeShader.SetUniform("TriangleCount", static_cast<int32_t>(triangleCount));
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _triangleBuffer.MainBufferId);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _triangleBuffer.PartialSumsBufferId);
        const int32_t aggregationGroups = static_cast<int32_t>(Math::IntegerDivCeilPositive(
            triangleCount, AggregatorElementsPerGroup));

        Shader::DispatchCompute(aggregationGroups, 1, 1);

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        _outputComputeShader.Use();
        _outputComputeShader.SetUniform("PartialsCount", static_cast<uint32_t>(aggregationGroups));
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
        if (properties.SubmergedVolume <= 0.0f)
        {
            return;
        }
        const glm::vec3 buoyancy = glm::vec3(0.0f, 9.80665f * properties.SubmergedVolume * 1000.0f, 0.0f);
        const glm::vec3 centerOfImmersionWorldSpace = glm::vec3(
            Transform.GetLocalToWorldMatrix() * glm::vec4(properties.CenterOfImmersion, 1.0f));
        PhysicsSystem.AddForce(Rigidbody, buoyancy, centerOfImmersionWorldSpace);
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
