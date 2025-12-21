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
            alignas(16) glm::mat4 ViewMatrix{};
            alignas(16) glm::mat4 CascadeViewProj[4]{};
            alignas(16) float CascadeSplits[4]{};
            alignas(16) glm::vec3 Direction;
            alignas(16) glm::vec3 Color;

            ShaderData()
            {
            }

            ShaderData(const DirectionalLightComponent& Light, const glm::mat4& ViewMatrix,
                       const glm::mat4 (&CascadeViewProj)[4], const float (&CascadeSplits)[4],
                       const glm::vec3& Direction) : ViewMatrix(ViewMatrix),
                                                     Direction(Direction),
                                                     Color(Light.Color)

            {
                std::memcpy(this->CascadeViewProj, CascadeViewProj, sizeof(glm::mat4) * 4);
                std::memcpy(this->CascadeSplits, CascadeSplits, sizeof(float) * 4);
            }
        };
    };
}
