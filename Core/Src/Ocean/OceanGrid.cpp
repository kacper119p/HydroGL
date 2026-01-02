#include "OceanGrid.h"

#include <array>
#include <glad/glad.h>
#include <glm/common.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "spdlog/spdlog.h"

namespace
{
    constexpr float PatchSize = 1.0f;
    constexpr float HalfPatchSize = 0.5f;
    constexpr uint32_t PatchSubdivisions = 64;
    constexpr uint32_t DefaultInstanceDataBufferSize = 1024;
    constexpr float VerticalExtent = 100.0f;
    constexpr float HorizontalMargin = 30.0f;
}

namespace Core
{
    OceanGrid::OceanGrid(const float HalfWidth)
        : _gridSize(HalfWidth * 2.0f)
    {
        _quantizationSize = PatchSize / static_cast<float>(PatchSubdivisions);

        constexpr uint32_t verticesOnEdge = PatchSubdivisions + 1;
        constexpr uint32_t vertexCount = verticesOnEdge * verticesOnEdge;
        constexpr uint32_t indexCount = PatchSubdivisions * PatchSubdivisions * 4;

        glm::vec3* const vertexPositions = new glm::vec3[vertexCount];
        uint32_t* const indices = new uint32_t[indexCount];
        _visiblePatches.reserve(DefaultInstanceDataBufferSize);

        for (uint32_t y = 0; y < verticesOnEdge; ++y)
        {
            for (uint32_t x = 0; x < verticesOnEdge; ++x)
            {
                vertexPositions[y * verticesOnEdge + x] = glm::vec3(
                    -HalfPatchSize + static_cast<float>(x) * _quantizationSize,
                    0.0f,
                    -HalfPatchSize + static_cast<float>(y) * _quantizationSize);
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

        _vao = GlVertexArrayUnique::Create();
        _vbo = GlBufferUnique::Create();
        _patchinstanceDataVbo = GlBufferUnique::Create();
        _ebo = GlBufferUnique::Create();

        glNamedBufferData(_vbo, static_cast<GLsizeiptr>(sizeof(glm::vec3) * vertexCount), vertexPositions,
                          GL_STATIC_DRAW);

        glEnableVertexArrayAttrib(_vao, 0);
        glVertexArrayAttribFormat(_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(_vao, 0, 0);
        glVertexArrayVertexBuffer(_vao, 0, _vbo, 0, sizeof(glm::vec3));

        _patchinstanceDataVboSize = DefaultInstanceDataBufferSize;
        glNamedBufferStorage(_patchinstanceDataVbo,
                             static_cast<GLsizeiptr>(sizeof(glm::vec4) * DefaultInstanceDataBufferSize), nullptr,
                             GL_DYNAMIC_STORAGE_BIT);

        glEnableVertexArrayAttrib(_vao, 1);
        glVertexArrayAttribFormat(_vao, 1, 4, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(_vao, 1, 1);
        glVertexArrayBindingDivisor(_vao, 1, 1);
        glVertexArrayVertexBuffer(_vao, 1, _patchinstanceDataVbo, 0, sizeof(glm::vec4));

        glNamedBufferStorage(_ebo, static_cast<GLsizeiptr>(sizeof(uint32_t) * indexCount), indices, GL_NONE);
        glVertexArrayElementBuffer(_vao, _ebo);

        _elementCount = static_cast<int>(indexCount);
        delete[] vertexPositions;
        delete[] indices;


        for (int32_t i = 0; i < LodLevels; i++)
        {
            _lodDistances[i] = _minLodDistance * static_cast<float>(std::pow(2, i));
        }
        float minPatchSize = _gridSize;
        for (int32_t i = 0; i < LodLevels / 2; i++)
        {
            minPatchSize *= 0.5f;
        }
        _quantizationSize = 1.0f / static_cast<float>(PatchSubdivisions) * minPatchSize;
    }

    OceanGrid::~OceanGrid() = default;

    void OceanGrid::Draw(const glm::mat4& ProjectionMatrix, const glm::mat4& ViewMatrix,
                         const glm::vec3& CameraPosition)
    {
        const glm::mat4 viewProjection = ProjectionMatrix * ViewMatrix;
        _visiblePatches.clear();

        const glm::vec2 gridOrigin =
                glm::floor(glm::vec2(CameraPosition.x, CameraPosition.z) / _quantizationSize) * _quantizationSize;

        SelectNode(CameraPosition, gridOrigin, _gridSize, LodLevels - 1);
        spdlog::info(_visiblePatches.size());

        if (!_visiblePatches.empty())
        {
            if (_visiblePatches.size() > static_cast<int32_t>(_patchinstanceDataVboSize))
            {
                _patchinstanceDataVboSize = _visiblePatches.size();
                _patchinstanceDataVbo = GlBufferUnique::Create();
                glNamedBufferStorage(_patchinstanceDataVbo,
                                     static_cast<int64_t>(sizeof(glm::vec4) * _visiblePatches.size()),
                                     _visiblePatches.data(),
                                     GL_DYNAMIC_STORAGE_BIT);
                glVertexArrayVertexBuffer(_vao, 1, _patchinstanceDataVbo, 0, sizeof(glm::vec4));
            } else
            {
                glNamedBufferSubData(_patchinstanceDataVbo,
                                     0,
                                     static_cast<int64_t>(sizeof(glm::vec4) * _visiblePatches.size()),
                                     _visiblePatches.data());
            }


            glBindVertexArray(_vao);
            glPatchParameteri(GL_PATCH_VERTICES, 4);

            //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDrawElementsInstanced(GL_PATCHES,
                                    _elementCount,
                                    GL_UNSIGNED_INT,
                                    nullptr,
                                    static_cast<int32_t>(_visiblePatches.size()));
            //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }

    uint32_t OceanGrid::FrustumCulling(const glm::mat4& ViewProjection, const glm::vec3& GridOffset) const
    {
        const float* const m = glm::value_ptr(ViewProjection);
        glm::vec4 planes[6]{
            glm::vec4(m[3] + m[0], m[7] + m[4], m[11] + m[8], m[15] + m[12]), // Left
            glm::vec4(m[3] - m[0], m[7] - m[4], m[11] - m[8], m[15] - m[12]), // Right
            glm::vec4(m[3] + m[1], m[7] + m[5], m[11] + m[9], m[15] + m[13]), // Bottom
            glm::vec4(m[3] - m[1], m[7] - m[5], m[11] - m[9], m[15] - m[13]), // Top
            glm::vec4(m[3] + m[2], m[7] + m[6], m[11] + m[10], m[15] + m[14]), // Near
            glm::vec4(m[3] - m[2], m[7] - m[6], m[11] - m[10], m[15] - m[14]) // Far
        };
        for (auto& plane : planes)
        {
            const float length = glm::length(glm::vec3(plane));
            plane /= length;
        }

        float planeBoxProjections[6];
        for (int i = 0; i < 6; ++i)
        {
            glm::vec3 absNormal = glm::abs(glm::vec3(planes[i]));
            // planeBoxProjections[i] = glm::dot(absNormal, _patchExtent);
        }

        uint32_t visiblePatchCount = 0;
        for (uint32_t i = 0; i < 0; ++i)
        {
            bool inside = true;
            const glm::vec3 worldPatchPos = /*_allPatchPositions[i]*/ +glm::vec3(GridOffset.x, 0.0f, GridOffset.z);

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
                // _visiblePatches[visiblePatchCount++] = _allPatchPositions[i];
            }
        }
        return visiblePatchCount;
    }

    bool OceanGrid::SelectNode(const glm::vec3& CameraPosition, const glm::vec2 NodePosition, const float NodeSize,
                               const int32_t Level)
    {
        if (!IsNodeInRange(CameraPosition, NodePosition, NodeSize, Level))
        {
            return false;
        }

        // if(!node->_boundingBox.frustumIntersect(_cameraFrustum)){
        //     // Skip nodes node not visible to camera.
        //     return true;
        // }

        if (Level == 0 || !IsNodeInRange(CameraPosition, NodePosition, NodeSize, Level - 1))
        {
            _visiblePatches.emplace_back(NodePosition.x, NodePosition.y, NodeSize, Level);
            return true;
        }

        const float childSize = NodeSize * 0.5f;
        const float quarter = NodeSize * 0.25f;

        const glm::vec2 childOffsets[4] = {
            {-quarter, -quarter},
            {quarter, -quarter},
            {quarter, quarter},
            {-quarter, quarter}
        };

        for (auto childOffset : childOffsets)
        {
            if (!SelectNode(CameraPosition, NodePosition + childOffset, childSize, Level - 1))
            {
                _visiblePatches.emplace_back(NodePosition.x + childOffset.x,
                                             NodePosition.y + childOffset.y,
                                             childSize,
                                             Level - 1);
            }
        }

        return true;
    }

    bool OceanGrid::IsNodeInRange(const glm::vec3& CameraPosition, glm::vec2 NodePosition, float NodeSize,
                                  const int32_t Level) const
    {
        const float halfSide = NodeSize * 0.5f;
        const float minX = NodePosition.x - halfSide;
        const float maxX = NodePosition.x + halfSide;
        const float minZ = NodePosition.y - halfSide;
        const float maxZ = NodePosition.y + halfSide;
        const float closestX = glm::max(minX, glm::min(CameraPosition.x, maxX));
        const float closestZ = glm::max(minZ, glm::min(CameraPosition.z, maxZ));

        const glm::vec3 closestPoint(closestX, 0.0f, closestZ);

        const glm::vec3 difference = CameraPosition - closestPoint;
        const float distanceSquared = glm::dot(difference, difference);

        return distanceSquared <= (_lodDistances[Level] * _lodDistances[Level]);
    }
}
