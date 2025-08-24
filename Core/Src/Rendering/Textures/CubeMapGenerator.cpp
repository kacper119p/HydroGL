#include "CubeMapGenerator.h"

#include "Skybox.h"
#include "Rendering/RendererHelpers/CubeRenderer.h"
#include "Rendering/RendererHelpers/ScreenQuad.h"
#include "Rendering/Shaders/Shader.h"
#include "Rendering/Shaders/ShaderBuilder.h"

namespace Core
{
    std::array<glm::mat4, 6> CubeMapGenerator::_captureViews
    {
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f),
                        glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f),
                        glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
                        glm::vec3(0.0f, 0.0f, 1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f),
                        glm::vec3(0.0f, 0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f),
                        glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f),
                        glm::vec3(0.0f, -1.0f, 0.0f))
    };

    uint32_t CubeMapGenerator::IrradianceMapFromSkybox(const uint32_t Skybox)
    {
        constexpr uint32_t resolution = 32;

        uint32_t irradianceMap;
        InitializeCubeMap(irradianceMap, resolution);

        uint32_t fbo;
        InitializeFbo(fbo);

        glm::mat4 captureProjection;
        GetCaptureProjection(captureProjection);

        const Shader shader = ShaderBuilder()
                .SetStage("Assets/Shaders/Utility/Convolution/Convolution.vert", ShaderStage::VertexShader)
                .SetStage("Assets/Shaders/Utility/Convolution/Convolution.frag", ShaderStage::FragmentShader)
                .Build();


        shader.Use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox);
        shader.SetUniform("ProjectionMatrix", captureProjection);

        InitializeViewPort(resolution);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDisable(GL_DEPTH_TEST);

        const int32_t viewMatrixLocation = shader.GetUniformLocation("ViewMatrix");

        for (uint32_t i = 0; i < 6; ++i)
        {
            Shader::SetUniform(viewMatrixLocation, _captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
            glClear(GL_COLOR_BUFFER_BIT);
            CubeRenderer::Draw();
        }

        glEnable(GL_DEPTH_TEST);

        glDeleteFramebuffers(1, &fbo);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return irradianceMap;
    }

    uint32_t CubeMapGenerator::PrefilterMapFromSkybox(const uint32_t Skybox, const uint32_t SkyboxResolution)
    {
        constexpr uint32_t resolution = 1024;
        constexpr uint32_t maxMipLevels = 10;

        uint32_t cubeMap;
        InitializeCubeMap(cubeMap, resolution);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        glm::mat4 captureProjection;
        GetCaptureProjection(captureProjection);

        uint32_t fbo;
        InitializeFbo(fbo);

        const Shader shader = ShaderBuilder()
                .SetStage("Assets/Shaders/Utility/SpecularIBLSetup/Prefilter.vert", ShaderStage::VertexShader)
                .SetStage("Assets/Shaders/Utility/SpecularIBLSetup/Prefilter.frag", ShaderStage::FragmentShader)
                .Build();

        shader.Use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox);
        shader.SetUniform("ProjectionMatrix", captureProjection);
        shader.SetUniform("Resolution", static_cast<float>(SkyboxResolution));

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        InitializeViewPort(resolution);

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDisable(GL_DEPTH_TEST);

        for (uint32_t mipLevel = 0; mipLevel < maxMipLevels; ++mipLevel)
        {
            const uint32_t mipWidth = resolution * std::pow(0.5, mipLevel);
            const uint32_t mipHeight = resolution * std::pow(0.5, mipLevel);
            glViewport(0, 0, mipWidth, mipHeight);
            const float roughness = static_cast<float>(mipLevel) / static_cast<float>(maxMipLevels - 1);
            shader.SetUniform("Roughness", roughness);
            for (uint32_t i = 0; i < 6; ++i)
            {
                shader.SetUniform("ViewMatrix", _captureViews[i]);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                       GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubeMap, mipLevel);
                glClear(GL_COLOR_BUFFER_BIT);
                CubeRenderer::Draw();
            }
        }

        glEnable(GL_DEPTH_TEST);

        glDeleteFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return cubeMap;
    }

    uint32_t CubeMapGenerator::CreateBrdfLut()
    {
        constexpr uint32_t resolution = 1024;

        unsigned int resultTexture;
        glGenTextures(1, &resultTexture);
        glBindTexture(GL_TEXTURE_2D, resultTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 1024, 1024, 0, GL_RG, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        uint32_t fbo;
        InitializeFbo(fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, resultTexture, 0);

        const Shader shader = ShaderBuilder()
                .SetStage("./Assets/Shaders/Utility/SpecularIBLSetup/BRDF.vert", ShaderStage::VertexShader)
                .SetStage("./Assets/Shaders/Utility/SpecularIBLSetup/BRDF.frag", ShaderStage::FragmentShader)
                .Build();

        InitializeViewPort(resolution);

        shader.Use();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ScreenQuad::Draw();

        glDeleteFramebuffers(1, &fbo);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return resultTexture;
    }

    void CubeMapGenerator::InitializeFbo(uint32_t& Fbo)
    {
        glGenFramebuffers(1, &Fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, Fbo);
    }

    void CubeMapGenerator::InitializeCubeMap(uint32_t& CubeMap, const uint32_t Resolution)
    {
        {
            glGenTextures(1, &CubeMap);
            glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMap);
            for (unsigned int i = 0; i < 6; ++i)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
                             Resolution, Resolution, 0, GL_RGB, GL_FLOAT, nullptr);
            }

            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
    }

    void CubeMapGenerator::InitializeViewPort(const uint32_t Resolution)
    {
        glViewport(0, 0, static_cast<GLsizei>(Resolution), static_cast<GLsizei>(Resolution));
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDepthFunc(GL_LESS);
    }

    void CubeMapGenerator::GetCaptureProjection(glm::mat4& ProjectionMatrix)
    {
        ProjectionMatrix = glm::perspective(glm::radians(90.0f), 1.0f,
                                            0.1f, 10.0f);
    }
}
