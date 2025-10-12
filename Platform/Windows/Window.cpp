#pragma comment(lib, "Shcore.lib")

#include "Window.h"

#include <cstdint>
#include <imgui.h>
#include <Windows.h>
#include <glad/glad.h>
#include <spdlog/spdlog.h>

#include "Debug.h"
#include "Input.h"
#include "GL/wglext.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND HWnd, UINT Msg, WPARAM WParam, LPARAM LParam);

namespace Platform
{
    static LRESULT CALLBACK WndProc(HWND Hwnd, const UINT Msg, const WPARAM WParam, const LPARAM LParam)
    {
        static bool isDragging = false;
        static POINT oldMousePosition;

        const Window* window = reinterpret_cast<Window*>(GetWindowLongPtr(Hwnd, GWLP_USERDATA));

        if (ImGui_ImplWin32_WndProcHandler(Hwnd, Msg, WParam, LParam))
            return 1LL;

        switch (Msg)
        {
            case WM_CREATE:
            {
                CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(LParam);
                SetWindowLongPtr(Hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
            }
            break;
            case WM_CLOSE:
                PostQuitMessage(0);
                return 0;
            case WM_SETCURSOR:
                if (LOWORD(LParam) == HTCLIENT)
                {
                    SetCursor(LoadCursor(nullptr, IDC_ARROW));
                }
                break;

            case WM_KEYDOWN:
                switch (WParam)
                {
                    case 'W':
                        Input::DispatchMovementEvent(Input::KeyboardButton::W, Input::ButtonEventType::KeyPressed);
                        break;
                    case 'A':
                        Input::DispatchMovementEvent(Input::KeyboardButton::A, Input::ButtonEventType::KeyPressed);
                        break;
                    case 'S':
                        Input::DispatchMovementEvent(Input::KeyboardButton::S, Input::ButtonEventType::KeyPressed);
                        break;
                    case 'D':
                        Input::DispatchMovementEvent(Input::KeyboardButton::D, Input::ButtonEventType::KeyPressed);
                        break;
                    default:
                        break;
                }
                break;
            case WM_KEYUP:
                switch (WParam)
                {
                    case 'W':
                        Input::DispatchMovementEvent(Input::KeyboardButton::W, Input::ButtonEventType::KeyReleased);
                        break;
                    case 'A':
                        Input::DispatchMovementEvent(Input::KeyboardButton::A, Input::ButtonEventType::KeyReleased);
                        break;
                    case 'S':
                        Input::DispatchMovementEvent(Input::KeyboardButton::S, Input::ButtonEventType::KeyReleased);
                        break;
                    case 'D':
                        Input::DispatchMovementEvent(Input::KeyboardButton::D, Input::ButtonEventType::KeyReleased);
                        break;
                    default:
                        break;
                }
                break;
            case WM_MOUSEMOVE:
            {
                if (isDragging)
                {
                    POINT currentMousePosition;
                    GetCursorPos(&currentMousePosition);
                    const float deltaX = static_cast<float>(currentMousePosition.x - oldMousePosition.x)
                                         / static_cast<float>(window->GetWindowWidth());
                    const float deltaY = static_cast<float>(currentMousePosition.y - oldMousePosition.y)
                                         / static_cast<float>(window->GetWindowHeight());
                    SetCursorPos(oldMousePosition.x, oldMousePosition.y);
                    Input::DispatchCameraRotationEvent(deltaX, deltaY);
                }
                break;
            }
            case WM_RBUTTONDOWN:
            {
                Input::DispatchCameraDragEvent(true);
                ShowCursor(FALSE);
                GetCursorPos(&oldMousePosition);
                isDragging = true;
                break;
            }
            case WM_RBUTTONUP:
                ShowCursor(TRUE);
                isDragging = false;
                Input::DispatchCameraDragEvent(false);
                break;
            case WM_SETFOCUS:
                if (isDragging)
                {
                    ShowCursor(TRUE);
                    isDragging = false;
                    Input::DispatchCameraDragEvent(false);
                }
                break;
            case WM_KILLFOCUS:
                ShowCursor(TRUE);
                isDragging = false;
                Input::StopMovement();
                break;
            default:
                break;
        }
        return DefWindowProc(Hwnd, Msg, WParam, LParam);
    }

    Window::Window(HWND WindowHandle, HDC DeviceContext, HGLRC OpenGlContext, const uint32_t WindowWidth,
                   const uint32_t WindowHeight) : _windowHandle(WindowHandle),
                                                  _deviceContext(DeviceContext),
                                                  _openGlContext(OpenGlContext),
                                                  _windowWidth(WindowWidth),
                                                  _windowHeight(WindowHeight)
    {
    }

    Window::~Window()
    {
        ChangeDisplaySettings(nullptr, 0);
        SetForegroundWindow(GetDesktopWindow());
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(_openGlContext);
        ReleaseDC(_windowHandle, _deviceContext);
        DestroyWindow(_windowHandle);
        _windowHandle = nullptr;
        _deviceContext = nullptr;
        _openGlContext = nullptr;
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

        const int32_t windowWidth = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
        const int32_t windowHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;

        HWND hwnd = CreateWindowEx(WS_EX_TOPMOST,
                                   className,
                                   "HydroGL",
                                   WS_POPUP,
                                   monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
                                   windowWidth,
                                   windowHeight,
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
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        glEnable(GL_MULTISAMPLE);

        Window* window = new Window(hwnd, deviceContext, fullGlContext, windowWidth, windowHeight);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
        return window;
    }

    void Window::PoolEvents()
    {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            switch (msg.message)
            {
                case WM_QUIT:
                    _shouldClose = true;
                    break;
                default:
                    break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    bool Window::ShouldClose() const
    {
        return _shouldClose;
    }

    void Window::MakeContextCurrent() const
    {
        wglMakeCurrent(_deviceContext, _openGlContext);
    }

    void Window::SwapBuffers() const
    {
        ::SwapBuffers(_deviceContext);
    }
}
