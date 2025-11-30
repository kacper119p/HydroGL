#include <algorithm>
#include <utility>
#include <vector>
#include <gtest/gtest.h>

#include "Utility/GlResourceUnique.h"

struct MockTraits
{
    static inline uint32_t NextId = 1;
    static inline std::vector<uint32_t> DeletedIds;

    static uint32_t Create()
    {
        return NextId++;
    }

    static void Delete(const uint32_t id)
    {
        DeletedIds.push_back(id);
    }

    static void ResetState()
    {
        NextId = 1;
        DeletedIds.clear();
    }
};

using TestResource = Core::Detail::GlResourceUnique<MockTraits>;

class GlResourceUniqueTests : public testing::Test
{
protected:
    GlResourceUniqueTests() = default;

    void SetUp() override
    {
        // Ensure a clean state before every test
        MockTraits::ResetState();
    }
};

TEST_F(GlResourceUniqueTests, LifecycleCreatesAndDeletes)
{ {
        TestResource resource = TestResource::Create();
        // NextId starts at 1, so the first resource should be 1
        EXPECT_EQ(resource.Get(), 1u);
        EXPECT_TRUE(resource); // operator bool()
    } // resource goes out of scope here

    // Verify Delete was called exactly once with the correct ID
    ASSERT_EQ(MockTraits::DeletedIds.size(), 1);
    EXPECT_EQ(MockTraits::DeletedIds[0], 1u);
}

TEST_F(GlResourceUniqueTests, MoveConstructorTransfersOwnership)
{
    TestResource source = TestResource::Create(); // ID: 1
    const TestResource destination(std::move(source));

    // Destination should have the ID
    EXPECT_EQ(destination.Get(), 1u);
    // Source should be empty (0)
    EXPECT_EQ(source.Get(), 0u);
    EXPECT_FALSE(source);

    // No deletions should have occurred yet
    EXPECT_TRUE(MockTraits::DeletedIds.empty());
}

TEST_F(GlResourceUniqueTests, MoveAssignmentDeletesOldAndAdoptsNew)
{
    TestResource oldRes = TestResource::Create(); // ID: 1
    TestResource newRes = TestResource::Create(); // ID: 2
    // Move newRes into oldRes.
    // Expectation: ID 1 is deleted. oldRes takes ID 2. newRes becomes 0.
    oldRes = std::move(newRes);

    EXPECT_EQ(oldRes.Get(), 2u);
    EXPECT_EQ(newRes.Get(), 0u);

    // Verify ID 1 was deleted immediately
    ASSERT_EQ(MockTraits::DeletedIds.size(), 1);
    EXPECT_EQ(MockTraits::DeletedIds[0], 1u);
}

TEST_F(GlResourceUniqueTests, MoveAssignmentSelfAssignmentDoNothing)
{
    TestResource res = TestResource::Create(); // ID: 1
    // Self move
    res = std::move(res);

    // Should remain unchanged
    EXPECT_EQ(res.Get(), 1u);
    EXPECT_TRUE(MockTraits::DeletedIds.empty());
}

TEST_F(GlResourceUniqueTests, ResetDeletesOldAndSetsNew)
{
    TestResource res = TestResource::Create(); // ID: 1
    // Manually reset with an arbitrary ID
    res.Reset(99u);

    EXPECT_EQ(res.Get(), 99u);
    // Verify ID 1 was deleted
    ASSERT_EQ(MockTraits::DeletedIds.size(), 1);
    EXPECT_EQ(MockTraits::DeletedIds[0], 1u);
}

TEST_F(GlResourceUniqueTests, ResetSameIdDoesNothing)
{
    TestResource res = TestResource::Create(); // ID: 1
    // Reset with the same ID
    res.Reset(1u);

    EXPECT_EQ(res.Get(), 1u);
    EXPECT_TRUE(MockTraits::DeletedIds.empty());
}

TEST_F(GlResourceUniqueTests, ReleaseDetachesOwnership)
{
    uint32_t detachedId = 0; {
        TestResource res = TestResource::Create(); // ID: 1
        detachedId = res.Release();

        // Object should now be empty
        EXPECT_EQ(res.Get(), 0u);
    } // Destructor runs here, but should do nothing

    // Verify NOTHING was deleted because we released it
    EXPECT_TRUE(MockTraits::DeletedIds.empty());
    // Verify we got the correct ID back
    EXPECT_EQ(detachedId, 1u);
}

TEST_F(GlResourceUniqueTests, ImplicitConversion)
{
    const TestResource res = TestResource::Create(); // ID: 1
    // Test operator uint32_t()
    const uint32_t rawId = res;
    EXPECT_EQ(rawId, 1u);
}

TEST_F(GlResourceUniqueTests, EqualityOperators)
{
    const TestResource res1 = TestResource::Create(); // ID: 1
    const TestResource res2 = TestResource::Create(); // ID: 2

    EXPECT_NE(res1, res2);
    EXPECT_EQ(res1, 1u);
    EXPECT_NE(res1, 2u);
}
