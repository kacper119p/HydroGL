#version 460 core

layout (quads, equal_spacing, ccw) in;

in vec3 tcPosition[];
out vec2 TexCoord;
out vec3 Position;

uniform float OceanSize;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

layout (binding = 3) uniform sampler2D DisplacementMap;

void main()
{
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    vec3 p0 = mix(tcPosition[0], tcPosition[1], u);
    vec3 p1 = mix(tcPosition[3], tcPosition[2], u);
    vec3 position = mix(p0, p1, v);

    vec2 texCoord = vec2(position.x, position.z) / OceanSize;

    vec3 finalPosition = position + texture(DisplacementMap, texCoord).xyz;

    Position = finalPosition;
    TexCoord = texCoord;

    gl_Position = ProjectionMatrix * ViewMatrix * vec4(finalPosition, 1.0);
}