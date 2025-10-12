#pragma once
#include "ImGuiContext.h"
#include "Ocean/WaterSystem.h"

namespace Application
{
    class Gui
    {
        Core::WaterSystem* _waterSystem = nullptr;
        bool _showSpectrumSettings = false;

    public:
        explicit Gui(const Platform::Window& Window, Core::WaterSystem& WaterSystem);

    public:
        ~Gui();

        void Render();
    };
}
