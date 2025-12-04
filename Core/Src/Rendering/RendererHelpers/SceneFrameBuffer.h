#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "Utility/GlResourceUnique.h"

namespace Core
{
    /**
     * @brief Multi-sampled FrameBuffer used for scene rendering.
     */
    class SceneFrameBuffer final
    {
    public:
        static constexpr uint8_t Samples = 16;

    private:
        GlFramebufferUnique _multiSampledBuffer;
        glm::ivec2 _resolution;
        GlTextureUnique _colorBuffer;
        GlRenderbufferUnique _depthStencilBuffer;

        GlFramebufferUnique _resolvedBuffer;
        GlTextureUnique _resolvedColorBuffer;
        GlRenderbufferUnique _resolvedDepthBuffer;

    public:
        explicit SceneFrameBuffer(const glm::ivec2& Resolution);

        ~SceneFrameBuffer();

    public:
        /**
         * @brief Returns id of the underlying multisampled FrameBuffer.
         */
        [[nodiscard]] unsigned int GetMultiSampledId() const
        {
            return _multiSampledBuffer;
        }

        /**
        * @brief Returns id of the underlying resolved FrameBuffer.
        */
        [[nodiscard]] unsigned int GetFrameBufferId() const
        {
            return _resolvedBuffer;
        }

        /**
         * @brief Returns resolution of this buffer.
         */
        [[nodiscard]] const glm::ivec2& GetResolution() const
        {
            return _resolution;
        }

        /**
         * @brief Updates resolution of this FrameBuffer.
         * @param Value A new resolution.
         */
        void SetResolution(const glm::ivec2& Value)
        {
            _resolution = Value;
            UpdateBuffers();
        }

        /**
         * @brief Binds underlying FrameBuffer for rendering.
         */
        void BindMultiSampled() const
        {
            glBindFramebuffer(GL_FRAMEBUFFER, _multiSampledBuffer);
        }

        void BindResolved() const
        {
            glBindFramebuffer(GL_FRAMEBUFFER, _resolvedBuffer);
        }

        /**
         * @brief Returns color texture attached to multisampled buffer.
         */
        [[nodiscard]] uint32_t GetMultiSampledColorBuffer() const
        {
            return _colorBuffer;
        }

        /**
        * @brief Returns color texture attached to resolved buffer.
        */
        [[nodiscard]] uint32_t GetResolvedColorBuffer() const
        {
            return _resolvedColorBuffer;
        }

        /**
        * @brief Returns depth render buffer attached to resolved buffer.
        */
        [[nodiscard]] uint32_t GetResolvedDepthBuffer() const
        {
            return _resolvedDepthBuffer;
        }

        /**
         * @brief Returns depth and stencil buffer attached to this buffer.
         */
        [[nodiscard]] uint32_t GetDepthStencilBuffer() const
        {
            return _depthStencilBuffer;
        }

        /**
         * @brief Resolves multisampling and writes result to Resolved color buffer.
         */
        void ResolveMultisampling() const;

    private:
        void UpdateBuffers() const;
    };
}
