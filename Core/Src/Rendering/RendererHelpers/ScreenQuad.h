#pragma once
#include <cstdint>


namespace Core
{
    /**
     * @brief Class used for rendering full screen quads.
     */
    class ScreenQuad
    {
    private:
        struct CachedData
        {
            uint32_t VertexArray = 0;
            uint32_t VertexBuffer = 0;

            ~CachedData();
        };

    private:
        static CachedData _cachedData;

    private:
        ScreenQuad() = default;

    public:
        ~ScreenQuad();

    public:
        /**
         * @brief Draws quad covering whole screen
         */
        static void Draw();

    private:
        static void Initialize();
    };
}
