/*
 * ezQuake C++ Port - Memory Module Tests
 * 
 * Tests for allocator and arena with thread safety validation.
 * 
 * Copyright (C) 2024 ezQuake Team
 */

#include <gtest/gtest.h>
#include "core/memory/allocator.hpp"
#include "core/memory/arena.hpp"
#include <thread>
#include <vector>
#include <atomic>
#include <random>
#include <chrono>
#include <cstring>

using namespace ezquake;
using namespace ezquake::memory;

//=============================================================================
// Allocator Basic Tests
//=============================================================================

class AllocatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset stats at start of each test
        startAllocs_ = getAllocationCount();
    }
    
    Size startAllocs_ = 0;
};

TEST_F(AllocatorTest, BasicAllocateAndDeallocate) {
    void* ptr = allocate(1024);
    ASSERT_NE(ptr, nullptr);
    
    // Write to memory to ensure it's valid
    std::memset(ptr, 0xAB, 1024);
    
    deallocate(ptr);
}

TEST_F(AllocatorTest, AllocateZeroed) {
    constexpr Size count = 100;
    constexpr Size size = sizeof(int);
    
    int* arr = static_cast<int*>(allocateZeroed(count, size));
    ASSERT_NE(arr, nullptr);
    
    // Verify all bytes are zero
    for (Size i = 0; i < count; i++) {
        EXPECT_EQ(arr[i], 0);
    }
    
    deallocate(arr);
}

TEST_F(AllocatorTest, Reallocate) {
    // Initial allocation
    char* ptr = static_cast<char*>(allocate(100));
    ASSERT_NE(ptr, nullptr);
    
    // Write pattern
    for (int i = 0; i < 100; i++) {
        ptr[i] = static_cast<char>(i);
    }
    
    // Reallocate larger
    ptr = static_cast<char*>(reallocate(ptr, 200));
    ASSERT_NE(ptr, nullptr);
    
    // Verify original data preserved
    for (int i = 0; i < 100; i++) {
        EXPECT_EQ(ptr[i], static_cast<char>(i));
    }
    
    // Write to new space
    for (int i = 100; i < 200; i++) {
        ptr[i] = static_cast<char>(i);
    }
    
    deallocate(ptr);
}

TEST_F(AllocatorTest, ReallocateSmallerDoesNotCrash) {
    char* ptr = static_cast<char*>(allocate(1000));
    ASSERT_NE(ptr, nullptr);
    
    for (int i = 0; i < 1000; i++) {
        ptr[i] = static_cast<char>(i % 256);
    }
    
    // Reallocate smaller
    ptr = static_cast<char*>(reallocate(ptr, 100));
    ASSERT_NE(ptr, nullptr);
    
    // Verify first part of data preserved
    for (int i = 0; i < 100; i++) {
        EXPECT_EQ(ptr[i], static_cast<char>(i % 256));
    }
    
    deallocate(ptr);
}

TEST_F(AllocatorTest, ReallocateNullActsLikeAllocate) {
    void* ptr = reallocate(nullptr, 100);
    ASSERT_NE(ptr, nullptr);
    deallocate(ptr);
}

TEST_F(AllocatorTest, ReallocateZeroActsLikeFree) {
    void* ptr = allocate(100);
    ASSERT_NE(ptr, nullptr);
    
    void* result = reallocate(ptr, 0);
    EXPECT_EQ(result, nullptr);
    // ptr is now freed, don't deallocate again
}

TEST_F(AllocatorTest, DuplicateString) {
    const char* original = "Hello, World!";
    char* copy = duplicateString(original);
    
    ASSERT_NE(copy, nullptr);
    EXPECT_STREQ(copy, original);
    EXPECT_NE(copy, original);  // Different pointer
    
    deallocate(copy);
}

TEST_F(AllocatorTest, DuplicateEmptyString) {
    const char* original = "";
    char* copy = duplicateString(original);
    
    ASSERT_NE(copy, nullptr);
    EXPECT_STREQ(copy, "");
    EXPECT_EQ(copy[0], '\0');
    
    deallocate(copy);
}

TEST_F(AllocatorTest, AlignedAllocation) {
    // All allocations should be at least 8-byte aligned
    for (Size size = 1; size <= 256; size++) {
        void* ptr = allocate(size);
        ASSERT_NE(ptr, nullptr);
        EXPECT_EQ(reinterpret_cast<std::uintptr_t>(ptr) % 8, 0u) 
            << "Allocation of size " << size << " is not 8-byte aligned";
        deallocate(ptr);
    }
}

TEST_F(AllocatorTest, NullDeallocateIsSafe) {
    // Should not crash
    deallocate(nullptr);
}

//=============================================================================
// Allocator Thread Safety Tests
//=============================================================================

class AllocatorThreadSafetyTest : public ::testing::Test {
protected:
    static constexpr int kNumThreads = 8;
    static constexpr int kOpsPerThread = 1000;
};

TEST_F(AllocatorThreadSafetyTest, ConcurrentAllocateDeallocate) {
    std::atomic<int> successCount{0};
    std::vector<std::thread> threads;
    
    for (int t = 0; t < kNumThreads; t++) {
        threads.emplace_back([&successCount]() {
            std::vector<void*> ptrs;
            ptrs.reserve(kOpsPerThread);
            
            for (int i = 0; i < kOpsPerThread; i++) {
                Size size = static_cast<Size>(64 + (i % 256));  // Varying sizes
                void* ptr = allocate(size);
                if (ptr) {
                    // Write to the memory to catch corruption
                    std::memset(ptr, 0xAB, size);
                    ptrs.push_back(ptr);
                    successCount++;
                }
            }
            
            // Deallocate all
            for (void* ptr : ptrs) {
                deallocate(ptr);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    EXPECT_EQ(successCount.load(), kNumThreads * kOpsPerThread);
}

TEST_F(AllocatorThreadSafetyTest, ConcurrentMixedOperations) {
    // This test does mixed allocate/deallocate/reallocate from multiple threads
    std::atomic<int> errors{0};
    std::vector<std::thread> threads;
    
    for (int t = 0; t < kNumThreads; t++) {
        threads.emplace_back([&errors, t]() {
            std::mt19937 rng(static_cast<unsigned>(t * 12345));
            std::vector<std::pair<void*, Size>> ptrs;
            
            for (int i = 0; i < kOpsPerThread; i++) {
                int op = static_cast<int>(rng() % 4);
                
                if (op == 0 || ptrs.empty()) {
                    // Allocate
                    Size size = static_cast<Size>(16 + static_cast<int>(rng() % 512));
                    void* ptr = allocate(size);
                    if (ptr) {
                        std::memset(ptr, 0xCD, size);
                        ptrs.push_back({ptr, size});
                    } else {
                        errors++;
                    }
                } else if (op == 1 && !ptrs.empty()) {
                    // Deallocate random
                    Size idx = rng() % ptrs.size();
                    deallocate(ptrs[idx].first);
                    ptrs.erase(ptrs.begin() + static_cast<std::ptrdiff_t>(idx));
                } else if (op == 2 && !ptrs.empty()) {
                    // Reallocate random
                    Size idx = rng() % ptrs.size();
                    Size newSize = static_cast<Size>(16 + static_cast<int>(rng() % 512));
                    void* newPtr = reallocate(ptrs[idx].first, newSize);
                    if (newPtr) {
                        ptrs[idx] = {newPtr, newSize};
                    } else {
                        // realloc to 0 returns null
                        if (newSize != 0) errors++;
                    }
                } else {
                    // Just allocate and immediately free
                    Size size = static_cast<Size>(16 + static_cast<int>(rng() % 256));
                    void* ptr = allocate(size);
                    if (ptr) {
                        std::memset(ptr, 0xEF, size);
                        deallocate(ptr);
                    }
                }
            }
            
            // Cleanup remaining
            for (auto& [ptr, size] : ptrs) {
                deallocate(ptr);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    EXPECT_EQ(errors.load(), 0);
}

TEST_F(AllocatorThreadSafetyTest, AllocationCountersAreConsistent) {
    // Test that the atomic counters don't lose updates under contention
    Size startAllocs = getAllocationCount();
    
    constexpr Size allocSize = 128;
    constexpr int numAllocs = kNumThreads * kOpsPerThread;
    
    std::vector<void*> allPtrs(static_cast<size_t>(numAllocs));
    std::vector<std::thread> threads;
    
    // All threads allocate
    for (int t = 0; t < kNumThreads; t++) {
        threads.emplace_back([&allPtrs, t]() {
            for (int i = 0; i < kOpsPerThread; i++) {
                size_t idx = static_cast<size_t>(t * kOpsPerThread + i);
                allPtrs[idx] = allocate(allocSize);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Check allocation count increased by expected amount
    Size allocDelta = getAllocationCount() - startAllocs;
    EXPECT_EQ(allocDelta, static_cast<Size>(numAllocs));
    
    // Deallocate all
    threads.clear();
    for (int t = 0; t < kNumThreads; t++) {
        threads.emplace_back([&allPtrs, t]() {
            for (int i = 0; i < kOpsPerThread; i++) {
                size_t idx = static_cast<size_t>(t * kOpsPerThread + i);
                deallocate(allPtrs[idx]);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
}

//=============================================================================
// Arena Basic Tests
//=============================================================================

class ArenaTest : public ::testing::Test {
protected:
    static constexpr Size kArenaSize = 1024 * 1024;  // 1 MB
};

TEST_F(ArenaTest, BasicAllocation) {
    Arena arena(kArenaSize);
    
    void* ptr = arena.allocate(1024);
    ASSERT_NE(ptr, nullptr);
    
    // Write to verify it's usable
    std::memset(ptr, 0xAB, 1024);
    
    // Should have used at least the allocation size
    EXPECT_GE(arena.getUsed(), 1024u);
}

TEST_F(ArenaTest, MultipleAllocations) {
    Arena arena(kArenaSize);
    
    void* ptr1 = arena.allocate(100);
    void* ptr2 = arena.allocate(200);
    void* ptr3 = arena.allocate(300);
    
    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);
    ASSERT_NE(ptr3, nullptr);
    
    // Pointers should be sequential (with alignment padding)
    EXPECT_GT(reinterpret_cast<std::uintptr_t>(ptr2), 
              reinterpret_cast<std::uintptr_t>(ptr1));
    EXPECT_GT(reinterpret_cast<std::uintptr_t>(ptr3), 
              reinterpret_cast<std::uintptr_t>(ptr2));
}

TEST_F(ArenaTest, FreeToMark) {
    Arena arena(kArenaSize);
    
    // Allocate some initial data
    void* ptr1 = arena.allocate(1000);
    ASSERT_NE(ptr1, nullptr);
    std::memset(ptr1, 0x11, 1000);
    
    // Save mark
    auto mark = arena.getLowMark();
    
    // Allocate more
    void* ptr2 = arena.allocate(2000);
    ASSERT_NE(ptr2, nullptr);
    std::memset(ptr2, 0x22, 2000);
    
    // Verify usage increased
    Size usedWithMore = arena.getUsed();
    EXPECT_GT(usedWithMore, mark);
    
    // Free back to mark
    arena.freeToMark(mark);
    
    // Usage should be back to original
    EXPECT_EQ(arena.getUsed(), mark);
}

TEST_F(ArenaTest, AllocatorOverflow) {
    Arena arena(1024);  // Small arena
    
    // Should succeed
    void* ptr1 = arena.allocate(400);
    ASSERT_NE(ptr1, nullptr);
    
    // Should also succeed  
    void* ptr2 = arena.allocate(256);
    ASSERT_NE(ptr2, nullptr);
    
    // Should throw - not enough space
    EXPECT_THROW(arena.allocate(512), std::runtime_error);
}

TEST_F(ArenaTest, ArenaScopeBasic) {
    Arena arena(kArenaSize);
    
    void* ptr1 = arena.allocate(100);
    ASSERT_NE(ptr1, nullptr);
    Size usedBefore = arena.getUsed();
    
    {
        ArenaScope scope(arena);
        
        void* ptr2 = arena.allocate(200);
        ASSERT_NE(ptr2, nullptr);
        void* ptr3 = arena.allocate(300);
        ASSERT_NE(ptr3, nullptr);
        
        EXPECT_GT(arena.getUsed(), usedBefore);
    }
    
    // After scope, usage should be restored
    EXPECT_EQ(arena.getUsed(), usedBefore);
}

TEST_F(ArenaTest, ThreadSafeMode) {
    Arena arena(kArenaSize, ThreadSafe::Yes);
    
    EXPECT_TRUE(arena.isThreadSafe());
    
    // Basic operations should still work
    void* ptr = arena.allocate(1024);
    ASSERT_NE(ptr, nullptr);
    std::memset(ptr, 0xAB, 1024);
}

TEST_F(ArenaTest, NonThreadSafeMode) {
    Arena arena(kArenaSize, ThreadSafe::No);
    
    EXPECT_FALSE(arena.isThreadSafe());
    
    void* ptr = arena.allocate(1024);
    ASSERT_NE(ptr, nullptr);
}

//=============================================================================
// Arena Thread Safety Tests
//=============================================================================

class ArenaThreadSafetyTest : public ::testing::Test {
protected:
    static constexpr Size kArenaSize = 16 * 1024 * 1024;  // 16 MB
    static constexpr int kNumThreads = 8;
    static constexpr int kOpsPerThread = 1000;
};

TEST_F(ArenaThreadSafetyTest, ConcurrentAllocations) {
    Arena arena(kArenaSize, ThreadSafe::Yes);
    
    std::atomic<int> successCount{0};
    std::atomic<int> failCount{0};
    std::vector<std::thread> threads;
    
    for (int t = 0; t < kNumThreads; t++) {
        threads.emplace_back([&arena, &successCount, &failCount]() {
            for (int i = 0; i < kOpsPerThread; i++) {
                Size size = static_cast<Size>(64 + (i % 256));
                void* ptr = arena.allocate(size);
                if (ptr) {
                    // Write pattern to detect memory corruption
                    std::memset(ptr, 0xAB, size);
                    successCount++;
                } else {
                    failCount++;
                }
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    // All allocations should succeed (arena is large enough)
    EXPECT_EQ(successCount.load(), kNumThreads * kOpsPerThread);
    EXPECT_EQ(failCount.load(), 0);
}

TEST_F(ArenaThreadSafetyTest, ConcurrentGetMark) {
    Arena arena(kArenaSize, ThreadSafe::Yes);
    
    // Pre-allocate some memory
    (void)arena.allocate(1024);
    
    std::vector<Size> marks(static_cast<size_t>(kNumThreads * kOpsPerThread));
    std::vector<std::thread> threads;
    
    for (int t = 0; t < kNumThreads; t++) {
        threads.emplace_back([&arena, &marks, t]() {
            for (int i = 0; i < kOpsPerThread; i++) {
                // Interleave allocate and getMark
                (void)arena.allocate(16);
                size_t idx = static_cast<size_t>(t * kOpsPerThread + i);
                marks[idx] = arena.getLowMark();
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    // Verify marks are all valid (>= some minimum)
    for (const auto& mark : marks) {
        EXPECT_GE(mark, 1024u);  // At least the initial allocation
    }
}

TEST_F(ArenaThreadSafetyTest, ReaderWriterPattern) {
    // Multiple readers calling getUsed/getFree while one writer allocates
    Arena arena(kArenaSize, ThreadSafe::Yes);
    
    std::atomic<bool> done{false};
    std::atomic<Size> maxUsedSeen{0};
    std::atomic<Size> readCount{0};
    std::vector<std::thread> readers;
    
    // Start reader threads
    for (int t = 0; t < kNumThreads - 1; t++) {
        readers.emplace_back([&arena, &done, &maxUsedSeen, &readCount]() {
            while (!done) {
                Size used = arena.getUsed();
                Size free = arena.getFree();
                
                // Both values should be reasonable (no crashes, no garbage)
                EXPECT_LE(used, kArenaSize);
                EXPECT_LE(free, kArenaSize);
                
                // Track max used seen
                Size expected = maxUsedSeen.load();
                while (used > expected && !maxUsedSeen.compare_exchange_weak(expected, used)) {}
                
                readCount++;
                std::this_thread::yield();
            }
        });
    }
    
    // Writer thread
    std::thread writer([&arena, &done]() {
        for (int i = 0; i < kOpsPerThread; i++) {
            (void)arena.allocate(128);
            std::this_thread::yield();
        }
        done = true;
    });
    
    writer.join();
    for (auto& r : readers) {
        r.join();
    }
    
    // Readers should have seen usage grow and done many reads
    EXPECT_GT(maxUsedSeen.load(), 0u);
    EXPECT_GT(readCount.load(), 0u);
}

TEST_F(ArenaThreadSafetyTest, DataIntegrityUnderContention) {
    // Verify that concurrent allocations don't corrupt each other's memory
    Arena arena(kArenaSize, ThreadSafe::Yes);
    
    struct Allocation {
        void* ptr;
        Size size;
        Byte pattern;
    };
    
    std::vector<std::vector<Allocation>> threadAllocs(static_cast<size_t>(kNumThreads));
    std::vector<std::thread> threads;
    std::atomic<bool> hasError{false};
    
    // Each thread allocates and writes a unique pattern
    for (int t = 0; t < kNumThreads; t++) {
        threads.emplace_back([&arena, &threadAllocs, &hasError, t]() {
            Byte pattern = static_cast<Byte>(t + 1);
            
            for (int i = 0; i < kOpsPerThread && !hasError; i++) {
                Size size = static_cast<Size>(32 + (i % 64));
                void* ptr = arena.allocate(size);
                if (ptr) {
                    std::memset(ptr, pattern, size);
                    threadAllocs[static_cast<size_t>(t)].push_back({ptr, size, pattern});
                }
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify all patterns are intact
    for (int t = 0; t < kNumThreads; t++) {
        for (const auto& alloc : threadAllocs[static_cast<size_t>(t)]) {
            const Byte* data = static_cast<const Byte*>(alloc.ptr);
            for (Size i = 0; i < alloc.size; i++) {
                if (data[i] != alloc.pattern) {
                    hasError = true;
                    FAIL() << "Memory corruption detected at thread " << t 
                           << ", offset " << i << ": expected " << static_cast<int>(alloc.pattern)
                           << ", got " << static_cast<int>(data[i]);
                }
            }
        }
    }
    
    EXPECT_FALSE(hasError.load());
}

//=============================================================================
// Stress Tests
//=============================================================================

class MemoryStressTest : public ::testing::Test {};

TEST_F(MemoryStressTest, AllocatorHighContention) {
    // Very high contention test with many small allocations
    constexpr int kNumThreads = 16;
    constexpr int kOpsPerThread = 5000;
    
    std::atomic<Size> totalAllocated{0};
    std::atomic<Size> totalDeallocated{0};
    std::vector<std::thread> threads;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int t = 0; t < kNumThreads; t++) {
        threads.emplace_back([&totalAllocated, &totalDeallocated]() {
            std::vector<void*> ptrs;
            ptrs.reserve(100);
            
            for (int i = 0; i < kOpsPerThread; i++) {
                if (ptrs.size() < 100 && (i % 3 != 0 || ptrs.empty())) {
                    // Allocate
                    void* ptr = allocate(static_cast<Size>(32 + (i % 128)));
                    if (ptr) {
                        ptrs.push_back(ptr);
                        totalAllocated++;
                    }
                } else if (!ptrs.empty()) {
                    // Deallocate oldest
                    deallocate(ptrs.front());
                    ptrs.erase(ptrs.begin());
                    totalDeallocated++;
                }
            }
            
            // Cleanup
            for (void* ptr : ptrs) {
                deallocate(ptr);
                totalDeallocated++;
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // All allocations should be deallocated
    EXPECT_EQ(totalAllocated.load(), totalDeallocated.load());
    
    // Print performance info (informational)
    Size totalOps = totalAllocated.load() + totalDeallocated.load();
    if (duration.count() > 0) {
        double opsPerSec = (static_cast<double>(totalOps) * 1000.0) / static_cast<double>(duration.count());
        std::cout << "Allocator stress test: " << totalOps << " ops in " 
                  << duration.count() << "ms (" << opsPerSec << " ops/sec)" << std::endl;
    }
}

TEST_F(MemoryStressTest, ArenaHighContention) {
    constexpr Size kArenaSize = 64 * 1024 * 1024;  // 64 MB
    constexpr int kNumThreads = 16;
    constexpr int kOpsPerThread = 10000;
    
    Arena arena(kArenaSize, ThreadSafe::Yes);
    
    std::atomic<Size> successfulAllocs{0};
    std::vector<std::thread> threads;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int t = 0; t < kNumThreads; t++) {
        threads.emplace_back([&arena, &successfulAllocs]() {
            for (int i = 0; i < kOpsPerThread; i++) {
                Size size = static_cast<Size>(16 + (i % 128));
                void* ptr = arena.allocate(size);
                if (ptr) {
                    // Quick write to ensure memory is valid
                    *static_cast<Byte*>(ptr) = 0xAB;
                    successfulAllocs++;
                }
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should have many successful allocations
    EXPECT_GT(successfulAllocs.load(), 0u);
    
    // Print performance info
    if (duration.count() > 0) {
        double opsPerSec = (static_cast<double>(successfulAllocs.load()) * 1000.0) / static_cast<double>(duration.count());
        std::cout << "Arena stress test: " << successfulAllocs.load() << " allocs in "
                  << duration.count() << "ms (" << opsPerSec << " ops/sec)" << std::endl;
    }
}

//=============================================================================
// ScopedAllocation Tests
//=============================================================================

TEST(ScopedAllocationTest, BasicUsage) {
    {
        ScopedAllocation<int> arr(100);
        ASSERT_NE(arr.get(), nullptr);
        
        for (int i = 0; i < 100; i++) {
            arr.get()[i] = i * 2;
        }
        
        for (int i = 0; i < 100; i++) {
            EXPECT_EQ(arr.get()[i], i * 2);
        }
    }
    // Memory should be freed here
}

TEST(ScopedAllocationTest, MoveSemantics) {
    ScopedAllocation<char> original(256);
    ASSERT_NE(original.get(), nullptr);
    
    char* originalPtr = original.get();
    std::memset(originalPtr, 'X', 256);
    
    // Move to new owner
    ScopedAllocation<char> moved = std::move(original);
    
    EXPECT_EQ(original.get(), nullptr);  // Original is null after move
    EXPECT_EQ(moved.get(), originalPtr);  // Moved has the pointer
    
    // Data should still be intact
    for (int i = 0; i < 256; i++) {
        EXPECT_EQ(moved.get()[i], 'X');
    }
}

TEST(ScopedAllocationTest, SubscriptOperator) {
    ScopedAllocation<int> arr(10);
    
    for (Size i = 0; i < 10; i++) {
        arr[i] = static_cast<int>(i * i);
    }
    
    for (Size i = 0; i < 10; i++) {
        EXPECT_EQ(arr[i], static_cast<int>(i * i));
    }
}
