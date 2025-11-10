#pragma once
#include <cstdint>
#include <glad/glad.h>
#include <glm/vec2.hpp>

#include "../../../../cmake-build-release-visual-studio/vcpkg_installed/x64-windows/include/spdlog/spdlog.h"
#include "Meshes/Mesh.h"

namespace Core
{
    class WaterEffectorComponent
    {
    public:
        Mesh Mesh;
        float WaveDampingMin;
        float WaveDampingMax;
        float DampingMaxVelocity;
        uint32_t MaskTexture;
        uint32_t InteractiveWavesTextures[3];
        uint64_t InteractiveWavesTextureHandles[3];
        float InteractiveWavesFieldSize;
        float IntersectionMaskPhysicalSize;
        int32_t InteractiveWavesTextureResolution;

    public:
        WaterEffectorComponent(const class Mesh& Mesh, const float WaveDampingMin, const float WaveDampingMax,
                               const float DampingMaxVelocity, const int32_t InteractiveWavesTextureResolution,
                               const float InteractiveWavesFieldSize,
                               const float IntersectionMaskPhysicalSize)
            : Mesh(Mesh), WaveDampingMin(WaveDampingMin), WaveDampingMax(WaveDampingMax),
              DampingMaxVelocity(DampingMaxVelocity), InteractiveWavesFieldSize(InteractiveWavesFieldSize),
              IntersectionMaskPhysicalSize(IntersectionMaskPhysicalSize),
              InteractiveWavesTextureResolution(InteractiveWavesTextureResolution)
        {
            constexpr float borderColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
            glCreateTextures(GL_TEXTURE_2D, 1, &MaskTexture);
            glTextureStorage2D(MaskTexture, 1, GL_R32F, InteractiveWavesTextureResolution,
                               InteractiveWavesTextureResolution);
            glTextureParameteri(MaskTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(MaskTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(MaskTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTextureParameteri(MaskTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glTextureParameterfv(MaskTexture, GL_TEXTURE_BORDER_COLOR, borderColor);

            spdlog::info("{}", MaskTexture);

            glCreateTextures(GL_TEXTURE_2D, 3, InteractiveWavesTextures);
            for (int32_t i = 0; i < 3; ++i)
            {
                glTextureStorage2D(InteractiveWavesTextures[i], 1, GL_RGBA32F, InteractiveWavesTextureResolution,
                                   InteractiveWavesTextureResolution);
                glTextureParameteri(InteractiveWavesTextures[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTextureParameteri(InteractiveWavesTextures[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTextureParameteri(InteractiveWavesTextures[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                glTextureParameteri(InteractiveWavesTextures[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
                glTextureParameterfv(InteractiveWavesTextures[i], GL_TEXTURE_BORDER_COLOR, borderColor);

                InteractiveWavesTextureHandles[i] = glGetTextureHandleARB(InteractiveWavesTextures[i]);
                glMakeTextureHandleResidentARB(InteractiveWavesTextureHandles[i]);
            }
        }

    public:
        static void Destroy(const WaterEffectorComponent& ToDestroy)
        {
            glDeleteTextures(1, &ToDestroy.MaskTexture);
            for (int i = 0; i < 3; ++i)
            {
                glMakeTextureHandleNonResidentARB(ToDestroy.InteractiveWavesTextureHandles[i]);
            }
            glDeleteTextures(3, ToDestroy.InteractiveWavesTextures);
        }
    };
}
