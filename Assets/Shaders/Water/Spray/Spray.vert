#version 460
#include "Water/Spray/SprayCommon.glsl"

layout(location = 0) in vec3 inputPosition;
layout(location = 1) in vec2 inputTexCoord;

DEFINE_PARTICLE_DATA_BUFFER(1, readonly);
DEFINE_PARTICLE_ALIVE_LIST_BUFFER(4, readonly);

out vec2 TexCoord;
out float NormalizedLife;
out vec3 WorldPosition;

void main()
{
    const uint particleIndex = AliveListBuffer.Indices[gl_InstanceID];
    const particle particle = ParticleDataBuffer.Particles[particleIndex];

    const vec3 cameraRight = vec3(ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
    const vec3 cameraUp = vec3(ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);

    float rotation = particle.Rotation;
    float rotationCos = cos(rotation);
    float rotationSin = sin(rotation);

    vec2 offset = inputTexCoord - 0.5;

    vec2 rotatedOffset;
    rotatedOffset.x = offset.x * rotationCos - offset.y * rotationSin;
    rotatedOffset.y = offset.x * rotationSin + offset.y * rotationCos;

    float normalizedLife = 1.0 - particle.LifeTime / LifeTime;
    float size = normalizedLife * SpraySize;

    const vec3 billboardOffset = (rotatedOffset.x * cameraRight + (rotatedOffset.y) * cameraUp) * size ;

    const vec3 worldPosition = particle.Position + billboardOffset;

    gl_Position = ProjectionMatrix * ViewMatrix * vec4(worldPosition, 1.0);
    TexCoord = inputTexCoord;
    NormalizedLife = normalizedLife;
    WorldPosition = worldPosition;
}
