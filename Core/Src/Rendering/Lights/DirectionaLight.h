#pragma once
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "Scene/TransformComponent.h"

namespace Core
{
    struct DirectionalLightComponent
    {
        glm::vec3 Color;

        struct alignas(16) ShaderData
        {
            glm::mat4 LightSpaceTransform;
            glm::vec3 Direction;
            [[maybe_unused]] float _padding0;
            glm::vec3 Color;
            [[maybe_unused]] float _padding1;

            ShaderData()
            {
            }

            ShaderData(const DirectionalLightComponent& spotlight, const glm::mat4& lightSpaceTransform,
                       const glm::vec3& Direction) :
                LightSpaceTransform(lightSpaceTransform),
                Direction(Direction),
                Color(spotlight.Color)

            {
            }
        };
    };
}
