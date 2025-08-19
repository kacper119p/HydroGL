#include "Application.h"

#include <chrono>

#include "Engine.h"
#include "Camera/CameraComponent.h"
#include "Rendering/Renderer.h"
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
    Core::Engine::Initialize(Window);
    Core::ShaderBuilder shaderBuilder;
    Core::Shader shader = shaderBuilder.SetStage("Assets/Shaders/PBR/PBR.vert", Core::ShaderStage::VertexShader)
            .SetStage("Assets/Shaders/PBR/PBR.frag", Core::ShaderStage::FragmentShader)
            .Build();

    _cameraManager = std::make_unique<CameraManager>();

    Core::Scene* scene = Core::Engine::GetScene();

    scene->SetSkybox(Core::Skybox::CreateSkybox("Assets/Textures/Skyboxes/Skybox.dds"));

    entt::registry* entityRegistry = scene->GetEntityRegistry();

    const entt::entity cameraEntity = entityRegistry->create();

    Core::TransformComponent& cameraTransform = entityRegistry->emplace<Core::TransformComponent>(cameraEntity);
    Core::CameraComponent& cameraComponent = entityRegistry->emplace<Core::CameraComponent>(cameraEntity);

    cameraComponent.ProjectionMatrix = glm::perspective(glm::radians(70.0f),
                                                        static_cast<float>(Window->GetWindowWidth())
                                                        / static_cast<float>(Window->GetWindowHeight()),
                                                        0.1f, 1000.0f);
    Window->MakeContextCurrent();

    auto lastTime = std::chrono::high_resolution_clock::now();
    while (!Window->ShouldClose())
    {
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> delta = currentTime - lastTime;
        lastTime = currentTime;
        float deltaTime = delta.count();

        Window->PoolEvents();
        _cameraManager->Update(*scene, deltaTime);

        Core::Renderer::RenderFrame(*scene);
        Window->SwapBuffers();
    }
    Core::Engine::Shutdown();
    return 0;
}
