#include "SystemInfoWindow.h"

#include <cstdint>
#include <format>
#include <imgui.h>
#include <string>

#include "SystemInfo.h"
#include "glad/glad.h"

namespace Application
{
    void SystemInfoWindow::Render(bool& ShowSystemInfo)
    {
        constexpr ImGuiWindowFlags windowFlags =
                ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoResize;

        if (ImGui::Begin("System Info", &ShowSystemInfo, windowFlags))
        {
            const GLubyte* renderer = glGetString(GL_RENDERER);
            const std::string cpuInfo = Platform::SystemInfo::GetCpuInfo();
            const int64_t ramGB = Platform::SystemInfo::GetTotalRamKB() / 1048576ull;
            const GLubyte* version = glGetString(GL_VERSION);
            const GLubyte* shadingLanguageVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);

            ImGui::Text("GPU: %s", reinterpret_cast<const char*>(renderer));
            ImGui::Text("CPU: %s", cpuInfo.c_str());
            ImGui::Text("RAM: %dGB", ramGB);
            ImGui::Text("OpenGL version: %s", reinterpret_cast<const char*>(version));
            ImGui::Text("Shading Language version: %s", reinterpret_cast<const char*>(shadingLanguageVersion));
            ImGui::Separator();
            ImGui::Text("Application average %.3f ms/frame\n%.1f FPS",
                        1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }
        ImGui::End();
    }
}
