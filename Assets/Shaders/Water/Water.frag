#version 460 core
#extension GL_ARB_bindless_texture : enable

#include "Water/WaterCommon.glsl"
#include "Normal.glsl"
#include "Lighting.glsl"

uniform vec3 CameraPosition;

in vec3 OriginalPosition;
in vec3 DisplacedPosition;
in vec4 ClipSpacePosition;

uniform mat4 ViewMatrix;
uniform vec2 WindVelocity = vec2(16.0, 16.0);
uniform float Fetch = 10000.0;
uniform float Delta = 1.0;
uniform float Swell = 0.5;

DEFINE_CASCADE_SETTINGS_BUFFER(0);

DEFINE_FOAM_PARAMETERS_BUFFER(0);

struct dynamicWaveSampleParameters
{
    vec3 ObjectPosition;
    float WaveFieldSize;
    layout(bindless_sampler) sampler2D WaveFieldTexture;
};

layout(std430, binding = 1) readonly restrict buffer dynamicWavesBuffer
{
    int DynamicWavesCount;
    float Padding[3];
    dynamicWaveSampleParameters Waves[];
} DynamicWavesBuffer;

layout (binding = 3) uniform sampler2DArray DisplacementMaps;
layout (binding = 4) uniform sampler2DArray SlopeMaps;
layout (binding = 5) uniform sampler2D SceneColor;
layout (binding = 6) uniform sampler2D SceneDepth;

layout (location = 0) out vec4 FragColor;

layout (early_fragment_tests) in;
void main()
{
    const vec2 texCoord = vec2(OriginalPosition.x, OriginalPosition.z) / CascadeSettingsBuffer.CascadeSettings[0].Length;
    vec4 slopeDfdxxDfdzz =  texture(SlopeMaps, vec3(texCoord, 0));
    float dfdzx = texture(DisplacementMaps, vec3(texCoord, 0)).w;

    for (int i = 1; i < CascadeSettingsBuffer.CascadeCount; ++i)
    {
        const vec2 texCoord = vec2(OriginalPosition.x, OriginalPosition.z) / CascadeSettingsBuffer.CascadeSettings[i].Length;
        slopeDfdxxDfdzz += texture(SlopeMaps, vec3(texCoord, i));
        dfdzx += texture(DisplacementMaps, vec3(texCoord, i)).w;
    }

    const vec2 slope = slopeDfdxxDfdzz.xy;
    const float jacobian = (1.0 + slopeDfdxxDfdzz.z) * (1.0 + slopeDfdxxDfdzz.w) - dfdzx * dfdzx;

    const float compression = clamp((FoamParameters.FoamThreshold - jacobian) / FoamParameters.FoamThreshold, 0.0, 1.0);
    const float foamMask = clamp(FoamParameters.FoamIntensity * pow(compression, FoamParameters.FoamExponent), 0.0, 1.0);
    const vec3 surfaceColor = mix(vec3(0.005, 0.1, 0.2), vec3(1.0, 1.0, 1.0), foamMask);
    const float metallic = mix(0.05, 0.0, foamMask);
    const float roughness = mix(0.5, 1.0, foamMask);

    vec2 dynamicSlope = vec2(0.0);
    for (int i = 0; i < DynamicWavesBuffer.DynamicWavesCount; ++i)
    {
        const vec2 waveLocalPosition =  DynamicWavesBuffer.Waves[i].ObjectPosition.xz - DisplacedPosition.xz;
        const vec2 waveTexCoord = waveLocalPosition / DynamicWavesBuffer.Waves[i].WaveFieldSize + vec2(0.5);
        const float texel = 1.0 / float(textureSize(DynamicWavesBuffer.Waves[i].WaveFieldTexture, 0).x);

        const float hC = texture(DynamicWavesBuffer.Waves[i].WaveFieldTexture, waveTexCoord).r;
        const float hX = texture(DynamicWavesBuffer.Waves[i].WaveFieldTexture, waveTexCoord + vec2(texel, 0)).r;
        const float hY = texture(DynamicWavesBuffer.Waves[i].WaveFieldTexture, waveTexCoord + vec2(0, texel)).r;

        const vec2 dh = vec2((hX - hC) / texel, (hY - hC) / texel);
        dynamicSlope += dh;
    }
    const vec3 normal = NormalFromSlope(slope + dynamicSlope);
    const vec3 viewSpaceNormal = mat3(ViewMatrix) * normal;

    const vec3 ViewDirection = normalize(CameraPosition - DisplacedPosition);
    vec3 color = CalculateLight(surfaceColor, metallic, roughness, normal, DisplacedPosition, ViewDirection, 1.0);

    const vec2 ndcCoordinates = ClipSpacePosition.xy / ClipSpacePosition.w;
    const vec2 screenCoordinates = ndcCoordinates * 0.5 + 0.5;

    const vec2 distortedScreenUV = screenCoordinates + viewSpaceNormal.xy * 0.01;

    float waterSurfaceDepth = gl_FragCoord.z;
    const float backgroundDepth = texture(SceneDepth, distortedScreenUV).r;

    const vec2 screenUV =  backgroundDepth < waterSurfaceDepth ? screenCoordinates : distortedScreenUV;

    color = mix(texture(SceneColor, screenUV).rgb, color, 0.5);

    FragColor = vec4(color, 1.0);
}
