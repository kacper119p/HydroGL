# include "TextureArraySliceViewer.h"

#include "Rendering/RendererHelpers/ScreenQuad.h"
#include "Rendering/Shaders/ShaderBuilder.h"

Application::TextureArraySliceViewer::TextureArraySliceViewer()
    : _shaderProgram(
          Core::ShaderBuilder()
          .SetStage("Assets/Shaders/TextureViewer/TextureViewer.frag", Core::ShaderStage::FragmentShader)
          .SetStage("Assets/Shaders/TextureViewer/TextureViewer.vert", Core::ShaderStage::VertexShader)
          .Build()),
      _layerIndexLocation(_shaderProgram.GetUniformLocation("LayerIndex"))
{
}

Application::TextureArraySliceViewer::~TextureArraySliceViewer()
{
    if (_displayTex)
    {
        glDeleteTextures(1, &_displayTex);
    }
    if (_fbo)
    {
        glDeleteFramebuffers(1, &_fbo);
    }
}

void Application::TextureArraySliceViewer::ShowLayer(const uint32_t ArrayTexture, const int32_t LayerIndex,
                                                     const ImVec2 Size)
{
    const ImVec2 position = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("TextureLayer", Size);

    const int width = static_cast<int>(Size.x);
    const int height = static_cast<int>(Size.y);

    EnsureFramebuffer(width, height);

    // Backup current framebuffer and viewport
    GLint prevFbo = 0;
    GLint prevViewport[4];
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevFbo);
    glGetIntegerv(GL_VIEWPORT, prevViewport);

    // Render array slice into offscreen framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    _shaderProgram.Use();
    glBindTextureUnit(0, ArrayTexture);
    glActiveTexture(GL_TEXTURE0);
    _shaderProgram.SetUniform(_layerIndexLocation, LayerIndex);

    Core::ScreenQuad::Draw();

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glUseProgram(0);

    // Restore framebuffer + viewport
    glBindFramebuffer(GL_FRAMEBUFFER, prevFbo);
    glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    const ImVec2 positionMin = position;
    const ImVec2 positionMax = ImVec2(position.x + Size.x, position.y + Size.y);

    drawList->AddImage(_displayTex, positionMin, positionMax,
                       ImVec2(1, 0),
                       ImVec2(0, 1)
    );
}

void Application::TextureArraySliceViewer::EnsureFramebuffer(const int32_t Width, const int32_t Height)
{
    if (Width == _fbWidth
        && Height == _fbHeight
        && _fbo
        && _displayTex)
    {
        return;
    }

    if (_displayTex)
    {
        glDeleteTextures(1, &_displayTex);
    }
    if (_fbo)
    {
        glDeleteFramebuffers(1, &_fbo);
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &_displayTex);
    glTextureStorage2D(_displayTex, 1, GL_RGBA8, Width, Height);
    glTextureParameteri(_displayTex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(_displayTex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(_displayTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(_displayTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glCreateFramebuffers(1, &_fbo);
    glNamedFramebufferTexture(_fbo, GL_COLOR_ATTACHMENT0, _displayTex, 0);
    constexpr GLenum drawBuffers = GL_COLOR_ATTACHMENT0;
    glNamedFramebufferDrawBuffers(_fbo, 1, &drawBuffers);

    _fbWidth = Width;
    _fbHeight = Height;
}
