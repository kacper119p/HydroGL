#include "Window.h"

#include <windows.h>
#include <glad/glad.h>

#include "Debug.h"
#include "GL/wglext.h"

namespace Platform
{
    static LRESULT CALLBACK WndProc(HWND Hwnd, const UINT Msg, const WPARAM WParam, const LPARAM LParam)
    {
        switch (Msg)
        {
            case WM_CLOSE:
                PostQuitMessage(0);
                return 0;
            case WM_SETCURSOR:
                if (LOWORD(LParam) == HTCLIENT)
                {
                    SetCursor(LoadCursor(nullptr, IDC_ARROW));
                }
                break;
            default:
                break;
        }
        return DefWindowProc(Hwnd, Msg, WParam, LParam);
    }

    Window::Window(HWND WindowHandle, HDC DeviceContext, HGLRC OpenGlContext):
        WindowHandle(WindowHandle),
        DeviceContext(DeviceContext),
        OpenGlContext(OpenGlContext)
    {
    }

    Window::~Window()
    {
        Close();
    }

    Window* Window::Create(HINSTANCE HInstance)
    {
        if (HInstance == nullptr)
        {
            HInstance = GetModuleHandle(nullptr);
        }
        constexpr char className[] = "WindowClass";
        WNDCLASS windowClass = {};
        windowClass.lpfnWndProc = WndProc;
        windowClass.hInstance = HInstance;
        windowClass.lpszClassName = className;
        RegisterClass(&windowClass);

        MONITORINFO monitorInfo = {sizeof(monitorInfo)};
        HMONITOR primaryMonitor = MonitorFromWindow(nullptr, MONITOR_DEFAULTTOPRIMARY);
        GetMonitorInfo(primaryMonitor, &monitorInfo);
        HWND hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_NOREDIRECTIONBITMAP,
                                   className,
                                   "HydroGL",
                                   WS_POPUP,
                                   monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
                                   monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                                   monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                                   nullptr, nullptr, HInstance, nullptr);
        if (!hwnd)
        {
            return nullptr;
        }

        ShowWindow(hwnd, SW_SHOW);
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

        HDC deviceContext = GetDC(hwnd);
        if (!deviceContext)
        {
            DestroyWindow(hwnd);
            return nullptr;
        }

        DEVMODE deviceMode = {};
        deviceMode.dmSize = sizeof(deviceMode);
        PIXELFORMATDESCRIPTOR pfd = {sizeof(PIXELFORMATDESCRIPTOR), 1};
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = deviceMode.dmBitsPerPel;
        pfd.cDepthBits = 24;
        pfd.iLayerType = PFD_MAIN_PLANE;

        const int pixelFormat = ChoosePixelFormat(deviceContext, &pfd);
        if (!SetPixelFormat(deviceContext, pixelFormat, &pfd))
        {
            ReleaseDC(hwnd, deviceContext);
            DestroyWindow(hwnd);
            return nullptr;
        }

        HGLRC tempContext = wglCreateContext(deviceContext);
        if (!tempContext)
        {
            ReleaseDC(hwnd, deviceContext);
            DestroyWindow(hwnd);
            return nullptr;
        }

        wglMakeCurrent(deviceContext, tempContext);

        if (!gladLoadGL())
        {
            MessageBox(nullptr, "Failed to initialize GLAD", "Error", MB_OK);
            wglMakeCurrent(nullptr, nullptr);
            wglDeleteContext(tempContext);
            ReleaseDC(hwnd, deviceContext);
            DestroyWindow(hwnd);
            return nullptr;
        }

        int contextAttribs[] = {
                WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
                WGL_CONTEXT_MINOR_VERSION_ARB, 6,
#if DEBUG
                WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
#else
                WGL_CONTEXT_FLAGS_ARB, 0,
#endif
                WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                0
        };

        PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB =
                reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));

        HGLRC fullGlContext = wglCreateContextAttribsARB(deviceContext, nullptr, contextAttribs);
        if (!fullGlContext)
        {
            MessageBox(nullptr, "Missing wglCreateContextAttribsARB", "Error", MB_OK);
            wglMakeCurrent(nullptr, nullptr);
            wglDeleteContext(tempContext);
            ReleaseDC(hwnd, deviceContext);
            DestroyWindow(hwnd);
            return nullptr;
        }

        wglDeleteContext(tempContext);

        wglMakeCurrent(deviceContext, fullGlContext);

#if DEBUG
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        Debug::EnableOpenGlDebugging();
#endif

        return new Window(hwnd, deviceContext, fullGlContext);
    }

    bool Window::ShouldClose()
    {
        wglMakeCurrent(DeviceContext, OpenGlContext);

        MSG msg;
        while (true)
        {
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    return true;
                }
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            // OpenGL rendering here
            glClearColor(1.0f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            SwapBuffers(DeviceContext);
        }
    }

    void Window::Close()
    {
        ChangeDisplaySettings(nullptr, 0);
        SetForegroundWindow(GetDesktopWindow());
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(OpenGlContext);
        ReleaseDC(WindowHandle, DeviceContext);
        DestroyWindow(WindowHandle);
        WindowHandle = nullptr;
        DeviceContext = nullptr;
        OpenGlContext = nullptr;
    }
}
