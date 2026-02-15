#pragma once
#include <entt/entity/registry.hpp>

#include "Rendering/RendererHelpers/DrawElementsIndirectCommand.h"
#include "Rendering/Shaders/Shader.h"
#include "Utility/GlResourceUnique.h"

namespace Core
{
    /**
     * @brief Renders visual debugging representations for water effectors.
     */
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
        /**
         * @brief Checks if the debug rendering for water effectors is currently active.
         * @return True if debug rendering is enabled, false otherwise.
         */
        [[nodiscard]] bool IsRendering() const
        {
            return _render;
        }

        /**
         * @brief Enables or disables the debug rendering for water effectors.
         * @param Value True to enable debug rendering, false to disable.
         */
        void SetRender(const bool Value)
        {
            _render = Value;
        }

    public:
        /**
         * @brief Default constructor.
         */
        WaterEffectorsRenderer();

    public:
        /**
         * @brief Renders debug visualizations for water effectors.
         * * @param EntityRegistry The EnTT registry containing the effector entities and their components.
         * @param ViewMatrix The current camera view matrix.
         * @param ProjectionMatrix The current camera projection matrix.
         */
        void RenderDebug(const entt::registry& EntityRegistry, const glm::mat4& ViewMatrix,
                         const glm::mat4& ProjectionMatrix);

    private:
        static GlBufferUnique CreateDebugIndirectDrawCommandsSsbo();

        static GlBufferUnique CreateDebugTransformsSsbo();
    };
}
