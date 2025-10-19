#include "PhysicsSystem.h"

#include <thread>

#include "Physics/BroadPhaseLayerInterface.h"
#include "Physics/ObjectLayerPairFilter.h"
#include "Physics/ObjectVsBroadPhaseLayerFilter.h"
#include "Components/RigidbodyComponent.h"
#include "Scene/TransformComponent.h"

namespace
{
    constexpr size_t TempAllocatorSize = 12 * 1024 * 1024; // 10 MB
    constexpr uint32_t MaxBodies = 65536;
    constexpr uint32_t MaxObjectPairs = 65536;
    constexpr uint32_t MaxContactConstraints = 1024;
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


        const Physics::BroadPhaseLayerInterface broadPhaseLayerInterface;
        const Physics::ObjectVsBroadPhaseLayerFilter objectVsBroadPhaseLayerFilter;
        const Physics::ObjectLayerPairFilter objectVsObjectLayerFilter;

        _jphPhysicsSystem->Init(MaxBodies, 0, MaxObjectPairs, MaxContactConstraints, broadPhaseLayerInterface,
                                objectVsBroadPhaseLayerFilter, objectVsObjectLayerFilter);
    }

    PhysicsSystem::~PhysicsSystem()
    {
        delete JPH::Factory::sInstance;
    }

    void PhysicsSystem::Step(entt::registry& Registry, const float DeltaTime) const
    {
        _jphPhysicsSystem->Update(DeltaTime, 1, _jphTempAllocator.get(), _jphJobSystem.get());

        SyncTransforms(Registry);
    }

    void PhysicsSystem::SyncTransforms(entt::registry& Registry) const
    {
        const auto view = Registry.view<Core::TransformComponent, Core::RigidbodyComponent>();

        for (auto [entity, transform, bodyComp] : view.each())
        {
            const JPH::BodyLockRead lock(_jphPhysicsSystem->GetBodyLockInterfaceNoLock(), bodyComp.GetBodyId());
            const JPH::Body& body = lock.GetBody();

            const JPH::RMat44& mat = body.GetWorldTransform();
            const JPH::Vec3 pos = mat.GetTranslation();
            const JPH::Quat rot = mat.GetRotation().GetQuaternion();

            transform.SetWorldPosition(glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ()));
            transform.SetLocalRotation(glm::quat(rot.GetW(), rot.GetX(), rot.GetY(), rot.GetZ()));
        }
    }
}
