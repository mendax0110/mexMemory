#include <gtest/gtest.h>
#include <memory/memory.h>
#include <thread>
#include <iostream>

using namespace memory;

class MemoryTest : public ::testing::Test
{
protected:

    void SetUp() override
    {
        enableAllocationTracking(true);
        AllocationTracker::clearAllocations();
    }

    void TearDown() override
    {
        EXPECT_EQ(AllocationTracker::checkLeaks(), 0);
    }
};

TEST_F(MemoryTest, BasicRefOperations)
{
    {
        auto ref = makeRef<int>(42);
        EXPECT_EQ(*ref, 42);
        EXPECT_EQ(ref.useCount(), 1);
        EXPECT_TRUE(ref.isValid());

        *ref = 100;
        EXPECT_EQ(*ref, 100);

        auto ptr = ref.get();
        EXPECT_NE(ptr, nullptr);
        EXPECT_EQ(*ptr, 100);
    }
}

TEST_F(MemoryTest, ReferenceCounting)
{
    auto ref1 = makeRef<std::string>("Hello");
    EXPECT_EQ(ref1.useCount(), 1);

    {
        auto ref2 = ref1;
        EXPECT_EQ(ref1.useCount(), 2);
        EXPECT_EQ(ref2.useCount(), 2);
        EXPECT_EQ(*ref1, "Hello");
        EXPECT_EQ(*ref2, "Hello");

        *ref2 = "World";
        EXPECT_EQ(*ref1, "World");
    }

    EXPECT_EQ(ref1.useCount(), 1);
}

TEST_F(MemoryTest, MoveSemantics)
{
    auto ref1 = makeRef<int>(100);
    auto ptr = ref1.get();

    auto ref2 = std::move(ref1);
    EXPECT_EQ(ref1.get(), nullptr);
    EXPECT_EQ(ref2.get(), ptr);
    EXPECT_EQ(*ref2, 100);

    ref1 = std::move(ref2);
    EXPECT_EQ(ref2.get(), nullptr);
    EXPECT_EQ(ref1.get(), ptr);
}

TEST_F(MemoryTest, WeakReference)
{
    WeakRef<std::string> weakRef;

    {
        auto strongRef = makeRef<std::string>("Test");
        weakRef = strongRef.weak();

        EXPECT_FALSE(weakRef.expired());
        EXPECT_TRUE(weakRef.canLock());

        auto locked = weakRef.lock();
        EXPECT_EQ(*locked, "Test");
        EXPECT_EQ(strongRef.useCount(), 2);
    }

    EXPECT_TRUE(weakRef.expired());
    EXPECT_FALSE(weakRef.canLock());
    EXPECT_EQ(weakRef.lock().get(), nullptr);
}

TEST_F(MemoryTest, ThreadSafety)
{
    const int numThreads = 10;
    const int numIterations = 1000;

    auto sharedRef = makeRef<std::atomic<int>>(0);

    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i)
    {
        threads.emplace_back([&]() {
            for (int j = 0; j < numIterations; ++j)
            {
                auto localRef = sharedRef;
                (*localRef)++;
            }
        });
    }

    for (auto& thread : threads)
    {
        thread.join();
    }

    EXPECT_EQ(*sharedRef, numThreads * numIterations);
    EXPECT_EQ(sharedRef.useCount(), 1);
}

struct CustomAllocator
{
    static int* allocate()
    {
        auto ptr = new int(0);
        *ptr = 42;
        return ptr;
    }

    static void deallocate(int* ptr) { delete ptr; }
};

TEST_F(MemoryTest, CustomAllocator)
{
    auto ref = makeRefWithAllocator<int, CustomAllocator>();
    EXPECT_EQ(*ref, 42);

    EXPECT_EQ(AllocationTracker::getAllocationCount(), 1);
    EXPECT_EQ(AllocationTracker::getTotalAllocatedBytes(), sizeof(int));
}

class Base
{
public:

    virtual ~Base() = default;
    virtual int value() const { return 1; }
};

class Derived : public Base
{
public:

    int value() const override { return 2; }
};

/*TEST_F(MemoryTest, DISABLED_Polymorphism)
{
    Ref<Base> baseRef = makeRef<Derived>();
    EXPECT_EQ(baseRef->value(), 2);

    WeakRef<Base> weakBase = baseRef.weak();
    auto locked = weakBase.lock();
    EXPECT_EQ(locked->value(), 2);
}*/

TEST_F(MemoryTest, AllocationTracking)
{
    {
        auto ref1 = makeRef<double>(3.14);
        auto ref2 = makeRef<char>('A');

        EXPECT_EQ(AllocationTracker::getAllocationCount(), 2);
        EXPECT_EQ(AllocationTracker::getTotalAllocatedBytes(), sizeof(double) + sizeof(char));
    }

    EXPECT_EQ(AllocationTracker::getAllocationCount(), 0);
}

TEST_F(MemoryTest, InvalidOperations)
{
    Ref<int> nullRef;
    EXPECT_FALSE(nullRef.isValid());
    EXPECT_EQ(nullRef.get(), nullptr);

    WeakRef<int> nullWeak;
    EXPECT_TRUE(nullWeak.expired());

    EXPECT_THROW(*nullRef = 42, std::runtime_error);
}

TEST_F(MemoryTest, ControlBlockManagement)
{
    WeakRef<int> weak;

    {
        auto strong = makeRef<int>(100);
        weak = strong.weak();

        EXPECT_EQ(weak.useCount(), 1);
        EXPECT_FALSE(weak.expired());
    }

    EXPECT_EQ(weak.useCount(), 0);
    EXPECT_TRUE(weak.expired());
}

TEST_F(MemoryTest, MultipleWeakReferences)
{
    WeakRef<int> weak1, weak2;

    {
        auto strong = makeRef<int>(200);
        weak1 = strong.weak();
        weak2 = strong.weak();

        EXPECT_EQ(weak1.useCount(), 1);
        EXPECT_EQ(weak2.useCount(), 1);

        auto locked1 = weak1.lock();
        auto locked2 = weak2.lock();

        EXPECT_EQ(*locked1, 200);
        EXPECT_EQ(*locked2, 200);
        EXPECT_EQ(strong.useCount(), 3);
    }

    EXPECT_TRUE(weak1.expired());
    EXPECT_TRUE(weak2.expired());
}

TEST_F(MemoryTest, SelfAssigment)
{
    auto ref = makeRef<int>(300);
    ref = ref;

    EXPECT_EQ(*ref, 300);
    EXPECT_EQ(ref.useCount(), 1);

    WeakRef<int> weak = ref.weak();
    weak = weak;

    EXPECT_FALSE(weak.expired());
}

TEST_F(MemoryTest, MoveToSelf)
{
    auto ref = makeRef<int>(400);
    ref = std::move(ref);

    EXPECT_EQ(*ref, 400);
    EXPECT_EQ(ref.useCount(), 1);
}

TEST_F(MemoryTest, AllocationMapTracking)
{
    struct TrackedStruct
    {
        int a;
        double b;
    };

    auto ref = makeRef<TrackedStruct>(TrackedStruct{1, 2.0});

    auto allocations = AllocationTracker::getAllocations();
    EXPECT_EQ(allocations.size(), 1);

    for (const auto& [ptr, info] : allocations)
    {
        EXPECT_EQ(info.size, sizeof(TrackedStruct));
        EXPECT_EQ(info.type, AllocationTracker::demangleTypeName<TrackedStruct>());
    }

    AllocationTracker::untrackAllocation(ref.get());
    EXPECT_EQ(AllocationTracker::getAllocationCount(), 0);

    AllocationTracker::trackAllocation(ref.get(), 1, __FILE__, __LINE__);
}

TEST_F(MemoryTest, ExceptionSafety)
{
    try
    {
        auto ref = makeRef<int>(500);
        throw std::runtime_error("Test exception");
    }
    catch (...)
    {
        EXPECT_EQ(AllocationTracker::getAllocationCount(), 0);
    }
}