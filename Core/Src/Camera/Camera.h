#pragma once
#include <glm/glm.hpp>

namespace Core
{
    class Camera
    {
    public:
        glm::mat4 ProjectionMatrix{};
        float RotationSensitivity{};
        float MovementSensitivity{};
    };
}
