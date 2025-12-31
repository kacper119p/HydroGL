# include "TextureArraySliceViewer.h"

#include "Rendering/RendererHelpers/ScreenQuad.h"
#include "Rendering/Shaders/ShaderBuilder.h"

Application::TextureArraySliceViewer::TextureArraySliceViewer()
    : _shaderProgram(
          Core::ShaderBuilder()
          .SetStage("Assets/Shaders/TextureViewer/TextureViewer.frag", Core::ShaderStage::FragmentShader)
          .SetStage("Assets/Shaders/TextureViewer/TextureViewer.vert", Core::ShaderStage::VertexShader)
          .Build()),
      _layerIndexLocation(_shaderProgram.GetUniformLocation("LayerIndex")),
      _sampler(CreateSampler())
{
}

Application::TextureArraySliceViewer::~TextureArraySliceViewer() = default;

void Application::TextureArraySliceViewer::ShowLayer(const uint32_t ArrayTexture, const int32_t LayerIndex,
                                                     const ImVec2 Size)
{
    const ImVec2 position = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("TextureLayer", Size);

    const int width = static_cast<int>(Size.x);
    const int height = static_cast<int>(Size.y);

    EnsureFramebuffer(width, height);

    // Backup current framebuffer and viewport
    GLint previousFbo = 0;
    GLint previousViewport[4];
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &previousFbo);
    glGetIntegerv(GL_VIEWPORT, previousViewport);

    // Render array slice into display texture
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    _shaderProgram.Use();
    glBindTextureUnit(0, ArrayTexture);
    glBindSampler(0, _sampler);
    _shaderProgram.SetUniform(_layerIndexLocation, LayerIndex);

    Core::ScreenQuad::Draw();

    // Restore framebuffer, viewport and default sampler; unbind texture
    glBindFramebuffer(GL_FRAMEBUFFER, previousFbo);
    glViewport(previousViewport[0], previousViewport[1], previousViewport[2], previousViewport[3]);
    glBindSampler(0, 0);
    glBindTextureUnit(0, 0);

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    const ImVec2 positionMin = position;
    const ImVec2 positionMax = ImVec2(position.x + Size.x, position.y + Size.y);

    drawList->AddImage(_displayTex.Get(), positionMin, positionMax,
                       ImVec2(1, 0),
                       ImVec2(0, 1)
    );
}

Core::GlSamplerUnique Application::TextureArraySliceViewer::CreateSampler()
{
    Core::GlSamplerUnique sampler = Core::GlSamplerUnique::Create();
    glSamplerParameteri(sampler, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return sampler;
}

void Application::TextureArraySliceViewer::EnsureFramebuffer(const int32_t Width, const int32_t Height)
{
    if (Width == _fbWidth && Height == _fbHeight
        && _fbo && _displayTex)
    {
        return;
    }

    _displayTex = Core::GlTextureUnique::Create(GL_TEXTURE_2D);
    _fbo = Core::GlFramebufferUnique::Create();

    glTextureStorage2D(_displayTex, 1, GL_RGBA8, Width, Height);
    glTextureParameteri(_displayTex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(_displayTex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(_displayTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(_displayTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glNamedFramebufferTexture(_fbo, GL_COLOR_ATTACHMENT0, _displayTex, 0);
    constexpr GLenum drawBuffers = GL_COLOR_ATTACHMENT0;
    glNamedFramebufferDrawBuffers(_fbo, 1, &drawBuffers);

    _fbWidth = Width;
    _fbHeight = Height;
}
