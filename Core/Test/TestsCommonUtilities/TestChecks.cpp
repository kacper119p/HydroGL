#include "TestChecks.h"

#include <gtest/gtest.h>

namespace TestUtilities
{
    void AssertMat4Equal(const glm::mat4& Tested, const glm::mat4& Expected)
    {
        for (int x = 0; x < 4; ++x)
        {
            for (int y = 0; y < 4; ++y)
            {
                ASSERT_FLOAT_EQ(Tested[x][y], Expected[x][y]);
            }
        }
    }
} // namespace TestUtilities
