#pragma once
#include <cstdint>

namespace Core
{
    class OceanGrid
    {
    private:
        uint32_t _vao = 0;
        uint32_t _vbo = 0;
        uint32_t _ivbo = 0;
        uint32_t _ebo = 0;
        int32_t _patchCount = 0;
        int32_t _elementCount = 0;

    public:
        OceanGrid(float HalfWidth, uint32_t PatchCount, uint32_t PatchSubdivisions);

        void Draw() const;

    public:
        ~OceanGrid();
    };
}
