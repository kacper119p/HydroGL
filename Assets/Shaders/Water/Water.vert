#version 460 core

layout (location = 0) in vec3 inputLocalPosition;
layout (location = 1) in vec3 inputPatchPosition;

out vec3 Position;

uniform vec2 GridOrigin;

void main()
{
    Position = inputLocalPosition + inputPatchPosition + vec3(GridOrigin.x, 0.0, GridOrigin.y);
    gl_Position = vec4(Position, 1.0);
}
