#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>

#include "Physics/ObjectLayers.h"


namespace Core::Physics
{
    class ObjectLayerPairFilter final : public JPH::ObjectLayerPairFilter
    {
    public:
        [[nodiscard]] bool
        ShouldCollide(const JPH::ObjectLayer InObject1, const JPH::ObjectLayer InObject2) const override
        {
            switch (InObject1)
            {
                case Layers::Static: // Static only collides with static
                    return InObject2 == Layers::Dynamic;
                case Layers::Dynamic: // Dynamic collides with everything
                    return true;
                default:
                    JPH_ASSERT(false);
                    return false;
            }
        }
    };
}
