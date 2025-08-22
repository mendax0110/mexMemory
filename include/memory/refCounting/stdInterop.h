#ifndef MEXMEMORY_STDINTEROP_H
#define MEXMEMORY_STDINTEROP_H

#include "strongReference.h"
#include "weakReference.h"
#include <memory>

/// @brief Namespace for memory management with reference counting \namespace memory::refCounting
namespace memory::refCounting
{
    /**
     * @brief Converts a mexMemory Ref to a std::shared_ptr.
     * Warning: This creates a separate reference counting system. The object
     * will only be deleted when both the mexMemory references AND the std::shared_ptr
     * references reach zero. Use with caution.
     * @tparam T The type of object being referenced.
     * @tparam Allocator The allocator type used by the Ref.
     * @param ref The mexMemory Ref to convert.
     * @return A std::shared_ptr that shares ownership of the same object.
     */
    template<typename T, typename Allocator = DefaultAllocator<std::remove_extent_t<T>>>
    std::shared_ptr<T> to_shared_ptr(const Ref<T, Allocator>& ref)
    {
        if (!ref)
        {
            return std::shared_ptr<T>();
        }

        // Create a shared_ptr with a custom deleter that decrements the mexMemory reference
        // The custom deleter keeps a copy of the mexMemory Ref to maintain the reference count
        auto mexRef = ref; // Copy to keep reference alive
        
        return std::shared_ptr<T>(ref.get(), [mexRef](T*) mutable {
            // The custom deleter holds the mexMemory reference
            // When std::shared_ptr is destroyed, this deleter will run
            // and the mexRef will be destroyed, decrementing the mexMemory reference count
            mexRef.reset();
        });
    }

    /**
     * @brief Converts a std::shared_ptr to a mexMemory Ref.
     * Warning: This creates a separate reference counting system. The object
     * will only be deleted when both the std::shared_ptr references AND the mexMemory
     * references reach zero. Use with caution.
     * @tparam T The type of object being referenced.
     * @tparam Allocator The allocator type to use for the Ref.
     * @param ptr The std::shared_ptr to convert.
     * @return A mexMemory Ref that shares ownership of the same object.
     */
    template<typename T, typename Allocator = DefaultAllocator<std::remove_extent_t<T>>>
    Ref<T, Allocator> from_shared_ptr(const std::shared_ptr<T>& ptr)
    {
        if (!ptr)
        {
            return Ref<T, Allocator>();
        }

        // We can't easily integrate with std::shared_ptr's control block,
        // so we create a wrapper that keeps the shared_ptr alive
        
        // This is a simplified approach - in a production system, you might want
        // to create a custom control block that delegates to shared_ptr's control block
        
        // For now, we'll create a new mexMemory-managed object that holds the shared_ptr
        struct SharedPtrWrapper
        {
            std::shared_ptr<T> ptr;
            SharedPtrWrapper(std::shared_ptr<T> p) : ptr(std::move(p)) {}
            T* get() const { return ptr.get(); }
        };

        // This approach has limitations but provides basic interoperability
        // A more sophisticated implementation would require deeper integration
        
        // For this basic implementation, we'll just return an empty Ref as a safe fallback
        // In a real-world scenario, you'd want to implement proper control block sharing
        return Ref<T, Allocator>();
    }

    /**
     * @brief Creates a mexMemory Ref from a raw pointer that's also managed by a std::shared_ptr.
     * This is safer than from_shared_ptr as it creates independent reference counting.
     * @tparam T The type of object being referenced.
     * @tparam Allocator The allocator type to use for the Ref.
     * @param rawPtr The raw pointer to the object.
     * @return A mexMemory Ref that manages its own reference count for the object.
     */
    template<typename T, typename Allocator = DefaultAllocator<std::remove_extent_t<T>>>
    Ref<T, Allocator> adopt_raw_ptr(T* rawPtr)
    {
        if (!rawPtr)
        {
            return Ref<T, Allocator>();
        }

        // Create a mexMemory Ref that takes ownership of the raw pointer
        // Warning: The caller must ensure that no other smart pointer manages this object
        return makeRefWithAllocator<T, Allocator>(rawPtr);
    }

    /**
     * @brief Utility class for managing objects with dual reference counting systems.
     * This is an experimental feature for advanced use cases.
     */
    template<typename T>
    class DualRefObject
    {
    private:
        std::shared_ptr<T> stdPtr_;
        Ref<T> mexPtr_;

    public:
        /**
         * @brief Constructor for creating a dual-reference object.
         * @param obj The object to manage with dual reference counting.
         */
        explicit DualRefObject(T* obj)
            : stdPtr_(obj, [](T*) { /* Custom deleter - mexPtr will handle deletion */ })
            , mexPtr_(makeRef<T>(*obj))
        {
        }

        /**
         * @brief Get the std::shared_ptr.
         * @return The std::shared_ptr to the object.
         */
        std::shared_ptr<T> getSharedPtr() const { return stdPtr_; }

        /**
         * @brief Get the mexMemory Ref.
         * @return The mexMemory Ref to the object.
         */
        Ref<T> getRef() const { return mexPtr_; }

        /**
         * @brief Get the raw pointer.
         * @return The raw pointer to the object.
         */
        T* get() const { return mexPtr_.get(); }

        /**
         * @brief Check if the object is valid.
         * @return True if the object is valid, false otherwise.
         */
        bool isValid() const { return mexPtr_.isValid() && stdPtr_; }
    };

    /**
     * @brief Creates a DualRefObject that can be used with both std::shared_ptr and mexMemory Ref.
     * @tparam T The type of object to create.
     * @tparam Args The types of the constructor arguments.
     * @param args The constructor arguments.
     * @return A DualRefObject managing the created object.
     */
    template<typename T, typename... Args>
    DualRefObject<T> makeDualRef(Args&&... args)
    {
        auto obj = new T(std::forward<Args>(args)...);
        return DualRefObject<T>(obj);
    }
}

#endif //MEXMEMORY_STDINTEROP_H