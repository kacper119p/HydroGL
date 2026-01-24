#include "WaterEffectorComponent.h"

#include <glad/glad.h>
#include <glm/geometric.hpp>

#include "Meshes/MeshUtility.h"

Core::WaterEffectorComponent::WaterEffectorComponent(const class Mesh& Mesh,
                                                     const glm::vec3& Position,
                                                     const float MeshTotalVolume,
                                                     const float DragCoefficient,
                                                     const float DisplacementStrength,
                                                     const float BackWaveHeightFactor,
                                                     const float FrontWaveHeightFactor,
                                                     const int32_t TexturesResolution)
    : Mesh(Mesh),
      DragCoefficient(DragCoefficient),
      IntersectionMaskFieldSize(glm::length(Mesh.GetBoundingBox().GetMax() -
                                            Mesh.GetBoundingBox().GetMin()) * 1.1f), MeshTotalVolume(MeshTotalVolume),
      BackWaveHeightFactor(BackWaveHeightFactor),
      FrontWaveHeightFactor(FrontWaveHeightFactor),
      DisplacementStrength(DisplacementStrength),
      TexturesResolution(TexturesResolution)
{
    PreviousPositions[0] = glm::vec2(Position.x, Position.z);
    PreviousPositions[1] = glm::vec2(Position.x, Position.z);

    constexpr float borderColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    glCreateTextures(GL_TEXTURE_2D, 1, &MaskTexture);
    glCreateTextures(GL_TEXTURE_2D, 3, InteractiveWavesTextures.Data());

    glTextureStorage2D(MaskTexture, 1, GL_R32F, TexturesResolution,
                       TexturesResolution);
    constexpr float clearColor[] = {0.0f, 0.0f, 0.0f, 0.0f};
    glClearTexImage(MaskTexture, 0, GL_RED, GL_FLOAT, clearColor);
    glTextureParameteri(MaskTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(MaskTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(MaskTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTextureParameteri(MaskTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTextureParameterfv(MaskTexture, GL_TEXTURE_BORDER_COLOR, borderColor);

    for (int32_t i = 0; i < 3; ++i)
    {
        glTextureStorage2D(InteractiveWavesTextures[i], 1, GL_R32F, TexturesResolution,
                           TexturesResolution);
        glClearTexImage(MaskTexture, 0, GL_RED, GL_FLOAT, clearColor);
        glTextureParameteri(InteractiveWavesTextures[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(InteractiveWavesTextures[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(InteractiveWavesTextures[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTextureParameteri(InteractiveWavesTextures[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTextureParameterfv(InteractiveWavesTextures[i], GL_TEXTURE_BORDER_COLOR, borderColor);
        InteractiveWavesTextureHandles[i] = glGetTextureHandleARB(InteractiveWavesTextures[i]);
        glMakeTextureHandleResidentARB(InteractiveWavesTextureHandles[i]);
    }
}

Core::WaterEffectorComponent::WaterEffectorComponent(const class Mesh& Mesh,
                                                     const glm::vec3& Position,
                                                     float DragCoefficient,
                                                     const float DisplacementStrength,
                                                     const float BackWaveHeightFactor,
                                                     const float FrontWaveHeightFactor,
                                                     const int32_t TexturesResolution)
    : WaterEffectorComponent(
        Mesh,
        Position,
        MeshUtility::CalculateMeshVolume(Mesh),
        DragCoefficient,
        DisplacementStrength,
        BackWaveHeightFactor,
        FrontWaveHeightFactor,
        TexturesResolution)
{
}

void Core::WaterEffectorComponent::Destroy(const WaterEffectorComponent& ToDestroy)
{
    glDeleteTextures(1, &ToDestroy.MaskTexture);
    for (int i = 0; i < 3; ++i)
    {
        glMakeTextureHandleNonResidentARB(ToDestroy.InteractiveWavesTextureHandles[i]);
    }
    glDeleteTextures(3, ToDestroy.InteractiveWavesTextures.Data());
}
