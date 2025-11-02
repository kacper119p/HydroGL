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


        _jphPhysicsSystem->Init(MaxBodies, 0, MaxObjectPairs, MaxContactConstraints, _broadPhaseLayerInterface,
                                _objectVsBroadPhaseLayerFilter, _objectVsObjectLayerFilter);
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
        JPH::BodyCreationSettings creationSettings(Shape,
                                                   JPH::RVec3(worldPosition.x, worldPosition.y, worldPosition.z),
                                                   JPH::Quat(worldRotation.x, worldRotation.y, worldRotation.z,
                                                             worldRotation.w),
                                                   MotionType,
                                                   CollisionLayer);
        creationSettings.mAngularDamping = 1.0f;
        return RigidbodyComponent(_jphPhysicsSystem->GetBodyInterface().CreateAndAddBody(creationSettings, Activation));
    }

    void PhysicsSystem::AddForce(const RigidbodyComponent& Rigidbody, const glm::vec3& Force)
    {
        const JPH::BodyID bodyId = Rigidbody.GetBodyId();
        const JPH::Vec3 joltForce(Force.x, Force.y, Force.z);
        _jphPhysicsSystem->GetBodyInterface().AddForce(bodyId, joltForce);
    }

    void PhysicsSystem::AddForce(const RigidbodyComponent& Rigidbody, const glm::vec3& Force, const glm::vec3& Position)
    {
        const JPH::BodyID bodyId = Rigidbody.GetBodyId();
        const JPH::Vec3 joltForce(Force.x, Force.y, Force.z);
        const JPH::Vec3 joltPosition(Position.x, Position.y, Position.z);
        _jphPhysicsSystem->GetBodyInterface().AddForce(bodyId, joltForce, joltPosition);
    }

    glm::vec3 PhysicsSystem::GetLinearVelocity(const RigidbodyComponent& Rigidbody) const
    {
        const JPH::BodyID bodyId = Rigidbody.GetBodyId();
        const JPH::Vec3 result = _jphPhysicsSystem->GetBodyInterface().GetLinearVelocity(bodyId);
        return glm::vec3(result.GetX(), result.GetY(), result.GetZ());
    }

    glm::vec3 PhysicsSystem::GetAngularVelocity(const RigidbodyComponent& Rigidbody) const
    {
        const JPH::BodyID bodyId = Rigidbody.GetBodyId();
        const JPH::Vec3 result = _jphPhysicsSystem->GetBodyInterface().GetAngularVelocity(bodyId);
        return glm::vec3(result.GetX(), result.GetY(), result.GetZ());
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
            const JPH::BodyID bodyId = rigidbodyComponent.GetBodyId();
            const JPH::RMat44& mat = _jphPhysicsSystem->GetBodyInterface().GetWorldTransform(bodyId);
            const JPH::Vec3 pos = mat.GetTranslation();
            const JPH::Quat rot = mat.GetRotation().GetQuaternion();

            transformComponent.SetWorldPosition(glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ()));
            transformComponent.SetWorldRotation(glm::quat(rot.GetW(), rot.GetX(), rot.GetY(), rot.GetZ()));
        }
    }
}
