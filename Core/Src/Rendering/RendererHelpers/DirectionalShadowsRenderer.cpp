#include "DirectionalShadowsRenderer.h"

#include <glm/ext/matrix_transform.hpp>

#include "Rendering/Shaders/ShaderBuilder.h"

namespace Core
{
    DirectionalShadowsRenderer::DirectionalShadowsRenderer()
        : _directionalLightSamplingData({}, {}, {0.067f * 200.0f, 0.133f * 200.0f, 0.267f * 200.0f, 200.0f}),
          _shadowPassUbo(CreateShadowPassUbo()),
          _shadowPassShader(ShaderBuilder()
              .SetStage("Assets/Shaders/Shadows/Directional/DirectionalShadow.vert", ShaderStage::VertexShader)
              .SetStage("Assets/Shaders/Shadows/Directional/DirectionalShadow.geom", ShaderStage::GeometryShader)
              .SetStage("Assets/Shaders/Shadows/Directional/DirectionalShadow.frag", ShaderStage::FragmentShader)
              .Build())
    {
        InitializeShadowMapsFramebuffer();
    }

    uint32_t DirectionalShadowsRenderer::RenderShadows(const CameraComponent& Camera, const glm::mat4& ViewMatrix,
                                                       const glm::vec3& LightDirection, const uint32_t TransformsBuffer,
                                                       const uint32_t IndirectDrawCommandsBuffer,
                                                       const uint32_t ObjectCount)
    {
        UpdateShadowCascadeMatrices(Camera, ViewMatrix, LightDirection);
        glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
        glViewport(0, 0, static_cast<int32_t>(ShadowMapResolution), static_cast<int32_t>(ShadowMapResolution));
        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glClear(GL_DEPTH_BUFFER_BIT);

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, _shadowPassUbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, TransformsBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, IndirectDrawCommandsBuffer);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, IndirectDrawCommandsBuffer);

        _shadowPassShader.Use();
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(ObjectCount), 0);

        return _shadowMaps;
    }

    void DirectionalShadowsRenderer::UpdateShadowCascadeMatrices(const CameraComponent& Camera,
                                                                 const glm::mat4& ViewMatrix,
                                                                 const glm::vec3& LightDirection)
    {
        for (size_t i = 0; i < CascadeCount; ++i)
        {
            float nearCameraPlane = (i == 0)
                                        ? Camera.GetNearPlane()
                                        : _directionalLightSamplingData.CascadeSplits[i - 1];
            float farCameraPlane = (i == CascadeCount - 1)
                                       ? Camera.GetFarPlane()
                                       : _directionalLightSamplingData.CascadeSplits[i];
            glm::mat4 cascadeProj = Camera.GetProjectionMatrix();
            std::array<glm::vec3, 8> corners{};
            GetFrustumCornersWorldSpace(corners, cascadeProj, ViewMatrix, nearCameraPlane, farCameraPlane);

            // Calculate Bounding Sphere
            glm::vec3 center = glm::vec3(0.0f);
            for (const auto& v : corners) center += v;
            center /= 8.0f;

            float radius = 0.0f;
            for (const auto& v : corners)
            {
                radius = std::max(radius, glm::distance(center, v));
            }
            radius = std::ceil(radius * 16.0f) / 16.0f;

            glm::vec3 maxExtents = glm::vec3(radius);
            glm::vec3 minExtents = -maxExtents;

            // Calculate Light View Matrix
            glm::vec3 lightPos = center + (glm::normalize(LightDirection) * radius);
            glm::mat4 lightView = glm::lookAt(lightPos, center, glm::vec3(0.0f, 1.0f, 0.0f));

            // Texel Snapping
            // Calculate the shadow matrix to find the offset
            glm::mat4 lightOrtho = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, -radius * 10.0f,
                                              radius * 10.0f);
            glm::mat4 shadowMatrix = lightOrtho * lightView;

            // Project origin (0,0,0) to shadow space
            glm::vec4 shadowOrigin = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            shadowOrigin = shadowMatrix * shadowOrigin;
            shadowOrigin *= (static_cast<float>(ShadowMapResolution) / 2.0f);

            // Find the rounding offset to snap to the nearest texel
            glm::vec4 roundedOrigin = glm::round(shadowOrigin);
            glm::vec4 roundOffset = roundedOrigin - shadowOrigin;
            roundOffset = roundOffset * (2.0f / static_cast<float>(ShadowMapResolution));
            roundOffset.z = 0.0f;
            roundOffset.w = 0.0f;

            // Apply snapping offset to the projection matrix
            lightOrtho[3] += roundOffset;

            _directionalLightSamplingData.CascadeViewProjMatrices[i] = lightOrtho * lightView;
        }
        _directionalLightSamplingData.ViewMatrix = ViewMatrix;
        glNamedBufferSubData(_shadowPassUbo, 0, sizeof(DirectionalLightSamplingData), &_directionalLightSamplingData);
    }

    void DirectionalShadowsRenderer::GetFrustumCornersWorldSpace(std::array<glm::vec3, 8>& Result,
                                                                 const glm::mat4& ProjectionMatrix,
                                                                 const glm::mat4& ViewMatrix,
                                                                 float NearCameraPlane, float FarCameraPlane)
    {
        const glm::mat4 invView = glm::inverse(ViewMatrix);
        const glm::vec3 cameraPosition = glm::vec3(invView[3]);
        const glm::vec3 cameraForward = glm::normalize(-glm::vec3(invView[2]));
        const glm::vec3 cameraRight = glm::normalize(glm::vec3(invView[0]));
        const glm::vec3 cameraLeft = glm::normalize(glm::vec3(invView[1]));

        // Assumes glm::perspective
        const float tanHalfVFov = 1.0f / ProjectionMatrix[1][1];
        const float aspectRatio = ProjectionMatrix[1][1] / ProjectionMatrix[0][0];
        const float tanHalfHFov = tanHalfVFov * aspectRatio;

        const glm::vec2 nearPlaneSize{NearCameraPlane * tanHalfHFov, NearCameraPlane * tanHalfVFov};
        const glm::vec2 farPlaneSize{FarCameraPlane * tanHalfHFov, FarCameraPlane * tanHalfVFov};

        const glm::vec3 nearCenter = cameraPosition + cameraForward * NearCameraPlane;
        const glm::vec3 farCenter = cameraPosition + cameraForward * FarCameraPlane;

        // Near Plane Corners
        Result[0] = nearCenter + cameraLeft * nearPlaneSize.y + cameraRight * nearPlaneSize.x; // Top Right
        Result[1] = nearCenter + cameraLeft * nearPlaneSize.y - cameraRight * nearPlaneSize.x; // Top Left
        Result[2] = nearCenter - cameraLeft * nearPlaneSize.y - cameraRight * nearPlaneSize.x; // Bottom Left
        Result[3] = nearCenter - cameraLeft * nearPlaneSize.y + cameraRight * nearPlaneSize.x; // Bottom Right

        // Far Plane Corners
        Result[4] = farCenter + cameraLeft * farPlaneSize.y + cameraRight * farPlaneSize.x; // Top Right
        Result[5] = farCenter + cameraLeft * farPlaneSize.y - cameraRight * farPlaneSize.x; // Top Left
        Result[6] = farCenter - cameraLeft * farPlaneSize.y - cameraRight * farPlaneSize.x; // Bottom Left
        Result[7] = farCenter - cameraLeft * farPlaneSize.y + cameraRight * farPlaneSize.x; // Bottom Right
    }

    GlBufferUnique DirectionalShadowsRenderer::CreateShadowPassUbo()
    {
        GlBufferUnique buffer = GlBufferUnique::Create();
        glNamedBufferStorage(buffer, sizeof(DirectionalLightSamplingData), nullptr, GL_DYNAMIC_STORAGE_BIT);
        return buffer;
    }

    void DirectionalShadowsRenderer::InitializeShadowMapsFramebuffer()
    {
        _framebuffer = GlFramebufferUnique::Create();
        _shadowMaps = GlTextureUnique::Create(GL_TEXTURE_2D_ARRAY);

        glTextureStorage3D(_shadowMaps, 1, GL_DEPTH_COMPONENT32F,
                           ShadowMapResolution, ShadowMapResolution, CascadeCount);
        glTextureParameteri(_shadowMaps, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(_shadowMaps, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(_shadowMaps, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTextureParameteri(_shadowMaps, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        glTextureParameteri(_shadowMaps, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTextureParameteri(_shadowMaps, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        constexpr float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
        glTextureParameterfv(_shadowMaps, GL_TEXTURE_BORDER_COLOR, borderColor);

        glNamedFramebufferTexture(_framebuffer, GL_DEPTH_ATTACHMENT, _shadowMaps, 0);
        glNamedFramebufferDrawBuffer(_framebuffer, GL_NONE);
        glNamedFramebufferReadBuffer(_framebuffer, GL_NONE);
    }
}
