#pragma once
#include "Rendering/Shaders/Shader.h"
#include "Rendering/Textures/Texture.h"
#include "Utility/GlResourceUnique.h"

namespace Core
{
    /**
     * @brief Manages a particle system for simulating and rendering water spray.
     */
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
            alignas(16) glm::vec3 FoamColor;
            alignas(4) float Lifetime;
            alignas(4) float DeltaTime;
            alignas(4) float Time;
            alignas(4) float SprayOpacity;
            alignas(4) float SpraySize;
            alignas(4) float SpawnThreshold;
            alignas(4) float WaterVelocityInfluence;
            alignas(4) float WindInfluence;
        };

    private:
        static constexpr int32_t MaxParticleCount = 524288; // 2^19

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

        float _spawnRange = 150.0f;
        float _lifetime = 1.0;
        float _sprayOpacity = 0.2f;
        float _spraySize = 3.0f;
        float _spawnThreshold = 0.25f;
        float _waterVelocityInfluence = 0.1f;
        float _windInfluence = 0.1f;
        float _spawnRate = MaxParticleCount / _lifetime;

    private:
        static GlBufferUnique CreateParticleDataSsbo();

        static GlBufferUnique CreateFreeListSsbo();

        static GlBufferUnique CreateIndirectDrawSsbo();

        static GlBufferUnique CreateAliveListSsbo();

        static GlBufferUnique CreateSprayParametersUbo();

    public:
        /**
         * @brief Gets the maximum distance from the camera at which new particles can spawn.
         * @return The spawn range in meters.
         */
        [[nodiscard]] float GetSpawnRange() const
        {
            return _spawnRange;
        }

        /**
         * @brief Sets the maximum distance from the camera at which new particles can spawn.
         * @param SpawnRange The spawn range in meters.
         */
        void SetSpawnRange(const float SpawnRange)
        {
            _spawnRange = SpawnRange;
        }

        /**
         * @brief Gets the maximum lifetime of a single spray particle.
         * @return The lifetime in seconds.
         */
        [[nodiscard]] float GetLifetime() const
        {
            return _lifetime;
        }

        /**
         * @brief Sets the maximum lifetime of a single spray particle and recalculates the spawn rate.
         * @param Lifetime The lifetime in seconds.
         */
        void SetLifetime(const float Lifetime)
        {
            _lifetime = Lifetime;
            _spawnRate = MaxParticleCount / _lifetime;
        }

        /**
         * @brief Gets the base opacity level for rendered spray particles.
         * @return The opacity value.
         */
        [[nodiscard]] float GetSprayOpacity() const
        {
            return _sprayOpacity;
        }

        /**
         * @brief Sets the base opacity level for rendered spray particles.
         * @param SprayOpacity The opacity value (0.0 to 1.0).
         */
        void SetSprayOpacity(const float SprayOpacity)
        {
            _sprayOpacity = SprayOpacity;
        }

        /**
         * @brief Gets the visual size of the spray particles.
         * @return The particles' size.
         */
        [[nodiscard]] float GetSpraySize() const
        {
            return _spraySize;
        }

        /**
         * @brief Sets the  size of the spray particles.
         * @param SpraySize The particles' size.
         */
        void SetSpraySize(const float SpraySize)
        {
            _spraySize = SpraySize;
        }

        /**
         * @brief Gets the threshold value determining when spray particles should be spawned based on foam mask.
         * @return The spawn threshold value.
         */
        [[nodiscard]] float GetSpawnThreshold() const
        {
            return _spawnThreshold;
        }

        /**
         * @brief Sets the threshold value determining when spray particles should be spawned based on foam mask.
         * @param SpawnThreshold The spawn threshold value.
         */
        void SetSpawnThreshold(const float SpawnThreshold)
        {
            _spawnThreshold = SpawnThreshold;
        }

        /**
         * @brief Gets the influence factor of underlying water velocity on the particles' motion.
         * @return The velocity influence factor.
         */
        [[nodiscard]] float GetWaterVelocityInfluence() const
        {
            return _waterVelocityInfluence;
        }

        /**
         * @brief Sets the influence factor of underlying water velocity on the particles' motion.
         * @param WaterVelocityInfluence The velocity influence factor.
         */
        void SetWaterVelocityInfluence(const float WaterVelocityInfluence)
        {
            _waterVelocityInfluence = WaterVelocityInfluence;
        }

        /**
         * @brief Gets the influence factor of global wind on the particles' motion.
         * @return The wind influence factor.
         */
        [[nodiscard]] float GetWindInfluence() const
        {
            return _windInfluence;
        }

        /**
         * @brief Sets the influence factor of global wind on the particles' motion.
         * @param WindInfluence The wind influence factor.
         */
        void SetWindInfluence(const float WindInfluence)
        {
            _windInfluence = WindInfluence;
        }

    public:
        /**
         * @brief Initializes the SprayManager.
         * @param WindowSize The dimensions of the render target.
         * @param MainDepthBuffer The OpenGL texture ID of the main depth buffer used for depth testing.
         */
        SprayManager(const glm::ivec2& WindowSize, uint32_t MainDepthBuffer);

    public:
        /**
         * @brief Destructor.
         */
        ~SprayManager();

    public:
        /**
         * @brief Executes compute shaders to update the particle system (spawning new particles and updating active ones).
         * @param DeltaTime The elapsed time since the last frame in seconds.
         * @param DisplacementMaps OpenGL texture array ID containing water displacement data.
         * @param SlopeMaps OpenGL texture array ID containing water slope data.
         * @param VelocityMaps OpenGL texture array ID containing water surface velocity data.
         * @param CascadeSettingsSsbo OpenGL SSBO ID containing water cascade configurations.
         * @param FoamSettingsUbo OpenGL UBO ID containing parameters for foam generation.
         * @param WindVelocity A 2D vector representing the velocity of the global wind.
         * @param SceneColor The ambient/directional color of the scene for blending.
         * @param FoamColor The base color applied to the generated foam and spray.
         * @param CameraPosition The current position of the camera in world space.
         * @param ViewMatrix The current view matrix.
         * @param ProjectionMatrix The current projection matrix.
         */
        void Update(float DeltaTime,
                    uint32_t DisplacementMaps,
                    uint32_t SlopeMaps,
                    uint32_t VelocityMaps,
                    uint32_t CascadeSettingsSsbo,
                    uint32_t FoamSettingsUbo,
                    const glm::vec2& WindVelocity,
                    const glm::vec3& SceneColor,
                    const glm::vec3& FoamColor,
                    const glm::vec3& CameraPosition,
                    const glm::mat4& ViewMatrix,
                    const glm::mat4& ProjectionMatrix) const;

        /**
         * @brief Renders the particles to the specified framebuffer.
         * @param TargetFrameBuffer The OpenGL ID of the target Framebuffer Object where particles will be rendered.
         */
        void Render(uint32_t TargetFrameBuffer) const;

    private:
        void PrepareNextFrame(const glm::mat4& ViewMatrix,
                              const glm::mat4& ProjectionMatrix,
                              const glm::vec3& CameraPosition,
                              const glm::vec3& SceneColor,
                              const glm::vec3& FoamColor,
                              const glm::vec2& WindVelocity,
                              uint32_t ParticlesToSpawn,
                              float DeltaTime) const;

        void BindData(uint32_t CascadeSettingsSsbo, uint32_t FoamSettingsUbo,
                      uint32_t DisplacementMaps, uint32_t SlopeMaps, uint32_t VelocityMaps) const;

        void CreateParticleFbo(const glm::ivec2& WindowSize, uint32_t MainDepthTexture);
    };
}
