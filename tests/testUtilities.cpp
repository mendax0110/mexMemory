#include <gtest/gtest.h>
#include "memory/memory.h"
#include <memory>

using namespace memory;

struct TestObject
{
    int value{0};
};

class UtilitiesTest : public ::testing::Test
{
protected:

    void SetUp() override
    {
        enableReferenceDebugging(false);
    }
};

TEST_F(UtilitiesTest, DebuggingEnableDisable)
{
    enableReferenceDebugging(true);
    auto ref1 = makeRef<TestObject>(42);

    enableReferenceDebugging(false);
    auto ref2 = makeRef<TestObject>(24);
}