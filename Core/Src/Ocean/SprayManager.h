#pragma once
#include "Rendering/Shaders/Shader.h"
#include "Utility/GlResourceUnique.h"

namespace Core
{
    class SprayManager
    {
    private:
        struct alignas(16) ParticleData
        {
            glm::vec3 Position;
            float Rotation;
            glm::vec3 Velocity;
            float LifeTime;
        };

    private:
        struct alignas(16) SprayParameters
        {
            alignas(16)glm::mat4 ViewMatrix;
            alignas(16)glm::mat4 ProjectionMatrix;
            alignas(16)glm::vec3 CameraPosition;
            alignas(4) uint32_t TotalParticleCount;
            alignas(4) uint32_t ParticlesSpawnedThisFrame;
            alignas(4) uint32_t Seed;
            alignas(4) float SpawnRange;
            alignas(4) float Lifetime;
            alignas(4) float DeltaTime;
        };

    private:
        static constexpr float MaxLifetime = 10.0f;
        static constexpr int32_t MaxParticleCount = 2048;
        static constexpr float SpawnRate = MaxParticleCount / MaxLifetime;

    private:
        Shader _spawnShader;
        Shader _updateShader;
        Shader _renderShader;

        GlBufferUnique _particleDataSsbo;
        GlBufferUnique _freeListSsbo;
        GlBufferUnique _indirectDrawBuffer;
        GlBufferUnique _aliveListSsbo;
        GlBufferUnique _sprayParametersUbo;

    private:
        static GlBufferUnique CreateParticleDataSsbo();

        static GlBufferUnique CreateFreeListSsbo();

        static GlBufferUnique CreateIndirectDrawSsbo();

        static GlBufferUnique CreateAliveListSsbo();

        static GlBufferUnique CreateSprayParametersUbo();

    public:
        SprayManager();

    public:
        ~SprayManager();

        void Update(float DeltaTime,
                    uint32_t DisplacementMaps,
                    uint32_t SlopeMaps,
                    uint32_t CascadeSettingsSsbo,
                    const glm::vec3& CameraPosition,
                    const glm::mat4& ViewMatrix,
                    const glm::mat4& ProjectionMatrix) const;

        void Render() const;
    };
}
