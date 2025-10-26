#pragma comment(lib, "Shcore.lib")

#include "Launcher.h"

#include <ShellScalingApi.h>
#include <Windows.h>

#include "Debug.h"
#include "SystemInfo.h"
#include "TimeManager.h"

namespace Platform
{
    int Launcher::Start(HINSTANCE HInstance)
    {
        TimeManager::Initialize();
        SystemInfo::Initialize();

        Window* window = Window::Create(HInstance);
        if (window == nullptr)
        {
            return -1;
        }
        return ApplicationMain(window);

        SystemInfo::Release();
    }
}

int WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    return Platform::Launcher::Start(hInstance);
}

int main(int, char**)
{
    CHECK(SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE) == S_OK);
    return Platform::Launcher::Start(nullptr);
}
