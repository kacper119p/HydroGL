#pragma once
#include <glm/vec2.hpp>

#include "Materials/MaterialManager.h"
#include "Rendering/Shaders/Shader.h"
#include "Rendering/Textures/Skybox.h"
#include "Scene/Scene.h"

namespace Core
{
    class Renderer
    {
    private:
        glm::uvec2 _resolution;

        Shader _skyboxShader;
        int32_t _skyboxViewMatrixLocation;
        int32_t _skyboxProjectionMatrixLocation;

        Shader _pbrShader;
        int32_t _pbrViewMatrixLocation;
        int32_t _pbrProjectionMatrixLocation;

    public:
        Renderer(const glm::uvec2& Resolution);

    public:
        ~Renderer();

        void Initialize(const glm::uvec2& Resolution);

        void Destroy();

        void RenderFrame(const Scene& Scene, const MaterialManager& MaterialManager);
    };
}
