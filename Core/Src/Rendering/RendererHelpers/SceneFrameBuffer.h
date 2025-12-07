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
        glm::ivec2 _resolution;

        GlFramebufferUnique _multiSampledFramebuffer;
        GlTextureUnique _multisampledColorBuffer;
        GlRenderbufferUnique _multisampledDepthStencilBuffer;

        GlFramebufferUnique _opaqueFramebuffer;
        GlTextureUnique _opaqueColorBuffer;
        GlTextureUnique _opaqueDepthBuffer;

        GlFramebufferUnique _transparentFramebuffer;
        GlTextureUnique _transparentColorBuffer;
        GlTextureUnique _transparentDepthBuffer;

    public:
        explicit SceneFrameBuffer(const glm::ivec2& Resolution);

        ~SceneFrameBuffer();

    public:
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
         * @brief Returns id of the underlying multisampled FrameBuffer.
         */
        [[nodiscard]] unsigned int GetMultiSampledFrameBuffer() const
        {
            return _multiSampledFramebuffer;
        }

        /**
        * @brief Returns color texture attached to multisampled buffer.
        */
        [[nodiscard]] uint32_t GetMultiSampledColorBuffer() const
        {
            return _multisampledColorBuffer;
        }

        /**
        * @brief Returns depth and stencil buffer attached multisampled buffer.
        */
        [[nodiscard]] uint32_t GetMultisampledDepthBuffer() const
        {
            return _multisampledDepthStencilBuffer;
        }

        /**
        * @brief Returns id of the underlying opaque FrameBuffer.
        */
        [[nodiscard]] unsigned int GetOpaqueFrameBuffer() const
        {
            return _opaqueFramebuffer;
        }

        /**
        * @brief Returns color texture attached to opaque buffer.
        */
        [[nodiscard]] uint32_t GetOpaqueColorBuffer() const
        {
            return _opaqueColorBuffer;
        }

        /**
        * @brief Returns depth render buffer attached to opaque frameBuffer.
        */
        [[nodiscard]] uint32_t GetOpaqueDepthBuffer() const
        {
            return _opaqueDepthBuffer;
        }

        /**
       * @brief Returns id of the underlying transparent FrameBuffer.
       */
        [[nodiscard]] uint32_t GetTransparentFrameBuffer() const
        {
            return _transparentFramebuffer;
        }

        /**
        * @brief Returns color texture attached to transparent buffer.
        */
        [[nodiscard]] uint32_t GetTransparentColorBuffer() const
        {
            return _transparentColorBuffer;
        }

        /**
        * @brief Returns depth render buffer attached to transparent frameBuffer.
        */
        [[nodiscard]] uint32_t GetTransparentDepthBuffer() const
        {
            return _opaqueDepthBuffer;
        }


        /**
         * @brief Binds underlying multisampled FrameBuffer for rendering.
         */
        void BindMultiSampledFramebuffer() const
        {
            glBindFramebuffer(GL_FRAMEBUFFER, _multiSampledFramebuffer);
        }

        /**
         * @brief Binds underlying opaque FrameBuffer for rendering.
         */
        void BindOpaqueFramebuffer() const
        {
            glBindFramebuffer(GL_FRAMEBUFFER, _opaqueFramebuffer);
        }

        /**
         * @brief Binds underlying transparent FrameBuffer for rendering.
         */
        void BindTransparentFramebuffer() const
        {
            glBindFramebuffer(GL_FRAMEBUFFER, _transparentFramebuffer);
        }

    public:
        /**
         * @brief Resolves multisampling and writes result to opaque buffer.
         */
        void ResolveMultisampling() const;

        /**
         * @brief Writes opaque buffer content to transparent buffer.
         */
        void BlitOpaqueToTransparent() const;

    private:
        void UpdateBuffers() const;
    };
}
