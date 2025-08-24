#pragma once
#include <stack>

#include "Rendering/Materials/Material.h"
#include "Rendering/Textures/Texture.h"

namespace Core
{
    class Texture;

    class MaterialManager
    {
    public:
        static constexpr uint32_t MaxMaterials = std::numeric_limits<uint8_t>::max() + 1;

    private:
        struct FreeBuffer
        {
        public:
            std::stack<MaterialId> FreeIndices;

        public:
            explicit FreeBuffer()
            {
                for (int32_t i = MaxMaterials; i >= 0; --i)
                {
                    FreeIndices.emplace(i);
                }
            }
        };

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
                                  Texture BaseMap,
                                  Texture RoughnessMetallicAmbientOcclusionMap,
                                  Texture NormalMap,
                                  Texture EmissiveMap);

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
