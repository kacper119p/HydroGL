#include "SceneFrameBuffer.h"

namespace
{
    constexpr GLenum ColorBufferFormat = GL_RGBA16F;
    constexpr GLenum DepthBufferFormat = GL_DEPTH24_STENCIL8;
}

namespace Core
{
    SceneFrameBuffer::SceneFrameBuffer(const glm::ivec2& Resolution) : _resolution(Resolution)
    {
        _multiSampledFramebuffer = GlFramebufferUnique::Create();
        _opaqueFramebuffer = GlFramebufferUnique::Create();
        _multisampledColorBuffer = GlTextureUnique::Create(GL_TEXTURE_2D_MULTISAMPLE);
        _opaqueColorBuffer = GlTextureUnique::Create(GL_TEXTURE_2D);
        _multisampledDepthStencilBuffer = GlRenderbufferUnique::Create();
        _resolvedDepthBuffer = GlTextureUnique::Create(GL_TEXTURE_2D);
        _transparentFramebuffer = GlFramebufferUnique::Create();
        _transparentColorBuffer = GlTextureUnique::Create(GL_TEXTURE_2D);
        UpdateBuffers();
    }

    SceneFrameBuffer::~SceneFrameBuffer() = default;

    void SceneFrameBuffer::ResolveMultisampling() const
    {
        glBlitNamedFramebuffer(_multiSampledFramebuffer, _opaqueFramebuffer,
                               0, 0, _resolution.x, _resolution.y,
                               0, 0, _resolution.x, _resolution.y,
                               GL_COLOR_BUFFER_BIT, GL_LINEAR);

        glBlitNamedFramebuffer(_multiSampledFramebuffer, _opaqueFramebuffer,
                               0, 0, _resolution.x, _resolution.y,
                               0, 0, _resolution.x, _resolution.y,
                               GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
    }

    void SceneFrameBuffer::BlitOpaqueToTransparent() const
    {
        glBlitNamedFramebuffer(_opaqueFramebuffer, _transparentFramebuffer,
                               0, 0, _resolution.x, _resolution.y,
                               0, 0, _resolution.x, _resolution.y,
                               GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

    void SceneFrameBuffer::UpdateBuffers() const
    {
        glTextureStorage2DMultisample(_multisampledColorBuffer, Samples, ColorBufferFormat, _resolution.x,
                                      _resolution.y,
                                      GL_TRUE);
        glNamedFramebufferTexture(_multiSampledFramebuffer, GL_COLOR_ATTACHMENT0, _multisampledColorBuffer, 0);

        glNamedRenderbufferStorageMultisample(_multisampledDepthStencilBuffer, Samples, DepthBufferFormat,
                                              _resolution.x,
                                              _resolution.y);
        glNamedFramebufferRenderbuffer(_multiSampledFramebuffer, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                       _multisampledDepthStencilBuffer);

        constexpr GLenum attachments[1] = {GL_COLOR_ATTACHMENT0};
        glNamedFramebufferDrawBuffers(_multiSampledFramebuffer, 1, attachments);

        glTextureStorage2D(_opaqueColorBuffer, 1, ColorBufferFormat, _resolution.x, _resolution.y);
        glTextureParameteri(_opaqueColorBuffer, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(_opaqueColorBuffer, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(_opaqueColorBuffer, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(_opaqueColorBuffer, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTextureStorage2D(_resolvedDepthBuffer, 1, DepthBufferFormat, _resolution.x, _resolution.y);
        glTextureParameteri(_resolvedDepthBuffer, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(_resolvedDepthBuffer, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(_resolvedDepthBuffer, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(_resolvedDepthBuffer, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glNamedFramebufferTexture(_opaqueFramebuffer, GL_COLOR_ATTACHMENT0, _opaqueColorBuffer, 0);
        glNamedFramebufferTexture(_opaqueFramebuffer, GL_DEPTH_STENCIL_ATTACHMENT, _resolvedDepthBuffer, 0);
        glNamedFramebufferDrawBuffers(_opaqueFramebuffer, 1, attachments);

        glTextureStorage2D(_transparentColorBuffer, 1, ColorBufferFormat, _resolution.x, _resolution.y);
        glTextureParameteri(_transparentColorBuffer, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(_transparentColorBuffer, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(_transparentColorBuffer, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(_transparentColorBuffer, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glNamedFramebufferTexture(_transparentFramebuffer, GL_COLOR_ATTACHMENT0, _transparentColorBuffer, 0);
        glNamedFramebufferTexture(_transparentFramebuffer, GL_DEPTH_STENCIL_ATTACHMENT, _resolvedDepthBuffer, 0);
        glNamedFramebufferDrawBuffers(_transparentFramebuffer, 1, attachments);
    }
}
