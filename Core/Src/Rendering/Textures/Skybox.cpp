#include "Skybox.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Debug.h"
#include "glad/glad.h"
#include "Rendering/RendererHelpers/CubeRenderer.h"
#include "Rendering/Shaders/Shader.h"
#include "Rendering/Shaders/ShaderBuilder.h"
#include "Rendering/Textures/DDSLoader.h"

namespace Core
{
    Skybox Skybox::CreateSkybox(const std::string_view& FilePath)
    {
        Skybox result;
        uint32_t width;
        uint32_t height;
        result._cubeMap = DdsLoader::LoadDds(FilePath, width, height);

        CHECK_MESSAGE(width == height, "Skybox requires square faces!")

        result._irradianceMap = IrradianceMapFromSkybox(result._cubeMap);
        result._prefilterMap = PrefilterMapFromSkybox(result._cubeMap, width);

        return result;
    }

    void Skybox::DestroySkybox(const Skybox& Skybox)
    {
        glDeleteTextures(1, &Skybox._cubeMap);
        glDeleteTextures(1, &Skybox._irradianceMap);
        glDeleteTextures(1, &Skybox._prefilterMap);
    }

    uint32_t Skybox::IrradianceMapFromSkybox(const uint32_t Skybox)
    {
        constexpr uint32_t resolution = 32;

        uint32_t irradianceMap;
        InitializeCubeMap(irradianceMap, resolution);

        uint32_t fbo;
        uint32_t rbo;
        InitializeFbo(fbo, rbo, resolution);

        glm::mat4 captureProjection;
        GetCaptureProjection(captureProjection);
        std::array<glm::mat4, 6> captureViews;
        GetCaptureViews(captureViews);

        const Shader shader = ShaderBuilder()
                .SetStage("./res/shaders/Utility/Convolution/Convolution.vert", ShaderStage::VertexShader)
                .SetStage("./res/shaders/Utility/Convolution/Convolution.frag", ShaderStage::FragmentShader)
                .Build();


        shader.Use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox);
        shader.SetUniform("ProjectionMatrix", captureProjection);

        InitializeViewPort(resolution);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDisable(GL_DEPTH_TEST);

        const int32_t viewMatrixLocation = shader.GetUniformLocation("viewMatrix");

        for (uint32_t i = 0; i < 6; ++i)
        {
            Shader::SetUniform(viewMatrixLocation, captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            CubeRenderer::Draw();
        }

        glEnable(GL_DEPTH_TEST);

        glDeleteFramebuffers(1, &fbo);
        glDeleteRenderbuffers(1, &rbo);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return irradianceMap;
    }

    uint32_t Skybox::PrefilterMapFromSkybox(const uint32_t Skybox, const uint32_t SkyboxResolution)
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
        std::array<glm::mat4, 6> captureViews;
        GetCaptureViews(captureViews);

        uint32_t fbo;
        uint32_t rbo;
        InitializeFbo(fbo, rbo, resolution);

        const Shader shader = ShaderBuilder()
                .SetStage("./res/shaders/Utility/SpecularIBLSetup/Prefilter.vert", ShaderStage::VertexShader)
                .SetStage("./res/shaders/Utility/SpecularIBLSetup/Prefilter.frag", ShaderStage::FragmentShader)
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
            glBindRenderbuffer(GL_RENDERBUFFER, rbo);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
                                  mipWidth, mipHeight);
            glViewport(0, 0, mipWidth, mipHeight);
            const float roughness = static_cast<float>(mipLevel) / static_cast<float>(maxMipLevels - 1);
            shader.SetUniform("Roughness", roughness);
            for (uint32_t i = 0; i < 6; ++i)
            {
                shader.SetUniform("ViewMatrix", captureViews[i]);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                       GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubeMap, mipLevel);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                CubeRenderer::Draw();
            }
        }

        glEnable(GL_DEPTH_TEST);

        glDeleteFramebuffers(1, &fbo);
        glDeleteRenderbuffers(1, &rbo);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return cubeMap;
    }

    void Skybox::InitializeFbo(uint32_t& Fbo, uint32_t& Rbo, const uint32_t Resolution)
    {
        {
            glGenFramebuffers(1, &Fbo);
            glGenRenderbuffers(1, &Rbo);
            glBindFramebuffer(GL_FRAMEBUFFER, Fbo);
            glBindRenderbuffer(GL_RENDERBUFFER, Rbo);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, Resolution, Resolution);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                      GL_RENDERBUFFER, Rbo);
        }
    }

    void Skybox::InitializeCubeMap(uint32_t& CubeMap, const uint32_t Resolution)
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

    void Skybox::InitializeViewPort(const uint32_t Resolution)
    {
        glViewport(0, 0, static_cast<GLsizei>(Resolution), static_cast<GLsizei>(Resolution));
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDepthFunc(GL_LESS);
    }

    void Skybox::GetCaptureProjection(glm::mat4& ProjectionMatrix)
    {
        ProjectionMatrix = glm::perspective(glm::radians(90.0f), 1.0f,
                                            0.1f, 10.0f);
    }

    void Skybox::GetCaptureViews(std::array<glm::mat4, 6>& ViewMatrices)
    {
        ViewMatrices[0] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f),
                                      glm::vec3(0.0f, -1.0f, 0.0f));
        ViewMatrices[1] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f),
                                      glm::vec3(0.0f, -1.0f, 0.0f));
        ViewMatrices[2] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
                                      glm::vec3(0.0f, 0.0f, 1.0f));
        ViewMatrices[3] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f),
                                      glm::vec3(0.0f, 0.0f, -1.0f));
        ViewMatrices[4] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f),
                                      glm::vec3(0.0f, -1.0f, 0.0f));
        ViewMatrices[5] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f),
                                      glm::vec3(0.0f, -1.0f, 0.0f));
    }
}
