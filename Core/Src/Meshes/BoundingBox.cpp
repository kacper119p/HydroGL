#include "BoundingBox.h"
#include "Meshes/MeshManager.h"

namespace Core
{
    BoundingBox::BoundingBox()
        : _min(0.0f),
          _max(0.0f)
    {
    }


    BoundingBox::BoundingBox(const glm::vec3* const Positions, const uint32_t VerticesCount)
    {
        _min = Positions[0];
        _max = Positions[0];

        for (int i = 1; i < VerticesCount; ++i)
        {
            _max = glm::max(_max, Positions[i]);
            _min = glm::min(_min, Positions[i]);
        }
    }
}
