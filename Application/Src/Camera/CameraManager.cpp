#include "CameraManager.h"

#include <glm/gtc/matrix_transform.hpp>
#include "Debug.h"
#include "Engine.h"
#include "Scene/TransformComponent.h"

#include "Input.h"
#include "Camera/CameraComponent.h"

namespace Application
{
    CameraManager::CameraManager(const Core::Engine* Engine) : _movementCallbackId(
                                                                   Platform::Input::SubscribeToMovementEvent(
                                                                       [this](const float X, const float Y)
                                                                       {
                                                                           HandleMovement(X, Y);
                                                                       })),
                                                               _rotationCallbackId(
                                                                   Platform::Input::SubscribeToCameraRotationEvent(
                                                                       [this](const float DeltaX, const float DeltaY)
                                                                       {
                                                                           HandleRotation(DeltaX, DeltaY);
                                                                       })),
                                                               _dragCallbackId(
                                                                   Platform::Input::SubscribeToCameraDragEvent(
                                                                       [this](const bool IsDragging)
                                                                       {
                                                                           HandleDrag(IsDragging);
                                                                       })),
                                                               _scene(Engine->GetScene())
    {
    }

    CameraManager::~CameraManager()
    {
        Platform::Input::UnsubscribeToMovementEvent(_movementCallbackId);
        Platform::Input::UnsubscribeToCameraRotationEvent(_rotationCallbackId);
        Platform::Input::UnsubscribeToCameraDragEvent(_dragCallbackId);
    }

    void CameraManager::HandleMovement(const float X, const float Y)
    {
        _movementX = X;
        _movementY = Y;
    }

    void CameraManager::HandleRotation(const float DeltaX, const float DeltaY)
    {
        entt::registry* entityRegistry = _scene->GetEntityRegistry();

        const auto cameraView = entityRegistry->view<Core::CameraComponent, Core::TransformComponent>();
        const entt::entity cameraEntity = cameraView.front();

        CHECK(entityRegistry->valid(cameraEntity), "Camera entity doesn't exist!");

        Core::TransformComponent& cameraTransform = entityRegistry->get<Core::TransformComponent>(cameraEntity);

        _pitch -= DeltaY * _rotationSensitivity;
        _yaw -= DeltaX * _rotationSensitivity;

        if (_yaw > glm::two_pi<float>())
        {
            _yaw -= glm::two_pi<float>();
        } else if (_yaw < -glm::two_pi<float>())
        {
            _yaw += glm::two_pi<float>();
        }

        _pitch = glm::clamp(_pitch,
                            -glm::half_pi<float>() + glm::half_pi<float>() * 0.05f,
                            glm::half_pi<float>() - glm::half_pi<float>() * 0.05f);

        const glm::quat yawQuat = glm::angleAxis(_yaw, glm::vec3(0.0f, 1.0f, 0.0f));
        const glm::vec3 rightAxis = yawQuat * glm::vec3(1.0f, 0.0f, 0.0f);
        const glm::quat pitchQuat = glm::angleAxis(_pitch, rightAxis);
        const glm::quat rotation = pitchQuat * yawQuat;

        cameraTransform.SetLocalRotation(rotation);
    }

    void CameraManager::HandleDrag(const bool IsDragging)
    {
        _isDragging = IsDragging;
    }

    void CameraManager::Update(Core::Scene& Scene, const float DeltaTime) const
    {
        entt::registry* entityRegistry = Scene.GetEntityRegistry();

        const auto cameraView = entityRegistry->view<Core::CameraComponent, Core::TransformComponent>();
        const entt::entity cameraEntity = cameraView.front();

        CHECK(entityRegistry->valid(cameraEntity), "Camera entity doesn't exist!");

        Core::TransformComponent& cameraTransform = entityRegistry->get<Core::TransformComponent>(cameraEntity);
        if (_movementX == 0.0f && _movementY == 0.0f)
        {
            return;
        }
        const glm::mat4& worldMatrix = cameraTransform.GetLocalToParentMatrix();
        const glm::vec3 forward = -glm::normalize(glm::vec3(worldMatrix[2]));
        const glm::vec3 right = glm::normalize(glm::vec3(worldMatrix[0]));
        const glm::vec3 movement = _movementSensitivity * DeltaTime * (forward * _movementY + right * _movementX);
        const glm::vec3 newPosition = cameraTransform.GetLocalPosition() + movement;
        cameraTransform.SetLocalPosition(newPosition);
    }
}
