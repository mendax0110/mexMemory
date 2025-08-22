#ifndef MEXMEMORY_REFERENCECASTING_H
#define MEXMEMORY_REFERENCECASTING_H

#include "reference.h"
#include "strongReference.h"
#include "weakReference.h"

/// @brief Namespace for memory management with reference counting \namespace memory::refCounting
namespace memory::refCounting
{
    /**
     * @brief Static cast for converting a Ref of one type to a Ref of another type.
     * @tparam U The type to cast to, which must be convertible from T.
     * @tparam T The type to cast from, which must be convertible to U.
     * @tparam Allocator A custom allocator type, defaults to DefaultAllocator for the element type of T.
     * @param ref The Ref object to cast from.
     * @return A Ref object of type U, or an empty Ref if the original ref is null.
     */
    template <typename U, typename T, typename Allocator = DefaultAllocator<std::remove_extent_t<T>>>
    Ref<U, Allocator> static_pointer_cast(const Ref<T, Allocator>& ref) noexcept
    {
        if (!ref) return Ref<U, Allocator>(nullptr);

        static_assert(std::is_convertible_v<T*, U*> || std::is_convertible_v<U*, T*>,
                     "static_pointer_cast can't convert between unrelated types");

        Ref<U, Allocator> result;
        result.controlBlock = reinterpret_cast<typename Ref<U, Allocator>::controlBlockType*>(ref.getControlBlock());
        result.retain();
        return result;
    }


    /**
     * @brief Dynamic cast for converting a Ref of one type to a Ref of another type with runtime type checking.
     * @tparam U The type to cast to, which must be polymorphic and related to T.
     * @tparam T The type to cast from, which must be polymorphic and related to U.
     * @tparam Allocator A custom allocator type, defaults to DefaultAllocator for the element type of T.
     * @param ref The Ref object to cast from.
     * @return A Ref object of type U if the cast succeeds, or an empty Ref if it fails.
     */
    template <typename U, typename T, typename Allocator = DefaultAllocator<std::remove_extent_t<T>>>
    Ref<U, Allocator> dynamic_pointer_cast(const Ref<T, Allocator>& ref) noexcept
    {
        if (!ref) return Ref<U, Allocator>(nullptr);

        static_assert(std::is_polymorphic_v<T>, "dynamic_pointer_cast requires polymorphic types");
        static_assert(std::is_polymorphic_v<U>, "dynamic_pointer_cast requires polymorphic types");
        static_assert(std::is_base_of_v<T, U> || std::is_base_of_v<U, T>,
                     "dynamic_pointer_cast can't convert between unrelated types");

        if (auto* castedPtr = dynamic_cast<U*>(ref.get()))
        {
            Ref<U, Allocator> result;
            result.controlBlock = reinterpret_cast<typename Ref<U, Allocator>::controlBlockType*>(ref.getControlBlock());
            result.retain();
            return result;
        }
        return Ref<U, Allocator>(nullptr);
    }

    /**
     * @brief Const cast for converting a Ref of one type to a Ref of another type, typically for const/non-const conversions.
     * @tparam U The type to cast to, which must be const/non-const equivalent to T.
     * @tparam T The type to cast from, which must be const/non-const equivalent to U.
     * @tparam Allocator A custom allocator type, defaults to DefaultAllocator for the element type of T.
     * @param ref The Ref object to cast from.
     * @return A Ref object of type U, or an empty Ref if the original ref is null.
     */
    template <typename U, typename T, typename Allocator = DefaultAllocator<std::remove_extent_t<T>>>
    Ref<U, Allocator> const_pointer_cast(const Ref<T, Allocator>& ref) noexcept
    {
        if (!ref) return Ref<U, Allocator>(nullptr);

        static_assert(std::is_convertible_v<T*, U*> || std::is_convertible_v<U*, T*>,
                      "const_pointer_cast can't convert between unrelated types");

        Ref<U, Allocator> result;
        result.controlBlock = reinterpret_cast<typename Ref<U, Allocator>::controlBlockType*>(ref.getControlBlock());
        result.retain();
        return result;
    }

    /**
     * @brief Reinterpret cast for converting a Ref of one type to a Ref of another type with raw pointer conversion.
     * Warning: This is unsafe and should be used with extreme caution. The allocator type is adjusted for the target type.
     * @tparam U The type to cast to.
     * @tparam T The type to cast from.
     * @tparam Allocator A custom allocator type, defaults to DefaultAllocator for the element type of T.
     * @param ref The Ref object to cast from.
     * @return A Ref object of type U with appropriate allocator, or an empty Ref if the original ref is null.
     */
    template <typename U, typename T, typename Allocator = DefaultAllocator<std::remove_extent_t<T>>>
    Ref<U, DefaultAllocator<std::remove_extent_t<U>>> reinterpret_pointer_cast(const Ref<T, Allocator>& ref) noexcept
    {
        using TargetAllocator = DefaultAllocator<std::remove_extent_t<U>>;
        
        if (!ref) return Ref<U, TargetAllocator>(nullptr);

        // This is inherently unsafe - we're reinterpreting both the pointer type and the allocator
        // The user must ensure the types are compatible
        Ref<U, TargetAllocator> result;
        result.controlBlock = reinterpret_cast<typename Ref<U, TargetAllocator>::controlBlockType*>(ref.getControlBlock());
        result.retain();
        return result;
    }
}


#endif //MEXMEMORY_REFERENCECASTING_H
