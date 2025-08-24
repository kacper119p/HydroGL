#pragma once
#include <glm/vec3.hpp>

#include "Scene/TransformComponent.h"

namespace Core
{
    struct PointLightComponent
    {
        glm::vec3 Color = glm::vec3(1.0f, 1.0f, 1.0f);

        float LinearFalloff = 0.0f;
        float QuadraticFalloff = 1.0f;
        float Range = 100.0f;

        struct alignas(16) ShaderData
        {
            glm::vec3 Color;
            float LinearFalloff;
            glm::vec3 Position;
            float QuadraticFalloff;
            float Range;
            [[maybe_unused]] float _padding[3];

            ShaderData()
            {
            }

            ShaderData(const PointLightComponent& pointLight, const glm::vec3& Position) :
                Color(pointLight.Color),
                LinearFalloff(pointLight.LinearFalloff),
                Position(Position),
                QuadraticFalloff(pointLight.QuadraticFalloff),
                Range(pointLight.Range)
            {
            }
        };
    };
}
