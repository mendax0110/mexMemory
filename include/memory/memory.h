#ifndef MEXMEMORY_MEMORY_H
#define MEXMEMORY_MEMORY_H

#include "refCounting/strongReference.h"
#include "refCounting/weakReference.h"
#include "refCounting/utilities.h"
#include "refCounting/allocationMap.h"
#include "refCounting/referenceCasting.h"
#include "refCounting/cycleDetection.h"
#include "refCounting/stdInterop.h"

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
    
    // Enhanced pointer casting functions
    using refCounting::static_pointer_cast;
    using refCounting::dynamic_pointer_cast;
    using refCounting::const_pointer_cast;
    using refCounting::reinterpret_pointer_cast;
    
    // Cycle detection functionality
    using refCounting::CycleDetector;
    using refCounting::enableCycleDetection;
    
    // std::shared_ptr interoperability
    using refCounting::to_shared_ptr;
    using refCounting::from_shared_ptr;
    using refCounting::adopt_raw_ptr;
    using refCounting::DualRefObject;
    using refCounting::makeDualRef;
}

#endif //MEXMEMORY_MEMORY_H
