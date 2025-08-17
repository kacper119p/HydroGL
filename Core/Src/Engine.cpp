#include "Engine.h"

#include <spdlog/spdlog.h>

#include "Rendering/Renderer.h"

namespace Core
{
    std::unique_ptr<Platform::Window> Engine::_window = nullptr;
    std::unique_ptr<Scene> Engine::_scene = nullptr;

    int32_t Engine::Initialize(Platform::Window* Window)
    {
        _window = static_cast<std::unique_ptr<Platform::Window>>(Window);
        _scene = std::make_unique<Scene>();
        Renderer::Initialize(glm::uvec2(Window->GetWindowWidth(), Window->GetWindowHeight()));
        spdlog::info("Engine Started.");
        return 0;
    }

    void Engine::Shutdown()
    {
        spdlog::info("Engine Shutdown.");
        _window->Close();
    }
}
