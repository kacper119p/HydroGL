#pragma once
#include <glm/glm.hpp>


namespace TestUtilities
{
    void AssertMat4Equal(const glm::mat4& Tested, const glm::mat4& Expected);

    void AssertVec3Equal(const glm::vec3& Tested, const glm::vec3& Expected);

    void AssertQuatEqual(const glm::quat& Tested, const glm::quat& Expected);
}
