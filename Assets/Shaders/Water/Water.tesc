#version 460 core

layout (vertices = 4) out;

in vec3 Position[];

out vec3 tcPosition[];

uniform vec3 CameraPosition;
uniform float MaxDistance = 50.0;
uniform float MinDistance = 10.0;
uniform float MaxTessLevel = 32.0;
uniform float MinTessLevel = 1.0;

float CalculateTesselationLevel(vec3 p0, vec3 p1)
{
    vec3 center = (p0 + p1) * 0.5;
    float distance = distance(center, CameraPosition);
    return mix(MaxTessLevel, MinTessLevel, clamp(distance / MaxDistance, 0.0, 1.0));
}

void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    tcPosition[gl_InvocationID] = Position[gl_InvocationID];

    if (gl_InvocationID == 0)
    {
        float tess0 = CalculateTesselationLevel(Position[0], Position[3]);
        float tess1 = CalculateTesselationLevel(Position[1], Position[0]);
        float tess2 = CalculateTesselationLevel(Position[2], Position[1]);
        float tess3 = CalculateTesselationLevel(Position[3], Position[2]);

        gl_TessLevelOuter[0] = tess0;
        gl_TessLevelOuter[1] = tess1;
        gl_TessLevelOuter[2] = tess2;
        gl_TessLevelOuter[3] = tess3;

        gl_TessLevelInner[0] = max(tess1, tess3);
        gl_TessLevelInner[1] = max(tess0, tess2);
    }
}
