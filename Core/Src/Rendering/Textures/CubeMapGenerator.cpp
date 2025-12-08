#include "CubeMapGenerator.h"

#include "Skybox.h"
#include "Rendering/RendererHelpers/CubeRenderer.h"
#include "Rendering/RendererHelpers/ScreenQuad.h"
#include "Rendering/Shaders/Shader.h"
#include "Rendering/Shaders/ShaderBuilder.h"

namespace
{
    constexpr uint32_t IrradianceMapResolution = 32;
    constexpr uint32_t PrefilterMapResolution = 1024;
    constexpr uint32_t PrefilterMapMipLevels = 10;
    constexpr GLenum CubeMapsInternalFormat = GL_RGB32F;

    const std::array<glm::mat4, 6> CaptureViews
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

    const glm::mat4 CaptureProjectionMatrix = glm::perspective(glm::radians(90.0f), 1.0f,
                                                               0.1f, 10.0f);

    void InitializeFbo(uint32_t& Fbo)
    {
        glGenFramebuffers(1, &Fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, Fbo);
    }

    void InitializeCubeMap(uint32_t& CubeMap, uint32_t Resolution, int32_t MipCount)
    {
        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &CubeMap);
        glTextureStorage2D(CubeMap, MipCount, CubeMapsInternalFormat, Resolution, Resolution);

        glTextureParameteri(CubeMap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(CubeMap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(CubeMap, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTextureParameteri(CubeMap, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(CubeMap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    void InitializeViewPort(uint32_t Resolution)
    {
        glViewport(0, 0, static_cast<GLsizei>(Resolution), static_cast<GLsizei>(Resolution));
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
    }
}

namespace Core
{
    uint32_t CubeMapGenerator::IrradianceMapFromSkybox(const uint32_t Skybox)
    {
        uint32_t irradianceMap;
        InitializeCubeMap(irradianceMap, IrradianceMapResolution, 1);

        uint32_t fbo;
        InitializeFbo(fbo);

        const Shader shader = ShaderBuilder()
                .SetStage("Assets/Shaders/Utility/Convolution/Convolution.vert", ShaderStage::VertexShader)
                .SetStage("Assets/Shaders/Utility/Convolution/Convolution.frag", ShaderStage::FragmentShader)
                .Build();

        shader.Use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox);
        shader.SetUniform("ProjectionMatrix", CaptureProjectionMatrix);

        InitializeViewPort(IrradianceMapResolution);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDisable(GL_DEPTH_TEST);

        const int32_t viewMatrixLocation = shader.GetUniformLocation("ViewMatrix");

        for (uint32_t i = 0; i < 6; ++i)
        {
            shader.SetUniform(viewMatrixLocation, CaptureViews[i]);
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
        uint32_t cubeMap;
        InitializeCubeMap(cubeMap, PrefilterMapResolution, PrefilterMapMipLevels);
        glTextureParameteri(cubeMap, GL_TEXTURE_MIN_FILTER,
                            GL_LINEAR_MIPMAP_LINEAR);
        glTextureParameteri(cubeMap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        uint32_t fbo;
        InitializeFbo(fbo);

        const Shader shader = ShaderBuilder()
                .SetStage("Assets/Shaders/Utility/SpecularIBLSetup/Prefilter.vert", ShaderStage::VertexShader)
                .SetStage("Assets/Shaders/Utility/SpecularIBLSetup/Prefilter.frag", ShaderStage::FragmentShader)
                .Build();

        shader.Use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox);
        shader.SetUniform("ProjectionMatrix", CaptureProjectionMatrix);
        shader.SetUniform("Resolution", static_cast<float>(SkyboxResolution));

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDisable(GL_DEPTH_TEST);

        for (uint32_t mipLevel = 0; mipLevel < PrefilterMapMipLevels; ++mipLevel)
        {
            const uint32_t mipWidth = PrefilterMapResolution * std::pow(0.5, mipLevel);
            const uint32_t mipHeight = PrefilterMapResolution * std::pow(0.5, mipLevel);
            glViewport(0, 0, mipWidth, mipHeight);
            const float roughness = static_cast<float>(mipLevel) / static_cast<float>(PrefilterMapMipLevels - 1);
            shader.SetUniform("Roughness", roughness);
            for (uint32_t i = 0; i < 6; ++i)
            {
                shader.SetUniform("ViewMatrix", CaptureViews[i]);
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

        uint32_t resultTexture;
        glCreateTextures(GL_TEXTURE_2D, 1, &resultTexture);
        glTextureStorage2D(resultTexture, 1, GL_RG16F, resolution, resolution);

        glTextureParameteri(resultTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(resultTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(resultTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(resultTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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

    glm::vec3 CubeMapGenerator::GetEnvironmentColorFromIrradianceMap(const uint32_t IrradianceMap)
    {
        const int32_t levels = static_cast<int32_t>(std::floor(std::log2(IrradianceMapResolution))) + 1;
        constexpr uint32_t faceCount = 6;
        GLuint tempTexture;
        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &tempTexture);
        glTextureStorage2D(tempTexture, levels, CubeMapsInternalFormat, IrradianceMapResolution,
                           IrradianceMapResolution);
        glCopyImageSubData(IrradianceMap, GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
                           tempTexture, GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
                           IrradianceMapResolution, IrradianceMapResolution, faceCount);

        glGenerateTextureMipmap(tempTexture);

        glm::vec3 faces[faceCount];

        glGetTextureImage(tempTexture, levels - 1,GL_RGB,GL_FLOAT, faceCount * sizeof(glm::vec3), faces);

        glDeleteTextures(1, &tempTexture);

        glm::vec3 averageColor(0.0f);
        for (uint32_t faceIndex = 0; faceIndex < faceCount; ++faceIndex)
        {
            averageColor += faces[faceIndex];
        }
        return averageColor / static_cast<float>(faceCount);
    }
}
