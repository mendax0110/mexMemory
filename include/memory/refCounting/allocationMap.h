#ifndef MEXMEMORY_ALLOCATIONMAP_H
#define MEXMEMORY_ALLOCATIONMAP_H

#include <unordered_map>
#include <mutex>
#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdlib.h>
#include <cstdlib>
#include <cxxabi.h>

/// @brief Namespace for memory management with reference counting \namespace memory::refCounting
namespace memory::refCounting
{
    /// @brief Class for tracking memory allocations and detecting leaks. \class AllocationTracker
    class AllocationTracker
    {
    public:

        /**
         * @brief Struct to hold information about a memory allocation.
         */
        struct AllocationInfo
        {
            void* ptr;
            size_t size;
            std::string type;
            std::string file;
            int line;

            /**
             * @brief Constructor to initialize AllocationInfo.
             * @param p The pointer to the allocated memory.
             * @param s The size of the allocated memory.
             * @param t The type of the allocated object.
             * @param f The file where the allocation occurred.
             * @param l The line number where the allocation occurred.
             */
            AllocationInfo(void* p, size_t s, const std::string& t, const std::string& f, int l)
                : ptr(p)
                , size(s)
                , type(t)
                , file(f)
                , line(l)
            {

            }
        };

    private:
        static inline std::unordered_map<void*, AllocationInfo> allocations_;
        static inline std::mutex mutex_;
        static inline bool enabled_ = false;
        static inline bool breakOnLeak_ = false;
        static inline std::ostream* leakStream_ = &std::cerr;

    public:

        /**
         * @brief Gets the current allocations map.
         * @return A constant reference to the map of allocations.
         */
        static const std::unordered_map<void*, AllocationInfo>& getAllocations() noexcept
        {
            return allocations_;
        }

        /**
         * @brief Gets the mutex used for thread-safe access to the allocations map.
         * @return A reference to the mutex.
         */
        static std::mutex& getMutex() noexcept
        {
            return mutex_;
        }

        /**
         * @brief Enables or disables memory allocation tracking.
         * @param enable The flag to enable or disable tracking.
         */
        static void enableTracking(bool enable) noexcept
        {
            std::lock_guard<std::mutex> lock(mutex_);
            enabled_ = enable;
        }

        /**
         * @brief Setter for the breakOnLeak flag.
         * @param enable The flag to enable or disable breaking on memory leaks.
         */
        static void setBreakOnLeak(bool enable) noexcept
        {
            breakOnLeak_ = enable;
        }

        /**
         * @brief Sets the output stream for leak reports.
         * @param stream The output stream to use for reporting memory leaks.
         */
        static void setLeakStream(std::ostream* stream) noexcept
        {
            leakStream_ = stream;
        }

        /**
         * @brief Demangles the type name of a given type T.
         * @tparam T The type to demangle.
         * @return A string containing the demangled type name.
         */
        template<typename T>
        static std::string demangleTypeName()
        {
            int status = 0;
            char* demangled = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status);
            if (demangled)
            {
                std::string result(demangled);
                free(demangled);
                return result;
            }
            return typeid(T).name();
        }

        /**
         * @brief Tracks a memory allocation for a given pointer and type.
         * @tparam T The type of the allocated object.
         * @param ptr The pointer to the allocated memory.
         * @param count The number of elements allocated (default is 1).
         * @param file The file where the allocation occurred (default is empty).
         * @param line The line number where the allocation occurred (default is 0).
         */
        template<typename T>
        static void trackAllocation(T* ptr, size_t count = 1, const std::string& file = "", int line = 0)
        {
            if (!enabled_) return;

            std::lock_guard<std::mutex> lock(mutex_);
            std::string typeName = demangleTypeName<T>();
            allocations_.emplace(ptr, AllocationInfo{
                ptr,
                sizeof(T) * count,
                typeName,
                file,
                line
            });
        }

        /**
         * @brief Untracks a memory allocation for a given pointer.
         * @param ptr The pointer to the allocated memory to untrack.
         */
        static void untrackAllocation(void* ptr) noexcept
        {
            if (!enabled_) return;
            std::lock_guard<std::mutex> lock(mutex_);
            if (allocations_.contains(ptr))
            {
                allocations_.erase(ptr);
            }
        }

        /**
         * @brief Clears all tracked memory allocations.
         */
        static void clearAllocations() noexcept
        {
            std::lock_guard<std::mutex> lock(mutex_);
            allocations_.clear();
        }

        /**
         * @brief Checks for memory leaks by iterating through tracked allocations.
         * @return A size_t representing the number of leaked allocations.
         */
        static size_t checkLeaks()
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (allocations_.empty())
            {
                return 0;
            }

            *leakStream_ << "\n=== MEMORY LEAKS DETECTION REPORT ===\n";
            *leakStream_ << std::setw(20) << "Pointer"
                        << std::setw(10) << "Size"
                        << std::setw(30) << "Type"
                        << std::setw(30) << "File"
                        << std::setw(5) << "Line"
                        << "\n";

            size_t totalLeaked = 0;
            for (const auto& [ptr, info] : allocations_)
            {
                *leakStream_ << std::setw(20) << info.ptr
                            << std::setw(10) << info.size
                            << std::setw(30) << info.type
                            << std::setw(30) << info.file
                            << std::setw(5) << info.line
                            << "\n";

                if (!info.file.empty() && breakOnLeak_)
                {
                    std::ostringstream oss;
                    oss << "Memory leak detected at " << info.file << ":" << info.line
                        << " for type " << info.type << " at address " << info.ptr
                        << " of size " << info.size << ".";
                    throw std::runtime_error(oss.str());
                }

                totalLeaked += info.size;
            }

            *leakStream_ << "\nTotal leaked memory: " << totalLeaked << " bytes\n";
            *leakStream_ << "====================================\n";

            if (breakOnLeak_)
            {
                abort();
            }

            return allocations_.size();
        }

        /**
         * @brief Gets the number of currently tracked allocations.
         * @return The number of tracked allocations.
         */
        static size_t getAllocationCount() noexcept
        {
            std::lock_guard<std::mutex> lock(mutex_);
            return allocations_.size();
        }

        /**
         * @brief Gets the total number of bytes allocated across all tracked allocations.
         * @return The total number of bytes allocated.
         */
        static size_t getTotalAllocatedBytes() noexcept
        {
            std::lock_guard<std::mutex> lock(mutex_);
            size_t total = 0;
            for (const auto& [ptr, info] : allocations_)
            {
                total += info.size;
            }
            return total;
        }

        /**
         * @brief Memory statistics for detailed analysis.
         */
        struct MemoryStatistics
        {
            size_t total_allocations{0};
            size_t total_bytes{0};
            size_t largest_allocation{0};
            size_t smallest_allocation{SIZE_MAX};
            double average_allocation_size{0.0};
            std::unordered_map<std::string, size_t> allocations_by_type;
            std::unordered_map<std::string, size_t> bytes_by_type;
        };

        /**
         * @brief Gets detailed memory usage statistics.
         * @return MemoryStatistics structure containing detailed information.
         */
        static MemoryStatistics getStatistics() noexcept
        {
            std::lock_guard<std::mutex> lock(mutex_);
            MemoryStatistics stats;

            stats.total_allocations = allocations_.size();
            
            for (const auto& [ptr, info] : allocations_)
            {
                stats.total_bytes += info.size;
                stats.largest_allocation = std::max(stats.largest_allocation, info.size);
                stats.smallest_allocation = std::min(stats.smallest_allocation, info.size);
                
                // Count by type
                stats.allocations_by_type[info.type]++;
                stats.bytes_by_type[info.type] += info.size;
            }

            if (stats.total_allocations > 0)
            {
                stats.average_allocation_size = static_cast<double>(stats.total_bytes) / stats.total_allocations;
            }
            else
            {
                stats.smallest_allocation = 0;
            }

            return stats;
        }

        /**
         * @brief Prints detailed memory statistics to the specified stream.
         * @param stream The output stream to print to (default: std::cout).
         */
        static void printStatistics(std::ostream* stream = &std::cout) noexcept
        {
            if (!stream) return;

            auto stats = getStatistics();
            
            *stream << "\n=== Memory Usage Statistics ===\n";
            *stream << "Total allocations: " << stats.total_allocations << "\n";
            *stream << "Total bytes: " << stats.total_bytes << "\n";
            
            if (stats.total_allocations > 0)
            {
                *stream << "Largest allocation: " << stats.largest_allocation << " bytes\n";
                *stream << "Smallest allocation: " << stats.smallest_allocation << " bytes\n";
                *stream << "Average allocation size: " << std::fixed << std::setprecision(2) 
                       << stats.average_allocation_size << " bytes\n";
            }

            if (!stats.allocations_by_type.empty())
            {
                *stream << "\nAllocations by type:\n";
                for (const auto& [type, count] : stats.allocations_by_type)
                {
                    auto bytes = stats.bytes_by_type.at(type);
                    *stream << "  " << std::setw(30) << type 
                           << ": " << std::setw(6) << count << " allocations, "
                           << std::setw(10) << bytes << " bytes\n";
                }
            }
            *stream << "==============================\n\n";
        }

        /**
         * @brief Gets allocations filtered by type name.
         * @param typeName The type name to filter by.
         * @return Vector of allocation info for the specified type.
         */
        static std::vector<AllocationInfo> getAllocationsByType(const std::string& typeName) noexcept
        {
            std::lock_guard<std::mutex> lock(mutex_);
            std::vector<AllocationInfo> result;
            
            for (const auto& [ptr, info] : allocations_)
            {
                if (info.type == typeName)
                {
                    result.push_back(info);
                }
            }
            
            return result;
        }
    };

    /// @brief LeakDetector class to automatically check for memory leaks on destruction. \class LeakDetector
    class LeakDetector
    {
    public:

        /**
         * @brief Default constructor for LeakDetector.
         */
        ~LeakDetector()
        {
            AllocationTracker::checkLeaks();
        }
    };

    static inline LeakDetector globalLeakDetector;
}

#define TRACK_ALLOC(ptr) \
    memory::refCounting::AllocationTracker::trackAllocation(ptr, 1, __FILE__, __LINE__)

#define UNTRACK_ALLOC(ptr) \
    memory::refCounting::AllocationTracker::untrackAllocation(ptr)

#endif //MEXMEMORY_ALLOCATIONMAP_H
