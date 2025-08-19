#include "SceneFrameBuffer.h"

namespace Core
{
    SceneFrameBuffer::SceneFrameBuffer(const glm::ivec2& Resolution) : _resolution(Resolution)
    {
        glGenFramebuffers(1, &_multiSampledId);
        glGenFramebuffers(1, &_resolvedNormalsFramebuffer);
        glGenFramebuffers(1, &_resolvedId);
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
        // Resolve color
        glBindFramebuffer(GL_READ_FRAMEBUFFER, _multiSampledId);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _resolvedId);
        glBlitFramebuffer(0, 0, _resolution.x, _resolution.y, 0, 0, _resolution.x, _resolution.y,
                          GL_COLOR_BUFFER_BIT, GL_LINEAR);

        // Resolve occlusion mask
        glReadBuffer(GL_COLOR_ATTACHMENT1);
        glDrawBuffer(GL_COLOR_ATTACHMENT1);
        glBlitFramebuffer(0, 0, _resolution.x, _resolution.y, 0, 0, _resolution.x, _resolution.y,
                          GL_COLOR_BUFFER_BIT, GL_LINEAR);

        // Resolve depth buffer
        glBlitFramebuffer(0, 0, _resolution.x, _resolution.y, 0, 0, _resolution.x, _resolution.y,
                          GL_DEPTH_BUFFER_BIT, GL_NEAREST);

        // Return default state
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
    }

    void SceneFrameBuffer::ResolveNormals() const
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, _multiSampledId);
        glReadBuffer(GL_COLOR_ATTACHMENT1);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _resolvedNormalsFramebuffer);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glBlitFramebuffer(0, 0, _resolution.x, _resolution.y, 0, 0, _resolution.x, _resolution.y,
                          GL_COLOR_BUFFER_BIT, GL_LINEAR);

        // Return default state
        glReadBuffer(GL_COLOR_ATTACHMENT0);
    }

    void SceneFrameBuffer::UpdateBuffers()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, _multiSampledId);

        glDeleteTextures(1, &_colorBuffer);
        glGenTextures(1, &_colorBuffer);
        glDeleteTextures(1, &_normalsBuffer);
        glGenTextures(1, &_normalsBuffer);
        glDeleteTextures(1, &_occlusionBuffer);
        glGenTextures(1, &_occlusionBuffer);
        glDeleteTextures(1, &_resolvedColorBuffer);
        glGenTextures(1, &_resolvedColorBuffer);
        glDeleteRenderbuffers(1, &_depthStencilBuffer);
        glGenRenderbuffers(1, &_depthStencilBuffer);
        glDeleteTextures(1, &_resolvedNormalsTexture);
        glGenTextures(1, &_resolvedNormalsTexture);
        glDeleteTextures(1, &_resolvedOcclusionBuffer);
        glGenTextures(1, &_resolvedOcclusionBuffer);
        glDeleteRenderbuffers(1, &_resolvedDepthBuffer);
        glGenRenderbuffers(1, &_resolvedDepthBuffer);

        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _colorBuffer);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, Samples, GL_RGBA16F, _resolution.x, _resolution.y, GL_TRUE);;
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, _colorBuffer, 0);

        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _normalsBuffer);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, Samples, GL_RGBA32F, _resolution.x, _resolution.y, GL_TRUE);;
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, _normalsBuffer, 0);

        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _occlusionBuffer);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, Samples, GL_R11F_G11F_B10F, _resolution.x, _resolution.y,
                                GL_TRUE);;

        constexpr GLenum attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
        glDrawBuffers(2, attachments);

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

        glBindTexture(GL_TEXTURE_2D, _resolvedOcclusionBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, _resolution.x, _resolution.y, 0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _resolvedOcclusionBuffer, 0);

        glDrawBuffers(2, attachments);

        glBindRenderbuffer(GL_RENDERBUFFER, _resolvedDepthBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, _resolution.x, _resolution.y);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _resolvedDepthBuffer);

        glBindFramebuffer(GL_FRAMEBUFFER, _resolvedNormalsFramebuffer);

        glBindTexture(GL_TEXTURE_2D, _resolvedNormalsTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _resolution.x, _resolution.y, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _resolvedNormalsTexture, 0);

        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}
