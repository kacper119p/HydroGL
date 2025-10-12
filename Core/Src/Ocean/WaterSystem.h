#pragma once
#include "Camera/CameraComponent.h"
#include "Ocean/OceanGrid.h"
#include "Rendering/Shaders/Shader.h"
#include "Scene/TransformComponent.h"

namespace Core
{
    class WaterSystem
    {
    public:
        struct alignas(4 * sizeof(float)) CascadeSettings
        {
            float WaveLengthLowerBound;
            float WaveLengthUpperBound;
            int32_t Length;
            float Padding{};

            CascadeSettings() : WaveLengthLowerBound(0.0f), WaveLengthUpperBound(0.0f), Length(0)
            {
            }

            CascadeSettings(const float WaveLengthLowerBound, const float WaveLengthUpperBound, const int32_t Length)
                : WaveLengthLowerBound(WaveLengthLowerBound),
                  WaveLengthUpperBound(WaveLengthUpperBound),
                  Length(Length)
            {
            }
        };

    private:
        struct alignas(sizeof(glm::vec2)) InitialSpectrumSettingsBuffer
        {
            glm::vec2 WindVelocity;
            float Fetch;
            float Delta;
            float Swell;
            float Alpha;
            float PeakWaveFrequency;
            float Padding;

            InitialSpectrumSettingsBuffer() = default;

            InitialSpectrumSettingsBuffer(const glm::vec2& WindVelocity, const float Fetch, const float Delta,
                                          const float Swell, const float Alpha, const float PeakWaveFrequency)
                : WindVelocity(WindVelocity),
                  Fetch(Fetch),
                  Delta(Delta),
                  Swell(Swell),
                  Alpha(Alpha),
                  PeakWaveFrequency(PeakWaveFrequency),
                  Padding(0.0f)
            {
            }
        };

        class CascadeSettingsBuffer
        {
        private:
            struct InternalBuffer
            {
                int32_t TextureResolution{};
                int32_t CascadeCount{};
                float Padding[2]{};
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

            CascadeSettingsBuffer(const int32_t TextureResolution,
                                  const std::initializer_list<CascadeSettings>&
                                  InitialValues) : _capacity(InitialValues.size())
            {
                _buffer = static_cast<InternalBuffer*>(malloc(InternalBuffer::CalculateSize(_capacity)));
                _buffer->TextureResolution = TextureResolution;
                _buffer->CascadeCount = InitialValues.size();
                memcpy(_buffer->CascadeSettings, InitialValues.begin(), InitialValues.size() * sizeof(CascadeSettings));
            }

        public:
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

    private:
        static constexpr float Gravity = 9.80665f;
        static constexpr int32_t CascadeCount = 3;
        static constexpr int32_t TextureResolution = 256;

    private:
        OceanGrid _oceanGrid;

        Shader _waterShader;
        int32_t _viewMatrixLocation;
        int32_t _projectionMatrixLocation;
        int32_t _cameraPositionLocation;
        int32_t _timeLocation;
        int32_t _gridOriginLocation;

        Shader _fftHorizontalShader;
        Shader _fftVerticalShader;
        Shader _assembleComputeShader;
        Shader _orientationComputeShader;
        Shader _initialSpectrumComputeShader;
        Shader _spectrumComputeShader;

        uint32_t _initialSpectrumTextures;
        uint32_t _hDxTextures;
        uint32_t _dzDfhxTextures;
        uint32_t _dfhzDfdxxTextures;
        uint32_t _dfdzxDfdzzTextures;
        uint32_t _spectrumTextures;
        uint32_t _slopeTextures;
        uint32_t _displacementTextures;
        uint32_t _tmpTextures;
        uint32_t _gaussianNoiseTexture = 0;

        InitialSpectrumSettingsBuffer _initialSpectrumSettingsBufferData;
        CascadeSettingsBuffer _cascadeSettings;

        uint32_t _initialSpectrumSettingsSsbo = 0;
        uint32_t _cascadeSettingsSsbo = 0;

    public:
        WaterSystem();

    private:
        static uint32_t CreateTextureArray(int32_t Resolution, int32_t Layers, int32_t InternalFormat, int32_t Format,
                                           int32_t Type, int32_t Filter, int32_t Wrap);

        static uint32_t CreateGaussianNoise(int32_t Resolution);

        static uint32_t CreateInitialSpectrumSettingsSsbo(const InitialSpectrumSettingsBuffer& InitialData);

        static uint32_t CreateCascadeSettingsSsbo(const CascadeSettingsBuffer& InitialData);

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
            return 22.0f * Gravity * Gravity / (glm::length(WindVelocity) * Fetch);
        }

    public:
        void SetWindVelocity(const glm::vec2 WindVelocity)
        {
            glm::vec2 value = WindVelocity;
            value.x *= -1.0f;
            _initialSpectrumSettingsBufferData.WindVelocity = value;
            _initialSpectrumSettingsBufferData.PeakWaveFrequency = CalculateInitialSpectrumPeakWaveFrequency(
                _initialSpectrumSettingsBufferData.WindVelocity, _initialSpectrumSettingsBufferData.Fetch, Gravity);
            _initialSpectrumSettingsBufferData.Alpha = CalculateInitialSpectrumAlpha(
                _initialSpectrumSettingsBufferData.WindVelocity, _initialSpectrumSettingsBufferData.Fetch, Gravity);
            SendInitialSpectrumSettingsToGpu();
            UpdateInitialSpectrum();
        }

        [[nodiscard]] glm::vec2 GetWindVelocity() const
        {
            glm::vec2 value = _initialSpectrumSettingsBufferData.WindVelocity;
            value.x *= -1.0f;
            return value;
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

    public:
        void Render(const TransformComponent& CameraTransform, const CameraComponent& Camera);

    private:
        void UpdateInitialSpectrum() const;

        void UpdateSpectrumTexture() const;

        void AssembleTextures() const;

        void RunFft(uint32_t& Texture);

        void SendInitialSpectrumSettingsToGpu() const;

        void SendCascadeSettingsToGpu() const;
    };
}
