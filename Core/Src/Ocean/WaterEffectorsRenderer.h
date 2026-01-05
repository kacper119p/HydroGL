#pragma once
#include <entt/entity/registry.hpp>

#include "Rendering/RendererHelpers/DrawElementsIndirectCommand.h"
#include "Rendering/Shaders/Shader.h"
#include "Utility/GlResourceUnique.h"

namespace Core
{
    class WaterEffectorsRenderer
    {
    private:
        struct alignas(16) DebugShaderUniformLocations
        {
        public:
            int32_t ViewMatrix;
            int32_t ProjectionMatrix;

        public:
            explicit DebugShaderUniformLocations(Shader SpectrumShader);
        };

    private:
        Shader _debugShader;
        DebugShaderUniformLocations _debugShaderUniformLocations;
        GlBufferUnique _debugIndirectDrawCommandsSsbo;
        GlBufferUnique _debugTransformsSsbo;

        std::vector<DrawElementsIndirectCommand> _debugIndirectDrawCommands;
        std::vector<glm::mat4> _debugTransformMatrices;

        bool _render = false;

    public:
        [[nodiscard]] bool IsRendering() const
        {
            return _render;
        }

        void SetRender(const bool Value)
        {
            _render = Value;
        }

    public:
        WaterEffectorsRenderer();

    public:
        void RenderDebug(const entt::registry& EntityRegistry, const glm::mat4& ViewMatrix,
                         const glm::mat4& ProjectionMatrix);

    private:
        static GlBufferUnique CreateDebugIndirectDrawCommandsSsbo();

        static GlBufferUnique CreateDebugTransformsSsbo();
    };
}
