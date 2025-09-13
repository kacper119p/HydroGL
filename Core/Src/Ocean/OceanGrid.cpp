#include "OceanGrid.h"

#include <glad/glad.h>
#include <glm/vec3.hpp>

namespace Core
{
    OceanGrid::OceanGrid(const float HalfWidth, const uint32_t PatchCount, const uint32_t PatchSubdivisions)
    {
        const float patchSize = (HalfWidth * 2.0f) / static_cast<float>(PatchCount);
        const float halfPatchSize = patchSize * 0.5f;
        _quadSize = patchSize / static_cast<float>(PatchSubdivisions);

        const uint32_t verticesOnEdge = PatchSubdivisions + 1;

        const uint32_t vertexCount = verticesOnEdge * verticesOnEdge;
        const uint32_t indexCount = PatchSubdivisions * PatchSubdivisions * 4;
        const uint32_t patchCount = PatchCount * PatchCount;

        glm::vec3* vertexPositions = new glm::vec3[vertexCount];
        uint32_t* indices = new uint32_t[indexCount];
        glm::vec3* patchPositions = new glm::vec3[patchCount];

        for (uint32_t y = 0; y < verticesOnEdge; ++y)
        {
            for (uint32_t x = 0; x < verticesOnEdge; ++x)
            {
                vertexPositions[y * verticesOnEdge + x] = glm::vec3(
                    -halfPatchSize + static_cast<float>(x) * _quadSize,
                    0.0f,
                    -halfPatchSize + static_cast<float>(y) * _quadSize);
            }
        }
        for (uint32_t y = 0; y < PatchSubdivisions; ++y)
        {
            for (uint32_t x = 0; x < PatchSubdivisions; ++x)
            {
                const uint32_t baseIndex = (y * PatchSubdivisions + x) * 4;
                const uint32_t lowerLeft = y * verticesOnEdge + x;
                const uint32_t upperLeft = lowerLeft + verticesOnEdge;

                indices[baseIndex + 0] = lowerLeft;
                indices[baseIndex + 1] = upperLeft;
                indices[baseIndex + 2] = upperLeft + 1;
                indices[baseIndex + 3] = lowerLeft + 1;
            }
        }

        for (uint32_t y = 0; y < PatchCount; ++y)
        {
            for (uint32_t x = 0; x < PatchCount; ++x)
            {
                patchPositions[y * PatchCount + x] = glm::vec3(
                    -HalfWidth + static_cast<float>(x) * patchSize,
                    0.0f,
                    -HalfWidth + static_cast<float>(y) * patchSize);
            }
        }

        glGenVertexArrays(1, &_vao);
        glGenBuffers(1, &_vbo);
        glGenBuffers(1, &_patchPositionsVbo);
        glGenBuffers(1, &_ebo);

        glBindVertexArray(_vao);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, static_cast<int64_t>(sizeof(glm::vec3) * vertexCount),
                     vertexPositions,GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), reinterpret_cast<void*>(0));

        glBindBuffer(GL_ARRAY_BUFFER, _patchPositionsVbo);
        glBufferData(GL_ARRAY_BUFFER, static_cast<int64_t>(sizeof(glm::vec3) * patchCount),
                     patchPositions, GL_STATIC_DRAW);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), reinterpret_cast<void*>(0));
        glVertexAttribDivisor(1, 1);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<int64_t>(sizeof(uint32_t) * indexCount),
                     indices, GL_STATIC_DRAW);


        delete[] vertexPositions;
        delete[] indices;
        delete[] patchPositions;

        _patchCount = static_cast<int>(patchCount);
        _elementCount = static_cast<int>(indexCount);
    }

    void OceanGrid::Draw() const
    {
        glBindVertexArray(_vao);
        glPatchParameteri(GL_PATCH_VERTICES, 4);
        glDrawElementsInstanced(GL_PATCHES, _elementCount, GL_UNSIGNED_INT,
                                nullptr, _patchCount);
    }

    OceanGrid::~OceanGrid()
    {
        glDeleteVertexArrays(1, &_vao);
        glDeleteBuffers(1, &_vbo);
        glDeleteBuffers(1, &_patchPositionsVbo);
        glDeleteBuffers(1, &_ebo);
    }
}
