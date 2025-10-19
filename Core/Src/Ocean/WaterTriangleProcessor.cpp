#include "WaterTriangleProcessor.h"

#include "glad/glad.h"
#include "Math/Math.h"
#include "Meshes/MeshManager.h"
#include "Components/WaterEffectorComponent.h"
#include "Rendering/Shaders/ShaderBuilder.h"
#include "Scene/TransformComponent.h"

namespace Core
{
    namespace
    {
        constexpr int32_t WorkGroupDim1D = 64;
        constexpr size_t DefaultTriangleBufferSize = 4096;
        constexpr GLenum TriangleBufferUsage = GL_DYNAMIC_READ;
        constexpr int32_t AggregatorElementsPerThread = 4;
        constexpr int32_t AggregatorElementsPerGroup = WorkGroupDim1D * AggregatorElementsPerThread;
    }

    WaterTriangleProcessor::WaterTriangleProcessor()
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

    void WaterTriangleProcessor::ProcessMeshes(Scene& Scene, const uint32_t DisplacementTextures) const
    {
        entt::registry* entityRegistry = Scene.GetEntityRegistry();
        auto view = entityRegistry->view<WaterEffectorComponent, TransformComponent>();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, DisplacementTextures);

        for (const entt::entity entity : view)
        {
            WaterEffectorComponent& waterEffector = view.get<WaterEffectorComponent>(entity);
            TransformComponent& transform = view.get<TransformComponent>(entity);
            ProcessMesh(waterEffector.GetMesh(), transform);
        }
    }

    void WaterTriangleProcessor::ProcessMesh(const Mesh& Mesh, const TransformComponent& Transform) const
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

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _triangleBuffer.MainBufferId);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, MeshManager::GetVertexBufferId());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, MeshManager::GetElementBufferId());

        Shader::DispatchCompute(Math::IntegerDivCeil(triangleCount, WorkGroupDim1D), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        _triangleAggregationComputeShader.Use();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _triangleBuffer.PartialSumsBufferId);
        const int32_t aggregationGroups = Math::IntegerDivCeil(sizeof(TriangleProperties) * triangleCount,
                                                               AggregatorElementsPerGroup);

        Shader::DispatchCompute(aggregationGroups, 1, 1);

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        _outputComputeShader.Use();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _triangleBuffer.OutputBufferId);
        Shader::DispatchCompute(Math::IntegerDivCeil(aggregationGroups, WorkGroupDim1D), 1, 1);

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _triangleBuffer.OutputBufferId);

        constexpr int32_t bufferSize = sizeof(TriangleProperties);
        const void* mappedBuffer = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(TriangleProperties),
                                                    GL_MAP_READ_BIT);

        std::vector<TriangleProperties> cpuData(bufferSize / sizeof(float));
        memcpy(cpuData.data(), mappedBuffer, bufferSize);

        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    }

    void WaterTriangleProcessor::ResizeTriangleBuffers(const uint32_t TriangleCount) const
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _triangleBuffer.MainBufferId);
        glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<int64_t>(sizeof(TriangleProperties) * TriangleCount),
                     nullptr,
                     TriangleBufferUsage);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _triangleBuffer.PartialSumsBufferId);
        glBufferData(
            GL_SHADER_STORAGE_BUFFER,
            Math::IntegerDivCeil(sizeof(TriangleProperties) * TriangleCount, AggregatorElementsPerGroup),
            nullptr,
            TriangleBufferUsage);
    }

    WaterTriangleProcessor::TriangleBuffer WaterTriangleProcessor::CreateTriangleBuffers()
    {
        uint32_t ids[3]{};
        glGenBuffers(3, ids);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ids[0]);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(TriangleProperties) * DefaultTriangleBufferSize, nullptr,
                     TriangleBufferUsage);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ids[1]);
        glBufferData(
            GL_SHADER_STORAGE_BUFFER,
            static_cast<int64_t>(Math::IntegerDivCeil(
                sizeof(TriangleProperties) * DefaultTriangleBufferSize,
                AggregatorElementsPerGroup)),
            nullptr, TriangleBufferUsage);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ids[2]);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(TriangleProperties), nullptr,
                     TriangleBufferUsage);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(TriangleProperties), nullptr,
                        GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT);

        TriangleProperties* ptr = static_cast<TriangleProperties*>(glMapBufferRange(
            GL_SHADER_STORAGE_BUFFER, 0, sizeof(TriangleProperties),
            GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT));

        return TriangleBuffer{ids[0], ids[1], ids[2], DefaultTriangleBufferSize, ptr};
    }

    uint32_t WaterTriangleProcessor::CreateMeshDataUbo()
    {
        uint32_t id = 0;
        glGenBuffers(1, &id);
        glBindBuffer(GL_UNIFORM_BUFFER, id);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(ShaderMeshData), nullptr, GL_DYNAMIC_DRAW);
        return id;
    }
}
