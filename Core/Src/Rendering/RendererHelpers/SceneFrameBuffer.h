#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

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
        uint32_t _multiSampledId = 0;
        glm::ivec2 _resolution;
        uint32_t _colorBuffer = 0;
        uint32_t _depthStencilBuffer = 0;

        uint32_t _resolvedId = 0;
        uint32_t _resolvedColorBuffer = 0;

    public:
        explicit SceneFrameBuffer(const glm::ivec2& Resolution);

        ~SceneFrameBuffer();

    public:
        /**
         * @brief Returns id of the underlying FrameBuffer.
         */
        [[nodiscard]] unsigned int GetMultiSampledId() const
        {
            return _multiSampledId;
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
            glBindFramebuffer(GL_FRAMEBUFFER, _multiSampledId);
        }

        void BindResolved() const
        {
            glBindFramebuffer(GL_FRAMEBUFFER, _resolvedId);
        }

        /**
         * @brief Returns color texture attached to this buffer.
         */
        [[nodiscard]] uint32_t GetMultiSampledColorBuffer() const
        {
            return _colorBuffer;
        }

        [[nodiscard]] uint32_t GetResolvedColorBuffer() const
        {
            return _resolvedColorBuffer;
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
