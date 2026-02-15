#pragma once
#include "Camera/CameraComponent.h"
#include "Ocean/OceanGrid.h"
#include "Ocean/SprayManager.h"
#include "Ocean/WaterEffectorsRenderer.h"
#include "Rendering/Shaders/Shader.h"
#include "Scene/TransformComponent.h"
#include "Utility/GlResourceUnique.h"

namespace Core
{
    /**
     * @brief System managing the entire water rendering pipeline.
     */
    class WaterSystem
    {
    public:
        /**
         * @brief Configuration for a single wave cascade used in the Tessendorf method.
         */
        struct alignas(16) CascadeSettings
        {
        public:
            /**
             * @brief Minimum wave number for this cascade.
             */
            float WaveNumberLowerBound;
            /**
            * @brief Maximum wave number for this cascade.
            */
            alignas(4) float WaveNumberUpperBound;
            /**
             * @brief The physical scale length of the cascade domain.
             */
            alignas(4) float Length;

        private:
            alignas(4) float Padding{};

        public:
            CascadeSettings() : WaveNumberLowerBound(0.0f), WaveNumberUpperBound(0.0f), Length(0)
            {
            }

            CascadeSettings(const float WaveLengthLowerBound, const float WaveLengthUpperBound, const float Length)
                : WaveNumberLowerBound(WaveLengthLowerBound),
                  WaveNumberUpperBound(WaveLengthUpperBound),
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

            alignas(4) float CameraNearPlane;
            alignas(4) float CameraFarPlane;
            alignas(4) float MinLodDistance;
            alignas(4) uint32_t PatchSubdivisions;
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
                  _buffer(static_cast<InternalBuffer*>(std::malloc(InternalBuffer::CalculateSize(_capacity))))
            {
                _buffer->TextureResolution = TextureResolution;
                _buffer->CascadeCount = 0;
            }

            CascadeSettingsBuffer(const int32_t TextureResolution, const int32_t VelocityLayers,
                                  const float VelocityMinHeight, const float VelocityMaxHeight,
                                  const std::initializer_list<CascadeSettings>&
                                  InitialValues) : _capacity(InitialValues.size())
            {
                _buffer = static_cast<InternalBuffer*>(std::malloc(InternalBuffer::CalculateSize(_capacity)));
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
                std::free(_buffer);
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
        static constexpr float VelocityMinHeight = -30.0f;
        static constexpr float VelocityMaxHeight = 10.0f;

    private:
        OceanGrid _oceanGrid;
        SprayManager _sprayManager;
        WaterEffectorsRenderer _waterEffectorsRenderer;

        Shader _waterNearShader;
        Shader _waterFarShader;
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
        /**
         * @brief Initializes the water system.
         * @param WindowSize The resolution of the main render target.
         * @param MainDepthBuffer The OpenGL texture ID of the main depth buffer used for depth tests.
         */
        WaterSystem(const glm::ivec2& WindowSize, uint32_t MainDepthBuffer);

    private:
        static GlTextureUnique CreateTextureArray(int32_t Layers);

        static GlTextureUnique CreateTextureArrayWithMipMapping();

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
        /**
         * @brief Gets a reference to the system managing spray particles.
         * @return A reference to the underlying SprayManager.
         */
        [[nodiscard]] SprayManager& GetSprayManager()
        {
            return _sprayManager;
        }

        /**
         * @brief Sets the threshold value determining when foam becomes visible.
         * @param FoamThreshold The jacobian threshold value.
         */
        void SetFoamThreshold(const float FoamThreshold)
        {
            _foamUniformBufferData.FoamThreshold = FoamThreshold;
            SentFoamMaskSettingsToGpu();
        }

        /**
         * @brief Gets the current foam generation threshold.
         * @return The jacobian threshold value.
         */
        [[nodiscard]] float GetFoamThreshold() const
        {
            return _foamUniformBufferData.FoamThreshold;
        }

        /**
         * @brief Sets the exponent used for shaping the foam falloff curve.
         * @param FoamExponent The falloff exponent.
         */
        void SetFoamExponent(const float FoamExponent)
        {
            _foamUniformBufferData.FoamExponent = FoamExponent;
            SentFoamMaskSettingsToGpu();
        }

        /**
         * @brief Gets the exponent used for shaping the foam falloff curve.
         * @return The falloff exponent.
         */
        [[nodiscard]] float GetFoamExponent() const
        {
            return _foamUniformBufferData.FoamExponent;
        }

        /**
         * @brief Sets the overall visual intensity of generated foam.
         * @param FoamIntensity The intensity multiplier.
         */
        void SetFoamIntensity(const float FoamIntensity)
        {
            _foamUniformBufferData.FoamIntensity = FoamIntensity;
            SentFoamMaskSettingsToGpu();
        }

        /**
         * @brief Gets the overall visual intensity of generated foam.
         * @return The intensity multiplier.
         */
        [[nodiscard]] float GetFoamIntensity() const
        {
            return _foamUniformBufferData.FoamIntensity;
        }

        /**
         * @brief Sets the maximum distance from the camera at which hardware tessellation is applied to the water mesh.
         * @param TesselationMaxDistance The distance in meters.
         */
        void SetTesselationMaxDistance(const float TesselationMaxDistance)
        {
            _waterShaderUniforms.TesselationMaxDistance = TesselationMaxDistance;
        }

        /**
         * @brief Gets the maximum distance for hardware tessellation.
         * @return The maximum tessellation distance in meters.
         */
        [[nodiscard]] float GetTesselationMaxDistance() const
        {
            return _waterShaderUniforms.TesselationMaxDistance;
        }

        /**
         * @brief Sets the distance from the camera at which maximum hardware tessellation is reached.
         * @param TesselationMinDistance The distance in meters.
         */
        void SetTesselationMinDistance(const float TesselationMinDistance)
        {
            _waterShaderUniforms.TesselationMinDistance = TesselationMinDistance;
        }

        /**
         * @brief Gets the distance for maximum hardware tessellation.
         * @return The minimum tessellation distance in meters.
         */
        [[nodiscard]] float GetTesselationMinDistance() const
        {
            return _waterShaderUniforms.TesselationMinDistance;
        }

        /**
         * @brief Sets the maximum tessellation level applied near the camera.
         * @param MaxTesselationLevel The highest subdivision multiplier.
         */
        void SetMaxTesselationLevel(const float MaxTesselationLevel)
        {
            _waterShaderUniforms.MaxTesselationLevel = MaxTesselationLevel;
        }

        /**
         * @brief Gets the maximum tessellation level.
         * @return The maximum tessellation multiplier.
         */
        [[nodiscard]] float GetMaxTesselationLevel() const
        {
            return _waterShaderUniforms.MaxTesselationLevel;
        }

        /**
         * @brief Sets the minimum tessellation level applied at the maximum tessellation distance or further.
         * @param MinTesselationLevel The lowest subdivision multiplier.
         */
        void SetMinTesselationLevel(const float MinTesselationLevel)
        {
            _waterShaderUniforms.MinTesselationLevel = MinTesselationLevel;
        }

        /**
         * @brief Gets the minimum tessellation level.
         * @return The minimum tessellation multiplier.
         */
        [[nodiscard]] float GetMinTesselationLevel() const
        {
            return _waterShaderUniforms.MinTesselationLevel;
        }

        /**
         * @brief Sets the primary base color of the water surface.
         * @param SurfaceColor The base linear RGB color.
         */
        void SetSurfaceColor(const glm::vec3& SurfaceColor)
        {
            _waterShaderUniforms.SurfaceColor = SurfaceColor;
        }

        /**
         * @brief Gets the primary base color of the water surface.
         * @return The base linear RGB color.
         */
        [[nodiscard]] glm::vec3 GetSurfaceColor() const
        {
            return _waterShaderUniforms.SurfaceColor;
        }

        /**
         * @brief Sets the PBR metallic factor for the water surface.
         * @param Metallic The metallic factor (0.0 to 1.0).
         */
        void SetMetallic(const float Metallic)
        {
            _waterShaderUniforms.Metallic = Metallic;
        }

        /**
         * @brief Gets the PBR metallic factor of the water surface.
         * @return The metallic factor.
         */
        [[nodiscard]] float GetMetallic() const
        {
            return _waterShaderUniforms.Metallic;
        }

        /**
         * @brief Sets the deep-water light absorption coefficients.
         * @param Absorption The absorption RGB coefficients.
         */
        void SetAbsorption(const glm::vec3& Absorption)
        {
            _waterShaderUniforms.Absorption = Absorption;
        }

        /**
         * @brief Gets the deep-water light absorption coefficients.
         * @return The absorption RGB coefficients.
         */
        [[nodiscard]] glm::vec3 GetAbsorption() const
        {
            return _waterShaderUniforms.Absorption;
        }

        /**
         * @brief Sets the PBR roughness factor of the water surface.
         * @param Roughness The roughness value (0.0 to 1.0).
         */
        void SetRoughness(const float Roughness)
        {
            _waterShaderUniforms.Roughness = Roughness;
        }

        /**
         * @brief Gets the PBR roughness factor of the water surface.
         * @return The roughness value.
         */
        [[nodiscard]] float GetRoughness() const
        {
            return _waterShaderUniforms.Roughness;
        }

        /**
         * @brief Sets the base color applied to the rendered foam.
         * @param FoamColor The foam linear RGB color.
         */
        void SetFoamColor(const glm::vec3& FoamColor)
        {
            _waterShaderUniforms.FoamColor = FoamColor;
        }

        /**
         * @brief Gets the base color applied to the rendered foam.
         * @return The foam linear RGB color.
         */
        [[nodiscard]] glm::vec3 GetFoamColor() const
        {
            return _waterShaderUniforms.FoamColor;
        }

        /**
         * @brief Sets the PBR metallic factor of the foam surface.
         * @param FoamMetallic The metallic factor (0.0 to 1.0).
         */
        void SetFoamMetallic(const float FoamMetallic)
        {
            _waterShaderUniforms.FoamMetallic = FoamMetallic;
        }

        /**
         * @brief Gets the PBR metallic factor of the foam surface.
         * @return The metallic factor.
         */
        [[nodiscard]] float GetFoamMetallic() const
        {
            return _waterShaderUniforms.FoamMetallic;
        }

        /**
         * @brief Sets the PBR roughness factor of the foam surface.
         * @param FoamRoughness The roughness value (0.0 to 1.0).
         */
        void SetFoamRoughness(const float FoamRoughness)
        {
            _waterShaderUniforms.FoamRoughness = FoamRoughness;
        }

        /**
         * @brief Gets the PBR roughness factor of the foam surface.
         * @return The roughness value.
         */
        [[nodiscard]] float GetFoamRoughness() const
        {
            return _waterShaderUniforms.FoamRoughness;
        }

        /**
         * @brief Sets the normal distortion strength used for Subsurface Scattering (SSS) calculations.
         * @param SssDistortion The SSS distortion coefficient.
         */
        void SetSssDistortion(const float SssDistortion)
        {
            _waterShaderUniforms.SssDistortion = SssDistortion;
        }

        /**
         * @brief Gets the normal distortion strength for Subsurface Scattering.
         * @return The SSS distortion coefficient.
         */
        [[nodiscard]] float GetSssDistortion() const
        {
            return _waterShaderUniforms.SssDistortion;
        }

        /**
         * @brief Sets the intensity exponent for the Subsurface Scattering effect.
         * @param SssPower The SSS power exponent.
         */
        void SetSssPower(const float SssPower)
        {
            _waterShaderUniforms.SssPower = SssPower;
        }

        /**
         * @brief Gets the intensity exponent for Subsurface Scattering.
         * @return The SSS power exponent.
         */
        [[nodiscard]] float GetSssPower() const
        {
            return _waterShaderUniforms.SssPower;
        }

        /**
         * @brief Sets the intensity multiplier for the Subsurface Scattering effect.
         * @param SssScale The SSS intensity multiplier.
         */
        void SetSssScale(const float SssScale)
        {
            _waterShaderUniforms.SssScale = SssScale;
        }

        /**
         * @brief Gets the intensity multiplier for Subsurface Scattering.
         * @return The SSS intensity multiplier.
         */
        [[nodiscard]] float GetSssScale() const
        {
            return _waterShaderUniforms.SssScale;
        }

        /**
         * @brief Sets the tint color applied to light scattered through waves via SSS.
         * @param SssColor The SSS linear RGB tint color.
         */
        void SetSssColor(const glm::vec3& SssColor)
        {
            _waterShaderUniforms.SssColor = SssColor;
        }

        /**
         * @brief Gets the tint color for Subsurface Scattering.
         * @return The SSS linear RGB tint color.
         */
        [[nodiscard]] const glm::vec3& GetSssColor() const
        {
            return _waterShaderUniforms.SssColor;
        }

        /**
          * @brief Sets the threshold value for SSS visibility.
          * @param SssThreshold The jacobian threshold value.
          */
        void SetSssThreshold(const float SssThreshold)
        {
            _waterShaderUniforms.SssThreshold = SssThreshold;
        }

        /**
          * @brief Gets the threshold value for SSS visibility.
          * @return The jacobian threshold value.
          */
        [[nodiscard]] float GetSssThreshold() const
        {
            return _waterShaderUniforms.SssThreshold;
        }

        /**
         * @brief Sets the global wind velocity vector.
         * @param WindVelocity The 2D vector representing wind velocity.
         */
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

        /**
         * @brief Gets the current global wind velocity.
         * @return The 2D wind velocity vector.
         */
        [[nodiscard]] glm::vec2 GetWindVelocity() const
        {
            return _initialSpectrumSettingsBufferData.WindVelocity;
        }

        /**
         * @brief Sets the fetch length (distance over which wind blows).
         * @param Fetch The wind fetch distance in meters.
         */
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

        /**
         * @brief Gets the current wind fetch distance (distance over which wind blows).
         * @return The wind fetch in meters.
         */
        [[nodiscard]] float GetFetch() const
        {
            return _initialSpectrumSettingsBufferData.Fetch;
        }

        /**
         * @brief Sets the spectrum delta which determines directionality of the wave spectrum.
         * @param Delta The spectrum delta parameter.
         */
        void SetDelta(const float Delta)
        {
            _initialSpectrumSettingsBufferData.Delta = Delta;
            SendInitialSpectrumSettingsToGpu();
            UpdateInitialSpectrum();
        }

        /**
         * @brief Gets the spectrum delta which determines directionality of the wave spectrum.
         * @return The spectrum delta parameter.
         */
        [[nodiscard]] float GetDelta() const
        {
            return _initialSpectrumSettingsBufferData.Delta;
        }

        /**
         * @brief Sets the swell modifier for the wave spectrum.
         * @param Swell The swell scalar modifier.
         */
        void SetSwell(const float Swell)
        {
            _initialSpectrumSettingsBufferData.Swell = Swell;
            SendInitialSpectrumSettingsToGpu();
            UpdateInitialSpectrum();
        }

        /**
         * @brief Gets the swell modifier.
         * @return The swell scalar modifier.
         */
        [[nodiscard]] float GetSwell() const
        {
            return _initialSpectrumSettingsBufferData.Swell;
        }

        /**
         * @brief Retrieves the OpenGL texture ID of the texture array containing water displacement maps.
         * @return The OpenGL texture array ID.
         */
        [[nodiscard]] uint32_t GetDisplacementTextures() const
        {
            return _displacementTextures;
        }

        /**
         * @brief Retrieves the OpenGL texture ID of the texture array containing water surface velocity maps.
         * @return The OpenGL texture array ID.
         */
        [[nodiscard]] uint32_t GetVelocityTextures() const
        {
            return _velocityTextures;
        }

        /**
         * @brief Retrieves the OpenGL SSBO ID containing cascade configurations used by shaders.
         * @return The OpenGL SSBO ID.
         */
        [[nodiscard]] uint32_t GetCascadeSettingsSsbo() const
        {
            return _cascadeSettingsSsbo;
        }

        /**
         * @brief Enables or disables rendering the water mesh as wireframe.
         * @param RenderAsWireframe True to render as wireframe, false for solid geometry.
         */
        void SetRenderWaterAsWireframe(const bool RenderAsWireframe)
        {
            _oceanGrid.SetRenderAsWireframe(RenderAsWireframe);
        }

        /**
         * @brief Checks if the water mesh is currently rendering in wireframe mode.
         * @return True if wireframe rendering is active.
         */
        [[nodiscard]] bool GetRenderWaterAsWireframe() const
        {
            return _oceanGrid.GetRenderAsWireframe();
        }

        /**
         * @brief Checks if debug visualizations for water effectors are currently being rendered.
         * @return True if effector debug rendering is active.
         */
        [[nodiscard]] bool GetRenderWaterEffectors() const
        {
            return _waterEffectorsRenderer.IsRendering();
        }

        /**
         * @brief Enables or disables debug visualizations for water effectors.
         * @param RenderDebugInfo True to enable effector debug rendering.
         */
        void SetRenderWaterEffectors(const bool RenderDebugInfo)
        {
            _waterEffectorsRenderer.SetRender(RenderDebugInfo);
        }

        /**
         * @brief Executes the full water pipeline: computes the new wave state, updates grid LOD, and draws the ocean.
         * @param DeltaTime Time elapsed since the last frame in seconds.
         * @param CameraTransform The transform component of the active camera.
         * @param Camera The active camera component.
         * @param AmbientLightColor The ambient color of the scene for basic lighting.
         * @param DynamicWavesSampleSsboId The SSBO containing dynamic wave interactions to overlay.
         * @param SceneColor The OpenGL texture ID of the scene color buffer.
         * @param SceneDepth The OpenGL texture ID of the scene depth buffer.
         */
        void Render(float DeltaTime, const TransformComponent& CameraTransform, const CameraComponent& Camera,
                    const glm::vec3& AmbientLightColor, uint32_t DynamicWavesSampleSsboId, uint32_t SceneColor,
                    uint32_t SceneDepth);

        /**
         * @brief Dispatches the spray rendering pass.
         * @param TargetFrameBuffer The OpenGL Framebuffer Object where spray should be drawn.
         */
        void RenderSpray(uint32_t TargetFrameBuffer) const;

        /**
         * @brief Renders the debug visualizers for water effectors.
         * @param Registry The entity registry containing effector components.
         * @param ViewMatrix The active camera's view matrix.
         * @param ProjectionMatrix The active camera's projection matrix.
         */
        void RenderWaterEffectorsDebug(const entt::registry& Registry, const glm::mat4& ViewMatrix,
                                       const glm::mat4& ProjectionMatrix);

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
