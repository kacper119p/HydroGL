#pragma once
#include "Engine.h"
#include "Input.h"
#include "Scene/Scene.h"

namespace Application
{
    class CameraManager
    {
    public:
        static constexpr float DefaultRotationSensitivity = 2.5f;
        static constexpr float DefaultMovementSensitivity = 2.0f;

    private:
        float _rotationSensitivity = DefaultRotationSensitivity;
        float _movementSensitivity = DefaultMovementSensitivity;

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
        [[nodiscard]] Core::TransformComponent* GetCameraParent() const
        {
            const auto cameraView = _scene->GetEntityRegistry()->view<Core::CameraComponent,
                Core::TransformComponent>();
            const entt::entity cameraEntity = cameraView.front();
            const Core::TransformComponent& transform = _scene->GetEntityRegistry()
                    ->get<Core::TransformComponent>(cameraEntity);
            return transform.GetSceneHierarchyNode().GetParent()->GetTransformComponent();
        }

        void SetCameraParent(const Core::TransformComponent& CameraOwner) const
        {
            const auto cameraView = _scene->GetEntityRegistry()->view<Core::CameraComponent,
                Core::TransformComponent>();
            const entt::entity cameraEntity = cameraView.front();
            const Core::TransformComponent& transform = _scene->GetEntityRegistry()->get<Core::TransformComponent>(
                cameraEntity);
            transform.GetSceneHierarchyNode().SetParent(CameraOwner.GetSceneHierarchyNode());
        }

    public:
        explicit CameraManager(const Core::Engine* Engine);

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
