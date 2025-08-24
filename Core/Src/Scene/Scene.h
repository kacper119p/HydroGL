#pragma once
#include <entt/entity/registry.hpp>

#include "Rendering/Textures/Skybox.h"

namespace Core
{
    class Scene
    {
    private:
        entt::registry _entityRegistry;
        Skybox _skybox;

    public:
        [[nodiscard]] entt::registry* GetEntityRegistry()
        {
            return &_entityRegistry;
        }

        [[nodiscard]] const entt::registry* GetEntityRegistry() const
        {
            return &_entityRegistry;
        }


        [[nodiscard]] Skybox GetSkybox() const
        {
            return _skybox;
        }

        void SetSkybox(const Skybox& Skybox)
        {
            _skybox = Skybox;
        }
    };
}
