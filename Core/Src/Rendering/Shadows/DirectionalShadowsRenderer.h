#pragma once
#include <array>
#include <glm/mat4x4.hpp>

#include "Camera/CameraComponent.h"
#include "Rendering/Lights/DirectionaLight.h"
#include "Rendering/Shaders/Shader.h"
#include "Utility/GlResourceUnique.h"

namespace Core
{
    /**
     * @brief Manages the data and generation of Cascaded Shadow Maps for directional lights.
     */
    class DirectionalShadowsRenderer
    {
    private:
        static constexpr uint32_t CascadeCount = 4;
        static constexpr uint32_t ShadowMapResolution = 2048;

    public:
        /**
         * @brief Data structure representing the GPU layout for directional light shadow sampling data.
         * Aligned to 16 bytes to match std140 layout requirements in shaders.
         */
        struct DirectionalLightSamplingData
        {
            /** @brief The view matrix of the current rendering camera. */
            alignas(16) glm::mat4 ViewMatrix;
            /** @brief Light space view-projection matrices for each shadow cascade. */
            alignas(16) glm::mat4 CascadeViewProjMatrices[CascadeCount];
            /** @brief Split depths (in view space) defining the boundaries of each cascade. */
            alignas(16) float CascadeSplits[CascadeCount];
        };

    private:
        DirectionalLightSamplingData _directionalLightSamplingData;

        GlFramebufferUnique _framebuffer;
        GlTextureUnique _shadowMaps;
        GlBufferUnique _shadowPassUbo;

        Shader _shadowPassShader;

    public:
        /**
         * @brief Retrieves the current sampling data used for shadow calculations.
         */
        [[nodiscard]] const DirectionalLightSamplingData& GetDirectionalLightSamplingData() const
        {
            return _directionalLightSamplingData;
        }

        /**
         * @brief Retrieves the id of the texture array containing the shadow maps.
         * @return Texture Array ID.
         */
        [[nodiscard]] uint32_t GetShadowMaps() const
        {
            return _shadowMaps.Get();
        }

        /**
         * @brief Constructs the renderer, initializing resources, framebuffers, and shaders.
         */
        DirectionalShadowsRenderer();

    public:
        /**
         * @brief Updates sampling data and renders the scene geometry into the shadow map cascades.
         *
         * @param Camera Camera that scene is rendered to.
         * @param ViewMatrix The view matrix of the camera.
         * @param LightDirection The normalized direction vector of the directional light.
         * @param TransformsBuffer The SSBO handle containing object model matrices.
         * @param IndirectDrawCommandsBuffer The buffer handle containing indirect draw commands.
         * @param ObjectCount The total number of objects in IndirectDrawCommandsBuffer.
         */
        void RenderShadows(const CameraComponent& Camera, const glm::mat4& ViewMatrix,
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
