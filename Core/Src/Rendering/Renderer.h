#pragma once
#include <glm/glm.hpp>

#include "Camera/CameraComponent.h"
#include "Lights/DirectionaLight.h"
#include "Lights/PointLightComponent.h"
#include "Lights/SpotlightComponent.h"
#include "Materials/MaterialManager.h"
#include "Postprocessing/BloomPostprocessingEffect.h"
#include "RendererHelpers/DrawElementsIndirectCommand.h"
#include "RendererHelpers/SceneFrameBuffer.h"
#include "Rendering/Shaders/Shader.h"
#include "Rendering/Textures/Skybox.h"
#include "Scene/Scene.h"

namespace Core
{
    class Renderer
    {
    public:
        static constexpr uint8_t MaxDirectionalLights = 1;
        static constexpr uint8_t MaxPointLights = 8;
        static constexpr uint8_t MaxSpotlights = 8;

    private:
        struct alignas(16) LightDataBuffer
        {
            uint32_t DirectionalLightCount = 0;
            uint32_t PointLightCount = 0;
            uint32_t SpotlightCount = 0;
            float _padding0{};
            DirectionalLightComponent::ShaderData DirectionalLight;
            PointLightComponent::ShaderData PointLights[MaxPointLights];
            SpotlightComponent::ShaderData SpotLights[MaxPointLights];
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

        uint32_t IndirectDrawCommandsSsbo = 0;
        uint32_t TransformsSsbo = 0;

        std::vector<DrawElementsIndirectCommand> _indirectDrawCommands;
        std::vector<glm::mat4> _transformMatrices;

        uint32_t _lightsSsbo = 0;
        uint32_t _brdfLut = 0;
        LightDataBuffer _lightDataBuffer;

        glm::mat4 DirectionalLightProjectionMatrix;

        SceneFrameBuffer _frameBuffer;
        BloomPostprocessingEffect _bloomPostprocessingEffect;

    public:
        explicit Renderer(const glm::uvec2& Resolution);

    public:
        ~Renderer();

        void RenderFrame(const Scene& Scene, const MaterialManager& MaterialManager);

    private:
        void BindLightData(const entt::registry* EntityRegistry, const CameraComponent& Camera,
                           const TransformComponent& CameraTransform, const Skybox& Skybox);
    };
}
