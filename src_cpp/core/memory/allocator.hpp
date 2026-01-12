/*
 * ezQuake C++ Port - Memory Allocator
 * 
 * Modern C++ memory management with RAII semantics.
 * Compatible with original Q_malloc/Q_free functions.
 * 
 * Copyright (C) 1996-1997 Id Software, Inc.
 * Copyright (C) 2024 ezQuake Team
 */

#ifndef EZQUAKE_CORE_MEMORY_ALLOCATOR_HPP
#define EZQUAKE_CORE_MEMORY_ALLOCATOR_HPP

#include "../types.hpp"
#include <memory>
#include <functional>

namespace ezquake {
namespace memory {

/**
 * @brief Custom deleter for Quake-style allocations
 */
struct QuakeDeleter {
    void operator()(void* ptr) const noexcept;
};

/**
 * @brief Unique pointer with Quake-style memory management
 */
template<typename T>
using QuakeUniquePtr = std::unique_ptr<T, QuakeDeleter>;

/**
 * @brief Allocate memory with tracking (Q_malloc replacement)
 */
[[nodiscard]] void* allocate(Size size, const char* label = nullptr);

/**
 * @brief Allocate zeroed memory (Q_calloc replacement)
 */
[[nodiscard]] void* allocateZeroed(Size count, Size size, const char* label = nullptr);

/**
 * @brief Reallocate memory (Q_realloc replacement)
 */
[[nodiscard]] void* reallocate(void* ptr, Size newSize, const char* label = nullptr);

/**
 * @brief Free memory (Q_free replacement)
 */
void deallocate(void* ptr) noexcept;

/**
 * @brief Duplicate string (Q_strdup replacement)
 */
[[nodiscard]] char* duplicateString(const char* src, const char* label = nullptr);

/**
 * @brief Get total allocated memory
 */
[[nodiscard]] Size getTotalAllocated() noexcept;

/**
 * @brief Get allocation count
 */
[[nodiscard]] Size getAllocationCount() noexcept;

/**
 * @brief Print memory usage report
 */
void printMemoryReport();

/**
 * @brief RAII wrapper for arbitrary allocations
 */
template<typename T>
class ScopedAllocation {
public:
    explicit ScopedAllocation(Size count = 1, const char* label = nullptr)
        : ptr_(static_cast<T*>(allocate(count * sizeof(T), label)))
        , count_(count)
    {}
    
    ~ScopedAllocation() {
        if (ptr_) {
            deallocate(ptr_);
        }
    }
    
    // Non-copyable
    ScopedAllocation(const ScopedAllocation&) = delete;
    ScopedAllocation& operator=(const ScopedAllocation&) = delete;
    
    // Movable
    ScopedAllocation(ScopedAllocation&& other) noexcept
        : ptr_(other.ptr_), count_(other.count_) {
        other.ptr_ = nullptr;
        other.count_ = 0;
    }
    
    ScopedAllocation& operator=(ScopedAllocation&& other) noexcept {
        if (this != &other) {
            if (ptr_) deallocate(ptr_);
            ptr_ = other.ptr_;
            count_ = other.count_;
            other.ptr_ = nullptr;
            other.count_ = 0;
        }
        return *this;
    }
    
    [[nodiscard]] T* get() noexcept { return ptr_; }
    [[nodiscard]] const T* get() const noexcept { return ptr_; }
    
    [[nodiscard]] T& operator[](Size i) noexcept { return ptr_[i]; }
    [[nodiscard]] const T& operator[](Size i) const noexcept { return ptr_[i]; }
    
    [[nodiscard]] T* operator->() noexcept { return ptr_; }
    [[nodiscard]] const T* operator->() const noexcept { return ptr_; }
    
    [[nodiscard]] T& operator*() noexcept { return *ptr_; }
    [[nodiscard]] const T& operator*() const noexcept { return *ptr_; }
    
    [[nodiscard]] Size count() const noexcept { return count_; }
    
    [[nodiscard]] T* release() noexcept {
        T* tmp = ptr_;
        ptr_ = nullptr;
        count_ = 0;
        return tmp;
    }
    
    explicit operator bool() const noexcept { return ptr_ != nullptr; }

private:
    T* ptr_ = nullptr;
    Size count_ = 0;
};

} // namespace memory
} // namespace ezquake

#endif // EZQUAKE_CORE_MEMORY_ALLOCATOR_HPP
