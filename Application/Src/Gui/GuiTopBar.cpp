#include "GuiTopBar.h"

#include <format>
#include <imgui.h>
#include <string>

namespace Application
{
    void GuiTopBar::Render(bool& ShowWaterSettings, bool& ShowSystemInfo,
                           bool& ShowTextureViewer)
    {
        ImGui::BeginMainMenuBar();
        if (ImGui::Button("Water Settings"))
        {
            ShowWaterSettings = !ShowWaterSettings;
        }
        if (ImGui::Button("System Info"))
        {
            ShowSystemInfo = !ShowSystemInfo;
        }
        if (ImGui::Button("Texture Viewer"))
        {
            ShowTextureViewer = !ShowTextureViewer;
        }

        ImGui::Text("Press [H] to toggle GUI");

        const std::string rightText = std::format("Application average: {:.3f} ms/frame({:.1f} FPS)",
                                                  1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        const ImGuiStyle& style = ImGui::GetStyle();
        const float windowWidth = ImGui::GetWindowWidth();
        const ImVec2 textSize = ImGui::CalcTextSize(rightText.c_str());
        float labelX = windowWidth - textSize.x - style.WindowPadding.x;
        const float minX = ImGui::GetCursorPosX() + style.ItemSpacing.x;
        if (labelX < minX) labelX = minX;
        ImGui::SameLine(labelX);
        ImGui::TextUnformatted(rightText.c_str());
        ImGui::EndMainMenuBar();
    }
}
