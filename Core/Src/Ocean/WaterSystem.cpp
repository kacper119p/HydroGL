#include "WaterSystem.h"

#include <chrono>
#include <iostream>
#include <numbers>
#include <random>

#include "TimeManager.h"
#include "Rendering/Shaders/ShaderBuilder.h"

namespace Core
{
    namespace
    {
        constexpr int32_t WorkGroupDim2D = 8;

        constexpr float Pi = std::numbers::pi_v<float>;

        constexpr glm::vec2 DefaultWindVelocity{16.0f, 16.0f};
        constexpr float DefaultFetch = 10000.0f;
        constexpr float DefaultDelta = 1.0f;
        constexpr float DefaultSwell = 0.0f;

        constexpr float DefaultFoamThreshold = 0.8f;
        constexpr float DefaultFoamExponent = 1.0f;
        constexpr float DefaultFoamIntensity = 5.0f;
    }

    WaterSystem::WaterShaderUniformLocations::WaterShaderUniformLocations(const Shader WaterShader)
        : ViewMatrix(WaterShader.GetUniformLocation("ViewMatrix")),
          ProjectionMatrix(WaterShader.GetUniformLocation("ProjectionMatrix")),
          CameraPosition(WaterShader.GetUniformLocation("CameraPosition")),
          Time(WaterShader.GetUniformLocation("Time")),
          GridOrigin(WaterShader.GetUniformLocation("GridOrigin"))
    {
    }

    WaterSystem::FftShaderUniformLocations::FftShaderUniformLocations(Shader FftShader)
        : TotalCount(FftShader.GetUniformLocation("TotalCount")),
          Subcount(FftShader.GetUniformLocation("Subcount"))
    {
    }

    WaterSystem::SpectrumShaderUniformLocations::SpectrumShaderUniformLocations(const Shader SpectrumShader)
        : Time(SpectrumShader.GetUniformLocation("Time"))
    {
    }

    WaterSystem::WaterSystem()
        : _oceanGrid(1000.0f, 100, 10),
          _waterShader(
              ShaderBuilder().SetStage("Assets/Shaders/Water/Water.vert", ShaderStage::VertexShader).
              SetStage("Assets/Shaders/Water/Water.tesc", ShaderStage::TesselationControlShader).
              SetStage("Assets/Shaders/Water/Water.tese", ShaderStage::TesselationEvaluationShader).
              SetStage("Assets/Shaders/Water/Water.frag", ShaderStage::FragmentShader).Build()),
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
          _foamMaskComputeShader(
              ShaderBuilder().SetStage("Assets/Shaders/Water/FoamMask.comp", ShaderStage::ComputeShader).Build()),
          _waterShaderUniformLocations(_waterShader),
          _fftHorizontalShaderUniformLocations(_fftHorizontalShader),
          _fftVerticalShaderUniformLocations(_fftVerticalShader),
          _spectrumShaderUniformLocations(_spectrumComputeShader),
          _initialSpectrumTextures(CreateTextureArray(CascadeCount)),
          _hDzDxDfhxTextures(CreateTextureArray(CascadeCount)),
          _dfhzDfdxxDfdzxDfdzz(CreateTextureArray(CascadeCount)),
          _velocityPackedTextures(CreateTextureArray(VelocityPackedTexturesCount * CascadeCount)),
          _slopeTextures(CreateSlopeTextureArray()),
          _displacementTextures(CreateTextureArray(CascadeCount)),
          _velocityTextures(CreateTextureArray(VelocityLayers * CascadeCount)),
          _tmpTextures(CreateTextureArray(VelocityPackedTexturesCount * CascadeCount)),
          _gaussianNoiseTexture(CreateGaussianNoise()),
          _foamMaskTextures(CreateFoamTextureArray()),
          _foamUniformBufferData(DefaultFoamThreshold, DefaultFoamExponent, DefaultFoamIntensity),
          _initialSpectrumSettingsBufferData(DefaultWindVelocity, DefaultFetch, DefaultDelta, DefaultSwell,
                                             CalculateInitialSpectrumAlpha(
                                                 DefaultWindVelocity, DefaultFetch, Gravity),
                                             CalculateInitialSpectrumPeakWaveFrequency(
                                                 DefaultWindVelocity, DefaultFetch, Gravity)),
          _cascadeSettings(TextureResolution, VelocityLayers, VelocityMinHeight, VelocityMaxHeight, {
                               {1e-5f, 0.75f * Pi, 256.0f},
                               {0.75f * Pi, 3.0f * Pi, 16.0f},
                               {3.0f * Pi, std::numeric_limits<float>::max(), 4.0f}
                           }),
          _initialSpectrumSettingsUbo(CreateInitialSpectrumSettingsUbo(_initialSpectrumSettingsBufferData)),
          _foamMaskSettingsUbo(CreateFoamMaskSettingsUbo(_foamUniformBufferData)),
          _cascadeSettingsSsbo(CreateCascadeSettingsSsbo(_cascadeSettings))
    {
        UpdateInitialSpectrum();
    }

    uint32_t WaterSystem::CreateTextureArray(const int32_t Layers)
    {
        uint32_t textureId;
        glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &textureId);
        glTextureStorage3D(textureId, 1, GL_RGBA32F, TextureResolution, TextureResolution, Layers);
        glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        return textureId;
    }

    uint32_t WaterSystem::CreateSlopeTextureArray()
    {
        uint32_t textureId;
        glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &textureId);
        const int32_t mipLevels = static_cast<int32_t>(std::floorf(std::log2f(TextureResolution))) + 1;
        glTextureStorage3D(textureId, mipLevels, GL_RGBA32F, TextureResolution, TextureResolution, CascadeCount);
        glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        return textureId;
    }

    uint32_t WaterSystem::CreateFoamTextureArray()
    {
        void* initialData = calloc(TextureResolution * TextureResolution * CascadeCount, sizeof(float));
        uint32_t textureId;
        glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &textureId);
        glTextureStorage3D(textureId, 1, GL_R32F, TextureResolution, TextureResolution, CascadeCount);
        glTextureSubImage3D(textureId, 0, 0, 0, 0, TextureResolution, TextureResolution, CascadeCount, GL_RED, GL_FLOAT,
                            initialData);
        glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        free(initialData);
        return textureId;
    }

    void WaterSystem::UpdateInitialSpectrum() const
    {
        _initialSpectrumComputeShader.Use();

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _cascadeSettingsSsbo);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, _initialSpectrumSettingsUbo);

        glBindImageTexture(0, _initialSpectrumTextures, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glBindImageTexture(1, _gaussianNoiseTexture, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG32F);

        Shader::DispatchCompute(TextureResolution / WorkGroupDim2D, TextureResolution / WorkGroupDim2D, CascadeCount);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
    }

    uint32_t WaterSystem::CreateGaussianNoise()
    {
        constexpr size_t count = 2 * TextureResolution * TextureResolution;
        float* textureData = new float[count];
        std::random_device rndDevice;
        std::mt19937 rng(rndDevice());
        std::normal_distribution<float> dist(0.0f, 1.0f);
        for (size_t i = 0; i < count; ++i)
        {
            textureData[i] = dist(rng);
        }

        uint32_t textureId;
        glCreateTextures(GL_TEXTURE_2D, 1, &textureId);
        glTextureStorage2D(textureId, 1, GL_RG32F, TextureResolution, TextureResolution);
        glTextureSubImage2D(textureId, 0, 0, 0, TextureResolution, TextureResolution, GL_RG, GL_FLOAT, textureData);
        glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        delete[] textureData;
        return textureId;
    }

    uint32_t WaterSystem::CreateInitialSpectrumSettingsUbo(const InitialSpectrumSettingsBuffer& InitialData)
    {
        uint32_t id;
        glGenBuffers(1, &id);
        glBindBuffer(GL_UNIFORM_BUFFER, id);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(InitialSpectrumSettingsBuffer), &InitialData,
                     GL_STATIC_COPY);
        return id;
    }

    uint32_t WaterSystem::CreateFoamMaskSettingsUbo(const FoamMaskShaderUniforms& InitialData)
    {
        uint32_t id;
        glCreateBuffers(1, &id);
        glNamedBufferStorage(id, sizeof(InitialSpectrumSettingsBuffer), &InitialData, GL_DYNAMIC_STORAGE_BIT);
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

    void WaterSystem::Render(const TransformComponent& CameraTransform, const CameraComponent& Camera,
                             const uint32_t DynamicWavesSampleSsboId) const
    {
        UpdateSpectrumTexture();

        RunFft(_hDzDxDfhxTextures, CascadeCount);
        RunFft(_dfhzDfdxxDfdzxDfdzz, CascadeCount);
        RunFft(_velocityPackedTextures, VelocityPackedTexturesCount * CascadeCount);

        AssembleTextures();
        UpdateFoamMask();

        glGenerateTextureMipmap(_slopeTextures);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        const glm::vec3 cameraPosition = CameraTransform.GetWorldPosition();
        const float quadSize = _oceanGrid.GetQuadSize();
        const glm::vec2 gridOrigin =
                glm::floor(glm::vec2(cameraPosition.x, cameraPosition.z) / quadSize) * quadSize;

        _waterShader.Use();
        _waterShader.SetUniform(_waterShaderUniformLocations.ViewMatrix,
                                glm::inverse(CameraTransform.GetLocalToWorldMatrix()));
        _waterShader.SetUniform(_waterShaderUniformLocations.ProjectionMatrix, Camera.ProjectionMatrix);
        _waterShader.SetUniform(_waterShaderUniformLocations.CameraPosition, cameraPosition);
        _waterShader.SetUniform(_waterShaderUniformLocations.Time, Platform::TimeManager::GetTime());
        _waterShader.SetUniform(_waterShaderUniformLocations.GridOrigin, gridOrigin);

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, _foamMaskSettingsUbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _cascadeSettingsSsbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, DynamicWavesSampleSsboId);

        glBindTextureUnit(3, _displacementTextures);
        glBindTextureUnit(4, _slopeTextures);

        _oceanGrid.Draw();

        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_BLEND);
    }

    void WaterSystem::UpdateSpectrumTexture() const
    {
        _spectrumComputeShader.Use();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _cascadeSettingsSsbo);
        _spectrumComputeShader.SetUniform(_spectrumShaderUniformLocations.Time, Platform::TimeManager::GetTime());

        glBindImageTexture(0, _hDzDxDfhxTextures, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glBindImageTexture(1, _dfhzDfdxxDfdzxDfdzz, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glBindImageTexture(2, _velocityPackedTextures, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glBindImageTexture(3, _initialSpectrumTextures, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);

        Shader::DispatchCompute(TextureResolution / WorkGroupDim2D, TextureResolution / WorkGroupDim2D, CascadeCount);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
    }

    void WaterSystem::AssembleTextures() const
    {
        _assembleComputeShader.Use();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _cascadeSettingsSsbo);

        glBindImageTexture(0, _hDzDxDfhxTextures, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);
        glBindImageTexture(1, _dfhzDfdxxDfdzxDfdzz, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);
        glBindImageTexture(2, _velocityPackedTextures, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);
        glBindImageTexture(3, _slopeTextures, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glBindImageTexture(4, _displacementTextures, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glBindImageTexture(5, _velocityTextures, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);

        Shader::DispatchCompute(TextureResolution / WorkGroupDim2D, TextureResolution / WorkGroupDim2D, CascadeCount);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

    void WaterSystem::UpdateFoamMask() const
    {
        _foamMaskComputeShader.Use();

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _cascadeSettingsSsbo);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, _foamMaskSettingsUbo);

        glBindTextureUnit(0, _velocityTextures);
        glBindTextureUnit(1, _slopeTextures);
        glBindTextureUnit(2, _displacementTextures);

        glBindImageTexture(3, _foamMaskTextures, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);

        Shader::DispatchCompute(TextureResolution / WorkGroupDim2D, TextureResolution / WorkGroupDim2D, CascadeCount);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

    void WaterSystem::RunFft(uint32_t Texture, const int32_t Layers) const
    {
        _fftHorizontalShader.Use();
        _fftHorizontalShader.SetUniform(_fftHorizontalShaderUniformLocations.TotalCount, TextureResolution);

        uint32_t tmp = _tmpTextures;

        bool swap = false;
        for (int p = 1; p < TextureResolution; p <<= 1)
        {
            glBindImageTexture(0, Texture, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(1, tmp, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
            _fftHorizontalShader.SetUniform(_fftHorizontalShaderUniformLocations.Subcount, p);

            Shader::DispatchCompute(TextureResolution / WorkGroupDim2D / 2, TextureResolution / WorkGroupDim2D,
                                    Layers);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

            std::swap(tmp, Texture);
            swap = !swap;
        }

        _fftVerticalShader.Use();
        _fftVerticalShader.SetUniform(_fftVerticalShaderUniformLocations.TotalCount, TextureResolution);

        for (int p = 1; p < TextureResolution; p <<= 1)
        {
            glBindImageTexture(0, Texture, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(1, tmp, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
            _fftVerticalShader.SetUniform(_fftVerticalShaderUniformLocations.Subcount, p);

            Shader::DispatchCompute(TextureResolution / WorkGroupDim2D / 2, TextureResolution / WorkGroupDim2D,
                                    Layers);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

            std::swap(tmp, Texture);
            swap = !swap;
        }
        if (swap)
        {
            glCopyImageSubData(Texture, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0,
                               tmp, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0,
                               TextureResolution, TextureResolution, Layers);
        }
    }

    void WaterSystem::SendInitialSpectrumSettingsToGpu() const
    {
        glNamedBufferSubData(_initialSpectrumSettingsUbo, 0, sizeof(_initialSpectrumSettingsBufferData),
                             &_initialSpectrumSettingsBufferData);
    }

    void WaterSystem::SentFoamMaskSettingsToGpu() const
    {
        constexpr int64_t decayRateOffset = offsetof(FoamMaskShaderUniforms, FoamMaskShaderUniforms::FoamThreshold);
        glNamedBufferSubData(_initialSpectrumSettingsUbo, decayRateOffset,
                             static_cast<int64_t>(sizeof(_initialSpectrumSettingsBufferData)),
                             &_foamUniformBufferData);
    }

    void WaterSystem::SendCascadeSettingsToGpu() const
    {
        glNamedBufferData(_cascadeSettingsSsbo, _cascadeSettings.GetDataSize(),
                          _cascadeSettings.GetData(), GL_STATIC_COPY);
    }
}
