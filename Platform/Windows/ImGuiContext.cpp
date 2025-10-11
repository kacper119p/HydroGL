#include "ImGuiContext.h"

#include <imgui.h>
#include <ImguiBackends/imgui_impl_opengl3.h>
#include <ImguiBackends/imgui_impl_win32.h>

namespace Platform
{
    void ImGuiContext::InitializeImGui(const Window& window)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        constexpr ImGuiConfigFlags ioConfigFlags =
                ImGuiConfigFlags_NavEnableKeyboard
                | ImGuiConfigFlags_DockingEnable;

        ImGui::GetIO().ConfigFlags |= ioConfigFlags;
        ImGui::StyleColorsDark();

        ImGui_ImplWin32_Init(window._windowHandle);
        ImGui_ImplOpenGL3_Init("#version 460");
    }

    void ImGuiContext::ImGuiBeginFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiContext::ImGuiEndFrame()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


        const ImGuiIO& io = ImGui::GetIO();
    }

    void ImGuiContext::ImGuiShutdown()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }
}
