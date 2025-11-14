#include "BoundingBox.h"

#include "Meshes/MeshManager.h"

namespace Core
{
    BoundingBox::BoundingBox()
        : _min(0.0f),
          _max(0.0f)
    {
    }


    BoundingBox::BoundingBox(const Mesh& Mesh)
    {
        const glm::vec3* positions = MeshManager::GetPositionsBufferData(Mesh.GetVerticesIndex());
        _min = positions[0];
        _max = positions[0];

        for (int i = 1; i < Mesh.GetVerticesCount(); ++i)
        {
            _max = glm::max(_max, positions[i]);
            _min = glm::min(_min, positions[i]);
        }
    }
}
