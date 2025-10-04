#pragma once

namespace Platform
{
    class TimeManager
    {
    public:
        static void Initialize();

        static float GetTime();
    };
}
