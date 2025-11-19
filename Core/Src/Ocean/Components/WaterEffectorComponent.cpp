#include "WaterEffectorComponent.h"

#include <glad/glad.h>
#include <glm/geometric.hpp>

#include "Meshes/MeshUtility.h"

Core::WaterEffectorComponent::WaterEffectorComponent(const class Mesh& Mesh,
                                                     const glm::vec3& Position,
                                                     const float MeshTotalVolume,
                                                     const float WaveDampingMin,
                                                     const float WaveDampingMax,
                                                     const float DampingMaxVelocity,
                                                     const float DisplacementStrength,
                                                     const float BackWaveHeightFactor,
                                                     const float FrontWaveHeightFactor,
                                                     const int32_t TexturesResolution,
                                                     const float InteractiveWavesFieldSize)
    : Mesh(Mesh),
      InteractiveWavesFieldSize(InteractiveWavesFieldSize),
      IntersectionMaskFieldSize(glm::length(Mesh.GetBoundingBox().GetMax() -
                                            Mesh.GetBoundingBox().GetMin()) * 1.1f), MeshTotalVolume(MeshTotalVolume),
      WaveDampingMin(WaveDampingMin),
      WaveDampingMax(WaveDampingMax),
      DampingMaxVelocity(DampingMaxVelocity),
      BackWaveHeightFactor(BackWaveHeightFactor),
      FrontWaveHeightFactor(FrontWaveHeightFactor),
      DisplacementStrength(DisplacementStrength),
      TexturesResolution(TexturesResolution)
{
    PreviousPositions[0] = glm::vec2(Position.x, Position.z);
    PreviousPositions[1] = glm::vec2(Position.x, Position.z);

    constexpr float borderColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    glCreateTextures(GL_TEXTURE_2D, 1, &MaskTexture);
    glCreateTextures(GL_TEXTURE_2D, 3, InteractiveWavesTextures);

    void* textureData = calloc(TexturesResolution * TexturesResolution, sizeof(float));

    glTextureStorage2D(MaskTexture, 1, GL_R32F, TexturesResolution,
                       TexturesResolution);
    glTextureSubImage2D(MaskTexture, 0, 0, 0, TexturesResolution, TexturesResolution, GL_RED,
                        GL_FLOAT, textureData);
    glTextureParameteri(MaskTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(MaskTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(MaskTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTextureParameteri(MaskTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTextureParameterfv(MaskTexture, GL_TEXTURE_BORDER_COLOR, borderColor);

    for (int32_t i = 0; i < 3; ++i)
    {
        glTextureStorage2D(InteractiveWavesTextures[i], 1, GL_R32F, TexturesResolution,
                           TexturesResolution);
        glTextureSubImage2D(InteractiveWavesTextures[i], 0, 0, 0, TexturesResolution, TexturesResolution, GL_RED,
                            GL_FLOAT, textureData);
        glTextureParameteri(InteractiveWavesTextures[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(InteractiveWavesTextures[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(InteractiveWavesTextures[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTextureParameteri(InteractiveWavesTextures[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTextureParameterfv(InteractiveWavesTextures[i], GL_TEXTURE_BORDER_COLOR, borderColor);
        InteractiveWavesTextureHandles[i] = glGetTextureHandleARB(InteractiveWavesTextures[i]);
        glMakeTextureHandleResidentARB(InteractiveWavesTextureHandles[i]);
    }
    free(textureData);
}

Core::WaterEffectorComponent::WaterEffectorComponent(const class Mesh& Mesh,
                                                     const glm::vec3& Position,
                                                     const float WaveDampingMin,
                                                     const float WaveDampingMax,
                                                     const float DampingMaxVelocity,
                                                     const float DisplacementStrength,
                                                     const float BackWaveHeightFactor,
                                                     const float FrontWaveHeightFactor,
                                                     const int32_t TexturesResolution,
                                                     const float InteractiveWavesFieldSize)
    : WaterEffectorComponent(
        Mesh,
        Position,
        MeshUtility::CalculateMeshVolume(Mesh),
        WaveDampingMin,
        WaveDampingMax,
        DampingMaxVelocity,
        DisplacementStrength,
        BackWaveHeightFactor,
        FrontWaveHeightFactor,
        TexturesResolution,
        InteractiveWavesFieldSize)
{
}

void Core::WaterEffectorComponent::Destroy(const WaterEffectorComponent& ToDestroy)
{
    glDeleteTextures(1, &ToDestroy.MaskTexture);
    for (int i = 0; i < 3; ++i)
    {
        glMakeTextureHandleNonResidentARB(ToDestroy.InteractiveWavesTextureHandles[i]);
    }
    glDeleteTextures(3, ToDestroy.InteractiveWavesTextures);
}
