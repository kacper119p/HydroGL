#include "Renderer.h"

#include "Debug.h"
#include "Engine.h"
#include "Camera/CameraComponent.h"
#include "Components/RenderedMeshComponent.h"
#include "Meshes/MeshManager.h"
#include "Rendering/Materials/MaterialManager.h"
#include "Rendering/RendererHelpers/CubeRenderer.h"
#include "Rendering/Shaders/ShaderBuilder.h"
#include "Scene/TransformComponent.h"
#include "Textures/CubeMapGenerator.h"

namespace Core
{
    Renderer::Renderer(const glm::uvec2& Resolution) : _resolution(Resolution), _skyboxShader(ShaderBuilder().
                                                           SetStage("Assets/Shaders/Skybox/Skybox.vert",
                                                                    ShaderStage::VertexShader)
                                                           .SetStage("Assets/Shaders/Skybox/Skybox.frag",
                                                                     ShaderStage::FragmentShader)
                                                           .Build()),
                                                       _skyboxViewMatrixLocation(
                                                           _skyboxShader.GetUniformLocation("ViewMatrix")),
                                                       _skyboxProjectionMatrixLocation(
                                                           _skyboxShader.GetUniformLocation("ProjectionMatrix")),
                                                       _pbrShader(ShaderBuilder().SetStage(
                                                               "Assets/Shaders/PBR/PBR.vert",
                                                               Core::ShaderStage::VertexShader)
                                                           .SetStage("Assets/Shaders/PBR/PBR.frag",
                                                                     Core::ShaderStage::FragmentShader)
                                                           .Build()),
                                                       _pbrViewMatrixLocation(
                                                           _pbrShader.GetUniformLocation("ViewMatrix")),
                                                       _pbrProjectionMatrixLocation(
                                                           _pbrShader.GetUniformLocation("ProjectionMatrix")),
                                                       _cameraPositionLocation(
                                                           _pbrShader.GetUniformLocation("CameraPosition")),
                                                       _brdfLut(CubeMapGenerator::CreateBrdfLut()),
                                                       DirectionalLightProjectionMatrix(
                                                           glm::ortho(-150.0f, 150.0f, -150.0f, 150.0f, 0.1f, 300.0f)),
                                                       _frameBuffer(Resolution),
                                                       _bloomPostprocessingEffect(Resolution)
    {
        constexpr int32_t defaultBufferSize = 1024;
        IndirectDrawCommandsSsbo = GlBufferUnique::Create();
        TransformsSsbo = GlBufferUnique::Create();
        _lightsSsbo = GlBufferUnique::Create();
        _indirectDrawCommands.resize(defaultBufferSize);
        _transformMatrices.resize(defaultBufferSize);
        glNamedBufferData(IndirectDrawCommandsSsbo,
                          static_cast<GLsizeiptr>(defaultBufferSize * sizeof(DrawElementsIndirectCommand)),
                          _indirectDrawCommands.data(), GL_DYNAMIC_DRAW);
        glNamedBufferData(TransformsSsbo, static_cast<GLsizeiptr>(defaultBufferSize * sizeof(glm::mat4)),
                          _transformMatrices.data(), GL_DYNAMIC_DRAW);
    }

    Renderer::~Renderer() = default;

    void Renderer::RenderFrame(const Engine& Engine, const float DeltaTime)
    {
        const Scene& scene = *Engine.GetScene();
        const entt::registry* entityRegistry = scene.GetEntityRegistry();
        const MaterialManager& materialManager = *Engine.GetMaterialManager();

        const auto cameraView = entityRegistry->view<CameraComponent, TransformComponent>();
        const entt::entity cameraEntity = cameraView.front();

        CHECK_MESSAGE(entityRegistry->valid(cameraEntity), "Camera entity doesn't exist!");

        const CameraComponent& camera = cameraView.get<CameraComponent>(cameraEntity);
        const TransformComponent& cameraTransform = cameraView.get<TransformComponent>(cameraEntity);
        glDepthMask(GL_TRUE);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        glViewport(0, 0, static_cast<int32_t>(_resolution.x), static_cast<int32_t>(_resolution.y));
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glCullFace(GL_BACK);
        _frameBuffer.BindMultiSampledFramebuffer();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ------------------------ MESHES ------------------------

        MeshManager::BindVao();

        materialManager.BindMaterialBuffer();
        const auto meshesView = entityRegistry->view<RenderedMeshComponent, TransformComponent>();
        const auto meshesViewComponents = meshesView.each();

        const size_t countHint = meshesView.size_hint();

        if (countHint > _indirectDrawCommands.capacity())
        {
            _indirectDrawCommands.resize(countHint);
            _transformMatrices.resize(countHint);
            glNamedBufferData(IndirectDrawCommandsSsbo,
                              static_cast<GLsizeiptr>(countHint * sizeof(DrawElementsIndirectCommand)),
                              _indirectDrawCommands.data(), GL_DYNAMIC_DRAW);
            glNamedBufferData(TransformsSsbo, static_cast<GLsizeiptr>(countHint * sizeof(glm::mat4)),
                              _transformMatrices.data(), GL_DYNAMIC_DRAW);
        }

        size_t trueCount = 0;

        for (const auto& [entity, renderedMesh, transform] : meshesViewComponents)
        {
            _transformMatrices[trueCount++] = transform.GetLocalToWorldMatrix();
        }

        size_t index = 0;
        for (const auto& [entity, renderedMesh, transform] : meshesViewComponents)
        {
            _indirectDrawCommands[index++] = {
                renderedMesh.Mesh.GetIndicesCount(),
                1,
                renderedMesh.Mesh.GetIndicesIndex(),
                static_cast<int32_t>(renderedMesh.Mesh.GetVerticesIndex()),
                renderedMesh.Material
            };
        }

        MeshManager::BindVao();
        materialManager.BindMaterialBuffer();
        _pbrShader.Use();
        _pbrShader.SetUniform(_pbrViewMatrixLocation, glm::inverse(cameraTransform.GetLocalToWorldMatrix()));
        _pbrShader.SetUniform(_pbrProjectionMatrixLocation, camera.ProjectionMatrix);
        _pbrShader.SetUniform(_cameraPositionLocation, cameraTransform.GetWorldPosition());

        BindLightData(entityRegistry, camera, cameraTransform, scene.GetSkybox());

        glNamedBufferSubData(IndirectDrawCommandsSsbo, 0,
                             static_cast<GLsizeiptr>(trueCount * sizeof(DrawElementsIndirectCommand)),
                             _indirectDrawCommands.data());
        glNamedBufferSubData(TransformsSsbo, 0, static_cast<GLsizeiptr>(trueCount * sizeof(glm::mat4)),
                             _transformMatrices.data());

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, TransformsSsbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, IndirectDrawCommandsSsbo);

        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, IndirectDrawCommandsSsbo);
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(trueCount), 0);

        _frameBuffer.ResolveMultisampling();
        _frameBuffer.BindOpaqueFramebuffer();

        // ------------------------ SKYBOX ------------------------

        _skyboxShader.Use();
        _skyboxShader.SetUniform(_skyboxViewMatrixLocation, glm::inverse(cameraTransform.GetLocalToWorldMatrix()));
        _skyboxShader.SetUniform(_skyboxProjectionMatrixLocation, camera.ProjectionMatrix);

        glBindTextureUnit(3, scene.GetSkybox().GetCubeMap());

        CubeRenderer::Draw();

        _frameBuffer.BlitOpaqueToTransparent();
        _frameBuffer.BindTransparentFramebuffer();

        Engine.GetWaterSystem()->Render(DeltaTime,
                                        cameraTransform,
                                        camera,
                                        scene.GetSkybox().GetEnvironmentColor(),
                                        Engine.GetWaterInteractionSystem()->GetDynamicWavesSampleBufferId(),
                                        _frameBuffer.GetOpaqueColorBuffer(),
                                        _frameBuffer.GetResolvedDepthBuffer());

        Engine.GetWaterSystem()->RenderSpray(_frameBuffer.GetTransparentFrameBuffer());

        _bloomPostprocessingEffect.Render(_frameBuffer.GetTransparentColorBuffer());
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

        glNamedBufferData(_lightsSsbo, sizeof(LightDataBuffer), &_lightDataBuffer, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, _lightsSsbo);

        glBindTextureUnit(0, Skybox.GetIrradianceMapMap());
        glBindTextureUnit(1, Skybox.GetPrefilterMap());
        glBindTextureUnit(2, _brdfLut);
    }
}
