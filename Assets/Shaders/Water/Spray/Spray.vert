#version 460
#include "Water/Spray/SprayCommon.glsl"

layout(location = 0) in vec3 inputPosition; // Quad vertex position
layout(location = 1) in vec2 inputTexCoord;

DEFINE_PARTICLE_DATA_BUFFER(1, readonly);
DEFINE_PARTICLE_ALIVE_LIST_BUFFER(4, readonly);

void main()
{
    const uint particleIndex = AliveListBuffer.Indices[gl_InstanceID];
    const particle particle = ParticleDataBuffer.Particles[particleIndex];

    const vec3 cameraRight = vec3(ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
    const vec3 cameraUp = vec3(ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);

    const vec3 billboardOffset = ((inputTexCoord.x - 0.5) * cameraRight + (inputTexCoord.y - 0.5) * cameraUp) * 0.1;
    const vec3 worldPos = particle.Position + billboardOffset;

    gl_Position = ProjectionMatrix * ViewMatrix * vec4(worldPos, 1.0);
}
