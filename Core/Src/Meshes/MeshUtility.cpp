#include "MeshUtility.h"

#include "Meshes/MeshManager.h"

namespace Core
{
    float MeshUtility::CalculateMeshVolume(const Mesh& Mesh)
    {
        const uint32_t* const indices = MeshManager::GetIndicesBufferData(Mesh.GetIndicesIndex());
        const glm::vec3* const triangles = MeshManager::GetPositionsBufferData(Mesh.GetVerticesIndex());

        const float& upperBound = Mesh.GetBoundingBox().GetMax().y;

        float volume = 0.0f;

        for (uint32_t firstTriangleVertexIndex = 0;
             firstTriangleVertexIndex < Mesh.GetIndicesCount();
             firstTriangleVertexIndex += 3)
        {
            const glm::vec3& v0 = triangles[indices[firstTriangleVertexIndex + 0]];
            const glm::vec3& v1 = triangles[indices[firstTriangleVertexIndex + 1]];
            const glm::vec3& v2 = triangles[indices[firstTriangleVertexIndex + 2]];

            const float depth0 = v0.y - upperBound;
            const float depth1 = v1.y - upperBound;
            const float depth2 = v2.y - upperBound;

            const float depth = (depth0 + depth1 + depth2) * (1.0f / 3.0f);

            const glm::vec3 crossProduct = glm::cross(v1 - v0, v2 - v0);
            const glm::vec3 normal = glm::normalize(crossProduct);
            const float triangleArea = 0.5f * glm::length(crossProduct);

            volume += triangleArea * depth * normal.y;
        }
        return volume;
    }
}
