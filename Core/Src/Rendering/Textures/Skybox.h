#pragma once
#include <array>
#include <string_view>
#include <glm/fwd.hpp>

namespace Core
{
    class Skybox
    {
    private:
        uint32_t _cubeMap = 0;
        uint32_t _irradianceMap = 0;
        uint32_t _prefilterMap = 0;

    public:
        [[nodiscard]] static Skybox CreateSkybox(const std::string_view& FilePath);

        static void DestroySkybox(const Skybox& Skybox);

    private:
        static uint32_t IrradianceMapFromSkybox(uint32_t Skybox);

        static uint32_t PrefilterMapFromSkybox(uint32_t Skybox, uint32_t SkyboxResolution);

        static void InitializeFbo(uint32_t& Fbo, uint32_t& Rbo, uint32_t Resolution);

        static void InitializeCubeMap(uint32_t& CubeMap, uint32_t Resolution);

        static void InitializeViewPort(uint32_t Resolution);

        static void GetCaptureProjection(glm::mat4& ProjectionMatrix);

        static void GetCaptureViews(std::array<glm::mat4, 6>& ViewMatrices);

    public:
        [[nodiscard]] uint32_t GetCubeMap() const
        {
            return _cubeMap;
        }

        [[nodiscard]] uint32_t GetIrradianceMapMap() const
        {
            return _irradianceMap;
        }

        [[nodiscard]] uint32_t GetPrefilterMap() const
        {
            return _prefilterMap;
        }
    };
}
