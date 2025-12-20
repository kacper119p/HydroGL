#version 460 core

#include "Water/WaterCommon.glsl"

layout (location = 0) in vec3 inputLocalPosition;
layout (location = 1) in vec3 inputPatchPosition;

out vec3 Position;

DEFINE_WATER_PARAMETERS_BUFFER(1);

void main()
{
    Position = inputLocalPosition + inputPatchPosition + vec3(WaterParameters.GridOrigin.x, 0.0, WaterParameters.GridOrigin.y);
    gl_Position = vec4(Position, 1.0);
}
