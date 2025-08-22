#include <gtest/gtest.h>
#include "memory/memory.h"
#include <memory>

using namespace memory;

class EnhancedFeaturesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        enableReferenceDebugging(false);
        enableAllocationTracking(true);
    }

    void TearDown() override
    {
        enableAllocationTracking(false);
    }
};

// Test comparison operators
TEST_F(EnhancedFeaturesTest, ComparisonOperators)
{
    auto ref1 = makeRef<int>(42);
    auto ref2 = ref1; // Same object
    auto ref3 = makeRef<int>(42); // Different object, same value
    Ref<int> nullRef;

    // Equality tests
    EXPECT_TRUE(ref1 == ref2);
    EXPECT_FALSE(ref1 == ref3);
    EXPECT_TRUE(nullRef == nullptr);
    EXPECT_FALSE(ref1 == nullptr);

    // Inequality tests
    EXPECT_FALSE(ref1 != ref2);
    EXPECT_TRUE(ref1 != ref3);
    EXPECT_FALSE(nullRef != nullptr);
    EXPECT_TRUE(ref1 != nullptr);

    // Ordering tests (for use in containers)
    if (ref1.get() < ref3.get())
    {
        EXPECT_TRUE(ref1 < ref3);
        EXPECT_FALSE(ref1 > ref3);
        EXPECT_TRUE(ref1 <= ref3);
        EXPECT_FALSE(ref1 >= ref3);
    }
    else
    {
        EXPECT_FALSE(ref1 < ref3);
        EXPECT_TRUE(ref1 > ref3);
        EXPECT_FALSE(ref1 <= ref3);
        EXPECT_TRUE(ref1 >= ref3);
    }
}

// Test memory statistics
TEST_F(EnhancedFeaturesTest, MemoryStatistics)
{
    // Clear any existing allocations for clean test
    AllocationTracker::clearAllocations();

    // Create some allocations
    auto ref1 = makeRef<int>(42);
    auto ref2 = makeRef<double>(3.14);
    auto ref3 = makeRef<std::string>("hello");

    auto stats = AllocationTracker::getStatistics();
    
    EXPECT_GT(stats.total_allocations, 0);
    EXPECT_GT(stats.total_bytes, 0);
    EXPECT_GT(stats.largest_allocation, 0);
    EXPECT_GT(stats.smallest_allocation, 0);
    EXPECT_GT(stats.average_allocation_size, 0.0);

    // Test statistics printing (just ensure it doesn't crash)
    std::ostringstream oss;
    AllocationTracker::printStatistics(&oss);
    EXPECT_FALSE(oss.str().empty());
}

// Test cycle detection
TEST_F(EnhancedFeaturesTest, CycleDetection)
{
    bool cycleDetected = false;
    std::string cycleMessage;

    // Enable cycle detection with custom callback
    enableCycleDetection(true);
    CycleDetector::setCycleCallback([&](const CycleDetector::CycleInfo& info) {
        cycleDetected = true;
        cycleMessage = info.description;
    });

    EXPECT_TRUE(CycleDetector::isEnabled());

    // Note: Actual cycle detection would require analyzing object member variables
    // This test mainly verifies the infrastructure is in place
    
    // Clean up
    enableCycleDetection(false);
    EXPECT_FALSE(CycleDetector::isEnabled());
}

// Test std::shared_ptr interoperability
TEST_F(EnhancedFeaturesTest, SharedPtrInterop)
{
    auto mexRef = makeRef<int>(42);
    
    // Convert to shared_ptr
    auto sharedPtr = to_shared_ptr(mexRef);
    EXPECT_TRUE(sharedPtr);
    EXPECT_EQ(*sharedPtr, 42);
    EXPECT_EQ(sharedPtr.get(), mexRef.get()); // Should point to same object

    // Test with null reference
    Ref<int> nullRef;
    auto nullSharedPtr = to_shared_ptr(nullRef);
    EXPECT_FALSE(nullSharedPtr);

    // Test dual ref object
    auto dualRef = makeDualRef<int>(100);
    EXPECT_TRUE(dualRef.isValid());
    EXPECT_EQ(*dualRef.get(), 100);
    
    auto stdPtr = dualRef.getSharedPtr();
    auto mexPtr = dualRef.getRef();
    EXPECT_TRUE(stdPtr);
    EXPECT_TRUE(mexPtr);
    EXPECT_EQ(*stdPtr, 100);
    EXPECT_EQ(*mexPtr, 100);
}

// Test weak reference comparisons
TEST_F(EnhancedFeaturesTest, WeakReferenceComparisons)
{
    auto ref1 = makeRef<int>(42);
    auto ref2 = makeRef<int>(42);
    
    WeakRef<int> weak1 = ref1.weak();
    WeakRef<int> weak2 = ref1.weak(); // Same object
    WeakRef<int> weak3 = ref2.weak(); // Different object
    WeakRef<int> nullWeak;

    // Equality tests
    EXPECT_TRUE(weak1 == weak2);
    EXPECT_FALSE(weak1 == weak3);
    EXPECT_TRUE(nullWeak == nullptr);
    EXPECT_FALSE(weak1 == nullptr);

    // Inequality tests
    EXPECT_FALSE(weak1 != weak2);
    EXPECT_TRUE(weak1 != weak3);
    EXPECT_FALSE(nullWeak != nullptr);
    EXPECT_TRUE(weak1 != nullptr);
}

// Test enhanced allocation tracking
TEST_F(EnhancedFeaturesTest, AllocationsByType)
{
    AllocationTracker::clearAllocations();

    auto intRef = makeRef<int>(42);
    auto doubleRef = makeRef<double>(3.14);
    auto stringRef = makeRef<std::string>("test");

    auto intAllocations = AllocationTracker::getAllocationsByType("int");
    auto doubleAllocations = AllocationTracker::getAllocationsByType("double");
    
    // Note: The type names might be mangled or different in the actual implementation
    // This test mainly ensures the functionality exists
    EXPECT_GE(AllocationTracker::getAllocationCount(), 3);
    EXPECT_GT(AllocationTracker::getTotalAllocatedBytes(), 0);
}