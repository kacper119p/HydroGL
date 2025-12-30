#version 460 core
#extension GL_ARB_bindless_texture : enable

#include "Water/WaterCommon.glsl"

layout (quads, equal_spacing, ccw) in;

in vec3 tcPosition[];

out vec3 OriginalPosition;
out vec3 DisplacedPosition;
out vec4 ClipSpacePosition;

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


void main()
{
    const float u = gl_TessCoord.x;
    const float v = gl_TessCoord.y;

    const vec3 p0 = mix(tcPosition[0], tcPosition[1], u);
    const vec3 p1 = mix(tcPosition[3], tcPosition[2], u);
    const vec3 position = mix(p0, p1, v);

    vec3 tessendorfDisplacement = vec3(0.0);
    for (int i = 0; i < CascadeSettingsBuffer.CascadeCount; ++i)
    {
        const vec2 texCoord = vec2(position.x, position.z) / CascadeSettingsBuffer.CascadeSettings[i].Length;
        tessendorfDisplacement += texture(DisplacementMaps, vec3(texCoord, i)).xyz;
    }
    vec3 tessendorfPosition = position + tessendorfDisplacement;

    vec3 dynamicDisplacement = vec3(0.0);
    for (int i = 0; i < dynamicWavesBuffer.DynamicWavesCount; ++i)
    {
        const vec2 waveLocalPosition =  dynamicWavesBuffer.Waves[i].ObjectPosition.xz - tessendorfPosition.xz;
        const vec2 waveTexCoord = waveLocalPosition / dynamicWavesBuffer.Waves[i].WaveFieldSize + vec2(0.5);
        dynamicDisplacement.y += texture(dynamicWavesBuffer.Waves[i].WaveFieldTexture, waveTexCoord).r;
    }
    
    OriginalPosition = position;
    DisplacedPosition = tessendorfPosition;

    ClipSpacePosition = WaterParameters.ProjectionMatrix * WaterParameters.ViewMatrix * vec4(tessendorfPosition + dynamicDisplacement, 1.0);
    gl_Position = ClipSpacePosition;
}
