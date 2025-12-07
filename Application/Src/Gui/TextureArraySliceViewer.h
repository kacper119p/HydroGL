#pragma once
#include <imgui.h>
#include <glad/glad.h>

#include "Rendering/Shaders/Shader.h"

namespace Application
{
    class TextureArraySliceViewer
    {
    private:
        Core::Shader _shaderProgram;
        int32_t _layerIndexLocation;
        GLuint _fbo = 0;
        GLuint _displayTex = 0;
        int _fbWidth = 0;
        int _fbHeight = 0;

    public:
        TextureArraySliceViewer();

        ~TextureArraySliceViewer();

        void ShowLayer(uint32_t ArrayTexture, int32_t LayerIndex, ImVec2 Size);

    private:
        void EnsureFramebuffer(int32_t Width, int32_t Height);
    };
}
