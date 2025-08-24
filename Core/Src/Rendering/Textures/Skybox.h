#pragma once
#include <array>
#include <string_view>
#include <glm/fwd.hpp>

namespace Core
{
    struct Skybox
    {
    private:
        uint32_t _cubeMap = 0;
        uint32_t _irradianceMap = 0;
        uint32_t _prefilterMap = 0;

    public:
        [[nodiscard]] static Skybox CreateSkybox(const std::string_view& FilePath);

        static void DestroySkybox(const Skybox& Skybox);

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
