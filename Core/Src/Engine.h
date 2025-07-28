#pragma once
#include <memory>

#include "Window.h"

namespace Core
{
    class Engine
    {
    private:
        static std::unique_ptr<Platform::Window> Window;

    public:
        static int Start(Platform::Window* Window);

        static void Shutdown();
    };
}
