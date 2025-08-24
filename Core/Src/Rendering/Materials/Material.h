#pragma once
#include "Rendering/Shaders/Shader.h"

namespace Core
{
    using MaterialId = uint32_t;

    struct alignas(16) Material
    {
        glm::vec3 BaseColor;
        float Roughness;
        glm::vec3 EmissiveColor;
        float Metallic;

        uint64_t BaseMap;
        uint64_t RoughnessMetallicAmbientOcclusionMap;
        uint64_t NormalMap;
        uint64_t EmissiveMap;
    };
}
