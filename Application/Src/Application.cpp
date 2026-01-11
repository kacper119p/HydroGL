#include "Application.h"

#include "Engine.h"
#include "TimeManager.h"
#include "Camera/CameraComponent.h"
#include "Meshes/MeshManager.h"
#include "Meshes/MeshUtility.h"
#include "Ocean/Components/WaterEffectorComponent.h"
#include "Physics/MeshColliderFactory.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/Renderer.h"
#include "Rendering/Components/RenderedMeshComponent.h"
#include "Rendering/Textures/Texture.h"
#include "Scene/TransformComponent.h"

namespace
{
    void CreateIslands(entt::registry* const EntityRegistry, Core::MaterialManager* const MaterialManager,
                       const Core::PhysicsSystem* const PhysicsSystem)
    {
        Core::Mesh mesh = Core::MeshManager::GetMesh("Assets/Models/Island/Island.glb");
        const Core::Texture colorMain = Core::Texture::GetTexture("Assets/Models/Island/Color.dds");
        const Core::Texture normal = Core::Texture::GetTexture("Assets/Textures/NormalDefault.dds");
        const Core::Texture rmao = Core::Texture::GetTexture("Assets/Models/Island/Rmao.dds");
        const Core::Texture emissive = Core::Texture::GetTexture("Assets/Textures/Black.dds");
        const Core::MaterialId materialMain = MaterialManager->CreateMaterial(
            glm::vec3(1.0f), 1.0f, glm::vec3(0.0f), 0.0f,
            colorMain, rmao, normal, emissive);

        const entt::entity entity = EntityRegistry->create();
        Core::TransformComponent& transform = EntityRegistry->emplace<Core::TransformComponent>(entity);
        Core::RenderedMeshComponent& renderedMesh = EntityRegistry->emplace<
            Core::RenderedMeshComponent>(entity);
        renderedMesh.Mesh = mesh;
        renderedMesh.Material = materialMain;
        transform.SetWorldPosition(glm::vec3(3000.0f, 0.0f, -3000.0f));

        const entt::entity entity2 = EntityRegistry->create();
        Core::TransformComponent& transform2 = EntityRegistry->emplace<Core::TransformComponent>(entity2);
        Core::RenderedMeshComponent& renderedMesh2 = EntityRegistry->emplace<
            Core::RenderedMeshComponent>(entity2);
        renderedMesh2.Mesh = mesh;
        renderedMesh2.Material = materialMain;
        transform2.SetWorldPosition(glm::vec3(-2000.0f, 0.0f, -3000.0f));
        transform2.SetWorldEulerAngles(glm::vec3(0.0f, 90.0f, 0.0f));
    }

    void CreateFishingShip(entt::registry* const EntityRegistry, Core::MaterialManager* const MaterialManager,
                           const Core::PhysicsSystem* const PhysicsSystem)
    {
        const Core::Texture black = Core::Texture::GetTexture("Assets/Textures/Black.dds");
        const Core::Texture white = Core::Texture::GetTexture("Assets/Textures/White.dds");
        const Core::Texture normal = Core::Texture::GetTexture("Assets/Textures/NormalDefault.dds");

        const entt::entity entityMain = EntityRegistry->create();
        Core::TransformComponent& transformMain = EntityRegistry->emplace<Core::TransformComponent>(entityMain);
        Core::RenderedMeshComponent& renderedMeshMain = EntityRegistry->emplace<
            Core::RenderedMeshComponent>(entityMain);
        renderedMeshMain.Mesh = Core::MeshManager::GetMesh("Assets/Models/FishingShip/Main.glb");
        const Core::Texture colorMain = Core::Texture::GetTexture("Assets/Models/FishingShip/ColorMain.dds");
        const Core::Texture normalMain = Core::Texture::GetTexture("Assets/Models/FishingShip/NormalMain.dds");
        const Core::Texture rmaoMain = Core::Texture::GetTexture("Assets/Models/FishingShip/RmaoMain.dds");
        const Core::MaterialId materialMain = MaterialManager->CreateMaterial(
            glm::vec3(1.0f), 1.0f, glm::vec3(0.0f), 1.0f,
            colorMain, rmaoMain, normalMain, black);
        renderedMeshMain.Material = materialMain;

        const entt::entity entityInterior = EntityRegistry->create();
        const Core::TransformComponent& transformInterior = EntityRegistry->emplace<Core::TransformComponent>(
            entityInterior);
        Core::RenderedMeshComponent& renderedMeshInterior = EntityRegistry->emplace<
            Core::RenderedMeshComponent>(entityInterior);
        transformInterior.GetSceneHierarchyNode().SetParent(transformMain.GetSceneHierarchyNode());
        renderedMeshInterior.Mesh = Core::MeshManager::GetMesh("Assets/Models/FishingShip/Interior.glb");
        const Core::Texture colorInterior = Core::Texture::GetTexture("Assets/Models/FishingShip/ColorInterior.dds");
        const Core::Texture normalInterior = Core::Texture::GetTexture("Assets/Models/FishingShip/NormalInterior.dds");
        const Core::Texture rmaoInterior = Core::Texture::GetTexture("Assets/Models/FishingShip/RmaoInterior.dds");
        const Core::MaterialId materialInterior = MaterialManager->CreateMaterial(
            glm::vec3(1.0f), 1.0f, glm::vec3(0.0f), 1.0f,
            colorInterior, rmaoInterior, normalInterior, black);
        renderedMeshInterior.Material = materialInterior;

        const entt::entity entityLight = EntityRegistry->create();
        const Core::TransformComponent& transformLight = EntityRegistry->emplace<Core::TransformComponent>(entityLight);
        Core::RenderedMeshComponent& renderedMeshLight = EntityRegistry->emplace<Core::RenderedMeshComponent>(
            entityLight);
        transformLight.GetSceneHierarchyNode().SetParent(transformMain.GetSceneHierarchyNode());
        renderedMeshLight.Mesh = Core::MeshManager::GetMesh("Assets/Models/FishingShip/Light.glb");
        const Core::MaterialId materialLight = MaterialManager->CreateMaterial(
            glm::vec3(0.0f), 0.0f, glm::vec3(15.0f), 0.0f,
            black, black, normal, white);
        renderedMeshLight.Material = materialLight;

        const Core::Mesh colliderMesh = Core::MeshManager::GetMesh("Assets/Models/FishingShip/Collider.glb");

        transformMain.SetLocalPosition(glm::vec3(-20.0f, 0.0f, 0.0f));
        EntityRegistry->emplace<Core::WaterEffectorComponent>(
            entityMain, colliderMesh, transformMain.GetWorldPosition(), 0.25f, -0.1f, 0.05f, 512);
        JPH::RefConst<JPH::Shape> colliderShape = Core::MeshColliderFactory::CreateConvexHullCollider(
            colliderMesh, 300.0f);
        Core::RigidbodyComponent rigidBodyComponent = PhysicsSystem->CreateRigidbodyComponent(
            transformMain, colliderShape, JPH::EMotionType::Dynamic, JPH::EActivation::Activate,
            Core::Physics::Layers::Dynamic);

        EntityRegistry->emplace<Core::RigidbodyComponent>(entityMain, rigidBodyComponent);
    }

    void CreateCargoShip(entt::registry* const EntityRegistry, Core::MaterialManager* const MaterialManager,
                         const Core::PhysicsSystem* const PhysicsSystem)
    {
        const Core::Texture emissive = Core::Texture::GetTexture("Assets/Textures/Black.dds");

        const entt::entity entityMain = EntityRegistry->create();
        Core::TransformComponent& transformMain = EntityRegistry->emplace<Core::TransformComponent>(entityMain);
        Core::RenderedMeshComponent& renderedMeshMain = EntityRegistry->emplace<
            Core::RenderedMeshComponent>(entityMain);
        renderedMeshMain.Mesh = Core::MeshManager::GetMesh("Assets/Models/CargoShip/CargoShipMain.glb");
        const Core::Texture colorMain = Core::Texture::GetTexture("Assets/Models/CargoShip/ColorMain.dds");
        const Core::Texture normalMain = Core::Texture::GetTexture("Assets/Models/CargoShip/NormalMain.dds");
        const Core::Texture rmaoMain = Core::Texture::GetTexture("Assets/Models/CargoShip/RmaoMain.dds");
        const Core::MaterialId materialMain = MaterialManager->CreateMaterial(
            glm::vec3(1.0f), 1.0f, glm::vec3(0.0f), 1.0f,
            colorMain, rmaoMain, normalMain, emissive);
        renderedMeshMain.Material = materialMain;

        const entt::entity entityA = EntityRegistry->create();
        const Core::TransformComponent& transformA = EntityRegistry->emplace<Core::TransformComponent>(entityA);
        Core::RenderedMeshComponent& renderedMeshA = EntityRegistry->emplace<
            Core::RenderedMeshComponent>(entityA);
        transformA.GetSceneHierarchyNode().SetParent(transformMain.GetSceneHierarchyNode());
        renderedMeshA.Mesh = Core::MeshManager::GetMesh("Assets/Models/CargoShip/CargoShipA.glb");
        const Core::Texture colorA = Core::Texture::GetTexture("Assets/Models/CargoShip/ColorA.dds");
        const Core::Texture normalA = Core::Texture::GetTexture("Assets/Models/CargoShip/NormalA.dds");
        const Core::Texture rmaoA = Core::Texture::GetTexture("Assets/Models/CargoShip/RmaoA.dds");
        const Core::MaterialId materialA = MaterialManager->CreateMaterial(
            glm::vec3(1.0f), 1.0f, glm::vec3(0.0f), 1.0f,
            colorA, rmaoA, normalA, emissive);
        renderedMeshA.Material = materialA;

        const entt::entity entityB = EntityRegistry->create();
        const Core::TransformComponent& transformB = EntityRegistry->emplace<Core::TransformComponent>(entityB);
        Core::RenderedMeshComponent& renderedMeshB = EntityRegistry->emplace<Core::RenderedMeshComponent>(entityB);
        transformB.GetSceneHierarchyNode().SetParent(transformMain.GetSceneHierarchyNode());
        renderedMeshB.Mesh = Core::MeshManager::GetMesh("Assets/Models/CargoShip/CargoShipB.glb");
        const Core::Texture colorB = Core::Texture::GetTexture("Assets/Models/CargoShip/ColorB.dds");
        const Core::Texture normalB = Core::Texture::GetTexture("Assets/Models/CargoShip/NormalB.dds");
        const Core::Texture rmaoB = Core::Texture::GetTexture("Assets/Models/CargoShip/RmaoB.dds");
        const Core::MaterialId materialB = MaterialManager->CreateMaterial(
            glm::vec3(1.0f), 1.0f, glm::vec3(0.0f), 1.0f,
            colorB, rmaoB, normalB, emissive);
        renderedMeshB.Material = materialB;

        const Core::Mesh colliderMesh = Core::MeshManager::GetMesh("Assets/Models/CargoShip/CargoShipCollider.glb");

        transformMain.SetLocalPosition(glm::vec3(0.0f, -2.0f, 0.0f));
        EntityRegistry->emplace<Core::WaterEffectorComponent>(
            entityMain, colliderMesh, transformMain.GetWorldPosition(), 0.03f, 0.05f, 0.15f, 512);
        JPH::RefConst<JPH::Shape> colliderShape = Core::MeshColliderFactory::CreateConvexHullCollider(
            colliderMesh, 300.0f);
        Core::RigidbodyComponent rigidBodyComponent = PhysicsSystem->CreateRigidbodyComponent(
            transformMain, colliderShape, JPH::EMotionType::Dynamic, JPH::EActivation::Activate,
            Core::Physics::Layers::Dynamic);

        EntityRegistry->emplace<Core::RigidbodyComponent>(entityMain, rigidBodyComponent);
    }

    void CreateBoxes(entt::registry* EntityRegistry, Core::MaterialManager* MaterialManager,
                     Core::PhysicsSystem* PhysicsSystem)
    {
        const Core::Mesh cubeMesh = Core::MeshManager::GetMesh("Assets/Meshes/Cube.fbx");
        const float cubeVolume = Core::MeshUtility::CalculateMeshVolume(cubeMesh);
        JPH::RefConst<JPH::Shape> colliderShape = Core::MeshColliderFactory::CreateConvexHullCollider(
            cubeMesh, 750.0f);
        const Core::Texture baseMap = Core::Texture::GetTexture("Assets/Textures/Box/BaseMap.dds");
        const Core::Texture emissiveMap = Core::Texture::GetTexture("Assets/Textures/Box/EmissiveMap.dds");
        const Core::Texture normalMap = Core::Texture::GetTexture("Assets/Textures/Box/NormalMap.dds");
        const Core::Texture rmaoMap = Core::Texture::GetTexture("Assets/Textures/Box/RmaoMap.dds");

        const Core::MaterialId material = MaterialManager->CreateMaterial(
            glm::vec3(1.0f), 1.0f, glm::vec3(0.0f), 0.0f,
            baseMap, rmaoMap, normalMap, emissiveMap);

        Core::Mesh mesh = Core::MeshManager::GetMesh("Assets/Meshes/Box.fbx");

        for (int32_t i = -5; i <= 5; ++i)
        {
            for (int32_t j = -5; j <= 5; ++j)
            {
                const entt::entity boxEntity = EntityRegistry->create();
                const glm::vec3 position(static_cast<float>(i) * 3.0f, 0.0f,
                                         static_cast<float>(j) * 3.0f);
                Core::TransformComponent& boxTransform = EntityRegistry->emplace<Core::TransformComponent>(boxEntity);
                Core::RenderedMeshComponent& boxRenderedMesh = EntityRegistry->emplace<Core::RenderedMeshComponent>(
                    boxEntity);
                EntityRegistry->emplace<Core::WaterEffectorComponent>(
                    boxEntity, cubeMesh, position, cubeVolume, 0.25f, -0.1f, 0.05f, 512);
                boxRenderedMesh.Mesh = mesh;
                boxRenderedMesh.Material = material;
                boxTransform.SetWorldPosition(position);
                Core::RigidbodyComponent boxRigidbodyComponent = PhysicsSystem->CreateRigidbodyComponent(
                    boxTransform, colliderShape, JPH::EMotionType::Dynamic, JPH::EActivation::Activate,
                    Core::Physics::Layers::Dynamic);

                EntityRegistry->emplace<Core::RigidbodyComponent>(boxEntity, boxRigidbodyComponent);
            }
        }
    }
}

int32_t Application::Application::Run(Platform::Window* Window)
{
    Window->MakeContextCurrent();

    std::unique_ptr<Core::Engine> engine = std::make_unique<Core::Engine>(Window);
    _cameraManager = std::make_unique<CameraManager>(engine.get());
    _gui = std::make_unique<Gui>(*Window, *engine->GetWaterSystem(), *_cameraManager);

    Core::Scene* scene = engine->GetScene();

    scene->SetSkybox(Core::Skybox::CreateSkybox("Assets/Textures/Skyboxes/Skybox.dds",
                                                "Assets/Textures/Skyboxes/SkyboxNoSunDisc.dds"));

    entt::registry* entityRegistry = scene->GetEntityRegistry();

    const entt::entity cameraEntity = entityRegistry->create();

    const Core::TransformComponent& cameraTransform = entityRegistry->emplace<Core::TransformComponent>(cameraEntity);
    const Core::CameraComponent& cameraComponent = entityRegistry->emplace<Core::CameraComponent>(
        cameraEntity, 70.0f,
        static_cast<float>(Window->GetWindowWidth())
        / static_cast<float>(Window->GetWindowHeight()),
        0.1f, 28284.2712f);

    CreateCargoShip(entityRegistry, engine->GetMaterialManager(), engine->GetPhysicsSystem());
    CreateFishingShip(entityRegistry, engine->GetMaterialManager(), engine->GetPhysicsSystem());
    CreateIslands(entityRegistry, engine->GetMaterialManager(), engine->GetPhysicsSystem());

    const entt::entity directionalLightEntity = entityRegistry->create();
    Core::TransformComponent& directionalLightTransform = entityRegistry->emplace<Core::TransformComponent>(
        directionalLightEntity);
    Core::DirectionalLightComponent& directionalLight = entityRegistry->emplace<Core::DirectionalLightComponent>(
        directionalLightEntity);

    // Matches Assets/Textures/Skyboxes/Skybox.dds
    directionalLight.Color = glm::vec3(7.34375, 2.43750f, 0.75000f);
    directionalLightTransform.SetLocalEulerAngles(glm::vec3(3.875f, 233.5f, 0.0f));

    float physicsAccumulatedTime = 0.0f;
    float lastTime = Platform::TimeManager::GetTime();
    while (!Window->ShouldClose())
    {
        const float currentTime = Platform::TimeManager::GetTime();
        const float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        Window->PoolEvents();

        physicsAccumulatedTime += deltaTime;
        while (physicsAccumulatedTime >= PhysicsTimeStep)
        {
            engine->GetWaterInteractionSystem()->ProcessMeshes(*engine->GetScene(), *engine->GetPhysicsSystem(),
                                                               engine->GetWaterSystem()->GetDisplacementTextures(),
                                                               engine->GetWaterSystem()->GetWindVelocity(),
                                                               engine->GetWaterSystem()->GetVelocityTextures(),
                                                               engine->GetWaterSystem()->GetCascadeSettingsSsbo(),
                                                               PhysicsTimeStep);
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
