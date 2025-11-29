#include "FoamSettingsWindow.h"

#include <imgui.h>

namespace Application
{
    void FoamSettingsWindow::Render(bool& ShowFoamSettings, Core::WaterSystem& WaterSystem)
    {
        constexpr ImGuiWindowFlags windowFlags =
                ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoResize;
        if (ImGui::Begin("Foam Settings", &ShowFoamSettings, windowFlags))
        {
            float foamThreshold = WaterSystem.GetFoamThreshold();
            if (ImGui::DragFloat("Foam Threshold", &foamThreshold, 0.01f, 0.0f,
                                 std::numeric_limits<float>::max(), "%.2f"))
            {
                WaterSystem.SetFoamThreshold(glm::max(foamThreshold, 0.0f));
            }
            float foamExponent = WaterSystem.GetFoamExponent();
            if (ImGui::DragFloat("Foam Exponent", &foamExponent, 0.05f, 0.0f,
                                 std::numeric_limits<float>::max(), "%.2f"))
            {
                WaterSystem.SetFoamExponent(glm::max(foamExponent, 0.0f));
            }
            float foamIntensity = WaterSystem.GetFoamIntensity();
            if (ImGui::DragFloat("Foam Intensity", &foamIntensity, 0.1f, 0.0f,
                                 std::numeric_limits<float>::max(), "%.2f"))
            {
                WaterSystem.SetFoamIntensity(glm::max(foamIntensity, 0.0f));
            }
        }
        ImGui::End();
    }
}
