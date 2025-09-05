#version 460 core

#include "Lighting.glsl"

uniform vec3 CameraPosition;

in vec2 TexCoord;
in vec3 Position;

layout (binding = 4) uniform sampler2D NormalMap;

layout (location = 0) out vec4 FragColor;

layout (early_fragment_tests) in;
void main()
{
    vec3 normal = normalize(texture(NormalMap, TexCoord).rgb);
    vec3 ViewDirection = normalize(CameraPosition - Position);
    vec3 Color = CalculateLight(vec3(0.0, 0.4, 0.8), 0.05, 0.5, normal, Position, ViewDirection, 1.0);

    FragColor = vec4(Color, 1.0);
}