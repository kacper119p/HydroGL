#pragma once
#include <cstdint>

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
        int32_t _patchCount = 0;
        int32_t _elementCount = 0;
        float _quadSize;

    public:
        [[nodiscard]] float GetQuadSize() const
        {
            return _quadSize;
        }

        OceanGrid(float HalfWidth, uint32_t PatchCount, uint32_t PatchSubdivisions);

        void Draw() const;

    public:
        ~OceanGrid();
    };
}
