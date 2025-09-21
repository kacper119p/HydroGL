#version 460 core

#include "Lighting.glsl"

uniform vec3 CameraPosition;

in vec2 TexCoord;
in vec3 Position;

layout (binding = 3) uniform sampler2D DisplacementMap;
layout (binding = 4) uniform sampler2D NormalMap;

layout (location = 0) out vec4 FragColor;

layout (early_fragment_tests) in;
void main()
{
    vec3 geometryNormal = normalize(cross(dFdy(Position), dFdx(Position)));
    vec3 normal = normalize(texture(NormalMap, TexCoord).rgb);
    vec3 ViewDirection = normalize(CameraPosition - Position);
    //vec3 color = CalculateLight(vec3(1.0, 1.0, 1.0), 0.0, 1.0, geometryNormal, Position, ViewDirection, 1.0);
    //vec3 color = CalculateLight(vec3(1.0, 1.0, 1.0), 1.0, 0.0, normal, Position, ViewDirection, 1.0);
    vec3 color = CalculateLight(vec3(0.0, 0.1, 0.25), 0.05, 0.8, normal, Position, ViewDirection, 1.0);

    FragColor = vec4(color, 1.0);
    // FragColor = vec4(normal, 1.0);
    //FragColor = vec4(texture(DisplacementMap, TexCoord).rgb + 0.1, 1.0);
    //FragColor = vec4(1.0, 0.0, 1.0, 1.0);
}
