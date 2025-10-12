#pragma once
#include "Window.h"

namespace Platform
{
    class ImGuiContext
    {
    public:
        static void InitializeImGui(const Window& window);

        static void ImGuiBeginFrame();

        static void ImGuiEndFrame();

        static void ImGuiShutdown();
    };
}
