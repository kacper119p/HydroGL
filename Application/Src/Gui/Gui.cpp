#include "Gui.h"

#include <format>
#include <imgui.h>

#include "ImGuiContext.h"
#include "SystemInfo.h"
#include "Camera/CameraManager.h"

namespace
{
    constexpr ImGuiWindowFlags FloatingWindowFlags =
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoResize;
}

namespace Application
{
    Gui::Gui(const Platform::Window& Window, Core::WaterSystem& WaterSystem, CameraManager& CameraManager)
        : _waterSystem(&WaterSystem),
          _cameraManager(&CameraManager),
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
        if (!_showGui)
        {
            return;
        }

        RenderTopBar();
        if (_showWaterSettings)
        {
            RenderWaterSettings();
        }
        if (_showSystemInfo)
        {
            RenderSystemInfoWindow();
        }
        if (_showCameraSettings)
        {
            RenderCameraSettings();
        }
        if (_showTextureViewer)
        {
            RenderTextureViewer();
        }

        Platform::ImGuiContext::ImGuiEndFrame();
    }

    void Gui::RenderTopBar()
    {
        ImGui::BeginMainMenuBar();
        if (ImGui::Button("Water Settings"))
        {
            _showWaterSettings = !_showWaterSettings;
        }
        if (ImGui::Button("System Info"))
        {
            _showSystemInfo = !_showSystemInfo;
        }
        if (ImGui::Button("Camera Settings"))
        {
            _showCameraSettings = !_showCameraSettings;
        }
        if (ImGui::Button("Texture Viewer"))
        {
            _showTextureViewer = !_showTextureViewer;
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

    void Gui::RenderWaterSettings()
    {
        if (ImGui::Begin("Water Settings", &_showWaterSettings, FloatingWindowFlags))
        {
            if (ImGui::CollapsingHeader("Wind Settings", ImGuiTreeNodeFlags_DefaultOpen))
            {
                glm::vec2 windVelocity = _waterSystem->GetWindVelocity();
                if (ImGui::DragFloat2("Wind Velocity [m/s]", &windVelocity[0]))
                {
                    _waterSystem->SetWindVelocity(windVelocity);
                }
                float fetch = _waterSystem->GetFetch();
                if (ImGui::DragFloat("Fetch [m]", &fetch, 1.0f, 0.1f,
                                     std::numeric_limits<float>::max(), "%.1f")
                )
                {
                    _waterSystem->SetFetch(glm::max(fetch, 0.1f));
                }
                float delta = _waterSystem->GetDelta();
                if (ImGui::SliderFloat("Delta", &delta, 0.0f, 1.0f))
                {
                    _waterSystem->SetDelta(delta);
                }
                float swell = _waterSystem->GetSwell();
                if (ImGui::SliderFloat("Swell", &swell, 0.0f, 1.0f))
                {
                    _waterSystem->SetSwell(swell);
                }
            }
            if (ImGui::CollapsingHeader("Foam Settings", ImGuiTreeNodeFlags_DefaultOpen))
            {
                float foamThreshold = _waterSystem->GetFoamThreshold();
                if (ImGui::DragFloat("Threshold", &foamThreshold, 0.01f, 0.0f,
                                     std::numeric_limits<float>::max(), "%.2f"))
                {
                    _waterSystem->SetFoamThreshold(glm::max(foamThreshold, 0.0f));
                }
                float foamExponent = _waterSystem->GetFoamExponent();
                if (ImGui::DragFloat("Exponent", &foamExponent, 0.05f, 0.0f,
                                     std::numeric_limits<float>::max(), "%.2f"))
                {
                    _waterSystem->SetFoamExponent(glm::max(foamExponent, 0.0f));
                }
                float foamIntensity = _waterSystem->GetFoamIntensity();
                if (ImGui::DragFloat("Intensity", &foamIntensity, 0.1f, 0.0f,
                                     std::numeric_limits<float>::max(), "%.2f"))
                {
                    _waterSystem->SetFoamIntensity(glm::max(foamIntensity, 0.0f));
                }
            }
            if (ImGui::CollapsingHeader("Spray Settings", ImGuiTreeNodeFlags_DefaultOpen))
            {
                Core::SprayManager& sprayManager = _waterSystem->GetSprayManager();

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

    void Gui::RenderSystemInfoWindow()
    {
        if (ImGui::Begin("System Info", &_showSystemInfo, FloatingWindowFlags))
        {
            const GLubyte* renderer = glGetString(GL_RENDERER);
            const std::string cpuInfo = Platform::SystemInfo::GetCpuInfo();
            const Platform::MemoryInfo ramInfo = Platform::SystemInfo::GetRamInfo();
            const GLubyte* version = glGetString(GL_VERSION);
            const GLubyte* shadingLanguageVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
            const Platform::MemoryInfo vRamInfo = Platform::SystemInfo::GetVRamInfo();
            constexpr uint64_t bytesToMbs = 1024ull * 1024ull;
            constexpr uint64_t bytesToGbs = bytesToMbs * 1024ull;

            ImGui::Text("GPU: %s", reinterpret_cast<const char*>(renderer));
            ImGui::Text("CPU: %s", cpuInfo.c_str());
            ImGui::Text("RAM: %.3lf/%lluGB", static_cast<double>(ramInfo.UsedMemory) / bytesToGbs,
                        ramInfo.TotalMemory / bytesToGbs);
            ImGui::Text("VRAM: %llu/%lluMB", vRamInfo.UsedMemory / bytesToMbs, vRamInfo.TotalMemory / bytesToMbs);
            ImGui::Text("OpenGL version: %s", reinterpret_cast<const char*>(version));
            ImGui::Text("Shading Language version: %s", reinterpret_cast<const char*>(shadingLanguageVersion));
            ImGui::Text("Resolution: %dx%d", _windowWidth, _windowHeight);
            ImGui::Separator();
            ImGui::Text("Application average: %.3f ms/frame\n%.1f FPS",
                        1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }
        ImGui::End();
    }

    void Gui::RenderCameraSettings()
    {
        if (ImGui::Begin("Camera Settings", &_showCameraSettings, FloatingWindowFlags))
        {
            float rotationSensitivity = _cameraManager->GetRotationSensitivity() /
                                        CameraManager::DefaultRotationSensitivity;
            if (ImGui::DragFloat("Look Sensitivity", &rotationSensitivity, 0.01f, 0.0f,
                                 std::numeric_limits<float>::max(), "%.2f"))
            {
                _cameraManager->SetRotationSensitivity(
                    glm::max(rotationSensitivity, 0.0f) * CameraManager::DefaultRotationSensitivity);
            }

            float movementSensitivity = _cameraManager->GetMovementSensitivity() /
                                        CameraManager::DefaultMovementSensitivity;
            if (ImGui::DragFloat("Movement Speed", &movementSensitivity, 0.1f, 0.1f,
                                 std::numeric_limits<float>::max(), "%.2f"))
            {
                _cameraManager->SetMovementSensitivity(
                    glm::max(movementSensitivity, 0.1f) * CameraManager::DefaultMovementSensitivity);
            }
        }
        ImGui::End();
    }

    void Gui::RenderTextureViewer()
    {
        if (ImGui::Begin("Texture Viewer", &_showTextureViewer, FloatingWindowFlags))
        {
            constexpr ImVec2 imageSize(512, 512);
            int32_t target = GL_NONE;
            if (!glIsTexture(_textureSliceViewerSelectedTexture))
            {
                ImGui::InvisibleButton("Texture", imageSize);
            } else
            {
                glGetTextureParameteriv(_textureSliceViewerSelectedTexture, GL_TEXTURE_TARGET, &target);
                if (target == GL_TEXTURE_2D_ARRAY)
                {
                    int32_t layerCount = 0;
                    glGetTextureLevelParameteriv(_textureSliceViewerSelectedTexture, 0, GL_TEXTURE_DEPTH, &layerCount);
                    _textureSliceViewerLayerIndex = std::clamp(_textureSliceViewerLayerIndex, 0, layerCount - 1);
                    _textureArraySliceViewer->ShowLayer(_textureSliceViewerSelectedTexture,
                                                        _textureSliceViewerLayerIndex, imageSize);
                } else if (target == GL_TEXTURE_2D)
                {
                    ImGui::Image(
                        reinterpret_cast<ImTextureID>(reinterpret_cast<void*>(static_cast<intptr_t>(
                            _textureSliceViewerSelectedTexture))),
                        imageSize);
                } else
                {
                    ImGui::InvisibleButton("Texture", imageSize);
                }
            }
            ImGui::InputInt("Texture ID", &_textureSliceViewerSelectedTexture);
            _textureSliceViewerSelectedTexture = std::max(_textureSliceViewerSelectedTexture, 0);

            if (target == GL_TEXTURE_2D_ARRAY)
            {
                ImGui::InputInt("Layer", &_textureSliceViewerLayerIndex);
            }
        }
        ImGui::End();
    }
}
