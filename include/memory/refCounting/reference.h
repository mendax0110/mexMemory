#ifndef MEXMEMORY_REFERENCE_H
#define MEXMEMORY_REFERENCE_H

#include "controlBlock.h"
#include "forwardDecl.h"
#include <utility>
#include <type_traits>

/// @brief Namespace for memory management with reference counting \namespace memory::refCounting
namespace memory::refCounting
{
    /**
     * @brief Base class for reference types, providing common functionality for strong and weak references.
     * @tparam T The type of object being referenced, can be a single object or an array.
     * @tparam Allocator The allocator to use for memory management, defaults to DefaultAllocator.
     */
    template <typename T, typename Allocator = DefaultAllocator<std::remove_extent_t<T>>>
    class ReferenceBase
    {
    protected:

        /**
         * @brief Alias for the element type of T, which can be a single object or an array.
         */
        using elementType = std::remove_extent_t<T>;
        using controlBlockType = ControlBlock<elementType, Allocator>;

        controlBlockType* controlBlock = nullptr;

        /**
         * @brief Default constructor for ReferenceBase.
         */
        ReferenceBase() noexcept = default;

        /**
         * @brief Constructs a ReferenceBase with a given control block.
         * @param cb The control block to associate with this reference.
         */
        explicit ReferenceBase(controlBlockType* cb) : controlBlock(cb) {}

        /**
         * @brief Swaps the control block with another ReferenceBase.
         * @param other The other ReferenceBase to swap with.
         */
        void swap(ReferenceBase& other) noexcept
        {
            std::swap(controlBlock, other.controlBlock);
        }

    public:

        /**
         * @brief Checks if the reference is valid, meaning it has a control block and the object exists.
         * @return A boolean indicating whether the reference is valid.
         */
        [[nodiscard]] bool isValid() const noexcept
        {
            return controlBlock and controlBlock->hadObject();
        }

        /**
         * @brief Operator overload to check if the reference is valid.
         * @return A boolean indicating whether the reference is valid.
         */
        [[nodiscard]] explicit operator bool() const noexcept
        {
            return isValid();
        }

        /**
         * @brief Gets the number of strong references to the object managed by this control block.
         * @return The number of strong references.
         */
        [[nodiscard]] size_t useCount() const noexcept
        {
            return controlBlock ? controlBlock->strongCount() : 0;
        }

        /**
         * @brief Resets the reference, decrementing the strong reference count and releasing the control block.
         */
        void reset() noexcept
        {
            if (controlBlock)
            {
                controlBlock->decrementStrong();
                controlBlock = nullptr;
            }
        }

        /**
         * @brief Gets the raw pointer to the object managed by this control block.
         * @return The raw pointer to the object, or nullptr if the control block is null.
         */
        [[nodiscard]] elementType* get() const noexcept
        {
            return controlBlock ? controlBlock->get() : nullptr;
        }

        /**
         * @brief Overloaded operator to access the object managed by this reference.
         * @return The raw pointer to the object managed by this reference.
         */
        [[nodiscard]] elementType* operator->() const noexcept
        {
            return get();
        }

        /**
         * @brief Overloaded dereference operator to access the object managed by this reference.
         * @return A reference to the object managed by this reference.
         */
        [[nodiscard]] elementType& operator*() const noexcept
        {
            return *get();
        }
    };
}

#endif //MEXMEMORY_REFERENCE_H