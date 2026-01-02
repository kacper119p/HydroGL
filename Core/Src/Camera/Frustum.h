#pragma once
#include <cstdint>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

namespace Core
{
    /**
     * @brief Represents a view frustum.
     */
    class Frustum
    {
    private:
        static constexpr uint32_t PlaneCount = 6;

    private:
        glm::vec4 _planes[PlaneCount];

    public:
        Frustum(const glm::mat4& ViewMatrix, const glm::mat4& ProjectionMatrix);

        explicit Frustum(const glm::mat4& ViewProjectionMatrix);

    public:
        /**
         * @brief Checks whether the frustum intersects the given point.
         * @param Point A point to check.
         * @return True if the frustum contains the point, false otherwise.
         */
        [[nodiscard]] bool IntersectsPoint(const glm::vec3& Point) const;

        /**
         * @brief Checks whether the frustum intersects with the given sphere.
         * @param Center Center of the sphere.
         * @param Radius Radius of the sphere.
         * @return True if the frustum intersects with the sphere, false otherwise.
         */
        [[nodiscard]] bool IntersectsSphere(const glm::vec3& Center, float Radius) const;

        /**
         * @brief Checks whether the frustum intersects with the given axis-aligned bounding box.
         * @param Position Center position of the AABB.
         * @param Extends Extends of the AABB.
         * @return True if the frustum intersects with the AABB, false otherwise.
         */
        [[nodiscard]] bool IntersectsAabb(const glm::vec3& Position, const glm::vec3& Extends) const;
    };
}
