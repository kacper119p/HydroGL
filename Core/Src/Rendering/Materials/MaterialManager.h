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

    public:
        static constexpr uint32_t MaxMaterials = std::numeric_limits<uint8_t>::max() + 1;

    private:
        Material _materials[MaxMaterials]{};
        uint32_t _buffers[MaxMaterials]{};
        FreeBuffer _freeBuffer{};

        uint32_t _materialSsbo = 0;

    public:
        MaterialManager();

        ~MaterialManager();

        Material& GetMaterial(const MaterialId Id)
        {
            return _materials[Id];
        }

        MaterialId CreateMaterial();

        MaterialId CreateMaterial(const glm::vec3& BaseColor,
                                  float Roughness,
                                  const glm::vec3& EmissiveColor,
                                  float Metallic,
                                  uint64_t BaseMap,
                                  uint64_t RoughnessMetallicAmbientOcclusionMap,
                                  uint64_t NormalMap,
                                  uint64_t EmissiveMap);

        void FreeMaterial(MaterialId Id);

        void SubmitMaterialToGpu(MaterialId Id) const;

        void BindMaterialBuffer() const;
    };
}
