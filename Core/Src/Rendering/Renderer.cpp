#include "Renderer.h"

namespace Core
{
    glm::ivec2 Resolution;

    void Renderer::Initialize(const glm::ivec2& Resolution, entt::registry& SceneRegistry)
    {
        _resolution = Resolution;
    }

    void Renderer::Destroy()
    {
    }

    void Renderer::RenderFrame()
    {
    }
}
