#pragma once
#include <glm/common.hpp>
#include <glm/vec3.hpp>

namespace Core
{
    struct BoundingBox
    {
    private:
        glm::vec3 _min;
        glm::vec3 _max;

    public:
        constexpr BoundingBox()
            : _min(0.0f),
              _max(0.0f)
        {
        }

        constexpr BoundingBox(const glm::vec3& Min, const glm::vec3& Max)
            : _min(Min),
              _max(Max)
        {
        }


        constexpr BoundingBox(const glm::vec3* const Positions, const uint32_t VerticesCount)
        {
            _min = Positions[0];
            _max = Positions[0];

            for (int i = 1; i < VerticesCount; ++i)
            {
                _max = glm::max(_max, Positions[i]);
                _min = glm::min(_min, Positions[i]);
            }
        }

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
