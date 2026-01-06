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
    constexpr uint32_t DefaultInstanceDataBufferSize = 1024;
    constexpr float VerticalExtent = 100.0f;
    constexpr float HorizontalMargin = 30.0f;

    void UpdateGpuBuffer(const std::vector<glm::vec4>& CpuBuffer, Core::GlBufferUnique& GpuBuffer,
                         uint32_t& GpuBufferCapacity)
    {
        if (CpuBuffer.size() > static_cast<int32_t>(GpuBufferCapacity))
        {
            GpuBufferCapacity = CpuBuffer.size();
            GpuBuffer = Core::GlBufferUnique::Create();
            glNamedBufferStorage(GpuBuffer,
                                 static_cast<int64_t>(sizeof(glm::vec4) * CpuBuffer.size()),
                                 CpuBuffer.data(),
                                 GL_DYNAMIC_STORAGE_BIT);
        } else
        {
            glNamedBufferSubData(GpuBuffer,
                                 0,
                                 static_cast<int64_t>(sizeof(glm::vec4) * CpuBuffer.size()),
                                 CpuBuffer.data());
        }
    }

    void RenderPatches(const Core::GlVertexArrayUnique& Vao, GLenum Mode, const int32_t ElementCount,
                       const int32_t Count,
                       const bool RenderAsWireframe)
    {
        glBindVertexArray(Vao);

        if (RenderAsWireframe)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }

        glDrawElementsInstanced(Mode,
                                ElementCount,
                                GL_UNSIGNED_INT,
                                nullptr,
                                Count);

        if (RenderAsWireframe)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }
}

namespace Core
{
    OceanGrid::OceanGrid()
    {
        constexpr float quadSize = PatchSize / static_cast<float>(PatchSubdivisions);
        constexpr uint32_t verticesOnEdge = PatchSubdivisions + 1;

        _vbo = CreateVbo(verticesOnEdge, quadSize);
        InitializeNearVao(verticesOnEdge);
        InitializeFarVao(verticesOnEdge);

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

    void OceanGrid::UpdateBuffers(const glm::mat4& ProjectionMatrix, const glm::mat4& ViewMatrix,
                                  const glm::vec3& CameraPosition)
    {
        _patchesNear.clear();
        _patchesFar.clear();

        const glm::vec2 gridOrigin =
                glm::floor(glm::vec2(CameraPosition.x, CameraPosition.z) / _quantizationSize) * _quantizationSize;
        const Frustum cameraFrustum(ViewMatrix, ProjectionMatrix);

        SelectNode(CameraPosition, gridOrigin, GridSize, LodLevels - 1, cameraFrustum);

        UpdateGpuBuffer(_patchesNear, _patchInstanceDataNearVbo, _patchInstanceDataNearVboSize);
        UpdateGpuBuffer(_patchesFar, _patchInstanceDataFarVbo, _patchInstanceDataFarVboSize);
    }

    void OceanGrid::RenderNearPatches() const
    {
        if (!_patchesNear.empty())
        {
            glPatchParameteri(GL_PATCH_VERTICES, 4);
            RenderPatches(_nearVao,
                          GL_PATCHES,
                          _nearElementCount,
                          static_cast<int32_t>(_patchesNear.size()),
                          _renderAsWireframe);;
        }
    }

    void OceanGrid::RenderFarPatches() const
    {
        if (!_patchesFar.empty())
        {
            RenderPatches(_farVao,
                          GL_TRIANGLES,
                          _farElementCount,
                          static_cast<int32_t>(_patchesFar.size()),
                          _renderAsWireframe);;
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

        if (Level == 0)
        {
            _patchesNear.emplace_back(NodePosition.x, NodePosition.y, NodeSize, Level);
            return true;
        }

        if (!IsNodeInRange(CameraPosition, NodePosition, NodeSize, Level - 1))
        {
            _patchesFar.emplace_back(NodePosition.x, NodePosition.y, NodeSize, Level);
        }

        const float childSize = NodeSize * 0.5f;
        const float quarter = NodeSize * 0.25f;

        const glm::vec2 childOffsets[4] = {
            {-quarter, -quarter},
            {quarter, -quarter},
            {quarter, quarter},
            {-quarter, quarter}
        };

        const int32_t childLevel = Level - 1;

        for (auto childOffset : childOffsets)
        {
            if (!SelectNode(CameraPosition, NodePosition + childOffset, childSize, childLevel, Frustum))
            {
                if (childLevel > 0)
                {
                    _patchesFar.emplace_back(NodePosition.x + childOffset.x,
                                             NodePosition.y + childOffset.y,
                                             childSize,
                                             childLevel);
                } else
                {
                    _patchesNear.emplace_back(NodePosition.x + childOffset.x,
                                              NodePosition.y + childOffset.y,
                                              childSize,
                                              childLevel);
                }
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

    void OceanGrid::InitializeNearVao(const uint32_t VerticesOnEdge)
    {
        _nearVao = GlVertexArrayUnique::Create();
        _nearEbo = GlBufferUnique::Create();
        _patchInstanceDataNearVbo = GlBufferUnique::Create();

        constexpr uint32_t indicesPerQuad = 4;
        constexpr uint32_t indexCount = PatchSubdivisions * PatchSubdivisions * indicesPerQuad;
        uint32_t* const indices = new uint32_t[indexCount];
        for (uint32_t y = 0; y < PatchSubdivisions; ++y)
        {
            for (uint32_t x = 0; x < PatchSubdivisions; ++x)
            {
                const uint32_t baseIndex = (y * PatchSubdivisions + x) * 4;
                const uint32_t lowerLeft = y * VerticesOnEdge + x;
                const uint32_t upperLeft = lowerLeft + VerticesOnEdge;

                indices[baseIndex + 0] = lowerLeft;
                indices[baseIndex + 1] = upperLeft;
                indices[baseIndex + 2] = upperLeft + 1;
                indices[baseIndex + 3] = lowerLeft + 1;
            }
        }

        glEnableVertexArrayAttrib(_nearVao, 0);
        glVertexArrayAttribFormat(_nearVao, 0, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(_nearVao, 0, 0);
        glVertexArrayVertexBuffer(_nearVao, 0, _vbo, 0, sizeof(glm::vec3));

        _patchesNear.reserve(DefaultInstanceDataBufferSize);
        _patchInstanceDataNearVboSize = DefaultInstanceDataBufferSize;
        glNamedBufferStorage(_patchInstanceDataNearVbo,
                             static_cast<GLsizeiptr>(sizeof(glm::vec4) * DefaultInstanceDataBufferSize), nullptr,
                             GL_DYNAMIC_STORAGE_BIT);

        glEnableVertexArrayAttrib(_nearVao, 1);
        glVertexArrayAttribFormat(_nearVao, 1, 4, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(_nearVao, 1, 1);
        glVertexArrayBindingDivisor(_nearVao, 1, 1);
        glVertexArrayVertexBuffer(_nearVao, 1, _patchInstanceDataNearVbo, 0, sizeof(glm::vec4));

        glNamedBufferStorage(_nearEbo, static_cast<GLsizeiptr>(sizeof(uint32_t) * indexCount), indices, GL_NONE);
        glVertexArrayElementBuffer(_nearVao, _nearEbo);

        _nearElementCount = static_cast<int>(indexCount);
        delete[] indices;
    }

    void OceanGrid::InitializeFarVao(uint32_t VerticesOnEdge)
    {
        _farVao = GlVertexArrayUnique::Create();
        _farEbo = GlBufferUnique::Create();
        _patchInstanceDataFarVbo = GlBufferUnique::Create();

        constexpr uint32_t indicesPerQuad = 6;
        constexpr uint32_t indexCount = PatchSubdivisions * PatchSubdivisions * indicesPerQuad;
        uint32_t* const indices = new uint32_t[indexCount];

        for (uint32_t y = 0; y < PatchSubdivisions; ++y)
        {
            for (uint32_t x = 0; x < PatchSubdivisions; ++x)
            {
                const uint32_t baseIndex = (y * PatchSubdivisions + x) * indicesPerQuad;
                const uint32_t lowerLeft = y * VerticesOnEdge + x;
                const uint32_t upperLeft = lowerLeft + VerticesOnEdge;
                const uint32_t upperRight = upperLeft + 1;
                const uint32_t lowerRight = lowerLeft + 1;

                indices[baseIndex + 0] = lowerLeft;
                indices[baseIndex + 1] = upperLeft;
                indices[baseIndex + 2] = upperRight;

                indices[baseIndex + 3] = lowerLeft;
                indices[baseIndex + 4] = upperRight;
                indices[baseIndex + 5] = lowerRight;
            }
        }

        glEnableVertexArrayAttrib(_farVao, 0);
        glVertexArrayAttribFormat(_farVao, 0, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(_farVao, 0, 0);
        glVertexArrayVertexBuffer(_farVao, 0, _vbo, 0, sizeof(glm::vec3));

        _patchesFar.reserve(DefaultInstanceDataBufferSize);
        _patchInstanceDataFarVboSize = DefaultInstanceDataBufferSize;
        glNamedBufferStorage(_patchInstanceDataFarVbo,
                             static_cast<GLsizeiptr>(sizeof(glm::vec4) * DefaultInstanceDataBufferSize), nullptr,
                             GL_DYNAMIC_STORAGE_BIT);

        glEnableVertexArrayAttrib(_farVao, 1);
        glVertexArrayAttribFormat(_farVao, 1, 4, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(_farVao, 1, 1);
        glVertexArrayBindingDivisor(_farVao, 1, 1);
        glVertexArrayVertexBuffer(_farVao, 1, _patchInstanceDataFarVbo, 0, sizeof(glm::vec4));

        glNamedBufferStorage(_farEbo, static_cast<GLsizeiptr>(sizeof(uint32_t) * indexCount), indices, GL_NONE);
        glVertexArrayElementBuffer(_farVao, _farEbo);

        _farElementCount = static_cast<int>(indexCount);
        delete[] indices;
    }

    GlBufferUnique OceanGrid::CreateVbo(const uint32_t VerticesOnEdge, const float QuadSize)
    {
        const uint32_t vertexCount = VerticesOnEdge * VerticesOnEdge;
        glm::vec3* const vertexPositions = reinterpret_cast<glm::vec3*>(malloc(sizeof(glm::vec3) * vertexCount));
        GlBufferUnique buffer = GlBufferUnique::Create();
        for (uint32_t y = 0; y < VerticesOnEdge; ++y)
        {
            for (uint32_t x = 0; x < VerticesOnEdge; ++x)
            {
                vertexPositions[y * VerticesOnEdge + x] = glm::vec3(
                    -HalfPatchSize + static_cast<float>(x) * QuadSize,
                    0.0f,
                    -HalfPatchSize + static_cast<float>(y) * QuadSize);
            }
        }

        glNamedBufferStorage(buffer, static_cast<GLsizeiptr>(sizeof(glm::vec3) * vertexCount), vertexPositions,
                             GL_NONE);

        free(vertexPositions);
        return buffer;
    }
}
