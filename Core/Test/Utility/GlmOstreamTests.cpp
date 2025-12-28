#include <sstream>
#include <glm/mat2x2.hpp>
#include <glm/mat2x3.hpp>
#include <glm/mat3x2.hpp>
#include <glm/mat4x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <gtest/gtest.h>

#include "Utility/GlmOstream.h"


class GlmOstreamTests : public testing::Test
{
protected:
    GlmOstreamTests() = default;

    template<typename T>
    static std::string ToString(const T& Value)
    {
        std::stringstream ss;
        ss << Value;
        return ss.str();
    }
};

TEST_F(GlmOstreamTests, Vector2Output)
{
    constexpr glm::vec2 v(1.0f, 2.0f);
    EXPECT_EQ(ToString(v), "{1, 2}");
}

TEST_F(GlmOstreamTests, Vector3Output)
{
    constexpr glm::vec3 v(1.0f, 2.0f, 3.0f);
    EXPECT_EQ(ToString(v), "{1, 2, 3}");
}

TEST_F(GlmOstreamTests, Vector4Output)
{
    constexpr glm::vec4 v(1.0f, 2.0f, 3.0f, 4.0f);
    EXPECT_EQ(ToString(v), "{1, 2, 3, 4}");
}

TEST_F(GlmOstreamTests, VectorIntOutput)
{
    constexpr glm::ivec3 v(-1, 0, 5);
    EXPECT_EQ(ToString(v), "{-1, 0, 5}");
}

TEST_F(GlmOstreamTests, Matrix2OutputColumnMajor)
{
    const glm::mat2 m(1.0f, 2.0f, 3.0f, 4.0f);

    // Expected format: {{col1}, {col2}}
    EXPECT_EQ(ToString(m), "{{1, 2}, {3, 4}}");
}

TEST_F(GlmOstreamTests, Matrix3OutputIdentity)
{
    const glm::mat3 m(1.0f); // Identity matrix
    EXPECT_EQ(ToString(m), "{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}");
}

TEST_F(GlmOstreamTests, Matrix4OutputFormat)
{
    glm::mat4 m(0.0f);
    m[0] = glm::vec4(1, 2, 3, 4); // Column 0
    m[1] = glm::vec4(5, 6, 7, 8); // Column 1
    m[2] = glm::vec4(9, 10, 11, 12); // Column 2
    m[3] = glm::vec4(13, 14, 15, 16); // Column 3

    const std::string expected =
            "{{1, 2, 3, 4}, "
            "{5, 6, 7, 8}, "
            "{9, 10, 11, 12}, "
            "{13, 14, 15, 16}}";

    // Expected format: {{col0}, {col1}, {col2}, {col3}}
    EXPECT_EQ(ToString(m), expected);
}

TEST_F(GlmOstreamTests, Matrix2x3Output)
{
    const glm::mat2x3 m(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f);

    // Expected format: {{col0}, {col1}}
    EXPECT_EQ(ToString(m), "{{1, 2, 3}, {4, 5, 6}}");
}

TEST_F(GlmOstreamTests, Matrix3x2Output)
{
    const glm::mat3x2 m(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f);

    // Expected format:t: {{col0}, {col1}, {col2}}
    EXPECT_EQ(ToString(m), "{{1, 2}, {3, 4}, {5, 6}}");
}

TEST_F(GlmOstreamTests, Matrix4x3Output)
{
    glm::mat4x3 m(0.0f); // Zero matrix

    m[0] = glm::vec3(1, 1, 1);
    m[3] = glm::vec3(9, 9, 9);
    const std::string expected =
            "{{1, 1, 1}, "
            "{0, 0, 0}, "
            "{0, 0, 0}, "
            "{9, 9, 9}}";

    // Expected format: {{col0}, {col1}, {col2}, {col3}}
    EXPECT_EQ(ToString(m), expected);
}

TEST_F(GlmOstreamTests, QuaternionOutput)
{
    constexpr glm::quat q(1.0f, 2.0f, 3.0f, 4.0f);
    // Expected Output: {w, x, y, z}
    EXPECT_EQ(ToString(q), "{1, 2, 3, 4}");
}

TEST_F(GlmOstreamTests, QuaternionIdentityOutput)
{
    constexpr glm::quat q = glm::identity<glm::quat>(); // w=1, x=0, y=0, z=0
    EXPECT_EQ(ToString(q), "{1, 0, 0, 0}");
}

TEST_F(GlmOstreamTests, QuaternionNegativeOutput)
{
    constexpr glm::quat q(-1.0f, -0.5f, 0.0f, 0.5f);
    EXPECT_EQ(ToString(q), "{-1, -0.5, 0, 0.5}");
}
