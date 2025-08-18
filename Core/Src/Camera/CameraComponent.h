#pragma once
#include <glm/glm.hpp>

namespace Core
{
    class CameraComponent
    {
    public:
        glm::mat4 ProjectionMatrix{};
    };
}
