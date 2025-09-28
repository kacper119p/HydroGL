#version 460 core

#include "WaterCommon.glsl"
#include "Lighting.glsl"

uniform vec3 CameraPosition;

in vec2 TexCoord;
in vec3 Position;

uniform vec2 WindVelocity = vec2(16.0, 16.0);
uniform float Fetch = 10000.0;
uniform float Delta = 1.0;
uniform float Swell = 0.5;

layout(std430, binding = 0) buffer CascadeSettingsBuffer
{
    int TextureResolution;
    int CascadeCount;
    float padding[2];
    cascadeSettings CascadeSettings[];
};

layout (binding = 3) uniform sampler2DArray DisplacementMap;
layout (binding = 4) uniform sampler2DArray NormalMap;

layout (location = 0) out vec4 FragColor;

layout (early_fragment_tests) in;
void main()
{
    vec3 geometryNormal = normalize(cross(dFdy(Position), dFdx(Position)));
    // vec3 normal = normalize(texture(NormalMap, TexCoord).rgb);
    vec3 ViewDirection = normalize(CameraPosition - Position);
    //vec3 color = CalculateLight(vec3(1.0, 1.0, 1.0), 0.0, 1.0, geometryNormal, Position, ViewDirection, 1.0);
    //vec3 color = CalculateLight(vec3(1.0, 1.0, 1.0), 1.0, 0.0, normal, Position, ViewDirection, 1.0);
    //vec3 color = CalculateLight(vec3(0.0, 0.1, 0.25), 0.05, 0.8, normal, Position, ViewDirection, 1.0);
    vec3 color = CalculateLight(vec3(0.0, 0.1, 0.25), 0.05, 0.8, geometryNormal, Position, ViewDirection, 1.0);

    FragColor = vec4(color, 1.0);
    // FragColor = vec4(normal, 1.0);
    //FragColor = vec4(texture(DisplacementMap, TexCoord).rgb + 0.1, 1.0);
    //FragColor = vec4(1.0, 0.0, 1.0, 1.0);
}
