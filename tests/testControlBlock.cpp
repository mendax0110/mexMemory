#include <gtest/gtest.h>
#include <memory/refCounting/controlBlock.h>

using namespace memory::refCounting;

struct TestObject
{
    int value{0};
};

class ControlBlockTest : public ::testing::Test
{
protected:

    void SetUp() override
    {
        DebugConfig::enableLogging = false;
    }
};

TEST_F(ControlBlockTest, ConstructorInitializesCounts)
{
    auto* obj = new TestObject();
    ControlBlock<TestObject> block(obj);

    EXPECT_EQ(block.strongCount(), 1);
    EXPECT_EQ(block.weakCount(), 0);
    EXPECT_TRUE(block.hadObject());
    EXPECT_EQ(block.get(), obj);
}

TEST_F(ControlBlockTest, StrongReferenceCounting)
{
    auto* obj = new TestObject;
    auto* block = new ControlBlock<TestObject>(obj);

    block->incrementStrong();
    EXPECT_EQ(block->strongCount(), 2);

    block->decrementStrong();
    EXPECT_EQ(block->strongCount(), 1);

    block->decrementStrong();
}

TEST_F(ControlBlockTest, WeakReferenceCounting)
{
    auto* obj = new TestObject;
    auto* block = new ControlBlock<TestObject>(obj);

    block->incrementWeak();
    EXPECT_EQ(block->weakCount(), 1);

    block->decrementWeak();
    EXPECT_EQ(block->weakCount(), 0);

    block->decrementStrong();
}

TEST_F(ControlBlockTest, ObjectDeletion)
{
    auto* obj = new TestObject;
    auto* block = new ControlBlock<TestObject>(obj);

    block->decrementStrong();
}