#include "Application.h"

#include <iostream>

#include "Engine.h"
#include "Camera/CameraComponent.h"
#include "Rendering/Renderer.h"
#include "Rendering/Shaders/Shader.h"
#include "Rendering/Shaders/ShaderBuilder.h"
#include "Scene/TransformComponent.h"

int ApplicationMain(Platform::Window* const Window)
{
    Core::Engine::Initialize(Window);
    Core::ShaderBuilder shaderBuilder;
    Core::Shader shader = shaderBuilder.SetStage("Assets/Shaders/PBR/PBR.vert", Core::ShaderStage::VertexShader)
            .SetStage("Assets/Shaders/PBR/PBR.frag", Core::ShaderStage::FragmentShader)
            .Build();

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
    while (!Window->ShouldClose())
    {
        Core::Renderer::RenderFrame(*scene);
        Window->SwapBuffers();
    }
    Core::Engine::Shutdown();
    return 0;
}
