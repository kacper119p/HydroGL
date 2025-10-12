#include "Gui.h"

#include <imgui.h>

#include "ImGuiContext.h"
#include "Gui/GuiTopBar.h"
#include "Gui/SpectrumSettingsWindow.h"

namespace Application
{
    Gui::Gui(const Platform::Window& Window, Core::WaterSystem& WaterSystem) : _waterSystem(&WaterSystem)
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
        GuiTopBar::Render(_showSpectrumSettings);
        if (_showSpectrumSettings)
        {
            SpectrumSettingsWindow::Render(_showSpectrumSettings, *_waterSystem);
        }
        Platform::ImGuiContext::ImGuiEndFrame();
    }
}
