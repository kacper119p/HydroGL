#pragma once
#include <cstdint>

namespace Core
{
    struct DrawElementsIndirectCommand
    {
        uint32_t count;
        uint32_t instanceCount;
        uint32_t firstIndex;
        uint32_t baseVertex;
        uint32_t baseInstance;
    };
}
