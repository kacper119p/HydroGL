#version 460 core
#extension GL_ARB_bindless_texture : enable

#include "Water/WaterCommon.glsl"
#include "Normal.glsl"
#include "Lighting.glsl"

in vec3 OriginalPosition;
in vec3 DisplacedPosition;
in vec4 ClipSpacePosition;

DEFINE_CASCADE_SETTINGS_BUFFER(0);

DEFINE_FOAM_PARAMETERS_BUFFER(0);

DEFINE_WATER_PARAMETERS_BUFFER(1);

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

/**
 * @brief Converts non-linear depth buffer values (0.0 to 1.0) into linear view-space depth.
 */
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0;

    return (2.0 * WaterParameters.CameraNearPlane * WaterParameters.CameraFarPlane) / (WaterParameters.CameraFarPlane + WaterParameters.CameraNearPlane - z * (WaterParameters.CameraFarPlane - WaterParameters.CameraNearPlane));
}

layout (early_fragment_tests) in;
void main()
{
    // ------------------------------------------------------------------
    // Retrieve data from all FFT cascades
    // ------------------------------------------------------------------
    const vec2 texCoord = vec2(OriginalPosition.x, OriginalPosition.z) / CascadeSettingsBuffer.CascadeSettings[0].Length;
    vec4 slopeDfdxxDfdzz =  texture(SlopeMaps, vec3(texCoord, 0));
    float dfdzx = texture(DisplacementMaps, vec3(texCoord, 0)).w;

    const float lowFrequencyJacobian = (1.0 + slopeDfdxxDfdzz.z) * (1.0 + slopeDfdxxDfdzz.w) - dfdzx * dfdzx;

    for (int i = 1; i < CascadeSettingsBuffer.CascadeCount; ++i)
    {
        const vec2 texCoord = vec2(OriginalPosition.x, OriginalPosition.z) / CascadeSettingsBuffer.CascadeSettings[i].Length;
        slopeDfdxxDfdzz += texture(SlopeMaps, vec3(texCoord, i));
        dfdzx += texture(DisplacementMaps, vec3(texCoord, i)).w;
    }

    // ------------------------------------------------------------------
    // Foam calculation and blending with water
    // ------------------------------------------------------------------
    const vec2 slope = slopeDfdxxDfdzz.xy;
    const float jacobian = (1.0 + slopeDfdxxDfdzz.z) * (1.0 + slopeDfdxxDfdzz.w) - dfdzx * dfdzx;

    const float compression = clamp((FoamParameters.FoamThreshold - jacobian) / FoamParameters.FoamThreshold, 0.0, 1.0);
    const float foamMask = clamp(FoamParameters.FoamIntensity * pow(compression, FoamParameters.FoamExponent), 0.0, 1.0);
    const vec3 baseSurfaceColor = mix(WaterParameters.SurfaceColor, WaterParameters.FoamColor, foamMask);
    const float metallic = mix(WaterParameters.Metallic, WaterParameters.FoamMetallic, foamMask);
    const float roughness = mix(WaterParameters.Roughness, WaterParameters.FoamRoughness, foamMask);

    // ------------------------------------------------------------------
    // Adding height and slope contributions from dynamic waves
    // ------------------------------------------------------------------
    vec2 dynamicSlope = vec2(0.0);
    for (int i = 0; i < DynamicWavesBuffer.DynamicWavesCount; ++i)
    {
        const vec2 waveLocalPosition =  DynamicWavesBuffer.Waves[i].ObjectPosition.xz - DisplacedPosition.xz;
        const vec2 waveTexCoord = waveLocalPosition / DynamicWavesBuffer.Waves[i].WaveFieldSize + vec2(0.5);
        const float texelSize = 1.0 / float(textureSize(DynamicWavesBuffer.Waves[i].WaveFieldTexture, 0).x);

        const float hC = texture(DynamicWavesBuffer.Waves[i].WaveFieldTexture, waveTexCoord).r;
        const float hX = texture(DynamicWavesBuffer.Waves[i].WaveFieldTexture, waveTexCoord + vec2(texelSize, 0)).r;
        const float hY = texture(DynamicWavesBuffer.Waves[i].WaveFieldTexture, waveTexCoord + vec2(0, texelSize)).r;

        const float physicalTexelSize = DynamicWavesBuffer.Waves[i].WaveFieldSize * texelSize;

        const vec2 dh = vec2((hX - hC) / physicalTexelSize, (hY - hC) / physicalTexelSize);
        dynamicSlope += dh;
    }

    // ------------------------------------------------------------------
    // Lighting
    // ------------------------------------------------------------------
    const vec3 normal = NormalFromSlope(slope + dynamicSlope);

    const vec3 ViewDirection = normalize(WaterParameters.CameraPosition - DisplacedPosition);
    const vec3 surfaceColor = CalculateLight(baseSurfaceColor, metallic, roughness, normal, DisplacedPosition, ViewDirection, 1.0);

    // ------------------------------------------------------------------
    // Refraction
    // ------------------------------------------------------------------
    const vec3 viewSpaceNormal = mat3(WaterParameters.ViewMatrix) * normal;

    const ivec2 screenSize = textureSize(SceneColor, 0);
    const float aspectRatio = float(screenSize.x) / float(screenSize.y);
    const vec2 distortion = vec2(viewSpaceNormal.x, viewSpaceNormal.y * aspectRatio) * 0.01;

    const vec2 ndcCoordinates = ClipSpacePosition.xy / ClipSpacePosition.w;
    const vec2 screenCoordinates = ndcCoordinates * 0.5 + 0.5;

    const float waterSurfaceDepth = gl_FragCoord.z;
    const float linearWaterDepth = LinearizeDepth(waterSurfaceDepth);

    const  float undistortedSceneDepth = texture(SceneDepth, screenCoordinates).r;
    const float linearUndistortedSceneDepth = LinearizeDepth(undistortedSceneDepth);

    const  float depthDifferenceStart = max(linearUndistortedSceneDepth - linearWaterDepth, 0.0);

    const  float softnessFactor = 2.0;
    const  float distortionScale = clamp(depthDifferenceStart * softnessFactor, 0.0, 1.0);

    const  vec2 finalDistortion = vec2(viewSpaceNormal.x, viewSpaceNormal.y * aspectRatio) * 0.01 * distortionScale;

    const   vec2 distortedScreenUV = screenCoordinates + finalDistortion;

    const float distortedSceneDepth = texture(SceneDepth, distortedScreenUV).r;
    const bool isOccluded = distortedSceneDepth < waterSurfaceDepth;

    const vec2 screenUV = isOccluded ? screenCoordinates : distortedScreenUV;

    // ------------------------------------------------------------------
    // Volumetric Absorption using Beer-Lambert law
    // ------------------------------------------------------------------

    const float finalBackgroundDepth = texture(SceneDepth, screenUV).r;
    const float linearFinalBackgroundDepth = LinearizeDepth(finalBackgroundDepth);
    const float finalDepthDifference = max(linearFinalBackgroundDepth - linearWaterDepth, 0.0);

    const vec3 transmittance = exp(-WaterParameters.Absorption * finalDepthDifference);

    const vec3 backgroundColor = texture(SceneColor, screenUV).rgb;
    const vec3 refractedColor = backgroundColor * transmittance;

    // ------------------------------------------------------------------
    // Subsurface Scattering
    // ------------------------------------------------------------------
    const float sssMask = clamp(WaterParameters.SssThreshold - lowFrequencyJacobian / WaterParameters.SssThreshold, 0.0, 1.0);

    const vec3 sssLight = normalize(LightsBuffer.DirectionalLight.Direction + (normal * WaterParameters.SssDistortion));
    const float sssDot = pow(clamp(dot(ViewDirection, -sssLight), 0.0, 1.0), WaterParameters.SssPower) * WaterParameters.SssScale;
    const float sssStrength = sssDot * sssMask;
    const vec3 sssColor = WaterParameters.SssColor * sssStrength * (1.0 - foamMask);
    // ------------------------------------------------------------------
    // Final color
    // ------------------------------------------------------------------

    const vec3 color = refractedColor + surfaceColor + sssColor;
    FragColor = vec4(color, 1.0);
}
