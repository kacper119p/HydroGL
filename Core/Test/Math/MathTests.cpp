#include <glm/mat2x2.hpp>
#include <glm/vec2.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <gtest/gtest.h>

#include "Math/Math.h"

class MathTests : public testing::Test
{
protected:
    MathTests() = default;
};

TEST_F(MathTests, IntegerDivCeilPositive)
{
    EXPECT_EQ(Core::Math::IntegerDivCeilPositive(3, 5), 1);
    EXPECT_EQ(Core::Math::IntegerDivCeilPositive(5, 5), 1);
    EXPECT_EQ(Core::Math::IntegerDivCeilPositive(10, 3), 4);
    EXPECT_EQ(Core::Math::IntegerDivCeilPositive(0, 5), 0);
    EXPECT_EQ(Core::Math::IntegerDivCeilPositive(7, 2), 4);
}

TEST_F(MathTests, IntegerDivCeil)
{
    // Both positive
    EXPECT_EQ(Core::Math::IntegerDivCeil(3, 5), 1);
    EXPECT_EQ(Core::Math::IntegerDivCeil(5, 5), 1);
    EXPECT_EQ(Core::Math::IntegerDivCeil(10, 3), 4);
    EXPECT_EQ(Core::Math::IntegerDivCeil(0, 5), 0);
    EXPECT_EQ(Core::Math::IntegerDivCeil(7, 2), 4);

    // Negative numerator
    EXPECT_EQ(Core::Math::IntegerDivCeil(-3, 5), 0);
    EXPECT_EQ(Core::Math::IntegerDivCeil(-5, 5), -1);
    EXPECT_EQ(Core::Math::IntegerDivCeil(-10, 3), -3);
    EXPECT_EQ(Core::Math::IntegerDivCeil(-7, 2), -3);
    EXPECT_EQ(Core::Math::IntegerDivCeil(-0, 2), 0);

    // Negative denominator
    EXPECT_EQ(Core::Math::IntegerDivCeil(3, -5), 0);
    EXPECT_EQ(Core::Math::IntegerDivCeil(5, -5), -1);
    EXPECT_EQ(Core::Math::IntegerDivCeil(10, -3), -3);
    EXPECT_EQ(Core::Math::IntegerDivCeil(7, -2), -3);
    EXPECT_EQ(Core::Math::IntegerDivCeil(-0, -2), 0);

    //Both negative
    EXPECT_EQ(Core::Math::IntegerDivCeil(-3, -5), 1);
    EXPECT_EQ(Core::Math::IntegerDivCeil(-5, -5), 1);
    EXPECT_EQ(Core::Math::IntegerDivCeil(-10, -3), 4);
    EXPECT_EQ(Core::Math::IntegerDivCeil(-7, -2), 4);
    EXPECT_EQ(Core::Math::IntegerDivCeil(0, -2), 0);
}

TEST_F(MathTests, Lerp)
{
    EXPECT_FLOAT_EQ(Core::Math::Lerp(0.0f, 10.0f, 0.0f), 0.0f);
    EXPECT_FLOAT_EQ(Core::Math::Lerp(0.0f, 10.0f, 1.0f), 10.0f);
    EXPECT_FLOAT_EQ(Core::Math::Lerp(0.0f, 10.0f, 0.5f), 5.0f);

    EXPECT_FLOAT_EQ(Core::Math::Lerp(0.0f, 10.0f, 2.0f), 20.0f);
    EXPECT_FLOAT_EQ(Core::Math::Lerp(0.0f, 10.0f, -2.0f), -20.0f);

    EXPECT_DOUBLE_EQ(Core::Math::Lerp(0.0, 10.0, 0.0), 0.0);
    EXPECT_DOUBLE_EQ(Core::Math::Lerp(0.0, 10.0, 1.0), 10.0);
    EXPECT_DOUBLE_EQ(Core::Math::Lerp(0.0, 10.0, 0.5), 5.0);

    EXPECT_DOUBLE_EQ(Core::Math::Lerp(0.0, 10.0, 2.0), 20.0f);
    EXPECT_DOUBLE_EQ(Core::Math::Lerp(0.0, 10.0, -2.0), -20.0f);
}

TEST_F(MathTests, CreateRotationMatrix2DFromUnitVector)
{
    constexpr glm::vec2 Up = glm::vec2(0.0f, 1.0f);

    glm::vec2 newUp = glm::vec2(0.0f, 1.0f);
    glm::mat2 rotationMatrix = Core::Math::CreateRotationMatrix2DFromUnitVector(newUp);
    EXPECT_EQ(newUp, Up * rotationMatrix);

    newUp = glm::vec2(1.0f, 0.0f);
    rotationMatrix = Core::Math::CreateRotationMatrix2DFromUnitVector(newUp);
    EXPECT_EQ(newUp, Up * rotationMatrix);
}
