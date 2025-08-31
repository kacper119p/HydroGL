#version 460 core

layout (vertices = 4) out;

in vec3 Position[];

out vec3 tcPosition[];

uniform vec3 CameraPosition;
uniform float MaxDistance = 100.0;
uniform float MinDistance = 10.0;
uniform float MaxTessLevel = 64.0;
uniform float MinTessLevel = 1.0;

void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    tcPosition[gl_InvocationID] = Position[gl_InvocationID];

    if (gl_InvocationID == 0)
    {
        vec3 center = (Position[0] + Position[1] + Position[2] + Position[3]) / 4.0;
        float distance = distance(center, CameraPosition);
        float noramlizedDistance =  clamp((distance - MinDistance) / (MaxDistance - MinDistance), 0.0, 1.0);
        float tessLevel = mix(MaxTessLevel, MinTessLevel, noramlizedDistance * noramlizedDistance);

        gl_TessLevelOuter[0] = tessLevel;
        gl_TessLevelOuter[1] = tessLevel;
        gl_TessLevelOuter[2] = tessLevel;
        gl_TessLevelOuter[3] = tessLevel;

        gl_TessLevelInner[0] = tessLevel;
        gl_TessLevelInner[1] = tessLevel;
    }
}