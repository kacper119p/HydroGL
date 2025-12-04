#version 460
#include "Water/Spray/SprayCommon.glsl"

layout (location = 0) out vec4 FragColor;

layout (early_fragment_tests) in;
void main()
{
    FragColor = vec4(SceneColor, 0.5);
}
