#version 460 core
#extension GL_ARB_bindless_texture: enable

#include "Lighting.glsl"
#include "ColorCorrection.glsl"

in vec2 TexCoord;
in vec3 Position;
in vec3 Normal;
in vec3 Tangent;
flat in int MaterialIndex;

uniform vec3 CameraPosition;

struct Material
{
    vec3 BaseColor;
    float Roughness;
    vec3 EmissiveColor;
    float Metallic;

    layout(bindless_sampler) sampler2D BaseMap;
    layout(bindless_sampler) sampler2D  RoughnessMetallicAmbientOcclusionMap;
    layout(bindless_sampler) sampler2D  NormalMap;
    layout(bindless_sampler) sampler2D EmissiveMap;
};

layout(std430, binding = 0) readonly restrict buffer MaterialBuffer
{
    Material Materials[256];
};


layout (location = 0) out vec4 FragColor;

layout (early_fragment_tests) in;
void main()
{
    vec3 BaseColor = SrgbToLinear(texture(Materials[MaterialIndex].BaseMap, TexCoord).rgb) * Materials[MaterialIndex].BaseColor;
    vec3 RoughnessMetallicAmbientOcclusion = texture(Materials[MaterialIndex].RoughnessMetallicAmbientOcclusionMap, TexCoord).rgb;
    float Roughness = RoughnessMetallicAmbientOcclusion.r * Materials[MaterialIndex].Roughness;
    float Metallic = RoughnessMetallicAmbientOcclusion.g * Materials[MaterialIndex].Metallic;
    float AmbientOcclusion = RoughnessMetallicAmbientOcclusion.b;

    vec3 Normal = normalize(Normal);
    vec3 Tangent = normalize(Tangent);
    vec3 Binormal = cross(Normal, Tangent);

    mat3 TangentToWorldMatrix = mat3(Tangent, Binormal, Normal);
    Normal = texture(Materials[MaterialIndex].NormalMap, TexCoord).rgb * 2.0 - 1.0;
    Normal = normalize(TangentToWorldMatrix * Normal);

    vec3 ViewDirection = normalize(CameraPosition - Position);

    vec3 Color = CalculateLight(BaseColor, Metallic, Roughness, Normal, Position, ViewDirection, AmbientOcclusion);

    vec3 Emissive = texture(Materials[MaterialIndex].EmissiveMap, TexCoord).rgb * Materials[MaterialIndex].EmissiveColor;
    Color += Emissive;
    Color = ApplyFog(Color, LinearizeDepth(gl_FragCoord.z), ViewDirection, LightsBuffer.DirectionalLight.Direction);

    FragColor = vec4(Color, 1.0);
}
