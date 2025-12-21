#version 460 core

layout (location = 0) in vec3 inputPosition;

layout(std430, binding = 1) readonly restrict buffer TransformBuffer
{
    mat4 ObjectToWorldMatrices[];
};

void main()
{
    uint drawID = gl_DrawID;
    mat4 ObjectToWorldMatrix = ObjectToWorldMatrices[drawID];
    gl_Position = ObjectToWorldMatrix * vec4(inputPosition, 1.0);
}
