#version 460 core
#include "Water/Spray/SprayCommon.glsl"

in vec2 TexCoords;

layout (binding = 0) uniform sampler2D Input;

layout (location = 0) out vec4 Output;

void main()
{
    Output = vec4(SceneColor, clamp(texture(Input, TexCoords).r, 0, 1));
}
