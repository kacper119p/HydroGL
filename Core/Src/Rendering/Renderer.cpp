#include "Renderer.h"

#include "Debug.h"
#include "Camera/CameraComponent.h"
#include "Rendering/RendererHelpers/CubeRenderer.h"
#include "Rendering/Shaders/ShaderBuilder.h"
#include "Scene/TransformComponent.h"

namespace Core
{
    glm::uvec2 Renderer::_resolution;
    Shader Renderer::_skyboxShader;
    int32_t Renderer::_skyboxViewMatrixLocation;
    int32_t Renderer::_skyboxProjectionMatrixLocation;

    void Renderer::Initialize(const glm::uvec2& Resolution)
    {
        _resolution = Resolution;
        _skyboxShader = ShaderBuilder().
                SetStage("Assets/Shaders/Skybox/Skybox.vert", ShaderStage::VertexShader)
                .SetStage("Assets/Shaders/Skybox/Skybox.frag", ShaderStage::FragmentShader)
                .Build();
        _skyboxViewMatrixLocation = _skyboxShader.GetUniformLocation("ViewMatrix");
        _skyboxProjectionMatrixLocation = _skyboxShader.GetUniformLocation("ProjectionMatrix");
    }

    void Renderer::Destroy()
    {
    }

    void Renderer::RenderFrame(const Scene& Scene)
    {
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
        const entt::registry* entityRegistry = Scene.GetEntityRegistry();

        const auto cameraView = entityRegistry->view<CameraComponent, TransformComponent>();
        const entt::entity cameraEntity = cameraView.front();

        CHECK_MESSAGE(entityRegistry->valid(cameraEntity), "Camera entity doesn't exist!");

        const CameraComponent& camera = cameraView.get<CameraComponent>(cameraEntity);
        const TransformComponent& cameraTransform = cameraView.get<TransformComponent>(cameraEntity);

        _skyboxShader.Use();
        Shader::SetUniform(_skyboxViewMatrixLocation, cameraTransform.GetLocalToWorldMatrix());
        Shader::SetUniform(_skyboxProjectionMatrixLocation, camera.ProjectionMatrix);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, Scene.GetSkybox().GetCubeMap());

        CubeRenderer::Draw();
    }
}
