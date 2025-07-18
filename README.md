# mexMemory - Reference Counting Memory Management Library

A C++ library implementing reference counting memory management with support for strong and weak references.

## Features

- Thread-safe reference counting using atomic operations
- Strong references (`Ref`) that manage object lifetime
- Weak references (`WeakRef`) that don't affect object lifetime
- Custom allocator support
- Debug logging capabilities
- Comprehensive test suite

## Requirements

- C++23 compatible compiler
- CMake 3.14 or higher

## Building

```bash
mkdir build
cd build
cmake ..
cmake --build .