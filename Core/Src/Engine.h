#pragma once
#include <memory>

#include "Window.h"
#include "Rendering/Renderer.h"
#include "Scene/Scene.h"

namespace Core
{
    class Engine
    {
    private:
        Platform::Window* _window;
        Scene* _scene;
        Renderer* _renderer;

    public:
        [[nodiscard]] Scene* GetScene() const
        {
            return _scene;
        }

        [[nodiscard]] Platform::Window* GetWindow() const
        {
            return _window;
        }

        [[nodiscard]] Renderer* GetRenderer() const
        {
            return _renderer;
        }

        explicit Engine(Platform::Window* Window);

    public:
        ~Engine();
    };
}
