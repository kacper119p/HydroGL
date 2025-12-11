#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "Collections/RingBuffer.h"

struct LifecycleStats
{
    int Constructed = 0;
    int Destructed = 0;

    [[nodiscard]] int Active() const
    {
        return Constructed - Destructed;
    }
};

struct LifecycleObject
{
    LifecycleStats* Stats = nullptr;
    int Id = -1;

    LifecycleObject() = default;

    LifecycleObject(LifecycleStats* Stats, int Id)
        : Stats(Stats), Id(Id)
    {
        if (Stats) Stats->Constructed++;
    }

    LifecycleObject(const LifecycleObject& Other)
        : Stats(Other.Stats), Id(Other.Id)
    {
        if (Stats) Stats->Constructed++;
    }

    LifecycleObject(LifecycleObject&& Other) noexcept
        : Stats(Other.Stats), Id(Other.Id)
    {
        if (Stats) Stats->Constructed++;
    }

    ~LifecycleObject()
    {
        if (Stats) Stats->Destructed++;
    }

    LifecycleObject& operator=(const LifecycleObject&) = delete;

    LifecycleObject& operator=(LifecycleObject&&) = delete;
};

class RingBufferTests : public testing::Test
{
protected:
    RingBufferTests() = default;
};

TEST_F(RingBufferTests, DefaultState)
{
    Core::RingBuffer<int32_t, 5> buffer;

    EXPECT_TRUE(buffer.Empty());
    EXPECT_FALSE(buffer.Full());
    EXPECT_EQ(buffer.Size(), 0);
    EXPECT_EQ(buffer.GetCapacity(), 5);
}

TEST_F(RingBufferTests, PushAndPop)
{
    Core::RingBuffer<int32_t, 3> buffer;

    EXPECT_TRUE(buffer.Push(10));
    EXPECT_TRUE(buffer.Push(20));

    EXPECT_EQ(buffer.Size(), 2);
    EXPECT_FALSE(buffer.Empty());

    EXPECT_EQ(buffer.Pop(), 10);
    EXPECT_EQ(buffer.Pop(), 20);

    EXPECT_TRUE(buffer.Empty());
    EXPECT_EQ(buffer.Size(), 0);
}

TEST_F(RingBufferTests, PushRejectionWhenFull)
{
    Core::RingBuffer<int32_t, 2> buffer;

    EXPECT_TRUE(buffer.Push(1));
    EXPECT_TRUE(buffer.Push(2));
    EXPECT_TRUE(buffer.Full());
    EXPECT_EQ(buffer.Size(), 2);

    // Should fail to push to a full buffer
    EXPECT_FALSE(buffer.Push(3));
    // Size and contents should remain unchanged
    EXPECT_EQ(buffer.Size(), 2);
    EXPECT_EQ(buffer.Peek(), 1);
}

TEST_F(RingBufferTests, PushForceOverwritesOldest)
{
    Core::RingBuffer<int32_t, 3> buffer;

    buffer.Push(1);
    buffer.Push(2);
    buffer.Push(3);

    EXPECT_TRUE(buffer.Full());

    // Force push should overwrite '1' (the oldest)
    buffer.PushForce(4);

    EXPECT_TRUE(buffer.Full());
    EXPECT_EQ(buffer.Size(), 3);
    // Order should now be 2, 3, 4
    EXPECT_EQ(buffer.Pop(), 2);
    EXPECT_EQ(buffer.Pop(), 3);
    EXPECT_EQ(buffer.Pop(), 4);
}

TEST_F(RingBufferTests, Peek)
{
    Core::RingBuffer<int32_t, 3> buffer;

    buffer.Push(100);
    buffer.Push(200);

    // Peek should return the head (next to pop) without removing it
    EXPECT_EQ(buffer.Peek(), 100);
    EXPECT_EQ(buffer.Size(), 2);
    buffer.Pop();
    EXPECT_EQ(buffer.Peek(), 200);
}

TEST_F(RingBufferTests, Clear)
{
    Core::RingBuffer<int32_t, 3> buffer;

    buffer.Push(1);
    buffer.Push(2);
    buffer.Clear();

    EXPECT_TRUE(buffer.Empty());
    EXPECT_FALSE(buffer.Full());
    EXPECT_EQ(buffer.Size(), 0);
    // Ensure we can start pushing again after clear
    EXPECT_TRUE(buffer.Push(3));
    EXPECT_EQ(buffer.Size(), 1);
}

TEST_F(RingBufferTests, IteratorStandardTraversal)
{
    Core::RingBuffer<int32_t, 5> buffer;
    buffer.Push(10);
    buffer.Push(20);
    buffer.Push(30);

    std::vector<int32_t> results;
    for (const auto& item : buffer)
    {
        results.push_back(item);
    }

    ASSERT_EQ(results.size(), 3);
    EXPECT_EQ(results[0], 10);
    EXPECT_EQ(results[1], 20);
    EXPECT_EQ(results[2], 30);
}

TEST_F(RingBufferTests, IteratorWrappedTraversal)
{
    // Test iterator logic when the internal buffer wraps around
    Core::RingBuffer<int32_t, 3> buffer;

    buffer.Push(1); // [1, _, _]
    buffer.Push(2); // [1, 2, _]
    buffer.Push(3); // [1, 2, 3] (Full)
    buffer.Pop(); // [_, 2, 3] (Head wrapped)
    buffer.Push(4); // [4, 2, 3] (Internally wraps, logical order: 2, 3, 4)

    std::vector<int32_t> results;
    for (const auto& item : buffer)
    {
        results.push_back(item);
    }

    ASSERT_EQ(results.size(), 3);
    EXPECT_EQ(results[0], 2);
    EXPECT_EQ(results[1], 3);
    EXPECT_EQ(results[2], 4);
}

TEST_F(RingBufferTests, ComplexTypes)
{
    // Verify aligned_storage and placement new/delete work for non-trivial types
    Core::RingBuffer<std::string, 3> buffer;

    std::string s1 = "Hello";
    buffer.Push(s1);
    buffer.Push(std::string("World"));

    EXPECT_EQ(buffer.Size(), 2);
    EXPECT_EQ(buffer.Peek(), "Hello");

    std::string result = buffer.Pop();
    EXPECT_EQ(result, "Hello");
    EXPECT_EQ(buffer.Peek(), "World");
}

TEST_F(RingBufferTests, IteratorComparison)
{
    Core::RingBuffer<int32_t, 3> buffer;
    buffer.Push(1);
    buffer.Push(2);

    auto it = buffer.begin();
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 2);

    ++it;
    EXPECT_EQ(it, buffer.end());
}

TEST_F(RingBufferTests, ComplexTypeLifecycleAndPushForce)
{
    LifecycleStats stats; {
        Core::RingBuffer<LifecycleObject, 3> buffer;

        // 1. Fill the buffer
        buffer.Push(LifecycleObject(&stats, 1));
        buffer.Push(LifecycleObject(&stats, 2));
        buffer.Push(LifecycleObject(&stats, 3));

        EXPECT_TRUE(buffer.Full());
        // 3 objects in buffer + temporary objects created during Push are all accounted for.
        // Net active objects should be exactly 3 (those inside the buffer).
        EXPECT_EQ(stats.Active(), 3);
        EXPECT_EQ(buffer.Peek().Id, 1);

        // 2. PushForce on a full buffer
        // Expected behavior: Oldest element (1) is destroyed BEFORE inserting new (4).
        buffer.PushForce(LifecycleObject(&stats, 4));

        // Logic check:
        // - Temp object (4) created: Active = 4
        // - PushForce destroys item (1) inside buffer: Active = 3
        // - PushForce move-constructs item (4) into buffer: Active = 4
        // - Temp object (4) destroyed at end of line: Active = 3
        EXPECT_EQ(stats.Active(), 3);
        EXPECT_EQ(buffer.Size(), 3);
        // Verify order: 1 is gone, head is now 2
        EXPECT_EQ(buffer.Peek().Id, 2);

        // Pop remaining items to verify validity
        auto item2 = buffer.Pop();
        EXPECT_EQ(item2.Id, 2);
        auto item3 = buffer.Pop();
        EXPECT_EQ(item3.Id, 3);
        auto item4 = buffer.Pop();
        EXPECT_EQ(item4.Id, 4);

        EXPECT_TRUE(buffer.Empty());
        // Active count is 3 because we hold item2, item3, item4 locally in this scope
        EXPECT_EQ(stats.Active(), 3);
    } // Scope ended: item2, item3, item4 destroyed. Buffer destroyed (was empty).
    EXPECT_EQ(stats.Active(), 0);
}

TEST_F(RingBufferTests, DestructorCleansUpRemainingItems)
{
    // This test verifies that if the RingBuffer goes out of scope while holding items,
    // it correctly calls destructors for those items.

    LifecycleStats stats; {
        Core::RingBuffer<LifecycleObject, 5> buffer;

        buffer.Push(LifecycleObject(&stats, 10));
        buffer.Push(LifecycleObject(&stats, 20));
        buffer.Push(LifecycleObject(&stats, 30));

        EXPECT_EQ(buffer.Size(), 3);
        EXPECT_EQ(stats.Active(), 3);

        // We explicitly DO NOT pop items here.
        // We let the buffer go out of scope.
    }

    // After buffer destruction, all items inside should have been destroyed.
    EXPECT_EQ(stats.Active(), 0);
    EXPECT_EQ(stats.Constructed, stats.Destructed);
}

TEST_F(RingBufferTests, EmplaceAvoidsTemporaryCopies)
{
    LifecycleStats stats;
    Core::RingBuffer<LifecycleObject, 3> buffer;

    // --- Scenario 1: Using Push ---
    // 1. LifecycleObject(&stats, 1) creates a temporary object (Constructed: 1)
    // 2. Push() move-constructs it into the buffer (Constructed: 2)
    // 3. Temporary object is destroyed at end of statement (Destructed: 1)
    buffer.Push(LifecycleObject(&stats, 1));
    // We expect 2 total constructions happened for 1 item
    EXPECT_EQ(stats.Constructed, 2);
    EXPECT_EQ(stats.Active(), 1);

    // Reset stats for clean comparison
    stats.Constructed = 0;
    stats.Destructed = 0;

    // --- Scenario 2: Using Emplace ---
    // 1. Arguments are forwarded. Object is constructed DIRECTLY in the buffer.
    // No temporary object is created, no move operation occurs.
    const bool success = buffer.Emplace(&stats, 2);

    EXPECT_TRUE(success);
    // We expect exactly 1 construction
    EXPECT_EQ(stats.Constructed, 1);
    EXPECT_EQ(stats.Active(), 1);

    // Verify data integrity
    EXPECT_EQ(buffer.Pop().Id, 1); // From Push
    EXPECT_EQ(buffer.Pop().Id, 2); // From Emplace
}

TEST_F(RingBufferTests, EmplaceForceOverwritesAndAvoidsCopies)
{
    LifecycleStats stats;
    Core::RingBuffer<LifecycleObject, 3> buffer;

    // 1. Fill the buffer
    buffer.Emplace(&stats, 10);
    buffer.Emplace(&stats, 20);
    buffer.Emplace(&stats, 30);

    EXPECT_TRUE(buffer.Full());
    EXPECT_EQ(stats.Active(), 3);
    EXPECT_EQ(buffer.Peek().Id, 10); // Oldest is 10

    // Instead of resetting counters (which breaks Active() calculation),
    // we snapshot the state before the operation.
    int prevConstructed = stats.Constructed;
    int prevDestructed = stats.Destructed;

    // 2. Perform EmplaceForce on a full buffer
    // Expected:
    // - 1 destruction (the oldest element '10')
    // - 1 construction (the new element '40' in-place)
    buffer.EmplaceForce(&stats, 40);

    // Verify Construction efficiency (should increase by exactly 1)
    EXPECT_EQ(stats.Constructed, prevConstructed + 1);
    // Verify Destruction logic (should increase by exactly 1)
    EXPECT_EQ(stats.Destructed, prevDestructed + 1);
    // State Check
    EXPECT_TRUE(buffer.Full());
    // Active count remains 3 (1 destroyed, 1 added, net change 0)
    EXPECT_EQ(stats.Active(), 3);
    // Check order: 10 is gone, 20 is the new head
    EXPECT_EQ(buffer.Peek().Id, 20);

    // Verify contents
    EXPECT_EQ(buffer.Pop().Id, 20);
    EXPECT_EQ(buffer.Pop().Id, 30);
    EXPECT_EQ(buffer.Pop().Id, 40);
}

TEST_F(RingBufferTests, CopyConstructor)
{
    LifecycleStats stats;
    Core::RingBuffer<LifecycleObject, 5> buffer1;

    buffer1.Push(LifecycleObject(&stats, 1));
    buffer1.Push(LifecycleObject(&stats, 2));

    // Initial state: 2 items active
    EXPECT_EQ(stats.Active(), 2);

    // Copy Constructor
    Core::RingBuffer<LifecycleObject, 5> buffer2(buffer1);

    // Should have 2 more items (copies)
    EXPECT_EQ(stats.Active(), 4);
    EXPECT_EQ(buffer2.Size(), 2);
    EXPECT_FALSE(buffer2.Empty());

    // Verify contents are independent
    EXPECT_EQ(buffer2.Pop().Id, 1);
    EXPECT_EQ(buffer2.Pop().Id, 2);

    // Buffer1 should be untouched
    EXPECT_EQ(buffer1.Size(), 2);
    EXPECT_EQ(buffer1.Peek().Id, 1);
}

TEST_F(RingBufferTests, MoveConstructor)
{
    LifecycleStats stats; {
        Core::RingBuffer<LifecycleObject, 5> buffer1;

        buffer1.Push(LifecycleObject(&stats, 10));
        buffer1.Push(LifecycleObject(&stats, 20));

        EXPECT_EQ(stats.Active(), 2);

        // Move Constructor
        Core::RingBuffer<LifecycleObject, 5> buffer2(std::move(buffer1));

        // Buffer2 receives items
        EXPECT_EQ(buffer2.Size(), 2);
        EXPECT_EQ(buffer2.Peek().Id, 10);

        // Buffer1 should be empty
        EXPECT_TRUE(buffer1.Empty());
        EXPECT_EQ(buffer1.Size(), 0);
    }
    // Both buffers out of scope -> All items destroyed
    EXPECT_EQ(stats.Active(), 0);
}

TEST_F(RingBufferTests, CopyAssignment)
{
    LifecycleStats stats;
    Core::RingBuffer<LifecycleObject, 5> b1;
    b1.Push(LifecycleObject(&stats, 1));

    Core::RingBuffer<LifecycleObject, 5> b2;
    b2.Push(LifecycleObject(&stats, 100));
    b2.Push(LifecycleObject(&stats, 200));

    EXPECT_EQ(stats.Active(), 3); // 1 in b1, 2 in b2

    // Copy Assignment: b2 = b1
    b2 = b1;

    // Expected:
    // - b2's original items (100, 200) destroyed (-2 active)
    // - b1's item (1) copied into b2 (+1 active)
    // Total active: 2 (b1 has 1, b2 has 1)
    EXPECT_EQ(stats.Active(), 2);

    EXPECT_EQ(b2.Size(), 1);
    EXPECT_EQ(b2.Peek().Id, 1);
}

TEST_F(RingBufferTests, MoveAssignment)
{
    LifecycleStats stats; {
        Core::RingBuffer<LifecycleObject, 5> b1;
        b1.Push(LifecycleObject(&stats, 1)); // Active: 1

        Core::RingBuffer<LifecycleObject, 5> b2;
        b2.Push(LifecycleObject(&stats, 99)); // Active: 2

        // Move Assignment
        b2 = std::move(b1);

        // Expected:
        // - b2's original item (99) is destroyed.
        // - b1's item (1) is moved to b2.
        // - b1 becomes empty.

        EXPECT_EQ(b2.Size(), 1);
        EXPECT_EQ(b2.Peek().Id, 1);
        EXPECT_TRUE(b1.Empty());
    }
    // Clean up verification
    EXPECT_EQ(stats.Active(), 0);
}

TEST_F(RingBufferTests, ClearMethodDestroysItems)
{
    LifecycleStats stats;
    Core::RingBuffer<LifecycleObject, 5> buffer;

    buffer.Push(LifecycleObject(&stats, 1));
    buffer.Push(LifecycleObject(&stats, 2));
    buffer.Push(LifecycleObject(&stats, 3));

    EXPECT_EQ(stats.Active(), 3);

    buffer.Clear();

    EXPECT_TRUE(buffer.Empty());
    EXPECT_EQ(stats.Active(), 0); // All items should be destroyed
}
