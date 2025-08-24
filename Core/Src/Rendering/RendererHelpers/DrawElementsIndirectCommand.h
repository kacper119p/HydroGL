#pragma once
#include <cstdint>

namespace Core
{
    struct DrawElementsIndirectCommand
    {
        uint32_t Count;
        uint32_t InstanceCount;
        uint32_t FirstIndex;
        uint32_t BaseVertex;
        uint32_t BaseInstance;
    };
}
