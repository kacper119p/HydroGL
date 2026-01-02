#pragma once
#include <cstdint>
#include <vector>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "Utility/GlResourceUnique.h"

namespace Core
{
    class OceanGrid
    {
    private:
        struct CDLODNodeData
        {
            glm::vec2 Position;
            float Scale;
            float Level;
        };

    private:
        static constexpr int32_t LodLevels = 10;

    private:
        float _lodDistances[LodLevels];
        float _minLodDistance = 300.0f;

        GlVertexArrayUnique _vao;
        GlBufferUnique _vbo;
        GlBufferUnique _patchPositionsVbo;
        GlBufferUnique _ebo;
        uint32_t _patchCount = 0;
        int32_t _elementCount = 0;
        float _quadSize;
        float _gridSize;

        glm::vec3* _allPatchPositions;
        glm::vec3* _visiblePatches;

    public:
        OceanGrid(float HalfWidth, uint32_t PatchCount, uint32_t PatchSubdivisions);

    public:
        ~OceanGrid();

    public:
        void Draw(const glm::mat4& ProjectionMatrix, const glm::mat4& ViewMatrix,
                  const glm::vec3& CameraPosition) const;

    private:
        [[nodiscard]] uint32_t FrustumCulling(const glm::mat4& ViewProjection, const glm::vec3& GridOffset) const;

        bool SelectNode(const glm::vec3& CameraPosition, glm::vec2 NodePosition, float NodeSize, int32_t Level,
                        int32_t& BufferHead) const;

        [[nodiscard]] bool IsNodeInRange(const glm::vec3& CameraPosition, glm::vec2 NodePosition, float NodeSize,
                                         int32_t Level) const;
    };
}
