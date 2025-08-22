cmake_minimum_required(VERSION 3.14)
project(mexMemory VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_library(mexMemory INTERFACE)
target_include_directories(mexMemory INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(googletest)

    add_executable(mexMemory_tests
            tests/testControlBlock.cpp
            tests/testStrongReference.cpp
            tests/testWeakReference.cpp
            tests/testUtilities.cpp
            tests/testIntegration.cpp
            tests/testDefaultAllocator.cpp
            tests/testDefaultAllocatorWithRef.cpp
            tests/testAllocationMap.cpp
            tests/testAllocRef.cpp
            tests/testPointerCasting.cpp
            tests/testEnhancedFeatures.cpp
    )

    target_link_libraries(mexMemory_tests
            #GTest::gtest
            GTest::gtest_main
            mexMemory
    )

    add_test(NAME ControlBlockTests COMMAND mexMemory_tests --gtest_filter=ControlBlockTest*)
    add_test(NAME StrongReferenceTests COMMAND mexMemory_tests --gtest_filter=StrongRefTest*)
    add_test(NAME WeakReferenceTests COMMAND mexMemory_tests --gtest_filter=WeakRefTest*)
    add_test(NAME UtilitiesTests COMMAND mexMemory_tests --gtest_filter=UtilitiesTest*)
    add_test(NAME IntegrationTests COMMAND mexMemory_tests --gtest_filter=IntegrationTest*)
    add_test(NAME DefaultAllocatorTests COMMAND mexMemory_tests --gtest_filter=DefaultAllocatorTest*)
    add_test(NAME DefaultAllocatorWithRefTests COMMAND mexMemory_tests --gtest_filter=DefaultAllocatorWithRefTest*)
    add_test(NAME AllocationMapTests COMMAND mexMemory_tests --gtest_filter=AllocationMapTest*)
    add_test(NAME AllocRefTests COMMAND mexMemory_tests --gtest_filter=MemoryTest*)
    add_test(NAME PointerCastingTests COMMAND mexMemory_tests --gtest_filter=PointerCastingTest*)
    add_test(NAME EnhancedFeaturesTests COMMAND mexMemory_tests --gtest_filter=EnhancedFeaturesTest*)
    add_test(NAME AllTests COMMAND mexMemory_tests)

    add_custom_target(run_tests ALL
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            DEPENDS mexMemory_tests
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Running all tests after build..."
    )
endif()
