#ifndef MEXMEMORY_FORWARDDECL_H
#define MEXMEMORY_FORWARDDECL_H

/// @brief Namespace for memory management with reference counting \namespace memory::refCounting
namespace memory::refCounting
{
    /**
     * @brief Forward declaration of WeakRef class.
     * @tparam T The type of object being referenced.
     * @tparam Allocator The allocator to use for memory management.
     */
    template <typename T, typename Allocator>
    class WeakRef;

    /**
     * @brief Forward declaration of Ref class.
     * @tparam T The type of object being referenced.
     * @tparam Allocator The allocator to use for memory management.
     */
    template <typename T, typename Allocator>
    class Ref;
}

#endif //MEXMEMORY_FORWARDDECL_H