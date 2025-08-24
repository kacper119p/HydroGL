#version 460 core
#include "Tonemapping.glsl"

in vec2 TexCoords;

layout (binding = 0) uniform sampler2D SceneColor;
layout (binding = 1) uniform sampler2D BloomColor;

const float GAMMA = 2.2;
const float BlOOM_INTENSITY = 0.1;

layout (location = 0) out vec3 FragColor;

void main()
{
    vec3 hdrColor = texture(SceneColor, TexCoords).rgb;
    vec3 bloomColor = max(vec3(0.0), texture(BloomColor, TexCoords).rgb) * BlOOM_INTENSITY;

    hdrColor += bloomColor;

    vec3 result = ApplyTonemapping(hdrColor);

    result = pow(result, vec3(1.0 / GAMMA));

    FragColor = result;
}