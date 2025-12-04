#pragma once
#include "Rendering/Shaders/Shader.h"
#include "Rendering/Textures/Texture.h"
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
            alignas(16) glm::mat4 ViewMatrix;
            alignas(16) glm::mat4 ProjectionMatrix;
            alignas(16) glm::vec3 CameraPosition;
            alignas(4) uint32_t TotalParticleCount;
            alignas(16) glm::vec3 SceneColor;
            alignas(4) uint32_t ParticlesSpawnedThisFrame;
            alignas(16) const glm::vec2 WindVelocity;
            alignas(4) uint32_t Seed;
            alignas(4) float SpawnRange;
            alignas(4) float Lifetime;
            alignas(4) float DeltaTime;
            alignas(4) float Time;
        };

    private:
        static constexpr float MaxLifetime = 1.0f;
        static constexpr int32_t MaxParticleCount = 524288; // 2^19
        static constexpr float SpawnRate = MaxParticleCount / MaxLifetime;

    private:
        Shader _spawnShader;
        Shader _updateShader;
        Shader _renderShader;
        Shader _blendShader;

        GlBufferUnique _particleDataSsbo;
        GlBufferUnique _freeListSsbo;
        GlBufferUnique _indirectDrawBuffer;
        GlBufferUnique _aliveListSsbo;
        GlBufferUnique _sprayParametersUbo;

        GlFramebufferUnique _particleFbo;
        GlTextureUnique _particleOpacityTexture;

        Texture _sprayTexture;

    private:
        static GlBufferUnique CreateParticleDataSsbo();

        static GlBufferUnique CreateFreeListSsbo();

        static GlBufferUnique CreateIndirectDrawSsbo();

        static GlBufferUnique CreateAliveListSsbo();

        static GlBufferUnique CreateSprayParametersUbo();

    public:
        SprayManager(const glm::ivec2& WindowSize, uint32_t MainDepthBuffer);

    public:
        ~SprayManager();

    public:
        void Update(float DeltaTime,
                    uint32_t DisplacementMaps,
                    uint32_t SlopeMaps,
                    uint32_t VelocityMaps,
                    uint32_t CascadeSettingsSsbo,
                    uint32_t FoamSettingsUbo,
                    const glm::vec2& WindVelocity,
                    const glm::vec3& SceneColor,
                    const glm::vec3& CameraPosition,
                    const glm::mat4& ViewMatrix,
                    const glm::mat4& ProjectionMatrix) const;

        void Render(uint32_t TargetFrameBuffer) const;

    private:
        void PrepareNextFrame(const glm::mat4& ViewMatrix,
                              const glm::mat4& ProjectionMatrix,
                              const glm::vec3& CameraPosition,
                              const glm::vec3& SceneColor,
                              const glm::vec2& WindVelocity,
                              uint32_t ParticlesToSpawn,
                              float DeltaTime) const;

        void BindData(uint32_t CascadeSettingsSsbo, uint32_t FoamSettingsUbo,
                      uint32_t DisplacementMaps, uint32_t SlopeMaps, uint32_t VelocityMaps) const;

        void CreateParticleFbo(const glm::ivec2& WindowSize, uint32_t MainDepthBuffer);
    };
}
