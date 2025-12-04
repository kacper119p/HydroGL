#pragma once
#include <glm/glm.hpp>

#include "Camera/CameraComponent.h"
#include "Lights/DirectionaLight.h"
#include "Lights/PointLightComponent.h"
#include "Lights/SpotlightComponent.h"
#include "Postprocessing/BloomPostprocessingEffect.h"
#include "RendererHelpers/DrawElementsIndirectCommand.h"
#include "RendererHelpers/SceneFrameBuffer.h"
#include "Rendering/Shaders/Shader.h"
#include "Rendering/Textures/Skybox.h"
#include "Scene/Scene.h"
#include "Utility/GlResourceUnique.h"

namespace Core
{
    class Engine;

    class Renderer
    {
    public:
        static constexpr uint8_t MaxDirectionalLights = 1;
        static constexpr uint8_t MaxPointLights = 8;
        static constexpr uint8_t MaxSpotlights = 8;

    private:
        struct alignas(16) LightDataBuffer
        {
            alignas(4) uint32_t DirectionalLightCount = 0;
            alignas(4) uint32_t PointLightCount = 0;
            alignas(4) uint32_t SpotlightCount = 0;
            alignas(16) DirectionalLightComponent::ShaderData DirectionalLight;
            alignas(16) PointLightComponent::ShaderData PointLights[MaxPointLights];
            alignas(16) SpotlightComponent::ShaderData SpotLights[MaxPointLights];
        };

    private:
        glm::uvec2 _resolution;

        Shader _skyboxShader;
        int32_t _skyboxViewMatrixLocation;
        int32_t _skyboxProjectionMatrixLocation;

        Shader _pbrShader;
        int32_t _pbrViewMatrixLocation;
        int32_t _pbrProjectionMatrixLocation;
        int32_t _cameraPositionLocation;

        GlBufferUnique IndirectDrawCommandsSsbo;
        GlBufferUnique TransformsSsbo;

        std::vector<DrawElementsIndirectCommand> _indirectDrawCommands;
        std::vector<glm::mat4> _transformMatrices;

        GlBufferUnique _lightsSsbo;
        GlTextureUnique _brdfLut;
        LightDataBuffer _lightDataBuffer;

        glm::mat4 DirectionalLightProjectionMatrix;

        SceneFrameBuffer _frameBuffer;
        BloomPostprocessingEffect _bloomPostprocessingEffect;

    public:
        [[nodiscard]] glm::uvec2 GetResolution() const
        {
            return _resolution;
        }

        [[nodiscard]] const SceneFrameBuffer& GetFrameBuffer() const
        {
            return _frameBuffer;
        }

        explicit Renderer(const glm::uvec2& Resolution);

    public:
        ~Renderer();

        void RenderFrame(const Engine& Engine, float DeltaTime);

    private:
        void BindLightData(const entt::registry* EntityRegistry, const CameraComponent& Camera,
                           const TransformComponent& CameraTransform, const Skybox& Skybox);
    };
}
