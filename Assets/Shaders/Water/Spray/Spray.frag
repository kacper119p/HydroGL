#version 460
#include "Water/Spray/SprayCommon.glsl"

layout (binding = 5) uniform sampler2D Texture;

in vec2 TexCoord;
in float NormalizedLife;
in vec3 WorldPosition;

layout (location = 0) out float FragColor;

layout (early_fragment_tests) in;
void main()
{
    float alpha = texture(Texture, TexCoord).a;
    float distanceBlend = 1.0 - min(length(CameraPosition - WorldPosition) / SpawnRange, 1.0);
    FragColor = alpha * (1.0 - NormalizedLife) * SprayOpacity * distanceBlend * distanceBlend;
}
