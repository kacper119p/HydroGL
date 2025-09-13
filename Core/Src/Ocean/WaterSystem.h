#pragma once
#include "Camera/CameraComponent.h"
#include "Ocean/OceanGrid.h"
#include "Rendering/Shaders/Shader.h"
#include "Scene/TransformComponent.h"

namespace Core
{
    class WaterSystem
    {
    private:
        static constexpr int32_t TextureResolution = 256;
        static constexpr int32_t WorkGroupDim = 16;

        const int _oceanSize = 512;
        const float _amplitude = 12.0f;

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
        Shader _normalComputeShader;
        Shader _orientationComputeShader;
        Shader _initialSpectrumComputeShader;
        Shader _spectrumComputeShader;

        uint32_t _initialSpectrumTexture = 0;
        uint32_t _dxTexture = 0;
        uint32_t _dyTexture = 0;
        uint32_t _dzTexture = 0;
        uint32_t _spectrumTexture = 0;
        uint32_t _normalTexture = 0;
        uint32_t _displacementTexture = 0;
        uint32_t _tmpTexture = 0;
        uint32_t _gaussianNoiseTexture = 0;

    public:
        WaterSystem();

    public:
        void Render(const TransformComponent& CameraTransform, const CameraComponent& Camera);

    private:
        void GenerateInitialSpectrum();

        void UpdateSpectrumTexture() const;

        void UpdateNormalAndDisplacement() const;

        void RunFft(uint32_t& Texture);

        static void CreateTexture(uint32_t* Id, int32_t Resolution, int32_t InternalFormat, int32_t Format,
                                  int32_t Type, int32_t Filter, int32_t Wrap);
    };
}
