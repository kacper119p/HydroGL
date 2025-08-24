#pragma once
#include "Engine.h"
#include "Input.h"
#include "Scene/Scene.h"

namespace Application
{
    class CameraManager
    {
    private:
        float _rotationSensitivity = 2.5f;
        float _movementSensitivity = 2.0f;

        float _movementX = 0.0f;
        float _movementY = 0.0f;

        float _pitch = 0.0f;
        float _yaw = 0.0f;

        Platform::Input::CallbackId _movementCallbackId = 0;
        Platform::Input::CallbackId _rotationCallbackId = 0;
        Platform::Input::CallbackId _dragCallbackId = 0;

        Core::Scene* _scene = nullptr;

        bool _isDragging = false;

    public:
        explicit CameraManager(const Core::Engine* Engine);

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

    public:
        void Update(Core::Scene& Scene, float DeltaTime) const;

    private:
        void HandleMovement(float X, float Y);

        void HandleRotation(float DeltaX, float DeltaY);

        void HandleDrag(bool IsDragging);
    };
}
