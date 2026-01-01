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

namespace
{
    constexpr int32_t DefaultBufferSize = 1024;
    constexpr float DefaultFogDensity = 0.001f;
}

namespace Core
{
    Renderer::Renderer(const glm::uvec2& Resolution)
        : _resolution(Resolution), _skyboxShader(ShaderBuilder().
              SetStage("Assets/Shaders/Skybox/Skybox.vert", ShaderStage::VertexShader)
              .SetStage("Assets/Shaders/Skybox/Skybox.frag", ShaderStage::FragmentShader)
              .Build()),
          _skyboxViewMatrixLocation(_skyboxShader.GetUniformLocation("ViewMatrix")),
          _skyboxProjectionMatrixLocation(_skyboxShader.GetUniformLocation("ProjectionMatrix")),
          _pbrShader(ShaderBuilder()
              .SetStage("Assets/Shaders/PBR/PBR.vert", ShaderStage::VertexShader)
              .SetStage("Assets/Shaders/PBR/PBR.frag", ShaderStage::FragmentShader)
              .Build()),
          _pbrViewMatrixLocation(_pbrShader.GetUniformLocation("ViewMatrix")),
          _pbrProjectionMatrixLocation(_pbrShader.GetUniformLocation("ProjectionMatrix")),
          _cameraPositionLocation(_pbrShader.GetUniformLocation("CameraPosition")),
          _depthPassShader(
              ShaderBuilder()
              .SetStage("Assets/Shaders/DefaultDepth/DefaultDepth.vert", ShaderStage::VertexShader)
              .SetStage("Assets/Shaders/DefaultDepth/DefaultDepth.frag", ShaderStage::FragmentShader)
              .Build()),
          _depthPassViewMatrixLocation(_depthPassShader.GetUniformLocation("ViewMatrix")),
          _depthPassProjectionMatrixLocation(_depthPassShader.GetUniformLocation("ProjectionMatrix")),
          _indirectDrawCommandsSsbo(GlBufferUnique::Create()),
          _transformsSsbo(GlBufferUnique::Create()),
          _lightsSsbo(GlBufferUnique::Create()),
          _brdfLut(CubeMapGenerator::CreateBrdfLut()),
          _directionalLightProjectionMatrix(glm::ortho(-150.0f, 150.0f, -150.0f, 150.0f, 0.1f, 300.0f)),
          _frameBuffer(Resolution),
          _bloomPostprocessingEffect(Resolution)
    {
        _indirectDrawCommands.resize(DefaultBufferSize);
        _transformMatrices.resize(DefaultBufferSize);
        glNamedBufferData(_indirectDrawCommandsSsbo,
                          static_cast<GLsizeiptr>(DefaultBufferSize * sizeof(DrawElementsIndirectCommand)),
                          _indirectDrawCommands.data(), GL_DYNAMIC_DRAW);
        glNamedBufferData(_transformsSsbo, static_cast<GLsizeiptr>(DefaultBufferSize * sizeof(glm::mat4)),
                          _transformMatrices.data(), GL_DYNAMIC_DRAW);

        _lightDataBuffer.FogDensity = DefaultFogDensity;
    }

    Renderer::~Renderer() = default;

    void Renderer::RenderFrame(const Engine& Engine, const float DeltaTime)
    {
        const Scene& scene = *Engine.GetScene();
        const entt::registry* entityRegistry = scene.GetEntityRegistry();
        const MaterialManager& materialManager = *Engine.GetMaterialManager();

        const auto cameraView = entityRegistry->view<CameraComponent, TransformComponent>();
        const entt::entity cameraEntity = cameraView.front();

        CHECK(entityRegistry->valid(cameraEntity), "Camera entity doesn't exist!");

        const CameraComponent& camera = cameraView.get<CameraComponent>(cameraEntity);
        const TransformComponent& cameraTransform = cameraView.get<TransformComponent>(cameraEntity);
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
            glNamedBufferData(_indirectDrawCommandsSsbo,
                              static_cast<GLsizeiptr>(countHint * sizeof(DrawElementsIndirectCommand)),
                              _indirectDrawCommands.data(), GL_DYNAMIC_DRAW);
            glNamedBufferData(_transformsSsbo, static_cast<GLsizeiptr>(countHint * sizeof(glm::mat4)),
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

        glNamedBufferSubData(_indirectDrawCommandsSsbo, 0,
                             static_cast<GLsizeiptr>(trueCount * sizeof(DrawElementsIndirectCommand)),
                             _indirectDrawCommands.data());
        glNamedBufferSubData(_transformsSsbo, 0, static_cast<GLsizeiptr>(trueCount * sizeof(glm::mat4)),
                             _transformMatrices.data());

        MeshManager::BindVao();

        // ------------------------ DIRECTIONAL LIGHT SHADOWS ------------------------

        const glm::mat4 viewMatrix = glm::inverse(cameraTransform.GetLocalToWorldMatrix());

        RenderShadows(entityRegistry, camera, viewMatrix, _transformsSsbo, _indirectDrawCommandsSsbo, trueCount);

        // ------------------------ MAIN RENDERING PASSES SETUP ------------------------
        materialManager.BindMaterialBuffer();

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

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _transformsSsbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _indirectDrawCommandsSsbo);

        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, _indirectDrawCommandsSsbo);

        // ------------------------ DEPTH PREPASS ------------------------

        _depthPassShader.Use();
        _depthPassShader.SetUniform(_depthPassViewMatrixLocation, viewMatrix);
        _depthPassShader.SetUniform(_depthPassProjectionMatrixLocation, camera.GetProjectionMatrix());

        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(trueCount), 0);

        // ------------------------ MAIN PASS ------------------------

        BindLightData(entityRegistry, viewMatrix, scene.GetSkybox(), camera);

        _pbrShader.Use();
        _pbrShader.SetUniform(_pbrViewMatrixLocation, viewMatrix);
        _pbrShader.SetUniform(_pbrProjectionMatrixLocation, camera.GetProjectionMatrix());
        _pbrShader.SetUniform(_cameraPositionLocation, cameraTransform.GetWorldPosition());

        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(trueCount), 0);

        // ------------------------ SKYBOX ------------------------

        _skyboxShader.Use();
        _skyboxShader.SetUniform(_skyboxViewMatrixLocation, glm::inverse(cameraTransform.GetLocalToWorldMatrix()));
        _skyboxShader.SetUniform(_skyboxProjectionMatrixLocation, camera.GetProjectionMatrix());

        glBindTextureUnit(3, scene.GetSkybox().GetCubeMap());

        CubeRenderer::Draw();

        _frameBuffer.ResolveMultisampling();
        //_frameBuffer.BindOpaqueFramebuffer();

        _frameBuffer.BlitOpaqueToTransparent();
        _frameBuffer.BindTransparentFramebuffer();

        Engine.GetWaterSystem()->Render(DeltaTime,
                                        cameraTransform,
                                        camera,
                                        scene.GetSkybox().GetEnvironmentColor(),
                                        Engine.GetWaterInteractionSystem()->GetDynamicWavesSampleBufferId(),
                                        _frameBuffer.GetOpaqueColorBuffer(),
                                        _frameBuffer.GetOpaqueDepthBuffer());

        Engine.GetWaterSystem()->RenderSpray(_frameBuffer.GetTransparentFrameBuffer());

        _bloomPostprocessingEffect.Render(_frameBuffer.GetTransparentColorBuffer());
    }

    void Renderer::BindLightData(const entt::registry* EntityRegistry, const glm::mat4& ViewMatrix,
                                 const Skybox& Skybox,
                                 const CameraComponent& Camera)
    {
        _lightDataBuffer.SceneAmbientColor = Skybox.GetEnvironmentColor();
        _lightDataBuffer.CameraNearPlane = Camera.GetNearPlane();
        _lightDataBuffer.CameraFarPlane = Camera.GetFarPlane();

        if (const auto& directionalLightView = EntityRegistry->view<DirectionalLightComponent, TransformComponent>();
            directionalLightView.begin() != directionalLightView.end())
        {
            entt::entity entity = *directionalLightView.begin();
            const DirectionalLightComponent& light = directionalLightView.get<DirectionalLightComponent>(entity);
            const TransformComponent& transform = directionalLightView.get<TransformComponent>(entity);
            const glm::vec3 lightDirection = transform.GetWorldForward();
            const DirectionalShadowsRenderer::DirectionalLightSamplingData& lightSamplingData =
                    _directionalShadowsRenderer.GetDirectionalLightSamplingData();

            DirectionalLightComponent::ShaderData lightData(light,
                                                            ViewMatrix,
                                                            lightSamplingData.CascadeViewProjMatrices,
                                                            lightSamplingData.CascadeSplits,
                                                            lightDirection);
            _lightDataBuffer.DirectionalLight = lightData;

            _lightDataBuffer.DirectionalLightCount = 1;
        } else
        {
            _lightDataBuffer.DirectionalLightCount = 0;
        }


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

        glBindTextureUnit(17, Skybox.GetIrradianceMapMap());
        glBindTextureUnit(18, Skybox.GetPrefilterMap());
        glBindTextureUnit(19, _brdfLut);
        glBindTextureUnit(20, _directionalShadowsRenderer.GetShadowMaps());
    }

    void Renderer::RenderShadows(const entt::registry* EntityRegistry, const CameraComponent& Camera,
                                 const glm::mat4& ViewMatrix, const uint32_t TransformsBuffer,
                                 const uint32_t IndirectDrawCommandsBuffer,
                                 const uint32_t ObjectCount)
    {
        const auto& directionalLightView = EntityRegistry->view<DirectionalLightComponent, TransformComponent>();

        if (const auto iterator = directionalLightView.begin(); iterator != directionalLightView.end())
        {
            const entt::entity entity = *iterator;
            const TransformComponent& transform = directionalLightView.get<TransformComponent>(entity);

            _directionalShadowsRenderer.RenderShadows(Camera, ViewMatrix, transform.GetWorldForward(),
                                                      TransformsBuffer, IndirectDrawCommandsBuffer, ObjectCount);
        }
    }
}
