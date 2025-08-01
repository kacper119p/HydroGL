#pragma once
#include <glm/glm.hpp>


namespace TestUtilities
{
    void AssertMat4Equal(const glm::mat4& Tested, const glm::mat4& Expected);
}
