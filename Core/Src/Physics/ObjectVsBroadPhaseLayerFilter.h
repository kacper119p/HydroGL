#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>

#include "Physics/BroadPhaseLayers.h"
#include "Physics/ObjectLayers.h"

namespace Core::Physics
{
    class ObjectVsBroadPhaseLayerFilter final : public JPH::ObjectVsBroadPhaseLayerFilter
    {
    public:
        [[nodiscard]] bool
        ShouldCollide(const JPH::ObjectLayer InLayer1, const JPH::BroadPhaseLayer InLayer2) const override
        {
            switch (InLayer1)
            {
                case Layers::Static: // Static objects only collide with dynamic broad phase layer
                    return InLayer2 == BroadPhaseLayers::Dynamic;
                case Layers::Dynamic: // Dynamic objects collide with all broad phase layers
                    return true;
                default:
                    JPH_ASSERT(false);
                    return false;
            }
        }
    };
}
