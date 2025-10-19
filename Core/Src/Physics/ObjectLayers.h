#pragma once
#include <cstdint>
#include <Jolt/Physics/Collision/ObjectLayer.h>

namespace Core::Physics::Layers
{
    constexpr JPH::ObjectLayer Static = 0;
    constexpr JPH::ObjectLayer Dynamic = 1;
    constexpr uint32_t LayerCount = 2;;
}
