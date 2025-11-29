#pragma once
#include <glm/mat3x3.hpp>
#include <glm/vec3.hpp>

namespace Core::Std140
{
    struct Mat3
    {
    public:
        glm::vec3 Column0;
        alignas(16) glm::vec3 Column1;
        alignas(16) glm::vec3 Column2;

    public:
        Mat3();

        Mat3(const glm::vec3& Column0, const glm::vec3& Column1, const glm::vec3& Column2)
            : Column0(Column0),
              Column1(Column1),
              Column2(Column2)
        {
        }

        explicit Mat3(const glm::mat3& Matrix)
            : Column0(Matrix[0]),
              Column1(Matrix[1]),
              Column2(Matrix[2])
        {
        }
    };
}
