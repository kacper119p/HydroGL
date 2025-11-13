#include "SpectrumSettingsWindow.h"

#include <imgui.h>

namespace Application
{
    void SpectrumSettingsWindow::Render(bool& ShowSpectrumSettings, Core::WaterSystem& WaterSystem)
    {
        constexpr ImGuiWindowFlags windowFlags =
                ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoResize;
        if (ImGui::Begin("Spectrum Settings", &ShowSpectrumSettings, windowFlags))
        {
            glm::vec2 windVelocity = WaterSystem.GetWindVelocity();
            if (ImGui::DragFloat2("Wind Velocity [m/s]", &windVelocity[0]))
            {
                WaterSystem.SetWindVelocity(windVelocity);
            }
            float fetch = WaterSystem.GetFetch();
            if (ImGui::DragFloat("Fetch [m]", &fetch, 1.0f, 0.1f,
                                 std::numeric_limits<float>::max(), "%.1f")
            )
            {
                WaterSystem.SetFetch(glm::max(fetch, 0.1f));
            }
            float delta = WaterSystem.GetDelta();
            if (ImGui::SliderFloat("Delta", &delta, 0.0f, 1.0f))
            {
                WaterSystem.SetDelta(delta);
            }
            float swell = WaterSystem.GetSwell();
            if (ImGui::SliderFloat("Swell", &swell, 0.0f, 1.0f))
            {
                WaterSystem.SetSwell(swell);
            }
        }
        ImGui::End();
    }
}
