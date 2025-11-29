#pragma once
#include <algorithm>
#include <cstdint>
#include <functional>
#include <utility>
#include <vector>

namespace Platform
{
    class Input
    {
        friend class Window;

    public:
        enum class KeyboardButton : uint8_t
        {
            None,
            W,
            A,
            S,
            D
        };

        enum class ButtonEventType : uint8_t
        {
            None,
            KeyPressed,
            KeyReleased,
        };

        using CallbackId = size_t;
        using MovementCallback = std::function<void(float, float)>;
        using CameraRotationCallback = std::function<void(float, float)>;
        using CameraDragCallback = std::function<void(bool)>;
        using ShowGuiCallback = std::function<void(bool)>;

    private:
        static std::vector<std::pair<CallbackId, MovementCallback>> _movementCallbacks;
        static std::vector<std::pair<CallbackId, CameraRotationCallback>> _cameraRotationCallbacks;
        static std::vector<std::pair<CallbackId, CameraDragCallback>> _cameraDragCallbacks;
        static std::vector<std::pair<CallbackId, ShowGuiCallback>> _showGuiCallbacks;
        static CallbackId _nextCallbackId;

        static float _inputX;
        static float _inputY;
        static bool _isCameraDragging;
        static bool _isShowingGui;

        static bool _wPressed;
        static bool _aPressed;
        static bool _sPressed;
        static bool _dPressed;

    public:
        Input(const Input&) = delete;

        Input& operator=(const Input&) = delete;

    private:
        Input() = default;

    public:
        ~Input() = default;

        static CallbackId SubscribeToMovementEvent(const MovementCallback& Callback)
        {
            _movementCallbacks.emplace_back(++_nextCallbackId, Callback);
            return _nextCallbackId;
        }

        static void UnsubscribeToMovementEvent(CallbackId Id)
        {
            const auto iterator = std::ranges::remove_if(_movementCallbacks,
                                                         [Id](const auto& Pair)
                                                         {
                                                             return Pair.first == Id;
                                                         }).begin();
        }

        static CallbackId SubscribeToCameraRotationEvent(const CameraRotationCallback& Callback)
        {
            _cameraRotationCallbacks.emplace_back(++_nextCallbackId, Callback);
            return _nextCallbackId;
        }

        static void UnsubscribeToCameraRotationEvent(CallbackId Id)
        {
            const auto iterator = std::ranges::remove_if(_cameraRotationCallbacks,
                                                         [Id](const auto& Pair)
                                                         {
                                                             return Pair.first == Id;
                                                         }).begin();
        }

        static CallbackId SubscribeToCameraDragEvent(const CameraDragCallback& Callback)
        {
            _cameraDragCallbacks.emplace_back(++_nextCallbackId, Callback);
            return _nextCallbackId;
        }

        static void UnsubscribeToCameraDragEvent(CallbackId Id)
        {
            const auto it = std::ranges::remove_if(_cameraDragCallbacks,
                                                   [Id](const auto& Pair)
                                                   {
                                                       return Pair.first == Id;
                                                   }).begin();
        }

        static CallbackId SubscribeToShowGuiEvent(const ShowGuiCallback& Callback)
        {
            _showGuiCallbacks.emplace_back(++_nextCallbackId, Callback);
            return _nextCallbackId;
        }

        static void UnsubscribeToShowGuiEvent(CallbackId Id)
        {
            const auto it = std::ranges::remove_if(_showGuiCallbacks,
                                                   [Id](const auto& Pair)
                                                   {
                                                       return Pair.first == Id;
                                                   }).begin();
        }

        static void DispatchMovementEvent(KeyboardButton Button, ButtonEventType Type);

        static void DispatchCameraRotationEvent(float DeltaX, float DeltaY);

        static void DispatchCameraDragEvent(bool IsDragging);

        static void DispatchShowGuiEvent();

        static void StopMovement();
    };
}
