#include "Application.h"

#include <iostream>

#include "Engine.h"

int ApplicationMain(Platform::Window* const Window)
{
    Core::Engine::Start(Window);
    while (!Window->ShouldClose())
    {

    }
    Core::Engine::Shutdown();
    return 0;
}
