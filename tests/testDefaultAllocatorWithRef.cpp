#include <gtest/gtest.h>
#include "memory/memory.h"

using namespace memory;

struct TestObject
{
    int value;
    TestObject(int v) : value(v) {}
};


class DefaultAllocatorWithRefTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        enableReferenceDebugging(false);
    }
};

TEST_F(DefaultAllocatorWithRefTest, MakeRefUsesDefaultAllocator)
{
    auto ref = makeRef<TestObject>(42);
    ASSERT_NE(ref.get(), nullptr);
    EXPECT_EQ(ref->value, 42);
    EXPECT_EQ(ref.useCount(), 1);
}

TEST_F(DefaultAllocatorWithRefTest, ArrayAllocationWithRef)
{
    const size_t size = 3;
    auto ref = makeRef<TestObject[]>(size);
    ASSERT_NE(ref.get(), nullptr);

    for (size_t i = 0; i < size; ++i)
    {
        ref.get()[i] = TestObject(static_cast<int>(i));
    }

    for (size_t i = 0; i < size; ++i)
    {
        EXPECT_EQ(ref.get()[i].value, static_cast<int>(i));
    }

    EXPECT_EQ(ref.useCount(), 1);
}

TEST_F(DefaultAllocatorWithRefTest, CustomTypeAllocation)
{
    struct CustomType
    {
        std::string name;
        int id;
        CustomType(std::string n, int i) : name(std::move(n)), id(i) {}
    };

    auto ref = makeRef<CustomType>("test", 123);
    ASSERT_NE(ref.get(), nullptr);
    EXPECT_EQ(ref->name, "test");
    EXPECT_EQ(ref->id, 123);
}