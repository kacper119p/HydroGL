#pragma once
#include <memory>

#include "Window.h"
#include "Scene/Scene.h"

namespace Core
{
    class Engine
    {
    private:
        static std::unique_ptr<Platform::Window> _window;
        static std::unique_ptr<Scene> _scene;

    public:
        static Scene* GetScene()
        {
            return _scene.get();
        }

    public:
        static int32_t Initialize(Platform::Window* Window);

        static void Shutdown();
    };
}
