#version 460

layout (location = 0) out vec4 FragColor;

layout (early_fragment_tests) in;
void main()
{
    FragColor = vec4(1.0);
}
