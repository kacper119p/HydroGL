#include "Gui.h"

#include <imgui.h>

#include "ImGuiContext.h"
#include "Gui/FoamSettingsWindow.h"
#include "Gui/GuiTopBar.h"
#include "Gui/SpectrumSettingsWindow.h"
#include "Gui/SystemInfoWindow.h"
#include "Gui/TextureViewerWindow.h"

namespace Application
{
    Gui::Gui(const Platform::Window& Window, Core::WaterSystem& WaterSystem)
        : _waterSystem(&WaterSystem),
          _textureArraySliceViewer(std::make_unique<TextureArraySliceViewer>()),
          _windowWidth(Window.GetWindowWidth()),
          _windowHeight(Window.GetWindowHeight()),
          _showGuiCallbackId(Platform::Input::SubscribeToShowGuiEvent(
              [this](const bool ShowGui)
              {
                  _showGui = ShowGui;
              }))
    {
        Platform::ImGuiContext::InitializeImGui(Window);
        constexpr float defaultImGuiFontSize = 13.0f;
        const float fontSize = defaultImGuiFontSize * static_cast<float>(Window.GetWindowWidth()) / 1080.0f;
        ImGui::GetIO().Fonts->AddFontFromFileTTF("Assets/Fonts/GoogleSansCode-VariableFont_wght.ttf", fontSize);
    }

    Gui::~Gui()
    {
        Platform::ImGuiContext::ImGuiShutdown();
        Platform::Input::UnsubscribeToShowGuiEvent(_showGuiCallbackId);
    }

    void Gui::Render()
    {
        Platform::ImGuiContext::ImGuiBeginFrame();
        if (_showGui)
        {
            GuiTopBar::Render(_showSpectrumSettings, _showFoamSettings, _showSystemInfo, _showTextureViewer);
            if (_showSpectrumSettings)
            {
                SpectrumSettingsWindow::Render(_showSpectrumSettings, *_waterSystem);
            }
            if (_showFoamSettings)
            {
                FoamSettingsWindow::Render(_showFoamSettings, *_waterSystem);
            }
            if (_showSystemInfo)
            {
                SystemInfoWindow::Render(_showSystemInfo, _windowWidth, _windowHeight);
            }
            if (_showTextureViewer)
            {
                TextureViewerWindow::Render(_showTextureViewer, *_textureArraySliceViewer);
            }
        }
        Platform::ImGuiContext::ImGuiEndFrame();
    }
}
