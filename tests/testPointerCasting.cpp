#include <gtest/gtest.h>
#include <memory/refCounting/referenceCasting.h>
#include "memory/refCounting/utilities.h"

using namespace memory::refCounting;

class PointerCastingTest : public ::testing::Test
{
protected:

    void SetUp() override
    {
        enableAllocationTracking(true);
    }

    void TearDown() override
    {
        EXPECT_EQ(AllocationTracker::checkLeaks(), 0);
    }
};

TEST_F(PointerCastingTest, SimpleStaticCast)
{
    struct Base { virtual ~Base() = default; };
    struct Derived : Base {};

    Ref<Derived> derivedRef = Ref<Derived>(new Derived());
    Ref<Base> baseRef = static_pointer_cast<Derived>(derivedRef);
    EXPECT_TRUE(baseRef.isValid());
    EXPECT_EQ(baseRef.useCount(), 2);
}

TEST_F(PointerCastingTest, ComplexTypeCast)
{
    struct Base
    {
        virtual ~Base() = default;
        virtual std::string foo()
        {
            return "";
        }
    };

    struct Derived : Base
    {
        std::string foo() override
        {
            auto msg = "Derived foo called";
            return msg;
        }
    };

    Ref<Derived> derivedRef = Ref<Derived>(new Derived());
    Ref<Base> baseRef = static_pointer_cast<Derived>(derivedRef);

    EXPECT_STREQ(baseRef->foo().c_str(), "Derived foo called");
    EXPECT_TRUE(baseRef.isValid());
    EXPECT_EQ(baseRef.useCount(), 2);
    EXPECT_TRUE(derivedRef.isValid());
    EXPECT_EQ(derivedRef.useCount(), 2);
}

/*TEST_F(PointerCastingTest, DISABLED_DynamicCast)
{
    struct Base { virtual ~Base() = default; };
    struct Derived : Base {};

    Ref<Base> baseRef = Ref<Base>(new Derived());
    Ref<Derived> derivedRef = dynamic_pointer_cast<Derived>(baseRef);

    EXPECT_TRUE(derivedRef.isValid());
    EXPECT_EQ(derivedRef.useCount(), 2);
}*/

/*TEST_F(PointerCastingTest, ConstCast)
{
    struct Base { virtual ~Base() = default; };
    struct Derived : Base {};

    Ref<Derived> derivedRef = Ref<Derived>(new Derived());
    Ref<const Base> constBaseRef = const_pointer_cast<Derived>(derivedRef);

    EXPECT_TRUE(constBaseRef.isValid());
    EXPECT_EQ(constBaseRef.useCount(), 2);
}*/