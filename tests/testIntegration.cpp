#include <gtest/gtest.h>
#include "memory/memory.h"
#include <memory>

using namespace memory;

struct TestObject
{
    int value{0};
    ~TestObject() { value = -1; }
};

class IntegrationTest : public ::testing::Test
{
protected:

    void SetUp() override
    {
        enableReferenceDebugging(false);
    }
};

TEST_F(IntegrationTest, ComplexReferenceChain)
{
    auto ref1 = makeRef<TestObject>(1);
    auto ref2 = ref1;
    WeakRef<TestObject> weak = ref1.weak();

    EXPECT_EQ(ref1.useCount(), 2);
    EXPECT_TRUE(weak.canLock());

    ref1.reset();
    EXPECT_EQ(ref2.useCount(), 1);
    EXPECT_TRUE(weak.canLock());

    ref2.reset();
    EXPECT_FALSE(weak.canLock());
}

TEST_F(IntegrationTest, WeakReferenceExpiresWhenLastStrongReferenceReleased)
{
    auto ref1 = makeRef<TestObject>(1);
    WeakRef<TestObject> weak = ref1.weak();

    EXPECT_TRUE(weak.canLock());

    ref1.reset();
    EXPECT_FALSE(weak.canLock());
}