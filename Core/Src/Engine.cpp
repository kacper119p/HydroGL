#include "Engine.h"

#include <spdlog/spdlog.h>

#include "Meshes/MeshManager.h"
#include "Rendering/Renderer.h"

namespace Core
{
    Engine::Engine(Platform::Window* Window) : _window(Window),
                                               _scene(new Scene()),
                                               _renderer(new Renderer(
                                                   glm::uvec2(Window->GetWindowWidth(), Window->GetWindowHeight()))),
                                               _materialManager(new MaterialManager()),
                                               _waterSystem(new WaterSystem())
    {
        MeshManager::Initialize();

        spdlog::info("Engine Started.");
    }

    Engine::~Engine()
    {
        spdlog::info("Engine Shutdown.");
        delete _window;
        delete _scene;
        delete _renderer;
        delete _materialManager;
        delete _waterSystem;

        MeshManager::FreeResources();
    }
}
