#include "WaterSettingsWindow.h"

#include <imgui.h>

namespace Application
{
    void WaterSettingsWindow::Render(bool& ShowWaterSettings, Core::WaterSystem& WaterSystem)
    {
        constexpr ImGuiWindowFlags windowFlags =
                ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoResize;
        if (ImGui::Begin("Foam Settings", &ShowWaterSettings, windowFlags))
        {
            if (ImGui::CollapsingHeader("Wind Settings", ImGuiTreeNodeFlags_DefaultOpen))
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
            if (ImGui::CollapsingHeader("Foam Settings", ImGuiTreeNodeFlags_DefaultOpen))
            {
                float foamThreshold = WaterSystem.GetFoamThreshold();
                if (ImGui::DragFloat("Threshold", &foamThreshold, 0.01f, 0.0f,
                                     std::numeric_limits<float>::max(), "%.2f"))
                {
                    WaterSystem.SetFoamThreshold(glm::max(foamThreshold, 0.0f));
                }
                float foamExponent = WaterSystem.GetFoamExponent();
                if (ImGui::DragFloat("Exponent", &foamExponent, 0.05f, 0.0f,
                                     std::numeric_limits<float>::max(), "%.2f"))
                {
                    WaterSystem.SetFoamExponent(glm::max(foamExponent, 0.0f));
                }
                float foamIntensity = WaterSystem.GetFoamIntensity();
                if (ImGui::DragFloat("Intensity", &foamIntensity, 0.1f, 0.0f,
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
                if (ImGui::SliderFloat("Opacity", &sprayOpacity, 0.0f, 1.0f))
                {
                    sprayManager.SetSprayOpacity(glm::clamp(sprayOpacity, 0.0f, 1.0f));
                }

                float spraySize = sprayManager.GetSpraySize();
                if (ImGui::DragFloat("Size", &spraySize, 0.001f, 0.1f,
                                     std::numeric_limits<float>::max(), "%.3f"))
                {
                    sprayManager.SetSpraySize(glm::max(spraySize, 0.1f));
                }

                float spawnThreshold = sprayManager.GetSpawnThreshold();
                if (ImGui::SliderFloat("Spawn Threshold", &spawnThreshold, 0.0f, 1.0f))
                {
                    sprayManager.SetSpawnThreshold(glm::clamp(spawnThreshold, 0.0f, 1.0f));
                }

                float waterVelocityInfluence = sprayManager.GetWaterVelocityInfluence();
                if (ImGui::SliderFloat("Water Velocity Influence", &waterVelocityInfluence, 0.0f, 1.0f))
                {
                    sprayManager.SetWaterVelocityInfluence(glm::clamp(waterVelocityInfluence, 0.0f, 1.0f));
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
