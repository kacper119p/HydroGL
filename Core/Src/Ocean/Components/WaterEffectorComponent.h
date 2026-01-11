#pragma once
#include <cstdint>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "Collections/CyclicBuffer.h"
#include "Meshes/Mesh.h"

namespace Core
{
    class WaterEffectorComponent
    {
    public:
        Mesh Mesh;
        uint32_t MaskTexture;
        CyclicBuffer<uint32_t, 3> InteractiveWavesTextures;
        CyclicBuffer<uint64_t, 3> InteractiveWavesTextureHandles;
        CyclicBuffer<glm::vec2, 2> PreviousPositions;
        CyclicBuffer<float, 2> PreviousInteractiveWavesFieldSizes;
        float DragCoefficient;
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
                               float DragCoefficient,
                               float DisplacementStrength,
                               float BackWaveHeightFactor,
                               float FrontWaveHeightFactor,
                               int32_t TexturesResolution);

        WaterEffectorComponent(const class Mesh& Mesh,
                               const glm::vec3& Position,
                               float DragCoefficient,
                               float DisplacementStrength,
                               float BackWaveHeightFactor,
                               float FrontWaveHeightFactor,
                               int32_t TexturesResolution);

    public:
        static void Destroy(const WaterEffectorComponent& ToDestroy);
    };
}
