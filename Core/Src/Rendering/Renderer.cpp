#include "Renderer.h"

#include "Debug.h"
#include "Camera/CameraComponent.h"
#include "Components/RenderedMeshComponent.h"
#include "Meshes/MeshManager.h"
#include "Rendering/RendererHelpers/CubeRenderer.h"
#include "Rendering/Shaders/ShaderBuilder.h"
#include "Scene/TransformComponent.h"
#include "Textures/CubeMapGenerator.h"

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
        _pbrProjectionMatrixLocation(_pbrShader.GetUniformLocation("ProjectionMatrix")),
        _cameraPositionLocation(_pbrShader.GetUniformLocation("CameraPosition")),
        _brdfLut(CubeMapGenerator::CreateBrdfLut()),
        DirectionalLightProjectionMatrix(glm::ortho(-150.0f, 150.0f, -150.0f, 150.0f, 0.1f, 300.0f))
    {
        glGenBuffers(1, &IndirectDrawCommandsSsbo);
        glGenBuffers(1, &TransformsSsbo);
        glGenBuffers(1, &_lightsSsbo);
        _indirectDrawCommands.reserve(1024);
    }

    Renderer::~Renderer()
    {
        glDeleteBuffers(1, &IndirectDrawCommandsSsbo);
        glDeleteBuffers(1, &TransformsSsbo);
        glDeleteBuffers(1, &_lightsSsbo);
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

        _indirectDrawCommands.clear();
        _transformMatrices.clear();

        for (const entt::entity entity : meshesView)
        {
            const RenderedMeshComponent& renderedMesh = meshesView.get<RenderedMeshComponent>(entity);

            _indirectDrawCommands.emplace_back(renderedMesh.Mesh.GetIndicesCount(),
                                               1,
                                               renderedMesh.Mesh.GetIndicesIndex(),
                                               renderedMesh.Mesh.GetVerticesIndex(),
                                               renderedMesh.Material);
        }
        for (const entt::entity entity : meshesView)
        {
            const TransformComponent& transform = meshesView.get<TransformComponent>(entity);
            _transformMatrices.emplace_back(transform.GetLocalToWorldMatrix());
        }

        MeshManager::BindVao();
        MaterialManager.BindMaterialBuffer();
        _pbrShader.Use();
        Shader::SetUniform(_pbrViewMatrixLocation, glm::inverse(cameraTransform.GetLocalToWorldMatrix()));
        Shader::SetUniform(_pbrProjectionMatrixLocation, camera.ProjectionMatrix);
        Shader::SetUniform(_cameraPositionLocation, cameraTransform.GetWorldPosition());

        BindLightData(entityRegistry, camera, cameraTransform, Scene.GetSkybox());

        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, IndirectDrawCommandsSsbo);
        glBufferData(
                GL_DRAW_INDIRECT_BUFFER,
                static_cast<int64_t>(_indirectDrawCommands.size() * sizeof(DrawElementsIndirectCommand)),
                _indirectDrawCommands.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, TransformsSsbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<int64_t>(_transformMatrices.size() * sizeof(glm::mat4)),
                     _transformMatrices.data(), GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, TransformsSsbo);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, IndirectDrawCommandsSsbo);

        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, _indirectDrawCommands.size(), 0);

        // ------------------------ SKYBOX ------------------------

        _skyboxShader.Use();
        Shader::SetUniform(_skyboxViewMatrixLocation, glm::inverse(cameraTransform.GetLocalToWorldMatrix()));
        Shader::SetUniform(_skyboxProjectionMatrixLocation, camera.ProjectionMatrix);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, Scene.GetSkybox().GetCubeMap());

        CubeRenderer::Draw();
    }

    void Renderer::BindLightData(const entt::registry* EntityRegistry, const CameraComponent& Camera,
                                 const TransformComponent& CameraTransform, const Skybox& Skybox)
    {
        const auto& directionalLightView = EntityRegistry->view<DirectionalLightComponent, TransformComponent>();
        uint32_t directionalLightCount = 0;

        for (const entt::entity entity : directionalLightView)
        {
            const TransformComponent& transform = directionalLightView.get<TransformComponent>(entity);

            const glm::vec3 lightDirection = transform.GetWorldForward();

            glm::vec3 cameraPosition = CameraTransform.GetWorldPosition();
            glm::mat4 lightView = glm::lookAt(cameraPosition + transform.GetWorldForward() * 100.0f,
                                              cameraPosition,
                                              glm::vec3(0.0f, 1.0f, 0.0f));

            glm::mat4 lightSpaceTransform = DirectionalLightProjectionMatrix * lightView;

            const DirectionalLightComponent& light = directionalLightView.get<DirectionalLightComponent>(entity);

            DirectionalLightComponent::ShaderData lightData(light, lightSpaceTransform, lightDirection);
            _lightDataBuffer.DirectionalLight = lightData;

            directionalLightCount++;
            if (directionalLightCount == MaxDirectionalLights)
            {
                break;
            }
        }
        _lightDataBuffer.DirectionalLightCount = directionalLightCount;

        const auto& pointLightsView = EntityRegistry->view<PointLightComponent, TransformComponent>();
        uint32_t pointLightCount = 0;

        for (const entt::entity entity : pointLightsView)
        {
            const TransformComponent& transform = pointLightsView.get<TransformComponent>(entity);
            const glm::vec3 lightPosition = transform.GetWorldPosition();

            const PointLightComponent& light = pointLightsView.get<PointLightComponent>(entity);

            PointLightComponent::ShaderData shaderData(light, lightPosition);
            _lightDataBuffer.PointLights[pointLightCount] = shaderData;
            pointLightCount++;
            if (pointLightCount == MaxPointLights)
            {
                break;
            }
        }
        _lightDataBuffer.PointLightCount = pointLightCount;

        const auto& spotlightsView = EntityRegistry->view<SpotlightComponent, TransformComponent>();
        uint32_t spotlightCount = 0;

        for (const entt::entity entity : spotlightsView)
        {
            const TransformComponent& transform = spotlightsView.get<TransformComponent>(entity);
            const glm::vec3 lightDirection = transform.GetWorldForward();
            const glm::vec3 lightPosition = transform.GetWorldPosition();

            const SpotlightComponent& light = spotlightsView.get<SpotlightComponent>(entity);

            SpotlightComponent::ShaderData shaderData(light, lightPosition, lightDirection);
            _lightDataBuffer.SpotLights[spotlightCount] = shaderData;
            spotlightCount++;
            if (spotlightCount == MaxSpotlights)
            {
                break;
            }
        }
        _lightDataBuffer.SpotlightCount = spotlightCount;

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _lightsSsbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(LightDataBuffer), &_lightDataBuffer, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, _lightsSsbo);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox.GetIrradianceMapMap());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox.GetPrefilterMap());
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, _brdfLut);
    }
}
