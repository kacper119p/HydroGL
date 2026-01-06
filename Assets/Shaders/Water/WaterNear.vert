#version 460 core

#include "Water/WaterCommon.glsl"

layout (location = 0) in vec3 inputLocalPosition;
layout (location = 1) in vec4 inputInstanceData;

out vec3 Position;

DEFINE_WATER_PARAMETERS_BUFFER(1);

vec2 MorphVertex(const vec2 LocalSpacePosition, const vec2 WorldSpacePosition, const float MorphFactor)
{
    const vec2 fraction = fract(LocalSpacePosition * float(WaterParameters.PatchSubdivisions) * 0.5) * 2.0 / float(WaterParameters.PatchSubdivisions);
    return WorldSpacePosition - fraction * inputInstanceData.z * MorphFactor;
}

void main()
{
    const vec2 patchPosition = inputInstanceData.xy;
    const float patchScale = inputInstanceData.z;
    const uint lodLevel = uint(inputInstanceData.w);

    const vec3 localPosition = inputLocalPosition;

    const vec2 worldPosition2D = patchPosition + (localPosition.xz * patchScale);

    const float distanceToCamera = distance(WaterParameters.CameraPosition, vec3(worldPosition2D.x, 0.0, worldPosition2D.y));

    float lodRangeLow = 0.0;
    float lodRangeHigh = WaterParameters.MinLodDistance;

    if (lodLevel != 0)
    {
        lodRangeLow = WaterParameters.MinLodDistance * pow(2.0, float(lodLevel - 1));
        lodRangeHigh = lodRangeLow * 2.0;
    }

    const float factor = (distanceToCamera - lodRangeLow) / (lodRangeHigh - lodRangeLow);
    const float morphFactor = clamp(factor / 0.5 - 1.0, 0.0, 1.0);

    const vec2 morphedWorldPosition = MorphVertex(localPosition.xz + vec2(0.5), worldPosition2D, morphFactor);

    Position = vec3(morphedWorldPosition.x, 0.0, morphedWorldPosition.y);
}
