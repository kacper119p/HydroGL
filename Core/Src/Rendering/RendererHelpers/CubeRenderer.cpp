#include "CubeRenderer.h"

#include "glad/glad.h"

namespace Core
{
    CubeRenderer::CachedData CubeRenderer::_cachedData;

    CubeRenderer::CachedData::~CachedData()
    {
        glDeleteBuffers(1, &VertexBuffer);
        glDeleteBuffers(1, &ElementBuffer);
        glDeleteVertexArrays(1, &VertexArray);
    }

    void CubeRenderer::Draw()
    {
        if (_cachedData.VertexBuffer == 0)
        {
            Initialize();
        }
        glBindVertexArray(_cachedData.VertexArray);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }

    void CubeRenderer::Initialize()
    {
        constexpr float vertices[] = {
            -0.5f, -0.5f, -0.5f, //0
            0.5f, -0.5f, -0.5f, //1
            -0.5f, 0.5f, -0.5f, //2
            0.5f, 0.5f, -0.5f, //3
            -0.5f, -0.5f, 0.5f, //4
            0.5f, -0.5f, 0.5f, //5
            -0.5f, 0.5f, 0.5f, //6
            0.5f, 0.5f, 0.5f, //7
        };

        constexpr unsigned int faceIndices[] = {
            0, 1, 3,
            3, 2, 0,
            7, 5, 4,
            4, 6, 7,
            0, 2, 6,
            6, 4, 0,
            3, 1, 7,
            5, 7, 1,
            5, 1, 0,
            0, 4, 5,
            3, 7, 2,
            6, 2, 7,
        };

        glGenVertexArrays(1, &_cachedData.VertexArray);
        glGenBuffers(1, &_cachedData.ElementBuffer);
        glGenBuffers(1, &_cachedData.VertexBuffer);

        glBindVertexArray(_cachedData.VertexArray);
        glBindBuffer(GL_ARRAY_BUFFER, _cachedData.VertexBuffer);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _cachedData.ElementBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(faceIndices), faceIndices,
                     GL_STATIC_DRAW);

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
                     vertices, GL_STATIC_DRAW);

        //Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void*>(0 * sizeof(float)));
        glBindVertexArray(0);
    }
}
