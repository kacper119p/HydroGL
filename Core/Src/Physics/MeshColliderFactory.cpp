#include "MeshColliderFactory.h"

#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include "Meshes/MeshManager.h"

namespace Core
{
    JPH::RefConst<JPH::Shape> MeshColliderFactory::CreateConvexHullCollider(const Mesh& Mesh)
    {
        JPH::Array<JPH::Vec3> joltVertices;
        joltVertices.reserve(Mesh.GetVerticesCount());
        const glm::vec3* meshVertices = MeshManager::GetPositionsBufferData(Mesh.GetVerticesIndex());

        for (uint32_t i = 0; i < Mesh.GetVerticesCount(); ++i)
        {
            const glm::vec3& v = meshVertices[i];
            joltVertices.emplace_back(v.x, v.y, v.z);
        }

        const JPH::ConvexHullShapeSettings shapeSettings(joltVertices);
        return shapeSettings.Create().Get();
    }
}
