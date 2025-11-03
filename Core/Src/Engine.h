#pragma once

#include "Window.h"
#include "Ocean/BuoyancySystem.h"
#include "Ocean/WaterSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/Renderer.h"
#include "Rendering/Materials/MaterialManager.h"
#include "Scene/Scene.h"

namespace Core
{
    class Engine
    {
    private:
        std::unique_ptr<Platform::Window> _window;
        std::unique_ptr<Scene> _scene;
        std::unique_ptr<Renderer> _renderer;
        std::unique_ptr<MaterialManager> _materialManager;
        std::unique_ptr<WaterSystem> _waterSystem;
        std::unique_ptr<BuoyancySystem> _buoyancySystem;
        std::unique_ptr<PhysicsSystem> _physicsSystem;

    public:
        [[nodiscard]] Scene* GetScene() const
        {
            return _scene.get();
        }

        [[nodiscard]] Platform::Window* GetWindow() const
        {
            return _window.get();
        }

        [[nodiscard]] Renderer* GetRenderer() const
        {
            return _renderer.get();
        }

        [[nodiscard]] MaterialManager* GetMaterialManager() const
        {
            return _materialManager.get();
        }

        [[nodiscard]] BuoyancySystem* GetBuoyancySystem() const
        {
            return _buoyancySystem.get();
        }

        [[nodiscard]] WaterSystem* GetWaterSystem() const
        {
            return _waterSystem.get();
        }

        [[nodiscard]] PhysicsSystem* GetPhysicsSystem() const
        {
            return _physicsSystem.get();
        }

    public:
        explicit Engine(Platform::Window* Window);

    public:
        ~Engine();
    };
}
