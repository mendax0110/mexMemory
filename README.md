# mexMemory - Reference Counting Memory Management Library

A C++ library implementing reference counting memory management with support for strong and weak references.

## Features

### Core Features
- Thread-safe reference counting using atomic operations
- Strong references (`Ref`) that manage object lifetime
- Weak references (`WeakRef`) that don't affect object lifetime
- Custom allocator support
- Debug logging capabilities
- Comprehensive test suite

### Enhanced Features (New)
- **Reference Comparison Operators**: Full set of comparison operators (==, !=, <, >, <=, >=) for use in containers and algorithms
- **Enhanced Pointer Casting**: Support for static_pointer_cast, dynamic_pointer_cast, and const_pointer_cast
- **Memory Statistics**: Detailed memory usage analysis including allocation tracking by type, size statistics, and leak detection
- **Circular Reference Detection**: Basic infrastructure for detecting potential memory leaks from circular references
- **std::shared_ptr Interoperability**: Conversion functions and dual reference objects for compatibility with standard library
- **Advanced Allocation Tracking**: Enhanced memory tracking with detailed statistics and type-based filtering

## Requirements

- C++23 compatible compiler
- CMake 3.14 or higher

## Building

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Basic Usage

```cpp
#include "memory/memory.h"
using namespace memory;

// Create a reference-counted object
auto ref = makeRef<int>(42);

// Copy references safely
auto ref2 = ref;
assert(ref.useCount() == 2);

// Create weak references
WeakRef<int> weak = ref.weak();

// Use comparison operators
assert(ref == ref2);
assert(ref != nullptr);

// Enable memory tracking
enableAllocationTracking(true);
AllocationTracker::printStatistics();

// Enable cycle detection
enableCycleDetection(true);
```

## Advanced Features

### Memory Statistics
```cpp
// Get detailed memory usage statistics
auto stats = AllocationTracker::getStatistics();
std::cout << "Total allocations: " << stats.total_allocations << std::endl;
std::cout << "Total bytes: " << stats.total_bytes << std::endl;
std::cout << "Average allocation size: " << stats.average_allocation_size << std::endl;

// Print comprehensive statistics
AllocationTracker::printStatistics();

// Get allocations by type
auto intAllocations = AllocationTracker::getAllocationsByType("int");
```

### Pointer Casting
```cpp
// Static casting (compile-time checked)
auto baseRef = static_pointer_cast<Base>(derivedRef);

// Dynamic casting (runtime checked for polymorphic types)
auto derivedRef = dynamic_pointer_cast<Derived>(baseRef);

// Const casting
auto constRef = const_pointer_cast<const int>(intRef);
```

### std::shared_ptr Integration
```cpp
// Convert mexMemory Ref to std::shared_ptr
auto mexRef = makeRef<int>(42);
auto sharedPtr = to_shared_ptr(mexRef);

// Create objects with dual reference counting
auto dualRef = makeDualRef<int>(100);
auto stdPtr = dualRef.getSharedPtr();
auto mexPtr = dualRef.getRef();
```

### Circular Reference Detection
```cpp
// Enable cycle detection with custom callback
enableCycleDetection(true);
CycleDetector::setCycleCallback([](const CycleDetector::CycleInfo& info) {
    std::cout << "Cycle detected: " << info.description << std::endl;
});
```

## API Reference

### Core Classes
- `Ref<T>`: Strong reference type that manages object lifetime
- `WeakRef<T>`: Weak reference type that doesn't affect object lifetime
- `AllocationTracker`: Memory allocation tracking and leak detection
- `CycleDetector`: Circular reference detection infrastructure

### Utility Functions
- `makeRef<T>(args...)`: Create a reference-counted object
- `enableReferenceDebugging(bool)`: Enable/disable debug logging
- `enableAllocationTracking(bool)`: Enable/disable memory tracking
- `enableCycleDetection(bool)`: Enable/disable cycle detection

### Casting Functions
- `static_pointer_cast<U>(ref)`: Static cast between reference types
- `dynamic_pointer_cast<U>(ref)`: Dynamic cast with runtime type checking
- `const_pointer_cast<U>(ref)`: Const qualification casting

### Interoperability
- `to_shared_ptr(ref)`: Convert mexMemory Ref to std::shared_ptr
- `makeDualRef<T>(args...)`: Create objects with dual reference counting