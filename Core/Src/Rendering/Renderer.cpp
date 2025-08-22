#include "Renderer.h"

#include "Debug.h"
#include "Camera/CameraComponent.h"
#include "Components/RenderedMeshComponent.h"
#include "Meshes/MeshManager.h"
#include "RendererHelpers/DrawElementsIndirectCommand.h"
#include "Rendering/RendererHelpers/CubeRenderer.h"
#include "Rendering/Shaders/ShaderBuilder.h"
#include "Scene/TransformComponent.h"

namespace Core
{
    Renderer::Renderer(const glm::uvec2& Resolution) :
        _resolution(Resolution), _skyboxShader(ShaderBuilder().
                SetStage("Assets/Shaders/Skybox/Skybox.vert", ShaderStage::VertexShader)
                .SetStage("Assets/Shaders/Skybox/Skybox.frag", ShaderStage::FragmentShader)
                .Build()),
        _skyboxViewMatrixLocation(_skyboxShader.GetUniformLocation("ViewMatrix")),
        _skyboxProjectionMatrixLocation(_skyboxShader.GetUniformLocation("ProjectionMatrix")),
        _pbrShader(ShaderBuilder().SetStage("Assets/Shaders/PBR/PBR.vert", Core::ShaderStage::VertexShader)
                .SetStage("Assets/Shaders/PBR/PBR.frag", Core::ShaderStage::FragmentShader)
                .Build()),
        _pbrViewMatrixLocation(_pbrShader.GetUniformLocation("ViewMatrix")),
        _pbrProjectionMatrixLocation(_pbrShader.GetUniformLocation("ProjectionMatrix"))
    {
    }

    Renderer::~Renderer()
    {
    }

    void Renderer::RenderFrame(const Scene& Scene, const MaterialManager& MaterialManager)
    {
        const entt::registry* entityRegistry = Scene.GetEntityRegistry();

        const auto cameraView = entityRegistry->view<CameraComponent, TransformComponent>();
        const entt::entity cameraEntity = cameraView.front();

        CHECK_MESSAGE(entityRegistry->valid(cameraEntity), "Camera entity doesn't exist!");

        const CameraComponent& camera = cameraView.get<CameraComponent>(cameraEntity);
        const TransformComponent& cameraTransform = cameraView.get<TransformComponent>(cameraEntity);
        glDepthMask(GL_TRUE);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glViewport(0, 0, static_cast<int32_t>(_resolution.x), static_cast<int32_t>(_resolution.y));
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glCullFace(GL_BACK);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // ------------------------ MESHES ------------------------

        MeshManager::BindVao();

        MaterialManager.BindMaterialBuffer();
        const auto meshesView = entityRegistry->view<RenderedMeshComponent, TransformComponent>();

        uint32_t renderedMeshes = 0;
        for (const entt::entity entity : meshesView)
        {
            const RenderedMeshComponent& renderedMesh = meshesView.get<RenderedMeshComponent>(entity);

            DrawElementsIndirectCommand cmd{renderedMesh.Mesh.GetIndicesCount(),
                                            2,
                                            renderedMesh.Mesh.GetIndicesIndex(),
                                            renderedMesh.Mesh.GetVerticesIndex(),
                                            renderedMesh.Material};

            const TransformComponent& transform = meshesView.get<TransformComponent>(entity);

            _pbrShader.Use();

            _pbrShader.SetUniform(_pbrViewMatrixLocation, glm::inverse(cameraTransform.GetLocalToWorldMatrix()));
            _pbrShader.SetUniform(_pbrProjectionMatrixLocation, camera.ProjectionMatrix);
            _pbrShader.SetUniform("ObjectToWorldMatrix", transform.GetLocalToWorldMatrix());

            MeshManager::BindVao();
            GLuint indirectBuffer;
            glGenBuffers(1, &indirectBuffer);
            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer);
            glBufferData(GL_DRAW_INDIRECT_BUFFER, 1 * sizeof(DrawElementsIndirectCommand), &cmd, GL_STATIC_DRAW);

            glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, 1, 0);

            glDeleteBuffers(1, &indirectBuffer);
            renderedMeshes++;
        }

        // ------------------------ SKYBOX ------------------------

        _skyboxShader.Use();
        Shader::SetUniform(_skyboxViewMatrixLocation, glm::inverse(cameraTransform.GetLocalToWorldMatrix()));
        Shader::SetUniform(_skyboxProjectionMatrixLocation, camera.ProjectionMatrix);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, Scene.GetSkybox().GetCubeMap());

        CubeRenderer::Draw();
    }
}
