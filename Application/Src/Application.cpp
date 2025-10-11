#include "Application.h"

#include "Engine.h"
#include "TimeManager.h"
#include "Camera/CameraComponent.h"
#include "Meshes/MeshManager.h"
#include "Rendering/Renderer.h"
#include "Rendering/Components/RenderedMeshComponent.h"
#include "Rendering/Shaders/ShaderBuilder.h"
#include "Rendering/Textures/Texture.h"
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

    _gui = std::make_unique<Gui>(*Window);

    std::unique_ptr<Core::Engine> engine = std::make_unique<Core::Engine>(Window);

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

    const Core::Texture baseMap = Core::Texture::GetTexture("Assets/Textures/Box/BaseMap.dds");
    const Core::Texture emissiveMap = Core::Texture::GetTexture("Assets/Textures/Box/EmissiveMap.dds");
    const Core::Texture normalMap = Core::Texture::GetTexture("Assets/Textures/Box/NormalMap.dds");
    const Core::Texture rmaoMap = Core::Texture::GetTexture("Assets/Textures/Box/RmaoMap.dds");

    const Core::MaterialId material = engine->GetMaterialManager()->CreateMaterial(
        glm::vec3(1.0f), 1.0f, glm::vec3(0.0f), 0.0f,
        baseMap, rmaoMap, normalMap, emissiveMap);

    Core::TransformComponent& meshTransform = entityRegistry->emplace<Core::TransformComponent>(meshEntity);
    Core::RenderedMeshComponent& renderedMesh = entityRegistry->emplace<Core::RenderedMeshComponent>(meshEntity);

    renderedMesh.Mesh = Core::MeshManager::GetMesh("Assets/Meshes/Box.fbx");
    renderedMesh.Material = material;
    meshTransform.SetWorldPosition(glm::vec3(-1.0f, 0.0f, 0.0f));

    const entt::entity meshEntity2 = entityRegistry->create();

    Core::TransformComponent& meshTransform2 = entityRegistry->emplace<Core::TransformComponent>(meshEntity2);
    Core::RenderedMeshComponent& renderedMesh2 = entityRegistry->emplace<Core::RenderedMeshComponent>(meshEntity2);

    renderedMesh2.Mesh = Core::MeshManager::GetMesh("Assets/Meshes/Box.fbx");
    renderedMesh2.Material = material;
    meshTransform2.SetWorldPosition(glm::vec3(1.0f, 0.0f, 0.0f));


    const entt::entity directionalLightEntity = entityRegistry->create();
    Core::TransformComponent& directionalLightTransform = entityRegistry->emplace<Core::TransformComponent>(
        directionalLightEntity);
    Core::DirectionalLightComponent& directionalLight = entityRegistry->emplace<Core::DirectionalLightComponent>(
        directionalLightEntity);
    directionalLight.Color = glm::vec3(0.0f, 0.0f, 0.0f);
    directionalLightTransform.SetLocalEulerAngles(glm::vec3(90.0f, 0.0f, 0.0f));

    float lastTime = Platform::TimeManager::GetTime();
    while (!Window->ShouldClose())
    {
        const float currentTime = Platform::TimeManager::GetTime();
        const float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        Window->PoolEvents();
        _cameraManager->Update(*scene, deltaTime);

        engine->GetRenderer()->RenderFrame(*engine, deltaTime);

        _gui->Render();

        Window->SwapBuffers();
    }

    return 0;
}
