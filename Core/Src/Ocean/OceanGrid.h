#pragma once
#include <cstdint>
#include <vector>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "Camera/Frustum.h"
#include "Utility/GlResourceUnique.h"

namespace Core
{
    class OceanGrid
    {
    private:
        struct QuadtreeNode
        {
            glm::vec2 Position;
            float Scale;
            float Level;
        };

    private:
        static constexpr int32_t LodLevels = 7;

    private:
        float _lodDistances[LodLevels];
        float _minLodDistance = 100.0f;

        GlVertexArrayUnique _vao;
        GlBufferUnique _vbo;
        GlBufferUnique _patchInstanceDataVbo;
        uint32_t _patchInstanceDataVboSize;
        GlBufferUnique _ebo;
        int32_t _elementCount = 0;
        float _quantizationSize;
        float _gridSize;

        std::vector<glm::vec4> _visiblePatches;

    public:
        explicit OceanGrid(float HalfWidth);

    public:
        ~OceanGrid();

    public:
        void Draw(const glm::mat4& ProjectionMatrix, const glm::mat4& ViewMatrix,
                  const glm::vec3& CameraPosition);

    private:
        bool SelectNode(const glm::vec3& CameraPosition, glm::vec2 NodePosition, float NodeSize, int32_t Level,
                        const Frustum& Frustum);

        [[nodiscard]] bool IsNodeInRange(const glm::vec3& CameraPosition, glm::vec2 NodePosition, float NodeSize,
                                         int32_t Level) const;
    };
}
