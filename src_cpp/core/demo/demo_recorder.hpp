/**
 * @file demo_recorder.hpp
 * @brief Demo recording functionality
 * 
 * Converted from cl_demo.c recording functions.
 * Handles writing QWD format demos with optional caching.
 */

#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <fstream>
#include <filesystem>
#include <functional>
#include <optional>
#include "demo_types.hpp"

namespace ezquake {

/**
 * @brief Demo file header information
 */
struct DemoHeader {
    float time{0.0f};
    DemoMessageType type{DemoMessageType::Read};
    
    static constexpr size_t SIZE = sizeof(float) + sizeof(uint8_t);
};

/**
 * @brief User command for demo recording
 * 
 * Original: usercmd_t structure in demos
 */
struct DemoUserCmd {
    float angles[3]{0, 0, 0};     ///< View angles
    int16_t forwardMove{0};        ///< Forward movement
    int16_t sideMove{0};           ///< Side movement
    int16_t upMove{0};             ///< Up movement
    uint8_t buttons{0};            ///< Button bits
    uint8_t impulse{0};            ///< Impulse command
    uint8_t msec{0};               ///< Milliseconds for command
    
    /**
     * @brief Serialize for demo writing (little-endian)
     */
    [[nodiscard]] std::vector<uint8_t> serialize() const;
    
    /**
     * @brief Deserialize from demo data
     */
    static std::optional<DemoUserCmd> deserialize(const uint8_t* data, size_t size);
};

/**
 * @brief Demo recording interface
 * 
 * Handles writing demo data to files with optional caching
 * for improved I/O performance.
 */
class DemoRecorder {
public:
    using FlushCallback = std::function<void(size_t bytesWritten)>;
    
    DemoRecorder() = default;
    ~DemoRecorder();
    
    // Non-copyable, movable
    DemoRecorder(const DemoRecorder&) = delete;
    DemoRecorder& operator=(const DemoRecorder&) = delete;
    DemoRecorder(DemoRecorder&&) noexcept = default;
    DemoRecorder& operator=(DemoRecorder&&) noexcept = default;
    
    //=========================================================================
    // Recording Control
    //=========================================================================
    
    /**
     * @brief Start recording to a file
     * 
     * @param path Path to demo file
     * @param mode Recording mode (Manual, Auto, Easy)
     * @param useCaching Enable write caching
     * @param cacheSize Cache size if caching enabled
     * @return true if recording started successfully
     */
    [[nodiscard]] bool start(const std::filesystem::path& path,
                             RecordingMode mode = RecordingMode::Manual,
                             bool useCaching = false,
                             size_t cacheSize = demo_cache::MIN_SIZE);
    
    /**
     * @brief Stop recording and close file
     */
    void stop();
    
    /**
     * @brief Check if currently recording
     */
    [[nodiscard]] bool isRecording() const noexcept {
        return state_.isRecording();
    }
    
    /**
     * @brief Get current recording state
     */
    [[nodiscard]] const RecordingState& state() const noexcept {
        return state_;
    }
    
    /**
     * @brief Get current filename
     */
    [[nodiscard]] std::string_view filename() const noexcept {
        return state_.filename;
    }
    
    /**
     * @brief Get bytes written so far
     */
    [[nodiscard]] size_t bytesWritten() const noexcept {
        return state_.bytesWritten;
    }
    
    //=========================================================================
    // Writing Demo Data
    //=========================================================================
    
    /**
     * @brief Write user command to demo
     * 
     * Original: CL_WriteDemoCmd
     * 
     * @param cmd User command
     * @param viewAngles Current view angles
     * @param time Current demo time
     */
    void writeUserCmd(const DemoUserCmd& cmd, const float viewAngles[3], float time);
    
    /**
     * @brief Write network message to demo
     * 
     * Original: CL_WriteDemoMessage
     * 
     * @param data Message data
     * @param size Message size
     * @param time Current demo time
     */
    void writeMessage(const void* data, size_t size, float time);
    
    /**
     * @brief Write startup message with sequence numbers
     * 
     * Original: CL_WriteStartupDemoMessage
     * 
     * @param data Message data
     * @param size Message size
     * @param sequence Sequence number
     * @param time Current demo time
     */
    void writeStartupMessage(const void* data, size_t size, int sequence, float time);
    
    /**
     * @brief Write sequence set message (once at demo start)
     * 
     * Original: CL_WriteSetDemoMessage
     * 
     * @param outgoingSeq Outgoing sequence number
     * @param incomingSeq Incoming sequence number
     * @param time Current demo time
     */
    void writeSetMessage(int outgoingSeq, int incomingSeq, float time);
    
    /**
     * @brief Write raw bytes to demo
     */
    void writeRaw(const void* data, size_t size);
    
    //=========================================================================
    // Callbacks
    //=========================================================================
    
    /**
     * @brief Set callback for flush events
     */
    void setFlushCallback(FlushCallback callback) {
        flushCallback_ = std::move(callback);
    }
    
private:
    /**
     * @brief Write demo header (time + type)
     */
    void writeHeader(float time, DemoMessageType type);
    
    /**
     * @brief Flush cache to file if needed
     */
    void flushIfNeeded();
    
    /**
     * @brief Flush all cached data
     */
    void flushAll();
    
    /**
     * @brief Write little-endian float
     */
    void writeFloat(float value);
    
    /**
     * @brief Write little-endian int32
     */
    void writeInt32(int32_t value);
    
    /**
     * @brief Write single byte
     */
    void writeByte(uint8_t value);
    
    RecordingState state_;
    std::ofstream file_;
    DemoCache cache_;
    FlushCallback flushCallback_;
};

/**
 * @brief Auto-recording helper
 * 
 * Manages automatic demo recording on match start/end.
 */
class AutoRecorder {
public:
    AutoRecorder() = default;
    explicit AutoRecorder(std::shared_ptr<DemoRecorder> recorder)
        : recorder_(std::move(recorder)) {}
    
    /**
     * @brief Enable auto-recording
     */
    void enable(bool enabled = true) { enabled_ = enabled; }
    
    /**
     * @brief Check if auto-recording is enabled
     */
    [[nodiscard]] bool isEnabled() const noexcept { return enabled_; }
    
    /**
     * @brief Check if auto-recording is in progress
     */
    [[nodiscard]] bool isRecording() const noexcept {
        return recorder_ && recorder_->isRecording() && 
               recorder_->state().mode == RecordingMode::Auto;
    }
    
    /**
     * @brief Called when match starts
     * 
     * @param basePath Base path for demo files
     * @param mapName Current map name
     * @param serverName Server name
     * @return true if recording started
     */
    bool onMatchStart(const std::filesystem::path& basePath,
                      std::string_view mapName,
                      std::string_view serverName);
    
    /**
     * @brief Called when match ends
     */
    void onMatchEnd();
    
    /**
     * @brief Generate auto-record filename
     */
    [[nodiscard]] static std::string generateFilename(std::string_view mapName,
                                                       std::string_view serverName);

private:
    std::shared_ptr<DemoRecorder> recorder_;
    bool enabled_{false};
};

} // namespace ezquake
