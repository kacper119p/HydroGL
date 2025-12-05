#pragma once
#include "Ocean/WaterSystem.h"

namespace Application
{
    class WaterSettingsWindow
    {
    public:
        static void Render(bool& ShowWaterSettings, Core::WaterSystem& WaterSystem);
    };
}
