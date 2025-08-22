#include "Application.h"

#include <chrono>

#include "Engine.h"
#include "Camera/CameraComponent.h"
#include "Meshes/MeshManager.h"
#include "Rendering/Renderer.h"
#include "Rendering/Components/RenderedMeshComponent.h"
#include "Rendering/Shaders/Shader.h"
#include "Rendering/Shaders/ShaderBuilder.h"
#include "Scene/TransformComponent.h"

int32_t ApplicationMain(Platform::Window* const Window)
{
    const std::unique_ptr<Application::Application> application = std::make_unique<Application::Application>();
    application->Run(Window);
    return 0;
}

int32_t Application::Application::Run(Platform::Window* Window)
{
    Window->MakeContextCurrent();

    std::unique_ptr<Core::Engine> engine = std::make_unique<Core::Engine>(Window);

    std::unique_ptr<Core::MaterialManager> materialManager = std::make_unique<Core::MaterialManager>();

    _cameraManager = std::make_unique<CameraManager>(engine.get());

    Core::Scene* scene = engine->GetScene();

    scene->SetSkybox(Core::Skybox::CreateSkybox("Assets/Textures/Skyboxes/Skybox.dds"));

    entt::registry* entityRegistry = scene->GetEntityRegistry();

    const entt::entity cameraEntity = entityRegistry->create();

    Core::TransformComponent& cameraTransform = entityRegistry->emplace<Core::TransformComponent>(cameraEntity);
    Core::CameraComponent& cameraComponent = entityRegistry->emplace<Core::CameraComponent>(cameraEntity);

    cameraComponent.ProjectionMatrix = glm::perspective(glm::radians(70.0f),
                                                        static_cast<float>(Window->GetWindowWidth())
                                                        / static_cast<float>(Window->GetWindowHeight()),
                                                        0.1f, 1000.0f);

    const entt::entity meshEntity = entityRegistry->create();

    Core::TransformComponent& meshTransform = entityRegistry->emplace<Core::TransformComponent>(meshEntity);
    Core::RenderedMeshComponent& renderedMesh = entityRegistry->emplace<Core::RenderedMeshComponent>(meshEntity);

    renderedMesh.Mesh = Core::MeshManager::GetMesh("Assets/Meshes/Torch.fbx");

    auto lastTime = std::chrono::high_resolution_clock::now();
    while (!Window->ShouldClose())
    {
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> delta = currentTime - lastTime;
        lastTime = currentTime;
        float deltaTime = delta.count();

        Window->PoolEvents();
        _cameraManager->Update(*scene, deltaTime);

        engine->GetRenderer()->RenderFrame(*scene, *materialManager.get());
        Window->SwapBuffers();
    }

    return 0;
}
