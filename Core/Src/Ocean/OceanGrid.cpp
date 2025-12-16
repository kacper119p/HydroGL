#include "OceanGrid.h"

#include <array>
#include <cmath>
#include <vector>
#include <glad/glad.h>
#include <glm/common.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Core
{
    OceanGrid::OceanGrid(const float HalfWidth, const uint32_t PatchCount, const uint32_t PatchSubdivisions)
    {
        const float patchSize = (HalfWidth * 2.0f) / static_cast<float>(PatchCount);
        const float halfPatchSize = patchSize * 0.5f;
        _quadSize = patchSize / static_cast<float>(PatchSubdivisions);

        constexpr float verticalExtend = 100.0f;
        constexpr float horizontalMargin = 30.0f;

        _patchExtent = glm::vec3(halfPatchSize + horizontalMargin, verticalExtend,
                                 halfPatchSize + horizontalMargin);

        const uint32_t verticesOnEdge = PatchSubdivisions + 1;

        const uint32_t vertexCount = verticesOnEdge * verticesOnEdge;
        const uint32_t indexCount = PatchSubdivisions * PatchSubdivisions * 4;
        const uint32_t totalPatchCount = PatchCount * PatchCount;

        glm::vec3* const vertexPositions = new glm::vec3[vertexCount];
        uint32_t* const indices = new uint32_t[indexCount];
        _patchCount = totalPatchCount;
        _allPatchPositions = new glm::vec3[totalPatchCount];
        _visiblePatches = new glm::vec3[totalPatchCount];

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
                _allPatchPositions[y * PatchCount + x] = glm::vec3(
                    -HalfWidth + static_cast<float>(x) * patchSize + halfPatchSize,
                    0.0f,
                    -HalfWidth + static_cast<float>(y) * patchSize + halfPatchSize);
            }
        }

        _vao = GlVertexArrayUnique::Create();
        _vbo = GlBufferUnique::Create();
        _patchPositionsVbo = GlBufferUnique::Create();
        _ebo = GlBufferUnique::Create();

        glNamedBufferData(_vbo, static_cast<GLsizeiptr>(sizeof(glm::vec3) * vertexCount), vertexPositions,
                          GL_STATIC_DRAW);

        glEnableVertexArrayAttrib(_vao, 0);
        glVertexArrayAttribFormat(_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(_vao, 0, 0);
        glVertexArrayVertexBuffer(_vao, 0, _vbo, 0, sizeof(glm::vec3));


        glNamedBufferStorage(_patchPositionsVbo, static_cast<GLsizeiptr>(sizeof(glm::vec3) * _patchCount), nullptr,
                             GL_DYNAMIC_STORAGE_BIT);

        glEnableVertexArrayAttrib(_vao, 1);
        glVertexArrayAttribFormat(_vao, 1, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(_vao, 1, 1);
        glVertexArrayBindingDivisor(_vao, 1, 1);
        glVertexArrayVertexBuffer(_vao, 1, _patchPositionsVbo, 0, sizeof(glm::vec3));

        glNamedBufferStorage(_ebo, static_cast<GLsizeiptr>(sizeof(uint32_t) * indexCount), indices, GL_NONE);
        glVertexArrayElementBuffer(_vao, _ebo);

        _elementCount = static_cast<int>(indexCount);
        delete[] vertexPositions;
        delete[] indices;
    }

    OceanGrid::~OceanGrid()
    {
        delete[] _allPatchPositions;
        delete[] _visiblePatches;
    }

    void OceanGrid::Draw(const glm::mat4& ProjectionMatrix, const glm::mat4& ViewMatrix,
                         const glm::vec3& CameraPosition) const
    {
        const glm::mat4 viewProjection = ProjectionMatrix * ViewMatrix;
        const uint32_t visiblePatchCount = FrustumCulling(viewProjection, CameraPosition);

        if (visiblePatchCount > 0)
        {
            glNamedBufferSubData(_patchPositionsVbo,
                                 0,
                                 static_cast<int64_t>(sizeof(glm::vec3) * visiblePatchCount),
                                 _visiblePatches);

            glBindVertexArray(_vao);
            glPatchParameteri(GL_PATCH_VERTICES, 4);

            glDrawElementsInstanced(GL_PATCHES,
                                    _elementCount,
                                    GL_UNSIGNED_INT,
                                    nullptr,
                                    static_cast<int32_t>(visiblePatchCount));
        }
    }

    uint32_t OceanGrid::FrustumCulling(const glm::mat4& ViewProjection, const glm::vec3& GridOffset) const
    {
        const float* const m = glm::value_ptr(ViewProjection);
        glm::vec4 planes[6]{
            planes[0] = glm::vec4(m[3] + m[0], m[7] + m[4], m[11] + m[8], m[15] + m[12]), // Left
            planes[1] = glm::vec4(m[3] - m[0], m[7] - m[4], m[11] - m[8], m[15] - m[12]), // Right
            planes[2] = glm::vec4(m[3] + m[1], m[7] + m[5], m[11] + m[9], m[15] + m[13]), // Bottom
            planes[3] = glm::vec4(m[3] - m[1], m[7] - m[5], m[11] - m[9], m[15] - m[13]), // Top
            planes[4] = glm::vec4(m[3] + m[2], m[7] + m[6], m[11] + m[10], m[15] + m[14]), // Near
            planes[5] = glm::vec4(m[3] - m[2], m[7] - m[6], m[11] - m[10], m[15] - m[14]) // Far
        };
        for (auto& plane : planes)
        {
            const float length = std::sqrt(plane.x * plane.x + plane.y * plane.y + plane.z * plane.z);
            plane /= length;
        }

        float planeBoxProjections[6];
        for (int i = 0; i < 6; ++i)
        {
            glm::vec3 absNormal = glm::abs(glm::vec3(planes[i]));
            planeBoxProjections[i] = glm::dot(absNormal, _patchExtent);
        }

        uint32_t visiblePatchCount = 0;
        for (uint32_t i = 0; i < _patchCount; ++i)
        {
            bool inside = true;
            const glm::vec3 worldPatchPos = _allPatchPositions[i] + GridOffset;

            for (uint32_t j = 0; j < 6; ++j)
            {
                const float distance = glm::dot(glm::vec3(planes[j]), worldPatchPos) + planes[j].w;
                if (distance < -planeBoxProjections[j])
                {
                    inside = false;
                    break;
                }
            }

            if (inside)
            {
                _visiblePatches[visiblePatchCount++] = _allPatchPositions[i];
            }
        }
        return visiblePatchCount;
    }
}
