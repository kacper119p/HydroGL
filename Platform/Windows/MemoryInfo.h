#pragma once
#include <cstdint>

namespace Platform
{
    struct MemoryInfo
    {
        uint64_t TotalMemory;
        uint64_t UsedMemory;
    };
}
