#include "WaterSystem.h"

#include <chrono>
#include <numbers>
#include <random>

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
                                 _normalComputeShader(ShaderBuilder().SetStage(
                                     "Assets/Shaders/Water/Normal.comp", ShaderStage::ComputeShader).Build()),
                                 _initialSpectrumComputeShader(ShaderBuilder().SetStage(
                                     "Assets/Shaders/Water/InitialSpectrum.comp",
                                     ShaderStage::ComputeShader).Build()),
                                 _spectrumComputeShader(ShaderBuilder().SetStage(
                                     "Assets/Shaders/Water/Spectrum.comp", ShaderStage::ComputeShader).Build())
    {
        CreateTexture(&_initialSpectrumTexture, TextureResolution, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_NEAREST,GL_REPEAT);
        CreateTexture(&_dxTexture, TextureResolution, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_NEAREST,GL_REPEAT);
        CreateTexture(&_dyTexture, TextureResolution, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_NEAREST,GL_REPEAT);
        CreateTexture(&_dzTexture, TextureResolution, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_NEAREST,GL_REPEAT);
        CreateTexture(&_spectrumTexture, TextureResolution, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_NEAREST,GL_REPEAT);
        CreateTexture(&_normalTexture, TextureResolution, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_LINEAR,GL_REPEAT);
        CreateTexture(&_displacementTexture, TextureResolution, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_LINEAR,GL_REPEAT);
        CreateTexture(&_tmpTexture, TextureResolution, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_NEAREST,GL_REPEAT);
        CreateTexture(&_gaussianNoiseTexture, TextureResolution, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_NEAREST,
                      GL_CLAMP_TO_BORDER);
        GenerateInitialSpectrum();
    }

    void WaterSystem::Render(const TransformComponent& CameraTransform, const CameraComponent& Camera)
    {
        UpdateSpectrumTexture();

        RunFft(_dyTexture);
        RunFft(_dxTexture);
        RunFft(_dzTexture);

        UpdateNormalAndDisplacement();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        const float currentTime
                = std::chrono::duration<float>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        const glm::vec3 cameraPosition = CameraTransform.GetWorldPosition();
        const float quadSize = _oceanGrid.GetQuadSize();
        const glm::vec2 gridOrigin =
                glm::floor(glm::vec2(cameraPosition.x, cameraPosition.z) / quadSize) * quadSize;

        _waterShader.Use();
        Shader::SetUniform(_viewMatrixLocation, glm::inverse(CameraTransform.GetLocalToWorldMatrix()));
        Shader::SetUniform(_projectionMatrixLocation, Camera.ProjectionMatrix);
        Shader::SetUniform(_cameraPositionLocation, cameraPosition);
        Shader::SetUniform(_timeLocation, currentTime);
        Shader::SetUniform(_gridOriginLocation, gridOrigin);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, _displacementTexture);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, _normalTexture);

        _oceanGrid.Draw();

        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_BLEND);
    }

    void WaterSystem::GenerateInitialSpectrum()
    {
        _initialSpectrumComputeShader.Use();
        _initialSpectrumComputeShader.SetUniform("u_resolution", TextureResolution);
        _initialSpectrumComputeShader.SetUniform("u_ocean_size", _oceanSize);
        _initialSpectrumComputeShader.SetUniform("u_amplitude", _amplitude);

        float wind_angle_rad = std::numbers::pi_v<float> * 30 / 180.f;
        constexpr float windMagnitude = 50.0f;
        _initialSpectrumComputeShader.SetUniform(
            "u_wind", glm::vec2(windMagnitude * cos(wind_angle_rad), windMagnitude * sin(wind_angle_rad)));

        constexpr uint32_t count = 4 * TextureResolution * TextureResolution;
        float* gaussianRnd = new float[count];
        std::random_device rndDevice;
        std::mt19937 rng(rndDevice());
        std::normal_distribution<float> dist(0.0f, 1.0f);
        for (uint32_t i = 0; i < count; ++i)
            gaussianRnd[i] = dist(rng);

        glBindTexture(GL_TEXTURE_2D, _gaussianNoiseTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TextureResolution, TextureResolution, 0, GL_RGBA, GL_FLOAT,
                     gaussianRnd);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        glBindImageTexture(0, _initialSpectrumTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glBindImageTexture(1, _gaussianNoiseTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

        glDispatchCompute(TextureResolution / WorkGroupDim, TextureResolution / WorkGroupDim, 1);
        glFinish();

        delete[] gaussianRnd;
    }

    void WaterSystem::UpdateSpectrumTexture() const
    {
        _spectrumComputeShader.Use();
        _spectrumComputeShader.SetUniform("u_resolution", TextureResolution);
        _spectrumComputeShader.SetUniform("u_ocean_size", _oceanSize);
        _spectrumComputeShader.SetUniform("u_choppiness", 1.5f);
        const float currentTime
                = std::chrono::duration<float>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        _spectrumComputeShader.SetUniform("u_time", currentTime);

        glBindImageTexture(0, _initialSpectrumTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        glBindImageTexture(1, _dyTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glBindImageTexture(2, _dxTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glBindImageTexture(3, _dzTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

        glDispatchCompute(TextureResolution / WorkGroupDim, TextureResolution / WorkGroupDim, 1);
        glFinish();
    }

    void WaterSystem::UpdateNormalAndDisplacement() const
    {
        _normalComputeShader.Use();

        glBindImageTexture(0, _dyTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        glBindImageTexture(1, _dxTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        glBindImageTexture(2, _dzTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        glBindImageTexture(3, _normalTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glBindImageTexture(4, _displacementTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

        glDispatchCompute(TextureResolution / WorkGroupDim, TextureResolution / WorkGroupDim, 1);
        glFinish();
    }

    void WaterSystem::RunFft(uint32_t& Texture)
    {
        _fftHorizontalShader.Use();
        _fftHorizontalShader.SetUniform("u_total_count", TextureResolution);

        uint32_t& tmp = _tmpTexture;

        bool swap = false;
        for (int p = 1; p < TextureResolution; p <<= 1)
        {
            glBindImageTexture(0, Texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(1, tmp, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
            _fftHorizontalShader.SetUniform("u_subseq_count", p);

            glDispatchCompute(TextureResolution, TextureResolution / 2, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            std::swap(tmp, Texture);
            swap = !swap;
        }

        _fftVerticalShader.Use();
        _fftVerticalShader.SetUniform("u_total_count", TextureResolution);

        for (int p = 1; p < TextureResolution; p <<= 1)
        {
            glBindImageTexture(0, Texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(1, tmp, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
            _fftVerticalShader.SetUniform("u_subseq_count", p);

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
