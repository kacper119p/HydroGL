#version 460 core

in vec2 TexCoords;

layout (binding = 0) uniform sampler2DArray textureArray;
uniform int LayerIndex;

out vec4 FragColor;

void main()
{
    FragColor = vec4(texture(textureArray, vec3(TexCoords, float(LayerIndex))).rgb, 1.0);
}