/*
 * ezQuake C++ Port - Memory Allocator Implementation
 * 
 * Thread-safe memory allocation with tracking.
 * 
 * Copyright (C) 1996-1997 Id Software, Inc.
 * Copyright (C) 2024 ezQuake Team
 */

#include "allocator.hpp"
#include <cstdlib>
#include <cstring>
#include <atomic>
#include <iostream>
#include <mutex>
#include <unordered_map>

namespace ezquake {
namespace memory {

namespace {

// Allocation header - stored before every allocation for size tracking
// This enables thread-safe tracking without a global map
struct AllocationHeader {
    Size size;          // Size of user data (not including header)
    Size magic;         // Magic number to detect corruption
    
    static constexpr Size MAGIC = 0xE20BABE01ULL;  // "EZ" = E2, readable magic
};

static_assert(sizeof(AllocationHeader) % 16 == 0 || sizeof(AllocationHeader) == 16,
              "AllocationHeader should be aligned");

// Thread-safe counters using atomics
std::atomic<Size> g_totalAllocated{0};
std::atomic<Size> g_allocationCount{0};

#ifdef DEBUG_MEMORY_ALLOCATIONS
std::mutex g_trackingMutex;
std::unordered_map<void*, std::pair<Size, std::string>> g_allocations;
#endif

// Convert user pointer to header pointer
inline AllocationHeader* getHeader(void* userPtr) noexcept {
    return reinterpret_cast<AllocationHeader*>(static_cast<Byte*>(userPtr) - sizeof(AllocationHeader));
}

// Convert header pointer to user pointer
inline void* getUserPtr(AllocationHeader* header) noexcept {
    return reinterpret_cast<Byte*>(header) + sizeof(AllocationHeader);
}

} // anonymous namespace

void QuakeDeleter::operator()(void* ptr) const noexcept {
    deallocate(ptr);
}

void* allocate(Size size, [[maybe_unused]] const char* label) {
    if (size == 0) {
        return nullptr;
    }
    
    // Allocate header + user data
    Size totalSize = sizeof(AllocationHeader) + size;
    void* rawPtr = std::malloc(totalSize);
    if (!rawPtr) {
        std::cerr << "ezQuake: Failed to allocate " << size << " bytes";
        if (label) std::cerr << " for " << label;
        std::cerr << std::endl;
        std::abort();
    }
    
    // Initialize header
    AllocationHeader* header = static_cast<AllocationHeader*>(rawPtr);
    header->size = size;
    header->magic = AllocationHeader::MAGIC;
    
    void* userPtr = getUserPtr(header);
    
    // Update counters atomically
    g_totalAllocated.fetch_add(size, std::memory_order_relaxed);
    g_allocationCount.fetch_add(1, std::memory_order_relaxed);
    
#ifdef DEBUG_MEMORY_ALLOCATIONS
    {
        std::lock_guard<std::mutex> lock(g_trackingMutex);
        g_allocations[userPtr] = {size, label ? label : ""};
    }
#endif
    
    return userPtr;
}

void* allocateZeroed(Size count, Size size, const char* label) {
    Size total = count * size;
    void* ptr = allocate(total, label);
    if (ptr) {
        std::memset(ptr, 0, total);
    }
    return ptr;
}

void* reallocate(void* ptr, Size newSize, [[maybe_unused]] const char* label) {
    if (!ptr) {
        return allocate(newSize, label);
    }
    
    if (newSize == 0) {
        deallocate(ptr);
        return nullptr;
    }
    
    // Get old size from header
    AllocationHeader* oldHeader = getHeader(ptr);
    if (oldHeader->magic != AllocationHeader::MAGIC) {
        std::cerr << "ezQuake: reallocate() on corrupted or invalid pointer" << std::endl;
        std::abort();
    }
    Size oldSize = oldHeader->size;
    
#ifdef DEBUG_MEMORY_ALLOCATIONS
    {
        std::lock_guard<std::mutex> lock(g_trackingMutex);
        g_allocations.erase(ptr);
    }
#endif
    
    // Allocate new block with header
    Size totalNewSize = sizeof(AllocationHeader) + newSize;
    void* newRawPtr = std::realloc(oldHeader, totalNewSize);
    if (!newRawPtr) {
        std::cerr << "ezQuake: Failed to reallocate to " << newSize << " bytes" << std::endl;
        std::abort();
    }
    
    // Update header
    AllocationHeader* newHeader = static_cast<AllocationHeader*>(newRawPtr);
    newHeader->size = newSize;
    newHeader->magic = AllocationHeader::MAGIC;
    
    void* newUserPtr = getUserPtr(newHeader);
    
    // Update counters atomically
    g_totalAllocated.fetch_sub(oldSize, std::memory_order_relaxed);
    g_totalAllocated.fetch_add(newSize, std::memory_order_relaxed);
    
#ifdef DEBUG_MEMORY_ALLOCATIONS
    {
        std::lock_guard<std::mutex> lock(g_trackingMutex);
        g_allocations[newUserPtr] = {newSize, label ? label : ""};
    }
#endif
    
    return newUserPtr;
}

void deallocate(void* ptr) noexcept {
    if (!ptr) return;
    
    // Get size from header
    AllocationHeader* header = getHeader(ptr);
    if (header->magic != AllocationHeader::MAGIC) {
        std::cerr << "ezQuake: deallocate() on corrupted or invalid pointer" << std::endl;
        // Don't abort in noexcept, but log the error
        return;
    }
    
    Size size = header->size;
    
    // Clear magic to detect double-free
    header->magic = 0;
    
    // Update counters atomically
    g_totalAllocated.fetch_sub(size, std::memory_order_relaxed);
    g_allocationCount.fetch_sub(1, std::memory_order_relaxed);
    
#ifdef DEBUG_MEMORY_ALLOCATIONS
    {
        std::lock_guard<std::mutex> lock(g_trackingMutex);
        g_allocations.erase(ptr);
    }
#endif
    
    std::free(header);
}

char* duplicateString(const char* src, const char* label) {
    if (!src) return nullptr;
    
    Size len = std::strlen(src) + 1;
    char* dst = static_cast<char*>(allocate(len, label));
    std::memcpy(dst, src, len);
    return dst;
}

Size getTotalAllocated() noexcept {
    return g_totalAllocated;
}

Size getAllocationCount() noexcept {
    return g_allocationCount;
}

void printMemoryReport() {
    std::cout << "=== ezQuake Memory Report ===" << std::endl;
    std::cout << "Total allocated: " << g_totalAllocated << " bytes" << std::endl;
    std::cout << "Allocation count: " << g_allocationCount << std::endl;
    
#ifdef DEBUG_MEMORY_ALLOCATIONS
    std::lock_guard<std::mutex> lock(g_trackingMutex);
    std::cout << "Active allocations:" << std::endl;
    for (const auto& [ptr, info] : g_allocations) {
        std::cout << "  " << ptr << ": " << info.first << " bytes";
        if (!info.second.empty()) {
            std::cout << " (" << info.second << ")";
        }
        std::cout << std::endl;
    }
#endif
}

} // namespace memory
} // namespace ezquake
