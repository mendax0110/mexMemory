#ifndef MEXMEMORY_UTILITIES_H
#define MEXMEMORY_UTILITIES_H

#include "strongReference.h"
#include <memory>
#include "allocationMap.h"

/// @brief Namespace for memory management with reference counting \namespace memory::refCounting
namespace memory::refCounting
{
    /**
     * @brief Debug configuration for reference counting memory management.
     * @param enable The flag to enable or disable debugging.
     * @param stream The output stream to log debug messages to. Defaults to std::cout.
     */
    inline void enableReferenceDebugging(bool enable, std::ostream* stream = &std::cout)
    {
        DebugConfig::enableLogging = enable;
        DebugConfig::logStream = stream;
    }

    /**
     * @brief Enables or disables memory allocation tracking.
     * @param enable The flag to enable or disable allocation tracking.
     * @param breakOnLeak The flag to break on memory leaks. Defaults to false.
     * @param stream The output stream to log memory leak reports to. Defaults to std::cerr.
     */
    inline void enableAllocationTracking(bool enable, bool breakOnLeak = false, std::ostream* stream = &std::cerr)
    {
        AllocationTracker::enableTracking(enable);
        AllocationTracker::setBreakOnLeak(breakOnLeak);
        AllocationTracker::setLeakStream(stream);
    }
}

#endif //MEXMEMORY_UTILITIES_H
