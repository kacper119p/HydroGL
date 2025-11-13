#include "WaterInteractionSystem.h"

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
        constexpr int64_t DynamicWavesHeaderSize = 16; // max(sizeof(int32_t), 16) for std430 alignment
        constexpr size_t DefaultDynamicWavesCapacity = 32;
    }

    WaterInteractionSystem::WaterInteractionSystem()
        : _triangleBuffer(CreateTriangleBuffers()),
          _dynamicWavesSampleBuffer(CreateDynamicWavesSampleBuffer()),
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
          _meshDataUbo(CreateMeshDataUbo()),
          _intersectionMaskUbo(CreateIntersectionMaskUbo()),
          _interactiveWavesUbo(CreateInteractiveWavesUbo()),
          _windVelocityLocation(_triangleProcessorComputeShader.GetUniformLocation("WindVelocity")),
          _triangleCountLocation(_triangleAggregationComputeShader.GetUniformLocation("TriangleCount")),
          _partialsCountLocation(_outputComputeShader.GetUniformLocation("PartialsCount"))
    {
    }

    WaterInteractionSystem::~WaterInteractionSystem()
    {
        glDeleteBuffers(1, &_triangleBuffer.MainBufferId);
        glDeleteBuffers(1, &_triangleBuffer.PartialSumsBufferId);
        glDeleteBuffers(1, &_triangleBuffer.OutputBufferId);
        glDeleteBuffers(1, &_triangleBuffer.IntersectionPolygonsId);
        glDeleteBuffers(1, &_dynamicWavesSampleBuffer.GpuBufferId);
        free(_dynamicWavesSampleBuffer.DynamicWavesBuffer);
        glDeleteBuffers(1, &_meshDataUbo);
        glDeleteBuffers(1, &_intersectionMaskUbo);
        glDeleteBuffers(1, &_interactiveWavesUbo);
    }

    void WaterInteractionSystem::ProcessMeshes(Scene& Scene, PhysicsSystem& PhysicsSystem,
                                               const uint32_t DisplacementTextures,
                                               const glm::vec2 WindVelocity,
                                               const uint32_t VelocityTextures,
                                               const uint32_t CascadeSettingsBuffer,
                                               const float DeltaTime)
    {
        entt::registry* entityRegistry = Scene.GetEntityRegistry();
        const ComponentsViewType view = entityRegistry->view<WaterEffectorComponent, TransformComponent,
            RigidbodyComponent>();

        glBindTextureUnit(0, DisplacementTextures);
        glBindTextureUnit(1, VelocityTextures);

        _triangleProcessorComputeShader.Use();
        Shader::SetUniform(_windVelocityLocation, WindVelocity);

        for (const auto [entity,waterEffector, transform, rigidbody] : view.each())
        {
            ProcessMesh(PhysicsSystem, waterEffector, transform, rigidbody, CascadeSettingsBuffer, DeltaTime);
        }

        UpdateDynamicWavesSampleSsbo(view);
    }

    void WaterInteractionSystem::ProcessMesh(PhysicsSystem& PhysicsSystem,
                                             WaterEffectorComponent& WaterEffectorComponent,
                                             const TransformComponent& Transform,
                                             const RigidbodyComponent& Rigidbody,
                                             const uint32_t CascadeSettingsBuffer,
                                             const float DeltaTime)
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
        DispatchInteractiveWavesCompute(WaterEffectorComponent, meshVelocity, worldPosition, DeltaTime);
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
        WaterEffectorComponent& WaterEffectorComponent, const glm::vec3& WorldPosition) const
    {
        _intersectionMaskComputeShader.Use();
        const IntersectionMaskUbo uboData{
            {WorldPosition.x, WorldPosition.z}, WaterEffectorComponent.IntersectionMaskFieldSize
        };
        std::swap(WaterEffectorComponent.MaskTextures[0], WaterEffectorComponent.MaskTextures[1]);
        glNamedBufferSubData(_intersectionMaskUbo, 0, sizeof(IntersectionMaskUbo), &uboData);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, _intersectionMaskUbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _triangleBuffer.IntersectionPolygonsId);
        glBindImageTexture(0, WaterEffectorComponent.MaskTextures[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

        Shader::DispatchCompute(WaterEffectorComponent.TexturesResolution / WorkGroupDim2D,
                                WaterEffectorComponent.TexturesResolution / WorkGroupDim2D,
                                1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

    void WaterInteractionSystem::DispatchInteractiveWavesCompute(WaterEffectorComponent& WaterEffectorComponent,
                                                                 const glm::vec3& ObjectVelocity,
                                                                 const glm::vec3& WorldPosition,
                                                                 const float DeltaTime) const
    {
        const float texelPhysicalSize = WaterEffectorComponent.InteractiveWavesFieldSize / static_cast<float>(
                                            WaterEffectorComponent.TexturesResolution);
        const float c = std::sqrtf(0.49f) * texelPhysicalSize / DeltaTime;
        const float aFactor = c * c * DeltaTime * DeltaTime / (texelPhysicalSize * texelPhysicalSize);

        const float dampingAlpha = std::clamp(glm::length(ObjectVelocity) / WaterEffectorComponent.DampingMaxVelocity,
                                              0.0f,
                                              1.0f);
        const float damping = Math::Lerp(WaterEffectorComponent.WaveDampingMin,
                                         WaterEffectorComponent.WaveDampingMax,
                                         dampingAlpha);

        const uint32_t tmp = WaterEffectorComponent.InteractiveWavesTextures[0];
        WaterEffectorComponent.InteractiveWavesTextures[0] = WaterEffectorComponent.InteractiveWavesTextures[1];
        WaterEffectorComponent.InteractiveWavesTextures[1] = WaterEffectorComponent.InteractiveWavesTextures[2];
        WaterEffectorComponent.InteractiveWavesTextures[2] = tmp;

        const float displacementStrength = glm::clamp(
            glm::length(ObjectVelocity) / WaterEffectorComponent.DampingMaxVelocity * WaterEffectorComponent.
            DisplacementStrength, 0.0f, 1.0f);

        const glm::vec2 worldPos2d(WorldPosition.x, WorldPosition.z);
        const InteractiveWavesShaderUbo uboData
        {
            WaterEffectorComponent.PreviousPositions[0],
            WaterEffectorComponent.PreviousPositions[1],
            worldPos2d,
            WaterEffectorComponent.InteractiveWavesFieldSize,
            WaterEffectorComponent.IntersectionMaskFieldSize,
            damping,
            aFactor,
            displacementStrength
        };

        _interactiveWavesComputeShader.Use();
        glBindTextureUnit(0, WaterEffectorComponent.MaskTextures[0]);
        glBindTextureUnit(1, WaterEffectorComponent.MaskTextures[1]);
        glBindImageTexture(2, WaterEffectorComponent.InteractiveWavesTextures[0], 0, GL_FALSE, 0, GL_READ_ONLY,
                           GL_R32F);
        glBindImageTexture(3, WaterEffectorComponent.InteractiveWavesTextures[1], 0, GL_FALSE, 0, GL_READ_ONLY,
                           GL_R32F);
        glBindImageTexture(4, WaterEffectorComponent.InteractiveWavesTextures[2], 0, GL_FALSE, 0, GL_WRITE_ONLY,
                           GL_R32F);

        glNamedBufferSubData(_interactiveWavesUbo, 0, sizeof(InteractiveWavesShaderUbo), &uboData);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, _interactiveWavesUbo);

        const int32_t simulationZonePixelSize = WaterEffectorComponent.TexturesResolution - 32;
        Shader::DispatchCompute(simulationZonePixelSize / WorkGroupDim2D,
                                simulationZonePixelSize / WorkGroupDim2D,
                                1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        WaterEffectorComponent.PreviousPositions[0] = WaterEffectorComponent.PreviousPositions[1];
        WaterEffectorComponent.PreviousPositions[1] = worldPos2d;
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

    void WaterInteractionSystem::ResizeDynamicWavesSampleSsbo(const uint32_t Count)
    {
        const int64_t bufferSize = DynamicWavesHeaderSize +
                                   static_cast<int64_t>(Count * sizeof(DynamicWavesSampleSsbo));
        _dynamicWavesSampleBuffer.DynamicWavesBuffer
                = static_cast<DynamicWavesSampleSsbo*>(
                    realloc(_dynamicWavesSampleBuffer.DynamicWavesBuffer, bufferSize));
        glNamedBufferData(_dynamicWavesSampleBuffer.GpuBufferId, bufferSize, nullptr, GL_DYNAMIC_COPY);
        _dynamicWavesSampleBuffer.Capacity = static_cast<int32_t>(Count);
    }

    void WaterInteractionSystem::UpdateDynamicWavesSampleSsbo(const ComponentsViewType& View)
    {
        const uint32_t sizeHint = View.size_hint();
        if (sizeHint > _dynamicWavesSampleBuffer.Capacity)
        {
            ResizeDynamicWavesSampleSsbo(sizeHint);
        }
        int32_t& count = _dynamicWavesSampleBuffer.DynamicWavesBuffer->DynamicWavesCount;
        count = 0;
        for (const auto [entity,waterEffector, transform, rigidbody] : View.each())
        {
            _dynamicWavesSampleBuffer.DynamicWavesBuffer->DynamicWaves[count]
                    = DynamicWaveSampleParameters{
                        transform.GetWorldPosition(),
                        waterEffector.InteractiveWavesFieldSize,
                        waterEffector.InteractiveWavesTextureHandles[2]
                    };
            ++count;
        }
        glNamedBufferSubData(_dynamicWavesSampleBuffer.GpuBufferId,
                             0,
                             DynamicWavesHeaderSize + count * static_cast<int64_t>(sizeof(DynamicWaveSampleParameters)),
                             _dynamicWavesSampleBuffer.DynamicWavesBuffer);
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
        glNamedBufferStorage(id, sizeof(ShaderMeshData), nullptr, GL_DYNAMIC_STORAGE_BIT);
        return id;
    }

    uint32_t WaterInteractionSystem::CreateIntersectionMaskUbo()
    {
        uint32_t id = 0;
        glCreateBuffers(1, &id);
        glNamedBufferStorage(id, sizeof(IntersectionMaskUbo), nullptr, GL_DYNAMIC_STORAGE_BIT);
        return id;
    }

    uint32_t WaterInteractionSystem::CreateInteractiveWavesUbo()
    {
        uint32_t id = 0;
        glCreateBuffers(1, &id);
        glNamedBufferStorage(id, sizeof(InteractiveWavesShaderUbo), nullptr, GL_DYNAMIC_STORAGE_BIT);
        return id;
    }

    WaterInteractionSystem::DynamicWavesSampleBuffer WaterInteractionSystem::CreateDynamicWavesSampleBuffer()
    {
        uint32_t id = 0;
        constexpr int64_t bufferSize = DynamicWavesHeaderSize
                                       + DefaultDynamicWavesCapacity * sizeof(DynamicWavesSampleSsbo);
        DynamicWavesSampleSsbo* data = static_cast<DynamicWavesSampleSsbo*>(malloc(bufferSize));
        glCreateBuffers(1, &id);
        glNamedBufferData(id, bufferSize, nullptr, GL_DYNAMIC_COPY);
        return DynamicWavesSampleBuffer{id, DefaultDynamicWavesCapacity, data};
    }
}
