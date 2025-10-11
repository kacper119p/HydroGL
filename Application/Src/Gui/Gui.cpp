#include "Gui.h"

#include <imgui.h>
#include <iostream>

#include "ImGuiContext.h"

namespace Application
{
    Gui::Gui(const Platform::Window& Window)
    {
        Platform::ImGuiContext::InitializeImGui(Window);
        constexpr float defaultImGuiFontSize = 13.0f;
        const float fontSize = defaultImGuiFontSize * static_cast<float>(Window.GetWindowWidth()) / 1080.0f;
        ImGui::GetIO().Fonts->AddFontFromFileTTF("Assets/Fonts/GoogleSansCode-VariableFont_wght.ttf", fontSize);
    }

    Gui::~Gui()
    {
        Platform::ImGuiContext::ImGuiShutdown();
    }

    void Gui::Render()
    {
        Platform::ImGuiContext::ImGuiBeginFrame();
        _topBar.Render();
        Platform::ImGuiContext::ImGuiEndFrame();
    }
}
