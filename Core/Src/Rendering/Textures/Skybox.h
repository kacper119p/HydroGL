#pragma once
#include <string_view>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>

#include "Utility/GlResourceUnique.h"

namespace Core
{
    struct Skybox
    {
    private:
        uint32_t _cubeMap{};
        uint32_t _irradianceMap{};
        uint32_t _prefilterMap{};
        glm::vec3 _environmentColor{};

    public:
        [[nodiscard]] static Skybox CreateSkybox(std::string_view SkyboxTexturePath, std::string_view IblTexturePath);

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

        [[nodiscard]] const glm::vec3& GetEnvironmentColor() const
        {
            return _environmentColor;
        }
    };
}
