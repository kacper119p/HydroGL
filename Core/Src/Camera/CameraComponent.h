#pragma once
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_clip_space.hpp>

namespace Core
{
    struct CameraComponent
    {
    private:
        glm::mat4 _projectionMatrix;
        float _nearPlane;
        float _farPlane;

    public:
        [[nodiscard]] const glm::mat4& GetProjectionMatrix() const
        {
            return _projectionMatrix;
        }

        [[nodiscard]] float GetNearPlane() const
        {
            return _nearPlane;
        }

        [[nodiscard]] float GetFarPlane() const
        {
            return _farPlane;
        }

        /**
         * @brief Creates a CameraComponent with a perspective projection matrix.
         * @param Fov Field of view in degrees.
         * @param AspectRatio Aspect ratio of the viewport.
         * @param NearPlane Near clipping plane distance.
         * @param FarPlane Far clipping plane distance.
         */
        CameraComponent(const float Fov, const float AspectRatio, const float NearPlane, const float FarPlane)
            : _projectionMatrix(glm::perspective(glm::radians(Fov), AspectRatio, NearPlane, FarPlane)),
              _nearPlane(NearPlane),
              _farPlane(FarPlane)
        {
        }

        // CameraComponent(const float Width, const float Height, const float NearPlane, const float FarPlane)
        //     : _projectionMatrix(glm::ortho(-Width * 0.5f, Width * 0.5f,
        //                                    -Height * 0.5f, Height * 0.5f, NearPlane, FarPlane)),
        //       _nearPlane(NearPlane),
        //       _farPlane(FarPlane)
        // {
        // }
    };
}
