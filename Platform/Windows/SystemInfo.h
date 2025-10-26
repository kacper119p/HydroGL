#pragma once

#include <cstdint>
#include <string>


#include "MemoryInfo.h"

namespace Platform::SystemInfo
{
    void Initialize();

    void Release();

    [[nodiscard]] std::string GetCpuInfo();

    [[nodiscard]] MemoryInfo GetRamInfo();

    [[nodiscard]] MemoryInfo GetVRamInfo();
}
