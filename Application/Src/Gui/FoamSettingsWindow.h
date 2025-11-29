#pragma once
#include "Ocean/WaterSystem.h"

namespace Application
{
    class FoamSettingsWindow
    {
    public:
        static void Render(bool& ShowFoamSettings, Core::WaterSystem& WaterSystem);
    };
}
