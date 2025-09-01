#version 460 core

layout (vertices = 4) out;

in vec3 Position[];

out vec3 tcPosition[];

uniform vec3 CameraPosition;
uniform float MaxDistance = 100.0;
uniform float MinDistance = 10.0;
uniform float MaxTessLevel = 64.0;
uniform float MinTessLevel = 1.0;

float CalculateTessLevel(vec3 patchCenter)
{
    float distance = distance(patchCenter, CameraPosition);
    float noramlizedDistance =  clamp((distance - MinDistance) / (MaxDistance - MinDistance), 0.0, 1.0);
    return mix(MaxTessLevel, MinTessLevel, noramlizedDistance * noramlizedDistance);
}

void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    tcPosition[gl_InvocationID] = Position[gl_InvocationID];

    if (gl_InvocationID == 0)
    {
        vec3 center0 = Position[0] + (Position[3] - Position[0]) * 0.5;
        vec3 center1 = Position[1] + (Position[0] - Position[1]) * 0.5;
        vec3 center2 = Position[2] + (Position[1] - Position[2]) * 0.5;
        vec3 center3 = Position[3] + (Position[2] - Position[3]) * 0.5;

        float distance0 = length(center0 - CameraPosition);
        float distance1 = length(center1 - CameraPosition);
        float distance2 = length(center2 - CameraPosition);
        float distance3 = length(center3 - CameraPosition);

        float tess0 = mix(MaxTessLevel, MinTessLevel, clamp(distance0 / MaxDistance, 0.0, 1.0));
        float tess1 = mix(MaxTessLevel, MinTessLevel, clamp(distance1 / MaxDistance, 0.0, 1.0));
        float tess2 = mix(MaxTessLevel, MinTessLevel, clamp(distance2 / MaxDistance, 0.0, 1.0));
        float tess3 = mix(MaxTessLevel, MinTessLevel, clamp(distance3 / MaxDistance, 0.0, 1.0));

        gl_TessLevelOuter[0] = tess0;
        gl_TessLevelOuter[1] = tess1;
        gl_TessLevelOuter[2] = tess2;
        gl_TessLevelOuter[3] = tess3;

        gl_TessLevelInner[0] = max(tess1, tess3);
        gl_TessLevelInner[1] = max(tess0, tess2);
    }
}