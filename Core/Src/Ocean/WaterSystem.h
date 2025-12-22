#pragma once
#include "Camera/CameraComponent.h"
#include "Ocean/OceanGrid.h"
#include "Ocean/SprayManager.h"
#include "Rendering/Shaders/Shader.h"
#include "Scene/TransformComponent.h"
#include "Utility/GlResourceUnique.h"

namespace Core
{
    class WaterSystem
    {
    public:
        struct alignas(16) CascadeSettings
        {
        public:
            float WaveLengthLowerBound;
            alignas(4) float WaveLengthUpperBound;
            alignas(4) float Length;

        private:
            alignas(4) float Padding{};

        public:
            CascadeSettings() : WaveLengthLowerBound(0.0f), WaveLengthUpperBound(0.0f), Length(0)
            {
            }

            CascadeSettings(const float WaveLengthLowerBound, const float WaveLengthUpperBound, const float Length)
                : WaveLengthLowerBound(WaveLengthLowerBound),
                  WaveLengthUpperBound(WaveLengthUpperBound),
                  Length(Length)
            {
            }
        };

    private:
        struct alignas(16) FoamMaskShaderUniforms
        {
        public:
            alignas(4) float FoamThreshold;
            alignas(4) float FoamExponent;
            alignas(4) float FoamIntensity;

        public:
            FoamMaskShaderUniforms(const float FoamThreshold,
                                   const float FoamExponent, const float FoamIntensity)
                : FoamThreshold(FoamThreshold),
                  FoamExponent(FoamExponent),
                  FoamIntensity(FoamIntensity)
            {
            }
        };

        struct alignas(16) InitialSpectrumSettingsBuffer
        {
        public:
            glm::vec2 WindVelocity;
            alignas(8) float Fetch;
            alignas(4) float Delta;
            alignas(4) float Swell;
            alignas(4) float Alpha;
            alignas(4) float PeakWaveFrequency;

        private:
            alignas(4) float Padding{};

        public:
            InitialSpectrumSettingsBuffer(const glm::vec2& WindVelocity, const float Fetch, const float Delta,
                                          const float Swell, const float Alpha, const float PeakWaveFrequency)
                : WindVelocity(WindVelocity),
                  Fetch(Fetch),
                  Delta(Delta),
                  Swell(Swell),
                  Alpha(Alpha),
                  PeakWaveFrequency(PeakWaveFrequency)
            {
            }
        };

        struct alignas(16) WaterShaderUniforms
        {
            alignas(16) glm::mat4 ViewMatrix;
            alignas(16) glm::mat4 ProjectionMatrix;

            alignas(16) glm::vec2 GridOrigin;
            alignas(4) float CameraNearPlane;
            alignas(4) float CameraFarPlane;
            alignas(16) glm::vec3 CameraPosition;

            alignas(4) float TesselationMaxDistance;
            alignas(4) float TesselationMinDistance;
            alignas(4) float MaxTesselationLevel;
            alignas(4) float MinTesselationLevel;

            alignas(16) glm::vec3 SurfaceColor;
            alignas(4) float Metallic;
            alignas(16) glm::vec3 Absorption;
            alignas(4) float Roughness;
            alignas(16) glm::vec3 FoamColor;
            alignas(4) float FoamMetallic;
            alignas(4) float FoamRoughness;

            alignas(4) float SssDistortion;
            alignas(4) float SssPower;
            alignas(4) float SssScale;
            alignas(16) glm::vec3 SssColor;
            alignas(4) float SssThreshold;

            WaterShaderUniforms(float TesselationMaxDistance,
                                float TesselationMinDistance,
                                float MaxTesselationLevel,
                                float MinTesselationLevel,
                                const glm::vec3& SurfaceColor,
                                float Metallic,
                                const glm::vec3& Absorption,
                                float Roughness,
                                const glm::vec3& FoamColor,
                                float FoamMetallic,
                                float FoamRoughness,
                                float SssDistortion,
                                float SssPower,
                                float SssScale,
                                const glm::vec3& SssColor,
                                float SssThreshold);
        };

        class CascadeSettingsBuffer
        {
        private:
            struct InternalBuffer
            {
                int32_t TextureResolution{};
                int32_t CascadeCount{};
                int32_t VelocityLayers{};
                float VelocityMinHeight{};
                float VelocityMaxHeight{};
                float Padding[3]{};
                CascadeSettings CascadeSettings[1]{};

                static constexpr int64_t CalculateSize(const uint32_t Count)
                {
                    return offsetof(InternalBuffer, CascadeSettings) + Count * sizeof(CascadeSettings);
                }
            };

        private:
            uint32_t _capacity;
            InternalBuffer* _buffer;

        public:
            explicit CascadeSettingsBuffer(const int32_t TextureResolution)
                : _capacity(3),
                  _buffer(static_cast<InternalBuffer*>(malloc(InternalBuffer::CalculateSize(_capacity))))
            {
                _buffer->TextureResolution = TextureResolution;
                _buffer->CascadeCount = 0;
            }

            CascadeSettingsBuffer(const int32_t TextureResolution, const int32_t VelocityLayers,
                                  const float VelocityMinHeight, const float VelocityMaxHeight,
                                  const std::initializer_list<CascadeSettings>&
                                  InitialValues) : _capacity(InitialValues.size())
            {
                _buffer = static_cast<InternalBuffer*>(malloc(InternalBuffer::CalculateSize(_capacity)));
                _buffer->TextureResolution = TextureResolution;
                _buffer->CascadeCount = static_cast<int32_t>(InitialValues.size());
                _buffer->VelocityLayers = VelocityLayers;
                _buffer->VelocityMinHeight = VelocityMinHeight;
                _buffer->VelocityMaxHeight = VelocityMaxHeight;
                memcpy(_buffer->CascadeSettings, InitialValues.begin(), InitialValues.size() * sizeof(CascadeSettings));
            }

        public:
            ~CascadeSettingsBuffer()
            {
                free(_buffer);
            }

            [[nodiscard]] int32_t GetTextureResolution() const
            {
                return _buffer->TextureResolution;
            }

            void SetTextureResolution(const int32_t TextureResolution)
            {
                _buffer->TextureResolution = TextureResolution;
            }

            [[nodiscard]] void* GetData() const
            {
                return _buffer;
            }

            [[nodiscard]] int64_t GetDataSize() const
            {
                return InternalBuffer::CalculateSize(_buffer->CascadeCount);
            }

            [[nodiscard]] uint32_t GetCascadeCount() const
            {
                return _buffer->CascadeCount;
            }

        public:
            void PushBack(const CascadeSettings& Settings)
            {
                ++_buffer->CascadeCount;
                if (_buffer->CascadeCount > _capacity)
                {
                    _buffer = static_cast<InternalBuffer*>(realloc(
                        _buffer, InternalBuffer::CalculateSize(_buffer->CascadeCount)));
                    _capacity = _buffer->CascadeCount;
                }
                _buffer->CascadeSettings[_buffer->CascadeCount - 1] = Settings;
            }

            void PopBack()
            {
                if (_buffer->CascadeCount > 0)
                {
                    --_buffer->CascadeCount;
                }
            }
        };

        struct FftShaderUniformLocations
        {
            int32_t TotalCount;
            int32_t Subcount;

            explicit FftShaderUniformLocations(Shader FftShader);
        };

        struct SpectrumShaderUniformLocations
        {
            int32_t Time;

            explicit SpectrumShaderUniformLocations(Shader SpectrumShader);
        };

    private:
        static constexpr float Gravity = 9.80665f;
        static constexpr int32_t CascadeCount = 3;
        static constexpr int32_t TextureResolution = 512; // Must be power of 2
        static constexpr int32_t VelocityLayers = 8; // Must be multiple of 4
        static constexpr int32_t VelocityPackedTexturesCount = VelocityLayers * 3 / 4;
        static constexpr float VelocityMinHeight = -40.0f;
        static constexpr float VelocityMaxHeight = 0.0f;

    private:
        OceanGrid _oceanGrid;
        SprayManager _sprayManager;

        Shader _waterShader;
        Shader _fftHorizontalShader;
        Shader _fftVerticalShader;
        Shader _assembleComputeShader;
        Shader _orientationComputeShader;
        Shader _initialSpectrumComputeShader;
        Shader _spectrumComputeShader;

        FftShaderUniformLocations _fftHorizontalShaderUniformLocations;
        FftShaderUniformLocations _fftVerticalShaderUniformLocations;
        SpectrumShaderUniformLocations _spectrumShaderUniformLocations;

        GlTextureUnique _initialSpectrumTextures;
        GlTextureUnique _hDzDxDfhxTextures;
        GlTextureUnique _dfhzDfdxxDfdzxDfdzz;
        GlTextureUnique _velocityPackedTextures;
        GlTextureUnique _slopeTextures;
        GlTextureUnique _displacementTextures;
        GlTextureUnique _velocityTextures;
        GlTextureUnique _tmpTextures;
        GlTextureUnique _gaussianNoiseTexture;

        FoamMaskShaderUniforms _foamUniformBufferData;
        WaterShaderUniforms _waterShaderUniforms;
        InitialSpectrumSettingsBuffer _initialSpectrumSettingsBufferData;
        CascadeSettingsBuffer _cascadeSettings;

        GlBufferUnique _initialSpectrumSettingsUbo;
        GlBufferUnique _foamMaskSettingsUbo;
        GlBufferUnique _waterSettingsUbo;
        GlBufferUnique _cascadeSettingsSsbo;

    public:
        WaterSystem(const glm::ivec2& WindowSize, uint32_t MainDepthBuffer);

    private:
        static GlTextureUnique CreateTextureArray(int32_t Layers);

        static GlTextureUnique CreateSlopeTextureArray();

        static GlTextureUnique CreateGaussianNoise();

        static GlBufferUnique CreateInitialSpectrumSettingsUbo(const InitialSpectrumSettingsBuffer& InitialData);

        static GlBufferUnique CreateFoamMaskSettingsUbo(const FoamMaskShaderUniforms& InitialData);

        static GlBufferUnique CreateWaterSettingsUbo(const WaterShaderUniforms& InitialData);

        static GlBufferUnique CreateCascadeSettingsSsbo(const CascadeSettingsBuffer& InitialData);

        static float CalculateInitialSpectrumAlpha(const glm::vec2 WindVelocity, const float Fetch,
                                                   const float Gravity)
        {
            const float windSpeed2 = glm::dot(WindVelocity, WindVelocity);
            return 0.076f * std::powf(windSpeed2 / (Fetch * Gravity), 0.22f);
        }

        static float CalculateInitialSpectrumPeakWaveFrequency(
            const glm::vec2 WindVelocity, const float Fetch,
            const float Gravity)
        {
            return 22.0f * std::powf(Gravity * Gravity / (glm::length(WindVelocity) * Fetch), 1.0f / 3.0f);
        }

    public:
        [[nodiscard]] SprayManager& GetSprayManager()
        {
            return _sprayManager;
        }

        void SetFoamThreshold(const float FoamThreshold)
        {
            _foamUniformBufferData.FoamThreshold = FoamThreshold;
            SentFoamMaskSettingsToGpu();
        }

        [[nodiscard]] float GetFoamThreshold() const
        {
            return _foamUniformBufferData.FoamThreshold;
        }

        void SetFoamExponent(const float FoamExponent)
        {
            _foamUniformBufferData.FoamExponent = FoamExponent;
            SentFoamMaskSettingsToGpu();
        }

        [[nodiscard]] float GetFoamExponent() const
        {
            return _foamUniformBufferData.FoamExponent;
        }

        void SetFoamIntensity(const float FoamIntensity)
        {
            _foamUniformBufferData.FoamIntensity = FoamIntensity;
            SentFoamMaskSettingsToGpu();
        }

        [[nodiscard]] float GetFoamIntensity() const
        {
            return _foamUniformBufferData.FoamIntensity;
        }

        void SetTesselationMaxDistance(const float TesselationMaxDistance)
        {
            _waterShaderUniforms.TesselationMaxDistance = TesselationMaxDistance;
        }

        [[nodiscard]] float GetTesselationMaxDistance() const
        {
            return _waterShaderUniforms.TesselationMaxDistance;
        }

        void SetTesselationMinDistance(const float TesselationMinDistance)
        {
            _waterShaderUniforms.TesselationMinDistance = TesselationMinDistance;
        }

        [[nodiscard]] float GetTesselationMinDistance() const
        {
            return _waterShaderUniforms.TesselationMinDistance;
        }

        void SetMaxTesselationLevel(const float MaxTesselationLevel)
        {
            _waterShaderUniforms.MaxTesselationLevel = MaxTesselationLevel;
        }

        [[nodiscard]] float GetMaxTesselationLevel() const
        {
            return _waterShaderUniforms.MaxTesselationLevel;
        }

        void SetMinTesselationLevel(const float MinTesselationLevel)
        {
            _waterShaderUniforms.MinTesselationLevel = MinTesselationLevel;
        }

        [[nodiscard]] float GetMinTesselationLevel() const
        {
            return _waterShaderUniforms.MinTesselationLevel;
        }

        void SetSurfaceColor(const glm::vec3& SurfaceColor)
        {
            _waterShaderUniforms.SurfaceColor = SurfaceColor;
        }

        [[nodiscard]] glm::vec3 GetSurfaceColor() const
        {
            return _waterShaderUniforms.SurfaceColor;
        }

        void SetMetallic(const float Metallic)
        {
            _waterShaderUniforms.Metallic = Metallic;
        }

        [[nodiscard]] float GetMetallic() const
        {
            return _waterShaderUniforms.Metallic;
        }

        void SetAbsorption(const glm::vec3& Absorption)
        {
            _waterShaderUniforms.Absorption = Absorption;
        }

        [[nodiscard]] glm::vec3 GetAbsorption() const
        {
            return _waterShaderUniforms.Absorption;
        }

        void SetRoughness(const float Roughness)
        {
            _waterShaderUniforms.Roughness = Roughness;
        }

        [[nodiscard]] float GetRoughness() const
        {
            return _waterShaderUniforms.Roughness;
        }

        void SetFoamColor(const glm::vec3& FoamColor)
        {
            _waterShaderUniforms.FoamColor = FoamColor;
        }

        [[nodiscard]] glm::vec3 GetFoamColor() const
        {
            return _waterShaderUniforms.FoamColor;
        }

        void SetFoamMetallic(const float FoamMetallic)
        {
            _waterShaderUniforms.FoamMetallic = FoamMetallic;
        }

        [[nodiscard]] float GetFoamMetallic() const
        {
            return _waterShaderUniforms.FoamMetallic;
        }

        void SetFoamRoughness(const float FoamRoughness)
        {
            _waterShaderUniforms.FoamRoughness = FoamRoughness;
        }

        [[nodiscard]] float GetFoamRoughness() const
        {
            return _waterShaderUniforms.FoamRoughness;
        }

        void SetWindVelocity(const glm::vec2 WindVelocity)
        {
            _initialSpectrumSettingsBufferData.WindVelocity = WindVelocity;
            _initialSpectrumSettingsBufferData.PeakWaveFrequency = CalculateInitialSpectrumPeakWaveFrequency(
                _initialSpectrumSettingsBufferData.WindVelocity, _initialSpectrumSettingsBufferData.Fetch, Gravity);
            _initialSpectrumSettingsBufferData.Alpha = CalculateInitialSpectrumAlpha(
                _initialSpectrumSettingsBufferData.WindVelocity, _initialSpectrumSettingsBufferData.Fetch, Gravity);
            SendInitialSpectrumSettingsToGpu();
            UpdateInitialSpectrum();
        }

        [[nodiscard]] glm::vec2 GetWindVelocity() const
        {
            return _initialSpectrumSettingsBufferData.WindVelocity;
        }

        void SetFetch(const float Fetch)
        {
            _initialSpectrumSettingsBufferData.Fetch = Fetch;
            _initialSpectrumSettingsBufferData.PeakWaveFrequency = CalculateInitialSpectrumPeakWaveFrequency(
                _initialSpectrumSettingsBufferData.WindVelocity, _initialSpectrumSettingsBufferData.Fetch, Gravity);
            _initialSpectrumSettingsBufferData.Alpha = CalculateInitialSpectrumAlpha(
                _initialSpectrumSettingsBufferData.WindVelocity, _initialSpectrumSettingsBufferData.Fetch, Gravity);
            SendInitialSpectrumSettingsToGpu();
            UpdateInitialSpectrum();
        }

        [[nodiscard]] float GetFetch() const
        {
            return _initialSpectrumSettingsBufferData.Fetch;
        }

        void SetDelta(const float Delta)
        {
            _initialSpectrumSettingsBufferData.Delta = Delta;
            SendInitialSpectrumSettingsToGpu();
            UpdateInitialSpectrum();
        }

        [[nodiscard]] float GetDelta() const
        {
            return _initialSpectrumSettingsBufferData.Delta;
        }

        void SetSwell(const float Swell)
        {
            _initialSpectrumSettingsBufferData.Swell = Swell;
            SendInitialSpectrumSettingsToGpu();
            UpdateInitialSpectrum();
        }

        [[nodiscard]] float GetSwell() const
        {
            return _initialSpectrumSettingsBufferData.Swell;
        }

        [[nodiscard]] uint32_t GetDisplacementTextures() const
        {
            return _displacementTextures;
        }

        [[nodiscard]] uint32_t GetVelocityTextures() const
        {
            return _velocityTextures;
        }

        [[nodiscard]] uint32_t GetCascadeSettingsSsbo() const
        {
            return _cascadeSettingsSsbo;
        }

        void Render(float DeltaTime, const TransformComponent& CameraTransform, const CameraComponent& Camera,
                    const glm::vec3& AmbientLightColor, uint32_t DynamicWavesSampleSsboId, uint32_t SceneColor,
                    uint32_t SceneDepth);

        void RenderSpray(uint32_t TargetFrameBuffer) const;

    private:
        void UpdateInitialSpectrum() const;

        void UpdateSpectrumTexture() const;

        void AssembleTextures() const;

        void RunFft(uint32_t Texture, int32_t Layers) const;

        void SendInitialSpectrumSettingsToGpu() const;

        void SentFoamMaskSettingsToGpu() const;

        void SendCascadeSettingsToGpu() const;
    };
}
