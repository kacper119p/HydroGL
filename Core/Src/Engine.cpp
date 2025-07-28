#include "Engine.h"

#include <spdlog/spdlog.h>

namespace Core
{
    std::unique_ptr<Platform::Window> Engine::Window = nullptr;

    int Engine::Start(Platform::Window* Window)
    {
        Engine::Window = static_cast<std::unique_ptr<Platform::Window>>(Window);
        spdlog::info("Engine Started.");
        return 0;
    }

    void Engine::Shutdown()
    {
        spdlog::info("Engine Shutdown.");
        Window->Close();
    }
}
