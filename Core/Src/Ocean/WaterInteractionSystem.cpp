#include "WaterInteractionSystem.h"

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
        constexpr int32_t WorkGroupDim2D = 8;
        constexpr size_t DefaultTriangleBufferSize = 4096;
        constexpr GLenum TriangleBufferUsage = GL_DYNAMIC_COPY;
        constexpr int32_t AggregatorElementsPerThread = 4;
        constexpr int32_t AggregatorElementsPerGroup = WorkGroupDim1D * AggregatorElementsPerThread;
        constexpr float WaterDensity = 1024.8103f;
        constexpr int64_t IntersectionPolygonsHeaderSize = 8; // max(sizeof(int32_t), 8) for std430 alignment
    }

    WaterInteractionSystem::WaterInteractionSystem()
        : _triangleBuffer(CreateTriangleBuffers()),
          _triangleProcessorComputeShader(
              ShaderBuilder().SetStage(
                  "Assets/Shaders/Water/WaterInteraction/TriangleProcessor.comp",
                  ShaderStage::ComputeShader).Build()),
          _triangleAggregationComputeShader(
              ShaderBuilder().SetStage(
                  "Assets/Shaders/Water/WaterInteraction/TriangleAggregator.comp",
                  ShaderStage::ComputeShader).Build()),
          _outputComputeShader(
              ShaderBuilder().SetStage(
                  "Assets/Shaders/Water/WaterInteraction/TriangleOutput.comp",
                  ShaderStage::ComputeShader).Build()),
          _intersectionMaskComputeShader(ShaderBuilder().SetStage(
              "Assets/Shaders/Water/WaterInteraction/IntersectionMask.comp",
              ShaderStage::ComputeShader).Build()),
          _interactiveWavesComputeShader(ShaderBuilder().SetStage(
              "Assets/Shaders/Water/WaterInteraction/InteractiveWave.comp",
              ShaderStage::ComputeShader).Build()),
          _intersectionMaskUbo(CreateIntersectionMaskUbo()),
          _meshDataUbo(CreateMeshDataUbo()),
          _windVelocityLocation(_triangleProcessorComputeShader.GetUniformLocation("WindVelocity")),
          _triangleCountLocation(_triangleAggregationComputeShader.GetUniformLocation("TriangleCount")),
          _partialsCountLocation(_outputComputeShader.GetUniformLocation("PartialsCount"))
    {
        spdlog::info("{}", _triangleBuffer.IntersectionPolygonsId);
    }

    WaterInteractionSystem::~WaterInteractionSystem()
    {
        glDeleteBuffers(1, &_triangleBuffer.MainBufferId);
        glDeleteBuffers(1, &_triangleBuffer.PartialSumsBufferId);
        glDeleteBuffers(1, &_triangleBuffer.OutputBufferId);
        glDeleteBuffers(1, &_triangleBuffer.IntersectionPolygonsId);
        glDeleteBuffers(1, &_meshDataUbo);
        glDeleteBuffers(1, &_intersectionMaskUbo);
    }

    void WaterInteractionSystem::ProcessMeshes(Scene& Scene, PhysicsSystem& PhysicsSystem,
                                               const uint32_t DisplacementTextures,
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
            ProcessMesh(PhysicsSystem, waterEffector, transform, rigidbody, CascadeSettingsBuffer);
        }
    }

    void WaterInteractionSystem::ProcessMesh(PhysicsSystem& PhysicsSystem,
                                             const WaterEffectorComponent& WaterEffectorComponent,
                                             const TransformComponent& Transform,
                                             const RigidbodyComponent& Rigidbody,
                                             const uint32_t CascadeSettingsBuffer)
    {
        const Mesh& mesh = WaterEffectorComponent.Mesh;
        const uint32_t triangleCount = mesh.GetTriangleCount();
        if (triangleCount > _triangleBuffer.Capacity)
        {
            ResizeTriangleBuffers(triangleCount);
        }

        const glm::vec3 meshVelocity = PhysicsSystem.GetLinearVelocity(Rigidbody);
        const glm::mat4& localToWorld = Transform.GetLocalToWorldMatrix();
        const glm::vec3 worldPosition = Transform.GetWorldPosition();

        ResetIntersectionPolygonsBuffer();
        DispatchTriangleProcessing(mesh, triangleCount, meshVelocity, localToWorld, CascadeSettingsBuffer);

        const int32_t aggregationGroups = static_cast<int32_t>(Math::IntegerDivCeilPositive(
            triangleCount, AggregatorElementsPerGroup));
        DispatchTriangleAggregation(triangleCount, aggregationGroups);
        DispatchOutputProcessing(aggregationGroups);
        ApplyForces(PhysicsSystem, Rigidbody, localToWorld, worldPosition);
        DispatchIntersectionMaskCompute(WaterEffectorComponent, worldPosition);
    }

    void WaterInteractionSystem::DispatchTriangleProcessing(const Mesh& Mesh,
                                                            const uint32_t TriangleCount,
                                                            const glm::vec3& MeshVelocity,
                                                            const glm::mat4& LocalToWorldMatrix,
                                                            const uint32_t CascadeSettingsBuffer) const
    {
        const ShaderMeshData meshData(
            LocalToWorldMatrix, MeshVelocity,
            _dragCoefficient,
            Mesh.GetVerticesIndex(), Mesh.GetIndicesIndex(), TriangleCount);

        _triangleProcessorComputeShader.Use();

        glNamedBufferSubData(_meshDataUbo, 0, sizeof(ShaderMeshData), &meshData);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, _meshDataUbo);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, CascadeSettingsBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _triangleBuffer.MainBufferId);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, MeshManager::GetVertexBufferId());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, MeshManager::GetElementBufferId());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, _triangleBuffer.IntersectionPolygonsId);

        Shader::DispatchCompute(static_cast<int32_t>(Math::IntegerDivCeilPositive(TriangleCount, WorkGroupDim1D)),
                                1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    auto WaterInteractionSystem::DispatchTriangleAggregation(const uint32_t TriangleCount,
                                                             const int32_t AggregationGroupsCount) const -> void
    {
        _triangleAggregationComputeShader.Use();
        Shader::SetUniform(_triangleCountLocation, static_cast<int32_t>(TriangleCount));
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _triangleBuffer.MainBufferId);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _triangleBuffer.PartialSumsBufferId);

        Shader::DispatchCompute(AggregationGroupsCount, 1, 1);

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    void WaterInteractionSystem::DispatchOutputProcessing(uint32_t AggregationGroupsCount) const
    {
        _outputComputeShader.Use();
        Shader::SetUniform(_partialsCountLocation, static_cast<uint32_t>(AggregationGroupsCount));
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _triangleBuffer.PartialSumsBufferId);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _triangleBuffer.OutputBufferId);
        Shader::DispatchCompute(1, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
    }

    void WaterInteractionSystem::DispatchIntersectionMaskCompute(
        const WaterEffectorComponent& WaterEffectorComponent, const glm::vec3& WorldPosition) const
    {
        _intersectionMaskComputeShader.Use();
        const IntersectionMaskUbo uboData{{WorldPosition.x, WorldPosition.z}, WaterEffectorComponent.IntersectionMaskPhysicalSize};
        glNamedBufferSubData(_intersectionMaskUbo, 0, sizeof(IntersectionMaskUbo), &uboData);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, _intersectionMaskUbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _triangleBuffer.IntersectionPolygonsId);
        glBindImageTexture(0, WaterEffectorComponent.MaskTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

        Shader::DispatchCompute(WaterEffectorComponent.InteractiveWavesTextureResolution / WorkGroupDim2D,
                                WaterEffectorComponent.InteractiveWavesTextureResolution / WorkGroupDim2D,
                                1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

    void WaterInteractionSystem::ApplyForces(PhysicsSystem& PhysicsSystem, const RigidbodyComponent& Rigidbody,
                                             const glm::mat4& LocalToWorldMatrix, const glm::vec3& WorldPosition) const
    {
        GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
        glDeleteSync(fence);

        const TriangleProperties& properties = *_triangleBuffer.MappedOutputBuffer;
        if (properties.SubmergedVolume <= 1e-3f)
        {
            PhysicsSystem.AddForce(Rigidbody, properties.Drag, WorldPosition);
            return;
        }
        const glm::vec3 buoyancy = glm::vec3(0.0f, 9.80665f * properties.SubmergedVolume * WaterDensity, 0.0f);
        const glm::vec3 centerOfImmersionWorldSpace = glm::vec3(
            LocalToWorldMatrix * glm::vec4(properties.CenterOfImmersion, 1.0f));
        PhysicsSystem.AddForce(Rigidbody, buoyancy + properties.Drag, centerOfImmersionWorldSpace);
    }

    void WaterInteractionSystem::ResizeTriangleBuffers(const uint32_t TriangleCount)
    {
        glNamedBufferData(_triangleBuffer.MainBufferId,
                          static_cast<GLsizeiptr>(sizeof(TriangleProperties) * TriangleCount),
                          nullptr, TriangleBufferUsage);
        const int32_t elementCount = static_cast<int32_t>(Math::IntegerDivCeilPositive(
            TriangleCount, AggregatorElementsPerGroup));
        glNamedBufferData(_triangleBuffer.PartialSumsBufferId,
                          static_cast<GLsizeiptr>(sizeof(TriangleProperties) * elementCount),
                          nullptr, TriangleBufferUsage);
        glNamedBufferData(_triangleBuffer.IntersectionPolygonsId,
                          static_cast<int64_t>(IntersectionPolygonsHeaderSize + sizeof(IntersectionSegment) *
                                               TriangleCount),
                          nullptr, TriangleBufferUsage);

        _triangleBuffer.Capacity = TriangleCount;
    }

    void WaterInteractionSystem::ResetIntersectionPolygonsBuffer() const
    {
        constexpr int32_t data = 0;
        glNamedBufferSubData(_triangleBuffer.IntersectionPolygonsId, 0, sizeof(int32_t), &data);
    }

    WaterInteractionSystem::TriangleBuffer WaterInteractionSystem::CreateTriangleBuffers()
    {
        uint32_t ids[4]{};
        glCreateBuffers(4, ids);
        glNamedBufferData(ids[0], sizeof(TriangleProperties) * DefaultTriangleBufferSize, nullptr, TriangleBufferUsage);
        glNamedBufferData(
            ids[1], static_cast<int64_t>(Math::IntegerDivCeilPositive(
                sizeof(TriangleProperties) * DefaultTriangleBufferSize, AggregatorElementsPerGroup)), nullptr,
            TriangleBufferUsage);
        glNamedBufferStorage(ids[2], sizeof(TriangleProperties), nullptr,
                             GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
        TriangleProperties* ptr = static_cast<TriangleProperties*>(glMapNamedBufferRange(
            ids[2], 0, sizeof(TriangleProperties), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT));
        glNamedBufferData(
            ids[3], IntersectionPolygonsHeaderSize + sizeof(IntersectionSegment) * DefaultTriangleBufferSize, nullptr,
            TriangleBufferUsage);

        return TriangleBuffer{ids[0], ids[1], ids[2], ids[3], DefaultTriangleBufferSize, ptr};
    }

    uint32_t WaterInteractionSystem::CreateMeshDataUbo()
    {
        uint32_t id = 0;
        glCreateBuffers(1, &id);
        glNamedBufferData(id, sizeof(ShaderMeshData), nullptr, GL_DYNAMIC_DRAW);
        return id;
    }

    uint32_t WaterInteractionSystem::CreateIntersectionMaskUbo()
    {
        uint32_t id = 0;
        glCreateBuffers(1, &id);
        glNamedBufferData(id, sizeof(IntersectionMaskUbo), nullptr, GL_DYNAMIC_DRAW);
        return id;
    }
}
