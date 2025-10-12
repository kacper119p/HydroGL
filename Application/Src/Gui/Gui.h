#pragma once
#include "ImGuiContext.h"
#include "Ocean/WaterSystem.h"

namespace Application
{
    class Gui
    {
        Core::WaterSystem* _waterSystem = nullptr;
        uint32_t _windowWidth;
        uint32_t _windowHeight;
        bool _showSpectrumSettings = false;
        bool _showSystemInfo = false;

    public:
        explicit Gui(const Platform::Window& Window, Core::WaterSystem& WaterSystem);

    public:
        ~Gui();

        void Render();
    };
}
