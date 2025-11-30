#include <gtest/gtest.h>

#include "Collections/CyclicBuffer.h"

class LoopBufferTests : public testing::Test
{
protected:
    LoopBufferTests() = default;
};

TEST_F(LoopBufferTests, AdvanceRight)
{
    Core::CyclicBuffer<int32_t, 3> buffer{1, 2, 3};

    buffer.AdvanceRight();

    EXPECT_EQ(buffer[0], 3);
    EXPECT_EQ(buffer[1], 1);
    EXPECT_EQ(buffer[2], 2);

    buffer.AdvanceRight();

    EXPECT_EQ(buffer[0], 2);
    EXPECT_EQ(buffer[1], 3);
    EXPECT_EQ(buffer[2], 1);
}

TEST_F(LoopBufferTests, AdvanceLeft)
{
    Core::CyclicBuffer<int32_t, 3> buffer{1, 2, 3};

    buffer.AdvanceLeft();

    EXPECT_EQ(buffer[0], 2);
    EXPECT_EQ(buffer[1], 3);
    EXPECT_EQ(buffer[2], 1);

    buffer.AdvanceLeft();

    EXPECT_EQ(buffer[0], 3);
    EXPECT_EQ(buffer[1], 1);
    EXPECT_EQ(buffer[2], 2);
}

TEST_F(LoopBufferTests, Iterator)
{
    Core::CyclicBuffer<int32_t, 3> buffer{1, 2, 3};

    int32_t expectedValue = 1;
    for (const int32_t& value : buffer)
    {
        EXPECT_EQ(value, expectedValue);
        ++expectedValue;
    }

    Core::CyclicBuffer<int32_t, 3> constBuffer{1, 2, 3};
    expectedValue = 1;
    for (const int32_t& value : constBuffer)
    {
        EXPECT_EQ(value, expectedValue);
        ++expectedValue;
    }
}

TEST_F(LoopBufferTests, DefaultConstructor)
{
    Core::CyclicBuffer<int32_t, 3> buffer;

    EXPECT_EQ(buffer[0], 0);
    EXPECT_EQ(buffer[1], 0);
    EXPECT_EQ(buffer[2], 0);
}

TEST_F(LoopBufferTests, InitializerListConstructor)
{
    Core::CyclicBuffer<int32_t, 5> buffer{1, 2, 3};

    EXPECT_EQ(buffer[0], 1);
    EXPECT_EQ(buffer[1], 2);
    EXPECT_EQ(buffer[2], 3);
    EXPECT_EQ(buffer[3], 0);
    EXPECT_EQ(buffer[4], 0);
}

TEST_F(LoopBufferTests, Size)
{
    Core::CyclicBuffer<int32_t, 3> buffer;

    EXPECT_EQ(buffer.Size(), 3);
}

TEST_F(LoopBufferTests, Data)
{
    Core::CyclicBuffer<int32_t, 3> buffer{1, 2, 3};

    int32_t* data = buffer.Data();
    EXPECT_EQ(data[0], 1);
    EXPECT_EQ(data[1], 2);
    EXPECT_EQ(data[2], 3);
}

TEST_F(LoopBufferTests, ConstData)
{
    const Core::CyclicBuffer<int32_t, 3> buffer{1, 2, 3};

    const int32_t* data = buffer.Data();
    EXPECT_EQ(data[0], 1);
    EXPECT_EQ(data[1], 2);
    EXPECT_EQ(data[2], 3);
}

TEST_F(LoopBufferTests, OperatorBracketWithIterator)
{
    Core::CyclicBuffer<int32_t, 3> buffer{1, 2, 3};

    auto it = buffer.begin();
    EXPECT_EQ(buffer[it], 1);
    ++it;
    EXPECT_EQ(buffer[it], 2);
}

TEST_F(LoopBufferTests, ConstOperatorBracketWithIterator)
{
    const Core::CyclicBuffer<int32_t, 3> buffer{1, 2, 3};

    auto it = buffer.begin();
    EXPECT_EQ(buffer[it], 1);
    ++it;
    EXPECT_EQ(buffer[it], 2);
}

TEST_F(LoopBufferTests, IteratorDecrement)
{
    Core::CyclicBuffer<int32_t, 3> buffer{1, 2, 3};

    auto it = buffer.end();
    --it;
    EXPECT_EQ(*it, 3);
    --it;
    EXPECT_EQ(*it, 2);
}

TEST_F(LoopBufferTests, IteratorPostDecrement)
{
    Core::CyclicBuffer<int32_t, 3> buffer{1, 2, 3};

    auto it = buffer.end();
    auto prev = it--;
    EXPECT_EQ(*it, 3);
    EXPECT_EQ(prev, buffer.end());
}

TEST_F(LoopBufferTests, IteratorArrow)
{
    struct TestStruct
    {
        int32_t value;
    };

    Core::CyclicBuffer<TestStruct, 3> buffer{{1}, {2}, {3}};

    auto it = buffer.begin();
    EXPECT_EQ(it->value, 1);
}

TEST_F(LoopBufferTests, ConstIterator)
{
    const Core::CyclicBuffer<int32_t, 3> buffer{1, 2, 3};

    auto it = buffer.cbegin();
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 2);
}

TEST_F(LoopBufferTests, MultipleAdvanceOperations)
{
    Core::CyclicBuffer<int32_t, 3> buffer{1, 2, 3};

    buffer.AdvanceRight();
    buffer.AdvanceLeft();

    EXPECT_EQ(buffer[0], 1);
    EXPECT_EQ(buffer[1], 2);
    EXPECT_EQ(buffer[2], 3);
}
