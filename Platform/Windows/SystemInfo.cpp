#include "SystemInfo.h"

#include <Windows.h>

#include <array>
#include <intrin.h>
#include <sysinfoapi.h>


namespace Platform
{
    std::string SystemInfo::GetCpuInfo()
    {
        std::array<int32_t, 4> integerBuffer = {};
        constexpr size_t sizeofIntegerBuffer = sizeof(int32_t) * integerBuffer.size();

        std::array<char, 64> charBuffer = {};

        constexpr std::array<uint32_t, 3> functionIds = {
            0x80000002, //manufacturer
            0x80000003, //model
            0x80000004 //clockspeed
        };

        std::string cpu;

        for (const uint32_t id : functionIds)
        {
            __cpuid(integerBuffer.data(), id);
            std::memcpy(charBuffer.data(), integerBuffer.data(), sizeofIntegerBuffer);
            cpu += std::string(charBuffer.data());
        }

        return cpu;
    }

    uint64_t SystemInfo::GetTotalRamKB()
    {
        uint64_t result;
        GetPhysicallyInstalledSystemMemory(&result);
        return result;
    }
}
