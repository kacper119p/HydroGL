#include "SceneFrameBuffer.h"

namespace Core
{
    SceneFrameBuffer::SceneFrameBuffer(const glm::ivec2& Resolution) : _resolution(Resolution)
    {
        _multiSampledBuffer = GlFramebufferUnique::Create();
        _resolvedBuffer = GlFramebufferUnique::Create();
        _colorBuffer = GlTextureUnique::Create(GL_TEXTURE_2D_MULTISAMPLE);
        _resolvedColorBuffer = GlTextureUnique::Create(GL_TEXTURE_2D);
        _depthStencilBuffer = GlRenderbufferUnique::Create();
        UpdateBuffers();
    }

    SceneFrameBuffer::~SceneFrameBuffer() = default;

    void SceneFrameBuffer::ResolveMultisampling() const
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, _multiSampledBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _resolvedBuffer);
        glBlitFramebuffer(0, 0, _resolution.x, _resolution.y, 0, 0, _resolution.x, _resolution.y,
                          GL_COLOR_BUFFER_BIT, GL_LINEAR);
    }

    void SceneFrameBuffer::UpdateBuffers() const
    {
        glTextureStorage2DMultisample(_colorBuffer, Samples, GL_RGBA16F, _resolution.x, _resolution.y, GL_TRUE);
        glNamedFramebufferTexture(_multiSampledBuffer, GL_COLOR_ATTACHMENT0, _colorBuffer, 0);

        glNamedRenderbufferStorageMultisample(_depthStencilBuffer, Samples, GL_DEPTH24_STENCIL8, _resolution.x,
                                              _resolution.y);
        glNamedFramebufferRenderbuffer(_multiSampledBuffer, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                       _depthStencilBuffer);

        constexpr GLenum attachments[1] = {GL_COLOR_ATTACHMENT0};
        glNamedFramebufferDrawBuffers(_multiSampledBuffer, 1, attachments);

        glTextureStorage2D(_resolvedColorBuffer, 1, GL_RGBA16F, _resolution.x, _resolution.y);
        glTextureParameteri(_resolvedColorBuffer, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(_resolvedColorBuffer, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(_resolvedColorBuffer, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(_resolvedColorBuffer, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glNamedFramebufferTexture(_resolvedBuffer, GL_COLOR_ATTACHMENT0, _resolvedColorBuffer, 0);
        glNamedFramebufferDrawBuffers(_resolvedBuffer, 1, attachments);
    }
}
