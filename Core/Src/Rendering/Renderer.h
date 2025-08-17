#pragma once
#include <entt/entity/registry.hpp>
#include <glm/vec2.hpp>

#include "Rendering/Textures/Skybox.h"

namespace Core
{
    class Renderer
    {
    private:
        static glm::ivec2 _resolution;
        static Skybox _skybox;

    public:
        static void Initialize(const glm::ivec2& Resolution, entt::registry& SceneRegistry);

        static void Destroy();

        static void RenderFrame();
    };
}
