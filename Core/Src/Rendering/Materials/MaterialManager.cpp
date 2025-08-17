#include "MaterialManager.h"

namespace Core
{
    Material MaterialManager::_materials[std::numeric_limits<uint8_t>::max() + 1]{};
    uint32_t MaterialManager::_buffers[std::numeric_limits<uint8_t>::max() + 1]{};
    MaterialManager::FreeBuffer MaterialManager::_freeBuffer;

    MaterialId MaterialManager::CreateMaterial()
    {
        MaterialId id = _freeBuffer.FreeIndices.top();
        _freeBuffer.FreeIndices.pop();

        glGenBuffers(1, &id);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Material), nullptr, GL_STATIC_DRAW);

        return id;
    }

    MaterialId MaterialManager::CreateMaterial(const glm::vec3& BaseColor, const float Roughness,
                                               const glm::vec3& EmissiveColor, const float Metallic,
                                               const uint64_t BaseMap,
                                               const uint64_t RoughnessMetallicAmbientOcclusionMap,
                                               const uint64_t NormalMap, const uint64_t EmissiveMap)
    {
        const MaterialId id = CreateMaterial();
        Material& material = _materials[id];
        material.BaseColor = BaseColor;
        material.Roughness = Roughness;
        material.EmissiveColor = EmissiveColor;
        material.BaseMap = BaseMap;
        material.RoughnessMetallicAmbientOcclusionMap = RoughnessMetallicAmbientOcclusionMap;
        material.Metallic = Metallic;
        material.NormalMap = NormalMap;
        material.EmissiveMap = EmissiveMap;

        SubmitMaterialToGpu(id);
        return id;
    }

    void MaterialManager::FreeMaterial(const MaterialId Id)
    {
        glDeleteBuffers(1, &_buffers[Id]);
        _freeBuffer.FreeIndices.push(Id);
    }

    void MaterialManager::SubmitMaterialToGpu(const MaterialId Id)
    {
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Material), _materials + Id, GL_STATIC_DRAW);
    }
}
