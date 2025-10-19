#pragma once
#include <entt/entity/registry.hpp>
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>

namespace Core
{
    class PhysicsSystem
    {
    private:
        std::unique_ptr<JPH::PhysicsSystem> _jphPhysicsSystem;
        std::unique_ptr<JPH::TempAllocatorImpl> _jphTempAllocator;
        std::unique_ptr<JPH::JobSystemThreadPool> _jphJobSystem;

    public:
        PhysicsSystem();

    public:
        ~PhysicsSystem();

    public:
        void Step(entt::registry& Registry, float DeltaTime) const;

        void SyncTransforms(entt::registry& Registry) const;
    };
}
