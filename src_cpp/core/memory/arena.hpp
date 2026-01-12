/*
 * ezQuake C++ Port - Arena Allocator (Hunk)
 * 
 * Stack-based memory allocator compatible with original Hunk system.
 * Provides RAII wrappers around Quake's hunk allocation.
 * 
 * Thread Safety:
 *   - Arena: Thread-safe when constructed with ThreadSafe::Yes
 *   - ArenaScope: NOT thread-safe - use one scope per thread
 *   - TempArena: NOT thread-safe - use one per thread
 * 
 * Copyright (C) 1996-1997 Id Software, Inc.
 * Copyright (C) 2024 ezQuake Team
 */

#ifndef EZQUAKE_CORE_MEMORY_ARENA_HPP
#define EZQUAKE_CORE_MEMORY_ARENA_HPP

#include "../types.hpp"
#include <memory>
#include <shared_mutex>
#include <atomic>

namespace ezquake {
namespace memory {

/// Thread safety mode for Arena
enum class ThreadSafe : bool {
    No = false,
    Yes = true
};

/**
 * @brief Stack-based arena allocator (Hunk replacement)
 * 
 * Provides fast allocation from a pre-allocated memory block.
 * Memory is freed in LIFO order using marks.
 * 
 * Thread Safety:
 *   When constructed with ThreadSafe::Yes, all operations are thread-safe.
 *   When constructed with ThreadSafe::No, no synchronization is performed
 *   for maximum performance (use when arena is owned by a single thread).
 */
class Arena : public NonCopyable {
public:
    /**
     * @brief Construct arena with given size
     * @param size Size in bytes
     * @param threadSafe Enable thread-safe operations
     */
    explicit Arena(Size size, ThreadSafe threadSafe = ThreadSafe::No);
    
    /**
     * @brief Destructor - frees underlying memory
     */
    ~Arena();
    
    /**
     * @brief Allocate memory with name for debugging
     * Thread-safe if constructed with ThreadSafe::Yes
     */
    [[nodiscard]] void* allocate(Size size, const char* name = nullptr);
    
    /**
     * @brief Allocate memory for array of T
     */
    template<typename T>
    [[nodiscard]] T* allocateArray(Size count, const char* name = nullptr) {
        return static_cast<T*>(allocate(count * sizeof(T), name));
    }
    
    /**
     * @brief Get current low mark
     * Thread-safe if constructed with ThreadSafe::Yes
     */
    [[nodiscard]] Size getLowMark() const noexcept;
    
    /**
     * @brief Free all allocations back to mark
     * Thread-safe if constructed with ThreadSafe::Yes
     */
    void freeToMark(Size mark);
    
    /**
     * @brief Get total size
     */
    [[nodiscard]] Size getSize() const noexcept { return size_; }
    
    /**
     * @brief Get used memory
     * Thread-safe if constructed with ThreadSafe::Yes
     */
    [[nodiscard]] Size getUsed() const noexcept;
    
    /**
     * @brief Get free memory
     * Thread-safe if constructed with ThreadSafe::Yes
     */
    [[nodiscard]] Size getFree() const noexcept;
    
    /**
     * @brief Check arena consistency
     * Thread-safe if constructed with ThreadSafe::Yes
     */
    void check() const;
    
    /**
     * @brief Print arena usage
     * Thread-safe if constructed with ThreadSafe::Yes
     */
    void print(bool all = false) const;
    
    /**
     * @brief Check if thread-safe mode is enabled
     */
    [[nodiscard]] bool isThreadSafe() const noexcept { return threadSafe_; }

private:
    Byte* base_ = nullptr;
    Size size_ = 0;
    std::atomic<Size> lowUsed_{0};  // Atomic for thread-safe reads
    bool threadSafe_ = false;
    mutable std::shared_mutex mutex_;  // Only used when threadSafe_ is true
};

/**
 * @brief RAII mark for arena allocations
 * 
 * Automatically frees all allocations made after mark creation
 * when the ArenaScope goes out of scope.
 */
class ArenaScope : public NonCopyable {
public:
    explicit ArenaScope(Arena& arena)
        : arena_(arena)
        , mark_(arena.getLowMark())
    {}
    
    ~ArenaScope() {
        arena_.freeToMark(mark_);
    }
    
    /**
     * @brief Allocate memory in this scope
     */
    [[nodiscard]] void* allocate(Size size, const char* name = nullptr) {
        return arena_.allocate(size, name);
    }
    
    template<typename T>
    [[nodiscard]] T* allocateArray(Size count, const char* name = nullptr) {
        return arena_.allocateArray<T>(count, name);
    }

private:
    Arena& arena_;
    Size mark_;
};

/**
 * @brief Temporary allocation from arena
 * 
 * Similar to ArenaScope but tracks a "temp" region that can
 * be flushed explicitly.
 */
class TempArena : public NonCopyable {
public:
    explicit TempArena(Arena& arena);
    ~TempArena();
    
    [[nodiscard]] void* allocate(Size size);
    [[nodiscard]] void* allocateMore(Size size);
    void flush();

private:
    Arena& arena_;
    Size mark_ = 0;
    bool active_ = false;
};

} // namespace memory
} // namespace ezquake

#endif // EZQUAKE_CORE_MEMORY_ARENA_HPP
