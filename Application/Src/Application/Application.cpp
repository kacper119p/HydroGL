#include "Application.h"

#include <iostream>

#include "Launcher.h"

int ApplicationMain(Platform::Window* const Window)
{
    while (!Window->ShouldClose())
    {

    }
    std::cout << "Hello World!" << std::endl;
    Window->Close();
    return 0;
}
