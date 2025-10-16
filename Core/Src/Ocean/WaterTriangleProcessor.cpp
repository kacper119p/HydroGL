#include "WaterTriangleProcessor.h"

#include "glad/glad.h"
#include "Meshes/MeshManager.h"
#include "Ocean/WaterEffectorComponent.h"
#include "Rendering/Shaders/ShaderBuilder.h"
#include "Scene/TransformComponent.h"

namespace Core
{
    namespace
    {
        constexpr int32_t WorkGroupDim1D = 64;
        constexpr size_t DefaultTriangleBufferSize = 4096;
        constexpr GLenum TriangleBufferUsage = GL_DYNAMIC_READ;
    }

    WaterTriangleProcessor::WaterTriangleProcessor() : _triangleBuffer(CreateTriangleBuffer()),
                                                       _triangleProcessorShader(
                                                           ShaderBuilder().SetStage(
                                                               "Assets/Shaders/Water/Buoyancy/TriangleProcessor.comp",
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
            ResizeTriangleBuffer(triangleCount);
        }

        const ShaderMeshData meshData{
            Transform.GetLocalToWorldMatrix(),
            Mesh.GetVerticesIndex(), Mesh.GetIndicesIndex(), triangleCount,
            0.0f
        };

        _triangleProcessorShader.Use();

        glBindBuffer(GL_UNIFORM_BUFFER, _meshDataUbo);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ShaderMeshData), &meshData);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, _meshDataUbo);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _triangleBuffer.Id);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, MeshManager::GetVertexBufferId());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, MeshManager::GetElementBufferId());

        Shader::DispatchCompute(triangleCount / WorkGroupDim1D, 1, 1);
    }

    void WaterTriangleProcessor::ResizeTriangleBuffer(const uint32_t TriangleCount) const
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _triangleBuffer.Id);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(TriangleProperties) * TriangleCount, nullptr,
                     TriangleBufferUsage);
    }

    WaterTriangleProcessor::TriangleBuffer WaterTriangleProcessor::CreateTriangleBuffer()
    {
        uint32_t id = 0;
        glGenBuffers(1, &id);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(TriangleProperties) * DefaultTriangleBufferSize, nullptr,
                     TriangleBufferUsage);
        return TriangleBuffer{id, DefaultTriangleBufferSize};
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
