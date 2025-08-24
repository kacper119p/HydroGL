#pragma once
#include <glm/glm.hpp>

namespace Core
{
    struct CameraComponent
    {
    public:
        glm::mat4 ProjectionMatrix{};
    };
}
