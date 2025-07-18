#include <gtest/gtest.h>
#include "memory/memory.h"

using namespace memory;

struct TestObject
{
    int value{0};
    ~TestObject() { value = -1; }
};

class StrongRefTest : public ::testing::Test
{
protected:

    void SetUp() override
    {
        enableReferenceDebugging(false);
    }
};

TEST_F(StrongRefTest, DefaultConstructor)
{
    Ref<TestObject> ref;
    EXPECT_FALSE(ref);
    EXPECT_EQ(ref.useCount(), 0);
}

TEST_F(StrongRefTest, ObjectConstructor)
{
    auto* obj = new TestObject{42};
    Ref<TestObject> ref(obj);

    EXPECT_TRUE(ref);
    EXPECT_EQ(ref->value, 42);
    EXPECT_EQ(ref.useCount(), 1);
}

TEST_F(StrongRefTest, CopyConstructor)
{
    auto ref1 = makeRef<TestObject>(42);
    Ref<TestObject> ref2(ref1);

    EXPECT_EQ(ref1.useCount(), 2);
    EXPECT_EQ(ref2.useCount(), 2);
    EXPECT_EQ(ref1.get(), ref2.get());
}

TEST_F(StrongRefTest, MoveConstructor)
{
    auto ref1 = makeRef<TestObject>(42);
    auto* ptr = ref1.get();
    Ref<TestObject> ref2(std::move(ref1));

    EXPECT_FALSE(ref1);
    EXPECT_TRUE(ref2);
    EXPECT_EQ(ref2.get(), ptr);
    EXPECT_EQ(ref2.useCount(), 1);
}

TEST_F(StrongRefTest, CopyAssignment)
{
    auto ref1 = makeRef<TestObject>(42);
    Ref<TestObject> ref2;
    ref2 = ref1;

    EXPECT_EQ(ref1.useCount(), 2);
    EXPECT_EQ(ref2.useCount(), 2);
}

TEST_F(StrongRefTest, MoveAssignment)
{
    auto ref1 = makeRef<TestObject>(42);
    auto* ptr = ref1.get();
    Ref<TestObject> ref2;
    ref2 = std::move(ref1);

    EXPECT_FALSE(ref1);
    EXPECT_TRUE(ref2);
    EXPECT_EQ(ref2.get(), ptr);
    EXPECT_EQ(ref2.useCount(), 1);
}

TEST_F(StrongRefTest, MakeRef)
{
    auto ref = makeRef<TestObject>(42);

    EXPECT_TRUE(ref);
    EXPECT_EQ(ref->value, 42);
    EXPECT_EQ(ref.useCount(), 1);
}

TEST_F(StrongRefTest, Reset)
{
    auto ref = makeRef<TestObject>(42);
    ref.reset();

    EXPECT_FALSE(ref);
    EXPECT_EQ(ref.useCount(), 0);
}

TEST_F(StrongRefTest, WeakReferenceCreation)
{
    auto ref = makeRef<TestObject>(42);
    auto weak = ref.weak();

    EXPECT_EQ(ref.useCount(), 1);
    EXPECT_TRUE(weak.canLock());
}
