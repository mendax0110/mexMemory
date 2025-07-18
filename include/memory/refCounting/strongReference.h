#ifndef MEXMEMORY_STRONGREFERENCE_H
#define MEXMEMORY_STRONGREFERENCE_H

#include "reference.h"
#include "weakReference.h"
#include <memory>
#include <utility>

/// @brief Namespace for memory management with reference counting \namespace memory::refCounting
namespace memory::refCounting
{
    /**
     * @brief Ref is a strong reference type that manages the lifetime of an object using reference counting.
     * @tparam T The type of object being referenced, can be a single object or an array.
     * @tparam Allocator The allocator to use for memory management, defaults to DefaultAllocator.
     */
    template <typename T, typename Allocator = DefaultAllocator<std::remove_extent_t<T>>>
    class Ref : public ReferenceBase<T, Allocator>
    {
        /**
         * @brief Type alias for the base class ReferenceBase.
         */
        using base = ReferenceBase<T, Allocator>;
        using base::controlBlock;

        /**
         * @brief Friend declaration for Ref to allow it to access private members of Ref.
         * @tparam U The type of object being referenced by Ref, can be different from T.
         * @tparam A The allocator used by Ref, can be different from Allocator.
         */
        template <typename U, typename A>
        friend class Ref;

        /**
         * @brief Friend declaration for WeakRef to allow it to access private members of Ref.
         * @tparam U The type of object being referenced by WeakRef, can be different from T.
         * @tparam A The allocator used by WeakRef, can be different from Allocator.
         */
        template <typename U, typename A>
        friend class WeakRef;

        /**
         * @brief Retains the strong reference to the object managed by this Ref.
         */
        void retain() noexcept
        {
            if (controlBlock)
            {
                controlBlock->incrementStrong();
            }
        }

        /**
         * @brief Releases the strong reference to the object managed by this Ref.
         */
        void release() noexcept
        {
            if (controlBlock)
            {
                controlBlock->decrementStrong();
                controlBlock = nullptr;
            }
        }

    protected:

        /**
         * @brief Constructs a Ref with a given control block.
         * @param cb The control block to associate with this reference.
         */
        explicit Ref(typename base::controlBlockType* cb) noexcept : base(cb)
        {
            // Don't increment here - ControlBlock already starts with count=1
        }

    public:

        /**
         * @brief Default constructor for Ref.
         */
        constexpr Ref() noexcept = default;

        /**
         * @brief Explicit constructor for Ref that initializes it with a nullopt value.
         */
        explicit Ref(std::nullopt_t) noexcept : Ref() {}

        /**
         * @brief Constructs a Ref from a raw pointer to an object of type U.
         * @tparam U The type of object being referenced, which must be convertible to T.
         * @param ptr The pointer to the object of type U.
         */
        template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        explicit Ref(U* ptr) : base(new ControlBlock<U, Allocator>(ptr))
        {
            // ControlBlock constructor already sets count to 1
        }

        /**
         * @brief Constructs a Ref from a raw pointer to an object of type Ref&
         * @param other The Ref object to copy from.
         */
        Ref(const Ref& other) noexcept : base(other.controlBlock)
        {
            retain();
        }

        /**
         * @brief Constructs a Ref from another Ref of a different type U, type A that is convertible to T.
         * @tparam U The type of object being referenced, which must be convertible to T.
         * @tparam A The allocator used by the other Ref, which must be convertible to Allocator.
         * @param other The Ref object to copy from.
         */
        template <typename U, typename A, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        Ref(const Ref<U, A>& other) noexcept : base(other.controlBlock)
        {
            retain();
        }

        /**
         * @brief Move constructor for Ref that transfers ownership of the control block from another Ref.
         * @param other The Ref object to move from.
         */
        Ref(Ref&& other) noexcept : base(other.controlBlock)
        {
            other.controlBlock = nullptr; // Transfer ownership without changing count
        }

        /**
         * @brief Move constructor for Ref that transfers ownership of the control block from another Ref of a different type U, type A that is convertible to T.
         * @tparam U The type of object being referenced, which must be convertible to T.
         * @tparam A The allocator used by the other Ref, which must be convertible to Allocator.
         * @param other The Ref object to move from.
         */
        template <typename U, typename A, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        Ref(Ref<U, A>&& other) noexcept : base(other.controlBlock)
        {
            other.controlBlock = nullptr; // Transfer ownership without changing count
        }

        /**
         * @brief Destructor for Ref that releases the strong reference to the object managed by this Ref.
         */
        ~Ref() { release(); }

        /**
         * @brief Copy assignment operator for Ref.
         * @param other The Ref object to copy from.
         * @return A reference to this Ref object.
         */
        Ref& operator=(const Ref& other) noexcept
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
         * @brief Operator overload for assigning another Ref of a different type U, type A that is convertible to T.
         * @tparam U The type of object being referenced, which must be convertible to T.
         * @tparam A The allocator used by the other Ref, which must be convertible to Allocator.
         * @param other The Ref object to copy from.
         * @return A reference to this Ref object.
         */
        template <typename U, typename A, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        Ref& operator=(const Ref<U, A>& other) noexcept
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
         * @brief Move assignment operator for Ref that transfers ownership of the control block from another Ref.
         * @param other The Ref object to move from.
         * @return A reference to this Ref object.
         */
        Ref& operator=(Ref&& other) noexcept
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
         * @brief Move assignment operator for Ref that transfers ownership of the control block from another Ref of a different type U, type A that is convertible to T.
         * @tparam U The type of object being referenced, which must be convertible to T.
         * @tparam A The allocator used by the other Ref, which must be convertible to Allocator.
         * @param other The Ref object to move from.
         * @return A reference to this Ref object.
         */
        template <typename U, typename A, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        Ref& operator=(Ref<U, A>&& other) noexcept
        {
            if (this != static_cast<const void*>(&other))
            {
                release();
                controlBlock = other.controlBlock;
                other.controlBlock = nullptr; // Transfer ownership without changing count
            }
            return *this;
        }

        /**
         * @brief Constructs a Ref from a WeakRef of type U, type A that is convertible to T.
         * @tparam U The type of object being referenced, which must be convertible to T.
         * @tparam A The allocator used by the WeakRef, which must be convertible to Allocator.
         * @param weak The WeakRef object to copy from.
         */
        template <typename U, typename A, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        explicit Ref(const WeakRef<U, A>& weak) : base(weak.controlBlock)
        {
            if (controlBlock && controlBlock->strongCount() > 0)
            {
                controlBlock->incrementStrong();
            }
            else
            {
                controlBlock = nullptr;
            }
        }

        /**
         * @brief Gets a weak reference to the object managed by this Ref.
         * @return The WeakRef object representing a weak reference to the same object.
         */
        [[nodiscard]] WeakRef<T, Allocator> weak() const noexcept
        {
            return WeakRef<T, Allocator>(*this);
        }

        /**
         * @brief Friend declaration for makeRef to allow access to private constructor.
         * @tparam U The type of object being referenced, which must be convertible to T.
         * @tparam Args The types of constructor arguments for the object.
         * @param args The constructor arguments for the object.
         * @return A pointer to the object managed by this Ref.
         */
        template <typename U, typename... Args>
        friend Ref<U> makeRef(Args&&... args);

        /**
         * @brief Friend declaration for makeRefWithAllocator to allow access to private constructor.
         * @tparam U The type of object being referenced, which must be convertible to T.
         * @tparam A The allocator used for memory management.
         * @tparam Args The types of constructor arguments for the object.
         * @param args The constructor arguments for the object.
         * @return A Ref object representing the newly created object with the specified allocator.
         */
        template <typename U, typename A, typename... Args>
        friend Ref<U, A> makeRefWithAllocator(Args&&... args);
    };

    /**
     * @brief Creates a Ref object for a given type T with the specified constructor arguments.
     * @tparam T The type of object being referenced, can be a single object or an array.
     * @tparam Args The types of constructor arguments for the object.
     * @param args The constructor arguments for the object.
     * @return A Ref object representing the newly created object.
     */
    template <typename T, typename... Args>
    Ref<T> makeRef(Args&&... args)
    {
        using ElementType = std::remove_extent_t<T>;
        return Ref<T>(new ControlBlock<ElementType, DefaultAllocator<ElementType>>(std::forward<Args>(args)...));
    }

    /**
     * @brief Creates a Ref object for a given type T with the specified constructor arguments using a custom allocator.
     * @tparam T The type of object being referenced, can be a single object or an array.
     * @tparam Allocator The allocator to use for memory management.
     * @tparam Args The types of constructor arguments for the object.
     * @param args The constructor arguments for the object.
     * @return A Ref object representing the newly created object with the specified allocator.
     */
    template <typename T, typename Allocator, typename... Args>
    Ref<T, Allocator> makeRefWithAllocator(Args&&... args)
    {
        using ElementType = std::remove_extent_t<T>;
        return Ref<T, Allocator>(new ControlBlock<ElementType, Allocator>(std::forward<Args>(args)...));
    }
}

#endif //MEXMEMORY_STRONGREFERENCE_H