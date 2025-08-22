#ifndef MEXMEMORY_CYCLEDETECTION_H
#define MEXMEMORY_CYCLEDETECTION_H

#include "reference.h"
#include <unordered_set>
#include <vector>
#include <functional>

/// @brief Namespace for memory management with reference counting \namespace memory::refCounting
namespace memory::refCounting
{
    /**
     * @brief Circular reference detector for debugging memory leaks caused by cycles.
     */
    class CycleDetector
    {
    public:
        /**
         * @brief Information about detected cycles.
         */
        struct CycleInfo
        {
            std::vector<void*> cycle_path;
            size_t cycle_length;
            std::string description;
        };

    private:
        static inline bool enabled_ = false;
        static inline std::function<void(const CycleInfo&)> callback_ = nullptr;

    public:
        /**
         * @brief Enables or disables cycle detection.
         * @param enable True to enable cycle detection, false to disable.
         */
        static void enableDetection(bool enable) noexcept
        {
            enabled_ = enable;
        }

        /**
         * @brief Sets a callback function to be called when cycles are detected.
         * @param callback The callback function to call when a cycle is found.
         */
        static void setCycleCallback(std::function<void(const CycleInfo&)> callback) noexcept
        {
            callback_ = std::move(callback);
        }

        /**
         * @brief Check if cycle detection is enabled.
         * @return True if cycle detection is enabled, false otherwise.
         */
        static bool isEnabled() noexcept
        {
            return enabled_;
        }

        /**
         * @brief Performs a depth-first search to detect cycles starting from a given control block.
         * @tparam T The type of object being referenced.
         * @tparam Allocator The allocator type.
         * @param startBlock The control block to start the search from.
         * @return True if a cycle is detected, false otherwise.
         */
        template<typename T, typename Allocator>
        static bool detectCycle(ControlBlock<T, Allocator>* startBlock)
        {
            if (!enabled_ || !startBlock) return false;

            std::unordered_set<void*> visited;
            std::unordered_set<void*> recursionStack;
            std::vector<void*> path;

            return dfsDetectCycle(startBlock, visited, recursionStack, path);
        }

        /**
         * @brief Reports a detected cycle.
         * @param cycle_path The path of pointers that form the cycle.
         */
        static void reportCycle(const std::vector<void*>& cycle_path)
        {
            if (!callback_) return;

            CycleInfo info;
            info.cycle_path = cycle_path;
            info.cycle_length = cycle_path.size();
            info.description = "Detected circular reference chain of length " + std::to_string(info.cycle_length);

            callback_(info);
        }

    private:
        /**
         * @brief Depth-first search implementation for cycle detection.
         * @param current The current control block being visited.
         * @param visited Set of already visited control blocks.
         * @param recursionStack Set of control blocks in the current recursion path.
         * @param path Current path being explored.
         * @return True if a cycle is detected, false otherwise.
         */
        static bool dfsDetectCycle(void* current, 
                                 std::unordered_set<void*>& visited,
                                 std::unordered_set<void*>& recursionStack,
                                 std::vector<void*>& path)
        {
            if (!current) return false;

            // If we've seen this node in the current path, we found a cycle
            if (recursionStack.find(current) != recursionStack.end())
            {
                // Extract the cycle from the path
                auto cycleStart = std::find(path.begin(), path.end(), current);
                if (cycleStart != path.end())
                {
                    std::vector<void*> cyclePath(cycleStart, path.end());
                    cyclePath.push_back(current); // Complete the cycle
                    reportCycle(cyclePath);
                    return true;
                }
            }

            // If already visited and not in recursion stack, no cycle from this node
            if (visited.find(current) != visited.end())
                return false;

            // Mark as visited and add to recursion stack
            visited.insert(current);
            recursionStack.insert(current);
            path.push_back(current);

            // Note: In a real implementation, we would traverse the actual references
            // from the control block. For now, this is a basic framework.
            // The actual implementation would need to examine the object's member variables
            // that are Ref or WeakRef types.

            // Remove from recursion stack and path when done with this node
            recursionStack.erase(current);
            path.pop_back();

            return false;
        }
    };

    /**
     * @brief Utility function to enable cycle detection with a default callback.
     * @param enable True to enable cycle detection.
     * @param logStream Stream to log cycle information to (default: std::cerr).
     */
    inline void enableCycleDetection(bool enable, std::ostream* logStream = &std::cerr)
    {
        CycleDetector::enableDetection(enable);
        
        if (enable)
        {
            CycleDetector::setCycleCallback([logStream](const CycleDetector::CycleInfo& info) {
                if (logStream)
                {
                    (*logStream) << "WARNING: " << info.description << std::endl;
                    (*logStream) << "Cycle path contains " << info.cycle_length << " objects" << std::endl;
                    for (size_t i = 0; i < info.cycle_path.size(); ++i)
                    {
                        (*logStream) << "  [" << i << "] " << info.cycle_path[i] << std::endl;
                    }
                    (*logStream) << std::endl;
                }
            });
        }
        else
        {
            CycleDetector::setCycleCallback(nullptr);
        }
    }
}

#endif //MEXMEMORY_CYCLEDETECTION_H