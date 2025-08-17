#pragma once
#include <cstdint>

namespace Core
{
    /**
     * @brief Class used to render cube maps.
     */
    class CubeRenderer final
    {
    private:
        struct CachedData
        {
            uint32_t VertexArray = 0;
            uint32_t VertexBuffer = 0;
            uint32_t ElementBuffer = 0;

            ~CachedData();
        };

    private:
        static CachedData _cachedData;

    public:
        /**
         * @brief Renders cube map.
         */
        static void Draw();

    private:
        static void Initialize();
    };
}
