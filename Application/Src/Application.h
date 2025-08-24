#pragma once

#include <memory>

#include "Window.h"
#include "Camera/CameraManager.h"

int32_t ApplicationMain(Platform::Window* Window);

namespace Application
{
    class Application
    {
    private:
        std::unique_ptr<CameraManager> _cameraManager = nullptr;

    public:
        int32_t Run(Platform::Window* Window);
    };
}

