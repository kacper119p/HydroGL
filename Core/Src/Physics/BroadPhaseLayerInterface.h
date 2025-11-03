#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>

#include "Physics/BroadPhaseLayers.h"
#include "Physics/ObjectLayers.h"

namespace Core::Physics
{
    class BroadPhaseLayerInterface final : public JPH::BroadPhaseLayerInterface
    {
    private:
        JPH::BroadPhaseLayer _objectToBroadPhaseLayer[Layers::LayerCount];

    public:
        BroadPhaseLayerInterface()
        {
            _objectToBroadPhaseLayer[Layers::Static] = BroadPhaseLayers::Static;
            _objectToBroadPhaseLayer[Layers::Dynamic] = BroadPhaseLayers::Dynamic;
        }

    public:
        [[nodiscard]] JPH::uint GetNumBroadPhaseLayers() const override
        {
            return Physics::BroadPhaseLayers::LayerCount;
        }


        JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer InLayer) const override
        {
            JPH_ASSERT(InLayer < Physics::Layers::LayerCount);
            return _objectToBroadPhaseLayer[InLayer];
        }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        [[nodiscard]] const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
        {
            switch (inLayer)
            {
                case (JPH::BroadPhaseLayer::Type) Physics::BroadPhaseLayers::Static:
                    return "STATIC";
                case (JPH::BroadPhaseLayer::Type) Physics::BroadPhaseLayers::Dynamic:
                    return "DYNAMIC";
                default: JPH_ASSERT(false);
                    return "INVALID";
            }
        }
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED
    };
}
