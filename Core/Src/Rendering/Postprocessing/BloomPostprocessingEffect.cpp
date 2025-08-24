#include "BloomPostprocessingEffect.h"

#include "Rendering/RendererHelpers/ScreenQuad.h"
#include "Rendering/Shaders/ShaderBuilder.h"

namespace Core
{
    BloomPostprocessingEffect::BloomPostprocessingEffect(const glm::uvec2 Resolution) :
        _resolution(Resolution),
        _prefilterShader(ShaderBuilder()
                .SetStage("Assets/Shaders/Bloom/Bloom.vert", ShaderStage::VertexShader)
                .SetStage("Assets/Shaders/Bloom/BloomFilter.frag", ShaderStage::FragmentShader)
                .Build()),
        _downSampleShader(ShaderBuilder()
                .SetStage("Assets/Shaders/Bloom/Bloom.vert", ShaderStage::VertexShader)
                .SetStage("Assets/Shaders/Bloom/Downsample.frag", ShaderStage::FragmentShader)
                .Build()),
        _upsampleShader(ShaderBuilder()
                .SetStage("Assets/Shaders/Bloom/Bloom.vert", ShaderStage::VertexShader)
                .SetStage("Assets/Shaders/Bloom/Upsample.frag", ShaderStage::FragmentShader)
                .Build()),
        _outputShader(ShaderBuilder()
                .SetStage("Assets/Shaders/Bloom/Bloom.vert", ShaderStage::VertexShader)
                .SetStage("Assets/Shaders/Bloom/BloomOutput.frag", ShaderStage::FragmentShader)
                .Build())
    {
        glGenFramebuffers(1, &_frameBuffer);

        glm::vec2 mipSize(Resolution);

        for (BloomMip& mip : _bloomMips)
        {
            mipSize *= 0.5f;
            mip.Size = mipSize;

            glGenTextures(1, &mip.Texture);
            glBindTexture(GL_TEXTURE_2D, mip.Texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, static_cast<int>(mipSize.x), static_cast<int>(mipSize.y),
                         0, GL_RGB, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            constexpr float borderColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        }

        glGenTextures(1, &_prefilteredColor);
        glBindTexture(GL_TEXTURE_2D, _prefilteredColor);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F,
                     static_cast<int>(Resolution.x), static_cast<int>(Resolution.y),
                     0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        constexpr float borderColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    }

    void BloomPostprocessingEffect::Render(const uint32_t SceneColorTexture)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);

        _prefilterShader.Use();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, _prefilteredColor, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, SceneColorTexture);
        ScreenQuad::Draw();

        _downSampleShader.Use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _prefilteredColor);

        for (const BloomMip& mip : _bloomMips)
        {
            glViewport(0, 0, static_cast<GLsizei>(mip.Size.x), static_cast<GLsizei>(mip.Size.y));
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D, mip.Texture, 0);
            ScreenQuad::Draw();
            glBindTexture(GL_TEXTURE_2D, mip.Texture);
        }


        _upsampleShader.Use();

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glBlendEquation(GL_FUNC_ADD);

        for (int i = MipCount - 1; i > 0; --i)
        {
            const BloomMip& mip = _bloomMips[i];
            const BloomMip& nextMip = _bloomMips[i - 1];

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mip.Texture);

            glViewport(0, 0, static_cast<GLsizei>(nextMip.Size.x), static_cast<GLsizei>(nextMip.Size.y));
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D, nextMip.Texture, 0);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            ScreenQuad::Draw();
        }

        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_BLEND);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        _outputShader.Use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, SceneColorTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, _bloomMips[0].Texture);

        glViewport(0, 0, _resolution.x, _resolution.y);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ScreenQuad::Draw();
    }

    BloomPostprocessingEffect::~BloomPostprocessingEffect()
    {
        for (BloomMip& bloomMip : _bloomMips)
        {
            glDeleteTextures(1, &bloomMip.Texture);
        }
        glDeleteTextures(1, &_prefilteredColor);
        glDeleteFramebuffers(1, &_frameBuffer);
    }
} // Engine
