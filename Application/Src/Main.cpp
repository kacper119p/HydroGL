#include <cstdint>
#include <memory>

#include "Application.h"
#include "Window.h"

int32_t ApplicationMain(Platform::Window* Window)
{
    const std::unique_ptr<Application::Application> application = std::make_unique<Application::Application>();
    application->Run(Window);
    return 0;
}
