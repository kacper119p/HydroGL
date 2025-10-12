#pragma once
#include <cstdint>
#include <string>

namespace Platform
{
    class SystemInfo
    {
    public:
        static [[nodiscard]] std::string GetCpuInfo();

        static [[nodiscard]] uint64_t GetTotalRamKB();
    };
}
