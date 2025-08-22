#version 460 core
#extension GL_ARB_bindless_texture: enable

#include "Lighting.glsl"

in vec2 TexCoord;
in vec3 Position;
in vec3 Normal;
in vec3 Tangent;

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


layout (location = 0) out vec3 FragColor;

layout (early_fragment_tests) in;
void main()
{
    vec3 BaseColor = texture(Materials[0].BaseMap, TexCoord).rgb * Materials[0].BaseColor;
    vec3 RoughnessMetallicAmbientOcclusion = texture(Materials[0].RoughnessMetallicAmbientOcclusionMap, TexCoord).rgb;
    float Roughness = RoughnessMetallicAmbientOcclusion.r * Materials[0].Roughness;
    float Metallic = RoughnessMetallicAmbientOcclusion.g * Materials[0].Metallic;
    float AmbientOcclusion = RoughnessMetallicAmbientOcclusion.b;

    vec3 Normal = normalize(Normal);
    vec3 Tangent = normalize(Tangent);
    vec3 Binormal = cross(Normal, Tangent);

    mat3 TangentToWorldMatrix = mat3(Tangent, Binormal, Normal);
    Normal = texture(Materials[0].NormalMap, TexCoord).rgb * 2.0 - 1.0;
    Normal = normalize(TangentToWorldMatrix * Normal);

    vec3 ViewDirection = normalize(CameraPosition - Position);

    vec3 Color = CalculateLight(BaseColor, Metallic, Roughness, Normal, Position, ViewDirection, AmbientOcclusion);

    vec3 Emissive = texture(Materials[0].EmissiveMap, TexCoord).rgb * Materials[0].EmissiveColor;
    Color += Emissive;
    Color = vec3(1.0, 1.0, 1.0);

    FragColor = Color;
}
