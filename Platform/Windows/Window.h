#pragma once

#include <cstdint>
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
        uint32_t _windowWidth = 0;
        uint32_t _windowHeight = 0;
        bool _shouldClose = false;

    private:
        Window(HWND WindowHandle, HDC DeviceContext, HGLRC OpenGlContext, uint32_t WindowWidth,
               uint32_t WindowHeight);

    public:
        ~Window();

    private:
        static Window* Create(HINSTANCE HInstance);

    public:
        [[nodiscard]] uint32_t GetWindowWidth() const
        {
            return _windowWidth;
        }

        [[nodiscard]] uint32_t GetWindowHeight() const
        {
            return _windowHeight;
        }

    public:
        void PoolEvents();

        [[nodiscard]] bool ShouldClose() const;

        void MakeContextCurrent() const;

        void SwapBuffers() const;

        void Close();
    };
}
