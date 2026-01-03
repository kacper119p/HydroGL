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
        static constexpr int32_t LodLevels = 8;
        static constexpr float MinLodDistance = 600.0f;
        static constexpr float GridSize = 20000.0f;

    private:
        float _lodDistances[LodLevels];

        GlVertexArrayUnique _vao;
        GlBufferUnique _vbo;
        GlBufferUnique _patchInstanceDataVbo;
        uint32_t _patchInstanceDataVboSize;
        GlBufferUnique _ebo;
        int32_t _elementCount = 0;
        float _quantizationSize;

        std::vector<glm::vec4> _visiblePatches;

        bool _renderAsWireframe = false;

    public:
        [[nodiscard]] float GetMinLodDistance() const
        {
            return _lodDistances[0];
        }

        [[nodiscard]] bool GetRenderAsWireframe() const
        {
            return _renderAsWireframe;
        }

        void SetRenderAsWireframe(const bool RenderAsWireframe)
        {
            _renderAsWireframe = RenderAsWireframe;
        }

    public:
        explicit OceanGrid();

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
