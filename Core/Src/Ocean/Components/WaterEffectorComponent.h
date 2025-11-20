#pragma once
#include <cstdint>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "Meshes/Mesh.h"

namespace Core
{
    class WaterEffectorComponent
    {
    public:
        Mesh Mesh;
        uint32_t MaskTexture;
        uint32_t InteractiveWavesTextures[3];
        uint64_t InteractiveWavesTextureHandles[3];
        glm::vec2 PreviousPositions[2];
        float InteractiveWavesFieldSize;
        float IntersectionMaskFieldSize;
        float MeshTotalVolume;
        float BackWaveHeightFactor;
        float FrontWaveHeightFactor;
        float DisplacementStrength;
        int32_t TexturesResolution;

    public:
        WaterEffectorComponent(const class Mesh& Mesh,
                               const glm::vec3& Position,
                               float MeshTotalVolume,
                               float DisplacementStrength,
                               float BackWaveHeightFactor,
                               float FrontWaveHeightFactor,
                               int32_t TexturesResolution,
                               float InteractiveWavesFieldSize);

        WaterEffectorComponent(const class Mesh& Mesh,
                               const glm::vec3& Position,
                               float DisplacementStrength,
                               float BackWaveHeightFactor,
                               float FrontWaveHeightFactor,
                               int32_t TexturesResolution,
                               float InteractiveWavesFieldSize);

    public:
        static void Destroy(const WaterEffectorComponent& ToDestroy);
    };
}
