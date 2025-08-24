#version 460 core

in vec3 Direction;

layout (binding = 0) uniform samplerCube Texture;

layout (location = 0) out vec4 FragColor;

layout (early_fragment_tests) in;
void main()
{
    vec3 color = texture(Texture, normalize(Direction)).rgb;
    FragColor = vec4(color, 1.0);
}