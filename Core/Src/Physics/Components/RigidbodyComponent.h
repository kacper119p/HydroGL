#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>

namespace Core
{
    class RigidbodyComponent
    {
    private:
        JPH::BodyID _bodyId;

    public:
        explicit RigidbodyComponent(const JPH::BodyID& BodyId)
            : _bodyId(BodyId)
        {
        }

        [[nodiscard]] JPH::BodyID GetBodyId() const
        {
            return _bodyId;
        }

        void SetBodyId(const JPH::BodyID& BodyId)
        {
            _bodyId = BodyId;
        }
    };
}
