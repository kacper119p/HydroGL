#include "WaterEffectorsRenderer.h"

#include "Meshes/MeshManager.h"
#include "Ocean/Components/WaterEffectorComponent.h"
#include "Rendering/Shaders/ShaderBuilder.h"
#include "Rendering/Shaders/ShaderStage.h"
#include "Scene/TransformComponent.h"


namespace
{
    constexpr int32_t DefaultDebugBufferSize = 32;
}

namespace Core
{
    WaterEffectorsRenderer::DebugShaderUniformLocations::DebugShaderUniformLocations(const Shader SpectrumShader)
        : ViewMatrix(SpectrumShader.GetUniformLocation("ViewMatrix")),
          ProjectionMatrix(SpectrumShader.GetUniformLocation("ProjectionMatrix"))
    {
    }

    WaterEffectorsRenderer::WaterEffectorsRenderer()
        : _debugShader(ShaderBuilder()
              .SetStage("Assets/Shaders/Water/Debug/WaterEffector.vert", ShaderStage::VertexShader).
              SetStage("Assets/Shaders/Water/Debug/WaterEffector.frag", ShaderStage::FragmentShader)
              .Build()),
          _debugShaderUniformLocations(_debugShader),
          _debugIndirectDrawCommandsSsbo(CreateDebugIndirectDrawCommandsSsbo()),
          _debugTransformsSsbo(CreateDebugTransformsSsbo())
    {
        _debugIndirectDrawCommands.resize(DefaultDebugBufferSize);
        _debugTransformMatrices.resize(DefaultDebugBufferSize);
    }

    void WaterEffectorsRenderer::RenderDebug(const entt::registry& EntityRegistry, const glm::mat4& ViewMatrix,
                                             const glm::mat4& ProjectionMatrix)
    {
        if (_render == false)
        {
            return;
        }

        glDisable(GL_CULL_FACE);

        MeshManager::BindVao();

        _debugShader.Use();
        _debugShader.SetUniform(_debugShaderUniformLocations.ViewMatrix, ViewMatrix);
        _debugShader.SetUniform(_debugShaderUniformLocations.ProjectionMatrix, ProjectionMatrix);

        const auto meshesView = EntityRegistry.view<WaterEffectorComponent, TransformComponent>();
        const auto meshesViewComponents = meshesView.each();

        const size_t countHint = meshesView.size_hint();

        if (countHint > _debugIndirectDrawCommands.capacity())
        {
            _debugIndirectDrawCommands.resize(countHint);
            _debugTransformMatrices.resize(countHint);
            glNamedBufferData(_debugIndirectDrawCommandsSsbo,
                              static_cast<GLsizeiptr>(countHint * sizeof(DrawElementsIndirectCommand)),
                              nullptr, GL_DYNAMIC_DRAW);
            glNamedBufferData(_debugTransformsSsbo, static_cast<GLsizeiptr>(countHint * sizeof(glm::mat4)),
                              nullptr, GL_DYNAMIC_DRAW);
        }

        size_t trueCount = 0;

        for (const auto& [entity, waterEffector, transform] : meshesViewComponents)
        {
            _debugTransformMatrices[trueCount++] = transform.GetLocalToWorldMatrix();
        }

        size_t index = 0;
        for (const auto& [entity, waterEffector, transform] : meshesViewComponents)
        {
            _debugIndirectDrawCommands[index++] = {
                waterEffector.Mesh.GetIndicesCount(),
                1,
                waterEffector.Mesh.GetIndicesIndex(),
                static_cast<int32_t>(waterEffector.Mesh.GetVerticesIndex()),
                0
            };
        }

        glNamedBufferSubData(_debugIndirectDrawCommandsSsbo, 0,
                             static_cast<GLsizeiptr>(trueCount * sizeof(DrawElementsIndirectCommand)),
                             _debugIndirectDrawCommands.data());
        glNamedBufferSubData(_debugTransformsSsbo, 0, static_cast<GLsizeiptr>(trueCount * sizeof(glm::mat4)),
                             _debugTransformMatrices.data());

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _debugTransformsSsbo);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, _debugIndirectDrawCommandsSsbo);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(trueCount), 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glEnable(GL_CULL_FACE);
    }

    GlBufferUnique WaterEffectorsRenderer::CreateDebugIndirectDrawCommandsSsbo()
    {
        GlBufferUnique buffer = GlBufferUnique::Create();
        glNamedBufferData(buffer, static_cast<GLsizeiptr>(DefaultDebugBufferSize * sizeof(DrawElementsIndirectCommand)),
                          nullptr, GL_DYNAMIC_DRAW);
        return buffer;
    }

    GlBufferUnique WaterEffectorsRenderer::CreateDebugTransformsSsbo()
    {
        GlBufferUnique buffer = GlBufferUnique::Create();
        glNamedBufferData(buffer, static_cast<GLsizeiptr>(DefaultDebugBufferSize * sizeof(glm::mat4)), nullptr,
                          GL_DYNAMIC_DRAW);
        return buffer;
    }
}
