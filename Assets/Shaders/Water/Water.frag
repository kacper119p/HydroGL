#version 460 core

#include "WaterCommon.glsl"
#include "Normal.glsl"
#include "Lighting.glsl"

uniform vec3 CameraPosition;

in vec2 TexCoord;
in vec3 OriginalPosition;
in vec3 DisplacedPosition;

uniform vec2 WindVelocity = vec2(16.0, 16.0);
uniform float Fetch = 10000.0;
uniform float Delta = 1.0;
uniform float Swell = 0.5;

layout(std430, binding = 0) readonly buffer CascadeSettingsBuffer
{
    int TextureResolution;
    int CascadeCount;
    float padding[2];
    cascadeSettings CascadeSettings[];
};

layout (binding = 3) uniform sampler2DArray DisplacementMaps;
layout (binding = 4) uniform sampler2DArray SlopeMaps;

layout (location = 0) out vec4 FragColor;

layout (early_fragment_tests) in;
void main()
{
    vec3 geometryNormal = normalize(cross(dFdy(DisplacedPosition), dFdx(DisplacedPosition)));

    const vec2 texCoord = vec2(OriginalPosition.x, OriginalPosition.z) / CascadeSettings[0].Length;
    vec4 slopeDfdxxDfdzz =  texture(SlopeMaps, vec3(texCoord, 0));
    float dfdzx = texture(DisplacementMaps, vec3(texCoord, 0)).w;

    for (int i = 1; i < CascadeCount; ++i)
    {
        const vec2 texCoord = vec2(OriginalPosition.x, OriginalPosition.z) / CascadeSettings[i].Length;
        slopeDfdxxDfdzz += texture(SlopeMaps, vec3(texCoord, i));
        dfdzx += texture(DisplacementMaps, vec3(texCoord, i)).w;
    }

    const vec2 slope = slopeDfdxxDfdzz.xy;
    const float jacobian = (1.0 + slopeDfdxxDfdzz.z) * (1.0 + slopeDfdxxDfdzz.w) - dfdzx * dfdzx;

    const float FoamThreshold = 1.1;
    const float FoamExponent  = 1.0;
    const float FoamIntensity = 5.0;

    float compression = clamp((jacobian - FoamThreshold) / FoamThreshold, 0.0, 1.0);
    float foamMask = clamp(FoamIntensity * pow(compression, FoamExponent), 0.0, 1.0);
    vec3 surfaceColor = mix(vec3(0.005, 0.1, 0.2), vec3(1.0, 1.0, 1.0), foamMask);
    float metallic = mix(0.05, 0.0, foamMask);
    float roughness = mix(0.5, 1.0, foamMask);

    const vec3 normal = NormalFromSlope(slope);

    vec3 ViewDirection = normalize(CameraPosition - DisplacedPosition);
    //vec3 color = CalculateLight(vec3(1.0, 1.0, 1.0), 0.0, 1.0, geometryNormal, DisplacedPosition, ViewDirection, 1.0);
    //vec3 color = CalculateLight(vec3(1.0, 1.0, 1.0), 1.0, 0.0, normal, DisplacedPosition, ViewDirection, 1.0);
    vec3 color = CalculateLight(surfaceColor, metallic, roughness, normal, DisplacedPosition, ViewDirection, 1.0);
    //vec3 color = CalculateLight(vec3(0.0, 0.1, 0.25), 0.05, 0.5, geometryNormal, DisplacedPosition, ViewDirection, 1.0);

    FragColor = vec4(color, 1.0);
    // FragColor = vec4(vec3((1.0 - clamp(0.0, 1.0, abs(dot(normal, geometryNormal))))) * 100000.0, 1.0);
    //FragColor = vec4(normal, 1.0);
    //FragColor = vec4(texture(DisplacementMap, TexCoord).rgb + 0.1, 1.0);
    //FragColor = vec4(1.0, 0.0, 1.0, 1.0);
    //FragColor = vec4(geometryNormal, 1.0);
    //FragColor = vec4(abs(normal - geometryNormal), 1.0);
}
