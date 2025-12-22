#include "DirectionalShadowsRenderer.h"

#include <glm/ext/matrix_transform.hpp>

#include "Rendering/Shaders/ShaderBuilder.h"

namespace
{
    constexpr float LightProjectionZMargin = 5.0f;
}

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

    void DirectionalShadowsRenderer::RenderShadows(const CameraComponent& Camera, const glm::mat4& ViewMatrix,
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
    }

    void DirectionalShadowsRenderer::UpdateShadowCascadeMatrices(const CameraComponent& Camera,
                                                                 const glm::mat4& ViewMatrix,
                                                                 const glm::vec3& LightDirection)
    {
        for (size_t i = 0; i < CascadeCount; ++i)
        {
            // Get Frustum Corners for the current cascade slice
            const float nearCameraPlane = (i == 0)
                                              ? Camera.GetNearPlane()
                                              : _directionalLightSamplingData.CascadeSplits[i - 1];
            const float farCameraPlane = (i == CascadeCount - 1)
                                             ? Camera.GetFarPlane()
                                             : _directionalLightSamplingData.CascadeSplits[i];
            const glm::mat4& projectionMatrix = Camera.GetProjectionMatrix();
            std::array<glm::vec3, 8> corners{};
            GetFrustumCornersWorldSpace(corners, projectionMatrix, ViewMatrix, nearCameraPlane, farCameraPlane);

            // Calculate Light View Matrix centered on frustum centroid
            glm::vec3 center = glm::vec3(0.0f);
            for (const auto& v : corners)
            {
                center += v;
            }
            center /= 8.0f;

            glm::mat4 lightView = glm::lookAt(center + glm::normalize(LightDirection), center,
                                              glm::vec3(0.0f, 1.0f, 0.0f));

            // Calculate AABB in Light Space to tightly fit the frustum slice
            glm::vec3 firstLightSpaceCorner = glm::vec3(lightView * glm::vec4(corners[0], 1.0f));
            glm::vec3 minCorner = firstLightSpaceCorner;
            glm::vec3 maxCorner = firstLightSpaceCorner;

            for (uint32_t j = 1; j < 8; ++j)
            {
                glm::vec3 lightSpaceCorner = glm::vec3(lightView * glm::vec4(corners[j], 1.0f));
                minCorner = glm::min(minCorner, glm::vec3(lightSpaceCorner));
                maxCorner = glm::max(maxCorner, glm::vec3(lightSpaceCorner));
            }

            // Extend Z-Range (Near/Far planes) to capture occluders outside the frustum
            minCorner.z = minCorner.z < 0.0f
                              ? minCorner.z * LightProjectionZMargin
                              : minCorner.z / LightProjectionZMargin;
            maxCorner.z = maxCorner.z < 0.0f
                              ? maxCorner.z / LightProjectionZMargin
                              : maxCorner.z * LightProjectionZMargin;

            // Texel Snapping
            float shadowMapWidth = static_cast<float>(ShadowMapResolution);

            const glm::vec2 texelSize{
                (maxCorner.x - minCorner.x) / shadowMapWidth,
                (maxCorner.y - minCorner.y) / shadowMapWidth
            };

            // Snap the min bounds to the nearest texel multiple
            minCorner.x = glm::floor(minCorner.x / texelSize.x) * texelSize.x;
            maxCorner.x = glm::floor(maxCorner.x / texelSize.x) * texelSize.x;
            minCorner.y = glm::floor(minCorner.y / texelSize.y) * texelSize.y;
            maxCorner.y = glm::floor(maxCorner.y / texelSize.y) * texelSize.y;

            // Final Light Projection Matrix
            glm::mat4 lightOrtho = glm::ortho(minCorner.x, maxCorner.x, minCorner.y, maxCorner.y, minCorner.z,
                                              maxCorner.z);
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
        const glm::vec3 cameraUp = glm::normalize(glm::vec3(invView[1]));

        // Assumes glm::perspective
        const float tanHalfVFov = 1.0f / ProjectionMatrix[1][1];
        const float aspectRatio = ProjectionMatrix[1][1] / ProjectionMatrix[0][0];
        const float tanHalfHFov = tanHalfVFov * aspectRatio;

        const glm::vec2 nearPlaneSize{NearCameraPlane * tanHalfHFov, NearCameraPlane * tanHalfVFov};
        const glm::vec2 farPlaneSize{FarCameraPlane * tanHalfHFov, FarCameraPlane * tanHalfVFov};

        const glm::vec3 nearCenter = cameraPosition + cameraForward * NearCameraPlane;
        const glm::vec3 farCenter = cameraPosition + cameraForward * FarCameraPlane;

        // Near Plane Corners
        Result[0] = nearCenter + cameraUp * nearPlaneSize.y + cameraRight * nearPlaneSize.x; // Top Right
        Result[1] = nearCenter + cameraUp * nearPlaneSize.y - cameraRight * nearPlaneSize.x; // Top Left
        Result[2] = nearCenter - cameraUp * nearPlaneSize.y - cameraRight * nearPlaneSize.x; // Bottom Left
        Result[3] = nearCenter - cameraUp * nearPlaneSize.y + cameraRight * nearPlaneSize.x; // Bottom Right

        // Far Plane Corners
        Result[4] = farCenter + cameraUp * farPlaneSize.y + cameraRight * farPlaneSize.x; // Top Right
        Result[5] = farCenter + cameraUp * farPlaneSize.y - cameraRight * farPlaneSize.x; // Top Left
        Result[6] = farCenter - cameraUp * farPlaneSize.y - cameraRight * farPlaneSize.x; // Bottom Left
        Result[7] = farCenter - cameraUp * farPlaneSize.y + cameraRight * farPlaneSize.x; // Bottom Right
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
