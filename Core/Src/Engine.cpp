#include "Engine.h"

#include <spdlog/spdlog.h>

namespace Core
{
    std::unique_ptr<Platform::Window> Engine::_window = nullptr;

    int Engine::Start(Platform::Window* Window)
    {
        Engine::_window = static_cast<std::unique_ptr<Platform::Window>>(Window);
        spdlog::info("Engine Started.");
        return 0;
    }

    void Engine::Shutdown()
    {
        spdlog::info("Engine Shutdown.");
        _window->Close();
    }
} // namespace Core
