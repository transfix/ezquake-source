/**
 * @file demo_player.hpp
 * @brief Demo playback functionality
 * 
 * Converted from cl_demo.c playback functions.
 * Handles reading and playing QWD/MVD format demos.
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
#include <span>
#include "demo_types.hpp"
#include "demo_recorder.hpp"

namespace ezquake {

/**
 * @brief Result of reading a demo message
 */
struct DemoReadResult {
    bool success{false};           ///< Read succeeded
    bool endOfDemo{false};         ///< Reached end of demo
    DemoMessageType type{DemoMessageType::Read}; ///< Message type
    float time{0.0f};              ///< Message timestamp
    std::vector<uint8_t> data;     ///< Message data
    int target{0};                 ///< Target player(s) for MVD
    
    /**
     * @brief Create success result
     */
    static DemoReadResult ok(DemoMessageType t, float tm, std::vector<uint8_t> d, int tgt = 0) {
        return {true, false, t, tm, std::move(d), tgt};
    }
    
    /**
     * @brief Create end-of-demo result
     */
    static DemoReadResult eof() {
        return {false, true, DemoMessageType::Read, 0.0f, {}, 0};
    }
    
    /**
     * @brief Create error result
     */
    static DemoReadResult error() {
        return {false, false, DemoMessageType::Read, 0.0f, {}, 0};
    }
};

/**
 * @brief Demo playback interface
 * 
 * Handles reading demo files and managing playback state
 * including seeking, speed control, and timedemo.
 */
class DemoPlayer {
public:
    /// Callback for demo messages
    using MessageCallback = std::function<void(const DemoReadResult&)>;
    
    /// Callback for playback events
    using EventCallback = std::function<void()>;
    
    DemoPlayer() = default;
    ~DemoPlayer();
    
    // Non-copyable, movable
    DemoPlayer(const DemoPlayer&) = delete;
    DemoPlayer& operator=(const DemoPlayer&) = delete;
    DemoPlayer(DemoPlayer&&) noexcept = default;
    DemoPlayer& operator=(DemoPlayer&&) noexcept = default;
    
    //=========================================================================
    // Playback Control
    //=========================================================================
    
    /**
     * @brief Start playing a demo file
     * 
     * @param path Path to demo file
     * @return true if playback started successfully
     */
    [[nodiscard]] bool start(const std::filesystem::path& path);
    
    /**
     * @brief Stop playback
     */
    void stop();
    
    /**
     * @brief Check if currently playing
     */
    [[nodiscard]] bool isPlaying() const noexcept {
        return state_.isPlaying();
    }
    
    /**
     * @brief Get current playback state
     */
    [[nodiscard]] const PlaybackState& state() const noexcept {
        return state_;
    }
    
    /**
     * @brief Get mutable playback state (for seeking etc.)
     */
    [[nodiscard]] PlaybackState& state() noexcept {
        return state_;
    }
    
    /**
     * @brief Get demo format
     */
    [[nodiscard]] DemoFormat format() const noexcept {
        return state_.format;
    }
    
    /**
     * @brief Get demo filename
     */
    [[nodiscard]] std::string_view filename() const noexcept {
        return state_.filename;
    }
    
    /**
     * @brief Get demo length in seconds
     */
    [[nodiscard]] float length() const noexcept {
        return state_.timing.length;
    }
    
    //=========================================================================
    // Playback Speed
    //=========================================================================
    
    /**
     * @brief Get current playback speed
     */
    [[nodiscard]] double speed() const noexcept {
        return state_.timing.speed;
    }
    
    /**
     * @brief Set playback speed
     * @param speed Speed multiplier (0 = paused, 1 = normal)
     */
    void setSpeed(double speed) noexcept {
        state_.timing.setSpeed(speed);
    }
    
    /**
     * @brief Pause playback
     */
    void pause() noexcept {
        previousSpeed_ = state_.timing.speed;
        state_.timing.speed = 0.0;
    }
    
    /**
     * @brief Resume playback at previous speed
     */
    void resume() noexcept {
        state_.timing.speed = previousSpeed_ > 0.0 ? previousSpeed_ : 1.0;
    }
    
    /**
     * @brief Toggle pause state
     */
    void togglePause() noexcept {
        if (state_.timing.isPaused()) {
            resume();
        } else {
            pause();
        }
    }
    
    /**
     * @brief Check if paused
     */
    [[nodiscard]] bool isPaused() const noexcept {
        return state_.isPaused();
    }
    
    //=========================================================================
    // Seeking
    //=========================================================================
    
    /**
     * @brief Jump to specific time in demo
     * 
     * @param targetTime Target time in seconds from start
     * @param seekType Type of seek operation
     */
    void jumpTo(double targetTime, DemoSeekType seekType = DemoSeekType::Normal);
    
    /**
     * @brief Jump relative to current position
     * 
     * @param offset Time offset in seconds (negative = rewind)
     */
    void jumpRelative(double offset);
    
    /**
     * @brief Jump to demo mark
     */
    void jumpToMark();
    
    /**
     * @brief Jump to end of demo
     */
    void jumpToEnd();
    
    /**
     * @brief Set demo mark at current position
     */
    void setMark();
    
    /**
     * @brief Cancel current seek operation
     */
    void cancelSeek() noexcept {
        state_.seekType = DemoSeekType::None;
    }
    
    /**
     * @brief Check if seeking
     */
    [[nodiscard]] bool isSeeking() const noexcept {
        return state_.isSeeking();
    }
    
    /**
     * @brief Start rewind operation
     */
    void startRewind(double targetTime);
    
    /**
     * @brief Stop rewind operation
     */
    void stopRewind();
    
    /**
     * @brief Check if rewinding
     */
    [[nodiscard]] bool isRewinding() const noexcept {
        return state_.rewinding;
    }
    
    //=========================================================================
    // Timedemo
    //=========================================================================
    
    /**
     * @brief Start timedemo (classic mode - fast as possible)
     */
    void startTimedemo();
    
    /**
     * @brief Start fixed-FPS timedemo
     * @param fps Target FPS for timing
     */
    void startTimedemoFixed(int fps = timedemo::DEFAULT_FPS);
    
    /**
     * @brief Stop timedemo and report results
     */
    void stopTimedemo();
    
    /**
     * @brief Check if timedemo is active
     */
    [[nodiscard]] bool isTimedemo() const noexcept {
        return state_.timedemo.isActive();
    }
    
    /**
     * @brief Get timedemo stats
     */
    [[nodiscard]] const TimedemoStats& timedemoStats() const noexcept {
        return state_.timedemo;
    }
    
    //=========================================================================
    // Reading Demo Data
    //=========================================================================
    
    /**
     * @brief Read next demo message
     * 
     * @return Read result containing message data or error
     */
    [[nodiscard]] DemoReadResult readMessage();
    
    /**
     * @brief Peek at next message without consuming
     */
    [[nodiscard]] DemoReadResult peekMessage();
    
    /**
     * @brief Read user command from demo
     * 
     * Original: CL_Demo_Read_Cmd
     */
    [[nodiscard]] std::optional<DemoUserCmd> readUserCmd();
    
    /**
     * @brief Get current file position
     */
    [[nodiscard]] size_t position() const noexcept {
        return filePosition_;
    }
    
    /**
     * @brief Get file size
     */
    [[nodiscard]] size_t fileSize() const noexcept {
        return fileSize_;
    }
    
    /**
     * @brief Get playback progress (0-100)
     */
    [[nodiscard]] float progress() const noexcept {
        if (fileSize_ == 0) return 0.0f;
        return static_cast<float>(filePosition_) * 100.0f / static_cast<float>(fileSize_);
    }
    
    //=========================================================================
    // Callbacks
    //=========================================================================
    
    /**
     * @brief Set callback for demo end
     */
    void setEndCallback(EventCallback callback) {
        endCallback_ = std::move(callback);
    }
    
    /**
     * @brief Set callback for seek complete
     */
    void setSeekCallback(EventCallback callback) {
        seekCallback_ = std::move(callback);
    }
    
private:
    /**
     * @brief Read demo header
     */
    [[nodiscard]] std::optional<DemoHeader> readHeader();
    
    /**
     * @brief Read raw bytes from file
     */
    [[nodiscard]] size_t readRaw(void* buffer, size_t size);
    
    /**
     * @brief Read little-endian float
     */
    [[nodiscard]] std::optional<float> readFloat();
    
    /**
     * @brief Read little-endian int32
     */
    [[nodiscard]] std::optional<int32_t> readInt32();
    
    /**
     * @brief Read single byte
     */
    [[nodiscard]] std::optional<uint8_t> readByte();
    
    /**
     * @brief Detect demo format from file
     */
    [[nodiscard]] DemoFormat detectFormat(const std::filesystem::path& path);
    
    /**
     * @brief Calculate demo length
     */
    void calculateLength();
    
    /**
     * @brief Seek to file position
     */
    bool seekTo(size_t position);
    
    PlaybackState state_;
    std::ifstream file_;
    size_t filePosition_{0};
    size_t fileSize_{0};
    double previousSpeed_{1.0};
    double markTime_{0.0};
    EventCallback endCallback_;
    EventCallback seekCallback_;
};

/**
 * @brief Demo playlist for sequential playback
 */
class DemoPlaylist {
public:
    static constexpr size_t MAX_DEMOS = 8;
    static constexpr size_t MAX_NAME_LENGTH = 16;
    
    DemoPlaylist() = default;
    
    /**
     * @brief Add demo to playlist
     */
    bool add(std::string_view name) {
        if (demos_.size() >= MAX_DEMOS) return false;
        demos_.emplace_back(name.substr(0, MAX_NAME_LENGTH));
        return true;
    }
    
    /**
     * @brief Clear playlist
     */
    void clear() {
        demos_.clear();
        currentIndex_ = 0;
    }
    
    /**
     * @brief Get current demo name
     */
    [[nodiscard]] std::optional<std::string_view> current() const {
        if (currentIndex_ >= demos_.size()) return std::nullopt;
        return demos_[currentIndex_];
    }
    
    /**
     * @brief Advance to next demo
     * @return true if there is a next demo
     */
    bool next() {
        if (currentIndex_ + 1 >= demos_.size()) return false;
        currentIndex_++;
        return true;
    }
    
    /**
     * @brief Go to previous demo
     * @return true if there is a previous demo
     */
    bool previous() {
        if (currentIndex_ == 0) return false;
        currentIndex_--;
        return true;
    }
    
    /**
     * @brief Check if playlist is empty
     */
    [[nodiscard]] bool empty() const noexcept { return demos_.empty(); }
    
    /**
     * @brief Get playlist size
     */
    [[nodiscard]] size_t size() const noexcept { return demos_.size(); }
    
    /**
     * @brief Get current index
     */
    [[nodiscard]] size_t index() const noexcept { return currentIndex_; }
    
    /**
     * @brief Check if at end of playlist
     */
    [[nodiscard]] bool atEnd() const noexcept {
        return currentIndex_ >= demos_.size();
    }

private:
    std::vector<std::string> demos_;
    size_t currentIndex_{0};
};

} // namespace ezquake
