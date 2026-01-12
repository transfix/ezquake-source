/*
 * ezQuake C++ Port - Console Variable System (No Singletons)
 * 
 * This module provides a modern C++20 implementation of the console variable
 * (cvar) system. CVars are used throughout the engine for configuration,
 * debugging, and runtime settings.
 * 
 * Architecture: CvarRegistry is NOT a singleton. It must be created and passed
 * by reference to code that needs it. This enables proper dependency injection,
 * testability, and eliminates global state.
 * 
 * Copyright (C) 1996-1997 Id Software, Inc.
 * Copyright (C) 2024 ezQuake Team
 */

#ifndef EZQUAKE_CORE_CVAR_REGISTRY_HPP
#define EZQUAKE_CORE_CVAR_REGISTRY_HPP

#include "core/types.hpp"
#include "core/text/string_utils.hpp"
#include <unordered_map>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <ranges>
#include <concepts>

namespace ezquake {

//=============================================================================
// Cvar Flags
//=============================================================================

/// Flags that control cvar behavior
enum class CvarFlags : UInt32 {
    None             = 0,
    Archive          = 1 << 0,   // Saved to config (obsolete in ezQuake)
    UserInfo         = 1 << 1,   // Mirrored to userinfo
    ServerInfo       = 1 << 2,   // Mirrored to serverinfo
    ReadOnly         = 1 << 3,   // Cannot be changed by user
    InitOnly         = 1 << 4,   // Can only be set during initialization
    UserCreated      = 1 << 5,   // Created by a set command
    UserArchive      = 1 << 6,   // Created by a seta command (obsolete)
    RulesetMax       = 1 << 7,   // Limited by ruleset (max)
    RulesetMin       = 1 << 8,   // Limited by ruleset (min)
    NoReset          = 1 << 9,   // Don't reset on cfg_load
    Temp             = 1 << 10,  // Created during config execution
    LatchGfx         = 1 << 11,  // Requires vid_restart
    Silent           = 1 << 12,  // No warnings on re-register
    Color            = 1 << 13,  // Parse as color value
    Auto             = 1 << 14,  // Has auto-calculated value
    ModCreated       = 1 << 15,  // Created by mod
    RecompileProgs   = 1 << 16,  // Recompile shaders on change
    TrackerColor     = 1 << 17,  // Parse as tracker color format
    QueuedTrigger    = 1 << 18,  // Found in config, registered later
    AutoSetRecent    = 1 << 19,  // Internal flag for auto-set
    UserInfoNoReset  = 1 << 20,  // UserInfo but don't reset on cfg_load
    LatchSound       = 1 << 21,  // Requires s_restart
    ReloadGfx        = 1 << 22,  // Requires texture reload
    
    // Combined flags
    Latch = LatchGfx | LatchSound,
    UserInfoNoCfgReset = UserInfo | UserInfoNoReset
};

/// Enable bitwise operations on CvarFlags
[[nodiscard]] inline constexpr CvarFlags operator|(CvarFlags a, CvarFlags b) noexcept {
    return static_cast<CvarFlags>(static_cast<UInt32>(a) | static_cast<UInt32>(b));
}
[[nodiscard]] inline constexpr CvarFlags operator&(CvarFlags a, CvarFlags b) noexcept {
    return static_cast<CvarFlags>(static_cast<UInt32>(a) & static_cast<UInt32>(b));
}
[[nodiscard]] inline constexpr CvarFlags operator~(CvarFlags a) noexcept {
    return static_cast<CvarFlags>(~static_cast<UInt32>(a));
}
inline constexpr CvarFlags& operator|=(CvarFlags& a, CvarFlags b) noexcept {
    return a = a | b;
}
inline constexpr CvarFlags& operator&=(CvarFlags& a, CvarFlags b) noexcept {
    return a = a & b;
}
[[nodiscard]] inline constexpr bool hasFlag(CvarFlags flags, CvarFlags test) noexcept {
    return (static_cast<UInt32>(flags) & static_cast<UInt32>(test)) != 0;
}

//=============================================================================
// RGBA Color Type
//=============================================================================

/// RGBA color stored in a cvar
struct CvarColor {
    Byte r = 255;
    Byte g = 255;
    Byte b = 255;
    Byte a = 255;
    
    constexpr CvarColor() noexcept = default;
    constexpr CvarColor(Byte r_, Byte g_, Byte b_, Byte a_ = 255) noexcept
        : r(r_), g(g_), b(b_), a(a_) {}
    
    /// Parse from string "r g b" or "r g b a" (0-255 values)
    [[nodiscard]] static CvarColor fromString(StringView str) noexcept;
    
    /// Parse from tracker format (3 digits 0-9)
    [[nodiscard]] static CvarColor fromTrackerString(StringView str) noexcept;
    
    /// Convert to string
    [[nodiscard]] String toString() const;
    
    [[nodiscard]] constexpr auto operator<=>(const CvarColor&) const noexcept = default;
};

//=============================================================================
// Forward Declarations
//=============================================================================

class Cvar;
class CvarRegistry;

//=============================================================================
// Cvar Change Callback
//=============================================================================

/// Callback type for cvar change notifications
/// Parameters: cvar reference, new value, cancel flag (set to true to reject change)
using CvarCallback = std::function<void(Cvar& var, StringView newValue, bool& cancel)>;

//=============================================================================
// Cvar Class
//=============================================================================

/**
 * @brief Console Variable (Cvar)
 * 
 * CVars store configuration values that can be modified at runtime via the
 * console. They support various types (string, float, int, color, bool) and
 * can have callbacks for change notification.
 * 
 * Thread Safety: Individual cvars are thread-safe through internal locking.
 */
class Cvar {
public:
    friend class CvarRegistry;
    
    //=========================================================================
    // Construction
    //=========================================================================
    
    /// Create a cvar with name, default value, and optional flags
    Cvar(String name, String defaultValue, CvarFlags flags = CvarFlags::None);
    
    /// Create a cvar with callback
    Cvar(String name, String defaultValue, CvarFlags flags, CvarCallback callback);
    
    // Non-copyable but movable
    Cvar(const Cvar&) = delete;
    Cvar& operator=(const Cvar&) = delete;
    Cvar(Cvar&&) noexcept = default;
    Cvar& operator=(Cvar&&) noexcept = default;
    
    //=========================================================================
    // Accessors (all thread-safe)
    //=========================================================================
    
    /// Get the cvar name
    [[nodiscard]] const String& name() const noexcept { return name_; }
    
    /// Get the string value
    [[nodiscard]] String string() const {
        std::shared_lock lock(mutex_);
        return string_;
    }
    
    /// Get as float
    [[nodiscard]] Float value() const noexcept {
        std::shared_lock lock(mutex_);
        return value_;
    }
    
    /// Get as integer
    [[nodiscard]] Int32 integer() const noexcept {
        std::shared_lock lock(mutex_);
        return integer_;
    }
    
    /// Get as boolean (non-zero = true)
    [[nodiscard]] Bool boolean() const noexcept {
        std::shared_lock lock(mutex_);
        return integer_ != 0;
    }
    
    /// Get as color
    [[nodiscard]] CvarColor color() const noexcept {
        std::shared_lock lock(mutex_);
        return color_;
    }
    
    /// Get the default value
    [[nodiscard]] const String& defaultValue() const noexcept { return defaultValue_; }
    
    /// Get the latched value (if any)
    [[nodiscard]] Optional<String> latchedString() const noexcept {
        std::shared_lock lock(mutex_);
        return latchedString_;
    }
    
    /// Get the auto value (if any)
    [[nodiscard]] Optional<String> autoString() const noexcept {
        std::shared_lock lock(mutex_);
        return autoString_;
    }
    
    /// Get flags
    [[nodiscard]] CvarFlags flags() const noexcept {
        std::shared_lock lock(mutex_);
        return flags_;
    }
    
    /// Check if a specific flag is set
    [[nodiscard]] bool hasFlag(CvarFlags flag) const noexcept {
        return ezquake::hasFlag(flags(), flag);
    }
    
    /// Get the group this cvar belongs to
    [[nodiscard]] StringView group() const noexcept {
        std::shared_lock lock(mutex_);
        return group_;
    }
    
    /// Check if modified since last cleared
    [[nodiscard]] bool isModified() const noexcept {
        std::shared_lock lock(mutex_);
        return modified_;
    }
    
    /// Clear modified flag
    void clearModified() noexcept {
        std::unique_lock lock(mutex_);
        modified_ = false;
    }
    
    //=========================================================================
    // Modifiers (all thread-safe)
    //=========================================================================
    
    /// Set string value (respects flags, calls callback)
    /// @return true if value was changed, false if rejected
    bool set(StringView value);
    
    /// Set float value
    bool setValue(Float value);
    
    /// Set integer value
    bool setInteger(Int32 value);
    
    /// Set boolean value
    bool setBoolean(Bool value);
    
    /// Force set (ignores ReadOnly flag)
    void forceSet(StringView value);
    
    /// Set without triggering callback
    void setIgnoreCallback(StringView value);
    
    /// Reset to default value
    void reset();
    
    /// Set flags
    void setFlags(CvarFlags flags) noexcept {
        std::unique_lock lock(mutex_);
        flags_ = flags;
    }
    
    /// Add flags
    void addFlags(CvarFlags flags) noexcept {
        std::unique_lock lock(mutex_);
        flags_ |= flags;
    }
    
    /// Remove flags
    void removeFlags(CvarFlags flags) noexcept {
        std::unique_lock lock(mutex_);
        flags_ &= ~flags;
    }
    
    /// Set auto value (for auto-calculated presentation values)
    void autoSet(StringView value);
    
    /// Clear auto value
    void autoReset();
    
    /// Set latched value (for latched cvars that require restart)
    void latchedSet(StringView value);
    
    /// Apply latched value (called on vid_restart etc.)
    void applyLatched();
    
    /// Set ruleset limits
    void setRulesetMin(Float min);
    void setRulesetMax(Float max);
    void setRulesetLock(Float value); // Sets both min and max
    
    /// Set the change callback
    void setCallback(CvarCallback callback);
    
    /// Force trigger the callback with current value
    bool triggerCallback();
    
    //=========================================================================
    // Conversion Operators
    //=========================================================================
    
    /// Convert to float
    explicit operator Float() const noexcept { return value(); }
    
    /// Convert to int
    explicit operator Int32() const noexcept { return integer(); }
    
    /// Convert to bool
    explicit operator Bool() const noexcept { return boolean(); }
    
private:
    /// Internal set without public API restrictions
    bool setInternal(StringView value, bool ignoreCallback, bool force);
    
    /// Parse string value into numeric types
    void parseValue();
    
    /// Check flag without taking lock (caller must hold lock)
    [[nodiscard]] bool hasFlagUnsafe(CvarFlags flag) const noexcept {
        return (static_cast<uint32_t>(flags_) & static_cast<uint32_t>(flag)) != 0;
    }
    
    String name_;  // Immutable after construction
    String defaultValue_;  // Immutable after construction
    String string_;        // Current value, protected by mutex_
    
    // Protected by mutex_
    mutable std::shared_mutex mutex_;
    CvarFlags flags_ = CvarFlags::None;
    CvarCallback callback_;
    
    Float value_ = 0.0f;
    Int32 integer_ = 0;
    CvarColor color_;
    
    Float minRulesetValue_ = 0.0f;
    Float maxRulesetValue_ = 0.0f;
    
    Optional<String> latchedString_;
    Optional<String> autoString_;
    
    bool modified_ = false;
    bool teamplay_ = false;
    
    String group_;
};

//=============================================================================
// Cvar Registry (NOT A SINGLETON)
//=============================================================================

/**
 * @brief Registry for all console variables
 * 
 * Provides registration, lookup, and iteration over all cvars.
 * 
 * IMPORTANT: This is NOT a singleton. Create an instance and pass it
 * by reference to code that needs it. This enables:
 * - Proper dependency injection
 * - Easy testing with isolated registries
 * - No global state
 * 
 * Thread Safety: Fully thread-safe for all operations.
 */
class CvarRegistry {
public:
    /// Create an empty registry
    CvarRegistry() = default;
    ~CvarRegistry() = default;
    
    // Non-copyable but movable
    CvarRegistry(const CvarRegistry&) = delete;
    CvarRegistry& operator=(const CvarRegistry&) = delete;
    CvarRegistry(CvarRegistry&&) noexcept = default;
    CvarRegistry& operator=(CvarRegistry&&) noexcept = default;
    
    //=========================================================================
    // Registration
    //=========================================================================
    
    /// Register a cvar (takes ownership)
    Cvar& registerCvar(UniquePtr<Cvar> cvar);
    
    /// Create and register a cvar
    Cvar& create(String name, String defaultValue, 
                 CvarFlags flags = CvarFlags::None);
    
    /// Create and register a cvar with callback
    Cvar& create(String name, String defaultValue,
                 CvarFlags flags, CvarCallback callback);
    
    /// Unregister and delete a cvar
    bool remove(StringView name);
    
    //=========================================================================
    // Lookup
    //=========================================================================
    
    /// Find a cvar by name (returns nullptr if not found)
    [[nodiscard]] Cvar* find(StringView name);
    [[nodiscard]] const Cvar* find(StringView name) const;
    
    /// Check if a cvar exists
    [[nodiscard]] bool exists(StringView name) const;
    
    /// Get value as float (returns 0 if not found)
    [[nodiscard]] Float value(StringView name) const;
    
    /// Get value as string (returns empty string if not found)
    [[nodiscard]] String string(StringView name) const;
    
    //=========================================================================
    // Iteration
    //=========================================================================
    
    /// Iterate over all cvars
    template<std::invocable<Cvar&> Func>
    void forEach(Func&& func) {
        std::shared_lock lock(mutex_);
        for (auto& [name, cvar] : cvars_) {
            func(*cvar);
        }
    }
    
    /// Iterate over all cvars (const)
    template<std::invocable<const Cvar&> Func>
    void forEach(Func&& func) const {
        std::shared_lock lock(mutex_);
        for (const auto& [name, cvar] : cvars_) {
            func(*cvar);
        }
    }
    
    /// Get all cvar names matching a pattern (glob)
    [[nodiscard]] Vector<String> findMatching(StringView pattern) const;
    
    /// Get all cvar names with specific flags
    [[nodiscard]] Vector<String> findWithFlags(CvarFlags flags) const;
    
    //=========================================================================
    // Bulk Operations
    //=========================================================================
    
    /// Clear modified flag on all cvars matching flags
    void clearModifiedFlags(CvarFlags flags = CvarFlags::None);
    
    /// Check if any cvar with matching flags is modified
    [[nodiscard]] bool anyModified(CvarFlags flags = CvarFlags::None) const;
    
    /// Apply all latched values
    void applyAllLatched();
    
    /// Reset all cvars to defaults (respects NoReset flag)
    void resetAll();
    
    //=========================================================================
    // Command Completion
    //=========================================================================
    
    /// Get completions for partial cvar name
    [[nodiscard]] Vector<String> complete(StringView partial) const;
    
    //=========================================================================
    // Group Management
    //=========================================================================
    
    /// Set current group for subsequently registered cvars
    void setCurrentGroup(StringView group);
    
    /// Reset current group to empty
    void resetCurrentGroup();
    
    /// Get all group names
    [[nodiscard]] Vector<String> getGroups() const;
    
    /// Get all cvars in a group
    [[nodiscard]] Vector<Cvar*> getCvarsInGroup(StringView group);
    
    //=========================================================================
    // Lifecycle
    //=========================================================================
    
    /// Initialize the cvar system
    void init();
    
    /// Shutdown and cleanup
    void shutdown();
    
    /// Clear all cvars
    void clear();
    
    /// Get total number of registered cvars
    [[nodiscard]] Size count() const;
    
private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<String, UniquePtr<Cvar>> cvars_;
    String currentGroup_;
    
    // For iteration order preservation
    Vector<String> cvarOrder_;
};

//=============================================================================
// Typed Cvar Reference
//=============================================================================

/**
 * @brief Type-safe reference to a cvar
 * 
 * Example usage:
 *   CvarRegistry registry;
 *   registry.create("sensitivity", "3.0");
 *   CvarRef<Float> sensitivity(registry, "sensitivity");
 *   float val = sensitivity.get();
 *   sensitivity.set(5.0f);
 */
template<typename T>
class CvarRef {
public:
    CvarRef(CvarRegistry& registry, StringView name) 
        : registry_(&registry), name_(name) {}
    
    /// Get the value
    [[nodiscard]] T get() const;
    
    /// Set the value
    bool set(T value);
    
    /// Get underlying cvar (may be null)
    [[nodiscard]] Cvar* cvar() { return registry_->find(name_); }
    [[nodiscard]] const Cvar* cvar() const { return registry_->find(name_); }
    
    /// Implicit conversion to value type
    operator T() const { return get(); }
    
private:
    CvarRegistry* registry_;
    String name_;
};

// Specializations
template<>
inline Float CvarRef<Float>::get() const { 
    auto* cv = registry_->find(name_);
    return cv ? cv->value() : 0.0f; 
}
template<>
inline bool CvarRef<Float>::set(Float value) { 
    auto* cv = registry_->find(name_);
    return cv ? cv->setValue(value) : false; 
}

template<>
inline Int32 CvarRef<Int32>::get() const { 
    auto* cv = registry_->find(name_);
    return cv ? cv->integer() : 0; 
}
template<>
inline bool CvarRef<Int32>::set(Int32 value) { 
    auto* cv = registry_->find(name_);
    return cv ? cv->setInteger(value) : false; 
}

template<>
inline Bool CvarRef<Bool>::get() const { 
    auto* cv = registry_->find(name_);
    return cv ? cv->boolean() : false; 
}
template<>
inline bool CvarRef<Bool>::set(Bool value) { 
    auto* cv = registry_->find(name_);
    return cv ? cv->setBoolean(value) : false; 
}

template<>
inline String CvarRef<String>::get() const { 
    auto* cv = registry_->find(name_);
    return cv ? cv->string() : String{}; 
}
template<>
inline bool CvarRef<String>::set(String value) { 
    auto* cv = registry_->find(name_);
    return cv ? cv->set(value) : false; 
}

} // namespace ezquake

#endif // EZQUAKE_CORE_CVAR_REGISTRY_HPP
