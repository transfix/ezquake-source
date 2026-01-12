/**
 * @file demo_player.cpp
 * @brief Demo playback implementation
 */

#include "demo_player.hpp"
#include <cstring>
#include <algorithm>

namespace ezquake {

//=============================================================================
// DemoPlayer
//=============================================================================

DemoPlayer::~DemoPlayer() {
    if (isPlaying()) {
        stop();
    }
}

bool DemoPlayer::start(const std::filesystem::path& path) {
    // Stop any existing playback
    if (isPlaying()) {
        stop();
    }
    
    // Check file exists
    if (!std::filesystem::exists(path)) {
        return false;
    }
    
    // Detect format
    DemoFormat format = detectFormat(path);
    if (format == DemoFormat::None) {
        return false;
    }
    
    // Open file
    file_.open(path, std::ios::binary);
    if (!file_.is_open()) {
        return false;
    }
    
    // Get file size
    file_.seekg(0, std::ios::end);
    fileSize_ = static_cast<size_t>(file_.tellg());
    file_.seekg(0, std::ios::beg);
    filePosition_ = 0;
    
    // Initialize state
    state_.reset();
    state_.filename = path.filename().string();
    state_.format = format;
    
    // Set playback mode based on format
    switch (format) {
        case DemoFormat::Qwd:
        case DemoFormat::Qwz:
            state_.mode = DemoPlaybackMode::Qwd;
            break;
        case DemoFormat::Mvd:
            state_.mode = DemoPlaybackMode::MvdFile;
            break;
        case DemoFormat::Dem:
            state_.mode = DemoPlaybackMode::Nq;
            break;
        default:
            state_.mode = DemoPlaybackMode::Qwd;
            break;
    }
    
    // Calculate demo length
    calculateLength();
    
    return true;
}

void DemoPlayer::stop() {
    if (!isPlaying()) {
        return;
    }
    
    // Stop timedemo if active
    if (isTimedemo()) {
        stopTimedemo();
    }
    
    // Close file
    file_.close();
    
    // Call end callback
    if (endCallback_) {
        endCallback_();
    }
    
    // Reset state
    state_.reset();
    filePosition_ = 0;
    fileSize_ = 0;
}

void DemoPlayer::jumpTo(double targetTime, DemoSeekType seekType) {
    if (!isPlaying()) return;
    
    state_.seekType = seekType;
    state_.timing.rewindTime = targetTime;
    
    // If target is before current position, need to rewind
    double currentTime = state_.timing.elapsedTime(state_.timing.packetTime);
    if (targetTime < currentTime) {
        startRewind(targetTime);
    }
}

void DemoPlayer::jumpRelative(double offset) {
    if (!isPlaying()) return;
    
    double currentTime = state_.timing.elapsedTime(state_.timing.packetTime);
    double targetTime = std::max(0.0, currentTime + offset);
    jumpTo(targetTime);
}

void DemoPlayer::jumpToMark() {
    if (!isPlaying() || markTime_ <= 0.0) return;
    
    jumpTo(markTime_, DemoSeekType::DemoMark);
}

void DemoPlayer::jumpToEnd() {
    if (!isPlaying()) return;
    
    // Jump to 1 second before end
    double targetTime = std::max(0.0, static_cast<double>(state_.timing.length) - 1.0);
    jumpTo(targetTime, DemoSeekType::End);
}

void DemoPlayer::setMark() {
    if (!isPlaying()) return;
    
    markTime_ = state_.timing.elapsedTime(state_.timing.packetTime);
}

void DemoPlayer::startRewind(double targetTime) {
    if (!isPlaying()) return;
    
    state_.rewinding = true;
    state_.timing.rewindTime = targetTime;
    
    // Seek to beginning of file
    seekTo(0);
    state_.timing.oldTime = 0.0;
    state_.timing.nextTime = 0.0;
}

void DemoPlayer::stopRewind() {
    state_.rewinding = false;
    state_.seekType = DemoSeekType::None;
    
    if (seekCallback_) {
        seekCallback_();
    }
}

void DemoPlayer::startTimedemo() {
    if (!isPlaying()) return;
    
    state_.timedemo.reset();
    state_.timedemo.mode = TimedemoMode::Classic;
    state_.timedemo.startTime = 0.0; // Will be set on second frame
    state_.timedemo.startFrame = 0;  // Will be set on second frame
}

void DemoPlayer::startTimedemoFixed(int fps) {
    if (!isPlaying()) return;
    
    fps = std::clamp(fps, timedemo::MIN_FPS, timedemo::MAX_FPS);
    
    state_.timedemo.reset();
    state_.timedemo.mode = TimedemoMode::FixedFps;
    state_.timedemo.frametime = 1.0 / fps;
    state_.timedemo.startTime = 0.0;
    state_.timedemo.startFrame = 0;
}

void DemoPlayer::stopTimedemo() {
    if (!isTimedemo()) return;
    
    // Results would be printed here
    state_.timedemo.reset();
}

DemoReadResult DemoPlayer::readMessage() {
    if (!isPlaying()) {
        return DemoReadResult::error();
    }
    
    // Read header
    auto header = readHeader();
    if (!header) {
        return DemoReadResult::eof();
    }
    
    float time = header->time;
    DemoMessageType type = header->type;
    int target = 0;
    
    // Handle MVD message types
    if (isMvdPlayback(state_.mode) && isMvdMessageType(type)) {
        // Read target byte for MVD messages
        auto targetByte = readByte();
        if (!targetByte) {
            return DemoReadResult::error();
        }
        target = *targetByte;
    }
    
    std::vector<uint8_t> data;
    
    switch (type) {
        case DemoMessageType::Cmd: {
            // User command - fixed size (approximately 21 bytes + view angles)
            data.resize(21 + 12); // cmd + view angles
            if (readRaw(data.data(), data.size()) != data.size()) {
                return DemoReadResult::error();
            }
            break;
        }
        
        case DemoMessageType::Set: {
            // Sequence set - 8 bytes (2 int32s)
            data.resize(8);
            if (readRaw(data.data(), data.size()) != data.size()) {
                return DemoReadResult::error();
            }
            break;
        }
        
        case DemoMessageType::Read:
        case DemoMessageType::Multiple:
        case DemoMessageType::Single:
        case DemoMessageType::Stats:
        case DemoMessageType::All: {
            // Network message - read length first
            auto length = readInt32();
            if (!length || *length <= 0 || *length > 65536) {
                return DemoReadResult::error();
            }
            data.resize(static_cast<size_t>(*length));
            if (readRaw(data.data(), data.size()) != data.size()) {
                return DemoReadResult::error();
            }
            break;
        }
    }
    
    // Update state
    state_.lastType = type;
    state_.lastTo = target;
    state_.timing.packetTime = time;
    
    return DemoReadResult::ok(type, time, std::move(data), target);
}

DemoReadResult DemoPlayer::peekMessage() {
    if (!isPlaying()) {
        return DemoReadResult::error();
    }
    
    // Save position
    size_t savedPos = filePosition_;
    
    // Read message
    auto result = readMessage();
    
    // Restore position
    seekTo(savedPos);
    
    return result;
}

std::optional<DemoUserCmd> DemoPlayer::readUserCmd() {
    auto result = readMessage();
    if (!result.success || result.type != DemoMessageType::Cmd) {
        return std::nullopt;
    }
    
    return DemoUserCmd::deserialize(result.data.data(), result.data.size());
}

std::optional<DemoHeader> DemoPlayer::readHeader() {
    auto time = readFloat();
    auto type = readByte();
    
    if (!time || !type) {
        return std::nullopt;
    }
    
    DemoHeader header;
    header.time = *time;
    header.type = static_cast<DemoMessageType>(*type & 0x07); // Lower 3 bits
    
    return header;
}

size_t DemoPlayer::readRaw(void* buffer, size_t size) {
    if (!file_.is_open() || size == 0) {
        return 0;
    }
    
    file_.read(static_cast<char*>(buffer), static_cast<std::streamsize>(size));
    size_t bytesRead = static_cast<size_t>(file_.gcount());
    filePosition_ += bytesRead;
    
    return bytesRead;
}

std::optional<float> DemoPlayer::readFloat() {
    uint8_t bytes[4];
    if (readRaw(bytes, 4) != 4) {
        return std::nullopt;
    }
    
    // Little-endian float
    uint32_t bits = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
    float value;
    std::memcpy(&value, &bits, sizeof(float));
    
    return value;
}

std::optional<int32_t> DemoPlayer::readInt32() {
    uint8_t bytes[4];
    if (readRaw(bytes, 4) != 4) {
        return std::nullopt;
    }
    
    // Little-endian int32
    return static_cast<int32_t>(bytes[0] | (bytes[1] << 8) | 
                                 (bytes[2] << 16) | (bytes[3] << 24));
}

std::optional<uint8_t> DemoPlayer::readByte() {
    uint8_t byte;
    if (readRaw(&byte, 1) != 1) {
        return std::nullopt;
    }
    return byte;
}

DemoFormat DemoPlayer::detectFormat(const std::filesystem::path& path) {
    return detectDemoFormat(path.extension().string());
}

void DemoPlayer::calculateLength() {
    if (!file_.is_open() || fileSize_ == 0) {
        state_.timing.length = 0.0f;
        return;
    }
    
    // Save current position
    size_t savedPos = filePosition_;
    
    // Scan through file to find last timestamp
    seekTo(0);
    float lastTime = 0.0f;
    float firstTime = 0.0f;
    bool first = true;
    
    while (true) {
        auto time = readFloat();
        if (!time) break;
        
        if (first) {
            firstTime = *time;
            first = false;
        }
        lastTime = *time;
        
        // Skip the rest of the message
        auto type = readByte();
        if (!type) break;
        
        DemoMessageType msgType = static_cast<DemoMessageType>(*type & 0x07);
        
        // Skip based on message type
        if (msgType == DemoMessageType::Cmd) {
            // Fixed size
            if (!seekTo(filePosition_ + 33)) break;
        } else if (msgType == DemoMessageType::Set) {
            // 8 bytes
            if (!seekTo(filePosition_ + 8)) break;
        } else {
            // Read length and skip
            auto length = readInt32();
            if (!length || *length <= 0) break;
            if (!seekTo(filePosition_ + static_cast<size_t>(*length))) break;
        }
    }
    
    state_.timing.length = lastTime - firstTime;
    state_.timing.startTime = firstTime;
    
    // Restore position
    seekTo(savedPos);
}

bool DemoPlayer::seekTo(size_t position) {
    if (!file_.is_open()) {
        return false;
    }
    
    file_.clear(); // Clear any error flags
    file_.seekg(static_cast<std::streamoff>(position), std::ios::beg);
    
    if (file_.fail()) {
        return false;
    }
    
    filePosition_ = position;
    return true;
}

} // namespace ezquake
