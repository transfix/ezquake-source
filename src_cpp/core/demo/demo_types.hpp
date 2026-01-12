/**
 * @file demo_types.hpp
 * @brief Demo recording and playback type definitions
 * 
 * Converted from cl_demo.c, cl_nqdemo.c, and client.h
 * Core types for the demo subsystem including message types,
 * playback states, and timing information.
 */

#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <functional>

namespace ezquake {

//=============================================================================
// Demo Message Types (from protocol.h)
//=============================================================================

/**
 * @brief Demo message types for QWD/MVD format
 * 
 * Original: dem_cmd, dem_read, dem_set, etc.
 */
enum class DemoMessageType : uint8_t {
    Cmd = 0,       ///< User cmd movement message
    Read = 1,      ///< Network message
    Set = 2,       ///< Sequence numbers (once at demo start)
    Multiple = 3,  ///< MVD: directed to several clients (mask)
    Single = 4,    ///< MVD: directed to single client
    Stats = 5,     ///< MVD: stats update for a player
    All = 6        ///< MVD: directed to all clients
};

/**
 * @brief Get name of demo message type for debugging
 */
constexpr std::string_view demoMessageTypeName(DemoMessageType type) noexcept {
    switch (type) {
        case DemoMessageType::Cmd:      return "cmd";
        case DemoMessageType::Read:     return "read";
        case DemoMessageType::Set:      return "set";
        case DemoMessageType::Multiple: return "multiple";
        case DemoMessageType::Single:   return "single";
        case DemoMessageType::Stats:    return "stats";
        case DemoMessageType::All:      return "all";
    }
    return "unknown";
}

/**
 * @brief Check if message type is MVD-specific
 */
constexpr bool isMvdMessageType(DemoMessageType type) noexcept {
    return type >= DemoMessageType::Multiple;
}

//=============================================================================
// Demo Format Types
//=============================================================================

/**
 * @brief Demo file format types
 */
enum class DemoFormat : uint8_t {
    None = 0,    ///< No demo
    Qwd = 1,     ///< QuakeWorld demo (.qwd)
    Mvd = 2,     ///< Multi-View Demo (.mvd)
    Qwz = 3,     ///< Compressed QWD (.qwz)
    Dem = 4,     ///< NetQuake demo (.dem)
    Qtv = 5      ///< QTV stream (live)
};

/**
 * @brief Get file extension for demo format
 */
constexpr std::string_view demoFormatExtension(DemoFormat format) noexcept {
    switch (format) {
        case DemoFormat::Qwd: return ".qwd";
        case DemoFormat::Mvd: return ".mvd";
        case DemoFormat::Qwz: return ".qwz";
        case DemoFormat::Dem: return ".dem";
        case DemoFormat::Qtv: return "";
        default: return "";
    }
}

/**
 * @brief Get format name for display
 */
constexpr std::string_view demoFormatName(DemoFormat format) noexcept {
    switch (format) {
        case DemoFormat::None: return "none";
        case DemoFormat::Qwd:  return "QWD";
        case DemoFormat::Mvd:  return "MVD";
        case DemoFormat::Qwz:  return "QWZ";
        case DemoFormat::Dem:  return "DEM";
        case DemoFormat::Qtv:  return "QTV";
    }
    return "unknown";
}

/**
 * @brief Detect demo format from filename
 */
inline DemoFormat detectDemoFormat(std::string_view filename) noexcept {
    if (filename.size() < 4) return DemoFormat::None;
    
    auto ext = filename.substr(filename.size() - 4);
    if (ext == ".qwd" || ext == ".QWD") return DemoFormat::Qwd;
    if (ext == ".mvd" || ext == ".MVD") return DemoFormat::Mvd;
    if (ext == ".qwz" || ext == ".QWZ") return DemoFormat::Qwz;
    if (ext == ".dem" || ext == ".DEM") return DemoFormat::Dem;
    
    return DemoFormat::None;
}

//=============================================================================
// Demo Playback State
//=============================================================================

/**
 * @brief Demo playback mode
 * 
 * Original: cls.demoplayback, cls.nqdemoplayback, cls.mvdplayback
 */
enum class DemoPlaybackMode : uint8_t {
    None = 0,      ///< Not playing a demo
    Qwd = 1,       ///< Playing QWD demo
    Nq = 2,        ///< Playing NetQuake demo
    MvdFile = 3,   ///< Playing MVD from file
    MvdQtv = 4     ///< Playing from QTV stream
};

/**
 * @brief Check if currently in any demo playback
 */
constexpr bool isPlayingDemo(DemoPlaybackMode mode) noexcept {
    return mode != DemoPlaybackMode::None;
}

/**
 * @brief Check if playing MVD (file or QTV)
 */
constexpr bool isMvdPlayback(DemoPlaybackMode mode) noexcept {
    return mode == DemoPlaybackMode::MvdFile || mode == DemoPlaybackMode::MvdQtv;
}

/**
 * @brief Check if playing from QTV stream
 */
constexpr bool isQtvPlayback(DemoPlaybackMode mode) noexcept {
    return mode == DemoPlaybackMode::MvdQtv;
}

//=============================================================================
// Demo Seeking
//=============================================================================

/**
 * @brief Demo seeking type
 * 
 * Original: demoseekingtype_t
 */
enum class DemoSeekType : uint8_t {
    None = 0,          ///< Not seeking
    Normal = 1,        ///< demo_jump seeking
    DemoMark = 2,      ///< demo_jump_mark seeking
    Status = 3,        ///< demo_jump_status seeking
    Found = 4,         ///< Seeking complete, should stop
    End = 5,           ///< demo_jump_end (intermission or 1s before end)
    FoundNoRewind = 6  ///< Found but don't auto-rewind
};

/**
 * @brief Check if actively seeking through demo
 */
constexpr bool isSeeking(DemoSeekType type) noexcept {
    return type != DemoSeekType::None;
}

/**
 * @brief Check if seeking is complete
 */
constexpr bool isSeekingComplete(DemoSeekType type) noexcept {
    return type == DemoSeekType::Found || type == DemoSeekType::FoundNoRewind;
}

/**
 * @brief Demo seek status match type
 * 
 * Original: demoseekingstatus_matchtype_t
 */
enum class SeekMatchType : uint8_t {
    Equal = 0,
    NotEqual = 1,
    LessThan = 2,
    GreaterThan = 3,
    BitOn = 4,
    BitOff = 5
};

/**
 * @brief Single condition for status-based seeking
 */
struct SeekCondition {
    SeekMatchType type{SeekMatchType::Equal};
    int stat{0};
    int value{0};
    
    /**
     * @brief Evaluate condition against a stat value
     */
    [[nodiscard]] bool evaluate(int actualValue) const noexcept {
        switch (type) {
            case SeekMatchType::Equal:       return actualValue == value;
            case SeekMatchType::NotEqual:    return actualValue != value;
            case SeekMatchType::LessThan:    return actualValue < value;
            case SeekMatchType::GreaterThan: return actualValue > value;
            case SeekMatchType::BitOn:       return (actualValue & value) != 0;
            case SeekMatchType::BitOff:      return (actualValue & value) == 0;
        }
        return false;
    }
};

/**
 * @brief Status-based seeking state
 * 
 * Original: demoseekingstatus_t
 */
struct SeekStatus {
    bool nonMatchingFound{false};  ///< Whether non-matching frame found
    std::vector<SeekCondition> conditions;
    
    /**
     * @brief Clear seeking status
     */
    void clear() {
        nonMatchingFound = false;
        conditions.clear();
    }
    
    /**
     * @brief Check if has conditions to evaluate
     */
    [[nodiscard]] bool hasConditions() const noexcept {
        return !conditions.empty();
    }
};

//=============================================================================
// Demo Timing
//=============================================================================

/**
 * @brief Demo playback timing information
 * 
 * Tracks all time-related state for demo playback including
 * seeking, rewinding, and speed control.
 */
struct DemoTiming {
    double oldTime{0.0};       ///< Previous demo time (olddemotime)
    double nextTime{0.0};      ///< Next expected demo time (nextdemotime)
    double startTime{0.0};     ///< Demo start time (demostarttime)
    double packetTime{0.0};    ///< Current packet timestamp
    double rewindTime{0.0};    ///< Target time for rewinding
    double bufferingTime{0.0}; ///< QTV buffering time
    double speed{1.0};         ///< Playback speed multiplier
    float length{0.0f};        ///< Total demo length in seconds
    
    /**
     * @brief Reset all timing
     */
    void reset() {
        oldTime = 0.0;
        nextTime = 0.0;
        startTime = 0.0;
        packetTime = 0.0;
        rewindTime = 0.0;
        bufferingTime = 0.0;
        speed = 1.0;
        length = 0.0f;
    }
    
    /**
     * @brief Get current position as percentage (0-100)
     */
    [[nodiscard]] float progressPercent(double currentTime) const noexcept {
        if (length <= 0.0f) return 0.0f;
        float elapsed = static_cast<float>(currentTime - startTime);
        return std::min(100.0f, std::max(0.0f, (elapsed / length) * 100.0f));
    }
    
    /**
     * @brief Get elapsed time in demo
     */
    [[nodiscard]] double elapsedTime(double currentTime) const noexcept {
        return currentTime - startTime;
    }
    
    /**
     * @brief Check if playback is paused (speed == 0)
     */
    [[nodiscard]] bool isPaused() const noexcept {
        return speed == 0.0;
    }
    
    /**
     * @brief Set playback speed with clamping
     */
    void setSpeed(double newSpeed) noexcept {
        speed = std::max(0.0, std::min(100.0, newSpeed));
    }
};

//=============================================================================
// Timedemo State
//=============================================================================

/**
 * @brief Timedemo benchmarking mode
 */
enum class TimedemoMode : uint8_t {
    Off = 0,      ///< Normal playback
    Classic = 1,  ///< Classic timedemo (as fast as possible)
    FixedFps = 2  ///< Fixed FPS timedemo
};

/// Timedemo constants
namespace timedemo {
    constexpr int DEFAULT_FPS = 308;   ///< Default fixed FPS
    constexpr int MIN_FPS = 20;        ///< Minimum fixed FPS
    constexpr int MAX_FPS = 10000;     ///< Maximum fixed FPS
}

/**
 * @brief Timedemo statistics
 * 
 * Original: cls.td_* fields
 */
struct TimedemoStats {
    TimedemoMode mode{TimedemoMode::Off};
    double lastFrame{0.0};           ///< To meter one message per frame
    int startFrame{0};               ///< framecount at start
    double startTime{0.0};           ///< realtime at second frame
    double frametime{0.0};           ///< frametime for fixed fps mode
    double nonRenderingTime{0.0};    ///< Time not in rendering loop
    int maxFrametimeMs{0};           ///< Worst ms score
    int maxFrametimeFrame{0};        ///< Frame with worst score
    std::array<int, 1000> frametimeStats{}; ///< 0.1ms buckets
    
    /**
     * @brief Reset all timedemo state
     */
    void reset() {
        mode = TimedemoMode::Off;
        lastFrame = 0.0;
        startFrame = 0;
        startTime = 0.0;
        frametime = 0.0;
        nonRenderingTime = 0.0;
        maxFrametimeMs = 0;
        maxFrametimeFrame = 0;
        frametimeStats.fill(0);
    }
    
    /**
     * @brief Check if timedemo is active
     */
    [[nodiscard]] bool isActive() const noexcept {
        return mode != TimedemoMode::Off;
    }
    
    /**
     * @brief Record a frame time
     */
    void recordFrametime(double ms, int currentFrame) {
        int bucket = static_cast<int>(ms * 10.0); // 0.1ms buckets
        if (bucket >= 0 && static_cast<size_t>(bucket) < frametimeStats.size()) {
            frametimeStats[static_cast<size_t>(bucket)]++;
        }
        int msInt = static_cast<int>(ms);
        if (msInt > maxFrametimeMs) {
            maxFrametimeMs = msInt;
            maxFrametimeFrame = currentFrame;
        }
    }
    
    /**
     * @brief Calculate FPS from timedemo results
     */
    [[nodiscard]] double calculateFps(int currentFrame, double currentTime) const noexcept {
        if (startTime == 0.0 || currentTime <= startTime) return 0.0;
        int frames = currentFrame - startFrame;
        return static_cast<double>(frames) / (currentTime - startTime);
    }
};

//=============================================================================
// Demo Recording State
//=============================================================================

/**
 * @brief Demo recording mode
 */
enum class RecordingMode : uint8_t {
    None = 0,       ///< Not recording
    Manual = 1,     ///< Manual record command
    Auto = 2,       ///< Auto-recording (match start)
    Easy = 3        ///< Easy-record (auto-named)
};

/**
 * @brief Demo recording state
 */
struct RecordingState {
    RecordingMode mode{RecordingMode::None};
    std::string filename;           ///< Current recording filename
    DemoFormat format{DemoFormat::Qwd}; ///< Recording format
    double startTime{0.0};          ///< Recording start time
    size_t bytesWritten{0};         ///< Bytes written so far
    bool useCaching{false};         ///< Using demo cache
    
    /**
     * @brief Check if currently recording
     */
    [[nodiscard]] bool isRecording() const noexcept {
        return mode != RecordingMode::None;
    }
    
    /**
     * @brief Reset recording state
     */
    void reset() {
        mode = RecordingMode::None;
        filename.clear();
        format = DemoFormat::Qwd;
        startTime = 0.0;
        bytesWritten = 0;
        useCaching = false;
    }
};

//=============================================================================
// Demo Playback State
//=============================================================================

/**
 * @brief Complete demo playback state
 */
struct PlaybackState {
    DemoPlaybackMode mode{DemoPlaybackMode::None};
    DemoFormat format{DemoFormat::None};
    std::string filename;
    DemoTiming timing;
    DemoSeekType seekType{DemoSeekType::None};
    SeekStatus seekStatus;
    TimedemoStats timedemo;
    bool rewinding{false};
    int lastTo{0};                  ///< Last dem_multiple/single target
    DemoMessageType lastType{DemoMessageType::Read}; ///< Last message type
    bool findTrack{false};          ///< Looking for player to track
    
    /**
     * @brief Check if playing any demo
     */
    [[nodiscard]] bool isPlaying() const noexcept {
        return mode != DemoPlaybackMode::None;
    }
    
    /**
     * @brief Check if seeking
     */
    [[nodiscard]] bool isSeeking() const noexcept {
        return seekType != DemoSeekType::None;
    }
    
    /**
     * @brief Check if playback is paused
     */
    [[nodiscard]] bool isPaused() const noexcept {
        return isPlaying() && timing.isPaused() && !isSeeking() && !timedemo.isActive();
    }
    
    /**
     * @brief Reset all playback state
     */
    void reset() {
        mode = DemoPlaybackMode::None;
        format = DemoFormat::None;
        filename.clear();
        timing.reset();
        seekType = DemoSeekType::None;
        seekStatus.clear();
        timedemo.reset();
        rewinding = false;
        lastTo = 0;
        lastType = DemoMessageType::Read;
        findTrack = false;
    }
};

//=============================================================================
// QTV-specific Types
//=============================================================================

/**
 * @brief QTV connection state
 */
enum class QtvState : uint8_t {
    Disconnected = 0,
    Connecting = 1,
    Buffering = 2,
    Playing = 3
};

/**
 * @brief QTV stream information
 */
struct QtvInfo {
    QtvState state{QtvState::Disconnected};
    std::string source;           ///< QTV source address
    float serverVersion{0.0f};    ///< qtvsv/proxy version
    int ezquakeExtensions{0};     ///< Supported extensions
    bool doNotBuffer{false};      ///< Skip buffering
    size_t bufferSize{0};         ///< Current buffer size
    size_t bufferTarget{0};       ///< Target buffer size
    
    /**
     * @brief Check if connected to QTV
     */
    [[nodiscard]] bool isConnected() const noexcept {
        return state != QtvState::Disconnected;
    }
    
    /**
     * @brief Check if actively playing
     */
    [[nodiscard]] bool isPlaying() const noexcept {
        return state == QtvState::Playing;
    }
    
    /**
     * @brief Get buffering progress (0-100)
     */
    [[nodiscard]] float bufferProgress() const noexcept {
        if (bufferTarget == 0) return 100.0f;
        return std::min(100.0f, static_cast<float>(bufferSize) * 100.0f / static_cast<float>(bufferTarget));
    }
    
    /**
     * @brief Reset QTV state
     */
    void reset() {
        state = QtvState::Disconnected;
        source.clear();
        serverVersion = 0.0f;
        ezquakeExtensions = 0;
        doNotBuffer = false;
        bufferSize = 0;
        bufferTarget = 0;
    }
};

//=============================================================================
// NetQuake Demo Types
//=============================================================================

/// NetQuake protocol constants
namespace nq_demo {
    constexpr int PROTOCOL_VERSION = 15;
    constexpr int MAX_CLIENTS = 16;
    constexpr int SIGNONS = 4;
    constexpr int MAX_EDICTS = 600;
    
    /// NQ entity update flags
    namespace update {
        constexpr int MOREBITS   = (1 << 0);
        constexpr int ORIGIN1    = (1 << 1);
        constexpr int ORIGIN2    = (1 << 2);
        constexpr int ORIGIN3    = (1 << 3);
        constexpr int ANGLE2     = (1 << 4);
        constexpr int NOLERP     = (1 << 5);
        constexpr int FRAME      = (1 << 6);
        constexpr int SIGNAL     = (1 << 7);
        constexpr int ANGLE1     = (1 << 8);
        constexpr int ANGLE3     = (1 << 9);
        constexpr int MODEL      = (1 << 10);
        constexpr int COLORMAP   = (1 << 11);
        constexpr int SKIN       = (1 << 12);
        constexpr int EFFECTS    = (1 << 13);
        constexpr int LONGENTITY = (1 << 14);
    }
}

/**
 * @brief NetQuake demo playback state
 */
struct NqDemoState {
    int signon{0};           ///< Current signon stage
    int numEntities{0};      ///< Number of entities
    int viewEntity{0};       ///< View entity number
    int forceCdTrack{0};     ///< Forced CD track
    int maxClients{0};       ///< Max clients for this demo
    float mtime[2]{0, 0};    ///< Message times
    bool standardQuake{true}; ///< Standard vs Hipnotic/Rogue
    bool playerTeleported{false}; ///< Hack for teleport detection
    
    /**
     * @brief Reset NQ demo state
     */
    void reset() {
        signon = 0;
        numEntities = 0;
        viewEntity = 0;
        forceCdTrack = 0;
        maxClients = 0;
        mtime[0] = mtime[1] = 0;
        standardQuake = true;
        playerTeleported = false;
    }
    
    /**
     * @brief Check if signon is complete
     */
    [[nodiscard]] bool isSignonComplete() const noexcept {
        return signon >= nq_demo::SIGNONS;
    }
};

//=============================================================================
// Demo Cache
//=============================================================================

/// Demo cache constants
namespace demo_cache {
    constexpr size_t MIN_SIZE = 2 * 1024 * 1024;    ///< 2 MB minimum
    constexpr size_t FLUSH_SIZE = 1 * 1024 * 1024;  ///< 1 MB flush threshold
}

/**
 * @brief Demo recording cache for buffered writes
 */
class DemoCache {
public:
    DemoCache() = default;
    explicit DemoCache(size_t size) { resize(size); }
    
    /**
     * @brief Resize the cache buffer
     */
    void resize(size_t size) {
        buffer_.resize(std::max(size, demo_cache::MIN_SIZE));
        clear();
    }
    
    /**
     * @brief Clear the cache
     */
    void clear() noexcept {
        writePos_ = 0;
    }
    
    /**
     * @brief Check if cache is enabled
     */
    [[nodiscard]] bool isEnabled() const noexcept {
        return !buffer_.empty();
    }
    
    /**
     * @brief Get current cache usage
     */
    [[nodiscard]] size_t size() const noexcept {
        return writePos_;
    }
    
    /**
     * @brief Get cache capacity
     */
    [[nodiscard]] size_t capacity() const noexcept {
        return buffer_.size();
    }
    
    /**
     * @brief Get available space
     */
    [[nodiscard]] size_t available() const noexcept {
        return buffer_.size() - writePos_;
    }
    
    /**
     * @brief Check if cache needs flushing
     */
    [[nodiscard]] bool needsFlush() const noexcept {
        return writePos_ >= buffer_.size() - demo_cache::FLUSH_SIZE;
    }
    
    /**
     * @brief Write data to cache
     * @return Number of bytes written
     */
    size_t write(const void* data, size_t size) {
        if (!isEnabled() || size > available()) {
            return 0;
        }
        std::memcpy(buffer_.data() + writePos_, data, size);
        writePos_ += size;
        return size;
    }
    
    /**
     * @brief Get data pointer for flushing
     */
    [[nodiscard]] const uint8_t* data() const noexcept {
        return buffer_.data();
    }
    
    /**
     * @brief Flush first N bytes from cache
     */
    void flush(size_t bytes) {
        if (bytes >= writePos_) {
            clear();
        } else {
            std::memmove(buffer_.data(), buffer_.data() + bytes, writePos_ - bytes);
            writePos_ -= bytes;
        }
    }

private:
    std::vector<uint8_t> buffer_;
    size_t writePos_{0};
};

} // namespace ezquake
