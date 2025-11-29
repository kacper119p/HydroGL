#include <glm/vec3.hpp>
#include <gtest/gtest.h>

#include "Meshes/BoundingBox.h"

class BoundingBoxTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_F(BoundingBoxTest, DefaultConstructor_InitializesMinMaxToZero)
{
    constexpr Core::BoundingBox bbox;
    EXPECT_EQ(bbox.GetMin(), glm::vec3(0.0f, 0.0f, 0.0f));
    EXPECT_EQ(bbox.GetMax(), glm::vec3(0.0f, 0.0f, 0.0f));
}

TEST_F(BoundingBoxTest, ConstructorWithMinMax_SetsCorrectValues)
{
    constexpr glm::vec3 min(-1.0f, -2.0f, -3.0f);
    constexpr glm::vec3 max(1.0f, 2.0f, 3.0f);
    constexpr Core::BoundingBox bbox(min, max);
    EXPECT_EQ(bbox.GetMin(), min);
    EXPECT_EQ(bbox.GetMax(), max);
}

TEST_F(BoundingBoxTest, ConstructorWithPositions_ComputesCorrectBounds)
{
    glm::vec3 positions[] = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 2.0f, 3.0f),
        glm::vec3(-1.0f, -2.0f, -3.0f),
        glm::vec3(0.5f, 1.5f, 2.5f)
    };
    const Core::BoundingBox bbox(positions, 4);
    EXPECT_EQ(bbox.GetMin(), glm::vec3(-1.0f, -2.0f, -3.0f));
    EXPECT_EQ(bbox.GetMax(), glm::vec3(1.0f, 2.0f, 3.0f));
}

TEST_F(BoundingBoxTest, ConstructorWithSinglePosition_SetsSameMinMax)
{
    constexpr glm::vec3 position(5.0f, 10.0f, 15.0f);
    const Core::BoundingBox bbox(&position, 1);
    EXPECT_EQ(bbox.GetMin(), position);
    EXPECT_EQ(bbox.GetMax(), position);
}

TEST_F(BoundingBoxTest, EqualityOperator_ReturnsTrueForEqualBoxes)
{
    constexpr glm::vec3 min(-1.0f, -1.0f, -1.0f);
    constexpr glm::vec3 max(1.0f, 1.0f, 1.0f);
    constexpr Core::BoundingBox bbox1(min, max);
    constexpr Core::BoundingBox bbox2(min, max);
    EXPECT_TRUE(bbox1 == bbox2);
}

TEST_F(BoundingBoxTest, EqualityOperator_ReturnsFalseForDifferentBoxes)
{
    constexpr Core::BoundingBox bbox1(glm::vec3(-1.0f), glm::vec3(1.0f));
    constexpr Core::BoundingBox bbox2(glm::vec3(-2.0f), glm::vec3(2.0f));
    EXPECT_FALSE(bbox1 == bbox2);
}

TEST_F(BoundingBoxTest, InequalityOperator_ReturnsTrueForDifferentBoxes)
{
    constexpr Core::BoundingBox bbox1(glm::vec3(-1.0f), glm::vec3(1.0f));
    constexpr Core::BoundingBox bbox2(glm::vec3(-2.0f), glm::vec3(2.0f));
    EXPECT_TRUE(bbox1 != bbox2);
}

TEST_F(BoundingBoxTest, InequalityOperator_ReturnsFalseForEqualBoxes)
{
    constexpr glm::vec3 min(-1.0f, -1.0f, -1.0f);
    constexpr glm::vec3 max(1.0f, 1.0f, 1.0f);

    constexpr Core::BoundingBox bbox1(min, max);
    constexpr Core::BoundingBox bbox2(min, max);

    EXPECT_FALSE(bbox1 != bbox2);
}
