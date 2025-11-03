#pragma once
#include <cstdint>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>

namespace Core::Physics::BroadPhaseLayers
{
    constexpr JPH::BroadPhaseLayer Static(0);
    constexpr JPH::BroadPhaseLayer Dynamic(1);
    constexpr uint32_t LayerCount(2);
}
