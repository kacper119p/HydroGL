#include "WaterSystem.h"

#include <chrono>
#include <numbers>
#include <random>

#include "TimeManager.h"
#include "Rendering/Shaders/ShaderBuilder.h"

namespace Core
{
    namespace
    {
        constexpr int32_t WorkGroupDim2D = 8;

        constexpr float Gravity = 9.80665f;
        constexpr float Pi = std::numbers::pi_v<float>;

        constexpr glm::vec2 DefaultWindVelocity{16.0f, 16.0f};
        constexpr float DefaultFetch = 10000.0f;
        constexpr float DefaultDelta = 1.0f;
        constexpr float DefaultSwell = 0.5f;
    }


    WaterSystem::WaterSystem()
        : _oceanGrid(1000.0f, 100, 10),
          _waterShader(
              ShaderBuilder().SetStage("Assets/Shaders/Water/Water.vert", ShaderStage::VertexShader).
              SetStage("Assets/Shaders/Water/Water.tesc", ShaderStage::TesselationControlShader).
              SetStage("Assets/Shaders/Water/Water.tese", ShaderStage::TesselationEvaluationShader).
              SetStage("Assets/Shaders/Water/Water.frag", ShaderStage::FragmentShader).Build()),
          _viewMatrixLocation(_waterShader.GetUniformLocation("ViewMatrix")),
          _projectionMatrixLocation(_waterShader.GetUniformLocation("ProjectionMatrix")),
          _cameraPositionLocation(_waterShader.GetUniformLocation("CameraPosition")),
          _timeLocation(_waterShader.GetUniformLocation("Time")),
          _gridOriginLocation(_waterShader.GetUniformLocation("GridOrigin")),
          _fftHorizontalShader(
              ShaderBuilder().SetStage("Assets/Shaders/Water/FftHorizontal.comp", ShaderStage::ComputeShader).Build()),
          _fftVerticalShader(
              ShaderBuilder().SetStage("Assets/Shaders/Water/FftVertical.comp", ShaderStage::ComputeShader).Build()),
          _assembleComputeShader(
              ShaderBuilder().SetStage("Assets/Shaders/Water/Assemble.comp", ShaderStage::ComputeShader).Build()),
          _initialSpectrumComputeShader(
              ShaderBuilder().SetStage("Assets/Shaders/Water/InitialSpectrum.comp", ShaderStage::ComputeShader).
              Build()),
          _spectrumComputeShader(
              ShaderBuilder().SetStage("Assets/Shaders/Water/Spectrum.comp", ShaderStage::ComputeShader).Build()),
          _initialSpectrumTextures(
              CreateTextureArray(TextureResolution, CascadeCount, GL_RGBA32F, GL_RGBA, GL_FLOAT,GL_NEAREST, GL_REPEAT)),
          _hDxTextures(
              CreateTextureArray(TextureResolution, CascadeCount, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_NEAREST,GL_REPEAT)),
          _dzDfhxTextures(
              CreateTextureArray(TextureResolution, CascadeCount, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_NEAREST,GL_REPEAT)),
          _dfhzDfdxxTextures(
              CreateTextureArray(TextureResolution, CascadeCount, GL_RGBA32F, GL_RGBA, GL_FLOAT,GL_NEAREST,GL_REPEAT)),
          _dfdzxDfdzzTextures(
              CreateTextureArray(TextureResolution, CascadeCount, GL_RGBA32F, GL_RGBA, GL_FLOAT,GL_NEAREST,GL_REPEAT)),
          _spectrumTextures(
              CreateTextureArray(TextureResolution, CascadeCount, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_NEAREST,GL_REPEAT)),
          _slopeTextures(
              CreateTextureArray(TextureResolution, CascadeCount, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_LINEAR,GL_REPEAT)),
          _displacementTextures(
              CreateTextureArray(TextureResolution, CascadeCount, GL_RGBA32F, GL_RGBA, GL_FLOAT,GL_LINEAR, GL_REPEAT)),
          _tmpTextures(
              CreateTextureArray(TextureResolution, CascadeCount, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_NEAREST,GL_REPEAT)),
          _gaussianNoiseTexture(CreateGaussianNoise(TextureResolution)),
          _initialSpectrumSettingsBufferData(DefaultWindVelocity, DefaultFetch, DefaultDelta, DefaultSwell,
                                             CalculateInitialSpectrumAlpha(
                                                 DefaultWindVelocity, DefaultFetch, Gravity),
                                             CalculateInitialSpectrumPeakWaveFrequency(
                                                 DefaultWindVelocity, DefaultFetch, Gravity)),
          _cascadeSettings(TextureResolution, {
                               {1e-5f, 0.75f * Pi, 256},
                               {0.75f * Pi, 3.0f * Pi, 16},
                               {3.0f * Pi, std::numeric_limits<float>::max(), 4}
                           }),
          _initialSpectrumSettingsSsbo(CreateInitialSpectrumSettingsSsbo(_initialSpectrumSettingsBufferData)),
          _cascadeSettingsSsbo(CreateCascadeSettingsSsbo(_cascadeSettings))
    {
        UpdateInitialSpectrum();
    }

    uint32_t WaterSystem::CreateTextureArray(const int32_t Resolution, const int32_t Layers,
                                             const int32_t InternalFormat,
                                             const int32_t Format, const int32_t Type, const int32_t Filter,
                                             const int32_t Wrap)
    {
        uint32_t textureId;
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D_ARRAY, textureId);
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, InternalFormat, Resolution, Resolution, Layers, 0, Format, Type, nullptr);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, Filter);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, Filter);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, Wrap);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, Wrap);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        return textureId;
    }

    void WaterSystem::UpdateInitialSpectrum() const
    {
        _initialSpectrumComputeShader.Use();

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _cascadeSettingsSsbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _initialSpectrumSettingsSsbo);

        glBindImageTexture(0, _initialSpectrumTextures, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glBindImageTexture(1, _gaussianNoiseTexture, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);

        Shader::DispatchCompute(TextureResolution / WorkGroupDim2D, TextureResolution / WorkGroupDim2D, CascadeCount);
        glFinish();
    }

    uint32_t WaterSystem::CreateGaussianNoise(const int32_t Resolution)
    {
        uint32_t textureId;
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        const size_t count = 4 * Resolution * Resolution;
        float* textureData = new float[count];
        std::random_device rndDevice;
        std::mt19937 rng(rndDevice());
        std::normal_distribution<float> dist(0.0f, 1.0f);
        for (size_t i = 0; i < count; ++i)
        {
            textureData[i] = dist(rng);
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TextureResolution, TextureResolution, 0, GL_RGBA, GL_FLOAT,
                     textureData);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        delete[] textureData;
        return textureId;
    }

    uint32_t WaterSystem::CreateInitialSpectrumSettingsSsbo(const InitialSpectrumSettingsBuffer& InitialData)
    {
        uint32_t id;
        glGenBuffers(1, &id);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(InitialSpectrumSettingsBuffer), &InitialData,
                     GL_STATIC_COPY);
        return id;
    }

    uint32_t WaterSystem::CreateCascadeSettingsSsbo(const CascadeSettingsBuffer& InitialData)
    {
        uint32_t id;
        glGenBuffers(1, &id);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
        glBufferData(GL_SHADER_STORAGE_BUFFER, InitialData.GetDataSize(), InitialData.GetData(), GL_STATIC_COPY);
        return id;
    }

    void WaterSystem::Render(const TransformComponent& CameraTransform, const CameraComponent& Camera)
    {
        UpdateSpectrumTexture();

        RunFft(_hDxTextures);
        RunFft(_dzDfhxTextures);
        RunFft(_dfhzDfdxxTextures);
        RunFft(_dfdzxDfdzzTextures);

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

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _cascadeSettingsSsbo);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D_ARRAY, _displacementTextures);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D_ARRAY, _slopeTextures);

        _oceanGrid.Draw();

        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_BLEND);
    }

    void WaterSystem::UpdateSpectrumTexture() const
    {
        _spectrumComputeShader.Use();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _cascadeSettingsSsbo);
        _spectrumComputeShader.SetUniform("Time", Platform::TimeManager::GetTime());

        glBindImageTexture(0, _hDxTextures, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glBindImageTexture(1, _dzDfhxTextures, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glBindImageTexture(2, _dfhzDfdxxTextures, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glBindImageTexture(3, _dfdzxDfdzzTextures, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glBindImageTexture(4, _initialSpectrumTextures, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);

        Shader::DispatchCompute(TextureResolution / WorkGroupDim2D, TextureResolution / WorkGroupDim2D, CascadeCount);
        glFinish();
    }

    void WaterSystem::AssembleTextures() const
    {
        _assembleComputeShader.Use();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _cascadeSettingsSsbo);

        glBindImageTexture(0, _hDxTextures, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);
        glBindImageTexture(1, _dzDfhxTextures, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);
        glBindImageTexture(2, _dfhzDfdxxTextures, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);
        glBindImageTexture(3, _dfdzxDfdzzTextures, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);
        glBindImageTexture(4, _slopeTextures, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glBindImageTexture(5, _displacementTextures, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);

        Shader::DispatchCompute(TextureResolution / WorkGroupDim2D, TextureResolution / WorkGroupDim2D, CascadeCount);
        glFinish();
    }

    void WaterSystem::RunFft(uint32_t& Texture)
    {
        _fftHorizontalShader.Use();
        _fftHorizontalShader.SetUniform("TotalCount", TextureResolution);

        uint32_t& tmp = _tmpTextures;

        bool swap = false;
        for (int p = 1; p < TextureResolution; p <<= 1)
        {
            glBindImageTexture(0, Texture, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(1, tmp, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
            _fftHorizontalShader.SetUniform("Subcount", p);

            Shader::DispatchCompute(TextureResolution / WorkGroupDim2D, TextureResolution / WorkGroupDim2D,
                                    CascadeCount);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            std::swap(tmp, Texture);
            swap = !swap;
        }

        _fftVerticalShader.Use();
        _fftVerticalShader.SetUniform("TotalCount", TextureResolution);

        for (int p = 1; p < TextureResolution; p <<= 1)
        {
            glBindImageTexture(0, Texture, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(1, tmp, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
            _fftVerticalShader.SetUniform("subCount", p);

            Shader::DispatchCompute(TextureResolution / WorkGroupDim2D, TextureResolution / WorkGroupDim2D,
                                    CascadeCount);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            std::swap(tmp, Texture);
            swap = !swap;
        }
        if (swap)
        {
            std::swap(tmp, Texture);
        }
    }

    void WaterSystem::SendInitialSpectrumSettingsToGpu() const
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _initialSpectrumSettingsSsbo);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(InitialSpectrumSettingsBuffer),
                        &_initialSpectrumSettingsBufferData);
    }

    void WaterSystem::SendCascadeSettingsToGpu() const
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _cascadeSettingsSsbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, _cascadeSettings.GetDataSize(), _cascadeSettings.GetData(),
                     GL_STATIC_COPY);
    }
}
