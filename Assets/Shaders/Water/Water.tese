#version 460 core

#include "WaterCommon.glsl"

layout (quads, equal_spacing, ccw) in;

in vec3 tcPosition[];
out vec3 OriginalPosition;
out vec3 DisplacedPosition;

uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

layout(std430, binding = 0) buffer CascadeSettingsBuffer
{
    int TextureResolution;
    int CascadeCount;
    float padding[2];
    cascadeSettings CascadeSettings[];
};

layout (binding = 3) uniform sampler2DArray DisplacementMaps;


void main()
{
    const float u = gl_TessCoord.x;
    const float v = gl_TessCoord.y;

    const vec3 p0 = mix(tcPosition[0], tcPosition[1], u);
    const vec3 p1 = mix(tcPosition[3], tcPosition[2], u);
    const vec3 position = mix(p0, p1, v);

    vec3 displacement = vec3(0.0);

    for (int i = 0; i < CascadeCount; ++i)
    {
        const vec2 texCoord = vec2(position.x, position.z) / CascadeSettings[i].Length;
        displacement += texture(DisplacementMaps, vec3(texCoord, i)).xyz;
    }
    vec3 finalPosition = position + displacement;

    OriginalPosition = position;
    DisplacedPosition = finalPosition;

    gl_Position = ProjectionMatrix * ViewMatrix * vec4(finalPosition, 1.0);
}
