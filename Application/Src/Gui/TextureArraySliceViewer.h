#pragma once
#include <imgui.h>
#include <glad/glad.h>

#include "Rendering/Shaders/Shader.h"
#include "Utility/GlResourceUnique.h"

namespace Application
{
    class TextureArraySliceViewer
    {
    private:
        Core::Shader _shaderProgram;
        int32_t _layerIndexLocation;
        Core::GlFramebufferUnique _fbo;
        Core::GlTextureUnique _displayTex;
        Core::GlSamplerUnique _sampler;
        int _fbWidth = 0;
        int _fbHeight = 0;

    public:
        TextureArraySliceViewer();

        ~TextureArraySliceViewer();

        void ShowLayer(uint32_t ArrayTexture, int32_t LayerIndex, ImVec2 Size);

    private:
        static Core::GlSamplerUnique CreateSampler();

        void EnsureFramebuffer(int32_t Width, int32_t Height);
    };
}
