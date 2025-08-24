#include "MaterialManager.h"

namespace Core
{
    MaterialManager::MaterialManager()
    {
        glGenBuffers(1, &_materialSsbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _materialSsbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Material) * MaxMaterials, nullptr, GL_STATIC_COPY);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _materialSsbo);
    }

    MaterialManager::~MaterialManager()
    {
        glDeleteBuffers(1, &_materialSsbo);
    }

    MaterialId MaterialManager::CreateMaterial()
    {
        const MaterialId id = _freeBuffer.FreeIndices.top();
        _freeBuffer.FreeIndices.pop();

        return id;
    }

    MaterialId MaterialManager::CreateMaterial(const glm::vec3& BaseColor,
                                               const float Roughness,
                                               const glm::vec3& EmissiveColor,
                                               const float Metallic,
                                               const Texture BaseMap,
                                               const Texture RoughnessMetallicAmbientOcclusionMap,
                                               const Texture NormalMap,
                                               const Texture EmissiveMap)
    {
        return CreateMaterial(BaseColor,
                              Roughness,
                              EmissiveColor,
                              Metallic,
                              BaseMap.GetHandleReadonly(),
                              RoughnessMetallicAmbientOcclusionMap.GetHandleReadonly(),
                              NormalMap.GetHandleReadonly(),
                              EmissiveMap.GetHandleReadonly());
    }

    MaterialId MaterialManager::CreateMaterial(const glm::vec3& BaseColor,
                                               const float Roughness,
                                               const glm::vec3& EmissiveColor,
                                               const float Metallic,
                                               const uint64_t BaseMap,
                                               const uint64_t RoughnessMetallicAmbientOcclusionMap,
                                               const uint64_t NormalMap,
                                               const uint64_t EmissiveMap)
    {
        const MaterialId id = CreateMaterial();
        Material& material = _materials[id];
        material.BaseColor = BaseColor;
        material.Roughness = Roughness;
        material.EmissiveColor = EmissiveColor;
        material.Metallic = Metallic;
        material.BaseMap = BaseMap;
        material.RoughnessMetallicAmbientOcclusionMap = RoughnessMetallicAmbientOcclusionMap;
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

    void MaterialManager::SubmitMaterialToGpu(const MaterialId Id) const
    {

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _materialSsbo);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, Id * static_cast<int64_t>(sizeof(Material)), sizeof(Material),
                        _materials + Id);
    }

    void MaterialManager::BindMaterialBuffer() const
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _materialSsbo);
    }
}
