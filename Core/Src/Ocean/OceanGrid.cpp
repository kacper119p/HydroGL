#include "OceanGrid.h"

#include <array>
#include <glad/glad.h>
#include <glm/common.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>

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
    OceanGrid::OceanGrid()
    {
        constexpr float quadSize = PatchSize / static_cast<float>(PatchSubdivisions);
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
                    -HalfPatchSize + static_cast<float>(x) * quadSize,
                    0.0f,
                    -HalfPatchSize + static_cast<float>(y) * quadSize);
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
        _patchInstanceDataVbo = GlBufferUnique::Create();
        _ebo = GlBufferUnique::Create();

        glNamedBufferData(_vbo, static_cast<GLsizeiptr>(sizeof(glm::vec3) * vertexCount), vertexPositions,
                          GL_STATIC_DRAW);

        glEnableVertexArrayAttrib(_vao, 0);
        glVertexArrayAttribFormat(_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(_vao, 0, 0);
        glVertexArrayVertexBuffer(_vao, 0, _vbo, 0, sizeof(glm::vec3));

        _patchInstanceDataVboSize = DefaultInstanceDataBufferSize;
        glNamedBufferStorage(_patchInstanceDataVbo,
                             static_cast<GLsizeiptr>(sizeof(glm::vec4) * DefaultInstanceDataBufferSize), nullptr,
                             GL_DYNAMIC_STORAGE_BIT);

        glEnableVertexArrayAttrib(_vao, 1);
        glVertexArrayAttribFormat(_vao, 1, 4, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(_vao, 1, 1);
        glVertexArrayBindingDivisor(_vao, 1, 1);
        glVertexArrayVertexBuffer(_vao, 1, _patchInstanceDataVbo, 0, sizeof(glm::vec4));

        glNamedBufferStorage(_ebo, static_cast<GLsizeiptr>(sizeof(uint32_t) * indexCount), indices, GL_NONE);
        glVertexArrayElementBuffer(_vao, _ebo);

        _elementCount = static_cast<int>(indexCount);
        delete[] vertexPositions;
        delete[] indices;


        for (int32_t i = 0; i < LodLevels; i++)
        {
            _lodDistances[i] = MinLodDistance * static_cast<float>(std::pow(2, i));
        }
        float quantizationPatchSize = GridSize;
        for (int32_t i = 0; i < LodLevels / 2; i++)
        {
            quantizationPatchSize *= 0.5f;
        }
        _quantizationSize = 1.0f / static_cast<float>(PatchSubdivisions) * quantizationPatchSize;
    }

    OceanGrid::~OceanGrid() = default;

    void OceanGrid::Draw(const glm::mat4& ProjectionMatrix, const glm::mat4& ViewMatrix,
                         const glm::vec3& CameraPosition)
    {
        _visiblePatches.clear();

        const glm::vec2 gridOrigin =
                glm::floor(glm::vec2(CameraPosition.x, CameraPosition.z) / _quantizationSize) * _quantizationSize;
        Frustum cameraFrustum(ViewMatrix, ProjectionMatrix);

        SelectNode(CameraPosition, gridOrigin, GridSize, LodLevels - 1, cameraFrustum);

        if (_visiblePatches.empty())
        {
            return;
        }

        if (_visiblePatches.size() > static_cast<int32_t>(_patchInstanceDataVboSize))
        {
            _patchInstanceDataVboSize = _visiblePatches.size();
            _patchInstanceDataVbo = GlBufferUnique::Create();
            glNamedBufferStorage(_patchInstanceDataVbo,
                                 static_cast<int64_t>(sizeof(glm::vec4) * _visiblePatches.size()),
                                 _visiblePatches.data(),
                                 GL_DYNAMIC_STORAGE_BIT);
            glVertexArrayVertexBuffer(_vao, 1, _patchInstanceDataVbo, 0, sizeof(glm::vec4));
        } else
        {
            glNamedBufferSubData(_patchInstanceDataVbo,
                                 0,
                                 static_cast<int64_t>(sizeof(glm::vec4) * _visiblePatches.size()),
                                 _visiblePatches.data());
        }


        glBindVertexArray(_vao);
        glPatchParameteri(GL_PATCH_VERTICES, 4);

        if (_renderAsWireframe)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }

        glDrawElementsInstanced(GL_PATCHES,
                                _elementCount,
                                GL_UNSIGNED_INT,
                                nullptr,
                                static_cast<int32_t>(_visiblePatches.size()));

        if (_renderAsWireframe)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }

    bool OceanGrid::SelectNode(const glm::vec3& CameraPosition, const glm::vec2 NodePosition, const float NodeSize,
                               const int32_t Level, const Frustum& Frustum)
    {
        if (!IsNodeInRange(CameraPosition, NodePosition, NodeSize, Level))
        {
            return false;
        }

        const float horizontalExtent = NodeSize / 2.0f + HorizontalMargin;
        if (!Frustum.IntersectsAabb(glm::vec3(NodePosition.x, 0.0f, NodePosition.y),
                                    glm::vec3(horizontalExtent, VerticalExtent, horizontalExtent)))
        {
            return true;
        }

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
            if (!SelectNode(CameraPosition, NodePosition + childOffset, childSize, Level - 1, Frustum))
            {
                _visiblePatches.emplace_back(NodePosition.x + childOffset.x,
                                             NodePosition.y + childOffset.y,
                                             childSize,
                                             Level - 1);
            }
        }

        return true;
    }

    bool OceanGrid::IsNodeInRange(const glm::vec3& CameraPosition, const glm::vec2 NodePosition, const float NodeSize,
                                  const int32_t Level) const
    {
        const float halfSide = NodeSize * 0.5f;
        const glm::vec2 minBounds = NodePosition - halfSide;
        const glm::vec2 maxBounds = NodePosition + halfSide;
        const glm::vec2 camera2D(CameraPosition.x, CameraPosition.z);
        const glm::vec2 closest2D = glm::clamp(camera2D, minBounds, maxBounds);

        const glm::vec3 closestPoint(closest2D.x, 0.0f, closest2D.y);

        const glm::vec3 difference = CameraPosition - closestPoint;
        const float distanceSquared = glm::dot(difference, difference);

        return distanceSquared <= (_lodDistances[Level] * _lodDistances[Level]);
    }
}
