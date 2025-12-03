#include "SprayManager.h"

#include "TimeManager.h"
#include "Rendering/RendererHelpers/DrawElementsIndirectCommand.h"
#include "Rendering/RendererHelpers/SpriteQuad.h"
#include "Rendering/Shaders/ShaderBuilder.h"

namespace Core
{
    GlBufferUnique SprayManager::CreateParticleDataSsbo()
    {
        void* InitialData = calloc(MaxParticleCount, sizeof(ParticleData));
        GlBufferUnique buffer = GlBufferUnique::Create();
        glNamedBufferStorage(buffer, MaxParticleCount * sizeof(ParticleData), InitialData, GL_DYNAMIC_STORAGE_BIT);
        free(InitialData);
        return buffer;
    }

    GlBufferUnique SprayManager::CreateFreeListSsbo()
    {
        uint32_t* const InitialData
                = static_cast<uint32_t*>(malloc(sizeof(int32_t) + MaxParticleCount * sizeof(uint32_t)));
        int32_t* freeCount = reinterpret_cast<int32_t*>(InitialData);
        *freeCount = MaxParticleCount;
        uint32_t* const indices = InitialData + 1;
        for (uint32_t i = 0; i < MaxParticleCount; ++i)
        {
            indices[i] = i;
        }
        GlBufferUnique buffer = GlBufferUnique::Create();
        glNamedBufferStorage(buffer, sizeof(int32_t) + MaxParticleCount * sizeof(uint32_t), InitialData,
                             GL_DYNAMIC_STORAGE_BIT);
        free(InitialData);
        return buffer;
    }

    GlBufferUnique SprayManager::CreateIndirectDrawSsbo()
    {
        DrawElementsIndirectCommand* InitialData = new DrawElementsIndirectCommand{6, 0, 0, 0, 0};
        GlBufferUnique buffer = GlBufferUnique::Create();
        glNamedBufferStorage(buffer, sizeof(DrawElementsIndirectCommand), InitialData, GL_DYNAMIC_STORAGE_BIT);
        free(InitialData);
        return buffer;
    }

    GlBufferUnique SprayManager::CreateAliveListSsbo()
    {
        void* InitialData = calloc(MaxParticleCount, sizeof(ParticleData));
        GlBufferUnique buffer = GlBufferUnique::Create();
        glNamedBufferStorage(buffer, MaxParticleCount * sizeof(ParticleData), InitialData, GL_DYNAMIC_STORAGE_BIT);
        free(InitialData);
        return buffer;
    }

    GlBufferUnique SprayManager::CreateSprayParametersUbo()
    {
        GlBufferUnique buffer = GlBufferUnique::Create();
        glNamedBufferStorage(buffer, MaxParticleCount * sizeof(SprayParameters), nullptr, GL_DYNAMIC_STORAGE_BIT);
        return buffer;
    }

    SprayManager::SprayManager()
        : _spawnShader(
              ShaderBuilder().SetStage("Assets/Shaders/Water/Spray/SpraySpawn.comp", ShaderStage::ComputeShader)
              .Build()),
          _updateShader(
              ShaderBuilder().SetStage("Assets/Shaders/Water/Spray/SprayUpdate.comp", ShaderStage::ComputeShader)
              .Build()),
          _renderShader(ShaderBuilder().SetStage("Assets/Shaders/Water/Spray/Spray.vert", ShaderStage::VertexShader)
              .SetStage("Assets/Shaders/Water/Spray/Spray.frag", ShaderStage::FragmentShader)
              .Build()),
          _particleDataSsbo(CreateParticleDataSsbo()),
          _freeListSsbo(CreateFreeListSsbo()),
          _indirectDrawBuffer(CreateIndirectDrawSsbo()),
          _aliveListSsbo(CreateAliveListSsbo()),
          _sprayParametersUbo(CreateSprayParametersUbo())
    {
    }

    SprayManager::~SprayManager() = default;

    void SprayManager::Update(const float DeltaTime,
                              const uint32_t DisplacementMaps,
                              const uint32_t SlopeMaps,
                              const uint32_t CascadeSettingsSsbo,
                              const glm::vec3& CameraPosition,
                              const glm::mat4& ViewMatrix,
                              const glm::mat4& ProjectionMatrix) const
    {
        const uint32_t ParticlesToSpawn = static_cast<uint32_t>(SpawnRate * DeltaTime);
        const float currentTime = Platform::TimeManager::GetTime();
        const SprayParameters sprayParameters
        {
            ViewMatrix,
            ProjectionMatrix,
            CameraPosition,
            MaxParticleCount,
            ParticlesToSpawn,
            *reinterpret_cast<const uint32_t*>(&currentTime),
            150.0f,
            10.0f,
            DeltaTime
        };

        glNamedBufferSubData(_sprayParametersUbo, 0, sizeof(SprayParameters), &sprayParameters);
        // --- 1. Reset & Clear ---
        static constexpr uint32_t zero = 0;
        glNamedBufferSubData(_indirectDrawBuffer,
                             offsetof(DrawElementsIndirectCommand, DrawElementsIndirectCommand::InstanceCount),
                             sizeof(uint32_t),
                             &zero);

        // --- 2. Spawn ---
        _spawnShader.Use();


        glBindBufferBase(GL_UNIFORM_BUFFER, 0, _sprayParametersUbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, CascadeSettingsSsbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _particleDataSsbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _freeListSsbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, _indirectDrawBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, _aliveListSsbo);

        glBindTextureUnit(3, DisplacementMaps);
        glBindTextureUnit(4, SlopeMaps);
        Shader::DispatchCompute((ParticlesToSpawn + 63) / 64, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);

        // --- 3. Update & Compact ---
        _updateShader.Use();

        Shader::DispatchCompute((MaxParticleCount + 63) / 64, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_COMMAND_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);
    }

    void SprayManager::Render() const
    {
        _renderShader.Use();
        SpriteQuad::BindVao();
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, _sprayParametersUbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _particleDataSsbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _freeListSsbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, _aliveListSsbo);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, _indirectDrawBuffer);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr);

        glDisable(GL_BLEND);
    }
}
