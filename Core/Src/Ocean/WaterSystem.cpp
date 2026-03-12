#include "WaterSystem.h"

#include <chrono>
#include <iostream>
#include <numbers>
#include <random>

#include "TimeManager.h"
#include "Meshes/MeshManager.h"
#include "Ocean/Components/WaterEffectorComponent.h"
#include "Rendering/Shaders/ShaderBuilder.h"
#include "spdlog/spdlog.h"

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

        constexpr float DefaultFoamThreshold = 2.0f;
        constexpr float DefaultFoamExponent = 4.45f;
        constexpr float DefaultFoamIntensity = 2.0f;

        constexpr float DefaultTesselationMaxDistance = 50.0f;
        constexpr float DefaultTesselationMinDistance = 10.0f;
        constexpr float DefaultMaxTesselationLevel = 32.0f;
        constexpr float DefaultMinTesselationLevel = 1.0f;

        constexpr glm::vec3 DefaultSurfaceColor{0.005f, 0.1f, 0.2f};
        constexpr float DefaultMetallic = 0.05f;
        constexpr glm::vec3 DefaultAbsorption{0.5f, 0.2f, 0.1f};
        constexpr float DefaultRoughness = 0.5f;
        constexpr glm::vec3 DefaultFoamColor{1.0f, 1.0f, 1.0f};
        constexpr float DefaultFoamMetallic = 0.0f;
        constexpr float DefaultFoamRoughness = 1.0f;

        constexpr float DefaultSssDistortion = 1.1f;
        constexpr float DefaultSssPower = 3.0f;
        constexpr float DefaultSssScale = 1.0f;
        constexpr glm::vec3 DefaultSssColor{0.0f, 0.556f, 0.396f};
        constexpr float DefaultSssThreshold = 1.1f;
    }

    WaterSystem::WaterShaderUniforms::WaterShaderUniforms(const float TesselationMaxDistance,
                                                          const float TesselationMinDistance,
                                                          const float MaxTesselationLevel,
                                                          const float MinTesselationLevel,
                                                          const glm::vec3& SurfaceColor,
                                                          const float Metallic,
                                                          const glm::vec3& Absorption,
                                                          const float Roughness,
                                                          const glm::vec3& FoamColor,
                                                          const float FoamMetallic,
                                                          const float FoamRoughness,
                                                          const float SssDistortion,
                                                          const float SssPower,
                                                          const float SssScale,
                                                          const glm::vec3& SssColor,
                                                          const float SssThreshold)
        : ViewMatrix(),
          ProjectionMatrix(),
          CameraNearPlane(0.0f),
          CameraFarPlane(0.0f),
          MinLodDistance(0.0f),
          PatchSubdivisions(0),
          CameraPosition(),
          TesselationMaxDistance(TesselationMaxDistance),
          TesselationMinDistance(TesselationMinDistance),
          MaxTesselationLevel(MaxTesselationLevel),
          MinTesselationLevel(MinTesselationLevel),
          SurfaceColor(SurfaceColor),
          Metallic(Metallic),
          Absorption(Absorption),
          Roughness(Roughness),
          FoamColor(FoamColor),
          FoamMetallic(FoamMetallic),
          FoamRoughness(FoamRoughness),
          SssDistortion(SssDistortion),
          SssPower(SssPower),
          SssScale(SssScale),
          SssColor(SssColor),
          SssThreshold(SssThreshold)
    {
    }

    WaterSystem::FftShaderUniformLocations::FftShaderUniformLocations(const Shader FftShader)
        : TotalCount(FftShader.GetUniformLocation("TotalCount")),
          Subcount(FftShader.GetUniformLocation("Subcount"))
    {
    }

    WaterSystem::SpectrumShaderUniformLocations::SpectrumShaderUniformLocations(const Shader SpectrumShader)
        : Time(SpectrumShader.GetUniformLocation("Time"))
    {
    }

    WaterSystem::WaterSystem(const glm::ivec2& WindowSize, const uint32_t MainDepthBuffer)
        : _sprayManager(WindowSize, MainDepthBuffer),
          _waterNearShader(
              ShaderBuilder().SetStage("Assets/Shaders/Water/WaterNear.vert", ShaderStage::VertexShader).
              SetStage("Assets/Shaders/Water/Water.tesc", ShaderStage::TesselationControlShader).
              SetStage("Assets/Shaders/Water/Water.tese", ShaderStage::TesselationEvaluationShader).
              SetStage("Assets/Shaders/Water/Water.frag", ShaderStage::FragmentShader)
              .Build()),
          _waterFarShader(
              ShaderBuilder().SetStage("Assets/Shaders/Water/WaterFar.vert", ShaderStage::VertexShader).
              SetStage("Assets/Shaders/Water/Water.frag", ShaderStage::FragmentShader)
              .Build()),
          _fftHorizontalShader(
              ShaderBuilder().SetStage("Assets/Shaders/Water/Spectrum/FftHorizontal.comp", ShaderStage::ComputeShader)
              .Build()),
          _fftVerticalShader(
              ShaderBuilder().SetStage("Assets/Shaders/Water/Spectrum/FftVertical.comp", ShaderStage::ComputeShader)
              .Build()),
          _assembleComputeShader(
              ShaderBuilder().SetStage("Assets/Shaders/Water/Spectrum/Assemble.comp", ShaderStage::ComputeShader)
              .Build()),
          _initialSpectrumComputeShader(
              ShaderBuilder().SetStage("Assets/Shaders/Water/Spectrum/InitialSpectrum.comp",
                                       ShaderStage::ComputeShader)
              .Build()),
          _spectrumComputeShader(
              ShaderBuilder().SetStage("Assets/Shaders/Water/Spectrum/Spectrum.comp", ShaderStage::ComputeShader)
              .Build()),
          _fftHorizontalShaderUniformLocations(_fftHorizontalShader),
          _fftVerticalShaderUniformLocations(_fftVerticalShader),
          _spectrumShaderUniformLocations(_spectrumComputeShader),
          _initialSpectrumTextures(CreateTextureArray(CascadeCount)),
          _hDzDxDfhxTextures(CreateTextureArray(CascadeCount)),
          _dfhzDfdxxDfdzxDfdzz(CreateTextureArray(CascadeCount)),
          _velocityPackedTextures(CreateTextureArray(VelocityPackedTexturesCount * CascadeCount)),
          _slopeTextures(CreateTextureArrayWithMipMapping()),
          _displacementTextures(CreateTextureArrayWithMipMapping()),
          _velocityTextures(CreateTextureArray(VelocityLayers * CascadeCount)),
          _tmpTextures(CreateTextureArray(VelocityPackedTexturesCount * CascadeCount)),
          _gaussianNoiseTexture(CreateGaussianNoise()),
          _foamUniformBufferData(DefaultFoamThreshold, DefaultFoamExponent, DefaultFoamIntensity),
          _waterShaderUniforms(DefaultTesselationMaxDistance, DefaultTesselationMinDistance,
                               DefaultMaxTesselationLevel, DefaultMinTesselationLevel,
                               DefaultSurfaceColor, DefaultMetallic,
                               DefaultAbsorption, DefaultRoughness,
                               DefaultFoamColor, DefaultFoamMetallic,
                               DefaultFoamRoughness, DefaultSssDistortion,
                               DefaultSssPower, DefaultSssScale,
                               DefaultSssColor, DefaultSssThreshold),
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
          _waterSettingsUbo(CreateWaterSettingsUbo(_waterShaderUniforms)),
          _cascadeSettingsSsbo(CreateCascadeSettingsSsbo(_cascadeSettings))
    {
        UpdateInitialSpectrum();
        spdlog::info("Gauss: {}", _gaussianNoiseTexture.Get());
        spdlog::info("InitialSpectrum: {}", _initialSpectrumTextures.Get());
        spdlog::info("TimeSpectrum1: {}", _hDzDxDfhxTextures.Get());
        spdlog::info("TimeSpectrum2: {}", _hDzDxDfhxTextures.Get());
    }

    GlTextureUnique WaterSystem::CreateTextureArray(const int32_t Layers)
    {
        GlTextureUnique texture = GlTextureUnique::Create(GL_TEXTURE_2D_ARRAY);
        glTextureStorage3D(texture, 1, GL_RGBA32F, TextureResolution, TextureResolution, Layers);
        glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTextureParameteri(texture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        return texture;
    }

    GlTextureUnique WaterSystem::CreateTextureArrayWithMipMapping()
    {
        GlTextureUnique texture = GlTextureUnique::Create(GL_TEXTURE_2D_ARRAY);
        const int32_t mipLevels = static_cast<int32_t>(std::floorf(std::log2f(TextureResolution))) + 1;
        glTextureStorage3D(texture, mipLevels, GL_RGBA32F, TextureResolution, TextureResolution, CascadeCount);
        glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTextureParameteri(texture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        return texture;
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

    GlTextureUnique WaterSystem::CreateGaussianNoise()
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

        GlTextureUnique texture = GlTextureUnique::Create(GL_TEXTURE_2D);
        glTextureStorage2D(texture, 1, GL_RG32F, TextureResolution, TextureResolution);
        glTextureSubImage2D(texture, 0, 0, 0, TextureResolution, TextureResolution, GL_RG, GL_FLOAT, textureData);
        glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        delete[] textureData;
        return texture;
    }

    GlBufferUnique WaterSystem::CreateInitialSpectrumSettingsUbo(const InitialSpectrumSettingsBuffer& InitialData)
    {
        GlBufferUnique buffer = GlBufferUnique::Create();
        glBindBuffer(GL_UNIFORM_BUFFER, buffer);
        glNamedBufferStorage(buffer, sizeof(InitialSpectrumSettingsBuffer), &InitialData,
                             GL_DYNAMIC_STORAGE_BIT);
        return buffer;
    }

    GlBufferUnique WaterSystem::CreateFoamMaskSettingsUbo(const FoamMaskShaderUniforms& InitialData)
    {
        GlBufferUnique buffer = GlBufferUnique::Create();
        glNamedBufferStorage(buffer, sizeof(FoamMaskShaderUniforms), &InitialData, GL_DYNAMIC_STORAGE_BIT);
        return buffer;
    }

    GlBufferUnique WaterSystem::CreateWaterSettingsUbo(const WaterShaderUniforms& InitialData)
    {
        GlBufferUnique buffer = GlBufferUnique::Create();
        glNamedBufferStorage(buffer, sizeof(WaterShaderUniforms), &InitialData, GL_DYNAMIC_STORAGE_BIT);
        return buffer;
    }

    GlBufferUnique WaterSystem::CreateCascadeSettingsSsbo(const CascadeSettingsBuffer& InitialData)
    {
        GlBufferUnique buffer = GlBufferUnique::Create();
        glNamedBufferData(buffer, InitialData.GetDataSize(), InitialData.GetData(), GL_STATIC_COPY);
        return buffer;
    }

    void WaterSystem::Render(const float DeltaTime,
                             const TransformComponent& CameraTransform,
                             const CameraComponent& Camera,
                             const glm::vec3& AmbientLightColor,
                             const uint32_t DynamicWavesSampleSsboId,
                             const uint32_t SceneColor,
                             const uint32_t SceneDepth)
    {
        if (_spectrumSettingsDirty)
        {
            SendInitialSpectrumSettingsToGpu();
            UpdateInitialSpectrum();
            _spectrumSettingsDirty = false;
        }
        if (_foamSettingsDirty)
        {
            SendFoamMaskSettingsToGpu();
            _foamSettingsDirty = false;
        }

        UpdateSpectrumTexture();

        RunFft(_hDzDxDfhxTextures, CascadeCount);
        RunFft(_dfhzDfdxxDfdzxDfdzz, CascadeCount);
        RunFft(_velocityPackedTextures, VelocityPackedTexturesCount * CascadeCount);

        AssembleTextures();

        glGenerateTextureMipmap(_slopeTextures);
        glGenerateTextureMipmap(_displacementTextures);
        const glm::vec3 cameraPosition = CameraTransform.GetWorldPosition();
        const glm::mat4 viewMatrix = glm::inverse(CameraTransform.GetLocalToWorldMatrix());

        _oceanGrid.UpdateBuffers(Camera.GetProjectionMatrix(), CameraTransform.GetWorldToLocalMatrix(), cameraPosition);

        _waterShaderUniforms.ViewMatrix = viewMatrix;
        _waterShaderUniforms.ProjectionMatrix = Camera.GetProjectionMatrix();
        _waterShaderUniforms.CameraNearPlane = Camera.GetNearPlane();
        _waterShaderUniforms.CameraFarPlane = Camera.GetFarPlane();
        _waterShaderUniforms.MinLodDistance = _oceanGrid.GetMinLodDistance();
        _waterShaderUniforms.PatchSubdivisions = OceanGrid::GetPatchSubdivisions();
        _waterShaderUniforms.CameraPosition = cameraPosition;

        glNamedBufferSubData(_waterSettingsUbo, 0, sizeof(WaterShaderUniforms), &_waterShaderUniforms);

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, _foamMaskSettingsUbo);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, _waterSettingsUbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _cascadeSettingsSsbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, DynamicWavesSampleSsboId);

        glBindTextureUnit(3, _displacementTextures);
        glBindTextureUnit(4, _slopeTextures);
        glBindTextureUnit(5, SceneColor);
        glBindTextureUnit(6, SceneDepth);

        _waterNearShader.Use();
        _oceanGrid.RenderNearPatches();
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, _foamMaskSettingsUbo);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, _waterSettingsUbo);
        _waterFarShader.Use();
        _oceanGrid.RenderFarPatches();

        _sprayManager.Update(DeltaTime,
                             _displacementTextures,
                             _slopeTextures,
                             _velocityTextures,
                             _cascadeSettingsSsbo,
                             _foamMaskSettingsUbo,
                             -_initialSpectrumSettingsBufferData.WindVelocity,
                             AmbientLightColor,
                             _waterShaderUniforms.FoamColor,
                             cameraPosition,
                             viewMatrix,
                             Camera.GetProjectionMatrix());
    }

    void WaterSystem::RenderSpray(const uint32_t TargetFrameBuffer) const
    {
        _sprayManager.Render(TargetFrameBuffer);
    }

    void WaterSystem::RenderWaterEffectorsDebug(const entt::registry& Registry, const glm::mat4& ViewMatrix,
                                                const glm::mat4& ProjectionMatrix)
    {
        _waterEffectorsRenderer.RenderDebug(Registry, ViewMatrix, ProjectionMatrix);
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
        glNamedBufferSubData(_initialSpectrumSettingsUbo, 0, sizeof(InitialSpectrumSettingsBuffer),
                             &_initialSpectrumSettingsBufferData);
    }

    void WaterSystem::SendFoamMaskSettingsToGpu() const
    {
        glNamedBufferSubData(_foamMaskSettingsUbo, 0,
                             static_cast<int64_t>(sizeof(FoamMaskShaderUniforms)),
                             &_foamUniformBufferData);
    }

    void WaterSystem::SendCascadeSettingsToGpu() const
    {
        glNamedBufferData(_cascadeSettingsSsbo, _cascadeSettings.GetDataSize(),
                          _cascadeSettings.GetData(), GL_STATIC_COPY);
    }
}
