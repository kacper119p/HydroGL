#pragma once

#include <memory>

#include "Window.h"
#include "Camera/CameraManager.h"
#include "Gui/Gui.h"

int32_t ApplicationMain(Platform::Window* Window);

namespace Application
{
    class Application
    {
    private:
        std::unique_ptr<CameraManager> _cameraManager = nullptr;
        std::unique_ptr<Gui> _gui;

    public:
        int32_t Run(Platform::Window* Window);
    };
}

