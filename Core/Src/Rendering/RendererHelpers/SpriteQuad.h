#pragma once

#include "Utility/GlResourceUnique.h"

namespace Core
{
    class SpriteQuad
    {
    private:
        struct CachedData
        {
            GlVertexArrayUnique VertexArray;
            GlBufferUnique VertexBuffer;
            GlBufferUnique ElementBuffer;
        };

    private:
        static CachedData _cachedData;

    private:
        SpriteQuad() = default;

    public:
        /**
         * @brief Draws quad covering whole screen
         */
        static void BindVao();

    private:
        static void Initialize();
    };
}
