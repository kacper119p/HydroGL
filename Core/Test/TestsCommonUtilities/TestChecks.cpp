#include "TestChecks.h"

#include <glm/detail/type_quat.hpp>
#include <gtest/gtest.h>

namespace
{
    float Epsilon = 1e-5f;
}

namespace TestUtilities
{
    void AssertMat4Equal(const glm::mat4& Tested, const glm::mat4& Expected)
    {
        for (int x = 0; x < 4; ++x)
        {
            for (int y = 0; y < 4; ++y)
            {
                ASSERT_NEAR(Tested[x][y], Expected[x][y], Epsilon);
            }
        }
    }

    void AssertVec3Equal(const glm::vec3& Tested, const glm::vec3& Expected)
    {
        ASSERT_NEAR(Tested.x, Expected.x, Epsilon);
        ASSERT_NEAR(Tested.y, Expected.y, Epsilon);
        ASSERT_NEAR(Tested.z, Expected.z, Epsilon);
    }

    void AssertQuatEqual(const glm::quat& Tested, const glm::quat& Expected)
    {
        ASSERT_NEAR(Tested.x, Expected.x, Epsilon);
        ASSERT_NEAR(Tested.y, Expected.y, Epsilon);
        ASSERT_NEAR(Tested.z, Expected.z, Epsilon);
        ASSERT_NEAR(Tested.w, Expected.w, Epsilon);
    }
}
