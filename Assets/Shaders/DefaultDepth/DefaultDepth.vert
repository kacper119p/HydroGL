#version 460 core

layout (location = 0) in vec3 inputPosition;

uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

layout(std430, binding = 1) readonly restrict buffer TransformBuffer
{
    mat4 ObjectToWorldMatrices[];
};

void main()
{
    uint drawID = gl_DrawID;
    mat4 ObjectToWorldMatrix = ObjectToWorldMatrices[drawID];
    gl_Position = ProjectionMatrix * ViewMatrix * ObjectToWorldMatrix * vec4(inputPosition, 1.0);
}
