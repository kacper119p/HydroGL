#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>

namespace Core
{
    class RigidbodyComponent
    {
    private:
        JPH::BodyID _bodyId;
        bool _isDynamic = true;

    public:
        [[nodiscard]] JPH::BodyID GetBodyId() const
        {
            return _bodyId;
        }

        void SetBodyId(const JPH::BodyID& BodyId)
        {
            _bodyId = BodyId;
        }

        [[nodiscard]] bool IsIsDynamic() const
        {
            return _isDynamic;
        }

        void SetIsDynamic(const bool IsDynamic)
        {
            _isDynamic = IsDynamic;
        }
    };
}
