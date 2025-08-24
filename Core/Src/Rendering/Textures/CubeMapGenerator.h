#pragma once
#include <array>
#include <cstdint>
#include <glm/mat4x4.hpp>

namespace Core
{
    class CubeMapGenerator
    {
    private:
        static std::array<glm::mat4, 6> _captureViews;

    private:
        CubeMapGenerator() = default;

    public:
        CubeMapGenerator(const CubeMapGenerator&) = delete;

        CubeMapGenerator& operator=(const CubeMapGenerator&) = delete;

    public:
        static uint32_t IrradianceMapFromSkybox(uint32_t Skybox);

        static uint32_t PrefilterMapFromSkybox(uint32_t Skybox, uint32_t SkyboxResolution);

        static uint32_t CreateBrdfLut();

    private:
        static void InitializeFbo(uint32_t& Fbo);

        static void InitializeCubeMap(uint32_t& CubeMap, uint32_t Resolution);

        static void InitializeViewPort(uint32_t Resolution);

        static void GetCaptureProjection(glm::mat4& ProjectionMatrix);
    };
}
