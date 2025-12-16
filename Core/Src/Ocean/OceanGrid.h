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
        GlVertexArrayUnique _vao;
        GlBufferUnique _vbo;
        GlBufferUnique _patchPositionsVbo;
        GlBufferUnique _ebo;
        uint32_t _patchCount = 0;
        int32_t _elementCount = 0;
        float _quadSize;

        glm::vec3* _allPatchPositions;
        glm::vec3* _visiblePatches;
        glm::vec3 _patchExtent;

    public:
        [[nodiscard]] float GetQuadSize() const
        {
            return _quadSize;
        }

    public:
        OceanGrid(float HalfWidth, uint32_t PatchCount, uint32_t PatchSubdivisions);

    public:
        ~OceanGrid();

    public:
        void Draw(const glm::mat4& ProjectionMatrix, const glm::mat4& ViewMatrix,
                  const glm::vec3& CameraPosition) const;

    private:
        [[nodiscard]] uint32_t FrustumCulling(const glm::mat4& ViewProjection, const glm::vec3& GridOffset) const;
    };
}
