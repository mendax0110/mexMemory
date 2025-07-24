#include <memory/refCounting/allocationMap.h>
#include <gtest/gtest.h>
#include <sstream>
#include <thread>

using namespace memory::refCounting;

class AllocationMapTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        AllocationTracker::enableTracking(true);
        AllocationTracker::setBreakOnLeak(false);
        testStream_.str("");
        AllocationTracker::setLeakStream(&testStream_);
    }

    void TearDown() override
    {
        AllocationTracker::enableTracking(false);
        AllocationTracker::setLeakStream(&std::cerr);
        AllocationTracker::clearAllocations();
    }

    std::stringstream testStream_;
};


TEST_F(AllocationMapTest, BasicTracking)
{
    int* intPtr = new int(42);
    TRACK_ALLOC(intPtr);

    EXPECT_EQ(AllocationTracker::getAllocationCount(), 1);
    EXPECT_EQ(AllocationTracker::getTotalAllocatedBytes(), sizeof(int));

    UNTRACK_ALLOC(intPtr);
    delete intPtr;

    EXPECT_EQ(AllocationTracker::getAllocationCount(), 0);
    EXPECT_EQ(AllocationTracker::getTotalAllocatedBytes(), 0);
}

TEST_F(AllocationMapTest, MultipleAllocations)
{
    const size_t count = 5;
    int* array[count];

    for (size_t i = 0; i < count; ++i) {
        array[i] = new int(i);
        TRACK_ALLOC(array[i]);
    }

    EXPECT_EQ(AllocationTracker::getAllocationCount(), count);
    EXPECT_EQ(AllocationTracker::getTotalAllocatedBytes(), count * sizeof(int));

    for (auto & i : array)
    {
        UNTRACK_ALLOC(i);
        delete i;
    }

    EXPECT_EQ(AllocationTracker::getAllocationCount(), 0);
}

TEST_F(AllocationMapTest, SourceLocationTracking)
{
    const char* testFile = "test_file.cpp";
    const int testLine = 123;

    auto* dblPtr = new double(3.14);
    AllocationTracker::trackAllocation(dblPtr, 1, testFile, testLine);

    auto leaks = AllocationTracker::checkLeaks();
    EXPECT_EQ(leaks, 1);

    std::string output = testStream_.str();
    EXPECT_NE(output.find(testFile), std::string::npos);
    EXPECT_NE(output.find(std::to_string(testLine)), std::string::npos);

    UNTRACK_ALLOC(dblPtr);
    delete dblPtr;
}

TEST_F(AllocationMapTest, ThreadSafety)
{
    const int threadCount = 10;
    const int allocsPerThread = 100;

    std::vector<std::thread> threads;
    std::vector<void*> pointers;
    pointers.reserve(threadCount * allocsPerThread);

    for (int i = 0; i < threadCount; ++i)
    {
        threads.emplace_back([&]() {
            for (int j = 0; j < allocsPerThread; ++j)
            {
                auto ptr = new int(j);
                TRACK_ALLOC(ptr);
                pointers.push_back(ptr);
            }
        });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    EXPECT_EQ(AllocationTracker::getAllocationCount(), threadCount * allocsPerThread);

    for (auto ptr : pointers)
    {
        UNTRACK_ALLOC(ptr);
        delete static_cast<int*>(ptr);
    }
}

TEST_F(AllocationMapTest, LeakDetectionOutput)
{
    float* floatPtr = new float(1.23f);
    TRACK_ALLOC(floatPtr);

    AllocationTracker::checkLeaks();
    std::string output = testStream_.str();

    EXPECT_NE(output.find("MEMORY LEAKS DETECTION REPORT"), std::string::npos);
    EXPECT_NE(output.find("Pointer"), std::string::npos);
    EXPECT_NE(output.find("Size"), std::string::npos);
    EXPECT_NE(output.find("Type"), std::string::npos);
    EXPECT_NE(output.find(std::to_string(sizeof(float))), std::string::npos);

    UNTRACK_ALLOC(floatPtr);
    delete floatPtr;
}

TEST_F(AllocationMapTest, BreakOnLeak)
{
    AllocationTracker::setBreakOnLeak(true);

    struct TestStruct { int a; double b; };
    TestStruct* structPtr = new TestStruct{1, 2.0};
    TRACK_ALLOC(structPtr);

    try
    {
        AllocationTracker::checkLeaks();
        FAIL() << "Expected std::runtime_error";
    }
    catch (const std::runtime_error& e)
    {
        EXPECT_NE(std::string(e.what()).find("Memory leak detected"), std::string::npos);
    }
    catch (...)
    {
        FAIL() << "Expected std::runtime_error";
    }

    UNTRACK_ALLOC(structPtr);
    delete structPtr;
}

TEST_F(AllocationMapTest, UntrackInvalidPointer)
{
    int dummy;
    UNTRACK_ALLOC(&dummy);

    UNTRACK_ALLOC(nullptr);
}

TEST_F(AllocationMapTest, TrackSamePointerTwice)
{
    int* ptr = new int(5);
    TRACK_ALLOC(ptr);

    TRACK_ALLOC(ptr);

    EXPECT_EQ(AllocationTracker::getAllocationCount(), 1);

    UNTRACK_ALLOC(ptr);
    delete ptr;
}

TEST_F(AllocationMapTest, DifferentTypesTracking)
{
    auto intPtr = new int(10);
    auto doublePtr = new double(3.14);
    auto charPtr = new char('A');

    TRACK_ALLOC(intPtr);
    TRACK_ALLOC(doublePtr);
    TRACK_ALLOC(charPtr);

    EXPECT_EQ(AllocationTracker::getAllocationCount(), 3);
    EXPECT_EQ(AllocationTracker::getTotalAllocatedBytes(),
              sizeof(int) + sizeof(double) + sizeof(char));

    AllocationTracker::checkLeaks();
    std::string output = testStream_.str();

    EXPECT_NE(output.find("int"), std::string::npos);
    EXPECT_NE(output.find("double"), std::string::npos);
    EXPECT_NE(output.find("char"), std::string::npos);

    UNTRACK_ALLOC(intPtr);
    UNTRACK_ALLOC(doublePtr);
    UNTRACK_ALLOC(charPtr);
    delete intPtr;
    delete doublePtr;
    delete charPtr;
}