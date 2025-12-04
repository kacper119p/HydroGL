#pragma once
#include <array>
#include <cstdint>
#include <glm/mat4x4.hpp>

namespace Core::CubeMapGenerator
{
    uint32_t IrradianceMapFromSkybox(uint32_t Skybox);

    uint32_t PrefilterMapFromSkybox(uint32_t Skybox, uint32_t SkyboxResolution);

    uint32_t CreateBrdfLut();

    glm::vec3 GetEnvironmentColorFromIrradianceMap(uint32_t IrradianceMap);
}
