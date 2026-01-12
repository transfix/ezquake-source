/*
 * ezQuake C++ Port - Client Persistent State
 * 
 * Persistent state that survives across server connections.
 * 
 * Copyright (C) 2024 ezQuake Team
 */

#pragma once

#include "core/types.hpp"
#include "core/client/client_types.hpp"
#include "core/net/net_types.hpp"
#include "core/net/netchan.hpp"
#include <array>
#include <cstdint>
#include <string>
#include <optional>

namespace ezquake {

//=============================================================================
// Download/Upload State
//=============================================================================

/**
 * @brief Download state and progress tracking
 */
class DownloadState {
public:
    DownloadState() noexcept = default;
    
    // Download type and progress
    [[nodiscard]] client::DownloadType type() const noexcept { return type_; }
    [[nodiscard]] int percent() const noexcept { return percent_; }
    [[nodiscard]] int rate() const noexcept { return rate_; }
    [[nodiscard]] double startTime() const noexcept { return startTime_; }
    [[nodiscard]] int number() const noexcept { return number_; }
    [[nodiscard]] bool isActive() const noexcept { return type_ != client::DownloadType::None; }
    
    [[nodiscard]] std::string_view name() const noexcept { return name_; }
    [[nodiscard]] std::string_view tempName() const noexcept { return tempName_; }
    
    void start(client::DownloadType type, std::string_view name, double time) {
        type_ = type;
        name_ = name;
        tempName_ = std::string(name) + ".tmp";
        percent_ = 0;
        rate_ = 0;
        startTime_ = time;
        number_++;
    }
    
    void updateProgress(int percent, int rate) noexcept {
        percent_ = percent;
        rate_ = rate;
    }
    
    void finish() noexcept {
        type_ = client::DownloadType::None;
        percent_ = 100;
    }
    
    void cancel() noexcept {
        type_ = client::DownloadType::None;
        percent_ = 0;
    }
    
    void clear() {
        type_ = client::DownloadType::None;
        name_.clear();
        tempName_.clear();
        number_ = 0;
        percent_ = 0;
        rate_ = 0;
        startTime_ = 0.0;
    }
    
private:
    client::DownloadType type_ = client::DownloadType::None;
    std::string name_;
    std::string tempName_;
    int number_ = 0;
    int percent_ = 0;
    int rate_ = 0;
    double startTime_ = 0.0;
};

/**
 * @brief Upload state and progress tracking
 */
class UploadState {
public:
    UploadState() noexcept = default;
    
    [[nodiscard]] bool isActive() const noexcept { return !name_.empty(); }
    [[nodiscard]] std::string_view name() const noexcept { return name_; }
    [[nodiscard]] int percent() const noexcept { return percent_; }
    [[nodiscard]] int rate() const noexcept { return rate_; }
    [[nodiscard]] int position() const noexcept { return position_; }
    [[nodiscard]] int size() const noexcept { return size_; }
    [[nodiscard]] bool isFile() const noexcept { return isFile_; }
    
    void start(std::string_view name, int size, bool isFile) {
        name_ = name;
        size_ = size;
        position_ = 0;
        percent_ = 0;
        rate_ = 0;
        isFile_ = isFile;
    }
    
    void updateProgress(int position, int rate) noexcept {
        position_ = position;
        rate_ = rate;
        if (size_ > 0) {
            percent_ = (position * 100) / size_;
        }
    }
    
    void finish() {
        percent_ = 100;
        name_.clear();
    }
    
    void clear() {
        name_.clear();
        percent_ = 0;
        rate_ = 0;
        position_ = 0;
        size_ = 0;
        isFile_ = false;
    }
    
private:
    std::string name_;
    int percent_ = 0;
    int rate_ = 0;
    int position_ = 0;
    int size_ = 0;
    bool isFile_ = false;
};

//=============================================================================
// Demo State
//=============================================================================

/**
 * @brief Demo recording and playback state
 */
class DemoState {
public:
    // Timedemo modes
    enum class TimedemoMode : std::uint8_t {
        Off = 0,
        Classic = 1,
        FixedFps = 2
    };
    
    DemoState() noexcept = default;
    
    // Recording
    [[nodiscard]] bool isRecording() const noexcept { return recording_; }
    [[nodiscard]] bool isMvdRecording() const noexcept { return mvdRecording_; }
    
    void startRecording(std::string_view name) {
        name_ = name;
        recording_ = true;
    }
    
    void stopRecording() noexcept {
        recording_ = false;
    }
    
    // Playback
    [[nodiscard]] bool isPlaying() const noexcept { return playing_; }
    [[nodiscard]] bool isNqDemo() const noexcept { return nqPlayback_; }
    [[nodiscard]] bool isMvdPlayback() const noexcept { return mvdPlayback_ != 0; }
    [[nodiscard]] bool isQtvPlayback() const noexcept { return mvdPlayback_ == 2; }
    [[nodiscard]] int mvdPlayback() const noexcept { return mvdPlayback_; }
    
    void startPlayback(std::string_view name, bool isNq = false) {
        name_ = name;
        playing_ = true;
        nqPlayback_ = isNq;
    }
    
    void stopPlayback() noexcept {
        playing_ = false;
        nqPlayback_ = false;
        mvdPlayback_ = 0;
    }
    
    void setMvdPlayback(int mode) noexcept { mvdPlayback_ = mode; }
    
    // Seeking
    [[nodiscard]] client::DemoSeekType seekType() const noexcept { return seekType_; }
    [[nodiscard]] bool isSeeking() const noexcept { return seekType_ != client::DemoSeekType::None; }
    [[nodiscard]] bool isRewinding() const noexcept { return rewinding_; }
    [[nodiscard]] double rewindTime() const noexcept { return rewindTime_; }
    
    void startSeeking(client::DemoSeekType type) noexcept { seekType_ = type; }
    void stopSeeking() noexcept { seekType_ = client::DemoSeekType::None; }
    void startRewind(double time) noexcept { rewinding_ = true; rewindTime_ = time; }
    void stopRewind() noexcept { rewinding_ = false; }
    
    // Timedemo
    [[nodiscard]] TimedemoMode timedemo() const noexcept { return timedemo_; }
    [[nodiscard]] bool isTimedemo() const noexcept { return timedemo_ != TimedemoMode::Off; }
    [[nodiscard]] int timedemoStartFrame() const noexcept { return tdStartFrame_; }
    [[nodiscard]] double timedemoStartTime() const noexcept { return tdStartTime_; }
    
    void startTimedemo(TimedemoMode mode, int frameCount, double time) noexcept {
        timedemo_ = mode;
        tdStartFrame_ = frameCount;
        tdStartTime_ = time;
        tdLastFrame_ = time;
    }
    
    void stopTimedemo() noexcept {
        timedemo_ = TimedemoMode::Off;
    }
    
    // Common
    [[nodiscard]] std::string_view name() const noexcept { return name_; }
    [[nodiscard]] double packetTime() const noexcept { return packetTime_; }
    void setPacketTime(double time) noexcept { packetTime_ = time; }
    
    void clear() {
        recording_ = false;
        mvdRecording_ = false;
        playing_ = false;
        nqPlayback_ = false;
        mvdPlayback_ = 0;
        seekType_ = client::DemoSeekType::None;
        rewinding_ = false;
        rewindTime_ = 0.0;
        timedemo_ = TimedemoMode::Off;
        name_.clear();
        packetTime_ = 0.0;
        tdStartFrame_ = 0;
        tdStartTime_ = 0.0;
        tdLastFrame_ = 0.0;
    }
    
private:
    bool recording_ = false;
    bool mvdRecording_ = false;
    bool playing_ = false;
    bool nqPlayback_ = false;
    int mvdPlayback_ = 0;  // 0 = none, 1 = MVD, 2 = QTV
    client::DemoSeekType seekType_ = client::DemoSeekType::None;
    bool rewinding_ = false;
    double rewindTime_ = 0.0;
    TimedemoMode timedemo_ = TimedemoMode::Off;
    std::string name_;
    double packetTime_ = 0.0;
    int tdStartFrame_ = 0;
    double tdStartTime_ = 0.0;
    double tdLastFrame_ = 0.0;
};

//=============================================================================
// Performance Stats
//=============================================================================

/**
 * @brief FPS and frame timing statistics
 */
class PerformanceStats {
public:
    PerformanceStats() noexcept = default;
    
    [[nodiscard]] double fps() const noexcept { return fps_; }
    [[nodiscard]] double minFps() const noexcept { return minFps_; }
    [[nodiscard]] double avgFrametime() const noexcept { return avgFrametime_; }
    [[nodiscard]] double maxFrametime() const noexcept { return maxFrametime_; }
    [[nodiscard]] float latency() const noexcept { return latency_; }
    
    void updateFps(double fps) noexcept {
        fps_ = fps;
        if (fps > 0.0 && fps < minFps_) {
            minFps_ = fps;
        }
    }
    
    void updateFrametime(double frametime) noexcept {
        avgFrametime_ = frametime;
        if (frametime > maxFrametime_) {
            maxFrametime_ = frametime;
        }
    }
    
    void updateLatency(float latency) noexcept {
        latency_ = latency;
    }
    
    void reset() noexcept {
        fps_ = 0.0;
        minFps_ = 999999.0;
        avgFrametime_ = 0.0;
        maxFrametime_ = 0.0;
        latency_ = 0.0f;
    }
    
private:
    double fps_ = 0.0;
    double minFps_ = 999999.0;
    double avgFrametime_ = 0.0;
    double maxFrametime_ = 0.0;
    float latency_ = 0.0f;
};

//=============================================================================
// QTV State
//=============================================================================

/**
 * @brief QTV (QuakeTV) connection state
 */
class QtvState {
public:
    QtvState() noexcept = default;
    
    [[nodiscard]] float serverVersion() const noexcept { return serverVersion_; }
    [[nodiscard]] int ezquakeExtensions() const noexcept { return ezquakeExtensions_; }
    [[nodiscard]] bool doNotBuffer() const noexcept { return doNotBuffer_; }
    [[nodiscard]] std::string_view source() const noexcept { return source_; }
    
    void setServerVersion(float version) noexcept { serverVersion_ = version; }
    void setEzquakeExtensions(int ext) noexcept { ezquakeExtensions_ = ext; }
    void setDoNotBuffer(bool value) noexcept { doNotBuffer_ = value; }
    void setSource(std::string_view source) { source_ = source; }
    
    void clear() {
        serverVersion_ = 0.0f;
        ezquakeExtensions_ = 0;
        doNotBuffer_ = false;
        source_.clear();
    }
    
private:
    float serverVersion_ = 0.0f;
    int ezquakeExtensions_ = 0;
    bool doNotBuffer_ = false;
    std::string source_;
};

//=============================================================================
// Protocol Extensions
//=============================================================================

/**
 * @brief FTE protocol extension flags
 */
class ProtocolExtensions {
public:
    ProtocolExtensions() noexcept = default;
    
    [[nodiscard]] std::uint32_t fte1() const noexcept { return fte1_; }
    [[nodiscard]] std::uint32_t fte2() const noexcept { return fte2_; }
    [[nodiscard]] std::uint32_t mvd1() const noexcept { return mvd1_; }
    
    void setFte1(std::uint32_t ext) noexcept { fte1_ = ext; }
    void setFte2(std::uint32_t ext) noexcept { fte2_ = ext; }
    void setMvd1(std::uint32_t ext) noexcept { mvd1_ = ext; }
    
    [[nodiscard]] bool hasFteExtension(std::uint32_t flag) const noexcept {
        return (fte1_ & flag) != 0;
    }
    
    [[nodiscard]] bool hasFte2Extension(std::uint32_t flag) const noexcept {
        return (fte2_ & flag) != 0;
    }
    
    [[nodiscard]] bool hasMvdExtension(std::uint32_t flag) const noexcept {
        return (mvd1_ & flag) != 0;
    }
    
    void clear() noexcept {
        fte1_ = 0;
        fte2_ = 0;
        mvd1_ = 0;
    }
    
private:
    std::uint32_t fte1_ = 0;
    std::uint32_t fte2_ = 0;
    std::uint32_t mvd1_ = 0;
};

//=============================================================================
// ClientPersistent - State surviving across server connections
//=============================================================================

/**
 * @brief Persistent client state that survives across server connections.
 * 
 * This contains connection info, demo state, downloads, and other
 * data that persists when switching servers.
 * 
 * Original: clientPersistent_t (cls)
 */
class ClientPersistent {
public:
    ClientPersistent() noexcept = default;
    
    // Connection state
    [[nodiscard]] client::ConnectionState state() const noexcept { return state_; }
    void setState(client::ConnectionState state) noexcept { state_ = state; }
    
    [[nodiscard]] bool isConnected() const noexcept {
        return state_ >= client::ConnectionState::Connected;
    }
    
    [[nodiscard]] bool isActive() const noexcept {
        return state_ == client::ConnectionState::Active;
    }
    
    // Timing
    [[nodiscard]] int frameCount() const noexcept { return frameCount_; }
    [[nodiscard]] double realtime() const noexcept { return realtime_; }
    [[nodiscard]] double frametime() const noexcept { return frametime_; }
    [[nodiscard]] double trueFrametime() const noexcept { return trueFrametime_; }
    [[nodiscard]] double demotime() const noexcept { return demotime_; }
    
    void incrementFrame() noexcept { frameCount_++; }
    void setRealtime(double time) noexcept { realtime_ = time; }
    void setFrametime(double time) noexcept { frametime_ = time; }
    void setTrueFrametime(double time) noexcept { trueFrametime_ = time; }
    void setDemotime(double time) noexcept { demotime_ = time; }
    
    // Server info
    [[nodiscard]] std::string_view serverName() const noexcept { return serverName_; }
    [[nodiscard]] const net::NetAddress& serverAddress() const noexcept { return serverAddress_; }
    [[nodiscard]] int qport() const noexcept { return qport_; }
    [[nodiscard]] int challenge() const noexcept { return challenge_; }
    
    void setServerName(std::string_view name) { serverName_ = name; }
    void setServerAddress(const net::NetAddress& addr) noexcept { serverAddress_ = addr; }
    void setQport(int port) noexcept { qport_ = port; }
    void setChallenge(int challenge) noexcept { challenge_ = challenge; }
    
    // Game directory
    [[nodiscard]] std::string_view gameDir() const noexcept { return gameDir_; }
    [[nodiscard]] std::string_view gameDirFile() const noexcept { return gameDirFile_; }
    void setGameDir(std::string_view dir) { gameDir_ = dir; }
    void setGameDirFile(std::string_view file) { gameDirFile_ = file; }
    
    // Userinfo
    [[nodiscard]] std::string_view userinfo() const noexcept { return userinfo_; }
    void setUserinfo(std::string_view info) { userinfo_ = info; }
    
    // Auth
    [[nodiscard]] std::string_view authToken() const noexcept { return authToken_; }
    void setAuthToken(std::string_view token) { authToken_ = token; }
    
    // Sub-state accessors
    [[nodiscard]] DownloadState& download() noexcept { return download_; }
    [[nodiscard]] const DownloadState& download() const noexcept { return download_; }
    
    [[nodiscard]] UploadState& upload() noexcept { return upload_; }
    [[nodiscard]] const UploadState& upload() const noexcept { return upload_; }
    
    [[nodiscard]] DemoState& demo() noexcept { return demo_; }
    [[nodiscard]] const DemoState& demo() const noexcept { return demo_; }
    
    [[nodiscard]] PerformanceStats& perfStats() noexcept { return perfStats_; }
    [[nodiscard]] const PerformanceStats& perfStats() const noexcept { return perfStats_; }
    
    [[nodiscard]] QtvState& qtv() noexcept { return qtv_; }
    [[nodiscard]] const QtvState& qtv() const noexcept { return qtv_; }
    
    [[nodiscard]] ProtocolExtensions& extensions() noexcept { return extensions_; }
    [[nodiscard]] const ProtocolExtensions& extensions() const noexcept { return extensions_; }
    
    // Demo message tracking
    [[nodiscard]] int lastTo() const noexcept { return lastTo_; }
    [[nodiscard]] int lastType() const noexcept { return lastType_; }
    [[nodiscard]] bool findTrack() const noexcept { return findTrack_; }
    
    void setLastTo(int to) noexcept { lastTo_ = to; }
    void setLastType(int type) noexcept { lastType_ = type; }
    void setFindTrack(bool find) noexcept { findTrack_ = find; }
    
    // Clear persistent state (for full disconnect)
    void clear() {
        state_ = client::ConnectionState::Disconnected;
        frameCount_ = 0;
        realtime_ = 0.0;
        frametime_ = 0.0;
        trueFrametime_ = 0.0;
        demotime_ = 0.0;
        serverName_.clear();
        serverAddress_ = net::NetAddress{};
        qport_ = 0;
        challenge_ = 0;
        gameDir_.clear();
        gameDirFile_.clear();
        userinfo_.clear();
        authToken_.clear();
        download_.clear();
        upload_.clear();
        demo_.clear();
        perfStats_.reset();
        qtv_.clear();
        extensions_.clear();
        lastTo_ = 0;
        lastType_ = 0;
        findTrack_ = false;
    }
    
private:
    client::ConnectionState state_ = client::ConnectionState::Disconnected;
    
    // Timing
    int frameCount_ = 0;
    double realtime_ = 0.0;
    double frametime_ = 0.0;
    double trueFrametime_ = 0.0;
    double demotime_ = 0.0;
    
    // Server connection
    std::string serverName_;
    net::NetAddress serverAddress_;
    int qport_ = 0;
    int challenge_ = 0;
    
    // Game directory
    std::string gameDir_;
    std::string gameDirFile_;
    
    // Userinfo
    std::string userinfo_;
    
    // Auth
    std::string authToken_;
    
    // Sub-states
    DownloadState download_;
    UploadState upload_;
    DemoState demo_;
    PerformanceStats perfStats_;
    QtvState qtv_;
    ProtocolExtensions extensions_;
    
    // Demo message tracking
    int lastTo_ = 0;
    int lastType_ = 0;
    bool findTrack_ = false;
};

} // namespace ezquake
