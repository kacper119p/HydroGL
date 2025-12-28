#include "SprayManager.h"

#include "TimeManager.h"
#include "Rendering/RendererHelpers/DrawElementsIndirectCommand.h"
#include "Rendering/RendererHelpers/ScreenQuad.h"
#include "Rendering/RendererHelpers/SpriteQuad.h"
#include "Rendering/Shaders/ShaderBuilder.h"

namespace Core
{
    GlBufferUnique SprayManager::CreateParticleDataSsbo()
    {
        void* initialData = std::calloc(MaxParticleCount, sizeof(ParticleData));
        GlBufferUnique buffer = GlBufferUnique::Create();
        glNamedBufferStorage(buffer, MaxParticleCount * sizeof(ParticleData), initialData, GL_DYNAMIC_STORAGE_BIT);
        std::free(initialData);
        return buffer;
    }

    GlBufferUnique SprayManager::CreateFreeListSsbo()
    {
        uint32_t* const initialData
                = static_cast<uint32_t*>(std::malloc(sizeof(int32_t) + MaxParticleCount * sizeof(uint32_t)));
        int32_t* freeCount = reinterpret_cast<int32_t*>(initialData);
        *freeCount = MaxParticleCount;
        uint32_t* const indices = initialData + 1;
        for (uint32_t i = 0; i < MaxParticleCount; ++i)
        {
            indices[i] = i;
        }
        GlBufferUnique buffer = GlBufferUnique::Create();
        glNamedBufferStorage(buffer, sizeof(int32_t) + MaxParticleCount * sizeof(uint32_t), initialData,
                             GL_DYNAMIC_STORAGE_BIT);
        std::free(initialData);
        return buffer;
    }

    GlBufferUnique SprayManager::CreateIndirectDrawSsbo()
    {
        const DrawElementsIndirectCommand* const initialData = new DrawElementsIndirectCommand{6, 0, 0, 0, 0};
        GlBufferUnique buffer = GlBufferUnique::Create();
        glNamedBufferStorage(buffer, sizeof(DrawElementsIndirectCommand), initialData, GL_DYNAMIC_STORAGE_BIT);
        delete initialData;
        return buffer;
    }

    GlBufferUnique SprayManager::CreateAliveListSsbo()
    {
        void* initialData = std::calloc(MaxParticleCount, sizeof(ParticleData));
        GlBufferUnique buffer = GlBufferUnique::Create();
        glNamedBufferStorage(buffer, MaxParticleCount * sizeof(uint32_t), initialData, GL_DYNAMIC_STORAGE_BIT);
        std::free(initialData);
        return buffer;
    }

    GlBufferUnique SprayManager::CreateSprayParametersUbo()
    {
        GlBufferUnique buffer = GlBufferUnique::Create();
        glNamedBufferStorage(buffer, MaxParticleCount * sizeof(SprayParameters), nullptr, GL_DYNAMIC_STORAGE_BIT);
        return buffer;
    }


    SprayManager::SprayManager(const glm::ivec2& WindowSize, const uint32_t MainDepthBuffer)
        : _spawnShader(
              ShaderBuilder().SetStage("Assets/Shaders/Water/Spray/SpraySpawn.comp", ShaderStage::ComputeShader)
              .Build()),
          _updateShader(
              ShaderBuilder().SetStage("Assets/Shaders/Water/Spray/SprayUpdate.comp", ShaderStage::ComputeShader)
              .Build()),
          _renderShader(ShaderBuilder().SetStage("Assets/Shaders/Water/Spray/Spray.vert", ShaderStage::VertexShader)
              .SetStage("Assets/Shaders/Water/Spray/Spray.frag", ShaderStage::FragmentShader)
              .Build()),
          _blendShader(ShaderBuilder().SetStage("Assets/Shaders/Water/Spray/ParticleOpacityPass.vert",
                                                ShaderStage::VertexShader)
              .SetStage("Assets/Shaders/Water/Spray/ParticleOpacityPass.frag",
                        ShaderStage::FragmentShader)
              .Build()),
          _particleDataSsbo(CreateParticleDataSsbo()),
          _freeListSsbo(CreateFreeListSsbo()),
          _indirectDrawBuffer(CreateIndirectDrawSsbo()),
          _aliveListSsbo(CreateAliveListSsbo()),
          _sprayParametersUbo(CreateSprayParametersUbo()),
          _sprayTexture(Texture::GetTexture("Assets/Textures/Spray/Spray.dds"))
    {
        CreateParticleFbo(WindowSize, MainDepthBuffer);
    }

    SprayManager::~SprayManager() = default;

    void SprayManager::Update(const float DeltaTime,
                              const uint32_t DisplacementMaps,
                              const uint32_t SlopeMaps,
                              const uint32_t VelocityMaps,
                              const uint32_t CascadeSettingsSsbo,
                              const uint32_t FoamSettingsUbo,
                              const glm::vec2& WindVelocity,
                              const glm::vec3& SceneColor,
                              const glm::vec3& FoamColor,
                              const glm::vec3& CameraPosition,
                              const glm::mat4& ViewMatrix,
                              const glm::mat4& ProjectionMatrix) const
    {
        const uint32_t particlesToSpawn = static_cast<uint32_t>(_spawnRate * DeltaTime);
        PrepareNextFrame(ViewMatrix, ProjectionMatrix, CameraPosition, SceneColor, FoamColor, WindVelocity,
                         particlesToSpawn, DeltaTime);
        BindData(CascadeSettingsSsbo, FoamSettingsUbo, DisplacementMaps, SlopeMaps, VelocityMaps);

        _spawnShader.Use();
        Shader::DispatchCompute(static_cast<int32_t>(particlesToSpawn + 63) / 64, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);

        _updateShader.Use();
        Shader::DispatchCompute((MaxParticleCount + 63) / 64, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_COMMAND_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);
    }

    void SprayManager::Render(const uint32_t TargetFrameBuffer) const
    {
        _renderShader.Use();

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, _sprayParametersUbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _particleDataSsbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _freeListSsbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, _aliveListSsbo);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, _indirectDrawBuffer);
        _sprayTexture.BindTextureUnit(5);

        constexpr float zero[1] = {0.0f};
        glClearNamedFramebufferfv(_particleFbo, GL_COLOR, 0, zero);
        glBindFramebuffer(GL_FRAMEBUFFER, _particleFbo);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDepthMask(GL_FALSE);
        glEnable(GL_DEPTH_TEST);

        SpriteQuad::BindVao();
        glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr);

        glDisable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBindFramebuffer(GL_FRAMEBUFFER, TargetFrameBuffer);

        _blendShader.Use();
        glBindTextureUnit(0, _particleOpacityTexture);
        ScreenQuad::Draw();

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
    }

    void SprayManager::PrepareNextFrame(const glm::mat4& ViewMatrix, const glm::mat4& ProjectionMatrix,
                                        const glm::vec3& CameraPosition, const glm::vec3& SceneColor,
                                        const glm::vec3& FoamColor, const glm::vec2& WindVelocity,
                                        const uint32_t ParticlesToSpawn, const float DeltaTime) const
    {
        const float currentTime = Platform::TimeManager::GetTime();
        const SprayParameters sprayParameters
        {
            ViewMatrix,
            ProjectionMatrix,
            CameraPosition,
            MaxParticleCount,
            SceneColor,
            ParticlesToSpawn,
            WindVelocity,
            *reinterpret_cast<const uint32_t*>(&currentTime),
            _spawnRange,
            FoamColor,
            _lifetime,
            DeltaTime,
            currentTime,
            _sprayOpacity,
            _spraySize,
            _spawnThreshold,
            _waterVelocityInfluence,
            _windInfluence
        };

        glNamedBufferSubData(_sprayParametersUbo, 0, sizeof(SprayParameters), &sprayParameters);

        static constexpr uint32_t Zero = 0;
        glNamedBufferSubData(_indirectDrawBuffer,
                             offsetof(DrawElementsIndirectCommand, DrawElementsIndirectCommand::InstanceCount),
                             sizeof(uint32_t),
                             &Zero);
    }

    void SprayManager::BindData(const uint32_t CascadeSettingsSsbo, const uint32_t FoamSettingsUbo,
                                const uint32_t DisplacementMaps, const uint32_t SlopeMaps,
                                const uint32_t VelocityMaps) const
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, _sprayParametersUbo);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, FoamSettingsUbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, CascadeSettingsSsbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _particleDataSsbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _freeListSsbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, _indirectDrawBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, _aliveListSsbo);

        glBindTextureUnit(3, DisplacementMaps);
        glBindTextureUnit(4, SlopeMaps);
        glBindTextureUnit(6, VelocityMaps);
    }

    void SprayManager::CreateParticleFbo(const glm::ivec2& WindowSize, const uint32_t MainDepthTexture)
    {
        _particleFbo = GlFramebufferUnique::Create();
        _particleOpacityTexture = GlTextureUnique::Create(GL_TEXTURE_2D);

        glTextureStorage2D(_particleOpacityTexture, 1, GL_R8, WindowSize.x, WindowSize.y);
        glTextureParameteri(_particleOpacityTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(_particleOpacityTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(_particleOpacityTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(_particleOpacityTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glNamedFramebufferTexture(_particleFbo, GL_COLOR_ATTACHMENT0, _particleOpacityTexture, 0);
        glNamedFramebufferTexture(_particleFbo, GL_DEPTH_STENCIL_ATTACHMENT, MainDepthTexture, 0);

        constexpr GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0};
        glNamedFramebufferDrawBuffers(_particleFbo, 1, drawBuffers);
    }
}
