#include "WaterSystem.h"

#include <chrono>
#include <numbers>
#include <random>
#include <spdlog/spdlog.h>

#include "TimeManager.h"
#include "Rendering/Shaders/ShaderBuilder.h"

namespace Core
{
    WaterSystem::WaterSystem() : _oceanGrid(1000.0f, 100, 10), _waterShader(
                                     ShaderBuilder().SetStage("Assets/Shaders/Water/Water.vert",
                                                              ShaderStage::VertexShader)
                                     .SetStage("Assets/Shaders/Water/Water.tesc",
                                               ShaderStage::TesselationControlShader)
                                     .SetStage("Assets/Shaders/Water/Water.tese",
                                               ShaderStage::TesselationEvaluationShader)
                                     .SetStage("Assets/Shaders/Water/Water.frag",
                                               ShaderStage::FragmentShader)
                                     .Build()),
                                 _viewMatrixLocation(_waterShader.GetUniformLocation("ViewMatrix")),
                                 _projectionMatrixLocation(_waterShader.GetUniformLocation("ProjectionMatrix")),
                                 _cameraPositionLocation(_waterShader.GetUniformLocation("CameraPosition")),
                                 _timeLocation(_waterShader.GetUniformLocation("Time")),
                                 _gridOriginLocation(_waterShader.GetUniformLocation("GridOrigin")),
                                 _fftHorizontalShader(ShaderBuilder().SetStage(
                                     "Assets/Shaders/Water/FftHorizontal.comp", ShaderStage::ComputeShader).Build()),
                                 _fftVerticalShader(
                                     ShaderBuilder().SetStage("Assets/Shaders/Water/FftVertical.comp",
                                                              ShaderStage::ComputeShader).Build()),
                                 _assembleComputeShader(ShaderBuilder().SetStage(
                                     "Assets/Shaders/Water/Assemble.comp", ShaderStage::ComputeShader).Build()),
                                 _initialSpectrumComputeShader(ShaderBuilder().SetStage(
                                     "Assets/Shaders/Water/InitialSpectrum.comp",
                                     ShaderStage::ComputeShader).Build()),
                                 _spectrumComputeShader(ShaderBuilder().SetStage(
                                     "Assets/Shaders/Water/Spectrum.comp", ShaderStage::ComputeShader).Build())
    {
        CreateTexture(&_initialSpectrumTexture, TextureResolution, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_NEAREST,GL_REPEAT);
        CreateTexture(&_hDxTexture, TextureResolution, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_NEAREST,GL_REPEAT);
        CreateTexture(&_dzDfhxTexture, TextureResolution, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_NEAREST,GL_REPEAT);
        CreateTexture(&_dfhzDfdxxTexture, TextureResolution, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_NEAREST,GL_REPEAT);
        CreateTexture(&_dfdzxDfdzzTexture, TextureResolution, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_NEAREST,GL_REPEAT);
        CreateTexture(&_spectrumTexture, TextureResolution, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_NEAREST,GL_REPEAT);
        CreateTexture(&_normalTexture, TextureResolution, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_LINEAR,GL_REPEAT);
        CreateTexture(&_displacementTexture, TextureResolution, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_LINEAR,GL_REPEAT);
        CreateTexture(&_tmpTexture, TextureResolution, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_NEAREST,GL_REPEAT);
        CreateTexture(&_gaussianNoiseTexture, TextureResolution, GL_RG32F, GL_RG, GL_FLOAT, GL_NEAREST,
                      GL_CLAMP_TO_BORDER);
        UpdateInitialSpectrum();
    }

    void WaterSystem::Render(const TransformComponent& CameraTransform, const CameraComponent& Camera)
    {
        UpdateSpectrumTexture();

        RunFft(_hDxTexture);
        RunFft(_dzDfhxTexture);
        RunFft(_dfhzDfdxxTexture);
        RunFft(_dfdzxDfdzzTexture);

        AssembleTextures();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        const glm::vec3 cameraPosition = CameraTransform.GetWorldPosition();
        const float quadSize = _oceanGrid.GetQuadSize();
        const glm::vec2 gridOrigin =
                glm::floor(glm::vec2(cameraPosition.x, cameraPosition.z) / quadSize) * quadSize;

        _waterShader.Use();
        Shader::SetUniform(_viewMatrixLocation, glm::inverse(CameraTransform.GetLocalToWorldMatrix()));
        Shader::SetUniform(_projectionMatrixLocation, Camera.ProjectionMatrix);
        Shader::SetUniform(_cameraPositionLocation, cameraPosition);
        Shader::SetUniform(_timeLocation, Platform::TimeManager::GetTime());
        Shader::SetUniform(_gridOriginLocation, gridOrigin);
        _waterShader.SetUniform("OceanSize", static_cast<float>(_oceanSize));

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, _displacementTexture);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, _normalTexture);

        _oceanGrid.Draw();

        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_BLEND);
    }

    void WaterSystem::UpdateInitialSpectrum() const
    {
        _initialSpectrumComputeShader.Use();
        _initialSpectrumComputeShader.SetUniform("TextureResolution", TextureResolution);
        _initialSpectrumComputeShader.SetUniform("CascadeSize", _oceanSize);

        constexpr uint32_t count = 2 * TextureResolution * TextureResolution;
        float* gaussianRnd = new float[count];
        std::random_device rndDevice;
        std::mt19937 rng(rndDevice());
        std::normal_distribution<float> dist(0.0f, 1.0f);
        for (uint32_t i = 0; i < count; ++i)
            gaussianRnd[i] = dist(rng);

        glBindTexture(GL_TEXTURE_2D, _gaussianNoiseTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, TextureResolution, TextureResolution, 0, GL_RG, GL_FLOAT,
                     gaussianRnd);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        glBindImageTexture(0, _initialSpectrumTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glBindImageTexture(1, _gaussianNoiseTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);

        glDispatchCompute(TextureResolution / WorkGroupDim, TextureResolution / WorkGroupDim, 1);
        glFinish();

        delete[] gaussianRnd;
    }

    void WaterSystem::UpdateSpectrumTexture() const
    {
        _spectrumComputeShader.Use();
        _spectrumComputeShader.SetUniform("TextureResolution", TextureResolution);
        _spectrumComputeShader.SetUniform("CascadeSize", _oceanSize);
        _spectrumComputeShader.SetUniform("Time", Platform::TimeManager::GetTime());

        glBindImageTexture(0, _hDxTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glBindImageTexture(1, _dzDfhxTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glBindImageTexture(2, _dfhzDfdxxTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glBindImageTexture(3, _dfdzxDfdzzTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glBindImageTexture(4, _initialSpectrumTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

        glDispatchCompute(TextureResolution / WorkGroupDim, TextureResolution / WorkGroupDim, 1);
        glFinish();
    }

    void WaterSystem::AssembleTextures() const
    {
        _assembleComputeShader.Use();
        _assembleComputeShader.SetUniform("OceanSize", static_cast<float>(_oceanSize));

        glBindImageTexture(0, _hDxTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        glBindImageTexture(1, _dzDfhxTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        glBindImageTexture(2, _dfhzDfdxxTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        glBindImageTexture(3, _dfdzxDfdzzTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        glBindImageTexture(4, _normalTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glBindImageTexture(5, _displacementTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

        glDispatchCompute(TextureResolution / WorkGroupDim, TextureResolution / WorkGroupDim, 1);
        glFinish();
    }

    void WaterSystem::RunFft(uint32_t& Texture)
    {
        _fftHorizontalShader.Use();
        _fftHorizontalShader.SetUniform("TotalCount", TextureResolution);

        uint32_t& tmp = _tmpTexture;

        bool swap = false;
        for (int p = 1; p < TextureResolution; p <<= 1)
        {
            glBindImageTexture(0, Texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(1, tmp, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
            _fftHorizontalShader.SetUniform("Subcount", p);

            glDispatchCompute(TextureResolution, TextureResolution / 2, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            std::swap(tmp, Texture);
            swap = !swap;
        }

        _fftVerticalShader.Use();
        _fftVerticalShader.SetUniform("TotalCount", TextureResolution);

        for (int p = 1; p < TextureResolution; p <<= 1)
        {
            glBindImageTexture(0, Texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(1, tmp, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
            _fftVerticalShader.SetUniform("subCount", p);

            glDispatchCompute(TextureResolution, TextureResolution / 2, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            std::swap(tmp, Texture);
            swap = !swap;
        }
        if (swap)
        {
            std::swap(tmp, Texture);
        }
    }

    void WaterSystem::CreateTexture(uint32_t* Id, const int32_t Resolution, const int32_t InternalFormat,
                                    const int32_t Format, const int32_t Type, const int32_t Filter, const int32_t Wrap)
    {
        glGenTextures(1, Id);
        glBindTexture(GL_TEXTURE_2D, *Id);
        glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, Resolution, Resolution, 0, Format, Type, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Wrap);
    }
}
