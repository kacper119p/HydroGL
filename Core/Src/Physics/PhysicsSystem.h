#pragma once
#include <entt/entity/registry.hpp>
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include "Physics/BroadPhaseLayerInterface.h"
#include "Physics/ObjectLayerPairFilter.h"
#include "Physics/ObjectVsBroadPhaseLayerFilter.h"
#include "Physics/Components/RigidbodyComponent.h"
#include "Scene/TransformComponent.h"

namespace Core
{
    class PhysicsSystem
    {
    private:
        std::unique_ptr<JPH::PhysicsSystem> _jphPhysicsSystem;
        std::unique_ptr<JPH::TempAllocatorImpl> _jphTempAllocator;
        std::unique_ptr<JPH::JobSystemThreadPool> _jphJobSystem;
        const Physics::BroadPhaseLayerInterface broadPhaseLayerInterface;
        const Physics::ObjectVsBroadPhaseLayerFilter objectVsBroadPhaseLayerFilter;
        const Physics::ObjectLayerPairFilter objectVsObjectLayerFilter;

    public:
        PhysicsSystem();

    public:
        ~PhysicsSystem();

    public:
        [[nodiscard]] RigidbodyComponent CreateRigidbodyComponent(const TransformComponent& Transform,
                                                                  const JPH::RefConst<JPH::Shape>& Shape,
                                                                  JPH::EMotionType MotionType,
                                                                  JPH::EActivation Activation,
                                                                  uint32_t CollisionLayer) const;

        void AddForce(const RigidbodyComponent& Rigidbody, const glm::vec3& Force);

    public:
        void Step(entt::registry& Registry, float DeltaTime) const;

        void SyncTransforms(entt::registry& Registry) const;
    };
}
