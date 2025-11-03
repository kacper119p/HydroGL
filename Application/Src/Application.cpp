#include "Application.h"

#include "Engine.h"
#include "TimeManager.h"
#include "Camera/CameraComponent.h"
#include "Meshes/MeshManager.h"
#include "Ocean/Components/WaterEffectorComponent.h"
#include "Physics/MeshColliderFactory.h"
#include "Physics/PhysicsSystem.h"
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

    std::unique_ptr<Core::Engine> engine = std::make_unique<Core::Engine>(Window);

    _gui = std::make_unique<Gui>(*Window, *engine->GetWaterSystem());

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

    const Core::Mesh cubeMesh = Core::MeshManager::GetMesh("Assets/Meshes/Cube.fbx");
    JPH::RefConst<JPH::Shape> colliderShape = Core::MeshColliderFactory::CreateConvexHullCollider(
        cubeMesh, 750.0f);

    const Core::Texture baseMap = Core::Texture::GetTexture("Assets/Textures/Box/BaseMap.dds");
    const Core::Texture emissiveMap = Core::Texture::GetTexture("Assets/Textures/Box/EmissiveMap.dds");
    const Core::Texture normalMap = Core::Texture::GetTexture("Assets/Textures/Box/NormalMap.dds");
    const Core::Texture rmaoMap = Core::Texture::GetTexture("Assets/Textures/Box/RmaoMap.dds");

    const Core::MaterialId material = engine->GetMaterialManager()->CreateMaterial(
        glm::vec3(1.0f), 1.0f, glm::vec3(0.0f), 0.0f,
        baseMap, rmaoMap, normalMap, emissiveMap);

    Core::Mesh mesh = Core::MeshManager::GetMesh("Assets/Meshes/Box.fbx");

    for (int32_t i = -2; i <= 2; ++i)
    {
        for (int32_t j = -2; j <= 2; ++j)
        {
            const entt::entity boxEntity = entityRegistry->create();

            Core::TransformComponent& boxTransform = entityRegistry->emplace<Core::TransformComponent>(boxEntity);
            Core::RenderedMeshComponent& boxRenderedMesh = entityRegistry->emplace<Core::RenderedMeshComponent>(
                boxEntity);
            Core::WaterEffectorComponent& boxWaterEffector = entityRegistry->emplace<Core::WaterEffectorComponent>(
                boxEntity, cubeMesh);

            boxRenderedMesh.Mesh = mesh;
            boxRenderedMesh.Material = material;
            boxTransform.SetWorldPosition(glm::vec3(static_cast<float>(i) * 3.0f, 0.0f,
                                                    static_cast<float>(j) * 3.0f));

            Core::RigidbodyComponent boxRigidbodyComponent = engine->GetPhysicsSystem()->CreateRigidbodyComponent(
                boxTransform, colliderShape, JPH::EMotionType::Dynamic, JPH::EActivation::Activate,
                Core::Physics::Layers::Dynamic);

            entityRegistry->emplace<Core::RigidbodyComponent>(boxEntity, boxRigidbodyComponent);
        }
    }

    const entt::entity directionalLightEntity = entityRegistry->create();
    Core::TransformComponent& directionalLightTransform = entityRegistry->emplace<Core::TransformComponent>(
        directionalLightEntity);
    Core::DirectionalLightComponent& directionalLight = entityRegistry->emplace<Core::DirectionalLightComponent>(
        directionalLightEntity);
    directionalLight.Color = glm::vec3(0.0f, 0.0f, 0.0f);
    directionalLightTransform.SetLocalEulerAngles(glm::vec3(90.0f, 0.0f, 0.0f));

    float physicsAccumulatedTime = 0.0f;
    float lastTime = Platform::TimeManager::GetTime();
    while (!Window->ShouldClose())
    {
        const float currentTime = Platform::TimeManager::GetTime();
        const float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        Window->PoolEvents();

        // engine->GetBuoyancySystem()->ProcessMeshes(*engine->GetScene(), *engine->GetPhysicsSystem(),
        //                                            engine->GetWaterSystem()->GetDisplacementTextures(),
        //                                            engine->GetWaterSystem()->GetCascadeSettingsSsbo());
        physicsAccumulatedTime += deltaTime;
        while (physicsAccumulatedTime >= PhysicsTimeStep)
        {
            engine->GetBuoyancySystem()->ProcessMeshes(*engine->GetScene(), *engine->GetPhysicsSystem(),
                                                       engine->GetWaterSystem()->GetDisplacementTextures(),
                                                       engine->GetWaterSystem()->GetWindVelocity(),
                                                       engine->GetWaterSystem()->GetVelocityTextures(),
                                                       engine->GetWaterSystem()->GetCascadeSettingsSsbo());
            engine->GetPhysicsSystem()->Step(*entityRegistry, PhysicsTimeStep);
            physicsAccumulatedTime -= PhysicsTimeStep;
        }

        _cameraManager->Update(*scene, deltaTime);

        engine->GetRenderer()->RenderFrame(*engine, deltaTime);

        _gui->Render();

        Window->SwapBuffers();
    }

    return 0;
}
