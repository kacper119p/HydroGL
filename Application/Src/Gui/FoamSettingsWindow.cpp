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
            if (ImGui::CollapsingHeader("Mask Settings", ImGuiTreeNodeFlags_DefaultOpen))
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
            if (ImGui::CollapsingHeader("Spray Settings", ImGuiTreeNodeFlags_DefaultOpen))
            {
                Core::SprayManager& sprayManager = WaterSystem.GetSprayManager();

                float spawnRange = sprayManager.GetSpawnRange();
                if (ImGui::DragFloat("Spawn Range", &spawnRange, 1.0f, 0.0f,
                                     std::numeric_limits<float>::max(), "%.1f"))
                {
                    sprayManager.SetSpawnRange(glm::max(spawnRange, 0.0f));
                }

                float lifetime = sprayManager.GetLifetime();
                if (ImGui::DragFloat("Lifetime", &lifetime, 0.1f, 0.1f,
                                     std::numeric_limits<float>::max(), "%.2f"))
                {
                    sprayManager.SetLifetime(glm::max(lifetime, 0.1f));
                }

                float sprayOpacity = sprayManager.GetSprayOpacity();
                if (ImGui::SliderFloat("Spray Opacity", &sprayOpacity, 0.0f, 1.0f))
                {
                    sprayManager.SetSprayOpacity(glm::clamp(sprayOpacity, 0.0f, 1.0f));
                }

                float spraySize = sprayManager.GetSpraySize();
                if (ImGui::DragFloat("Spray Size", &spraySize, 0.001f, 0.1f,
                                     std::numeric_limits<float>::max(), "%.3f"))
                {
                    sprayManager.SetSpraySize(glm::max(spraySize, 0.1f));
                }

                float spawnThreshold = sprayManager.GetSpawnThreshold();
                if (ImGui::SliderFloat("Spawn Threshold", &spawnThreshold, 0.0f, 1.0f))
                {
                    sprayManager.SetSpawnThreshold(glm::clamp(spawnThreshold, 0.0f, 1.0f));
                }

                float windInfluence = sprayManager.GetWindInfluence();
                if (ImGui::SliderFloat("Wind Influence", &windInfluence, 0.0f, 1.0f))
                {
                    sprayManager.SetWindInfluence(glm::clamp(windInfluence, 0.0f, 1.0f));
                }
            }
        }
        ImGui::End();
    }
}
