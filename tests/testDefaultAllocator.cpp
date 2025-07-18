#include <gtest/gtest.h>
#include "memory/memory.h"

using namespace memory;

struct TestObject
{
    int value;
    explicit TestObject(int v) : value(v) {}
};

TEST(DefaultAllocatorTest, AllocateSingleObject)
{
    auto* obj = refCounting::DefaultAllocator<TestObject>::allocate(42);
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->value, 42);
    refCounting::DefaultAllocator<TestObject>::deallocate(obj);
}

TEST(DefaultAllocatorTest, DeallocateNullptr)
{
    refCounting::DefaultAllocator<TestObject>::deallocate(nullptr);
    refCounting::DefaultAllocator<TestObject[]>::deallocate(nullptr);
}

TEST(DefaultAllocatorTest, AllocateWithMultipleArgs)
{
    struct MultiArgObj
    {
        int a;
        double b;
        std::string c;
        MultiArgObj(int x, double y, std::string z) : a(x), b(y), c(std::move(z)) {}
    };

    auto* obj = refCounting::DefaultAllocator<MultiArgObj>::allocate(10, 3.14, "test");
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->a, 10);
    EXPECT_DOUBLE_EQ(obj->b, 3.14);
    EXPECT_EQ(obj->c, "test");
    refCounting::DefaultAllocator<MultiArgObj>::deallocate(obj);
}

TEST(DefaultAllocatorTest, ArrayAllocationSize)
{
    const size_t size = 100;
    auto* arr = refCounting::DefaultAllocator<int[]>::allocate(size);
    ASSERT_NE(arr, nullptr);

    for (size_t i = 0; i < size; ++i)
    {
        arr[i] = static_cast<int>(i);
    }

    for (size_t i = 0; i < size; ++i)
    {
        EXPECT_EQ(arr[i], static_cast<int>(i));
    }

    refCounting::DefaultAllocator<int[]>::deallocate(arr);
}