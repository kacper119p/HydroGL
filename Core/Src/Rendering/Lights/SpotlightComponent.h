#pragma once
#include <glm/vec3.hpp>

#include "Scene/TransformComponent.h"

namespace Core
{
    struct SpotlightComponent
    {
        glm::vec3 Color = glm::vec3(1.0f, 1.0f, 1.0f);

        float OuterAngle = 90.0f;
        float InnerAngle = 45.0f;

        float LinearFalloff = 0.0f;
        float QuadraticFalloff = 1.0f;
        float Range = 100.0f;

        struct alignas(16) ShaderData
        {
            glm::vec3 Direction;
            float OuterAngle;
            glm::vec3 Color;
            float InnerAngle;
            glm::vec3 Position;
            float LinearFalloff;
            float QuadraticFalloff;
            float Range;

            ShaderData()
            {
            }

            ShaderData(const SpotlightComponent& spotlight, const glm::vec3& Direction, const glm::vec3& Position) :
                Direction(Direction),
                OuterAngle(spotlight.OuterAngle),
                Color(spotlight.Color),
                InnerAngle(spotlight.InnerAngle),
                Position(Position),
                LinearFalloff(spotlight.LinearFalloff),
                QuadraticFalloff(spotlight.QuadraticFalloff),
                Range(spotlight.Range)
            {
            }
        };
    };
}
