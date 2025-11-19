#pragma once
#include <glm/vec3.hpp>

namespace Core
{
    struct BoundingBox
    {
    private:
        glm::vec3 _min;
        glm::vec3 _max;

    public:
        BoundingBox();

        BoundingBox(const glm::vec3& Min, const glm::vec3& Max);

        BoundingBox(const glm::vec3* Positions, uint32_t VerticesCount);

    public:
        [[nodiscard]] const glm::vec3& GetMin() const
        {
            return _min;
        }

        [[nodiscard]] const glm::vec3& GetMax() const
        {
            return _max;
        }

        friend bool operator==(const BoundingBox& Lhs, const BoundingBox& Rhs)
        {
            return Lhs._min == Rhs._min
                   && Lhs._max == Rhs._max;
        }

        friend bool operator!=(const BoundingBox& Lhs, const BoundingBox& Rhs)
        {
            return !(Lhs == Rhs);
        }
    };
}
