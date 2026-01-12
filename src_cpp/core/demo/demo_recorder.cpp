/**
 * @file demo_recorder.cpp
 * @brief Demo recording implementation
 */

#include "demo_recorder.hpp"
#include <cstring>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace ezquake {

//=============================================================================
// DemoUserCmd
//=============================================================================

std::vector<uint8_t> DemoUserCmd::serialize() const {
    std::vector<uint8_t> data;
    data.reserve(32);
    
    // Angles (3 floats, little-endian)
    for (int i = 0; i < 3; ++i) {
        uint32_t bits;
        std::memcpy(&bits, &angles[i], sizeof(float));
        data.push_back(static_cast<uint8_t>(bits & 0xFF));
        data.push_back(static_cast<uint8_t>((bits >> 8) & 0xFF));
        data.push_back(static_cast<uint8_t>((bits >> 16) & 0xFF));
        data.push_back(static_cast<uint8_t>((bits >> 24) & 0xFF));
    }
    
    // Movement (3 shorts, little-endian)
    auto writeShort = [&data](int16_t val) {
        data.push_back(static_cast<uint8_t>(val & 0xFF));
        data.push_back(static_cast<uint8_t>((val >> 8) & 0xFF));
    };
    writeShort(forwardMove);
    writeShort(sideMove);
    writeShort(upMove);
    
    // Buttons, impulse, msec
    data.push_back(buttons);
    data.push_back(impulse);
    data.push_back(msec);
    
    return data;
}

std::optional<DemoUserCmd> DemoUserCmd::deserialize(const uint8_t* data, size_t size) {
    if (size < 21) return std::nullopt; // 12 + 6 + 3 = 21 bytes minimum
    
    DemoUserCmd cmd;
    size_t pos = 0;
    
    // Angles (3 floats)
    for (int i = 0; i < 3; ++i) {
        uint32_t bits = data[pos] | (data[pos+1] << 8) | 
                       (data[pos+2] << 16) | (data[pos+3] << 24);
        std::memcpy(&cmd.angles[i], &bits, sizeof(float));
        pos += 4;
    }
    
    // Movement (3 shorts)
    auto readShort = [&data, &pos]() -> int16_t {
        int16_t val = static_cast<int16_t>(data[pos] | (data[pos+1] << 8));
        pos += 2;
        return val;
    };
    cmd.forwardMove = readShort();
    cmd.sideMove = readShort();
    cmd.upMove = readShort();
    
    // Buttons, impulse, msec
    cmd.buttons = data[pos++];
    cmd.impulse = data[pos++];
    cmd.msec = data[pos++];
    
    return cmd;
}

//=============================================================================
// DemoRecorder
//=============================================================================

DemoRecorder::~DemoRecorder() {
    if (isRecording()) {
        stop();
    }
}

bool DemoRecorder::start(const std::filesystem::path& path,
                         RecordingMode mode,
                         bool useCaching,
                         size_t cacheSize) {
    // Stop any existing recording
    if (isRecording()) {
        stop();
    }
    
    // Open file for writing
    file_.open(path, std::ios::binary | std::ios::trunc);
    if (!file_.is_open()) {
        return false;
    }
    
    // Initialize state
    state_.mode = mode;
    state_.filename = path.filename().string();
    state_.format = detectDemoFormat(state_.filename);
    if (state_.format == DemoFormat::None) {
        state_.format = DemoFormat::Qwd; // Default to QWD
    }
    state_.startTime = 0.0;
    state_.bytesWritten = 0;
    state_.useCaching = useCaching;
    
    // Setup caching if requested
    if (useCaching) {
        cache_.resize(cacheSize);
    } else {
        cache_.resize(0);
    }
    
    return true;
}

void DemoRecorder::stop() {
    if (!isRecording()) {
        return;
    }
    
    // Flush any remaining cached data
    flushAll();
    
    // Close file
    file_.close();
    
    // Reset state
    state_.reset();
    cache_.clear();
}

void DemoRecorder::writeUserCmd(const DemoUserCmd& cmd, const float viewAngles[3], float time) {
    if (!isRecording()) return;
    
    // Write header
    writeHeader(time, DemoMessageType::Cmd);
    
    // Write user command
    auto cmdData = cmd.serialize();
    writeRaw(cmdData.data(), cmdData.size());
    
    // Write view angles (3 floats)
    for (int i = 0; i < 3; ++i) {
        writeFloat(viewAngles[i]);
    }
    
    flushIfNeeded();
}

void DemoRecorder::writeMessage(const void* data, size_t size, float time) {
    if (!isRecording()) return;
    
    // Write header
    writeHeader(time, DemoMessageType::Read);
    
    // Write message length
    writeInt32(static_cast<int32_t>(size));
    
    // Write message data
    writeRaw(data, size);
    
    flushIfNeeded();
}

void DemoRecorder::writeStartupMessage(const void* data, size_t size, int sequence, float time) {
    if (!isRecording()) return;
    
    // Write header
    writeHeader(time, DemoMessageType::Read);
    
    // Write message length (size + 8 for two sequence numbers)
    writeInt32(static_cast<int32_t>(size + 8));
    
    // Write sequence number twice
    writeInt32(sequence);
    writeInt32(sequence);
    
    // Write message data
    writeRaw(data, size);
    
    flushIfNeeded();
}

void DemoRecorder::writeSetMessage(int outgoingSeq, int incomingSeq, float time) {
    if (!isRecording()) return;
    
    // Write header
    writeHeader(time, DemoMessageType::Set);
    
    // Write sequence numbers
    writeInt32(outgoingSeq);
    writeInt32(incomingSeq);
    
    flushIfNeeded();
}

void DemoRecorder::writeRaw(const void* data, size_t size) {
    if (!isRecording() || size == 0) return;
    
    if (cache_.isEnabled()) {
        // Try to write to cache
        if (cache_.write(data, size) == 0) {
            // Cache full, flush and try again
            flushAll();
            if (cache_.write(data, size) == 0) {
                // Still can't fit, write directly
                file_.write(static_cast<const char*>(data), static_cast<std::streamsize>(size));
            }
        }
    } else {
        // Write directly to file
        file_.write(static_cast<const char*>(data), static_cast<std::streamsize>(size));
    }
    
    state_.bytesWritten += size;
}

void DemoRecorder::writeHeader(float time, DemoMessageType type) {
    writeFloat(time);
    writeByte(static_cast<uint8_t>(type));
}

void DemoRecorder::flushIfNeeded() {
    if (cache_.isEnabled() && cache_.needsFlush()) {
        size_t toFlush = std::min(cache_.size(), demo_cache::FLUSH_SIZE);
        file_.write(reinterpret_cast<const char*>(cache_.data()), static_cast<std::streamsize>(toFlush));
        cache_.flush(toFlush);
        file_.flush();
        
        if (flushCallback_) {
            flushCallback_(toFlush);
        }
    }
}

void DemoRecorder::flushAll() {
    if (cache_.isEnabled() && cache_.size() > 0) {
        file_.write(reinterpret_cast<const char*>(cache_.data()), static_cast<std::streamsize>(cache_.size()));
        
        if (flushCallback_) {
            flushCallback_(cache_.size());
        }
        
        cache_.clear();
    }
    file_.flush();
}

void DemoRecorder::writeFloat(float value) {
    // Little-endian float
    uint32_t bits;
    std::memcpy(&bits, &value, sizeof(float));
    uint8_t bytes[4] = {
        static_cast<uint8_t>(bits & 0xFF),
        static_cast<uint8_t>((bits >> 8) & 0xFF),
        static_cast<uint8_t>((bits >> 16) & 0xFF),
        static_cast<uint8_t>((bits >> 24) & 0xFF)
    };
    writeRaw(bytes, 4);
}

void DemoRecorder::writeInt32(int32_t value) {
    // Little-endian int32
    uint8_t bytes[4] = {
        static_cast<uint8_t>(value & 0xFF),
        static_cast<uint8_t>((value >> 8) & 0xFF),
        static_cast<uint8_t>((value >> 16) & 0xFF),
        static_cast<uint8_t>((value >> 24) & 0xFF)
    };
    writeRaw(bytes, 4);
}

void DemoRecorder::writeByte(uint8_t value) {
    writeRaw(&value, 1);
}

//=============================================================================
// AutoRecorder
//=============================================================================

bool AutoRecorder::onMatchStart(const std::filesystem::path& basePath,
                                std::string_view mapName,
                                std::string_view serverName) {
    if (!enabled_ || !recorder_) {
        return false;
    }
    
    // Generate filename
    std::string filename = generateFilename(mapName, serverName);
    auto fullPath = basePath / filename;
    
    // Start recording
    return recorder_->start(fullPath, RecordingMode::Auto);
}

void AutoRecorder::onMatchEnd() {
    if (isRecording()) {
        recorder_->stop();
    }
}

std::string AutoRecorder::generateFilename(std::string_view mapName,
                                           std::string_view serverName) {
    // Get current timestamp
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d_%H-%M-%S");
    ss << "_" << mapName;
    
    // Add sanitized server name if available
    if (!serverName.empty()) {
        ss << "_";
        for (char c : serverName) {
            if (std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_') {
                ss << c;
            }
        }
    }
    
    ss << ".qwd";
    return ss.str();
}

} // namespace ezquake
