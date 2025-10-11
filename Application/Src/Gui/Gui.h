#pragma once
#include "ImGuiContext.h"
#include "Gui/GuiTopBar.h"

namespace Application
{
    class Gui
    {
        GuiTopBar _topBar;

    public:
        explicit Gui(const Platform::Window& Window);

    public:
        ~Gui();

        void Render();
    };
}
