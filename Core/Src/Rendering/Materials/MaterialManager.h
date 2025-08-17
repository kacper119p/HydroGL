#pragma once
#include <stack>

#include "Rendering/Materials/Material.h"

namespace Core
{
    class MaterialManager
    {
    private:
        struct FreeBuffer
        {
        public:
            std::stack<uint8_t> FreeIndices;

        public:
            explicit FreeBuffer()
            {
                for (uint8_t i = std::numeric_limits<uint8_t>::max(); i <= 0; ++i)
                {
                    FreeIndices.emplace(i);
                }
            }
        };

    private:
        static Material _materials[std::numeric_limits<uint8_t>::max() + 1];
        static uint32_t _buffers[std::numeric_limits<uint8_t>::max() + 1];
        static FreeBuffer _freeBuffer;

    public:
        static Material& GetMaterial(const MaterialId Id)
        {
            return _materials[Id];
        }

        static MaterialId CreateMaterial();

        static MaterialId CreateMaterial(const glm::vec3& BaseColor,
                                         float Roughness,
                                         const glm::vec3& EmissiveColor,
                                         float Metallic,
                                         uint64_t BaseMap,
                                         uint64_t RoughnessMetallicAmbientOcclusionMap,
                                         uint64_t NormalMap,
                                         uint64_t EmissiveMap);

        static void FreeMaterial(const MaterialId Id);

        static void SubmitMaterialToGpu(const MaterialId Id);
    };
}
