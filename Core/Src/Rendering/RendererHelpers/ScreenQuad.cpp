#include "ScreenQuad.h"
#include "glad/glad.h"

namespace Core
{
    struct ScreenQuad::CachedData ScreenQuad::_cachedData;

    ScreenQuad::CachedData::~CachedData()
    {
        glDeleteBuffers(1, &VertexBuffer);
        glDeleteVertexArrays(1, &VertexArray);
    }

    ScreenQuad::~ScreenQuad() = default;

    void ScreenQuad::Draw()
    {
        if (_cachedData.VertexBuffer == 0)
        {
            Initialize();
        }
        glBindVertexArray(_cachedData.VertexArray);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
    }

    void ScreenQuad::Initialize()
    {
        constexpr float vertices[] = {
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom left
            3.0f, -1.0f, 0.0f, 2.0f, 0.0f, // bottom right way off-screen
            -1.0f, 3.0f, 0.0f, 0.0f, 2.0f // top left way off-screen
        };

        glGenVertexArrays(1, &_cachedData.VertexArray);
        glGenBuffers(1, &_cachedData.VertexBuffer);

        glBindVertexArray(_cachedData.VertexArray);
        glBindBuffer(GL_ARRAY_BUFFER, _cachedData.VertexBuffer);

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
                     vertices, GL_STATIC_DRAW);

        //Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(0 * sizeof(float)));

        //TexCoords
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));

        glBindVertexArray(0);
    }
}
