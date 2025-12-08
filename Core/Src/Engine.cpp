#include "Engine.h"

#include <spdlog/spdlog.h>

#include "Meshes/MeshManager.h"
#include "Rendering/Renderer.h"

namespace Core
{
    Engine::Engine(Platform::Window* Window) : _window(Window),
                                               _scene(std::make_unique<Scene>()),
                                               _renderer(std::make_unique<Renderer>(
                                                   glm::uvec2(Window->GetWindowWidth(), Window->GetWindowHeight()))),
                                               _materialManager(std::make_unique<MaterialManager>()),
                                               _waterSystem(std::make_unique<WaterSystem>(
                                                   glm::ivec2(Window->GetWindowWidth(),
                                                              Window->GetWindowHeight()),
                                                   _renderer->GetFrameBuffer().GetTransparentDepthBuffer())),
                                               _waterInteractionSystem(std::make_unique<WaterInteractionSystem>()),
                                               _physicsSystem(std::make_unique<PhysicsSystem>())
    {
        MeshManager::Initialize();

        spdlog::info("Engine Started.");
    }

    Engine::~Engine()
    {
        spdlog::info("Engine Shutdown.");

        MeshManager::FreeResources();
    }
}
