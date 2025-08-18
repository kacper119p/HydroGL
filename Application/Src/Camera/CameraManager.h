#pragma once
#include "Input.h"
#include "Scene/Scene.h"

namespace Application
{
    class CameraManager
    {
    private:
        float _rotationSensitivity = 1.0f;
        float _movementSensitivity = 1.0f;

        float _movementX = 0.0f;
        float _movementY = 0.0f;
        bool _isDragging = false;

        float _pitch = 0.0f;
        float _yaw = 0.0f;

        Platform::Input::CallbackId _movementCallbackId = 0;
        Platform::Input::CallbackId _rotationCallbackId = 0;
        Platform::Input::CallbackId _dragCallbackId = 0;

    public:
        CameraManager();

    public:
        ~CameraManager();

    public:
        [[nodiscard]] float GetRotationSensitivity() const
        {
            return _rotationSensitivity;
        }

        void SetRotationSensitivity(const float RotationSensitivity)
        {
            _rotationSensitivity = RotationSensitivity;
        }

        [[nodiscard]] float GetMovementSensitivity() const
        {
            return _movementSensitivity;
        }

        void SetMovementSensitivity(const float MovementSensitivity)
        {
            _movementSensitivity = MovementSensitivity;
        }

    private:
        void HandleMovement(float X, float Y);

        void HandleRotation(float DeltaX, float DeltaY);

        void HandleDrag(bool IsDragging);

    private:
        void Update(Core::Scene& Scene, float DeltaTime) const;
    };
}
