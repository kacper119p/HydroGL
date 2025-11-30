#pragma once
#include "Rendering/Shaders/Shader.h"

namespace Core
{
    class SprayManager
    {
    private:
        struct alignas(16) ParticleData
        {
            glm::vec3 Position;
            float Rotation;
            glm::vec3 Velocity;
            float LifeTime;
        };

    private:
        Shader _spawnerShader;
        Shader _updateShader;
        Shader _renderShader;

    public:
        SprayManager();

    public:
        void Update(float DeltaTime);

        void Render();
    };
}
