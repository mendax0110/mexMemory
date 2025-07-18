#ifndef MEXMEMORY_UTILITIES_H
#define MEXMEMORY_UTILITIES_H

#include "strongReference.h"
#include <memory>

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
}

#endif //MEXMEMORY_UTILITIES_H
