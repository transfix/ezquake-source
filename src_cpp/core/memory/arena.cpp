/*
 * ezQuake C++ Port - Arena Allocator Implementation
 * 
 * Copyright (C) 1996-1997 Id Software, Inc.
 * Copyright (C) 2024 ezQuake Team
 */

#include "arena.hpp"
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <mutex>
#include <shared_mutex>

namespace ezquake {
namespace memory {

namespace {

constexpr UInt32 HUNK_SENTINEL = 0x1df001ed;
constexpr Size ALIGNMENT = 16;

struct HunkHeader {
    UInt32 sentinel;
    Int32 size;  // Including sizeof(HunkHeader)
    char name[8];
};

Size alignUp(Size value, Size alignment) {
    return (value + alignment - 1) & ~(alignment - 1);
}

} // anonymous namespace

//=============================================================================
// Arena
//=============================================================================

Arena::Arena(Size size, ThreadSafe threadSafe) 
    : size_(size)
    , threadSafe_(static_cast<bool>(threadSafe)) 
{
    base_ = static_cast<Byte*>(std::aligned_alloc(ALIGNMENT, size));
    if (!base_) {
        throw std::bad_alloc();
    }
}

Arena::~Arena() {
    std::free(base_);
}

void* Arena::allocate(Size size, const char* name) {
    // Add header size and align
    Size totalSize = alignUp(sizeof(HunkHeader) + size, ALIGNMENT);
    
    if (threadSafe_) {
        std::unique_lock lock(mutex_);
        
        Size currentUsed = lowUsed_.load(std::memory_order_relaxed);
        if (currentUsed + totalSize > size_) {
            throw std::runtime_error("Arena: out of memory");
        }
        
        HunkHeader* header = reinterpret_cast<HunkHeader*>(base_ + currentUsed);
        header->sentinel = HUNK_SENTINEL;
        header->size = static_cast<Int32>(totalSize);
        
        if (name) {
            std::strncpy(header->name, name, sizeof(header->name) - 1);
            header->name[sizeof(header->name) - 1] = '\0';
        } else {
            std::memset(header->name, 0, sizeof(header->name));
        }
        
        lowUsed_.store(currentUsed + totalSize, std::memory_order_release);
        
        return header + 1;
    } else {
        Size currentUsed = lowUsed_.load(std::memory_order_relaxed);
        if (currentUsed + totalSize > size_) {
            throw std::runtime_error("Arena: out of memory");
        }
        
        HunkHeader* header = reinterpret_cast<HunkHeader*>(base_ + currentUsed);
        header->sentinel = HUNK_SENTINEL;
        header->size = static_cast<Int32>(totalSize);
        
        if (name) {
            std::strncpy(header->name, name, sizeof(header->name) - 1);
            header->name[sizeof(header->name) - 1] = '\0';
        } else {
            std::memset(header->name, 0, sizeof(header->name));
        }
        
        lowUsed_.store(currentUsed + totalSize, std::memory_order_relaxed);
        
        return header + 1;
    }
}

Size Arena::getLowMark() const noexcept {
    if (threadSafe_) {
        std::shared_lock lock(mutex_);
        return lowUsed_.load(std::memory_order_acquire);
    }
    return lowUsed_.load(std::memory_order_relaxed);
}

Size Arena::getUsed() const noexcept {
    return getLowMark();
}

Size Arena::getFree() const noexcept {
    return size_ - getLowMark();
}

void Arena::freeToMark(Size mark) {
    if (threadSafe_) {
        std::unique_lock lock(mutex_);
        Size currentUsed = lowUsed_.load(std::memory_order_relaxed);
        if (mark > currentUsed) {
            throw std::runtime_error("Arena: invalid mark");
        }
        lowUsed_.store(mark, std::memory_order_release);
    } else {
        Size currentUsed = lowUsed_.load(std::memory_order_relaxed);
        if (mark > currentUsed) {
            throw std::runtime_error("Arena: invalid mark");
        }
        lowUsed_.store(mark, std::memory_order_relaxed);
    }
}

void Arena::check() const {
    auto checkImpl = [this]() {
        Size offset = 0;
        Size used = lowUsed_.load(std::memory_order_acquire);
        while (offset < used) {
            const HunkHeader* header = reinterpret_cast<const HunkHeader*>(base_ + offset);
            
            if (header->sentinel != HUNK_SENTINEL) {
                throw std::runtime_error("Arena: trashed sentinel");
            }
            
            if (header->size < static_cast<Int32>(sizeof(HunkHeader)) || 
                static_cast<Size>(header->size) + offset > size_) {
                throw std::runtime_error("Arena: bad size");
            }
            
            offset += static_cast<Size>(header->size);
        }
    };
    
    if (threadSafe_) {
        std::shared_lock lock(mutex_);
        checkImpl();
    } else {
        checkImpl();
    }
}

void Arena::print(bool all) const {
    auto printImpl = [this, all]() {
        Size used = lowUsed_.load(std::memory_order_acquire);
        std::cout << "Arena: " << size_ << " bytes total" << std::endl;
        std::cout << "  Low used: " << used << " bytes" << std::endl;
        std::cout << "  Free: " << (size_ - used) << " bytes" << std::endl;
        std::cout << "  Thread-safe: " << (threadSafe_ ? "yes" : "no") << std::endl;
        
        if (all) {
            Size offset = 0;
            while (offset < used) {
                const HunkHeader* header = reinterpret_cast<const HunkHeader*>(base_ + offset);
                
                char name[9];
                std::memcpy(name, header->name, 8);
                name[8] = '\0';
                
                std::cout << "  " << header << ": " << header->size << " bytes";
                if (name[0]) std::cout << " (" << name << ")";
                std::cout << std::endl;
                
                offset += static_cast<Size>(header->size);
            }
        }
    };
    
    if (threadSafe_) {
        std::shared_lock lock(mutex_);
        printImpl();
    } else {
        printImpl();
    }
}

//=============================================================================
// TempArena
//=============================================================================

TempArena::TempArena(Arena& arena) : arena_(arena) {}

TempArena::~TempArena() {
    if (active_) {
        flush();
    }
}

void* TempArena::allocate(Size size) {
    if (active_) {
        flush();
    }
    
    mark_ = arena_.getLowMark();
    active_ = true;
    
    return arena_.allocate(size, "temp");
}

void* TempArena::allocateMore(Size size) {
    if (!active_) {
        return allocate(size);
    }
    return arena_.allocate(size, "temp");
}

void TempArena::flush() {
    if (active_) {
        arena_.freeToMark(mark_);
        active_ = false;
    }
}

} // namespace memory
} // namespace ezquake
