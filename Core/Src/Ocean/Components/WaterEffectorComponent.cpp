#include "WaterEffectorComponent.h"

#include <glad/glad.h>

Core::WaterEffectorComponent::WaterEffectorComponent(const Core::Mesh& Mesh,
                                                     const glm::vec3& Position,
                                                     const float WaveDampingMin,
                                                     const float WaveDampingMax,
                                                     const float DampingMaxVelocity,
                                                     const float DisplacementStrength,
                                                     const int32_t TexturesResolution,
                                                     const float InteractiveWavesFieldSize,
                                                     const float IntersectionMaskPhysicalSize) : Mesh(Mesh),
    WaveDampingMin(WaveDampingMin), WaveDampingMax(WaveDampingMax),
    DampingMaxVelocity(DampingMaxVelocity),
    DisplacementStrength(DisplacementStrength),
    InteractiveWavesFieldSize(InteractiveWavesFieldSize),
    IntersectionMaskFieldSize(IntersectionMaskPhysicalSize),
    TexturesResolution(TexturesResolution)
{
    PreviousPositions[0] = glm::vec2(Position.x, Position.z);
    PreviousPositions[1] = glm::vec2(Position.x, Position.z);

    constexpr float borderColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    glCreateTextures(GL_TEXTURE_2D, 2, MaskTextures);
    glCreateTextures(GL_TEXTURE_2D, 3, InteractiveWavesTextures);
    void* textureData = calloc(TexturesResolution * TexturesResolution, sizeof(float));
    for (int32_t i = 0; i < 2; ++i)
    {
        glTextureStorage2D(MaskTextures[i], 1, GL_R32F, TexturesResolution,
                           TexturesResolution);
        glTextureSubImage2D(MaskTextures[i], 0, 0, 0, TexturesResolution, TexturesResolution, GL_RED,
                            GL_FLOAT, textureData);
        glTextureParameteri(MaskTextures[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(MaskTextures[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(MaskTextures[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTextureParameteri(MaskTextures[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTextureParameterfv(MaskTextures[i], GL_TEXTURE_BORDER_COLOR, borderColor);
    }
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

void Core::WaterEffectorComponent::Destroy(const WaterEffectorComponent& ToDestroy)
{
    glDeleteTextures(2, ToDestroy.MaskTextures);
    for (int i = 0; i < 3; ++i)
    {
        glMakeTextureHandleNonResidentARB(ToDestroy.InteractiveWavesTextureHandles[i]);
    }
    glDeleteTextures(3, ToDestroy.InteractiveWavesTextures);
}
