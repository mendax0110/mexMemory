#ifndef MEXMEMORY_MEMORY_H
#define MEXMEMORY_MEMORY_H

#include "refCounting/strongReference.h"
#include "refCounting/weakReference.h"
#include "refCounting/utilities.h"
#include "refCounting/allocationMap.h"

/// @brief Namespace for memory management with reference counting \namespace memory
namespace memory
{
    using refCounting::Ref;
    using refCounting::WeakRef;
    using refCounting::makeRef;
    using refCounting::makeRefWithAllocator;
    using refCounting::enableReferenceDebugging;
    using refCounting::enableAllocationTracking;
    using refCounting::DefaultAllocator;
    using refCounting::AllocationTracker;
}

#endif //MEXMEMORY_MEMORY_H
