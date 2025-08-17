#pragma once
#include <glm/vec2.hpp>

#include "Rendering/Shaders/Shader.h"
#include "Rendering/Textures/Skybox.h"
#include "Scene/Scene.h"

namespace Core
{
    class Renderer
    {
    private:
        static glm::uvec2 _resolution;
        static Shader _skyboxShader;
        static int32_t _skyboxViewMatrixLocation;
        static int32_t _skyboxProjectionMatrixLocation;

    public:
        static void Initialize(const glm::uvec2& Resolution);

        static void Destroy();

        static void RenderFrame(const Scene& Scene);
    };
}
