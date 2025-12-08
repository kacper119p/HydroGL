#include "TimeManager.h"

#include <windows.h>

namespace
{
    LARGE_INTEGER Frequency;
    LARGE_INTEGER Start;
}

namespace Platform
{
    void TimeManager::Initialize()
    {
        QueryPerformanceFrequency(&Frequency);
        QueryPerformanceCounter(&Start);
    }

    float TimeManager::GetTime()
    {
        LARGE_INTEGER end;
        QueryPerformanceCounter(&end);
        return static_cast<float>(end.QuadPart - Start.QuadPart) / static_cast<float>(Frequency.QuadPart);
    }
}
