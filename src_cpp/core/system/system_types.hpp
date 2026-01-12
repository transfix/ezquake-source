/**
 * @file system_types.hpp
 * @brief System and platform types for ezQuake C++20 port
 * 
 * Provides modern C++ equivalents of sys.h and localtime.h types.
 */

#pragma once

#include <array>
#include <cstdint>
#include <string_view>
#include <chrono>
#include <optional>

namespace ezquake::sys {

// =============================================================================
// Time-related types
// =============================================================================

/**
 * Cross-platform time structure.
 * Original: SYSTEMTIME from localtime.h (matching Windows SYSTEMTIME)
 */
struct SystemTime {
    int16_t year{0};
    int16_t month{0};       // 1-12
    int16_t dayOfWeek{0};   // 0-6 (Sunday = 0)
    int16_t day{0};         // 1-31
    int16_t hour{0};        // 0-23
    int16_t minute{0};      // 0-59
    int16_t second{0};      // 0-59
    int16_t milliseconds{0}; // 0-999
    
    /**
     * Check if the time is valid.
     */
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return year > 0 && month >= 1 && month <= 12 &&
               day >= 1 && day <= 31 && hour >= 0 && hour <= 23 &&
               minute >= 0 && minute <= 59 && second >= 0 && second <= 59;
    }
    
    /**
     * Compare two SystemTime values.
     * Returns: <0 if lhs < rhs, 0 if equal, >0 if lhs > rhs
     */
    [[nodiscard]] constexpr int compare(const SystemTime& other) const noexcept {
        if (year != other.year) return year - other.year;
        if (month != other.month) return month - other.month;
        if (day != other.day) return day - other.day;
        if (hour != other.hour) return hour - other.hour;
        if (minute != other.minute) return minute - other.minute;
        if (second != other.second) return second - other.second;
        return milliseconds - other.milliseconds;
    }
    
    [[nodiscard]] constexpr bool operator==(const SystemTime& other) const noexcept {
        return compare(other) == 0;
    }
    
    [[nodiscard]] constexpr bool operator!=(const SystemTime& other) const noexcept {
        return compare(other) != 0;
    }
    
    [[nodiscard]] constexpr bool operator<(const SystemTime& other) const noexcept {
        return compare(other) < 0;
    }
    
    [[nodiscard]] constexpr bool operator<=(const SystemTime& other) const noexcept {
        return compare(other) <= 0;
    }
    
    [[nodiscard]] constexpr bool operator>(const SystemTime& other) const noexcept {
        return compare(other) > 0;
    }
    
    [[nodiscard]] constexpr bool operator>=(const SystemTime& other) const noexcept {
        return compare(other) >= 0;
    }
    
    /**
     * Convert to total seconds since midnight.
     */
    [[nodiscard]] constexpr int32_t totalSeconds() const noexcept {
        return hour * 3600 + minute * 60 + second;
    }
    
    /**
     * Convert to total milliseconds since midnight.
     */
    [[nodiscard]] constexpr int64_t totalMilliseconds() const noexcept {
        return static_cast<int64_t>(totalSeconds()) * 1000 + milliseconds;
    }
};

// =============================================================================
// Timer Resolution Types
// =============================================================================

/**
 * Timer resolution session for high-precision sleep.
 * Original: timerresolution_session_t from sys.h
 * 
 * On Windows, to make Sleep(1) actually sleep for ~1ms instead of 15-18ms,
 * you need to request increased timer resolution.
 */
struct TimerResolutionSession {
    bool isSet{false};
    uint32_t intervalMs{0};
    
    /**
     * Check if high-resolution timing is active.
     */
    [[nodiscard]] constexpr bool isActive() const noexcept {
        return isSet && intervalMs > 0;
    }
    
    /**
     * Reset to default state.
     */
    constexpr void reset() noexcept {
        isSet = false;
        intervalMs = 0;
    }
};

// =============================================================================
// File Types
// =============================================================================

namespace limits {
    inline constexpr size_t MAX_DIRFILES = 4096;
    inline constexpr size_t MAX_DEMO_NAME = 196;
    inline constexpr size_t MAX_PATH_LENGTH = 4096;
    inline constexpr size_t MAX_FILENAME_LENGTH = 256;
} // namespace limits

/**
 * File information entry.
 * Original: file_t from sys.h
 */
struct FileInfo {
    std::array<char, limits::MAX_DEMO_NAME> name{};
    int32_t size{0};
    int32_t modTime{0};  // Unix timestamp
    bool isDirectory{false};
    
    /**
     * Get name as string view.
     */
    [[nodiscard]] std::string_view getName() const noexcept {
        return std::string_view(name.data());
    }
    
    /**
     * Set name from string.
     */
    void setName(std::string_view newName) noexcept {
        size_t len = std::min(newName.size(), name.size() - 1);
        for (size_t i = 0; i < len; ++i) {
            name[i] = newName[i];
        }
        name[len] = '\0';
    }
    
    /**
     * Check if this is a file (not a directory).
     */
    [[nodiscard]] constexpr bool isFile() const noexcept {
        return !isDirectory;
    }
    
    /**
     * Check if this is a valid entry.
     */
    [[nodiscard]] bool isValid() const noexcept {
        return name[0] != '\0';
    }
};

// =============================================================================
// Platform Detection
// =============================================================================

/**
 * Operating system type.
 */
enum class OSType : uint8_t {
    Unknown = 0,
    Windows,
    Linux,
    MacOS,
    FreeBSD,
    
    Count
};

/**
 * Get display name for OS type.
 */
[[nodiscard]] constexpr const char* getOSName(OSType os) noexcept {
    switch (os) {
        case OSType::Windows: return "Windows";
        case OSType::Linux: return "Linux";
        case OSType::MacOS: return "macOS";
        case OSType::FreeBSD: return "FreeBSD";
        default: return "Unknown";
    }
}

/**
 * Get current OS type at compile time.
 */
[[nodiscard]] constexpr OSType getCurrentOS() noexcept {
#if defined(_WIN32) || defined(_WIN64)
    return OSType::Windows;
#elif defined(__APPLE__)
    return OSType::MacOS;
#elif defined(__FreeBSD__)
    return OSType::FreeBSD;
#elif defined(__linux__)
    return OSType::Linux;
#else
    return OSType::Unknown;
#endif
}

/**
 * CPU architecture type.
 */
enum class CPUArch : uint8_t {
    Unknown = 0,
    x86,
    x86_64,
    ARM,
    ARM64,
    
    Count
};

/**
 * Get display name for CPU architecture.
 */
[[nodiscard]] constexpr const char* getCPUArchName(CPUArch arch) noexcept {
    switch (arch) {
        case CPUArch::x86: return "x86";
        case CPUArch::x86_64: return "x86_64";
        case CPUArch::ARM: return "ARM";
        case CPUArch::ARM64: return "ARM64";
        default: return "Unknown";
    }
}

/**
 * Get current CPU architecture at compile time.
 */
[[nodiscard]] constexpr CPUArch getCurrentCPUArch() noexcept {
#if defined(__x86_64__) || defined(_M_X64)
    return CPUArch::x86_64;
#elif defined(__i386__) || defined(_M_IX86)
    return CPUArch::x86;
#elif defined(__aarch64__) || defined(_M_ARM64)
    return CPUArch::ARM64;
#elif defined(__arm__) || defined(_M_ARM)
    return CPUArch::ARM;
#else
    return CPUArch::Unknown;
#endif
}

// =============================================================================
// Build Configuration
// =============================================================================

/**
 * Build type.
 */
enum class BuildType : uint8_t {
    Debug = 0,
    Release,
    RelWithDebInfo,
    MinSizeRel,
    
    Count
};

/**
 * Get display name for build type.
 */
[[nodiscard]] constexpr const char* getBuildTypeName(BuildType type) noexcept {
    switch (type) {
        case BuildType::Debug: return "Debug";
        case BuildType::Release: return "Release";
        case BuildType::RelWithDebInfo: return "RelWithDebInfo";
        case BuildType::MinSizeRel: return "MinSizeRel";
        default: return "Unknown";
    }
}

/**
 * Platform information.
 */
struct PlatformInfo {
    OSType os{getCurrentOS()};
    CPUArch arch{getCurrentCPUArch()};
    BuildType buildType{BuildType::Release};
    bool is64Bit{sizeof(void*) == 8};
    bool hasSSE{false};
    bool hasSSE2{false};
    bool hasAVX{false};
    bool hasAVX2{false};
    
    /**
     * Check if we're on a 32-bit platform.
     */
    [[nodiscard]] constexpr bool is32Bit() const noexcept {
        return !is64Bit;
    }
    
    /**
     * Check if SIMD is available.
     */
    [[nodiscard]] constexpr bool hasSIMD() const noexcept {
        return hasSSE || hasAVX;
    }
};

// =============================================================================
// Error Handling
// =============================================================================

/**
 * System error level.
 */
enum class ErrorLevel : uint8_t {
    None = 0,
    Warning,
    Error,
    Fatal,
    
    Count
};

/**
 * Get display name for error level.
 */
[[nodiscard]] constexpr const char* getErrorLevelName(ErrorLevel level) noexcept {
    switch (level) {
        case ErrorLevel::None: return "none";
        case ErrorLevel::Warning: return "warning";
        case ErrorLevel::Error: return "error";
        case ErrorLevel::Fatal: return "fatal";
        default: return "unknown";
    }
}

/**
 * Check if error level should terminate.
 */
[[nodiscard]] constexpr bool shouldTerminate(ErrorLevel level) noexcept {
    return level == ErrorLevel::Fatal;
}

/**
 * System error information.
 */
struct SystemError {
    ErrorLevel level{ErrorLevel::None};
    int32_t code{0};
    std::array<char, 256> message{};
    
    /**
     * Check if there's an error.
     */
    [[nodiscard]] constexpr bool hasError() const noexcept {
        return level != ErrorLevel::None;
    }
    
    /**
     * Get message as string view.
     */
    [[nodiscard]] std::string_view getMessage() const noexcept {
        return std::string_view(message.data());
    }
    
    /**
     * Set message from string.
     */
    void setMessage(std::string_view msg) noexcept {
        size_t len = std::min(msg.size(), message.size() - 1);
        for (size_t i = 0; i < len; ++i) {
            message[i] = msg[i];
        }
        message[len] = '\0';
    }
    
    /**
     * Create a warning.
     */
    [[nodiscard]] static SystemError warning(std::string_view msg, int32_t code = 0) noexcept {
        SystemError err;
        err.level = ErrorLevel::Warning;
        err.code = code;
        err.setMessage(msg);
        return err;
    }
    
    /**
     * Create an error.
     */
    [[nodiscard]] static SystemError error(std::string_view msg, int32_t code = 0) noexcept {
        SystemError err;
        err.level = ErrorLevel::Error;
        err.code = code;
        err.setMessage(msg);
        return err;
    }
    
    /**
     * Create a fatal error.
     */
    [[nodiscard]] static SystemError fatal(std::string_view msg, int32_t code = 0) noexcept {
        SystemError err;
        err.level = ErrorLevel::Fatal;
        err.code = code;
        err.setMessage(msg);
        return err;
    }
};

// =============================================================================
// Process/Thread Types
// =============================================================================

/**
 * Thread priority level.
 */
enum class ThreadPriority : uint8_t {
    Lowest = 0,
    Low,
    Normal,
    High,
    Highest,
    TimeCritical,
    
    Count
};

/**
 * Get display name for thread priority.
 */
[[nodiscard]] constexpr const char* getThreadPriorityName(ThreadPriority priority) noexcept {
    switch (priority) {
        case ThreadPriority::Lowest: return "lowest";
        case ThreadPriority::Low: return "low";
        case ThreadPriority::Normal: return "normal";
        case ThreadPriority::High: return "high";
        case ThreadPriority::Highest: return "highest";
        case ThreadPriority::TimeCritical: return "time_critical";
        default: return "unknown";
    }
}

/**
 * Thread information.
 */
struct ThreadInfo {
    uint64_t id{0};
    ThreadPriority priority{ThreadPriority::Normal};
    bool isMainThread{false};
    
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return id != 0;
    }
};

// =============================================================================
// Performance Timing
// =============================================================================

/**
 * Performance counter for high-resolution timing.
 */
struct PerformanceCounter {
    int64_t frequency{0};      // Counts per second
    int64_t startCount{0};     // Starting count
    int64_t currentCount{0};   // Current count
    
    /**
     * Get elapsed time in seconds.
     */
    [[nodiscard]] constexpr double elapsedSeconds() const noexcept {
        if (frequency <= 0) return 0.0;
        return static_cast<double>(currentCount - startCount) / static_cast<double>(frequency);
    }
    
    /**
     * Get elapsed time in milliseconds.
     */
    [[nodiscard]] constexpr double elapsedMilliseconds() const noexcept {
        return elapsedSeconds() * 1000.0;
    }
    
    /**
     * Get elapsed time in microseconds.
     */
    [[nodiscard]] constexpr double elapsedMicroseconds() const noexcept {
        return elapsedSeconds() * 1000000.0;
    }
    
    /**
     * Reset the counter.
     */
    constexpr void reset() noexcept {
        startCount = currentCount;
    }
};

/**
 * Frame timing information.
 */
struct FrameTiming {
    double realTime{0.0};        // Real clock time
    double frameTime{0.0};       // Time since last frame
    double gameTime{0.0};        // In-game time (may be paused/scaled)
    uint64_t frameCount{0};      // Total frames rendered
    double averageFps{0.0};      // Average FPS
    double instantFps{0.0};      // Current frame FPS
    
    /**
     * Update frame timing.
     */
    void update(double newRealTime, double newGameTime) noexcept {
        frameTime = newRealTime - realTime;
        realTime = newRealTime;
        gameTime = newGameTime;
        frameCount++;
        
        if (frameTime > 0.0) {
            instantFps = 1.0 / frameTime;
        }
        
        // Simple exponential moving average for average FPS
        constexpr double alpha = 0.1;
        averageFps = alpha * instantFps + (1.0 - alpha) * averageFps;
    }
    
    /**
     * Reset timing.
     */
    void reset() noexcept {
        realTime = 0.0;
        frameTime = 0.0;
        gameTime = 0.0;
        frameCount = 0;
        averageFps = 0.0;
        instantFps = 0.0;
    }
};

// =============================================================================
// Helper Functions
// =============================================================================

/**
 * Create SystemTime from components.
 */
[[nodiscard]] constexpr SystemTime makeSystemTime(
    int16_t year, int16_t month, int16_t day,
    int16_t hour = 0, int16_t minute = 0, int16_t second = 0,
    int16_t milliseconds = 0) noexcept 
{
    SystemTime st;
    st.year = year;
    st.month = month;
    st.day = day;
    st.hour = hour;
    st.minute = minute;
    st.second = second;
    st.milliseconds = milliseconds;
    return st;
}

/**
 * Calculate difference between two SystemTime values in seconds.
 * Note: This is a simplified calculation, not accounting for DST etc.
 */
[[nodiscard]] constexpr int64_t diffSeconds(const SystemTime& a, const SystemTime& b) noexcept {
    // Simple calculation - good for same-day comparisons
    return a.totalSeconds() - b.totalSeconds();
}

/**
 * Format time as HH:MM:SS.
 */
[[nodiscard]] inline std::array<char, 16> formatTimeHMS(const SystemTime& time) noexcept {
    std::array<char, 16> buffer{};
    int h = time.hour;
    int m = time.minute;
    int s = time.second;
    
    buffer[0] = static_cast<char>('0' + (h / 10));
    buffer[1] = static_cast<char>('0' + (h % 10));
    buffer[2] = ':';
    buffer[3] = static_cast<char>('0' + (m / 10));
    buffer[4] = static_cast<char>('0' + (m % 10));
    buffer[5] = ':';
    buffer[6] = static_cast<char>('0' + (s / 10));
    buffer[7] = static_cast<char>('0' + (s % 10));
    buffer[8] = '\0';
    
    return buffer;
}

} // namespace ezquake::sys
