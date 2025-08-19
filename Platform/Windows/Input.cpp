#include "Input.h"

#include <ranges>

namespace Platform
{
    Input::CallbackId Input::_nextCallbackId = 0;

    std::vector<std::pair<Input::CallbackId, Input::MovementCallback>> Input::_movementCallbacks;
    std::vector<std::pair<Input::CallbackId, Input::CameraRotationCallback>> Input::_cameraRotationCallbacks;
    std::vector<std::pair<Input::CallbackId, Input::CameraDragCallback>> Input::_cameraDragCallbacks;

    float Input::_inputX = 0.0f;
    float Input::_inputY = 0.0f;
    bool Input::_isCameraDragging = false;


    void Input::DispatchMovementEvent(const KeyboardButton Button, const ButtonEventType Type)
    {
        switch (Type)
        {
            case ButtonEventType::KeyPressed:
                switch (Button)
                {
                    case KeyboardButton::W:
                        _inputY += 1.0f;
                        break;
                    case KeyboardButton::A:
                        _inputX -= 1.0f;
                        break;
                    case KeyboardButton::S:
                        _inputY -= 1.0f;
                        break;
                    case KeyboardButton::D:
                        _inputX += 1.0f;
                        break;
                    default:
                        break;
                }
                break;

            case ButtonEventType::KeyReleased:
                switch (Button)
                {
                    case KeyboardButton::W:
                        _inputY -= 1.0f;
                        break;
                    case KeyboardButton::A:
                        _inputX += 1.0f;
                        break;
                    case KeyboardButton::S:
                        _inputY += 1.0f;
                        break;
                    case KeyboardButton::D:
                        _inputX -= 1.0f;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }

        for (const auto& callback : _movementCallbacks | std::views::values)
        {
            callback(_inputX, _inputY);
        }
    }

    void Input::DispatchCameraRotationEvent(const float DeltaX, const float DeltaY)
    {
        for (const auto& callback : _cameraRotationCallbacks | std::views::values)
        {
            callback(DeltaX, DeltaY);
        }
    }

    void Input::DispatchCameraDragEvent(const bool IsDragging)
    {
        for (const auto& callback : _cameraDragCallbacks | std::views::values)
        {
            callback(IsDragging);
        }
        _isCameraDragging = IsDragging;
    }
}
