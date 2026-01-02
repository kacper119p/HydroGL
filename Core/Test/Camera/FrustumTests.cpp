#include <gtest/gtest.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "Camera/Frustum.h"

class FrustumTests : public testing::Test
{
protected:
    FrustumTests() = default;

    /**
     * @brief Helper to create a standard perspective frustum for testing<br>
     * Camera at (0,0,0) looking down -Z<br>
     * FOV: 90 degrees<br>
     * Aspect Ratio: 1.0<br>
     * Near: 1.0, Far: 10.0<br>
     * Visible range in Z: [-1.0, -10.0]<br>
     * Visible range in X/Y at Z=d: [-d, d]<br>
     */
    static Core::Frustum CreateTestFrustum()
    {
        const glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, 10.0f);
        const glm::mat4 view = glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        return Core::Frustum(view, projection);
    }
};

TEST_F(FrustumTests, IntersectsPoint)
{
    const Core::Frustum frustum = CreateTestFrustum();

    // Point clearly inside
    EXPECT_TRUE(frustum.IntersectsPoint(glm::vec3(0.0f, 0.0f, -5.0f)));

    // Points on the boundary (approximate due to float precision)
    EXPECT_TRUE(frustum.IntersectsPoint(glm::vec3(4.99f, 0.0f, -5.0f)));
    EXPECT_TRUE(frustum.IntersectsPoint(glm::vec3(-4.99f, 0.0f, -5.0f)));

    // Points outside Near/Far planes
    EXPECT_FALSE(frustum.IntersectsPoint(glm::vec3(0.0f, 0.0f, -0.5f))); // Too close (< Near 1.0)
    EXPECT_FALSE(frustum.IntersectsPoint(glm::vec3(0.0f, 0.0f, -10.1f))); // Too far (> Far 10.0)

    // Points outside Left/Right/Top/Bottom
    EXPECT_FALSE(frustum.IntersectsPoint(glm::vec3(5.1f, 0.0f, -5.0f)));
    EXPECT_FALSE(frustum.IntersectsPoint(glm::vec3(-5.1f, 0.0f, -5.0f)));
    EXPECT_FALSE(frustum.IntersectsPoint(glm::vec3(0.0f, 5.1f, -5.0f)));
    EXPECT_FALSE(frustum.IntersectsPoint(glm::vec3(0.0f, -5.1f, -5.0f)));
}

TEST_F(FrustumTests, IntersectsSphere)
{
    const Core::Frustum frustum = CreateTestFrustum();

    // Sphere fully inside
    EXPECT_TRUE(frustum.IntersectsSphere(glm::vec3(0.0f, 0.0f, -5.0f), 1.0f));

    // Sphere partially inside (intersecting side plane)
    EXPECT_TRUE(frustum.IntersectsSphere(glm::vec3(0.0f, 6.0f, -5.0f), 2.0f));

    // Sphere outside (Near plane)
    EXPECT_FALSE(frustum.IntersectsSphere(glm::vec3(0.0f, 0.0f, -0.5f), 0.4f));

    // Sphere intersecting Near plane
    EXPECT_TRUE(frustum.IntersectsSphere(glm::vec3(0.0f, 0.0f, -0.5f), 0.6f));

    // Sphere completely outside
    EXPECT_FALSE(frustum.IntersectsSphere(glm::vec3(0.0f, 20.0f, -5.0f), 1.0f));
}

TEST_F(FrustumTests, IntersectsAabb)
{
    const Core::Frustum frustum = CreateTestFrustum();

    // AABB fully inside
    EXPECT_TRUE(frustum.IntersectsAabb(glm::vec3(0.0f, 0.0f, -5.0f), glm::vec3(1.0f)));

    // AABB intersecting Right plane
    EXPECT_TRUE(frustum.IntersectsAabb(glm::vec3(6.0f, 0.0f, -5.0f), glm::vec3(1.5f, 1.0f, 1.0f)));

    // AABB Outside (Behind View)
    EXPECT_FALSE(frustum.IntersectsAabb(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(1.0f)));

    // AABB Outside (Too far)
    EXPECT_FALSE(frustum.IntersectsAabb(glm::vec3(0.0f, 0.0f, -15.0f), glm::vec3(1.0f)));

    // AABB encompassing the frustum
    EXPECT_TRUE(frustum.IntersectsAabb(glm::vec3(0.0f, 0.0f, -5.0f), glm::vec3(50.0f)));
}
