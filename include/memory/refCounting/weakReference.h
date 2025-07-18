#ifndef MEXMEMORY_WEAKREFERENCE_H
#define MEXMEMORY_WEAKREFERENCE_H

#include "reference.h"
#include "strongReference.h"
#include <utility>
#include <memory>

/// @brief Namespace for memory management with reference counting \namespace memory::refCounting
namespace memory::refCounting
{
    /**
     * @brief WeakRef is a weak reference type that allows access to an object without affecting its lifetime.
     * @tparam T The type of object being referenced, can be a single object or an array.
     * @tparam Allocator The allocator to use for memory management, defaults to DefaultAllocator.
     */
    template <typename T, typename Allocator = DefaultAllocator<std::remove_extent_t<T>>>
    class WeakRef : public ReferenceBase<T, Allocator>
    {
        /**
         * @brief Type alias for the base class ReferenceBase.
         */
        using base = ReferenceBase<T, Allocator>;
        using base::controlBlock;

        /**
         * @brief Friend declaration for weak reference to allow it to access private members of Ref.
         * @tparam U The type of object being referenced by WeakRef, can be different from T.
         * @tparam A The allocator used by WeakRef, can be different from Allocator.
         */
        template <typename U, typename A>
        friend class WeakRef;

        /**
         * @brief Friend declaration for Ref to allow it to access private members of WeakRef.
         * @tparam U The type of object being referenced by Ref, can be different from T.
         * @tparam A The allocator used by Ref, can be different from Allocator.
         */
        template <typename U, typename A>
        friend class Ref;

        /**
         * @brief Retains the weak reference to the object managed by this WeakRef.
         */
        void retain() noexcept
        {
            if (controlBlock)
            {
                controlBlock->incrementWeak();
            }
        }

        /**
         * @brief Releases the weak reference to the object managed by this WeakRef.
         */
        void release() noexcept
        {
            if (controlBlock)
            {
                controlBlock->decrementWeak();
                controlBlock = nullptr;
            }
        }

    public:

        /**
         * @brief Default constructor for WeakRef.
         */
        constexpr WeakRef() noexcept = default;

        /**
         * @brief Constructs a WeakRef from a strong reference.
         * @param strongRef The strong reference to create a weak reference from.
         */
        WeakRef(const Ref<T, Allocator>& strongRef) noexcept : base(strongRef.controlBlock)
        {
            retain();
        }

        /**
         * @brief Constructs a WeakRef from a strong reference of a different type U, type A that is convertible to T.
         * @tparam U The type of object being referenced, which must be convertible to T.
         * @tparam A The allocator used by the strong reference, which must be convertible to Allocator.
         * @param strongRef The strong reference to create a weak reference from.
         */
        template <typename U, typename A, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        WeakRef(const Ref<U, A>& strongRef) noexcept : base(strongRef.controlBlock)
        {
            retain();
        }

        /**
         * @brief Constructs a WeakRef from another WeakRef.
         * @param other The WeakRef to copy from.
         */
        WeakRef(const WeakRef& other) noexcept : base(other.controlBlock)
        {
            retain();
        }

        /**
         * @brief Constructs a WeakRef from another WeakRef of a different type U, type A that is convertible to T.
         * @tparam U The type of object being referenced, which must be convertible to T.
         * @tparam A The allocator used by the other WeakRef, which must be convertible to Allocator.
         * @param other The WeakRef to copy from.
         */
        template <typename U, typename A, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        WeakRef(const WeakRef<U, A>& other) noexcept : base(other.controlBlock)
        {
            retain();
        }

        /**
         * @brief Move constructor for WeakRef that transfers ownership of the control block from another WeakRef.
         * @param other The WeakRef to move from.
         */
        WeakRef(WeakRef&& other) noexcept : base(other.controlBlock)
        {
            other.controlBlock = nullptr;
        }

        /**
         * @brief Move constructor for WeakRef that transfers ownership of the control block from another WeakRef of a different type U, type A that is convertible to T.
         * @tparam U The type of object being referenced, which must be convertible to T.
         * @tparam A The allocator used by the other WeakRef, which must be convertible to Allocator.
         * @param other The WeakRef to move from.
         */
        template <typename U, typename A, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        WeakRef(WeakRef<U, A>&& other) noexcept : base(other.controlBlock)
        {
            other.controlBlock = nullptr;
        }

        /**
         * @brief Destructor for WeakRef that releases the control block.
         */
        ~WeakRef()
        {
            release();
        }

        /**
         * @brief Copy assignment operator for WeakRef that copies the control block from another WeakRef.
         * @param other The WeakRef to copy from.
         * @return The current WeakRef object.
         */
        WeakRef& operator=(const WeakRef& other) noexcept
        {
            if (this != &other)
            {
                release();
                controlBlock = other.controlBlock;
                retain();
            }
            return *this;
        }

        /**
         * @brief Copy assignment operator for WeakRef that copies the control block from another WeakRef of a different type U, type A that is convertible to T.
         * @tparam U The type of object being referenced, which must be convertible to T.
         * @tparam A The allocator used by the other WeakRef, which must be convertible to Allocator.
         * @param other The WeakRef to copy from.
         * @return A reference to the current WeakRef object.
         */
        template <typename U, typename A, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        WeakRef& operator=(const WeakRef<U, A>& other) noexcept
        {
            if (this != static_cast<const void*>(&other))
            {
                release();
                controlBlock = other.controlBlock;
                retain();
            }
            return *this;
        }

        /**
         * @brief Move assignment operator for WeakRef that transfers ownership of the control block from another WeakRef.
         * @param other The WeakRef to move from.
         * @return A reference to the current WeakRef object.
         */
        WeakRef& operator=(WeakRef&& other) noexcept
        {
            if (this != &other)
            {
                release();
                controlBlock = other.controlBlock;
                other.controlBlock = nullptr;
            }
            return *this;
        }

        /**
         * @brief Move assignment operator for WeakRef that transfers ownership of the control block from another WeakRef of a different type U, type A that is convertible to T.
         * @tparam U The type of object being referenced, which must be convertible to T.
         * @tparam A The allocator used by the other WeakRef, which must be convertible to Allocator.
         * @param other The WeakRef to move from.
         * @return A reference to the current WeakRef object.
         */
        template <typename U, typename A, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        WeakRef& operator=(WeakRef<U, A>&& other) noexcept
        {
            if (this != static_cast<const void*>(&other))
            {
                release();
                controlBlock = other.controlBlock;
                other.controlBlock = nullptr;
            }
            return *this;
        }

        /**
         * @brief Checks if the weak reference has expired, meaning there are no strong references to the object.
         * @return A boolean indicating whether the weak reference has expired.
         */
        [[nodiscard]] bool expired() const noexcept
        {
            return !controlBlock || controlBlock->strongCount() == 0;
        }

        /**
         * @brief Checks if the weak reference can be locked, meaning there are strong references to the object.
         * @return A boolean indicating whether the weak reference can be locked.
         */
        [[nodiscard]] bool canLock() const noexcept
        {
            return controlBlock && controlBlock->strongCount() > 0;
        }

        /**
         * @brief Locks the weak reference, creating a strong reference to the object if it exists.
         * @return The Ref object representing a strong reference to the object, or an empty Ref if the weak reference has expired.
         */
        [[nodiscard]] Ref<T, Allocator> lock() const noexcept
        {
            if (canLock())
            {
                controlBlock->incrementStrong();
                return Ref<T, Allocator>(controlBlock);
            }
            return Ref<T, Allocator>();
        }
    };
}

#endif //MEXMEMORY_WEAKREFERENCE_H