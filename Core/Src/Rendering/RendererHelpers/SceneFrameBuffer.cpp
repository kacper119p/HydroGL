#include "SceneFrameBuffer.h"

namespace Core
{
    SceneFrameBuffer::SceneFrameBuffer(const glm::ivec2& Resolution) :
        _resolution(Resolution)
    {
        glGenFramebuffers(1, &_multiSampledId);
        glGenFramebuffers(1, &_resolvedId);
        glGenTextures(1, &_colorBuffer);
        glGenTextures(1, &_resolvedColorBuffer);
        glGenRenderbuffers(1, &_depthStencilBuffer);
        UpdateBuffers();
    }

    SceneFrameBuffer::~SceneFrameBuffer()
    {
        glDeleteFramebuffers(1, &_multiSampledId);
        glDeleteFramebuffers(1, &_resolvedId);
        glDeleteTextures(1, &_colorBuffer);
        glDeleteTextures(1, &_resolvedColorBuffer);
        glDeleteRenderbuffers(1, &_depthStencilBuffer);
    }

    void SceneFrameBuffer::ResolveMultisampling() const
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, _multiSampledId);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _resolvedId);
        glBlitFramebuffer(0, 0, _resolution.x, _resolution.y, 0, 0, _resolution.x, _resolution.y,
                          GL_COLOR_BUFFER_BIT, GL_LINEAR);
    }

    void SceneFrameBuffer::UpdateBuffers() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, _multiSampledId);

        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _colorBuffer);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, Samples, GL_RGBA16F, _resolution.x, _resolution.y, GL_TRUE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, _colorBuffer, 0);

        constexpr GLenum attachments[1] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, attachments);

        glBindRenderbuffer(GL_RENDERBUFFER, _depthStencilBuffer);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, Samples, GL_DEPTH24_STENCIL8, _resolution.x, _resolution.y);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthStencilBuffer);


        glBindFramebuffer(GL_FRAMEBUFFER, _resolvedId);

        glBindTexture(GL_TEXTURE_2D, _resolvedColorBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _resolution.x, _resolution.y, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _resolvedColorBuffer, 0);

        glDrawBuffers(1, attachments);

        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}
