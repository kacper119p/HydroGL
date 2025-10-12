#pragma once
#include <cstdint>

namespace Application
{
    class SystemInfoWindow
    {
    public:
        static void Render(bool& ShowSystemInfo, uint32_t WindowWidth, uint32_t WindowHeight);
    };
}
