#ifndef MEXMEMORY_CONTROLBLOCK_H
#define MEXMEMORY_CONTROLBLOCK_H

#include <atomic>
#include <memory>
#include <utility>
#include <iostream>
#include <string_view>
#include <memory/refCounting/allocationMap.h>

/// @brief memory::refCounting namespace, which contains the ControlBlock class for reference counting memory management \namespace memory::refCounting
namespace memory::refCounting
{
    /**
     * @brief DefaultAllocator is a simple allocator that uses new/delete for memory management.
     * @tparam T The type of object to allocate.
     */
    template <typename T>
    struct DefaultAllocator
    {
        /**
         * @brief Allocates memory for a single object of type T.
         * @return A pointer to the allocated object.
         */
        static T* allocate() { return new T(); }

        /**
         * @brief Allocates memory for a single object of type T with constructor arguments.
         * @tparam Args The types of the constructor arguments.
         * @param args The constructor arguments.
         * @return A pointer to the allocated object.
         */
        template <typename... Args>
        static T* allocate(Args&&... args) { return new T(std::forward<Args>(args)...); }

        /**
         * @brief Deallocates memory for a polymorphic object of type T.
         * @tparam U The type of the object to deallocate, which must be polymorphic.
         * @param ptr The pointer to the object to deallocate.
         */
        template <typename U>
        static void deallocate_polymorphic(U* ptr)
        {
            delete ptr;
        }

        /**
         * @brief Deallocates memory for a single object of type T.
         * @param ptr The pointer to the object to deallocate.
         */
        static void deallocate(T* ptr)
        {
            if constexpr (std::is_polymorphic_v<T>)
            {
                deallocate_polymorphic(ptr);
                //delete ptr;
            }
            else
            {
                delete ptr;
            }
        }
    };

    /**
     * @brief Specialization of DefaultAllocator for arrays of type T.
     * @tparam T The type of objects in the array.
     */
    template <typename T>
    struct DefaultAllocator<T[]>
    {
        /**
         * @brief Allocates memory for an array of type T.
         * @param size The number of elements in the array.
         * @return A pointer to the allocated array.
         */
        static T* allocate(size_t size) { return new T[size]; }

        /**
         * @brief Deallocates memory for an array of type T.
         * @param ptr The pointer to the array to deallocate.
         */
        static void deallocate(T* ptr) { delete[] ptr; }
    };

    /**
     * @brief DebugConfig is a configuration struct for enabling/disabling logging and setting the log stream.
     */
    struct DebugConfig
    {
        static inline bool enableLogging = false;
        static inline std::ostream* logStream = &std::cout;
    };

    /**
     * @brief ControlBlock is a reference counting control block for managing the lifetime of objects.
     * It supports strong and weak references, and provides thread-safe reference counting.
     * @tparam T The type of object being managed.
     * @tparam Allocator The allocator to use for memory management (default is DefaultAllocator).
     */
    template <typename T, typename Allocator = DefaultAllocator<T>>
    class ControlBlock
    {
    public:

        /**
         * @brief Constructs a ControlBlock with a pointer to an object of type T.
         * @param ptr The pointer to the object.
         */
        explicit ControlBlock(T* ptr) : objectPtr(ptr)
        {
            TRACK_ALLOC(ptr);
            logCreation();
        }

        /**
         * @brief Constructs a ControlBlock with constructor arguments for the object of type T. (with our Allocator)
         * @tparam Args The types of the constructor arguments.
         * @param args The constructor arguments.
         */
        template <typename... Args>
        explicit ControlBlock(Args&&... args) : objectPtr(Allocator::allocate(std::forward<Args>(args)...)), typeInfo(&typeid(T))
        {
            TRACK_ALLOC(objectPtr);
            logCreation();
        }

        /**
         * @brief Default constructor for ControlBlock, initializes with a nullptr object pointer.
         */
        ~ControlBlock()
        {
            logDestruction();
            if (objectPtr)
            {
                if (strongRefs.load(std::memory_order_relaxed) > 0)
                {
                    UNTRACK_ALLOC(objectPtr);
                }
                //UNTRACK_ALLOC(objectPtr);
                Allocator::deallocate(objectPtr);
                objectPtr = nullptr;
            }
        }

        /**
         * @brief Casts the object pointer to a different type U if possible.
         * @tparam U The type to cast to.
         * @return A pointer to the object of type U if the cast is valid, otherwise nullptr.
         */
        template<typename U>
        U* cast()
        {
            if (typeid(U) == *typeInfo || std::is_base_of_v<U, T> || std::is_base_of_v<T, U>)
            {
                return static_cast<U*>(objectPtr);
            }
            return nullptr;
        }

        /**
         * @brief Safely casts the object pointer to a different type U, using dynamic_cast if necessary.
         * @tparam U The type to cast to.
         * @param ptr The pointer to the object of type T.
         * @return A pointer to the object of type U if the cast is valid, otherwise nullptr.
         */
        template<typename U>
        static U* safe_cast(T* ptr)
        {
            if constexpr (std::is_base_of_v<T, U> || std::is_base_of_v<U, T>)
            {
                return static_cast<U*>(ptr);
            }
            else
            {
                return dynamic_cast<U*>(ptr);
            }
        }

        /**
         * @brief Deallocates the object managed by this ControlBlock, if it exists.
         * This method is called when the last strong reference to the object is released.
         */
        void deallocateObject()
        {
            if (objectPtr)
            {
                UNTRACK_ALLOC(objectPtr);
                if (*typeInfo == typeid(T))
                {
                    Allocator::deallocate(static_cast<T*>(objectPtr));
                }
                else
                {
                    operator delete(objectPtr);
                }
                objectPtr = nullptr;
            }
        }

        /**
         * @brief Sets the object pointer to a new pointer, deallocating the old object if it exists.
         * @param ptr The new pointer to set.
         */
        void setObjectPtr(T* ptr) noexcept
        {
            if (objectPtr)
            {
                logAction("Deleting old object");
                UNTRACK_ALLOC(objectPtr);
                Allocator::deallocate(objectPtr);
            }
            objectPtr = ptr;
            if (ptr)
            {
                TRACK_ALLOC(ptr);
            }
            logAction("Setting new object");
        }

        /**
         * @brief Gets the pointer to the object managed by this ControlBlock.
         * @return A pointer to the object.
         */
        [[nodiscard]] T* getObjectPtr() const noexcept
        {
            return objectPtr;
        }

        /**
         * @brief Increments the strong reference count.
         */
        void incrementStrong() noexcept
        {
            strongRefs.fetch_add(1, std::memory_order_relaxed);
            logReferenceChange("Increment strong reference", strongRefs.load());
        }

        /**
         * @brief Decrements the strong reference count and deletes the object if it reaches zero.
         */
        void decrementStrong() noexcept
        {
            auto prev = strongRefs.fetch_sub(1, std::memory_order_acq_rel);
            logReferenceChange("Decrement strong reference", prev - 1);

            if (prev == 1)
            {
                if (objectPtr)
                {
                    logAction("Deleting object");
                    UNTRACK_ALLOC(objectPtr);
                    Allocator::deallocate(objectPtr);
                }
                objectPtr = nullptr;

                if (weakRefs.load(std::memory_order_relaxed) == 0)
                {
                    logAction("Deleting control block (no weak references)");
                    delete this;
                }
            }
        }

        /**
         * @brief Increments the weak reference count.
         */
        void incrementWeak() noexcept
        {
            weakRefs.fetch_add(1, std::memory_order_relaxed);
            logReferenceChange("Increment weak reference", weakRefs.load());
        }

        /**
         * @brief Decrements the weak reference count and deletes the control block if it reaches zero and there are no strong references.
         */
        void decrementWeak() noexcept
        {
            auto prev = weakRefs.fetch_sub(1, std::memory_order_acq_rel);
            logReferenceChange("Decrement weak reference", prev - 1);

            if (prev == 1)
            {
                if (strongRefs.load(std::memory_order_relaxed) == 0)
                {
                    logAction("Deleting control block (no strong references)");
                    delete this;
                }
            }
        }

        /**
         * @brief Gets the current strong reference count.
         * @return A size_t representing the number of strong references.
         */
        [[nodiscard]] size_t strongCount() const noexcept
        {
            return strongRefs.load(std::memory_order_relaxed);
        }

        /**
         * @brief Sets the strong reference count to a specific value.
         * @param count The new strong reference count.
         * @return The new strong reference count.
         */
        size_t setStrongCount(size_t count) noexcept
        {
            strongRefs.store(count, std::memory_order_relaxed);
            logReferenceChange("Set strong reference count", count);
            return count;
        }

        /**
         * @brief Gets the current weak reference count.
         * @return A size_t representing the number of weak references.
         */
        [[nodiscard]] size_t weakCount() const noexcept
        {
            return weakRefs.load(std::memory_order_relaxed);
        }

        /**
         * @brief Checks If has object pointer.
         * @return A boolean indicating whether the ControlBlock has an object pointer.
         */
        [[nodiscard]] bool hadObject() const noexcept
        {
            return objectPtr != nullptr;
        }

        /**
         * @brief Gets the object pointer.
         * @return A pointer to the object managed by this ControlBlock.
         */
        T* get() const noexcept
        {
            return objectPtr;
        }

    private:
        T* objectPtr;
        std::type_info const* typeInfo;
        std::atomic<size_t> strongRefs{1};
        std::atomic<size_t> weakRefs{0};

        /**
         * @brief Logs creattion of a object.
         */
        void logCreation() const
        {
            if (DebugConfig::enableLogging)
            {
                *DebugConfig::logStream
                        << "[ControlBlock] Created for object at "
                        << static_cast<const void*>(objectPtr)
                        << "\n";
            }
        }

        /**
         * @brief Logs destruction of a object.
         */
        void logDestruction() const
        {
            if (DebugConfig::enableLogging)
            {
                *DebugConfig::logStream
                        << "[ControlBlock] Destroyed for object at "
                        << static_cast<const void*>(objectPtr)
                        << "\n";
            }
        }

        /**
         * @brief Logs changes in reference counts.
         * @param action The action being logged (increment/decrement).
         * @param count The current count after the change.
         */
        void logReferenceChange(const std::string_view action, size_t count) const
        {
            if (DebugConfig::enableLogging)
            {
                *DebugConfig::logStream
                        << "[ControlBlock] " << action
                        << ", current count: " << count
                        << " for object at "
                        << static_cast<const void*>(objectPtr)
                        << "\n";
            }
        }

        /**
         * @brief Logs an action performed on the ControlBlock.
         * @param action The action being logged.
         */
        void logAction(const std::string_view action) const
        {
            if (DebugConfig::enableLogging)
            {
                *DebugConfig::logStream
                        << "[ControlBlock] " << action
                        << " for object at "
                        << static_cast<const void*>(objectPtr)
                        << "\n";
            }
        }
    };
}

#endif //MEXMEMORY_CONTROLBLOCK_H