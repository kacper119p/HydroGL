#pragma once
#include <memory>

#include "Window.h"

namespace Core
{
    class Engine
    {
    private:
        static std::unique_ptr<Platform::Window> _window;

    public:
        static int Start(Platform::Window* Window);

        static void Shutdown();
    };
}
