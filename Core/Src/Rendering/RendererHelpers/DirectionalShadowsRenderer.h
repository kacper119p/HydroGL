#pragma once
#include <array>
#include <glm/mat4x4.hpp>

#include "Camera/CameraComponent.h"
#include "Rendering/Lights/DirectionaLight.h"
#include "Rendering/Shaders/Shader.h"
#include "Utility/GlResourceUnique.h"

namespace Core
{
    class DirectionalShadowsRenderer
    {
    private:
        static constexpr uint32_t CascadeCount = 4;
        static constexpr uint32_t ShadowMapResolution = 1024;

    public:
        struct DirectionalLightSamplingData
        {
            alignas(16) glm::mat4 ViewMatrix;
            alignas(16) glm::mat4 CascadeViewProjMatrices[CascadeCount];
            alignas(16) float CascadeSplits[CascadeCount];
        };

    private:
        DirectionalLightSamplingData _directionalLightSamplingData;

        GlFramebufferUnique _framebuffer;
        GlTextureUnique _shadowMaps;
        GlBufferUnique _shadowPassUbo;

        Shader _shadowPassShader;

    public:
        [[nodiscard]] DirectionalLightSamplingData GetDirectionalLightSamplingData() const
        {
            return _directionalLightSamplingData;
        }

        [[nodiscard]] uint32_t GetShadowMaps() const
        {
            return _shadowMaps.Get();
        }

        DirectionalShadowsRenderer();

    public:
        [[nodiscard]] uint32_t RenderShadows(const CameraComponent& Camera, const glm::mat4& ViewMatrix,
                                             const glm::vec3& LightDirection, uint32_t TransformsBuffer,
                                             uint32_t IndirectDrawCommandsBuffer, uint32_t ObjectCount);

    private:
        void UpdateShadowCascadeMatrices(const CameraComponent& Camera, const glm::mat4& ViewMatrix,
                                         const glm::vec3& LightDirection);

        static void GetFrustumCornersWorldSpace(std::array<glm::vec3, 8>& Result, const glm::mat4& ProjectionMatrix,
                                                const glm::mat4& ViewMatrix,
                                                float NearCameraPlane, float FarCameraPlane);

    private:
        static GlBufferUnique CreateShadowPassUbo();

        void InitializeShadowMapsFramebuffer();
    };
}
