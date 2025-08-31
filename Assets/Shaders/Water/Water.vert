#version 460 core

layout (location = 0) in vec3 inputLocalPosition;
layout (location = 1) in vec3 inputPatchPosition;

out vec3 Position;

uniform vec3 CameraPosition;

void main()
{
    Position = inputLocalPosition + inputPatchPosition + vec3(CameraPosition.x, 0.0, CameraPosition.z);
    gl_Position = vec4(Position, 1.0);
}
