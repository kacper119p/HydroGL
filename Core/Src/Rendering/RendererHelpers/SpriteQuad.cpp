#include "SpriteQuad.h"

namespace Core
{
    SpriteQuad::CachedData SpriteQuad::_cachedData;

    void SpriteQuad::BindVao()
    {
        if (_cachedData.VertexBuffer == 0u)
        {
            Initialize();
        }
        glBindVertexArray(_cachedData.VertexArray);
    }

    void SpriteQuad::Initialize()
    {
        constexpr float vertices[] = {
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // Bottom Left
            0.5f, -0.5f, 0.0f, 1.0f, 0.0f, // Bottom Right
            0.5f, 0.5f, 0.0f, 1.0f, 1.0f, // Top Right
            -0.5f, 0.5f, 0.0f, 0.0f, 1.0f // Top Left
        };

        constexpr uint32_t indices[] = {
            0, 1, 2,
            0, 2, 3
        };

        _cachedData.VertexArray = GlVertexArrayUnique::Create();
        _cachedData.VertexBuffer = GlBufferUnique::Create();
        _cachedData.ElementBuffer = GlBufferUnique::Create();

        glNamedBufferStorage(_cachedData.VertexBuffer, sizeof(vertices), vertices, 0);
        glNamedBufferStorage(_cachedData.ElementBuffer, sizeof(indices), indices, 0);

        constexpr int64_t stride = 5 * sizeof(float);
        glVertexArrayVertexBuffer(_cachedData.VertexArray, 0, _cachedData.VertexBuffer, 0, stride);
        glVertexArrayElementBuffer(_cachedData.VertexArray, _cachedData.ElementBuffer);

        glEnableVertexArrayAttrib(_cachedData.VertexArray, 0);
        glVertexArrayAttribFormat(_cachedData.VertexArray, 0, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(_cachedData.VertexArray, 0, 0);

        glEnableVertexArrayAttrib(_cachedData.VertexArray, 1);
        glVertexArrayAttribFormat(_cachedData.VertexArray, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float));
        glVertexArrayAttribBinding(_cachedData.VertexArray, 1, 0);
    }
}
