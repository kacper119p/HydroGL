#include "SystemInfoWindow.h"

#include <cstdint>
#include <format>
#include <imgui.h>
#include <string>

#include "SystemInfo.h"
#include "glad/glad.h"

namespace Application
{
    void SystemInfoWindow::Render(bool& ShowSystemInfo, const uint32_t WindowWidth, const uint32_t WindowHeight)
    {
        constexpr ImGuiWindowFlags windowFlags =
                ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoResize;

        if (ImGui::Begin("System Info", &ShowSystemInfo, windowFlags))
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
            ImGui::Text("Resolution: %dx%d", WindowWidth, WindowHeight);
            ImGui::Separator();
            ImGui::Text("Application average: %.3f ms/frame\n%.1f FPS",
                        1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }
        ImGui::End();
    }
}
