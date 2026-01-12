/**
 * @file trigger_types.hpp
 * @brief Regex trigger and message trigger types
 * 
 * This module covers:
 * - Regex-based message triggers
 * - Internal triggers for parsing
 * - Trigger flags and matching options
 * - Message type filters
 */

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <functional>
#include <chrono>

namespace ezquake::trigger {

// ============================================================================
// Constants
// ============================================================================

namespace trigger_limits {
    constexpr int32_t MAX_TRIGGER_NAME = 64;
    constexpr int32_t MAX_REGEX_LENGTH = 1024;
    constexpr int32_t MAX_TRIGGERS = 256;
    constexpr float DEFAULT_MIN_INTERVAL = 0.0f;  // No rate limiting by default
}

// ============================================================================
// Trigger Flags
// ============================================================================

/**
 * @brief Trigger flag bits for matching behavior
 * 
 * Matches RE_* defines from tp_triggers.h.
 */
enum class TriggerFlag : uint32_t {
    None = 0,
    
    // Message type filters
    PrintLow      = (1 << 0),   // RE_PRINT_LOW - Match low priority prints
    PrintMedium   = (1 << 1),   // RE_PRINT_NEDIUM - Match medium priority
    PrintHigh     = (1 << 2),   // RE_PRINT_HIGH - Match high priority
    PrintChat     = (1 << 3),   // RE_PRINT_CHAT - Match chat messages
    PrintCenter   = (1 << 4),   // RE_PRINT_CENTER - Match centerprint
    PrintEcho     = (1 << 5),   // RE_PRINT_ECHO - Match echo command output
    PrintInternal = (1 << 6),   // RE_PRINT_INTERNAL - Match internal messages
    
    // Behavior flags
    Final         = (1 << 8),   // RE_FINAL - Stop processing after match
    RemoveString  = (1 << 9),   // RE_REMOVESTR - Hide matched string
    NoLog         = (1 << 10),  // RE_NOLOG - Don't log matched string
    Enabled       = (1 << 11),  // RE_ENABLED - Trigger is active
    NoAction      = (1 << 12),  // RE_NOACTION - Match but don't execute
    
    // Common combinations
    PrintAll      = PrintLow | PrintMedium | PrintHigh | PrintChat | PrintCenter,  // RE_PRINT_ALL
};

// Enable bitwise operations
[[nodiscard]] constexpr TriggerFlag operator|(TriggerFlag a, TriggerFlag b) noexcept {
    return static_cast<TriggerFlag>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

[[nodiscard]] constexpr TriggerFlag operator&(TriggerFlag a, TriggerFlag b) noexcept {
    return static_cast<TriggerFlag>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

[[nodiscard]] constexpr TriggerFlag operator~(TriggerFlag a) noexcept {
    return static_cast<TriggerFlag>(~static_cast<uint32_t>(a));
}

constexpr TriggerFlag& operator|=(TriggerFlag& a, TriggerFlag b) noexcept {
    a = a | b;
    return a;
}

constexpr TriggerFlag& operator&=(TriggerFlag& a, TriggerFlag b) noexcept {
    a = a & b;
    return a;
}

[[nodiscard]] constexpr bool hasFlag(TriggerFlag flags, TriggerFlag flag) noexcept {
    return (static_cast<uint32_t>(flags) & static_cast<uint32_t>(flag)) != 0;
}

// ============================================================================
// Message Type for Trigger Matching
// ============================================================================

/**
 * @brief Message types for trigger filtering
 */
enum class MessageType : uint8_t {
    Low = 0,        // Low priority game messages
    Medium = 1,     // Medium priority
    High = 2,       // High priority
    Chat = 3,       // Chat messages
    Center = 4,     // Centerprint messages
    Echo = 5,       // Echo command output
    Internal = 6,   // Internal parsing messages
};

[[nodiscard]] constexpr std::string_view messageTypeName(MessageType type) noexcept {
    switch (type) {
        case MessageType::Low: return "low";
        case MessageType::Medium: return "medium";
        case MessageType::High: return "high";
        case MessageType::Chat: return "chat";
        case MessageType::Center: return "center";
        case MessageType::Echo: return "echo";
        case MessageType::Internal: return "internal";
    }
    return "unknown";
}

// Convert message type to trigger flag
[[nodiscard]] constexpr TriggerFlag messageTypeToFlag(MessageType type) noexcept {
    switch (type) {
        case MessageType::Low: return TriggerFlag::PrintLow;
        case MessageType::Medium: return TriggerFlag::PrintMedium;
        case MessageType::High: return TriggerFlag::PrintHigh;
        case MessageType::Chat: return TriggerFlag::PrintChat;
        case MessageType::Center: return TriggerFlag::PrintCenter;
        case MessageType::Echo: return TriggerFlag::PrintEcho;
        case MessageType::Internal: return TriggerFlag::PrintInternal;
    }
    return TriggerFlag::None;
}

// ============================================================================
// Match Result
// ============================================================================

/**
 * @brief Result of a trigger match operation
 */
struct TriggerMatchResult {
    bool matched = false;
    bool shouldHide = false;      // RemoveString flag was set
    bool shouldNotLog = false;    // NoLog flag was set
    bool isFinal = false;         // Final flag - stop processing
    std::string capturedText;     // Text captured by regex groups
    int triggerIndex = -1;        // Index of matched trigger
    
    [[nodiscard]] constexpr bool wasActionable() const noexcept {
        return matched && !shouldHide;
    }
    
    [[nodiscard]] static TriggerMatchResult noMatch() noexcept {
        return TriggerMatchResult{};
    }
};

// ============================================================================
// Regex Trigger
// ============================================================================

/**
 * @brief Regex-based message trigger
 * 
 * Matches pcre_trigger_t from tp_triggers.h.
 */
struct RegexTrigger {
    std::string name;                   // Trigger name (for commands)
    std::string pattern;                // Regex pattern string
    TriggerFlag flags = TriggerFlag::PrintAll | TriggerFlag::Enabled;
    float minInterval = 0.0f;           // Minimum seconds between activations
    double lastTriggerTime = 0.0;       // Last activation time
    int32_t triggerCount = 0;           // Number of times triggered
    
    // Compiled regex handle (opaque - actual PCRE2 handle managed elsewhere)
    void* compiledRegex = nullptr;
    
    [[nodiscard]] bool isEnabled() const noexcept {
        return hasFlag(flags, TriggerFlag::Enabled);
    }
    
    [[nodiscard]] bool isValid() const noexcept {
        return !name.empty() && !pattern.empty();
    }
    
    [[nodiscard]] bool matchesMessageType(MessageType type) const noexcept {
        return hasFlag(flags, messageTypeToFlag(type));
    }
    
    [[nodiscard]] bool canTrigger(double currentTime) const noexcept {
        if (!isEnabled()) return false;
        if (minInterval <= 0.0f) return true;
        return (currentTime - lastTriggerTime) >= static_cast<double>(minInterval);
    }
    
    void recordTrigger(double currentTime) noexcept {
        lastTriggerTime = currentTime;
        ++triggerCount;
    }
    
    void reset() noexcept {
        lastTriggerTime = 0.0;
        triggerCount = 0;
    }
    
    void setEnabled(bool enabled) noexcept {
        if (enabled) {
            flags |= TriggerFlag::Enabled;
        } else {
            flags &= ~TriggerFlag::Enabled;
        }
    }
};

// ============================================================================
// Internal Trigger
// ============================================================================

/**
 * @brief Function signature for internal trigger callbacks
 */
using InternalTriggerFunc = std::function<void(std::string_view message)>;

/**
 * @brief Internal trigger for programmatic message handling
 * 
 * Matches pcre_internal_trigger_t from tp_triggers.h.
 */
struct InternalTrigger {
    std::string pattern;                    // Regex pattern
    InternalTriggerFunc callback;           // Callback function
    TriggerFlag flags = TriggerFlag::PrintAll;
    void* compiledRegex = nullptr;          // Compiled regex handle
    
    [[nodiscard]] bool isValid() const noexcept {
        return !pattern.empty() && callback != nullptr;
    }
    
    [[nodiscard]] bool matchesMessageType(MessageType type) const noexcept {
        return hasFlag(flags, messageTypeToFlag(type));
    }
};

// ============================================================================
// Trigger Manager State
// ============================================================================

/**
 * @brief State for trigger management
 */
struct TriggerState {
    std::vector<RegexTrigger> userTriggers;
    std::vector<InternalTrigger> internalTriggers;
    bool enabled = true;           // Master enable switch
    bool processing = false;       // Currently processing (prevents recursion)
    
    [[nodiscard]] size_t userTriggerCount() const noexcept {
        return userTriggers.size();
    }
    
    [[nodiscard]] size_t internalTriggerCount() const noexcept {
        return internalTriggers.size();
    }
    
    [[nodiscard]] size_t enabledUserTriggerCount() const noexcept {
        size_t count = 0;
        for (const auto& t : userTriggers) {
            if (t.isEnabled()) ++count;
        }
        return count;
    }
    
    [[nodiscard]] RegexTrigger* findUserTrigger(std::string_view name) noexcept {
        for (auto& t : userTriggers) {
            if (t.name == name) return &t;
        }
        return nullptr;
    }
    
    [[nodiscard]] const RegexTrigger* findUserTrigger(std::string_view name) const noexcept {
        for (const auto& t : userTriggers) {
            if (t.name == name) return &t;
        }
        return nullptr;
    }
    
    bool removeUserTrigger(std::string_view name) noexcept {
        auto it = std::find_if(userTriggers.begin(), userTriggers.end(),
            [name](const RegexTrigger& t) { return t.name == name; });
        if (it != userTriggers.end()) {
            userTriggers.erase(it);
            return true;
        }
        return false;
    }
    
    void resetAllTriggerTimes() noexcept {
        for (auto& t : userTriggers) {
            t.lastTriggerTime = 0.0;
        }
    }
    
    void clearUserTriggers() noexcept {
        userTriggers.clear();
    }
    
    void clearInternalTriggers() noexcept {
        internalTriggers.clear();
    }
    
    void reset() noexcept {
        clearUserTriggers();
        clearInternalTriggers();
        enabled = true;
        processing = false;
    }
};

// ============================================================================
// Message Trigger (Legacy Compatibility)
// ============================================================================

/**
 * @brief Legacy message trigger type
 * 
 * For compatibility with TP_SearchForMsgTriggers() style triggers.
 */
struct MessageTrigger {
    std::string pattern;          // Pattern to match
    std::string action;           // Command to execute on match
    int32_t priority = 0;         // Execution priority (higher = earlier)
    bool enabled = true;
    
    [[nodiscard]] bool isValid() const noexcept {
        return !pattern.empty() && !action.empty();
    }
};

// ============================================================================
// Trigger Statistics
// ============================================================================

/**
 * @brief Statistics for trigger system
 */
struct TriggerStats {
    uint64_t messagesProcessed = 0;
    uint64_t triggersMatched = 0;
    uint64_t messagesHidden = 0;
    uint64_t messagesNotLogged = 0;
    double lastProcessTime = 0.0;
    
    void recordMessage() noexcept {
        ++messagesProcessed;
    }
    
    void recordMatch(const TriggerMatchResult& result) noexcept {
        if (result.matched) {
            ++triggersMatched;
            if (result.shouldHide) ++messagesHidden;
            if (result.shouldNotLog) ++messagesNotLogged;
        }
    }
    
    void reset() noexcept {
        messagesProcessed = 0;
        triggersMatched = 0;
        messagesHidden = 0;
        messagesNotLogged = 0;
        lastProcessTime = 0.0;
    }
    
    [[nodiscard]] double matchRate() const noexcept {
        if (messagesProcessed == 0) return 0.0;
        return static_cast<double>(triggersMatched) / static_cast<double>(messagesProcessed);
    }
};

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * @brief Parse trigger flags from string
 */
[[nodiscard]] inline TriggerFlag parseTriggerFlags(std::string_view flagStr) noexcept {
    TriggerFlag flags = TriggerFlag::None;
    
    for (char c : flagStr) {
        switch (c) {
            case 'l': case 'L': flags |= TriggerFlag::PrintLow; break;
            case 'm': case 'M': flags |= TriggerFlag::PrintMedium; break;
            case 'h': case 'H': flags |= TriggerFlag::PrintHigh; break;
            case 'c': case 'C': flags |= TriggerFlag::PrintChat; break;
            case 'p': case 'P': flags |= TriggerFlag::PrintCenter; break;
            case 'e': case 'E': flags |= TriggerFlag::PrintEcho; break;
            case 'i': case 'I': flags |= TriggerFlag::PrintInternal; break;
            case 'f': case 'F': flags |= TriggerFlag::Final; break;
            case 'r': case 'R': flags |= TriggerFlag::RemoveString; break;
            case 'n': case 'N': flags |= TriggerFlag::NoLog; break;
            case 'a': case 'A': flags |= TriggerFlag::NoAction; break;
        }
    }
    
    return flags;
}

/**
 * @brief Convert trigger flags to string representation
 */
[[nodiscard]] inline std::string triggerFlagsToString(TriggerFlag flags) noexcept {
    std::string result;
    
    if (hasFlag(flags, TriggerFlag::PrintLow)) result += 'l';
    if (hasFlag(flags, TriggerFlag::PrintMedium)) result += 'm';
    if (hasFlag(flags, TriggerFlag::PrintHigh)) result += 'h';
    if (hasFlag(flags, TriggerFlag::PrintChat)) result += 'c';
    if (hasFlag(flags, TriggerFlag::PrintCenter)) result += 'p';
    if (hasFlag(flags, TriggerFlag::PrintEcho)) result += 'e';
    if (hasFlag(flags, TriggerFlag::PrintInternal)) result += 'i';
    if (hasFlag(flags, TriggerFlag::Final)) result += 'f';
    if (hasFlag(flags, TriggerFlag::RemoveString)) result += 'r';
    if (hasFlag(flags, TriggerFlag::NoLog)) result += 'n';
    if (hasFlag(flags, TriggerFlag::NoAction)) result += 'a';
    
    return result;
}

/**
 * @brief Create a default regex trigger
 */
[[nodiscard]] inline RegexTrigger createTrigger(std::string_view name, 
                                                 std::string_view pattern,
                                                 TriggerFlag flags = TriggerFlag::PrintAll | TriggerFlag::Enabled) noexcept {
    RegexTrigger trigger;
    trigger.name = std::string(name);
    trigger.pattern = std::string(pattern);
    trigger.flags = flags;
    return trigger;
}

/**
 * @brief Apply match result flags to result struct
 */
[[nodiscard]] inline TriggerMatchResult createMatchResult(const RegexTrigger& trigger, 
                                                           bool matched,
                                                           int32_t index = -1) noexcept {
    TriggerMatchResult result;
    result.matched = matched;
    result.triggerIndex = index;
    
    if (matched) {
        result.shouldHide = hasFlag(trigger.flags, TriggerFlag::RemoveString);
        result.shouldNotLog = hasFlag(trigger.flags, TriggerFlag::NoLog);
        result.isFinal = hasFlag(trigger.flags, TriggerFlag::Final);
    }
    
    return result;
}

} // namespace ezquake::trigger
