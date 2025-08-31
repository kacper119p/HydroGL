#version 460 core

#include "Lighting.glsl"

in vec2 TexCoord;
in vec3 Position;
in vec3 Normal;
in vec3 Tangent;

out vec4 FragColor;

uniform vec3 CameraPosition;

void main()
{
    vec3 ViewDirection = normalize(CameraPosition - Position);
    vec3 Color = CalculateLight(vec3(0.0, 0.4, 0.8), 0.1, 0.1, Normal, Position, ViewDirection, 0.0);

    FragColor = vec4(Color, 1.0);
}