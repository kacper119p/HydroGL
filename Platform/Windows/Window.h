#pragma once

#include <windows.h>

namespace Platform
{
    class Window
    {
        friend class Launcher;

    private:
        HWND _windowHandle = nullptr;
        HDC _deviceContext = nullptr;
        HGLRC _openGlContext = nullptr;

    private:
        Window(HWND WindowHandle, HDC DeviceContext, HGLRC OpenGlContext);

    public:
        ~Window();

    private:
        static Window* Create(HINSTANCE HInstance);

    public:
        bool ShouldClose();

        void Close();
    };
}
