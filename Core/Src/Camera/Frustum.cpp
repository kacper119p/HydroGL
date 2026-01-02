#include "Frustum.h"

#include "glm/gtc/type_ptr.inl"

namespace Core
{
    Frustum::Frustum(const glm::mat4& ViewMatrix, const glm::mat4& ProjectionMatrix) : Frustum(
        ProjectionMatrix * ViewMatrix)
    {
    }

    Frustum::Frustum(const glm::mat4& ViewProjectionMatrix)
    {
        const float* const m = glm::value_ptr(ViewProjectionMatrix);
        _planes[0] = glm::vec4(m[3] + m[0], m[7] + m[4], m[11] + m[8], m[15] + m[12]); // Left
        _planes[1] = glm::vec4(m[3] - m[0], m[7] - m[4], m[11] - m[8], m[15] - m[12]); // Right
        _planes[2] = glm::vec4(m[3] + m[1], m[7] + m[5], m[11] + m[9], m[15] + m[13]); // Bottom
        _planes[3] = glm::vec4(m[3] - m[1], m[7] - m[5], m[11] - m[9], m[15] - m[13]); // Top
        _planes[4] = glm::vec4(m[3] + m[2], m[7] + m[6], m[11] + m[10], m[15] + m[14]); // Near
        _planes[5] = glm::vec4(m[3] - m[2], m[7] - m[6], m[11] - m[10], m[15] - m[14]); // Far

        for (glm::vec4& plane : _planes)
        {
            const float length = glm::length(glm::vec3(plane));
            plane /= length;
        }
    }

    bool Frustum::IntersectsPoint(const glm::vec3& Point) const
    {
        for (const glm::vec4& plane : _planes)
        {
            const float distance = glm::dot(glm::vec3(plane), Point) + plane.w;
            if (distance < 0.0f)
            {
                return false;
            }
        }
        return true;
    }

    bool Frustum::IntersectsSphere(const glm::vec3& Center, const float Radius) const
    {
        for (const glm::vec4& plane : _planes)
        {
            const float distance = glm::dot(glm::vec3(plane), Center) + plane.w;
            if (distance < -Radius)
            {
                return false;
            }
        }
        return true;
    }

    bool Frustum::IntersectsAabb(const glm::vec3& Position, const glm::vec3& Extends) const
    {
        for (const glm::vec4& plane : _planes)
        {
            const glm::vec3 planeNormal = glm::vec3(plane);
            const float distance = glm::dot(planeNormal, Position) + plane.w;
            const float radius = glm::dot(glm::abs(planeNormal), Extends);

            if (distance < -radius)
            {
                return false;
            }
        }
        return true;
    }
}
