/**
 * @file logging_types.hpp
 * @brief Server and client logging types
 * 
 * This module covers:
 * - Server log types (console, error, frag, etc.)
 * - Log entry structure
 * - Logging configuration
 * - Player ignore/filter types
 */

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <optional>
#include <chrono>
#include <functional>

namespace ezquake::logging {

// ============================================================================
// Constants
// ============================================================================

namespace logging_limits {
    constexpr int32_t MAX_LOG_TYPES = 8;          // MAX_LOG from sv_log.h
    constexpr int32_t MAX_TEAM_IGNORE = 4;        // MAX_TEAMIGNORELIST
    constexpr int32_t FLOOD_LIST_SIZE = 10;       // FLOODLIST_SIZE
    constexpr int32_t MAX_FLOOD_MESSAGE = 2048;   // flood_t data size
    constexpr int32_t MAX_LOG_MESSAGE = 4096;
    constexpr int32_t MAX_LOG_FILENAME = 256;
    constexpr float DEFAULT_FLOOD_DURATION = 4.0f;
}

// ============================================================================
// Log Type
// ============================================================================

/**
 * @brief Server log types
 * 
 * Matches the log type enum from sv_log.h.
 */
enum class LogType : uint8_t {
    Console = 0,   // CONSOLE_LOG - General console output
    Error = 1,     // ERROR_LOG - Error messages
    Rcon = 2,      // RCON_LOG - Remote console commands
    Telnet = 3,    // TELNET_LOG - Telnet connections
    Frag = 4,      // FRAG_LOG - Frag/kill logging
    Player = 5,    // PLAYER_LOG - Player events
    ModFrag = 6,   // MOD_FRAG_LOG - Mod frag logging
    
    Count = 7      // MAX_LOG
};

[[nodiscard]] constexpr std::string_view logTypeName(LogType type) noexcept {
    switch (type) {
        case LogType::Console: return "console";
        case LogType::Error: return "error";
        case LogType::Rcon: return "rcon";
        case LogType::Telnet: return "telnet";
        case LogType::Frag: return "frag";
        case LogType::Player: return "player";
        case LogType::ModFrag: return "modfrag";
        default: return "unknown";
    }
}

[[nodiscard]] constexpr std::string_view logTypeDefaultFilename(LogType type) noexcept {
    switch (type) {
        case LogType::Console: return "console.log";
        case LogType::Error: return "error.log";
        case LogType::Rcon: return "rcon.log";
        case LogType::Telnet: return "telnet.log";
        case LogType::Frag: return "frag.log";
        case LogType::Player: return "player.log";
        case LogType::ModFrag: return "modfrag.log";
        default: return "default.log";
    }
}

// ============================================================================
// Log Level
// ============================================================================

/**
 * @brief Log severity levels
 */
enum class LogLevel : uint8_t {
    Debug = 0,     // Verbose debug output
    Info = 1,      // Informational messages
    Warning = 2,   // Warnings
    Error = 3,     // Errors
    Critical = 4,  // Critical errors
    
    All = Debug,   // Log everything
    None = 255     // Log nothing
};

[[nodiscard]] constexpr std::string_view logLevelName(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::Debug: return "debug";
        case LogLevel::Info: return "info";
        case LogLevel::Warning: return "warning";
        case LogLevel::Error: return "error";
        case LogLevel::Critical: return "critical";
        case LogLevel::None: return "none";
        default: return "unknown";
    }
}

[[nodiscard]] constexpr bool shouldLog(LogLevel messageLevel, LogLevel configLevel) noexcept {
    if (configLevel == LogLevel::None) return false;
    return static_cast<uint8_t>(messageLevel) >= static_cast<uint8_t>(configLevel);
}

// ============================================================================
// Log Entry
// ============================================================================

/**
 * @brief Single log entry
 */
struct LogEntry {
    std::string message;
    LogType type = LogType::Console;
    LogLevel level = LogLevel::Info;
    double timestamp = 0.0;        // Game time when logged
    int32_t frameNumber = 0;       // Frame when logged
    
    [[nodiscard]] bool isValid() const noexcept {
        return !message.empty();
    }
    
    [[nodiscard]] bool isError() const noexcept {
        return level >= LogLevel::Error;
    }
};

// ============================================================================
// Log Configuration
// ============================================================================

/**
 * @brief Configuration for a single log type
 * 
 * Matches log_t from sv_log.h.
 */
struct LogConfig {
    std::string command;       // Console command name
    std::string filename;      // Log file name
    std::string messageOff;    // Message when log is disabled
    std::string messageOn;     // Message when log is enabled
    LogLevel minLevel = LogLevel::Info;
    bool enabled = false;
    bool appendMode = true;    // Append vs overwrite
    
    // File handle (opaque - actual FILE* managed elsewhere)
    void* fileHandle = nullptr;
    
    [[nodiscard]] bool isOpen() const noexcept {
        return fileHandle != nullptr;
    }
    
    [[nodiscard]] bool shouldLog(LogLevel level) const noexcept {
        return enabled && ::ezquake::logging::shouldLog(level, minLevel);
    }
};

// ============================================================================
// Log Manager State
// ============================================================================

/**
 * @brief State for all server logs
 */
struct LogManagerState {
    std::array<LogConfig, logging_limits::MAX_LOG_TYPES> logs;
    bool globalEnabled = true;
    double lastFlushTime = 0.0;
    float flushInterval = 1.0f;  // Flush every second
    
    [[nodiscard]] LogConfig* getLog(LogType type) noexcept {
        auto idx = static_cast<size_t>(type);
        if (idx >= logs.size()) return nullptr;
        return &logs[idx];
    }
    
    [[nodiscard]] const LogConfig* getLog(LogType type) const noexcept {
        auto idx = static_cast<size_t>(type);
        if (idx >= logs.size()) return nullptr;
        return &logs[idx];
    }
    
    void enableAll() noexcept {
        for (auto& log : logs) {
            log.enabled = true;
        }
    }
    
    void disableAll() noexcept {
        for (auto& log : logs) {
            log.enabled = false;
        }
    }
    
    [[nodiscard]] size_t enabledCount() const noexcept {
        size_t count = 0;
        for (const auto& log : logs) {
            if (log.enabled) ++count;
        }
        return count;
    }
    
    void reset() noexcept {
        for (auto& log : logs) {
            log.enabled = false;
            log.fileHandle = nullptr;
        }
        globalEnabled = true;
        lastFlushTime = 0.0;
    }
};

// ============================================================================
// Flood Protection
// ============================================================================

/**
 * @brief Flood message entry
 * 
 * Matches flood_t from ignore.c.
 */
struct FloodEntry {
    std::string data;
    float timestamp = 0.0f;
    
    [[nodiscard]] bool isEmpty() const noexcept {
        return data.empty();
    }
    
    void clear() noexcept {
        data.clear();
        timestamp = 0.0f;
    }
};

/**
 * @brief Flood protection state
 */
struct FloodProtection {
    std::array<FloodEntry, logging_limits::FLOOD_LIST_SIZE> entries;
    size_t currentIndex = 0;
    float duration = logging_limits::DEFAULT_FLOOD_DURATION;
    bool enabled = false;
    
    [[nodiscard]] bool isFlooding(std::string_view message, float currentTime) const noexcept {
        if (!enabled) return false;
        
        float cutoffTime = currentTime - duration;
        for (const auto& entry : entries) {
            if (!entry.isEmpty() && entry.timestamp > cutoffTime && entry.data == message) {
                return true;
            }
        }
        return false;
    }
    
    void recordMessage(std::string_view message, float currentTime) noexcept {
        entries[currentIndex].data = std::string(message);
        entries[currentIndex].timestamp = currentTime;
        currentIndex = (currentIndex + 1) % entries.size();
    }
    
    void clear() noexcept {
        for (auto& entry : entries) {
            entry.clear();
        }
        currentIndex = 0;
    }
    
    void reset() noexcept {
        clear();
        duration = logging_limits::DEFAULT_FLOOD_DURATION;
        enabled = false;
    }
};

// ============================================================================
// Ignore List
// ============================================================================

/**
 * @brief Ignore mode for chat filtering
 */
enum class IgnoreMode : uint8_t {
    Disabled = 0,    // No ignoring
    Normal = 1,      // Standard ignore
    Absolute = 2     // Absolute ignore (also centerprints etc.)
};

[[nodiscard]] constexpr std::string_view ignoreModeName(IgnoreMode mode) noexcept {
    switch (mode) {
        case IgnoreMode::Disabled: return "disabled";
        case IgnoreMode::Normal: return "normal";
        case IgnoreMode::Absolute: return "absolute";
    }
    return "unknown";
}

/**
 * @brief Spectator ignore mode
 */
enum class SpectatorIgnore : uint8_t {
    None = 0,        // Don't ignore spectators
    WhenPlaying = 1, // Ignore when you're playing
    Always = 2       // Always ignore spectators
};

/**
 * @brief Player ignore entry
 */
struct IgnoreEntry {
    int32_t playerSlot = -1;   // Player slot (-1 if not set)
    bool ignored = false;       // Chat ignored
    bool voiceIgnored = false;  // VOIP ignored
    
    [[nodiscard]] bool isSet() const noexcept {
        return playerSlot >= 0;
    }
    
    [[nodiscard]] bool isIgnored() const noexcept {
        return isSet() && (ignored || voiceIgnored);
    }
};

/**
 * @brief Ignore list state
 */
struct IgnoreState {
    std::vector<IgnoreEntry> playerIgnores;  // Per-player ignores
    std::vector<std::string> teamIgnores;    // Ignored team names
    
    IgnoreMode mode = IgnoreMode::Disabled;
    SpectatorIgnore spectatorIgnore = SpectatorIgnore::None;
    bool ignoreQizmoSpectators = false;
    bool ignoreQtvObservers = false;
    bool ignoreOpponents = false;
    bool ignoreNoWeapon = false;        // Ignore "no weapon" messages
    bool ignoreNotEnoughAmmo = false;   // Ignore "not enough ammo" messages
    
    FloodProtection floodProtection;
    
    [[nodiscard]] bool isPlayerIgnored(int32_t slot) const noexcept {
        if (slot < 0 || static_cast<size_t>(slot) >= playerIgnores.size()) {
            return false;
        }
        return playerIgnores[static_cast<size_t>(slot)].ignored;
    }
    
    [[nodiscard]] bool isTeamIgnored(std::string_view teamName) const noexcept {
        for (const auto& team : teamIgnores) {
            if (team == teamName) return true;
        }
        return false;
    }
    
    bool setPlayerIgnored(int32_t slot, bool ignored) noexcept {
        if (slot < 0) return false;
        
        auto index = static_cast<size_t>(slot);
        if (index >= playerIgnores.size()) {
            playerIgnores.resize(index + 1);
        }
        
        playerIgnores[index].playerSlot = slot;
        playerIgnores[index].ignored = ignored;
        return true;
    }
    
    bool addTeamIgnore(std::string_view teamName) noexcept {
        if (teamIgnores.size() >= logging_limits::MAX_TEAM_IGNORE) {
            return false;
        }
        if (isTeamIgnored(teamName)) return false;
        teamIgnores.push_back(std::string(teamName));
        return true;
    }
    
    bool removeTeamIgnore(std::string_view teamName) noexcept {
        auto it = std::find(teamIgnores.begin(), teamIgnores.end(), teamName);
        if (it != teamIgnores.end()) {
            teamIgnores.erase(it);
            return true;
        }
        return false;
    }
    
    [[nodiscard]] size_t ignoredPlayerCount() const noexcept {
        size_t count = 0;
        for (const auto& entry : playerIgnores) {
            if (entry.ignored) ++count;
        }
        return count;
    }
    
    void clearPlayerIgnores() noexcept {
        playerIgnores.clear();
    }
    
    void clearTeamIgnores() noexcept {
        teamIgnores.clear();
    }
    
    void reset() noexcept {
        playerIgnores.clear();
        teamIgnores.clear();
        mode = IgnoreMode::Disabled;
        spectatorIgnore = SpectatorIgnore::None;
        ignoreQizmoSpectators = false;
        ignoreQtvObservers = false;
        ignoreOpponents = false;
        ignoreNoWeapon = false;
        ignoreNotEnoughAmmo = false;
        floodProtection.reset();
    }
};

// ============================================================================
// Console Logging
// ============================================================================

/**
 * @brief Console log destination
 */
enum class ConsoleLogDest : uint8_t {
    None = 0,
    File = (1 << 0),
    Screen = (1 << 1),
    Both = File | Screen
};

[[nodiscard]] constexpr ConsoleLogDest operator|(ConsoleLogDest a, ConsoleLogDest b) noexcept {
    return static_cast<ConsoleLogDest>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

[[nodiscard]] constexpr bool hasDestination(ConsoleLogDest dest, ConsoleLogDest check) noexcept {
    return (static_cast<uint8_t>(dest) & static_cast<uint8_t>(check)) != 0;
}

/**
 * @brief Console log configuration
 */
struct ConsoleLogConfig {
    std::string filename = "qconsole.log";
    ConsoleLogDest destination = ConsoleLogDest::Both;
    bool timestamps = false;
    bool overwrite = false;  // Overwrite vs append
    bool enabled = true;
    
    [[nodiscard]] bool shouldLogToFile() const noexcept {
        return enabled && hasDestination(destination, ConsoleLogDest::File);
    }
    
    [[nodiscard]] bool shouldLogToScreen() const noexcept {
        return enabled && hasDestination(destination, ConsoleLogDest::Screen);
    }
};

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * @brief Create default log configurations
 */
[[nodiscard]] inline std::array<LogConfig, logging_limits::MAX_LOG_TYPES> createDefaultLogConfigs() noexcept {
    std::array<LogConfig, logging_limits::MAX_LOG_TYPES> configs;
    
    for (size_t i = 0; i < configs.size(); ++i) {
        auto type = static_cast<LogType>(i);
        configs[i].filename = std::string(logTypeDefaultFilename(type));
        configs[i].command = std::string(logTypeName(type)) + "log";
        configs[i].messageOn = "Logging to " + configs[i].filename;
        configs[i].messageOff = "Logging disabled";
    }
    
    return configs;
}

/**
 * @brief Create default ignore state
 */
[[nodiscard]] inline IgnoreState createDefaultIgnoreState(size_t maxPlayers = 32) noexcept {
    IgnoreState state;
    state.playerIgnores.resize(maxPlayers);
    return state;
}

/**
 * @brief Format log entry for file output
 */
[[nodiscard]] inline std::string formatLogEntry(const LogEntry& entry, bool includeTimestamp = true) noexcept {
    std::string result;
    
    if (includeTimestamp) {
        // Simple timestamp format
        int minutes = static_cast<int>(entry.timestamp) / 60;
        int seconds = static_cast<int>(entry.timestamp) % 60;
        char buf[32];
        snprintf(buf, sizeof(buf), "[%02d:%02d] ", minutes, seconds);
        result += buf;
    }
    
    result += entry.message;
    return result;
}

} // namespace ezquake::logging
