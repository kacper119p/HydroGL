#pragma once

#include "Window.h"
#include "Ocean/WaterSystem.h"
#include "Rendering/Renderer.h"
#include "Rendering/Materials/MaterialManager.h"
#include "Scene/Scene.h"

namespace Core
{
    class Engine
    {
    private:
        Platform::Window* _window;
        Scene* _scene;
        Renderer* _renderer;
        MaterialManager* _materialManager;
        WaterSystem* _waterSystem;

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

        [[nodiscard]] MaterialManager* GetMaterialManager() const
        {
            return _materialManager;
        }

        [[nodiscard]] WaterSystem* GetWaterSystem() const
        {
            return _waterSystem;
        }

    public:
        explicit Engine(Platform::Window* Window);

    public:
        ~Engine();
    };
}
