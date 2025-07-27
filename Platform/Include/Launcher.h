#pragma once
#include "Window.h"

extern int ApplicationMain(Platform::Window* Window);

namespace Platform
{
    class Launcher
    {
    private:
        Launcher() = default;

    public:
        Launcher(const Launcher&) = delete;

        Launcher& operator=(const Launcher&) = delete;

    public:
        static int Start(HINSTANCE HInstance);
    };
}
