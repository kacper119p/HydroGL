#pragma once

#include <memory>

#include "Window.h"
#include "Camera/CameraManager.h"
#include "Gui/Gui.h"

namespace Application
{
    class Application
    {
    public:
        static constexpr float PhysicsTimeStep = 1.0f / 60.0f;

    private:
        std::unique_ptr<CameraManager> _cameraManager = nullptr;
        std::unique_ptr<Gui> _gui;

    public:
        int32_t Run(Platform::Window* Window);
    };
}

