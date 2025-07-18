#include <gtest/gtest.h>
#include "memory/memory.h"

using namespace memory;

struct TestObject
{
    int value{0};
};

class WeakRefTest : public ::testing::Test
{
protected:

    void SetUp() override
    {
        enableReferenceDebugging(false);
    }
};

TEST_F(WeakRefTest, DefaultConstructor)
{
    WeakRef<TestObject> ref;

    EXPECT_FALSE(ref.canLock());
    EXPECT_TRUE(ref.expired());
}

TEST_F(WeakRefTest, CreateFromStrongRef)
{
    auto strong = makeRef<TestObject>(42);
    WeakRef<TestObject> weak(strong);

    EXPECT_TRUE(weak.canLock());
    EXPECT_FALSE(weak.expired());
}

TEST_F(WeakRefTest, LockValid)
{
    auto strong = makeRef<TestObject>(42);
    WeakRef<TestObject> weak(strong);

    auto locked = weak.lock();
    EXPECT_TRUE(locked);
    EXPECT_EQ(locked->value, 42);
    EXPECT_EQ(strong.useCount(), 2);
}

TEST_F(WeakRefTest, LockExpired)
{
    WeakRef<TestObject> weak;
    {
        auto strong = makeRef<TestObject>(42);
        weak = strong.weak();
    }

    auto locked = weak.lock();
    EXPECT_FALSE(locked);
    EXPECT_TRUE(weak.expired());
}

TEST_F(WeakRefTest, CopyConstructor)
{
    auto strong = makeRef<TestObject>(42);
    WeakRef<TestObject> weak1(strong);
    WeakRef<TestObject> weak2(weak1);

    EXPECT_TRUE(weak1.canLock());
    EXPECT_TRUE(weak2.canLock());
}

TEST_F(WeakRefTest, MoveConstructor)
{
    auto strong = makeRef<TestObject>(42);
    WeakRef<TestObject> weak1(strong);
    WeakRef<TestObject> weak2(std::move(weak1));

    EXPECT_FALSE(weak1.canLock());
    EXPECT_TRUE(weak2.canLock());
}

TEST_F(WeakRefTest, CopyAssignment)
{
    auto strong = makeRef<TestObject>(42);
    WeakRef<TestObject> weak1(strong);
    WeakRef<TestObject> weak2;
    weak2 = weak1;

    EXPECT_TRUE(weak1.canLock());
    EXPECT_TRUE(weak2.canLock());
}

TEST_F(WeakRefTest, MoveAssignment)
{
    auto strong = makeRef<TestObject>(42);
    WeakRef<TestObject> weak1(strong);
    WeakRef<TestObject> weak2;
    weak2 = std::move(weak1);

    EXPECT_FALSE(weak1.canLock());
    EXPECT_TRUE(weak2.canLock());
}