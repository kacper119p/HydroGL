#include "PhysicsSystem.h"

#include <thread>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

#include "Components/RigidbodyComponent.h"
#include "Physics/BroadPhaseLayerInterface.h"
#include "Physics/ObjectLayerPairFilter.h"
#include "Physics/ObjectVsBroadPhaseLayerFilter.h"
#include "Scene/TransformComponent.h"

namespace
{
    constexpr size_t TempAllocatorSize = 12 * 1024 * 1024; // 10 MB
    constexpr uint32_t MaxBodies = 65536;
    constexpr uint32_t MaxObjectPairs = 65536;
    constexpr uint32_t MaxContactConstraints = 1024;
    constexpr float Gravity = 9.80665f;
}

namespace Core
{
    PhysicsSystem::PhysicsSystem()
    {
        JPH::RegisterDefaultAllocator();
        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();

        _jphTempAllocator = std::make_unique<JPH::TempAllocatorImpl>(TempAllocatorSize);

        _jphJobSystem = std::make_unique<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers,
                                                                   static_cast<int32_t>(
                                                                       std::thread::hardware_concurrency() - 1));

        _jphPhysicsSystem = std::make_unique<JPH::PhysicsSystem>();


        _jphPhysicsSystem->Init(MaxBodies, 0, MaxObjectPairs, MaxContactConstraints, broadPhaseLayerInterface,
                                objectVsBroadPhaseLayerFilter, objectVsObjectLayerFilter);
        _jphPhysicsSystem->SetGravity(JPH::Vec3(0.0f, -Gravity, 0.0f));
    }

    PhysicsSystem::~PhysicsSystem()
    {
        delete JPH::Factory::sInstance;
    }

    RigidbodyComponent PhysicsSystem::CreateRigidbodyComponent(const TransformComponent& Transform,
                                                               const JPH::RefConst<JPH::Shape>& Shape,
                                                               const JPH::EMotionType MotionType,
                                                               const JPH::EActivation Activation,
                                                               const uint32_t CollisionLayer)
    const
    {
        const glm::vec3 worldPosition = Transform.GetWorldPosition();
        const glm::quat worldRotation = Transform.GetWorldRotation();
        const JPH::BodyCreationSettings creationSettings(Shape,
                                                         JPH::RVec3(worldPosition.x, worldPosition.y, worldPosition.z),
                                                         JPH::Quat(worldRotation.x, worldRotation.y, worldRotation.z,
                                                                   worldRotation.w),
                                                         MotionType,
                                                         CollisionLayer);
        return RigidbodyComponent(_jphPhysicsSystem->GetBodyInterface().CreateAndAddBody(creationSettings, Activation));
    }

    void PhysicsSystem::Step(entt::registry& Registry, const float DeltaTime) const
    {
        _jphPhysicsSystem->Update(DeltaTime, 1, _jphTempAllocator.get(), _jphJobSystem.get());

        SyncTransforms(Registry);
    }

    void PhysicsSystem::SyncTransforms(entt::registry& Registry) const
    {
        const auto view = Registry.view<Core::TransformComponent, Core::RigidbodyComponent>();

        for (auto [entity, transformComponent, rigidbodyComponent] : view.each())
        {
            const JPH::BodyLockRead lock(_jphPhysicsSystem->GetBodyLockInterfaceNoLock(),
                                         rigidbodyComponent.GetBodyId());
            const JPH::Body& body = lock.GetBody();

            const JPH::RMat44& mat = body.GetWorldTransform();
            const JPH::Vec3 pos = mat.GetTranslation();
            const JPH::Quat rot = mat.GetRotation().GetQuaternion();

            transformComponent.SetWorldPosition(glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ()));
            transformComponent.SetLocalRotation(glm::quat(rot.GetW(), rot.GetX(), rot.GetY(), rot.GetZ()));
        }
    }
}
