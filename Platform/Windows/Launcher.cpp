#include "Launcher.h"

#include <windows.h>

#include "TimeManager.h"

namespace Platform
{
    int Launcher::Start(HINSTANCE HInstance)
    {
        TimeManager::Initialize();
        Window* window = Window::Create(HInstance);
        if (window == nullptr)
        {
            return -1;
        }
        return ApplicationMain(window);
    }
}

int WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    return Platform::Launcher::Start(hInstance);
}

int main(int, char**)
{
    return Platform::Launcher::Start(nullptr);
}
