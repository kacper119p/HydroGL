#version 460 core

#include "Lighting.glsl"

uniform vec3 CameraPosition;

in vec2 TexCoord;
in vec3 Position;
in vec3 Normal;
in vec3 Tangent;

layout (location = 0) out vec4 FragColor;

layout (early_fragment_tests) in;
void main()
{
    vec3 ViewDirection = normalize(CameraPosition - Position);
    vec3 Color = CalculateLight(vec3(0.0, 0.4, 0.8), 0.1, 0.1, Normal, Position, ViewDirection, 1.0);

    FragColor = vec4(Color, 1.0);
}