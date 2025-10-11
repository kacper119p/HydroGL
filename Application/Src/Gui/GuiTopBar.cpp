#include "GuiTopBar.h"

#include <format>
#include <imgui.h>
#include <string>

namespace Application
{
    void GuiTopBar::Render()
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        constexpr ImGuiWindowFlags hostFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                               ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
                                               ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpaceHost", nullptr, hostFlags);
        ImGui::PopStyleVar();
        const ImGuiID dockspaceId = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

        ImGuiIO& io = ImGui::GetIO();

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::Button("Left Button"))
            {
            }

            const std::string rightText = std::format("Application average {:.3f} ms/frame({:.1f} FPS)",
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
        ImGui::End();
    }
}
