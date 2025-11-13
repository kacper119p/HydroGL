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
        float WaveDampingMin;
        float WaveDampingMax;
        float DampingMaxVelocity;
        uint32_t MaskTextures[2];
        uint32_t InteractiveWavesTextures[3];
        uint64_t InteractiveWavesTextureHandles[3];
        glm::vec2 PreviousPositions[2];
        float InteractiveWavesFieldSize;
        float IntersectionMaskFieldSize;
        int32_t TexturesResolution;

    public:
        WaterEffectorComponent(const class Mesh& Mesh,
                               const glm::vec3& Position,
                               float WaveDampingMin,
                               float WaveDampingMax,
                               float DampingMaxVelocity,
                               int32_t TexturesResolution,
                               float InteractiveWavesFieldSize,
                               float IntersectionMaskPhysicalSize);

    public:
        static void Destroy(const WaterEffectorComponent& ToDestroy);
    };
}
