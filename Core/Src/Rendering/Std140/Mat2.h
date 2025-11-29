#pragma once
#include <glm/mat2x2.hpp>
#include <glm/vec2.hpp>

namespace Core::Std140
{
    struct Mat2
    {
    public:
        glm::vec2 Column0;
        alignas(16) glm::vec2 Column1;

    public:
        Mat2();

        Mat2(const glm::vec2& Column0, const glm::vec2& Column1)
            : Column0(Column0),
              Column1(Column1)
        {
        }

        explicit Mat2(const glm::mat2& Matrix)
            : Column0(Matrix[0]),
              Column1(Matrix[1])
        {
        }
    };
}
