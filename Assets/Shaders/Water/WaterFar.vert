#version 460 core
#extension GL_ARB_bindless_texture : enable

#include "Water/WaterCommon.glsl"

DEFINE_WATER_PARAMETERS_BUFFER(1);

DEFINE_CASCADE_SETTINGS_BUFFER(0);

struct dynamicWaveSampleParameters
{
    vec3 ObjectPosition;
    float WaveFieldSize;
    layout(bindless_sampler) sampler2D WaveFieldTexture;
    uint WaveFieldTextureSize;
};

layout(std430, binding = 1) readonly restrict buffer DynamicWavesBuffer
{
    int DynamicWavesCount;
    float Padding[3];
    dynamicWaveSampleParameters Waves[];
} dynamicWavesBuffer;

layout (binding = 3) uniform sampler2DArray DisplacementMaps;

layout (location = 0) in vec3 inputLocalPosition;
layout (location = 1) in vec4 inputInstanceData;

out vec3 OriginalPosition;
out vec3 AmbientDisplacementPosition;
out vec3 WorldPosition;
out vec4 ClipSpacePosition;

vec2 MorphVertex(const vec2 LocalSpacePosition, const vec2 WorldSpacePosition, const float MorphFactor)
{
    const vec2 fraction = fract(LocalSpacePosition * float(WaterParameters.PatchSubdivisions) * 0.5) * 2.0 / float(WaterParameters.PatchSubdivisions);
    return WorldSpacePosition - fraction * inputInstanceData.z * MorphFactor;
}

void main()
{
    const vec2 patchPosition = inputInstanceData.xy;
    const float patchScale = inputInstanceData.z;
    const uint lodLevel = uint(inputInstanceData.w);

    const vec3 localPosition = inputLocalPosition;

    const vec2 worldPosition2D = patchPosition + (localPosition.xz * patchScale);

    const float distanceToCamera = distance(WaterParameters.CameraPosition, vec3(worldPosition2D.x, 0.0, worldPosition2D.y));

    float lodRangeLow = 0.0;
    float lodRangeHigh = WaterParameters.MinLodDistance;

    if (lodLevel != 0)
    {
        lodRangeLow = WaterParameters.MinLodDistance * pow(2.0, float(lodLevel - 1));
        lodRangeHigh = lodRangeLow * 2.0;
    }

    const float factor = (distanceToCamera - lodRangeLow) / (lodRangeHigh - lodRangeLow);
    const float morphFactor = clamp((factor - 0.7) / 0.3, 0.0, 1.0);

    const vec2 morphedWorldPosition = MorphVertex(localPosition.xz + vec2(0.5), worldPosition2D, morphFactor);

    const vec3 position = vec3(morphedWorldPosition.x, 0.0, morphedWorldPosition.y);

    vec3 tessendorfDisplacement = vec3(0.0);
    for (int i = 0; i < CascadeSettingsBuffer.CascadeCount; ++i)
    {
        const vec2 texCoord = vec2(position.x, position.z) / CascadeSettingsBuffer.CascadeSettings[i].Length;
        tessendorfDisplacement += texture(DisplacementMaps, vec3(texCoord, i)).xyz;
    }
    const vec3 tessendorfPosition = position + tessendorfDisplacement;

    vec3 dynamicDisplacement = vec3(0.0);
    for (int i = 0; i < dynamicWavesBuffer.DynamicWavesCount; ++i)
    {
        const vec2 waveLocalPosition =  dynamicWavesBuffer.Waves[i].ObjectPosition.xz - tessendorfPosition.xz;
        const vec2 waveTexCoord = waveLocalPosition / dynamicWavesBuffer.Waves[i].WaveFieldSize + vec2(0.5);
        dynamicDisplacement.y += texture(dynamicWavesBuffer.Waves[i].WaveFieldTexture, waveTexCoord).r;
    }

    const vec3 worldPosition = tessendorfPosition + dynamicDisplacement;

    OriginalPosition = position;
    AmbientDisplacementPosition = tessendorfPosition;
    WorldPosition = worldPosition;

    ClipSpacePosition = WaterParameters.ProjectionMatrix * WaterParameters.ViewMatrix * vec4(worldPosition, 1.0);
    gl_Position = ClipSpacePosition;
}

