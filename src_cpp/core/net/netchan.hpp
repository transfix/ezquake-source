// netchan.hpp - Reliable network channel
// Part of ezQuake C++20 conversion - Iteration 7
// 
// NetChannel provides reliable, ordered message delivery over UDP.
// Based on Quake's netchan system.

#pragma once

#include "net_types.hpp"
#include "socket.hpp"
#include <vector>
#include <array>
#include <chrono>
#include <functional>

namespace ezquake::net {

// ============================================================================
// Message Buffer - Read/Write buffer for network messages
// ============================================================================

class MessageBuffer {
public:
    static constexpr size_t DEFAULT_SIZE = protocol::MAX_MSGLEN;
    
    // Constructors
    MessageBuffer();
    explicit MessageBuffer(size_t capacity);
    explicit MessageBuffer(std::span<const Byte> data);
    
    // Writing
    void writeByte(uint8_t value);
    void writeShort(int16_t value);
    void writeLong(int32_t value);
    void writeFloat(float value);
    void writeString(std::string_view str);
    void writeData(std::span<const Byte> data);
    void writeCoord(float value);  // Quake coordinate encoding
    void writeAngle(float value);  // Quake angle encoding
    
    // Reading
    [[nodiscard]] NetResult<uint8_t> readByte();
    [[nodiscard]] NetResult<int16_t> readShort();
    [[nodiscard]] NetResult<int32_t> readLong();
    [[nodiscard]] NetResult<float> readFloat();
    [[nodiscard]] NetResult<std::string> readString();
    [[nodiscard]] NetResult<std::vector<Byte>> readData(size_t count);
    [[nodiscard]] NetResult<float> readCoord();
    [[nodiscard]] NetResult<float> readAngle();
    
    // Buffer management
    void clear();
    void beginReading();
    [[nodiscard]] size_t size() const noexcept { return writePos_; }
    [[nodiscard]] size_t remaining() const noexcept { return writePos_ - readPos_; }
    [[nodiscard]] size_t capacity() const noexcept { return data_.size(); }
    [[nodiscard]] bool hasOverflowed() const noexcept { return overflowed_; }
    [[nodiscard]] bool hasBadRead() const noexcept { return badRead_; }
    [[nodiscard]] bool eof() const noexcept { return readPos_ >= writePos_; }
    
    // Direct data access
    [[nodiscard]] const Byte* data() const noexcept { return data_.data(); }
    [[nodiscard]] Byte* data() noexcept { return data_.data(); }
    [[nodiscard]] std::span<const Byte> span() const noexcept { 
        return {data_.data(), writePos_}; 
    }
    [[nodiscard]] std::span<Byte> span() noexcept { 
        return {data_.data(), writePos_}; 
    }
    
    // Compatibility with raw data
    void setData(std::span<const Byte> newData);
    void setSize(size_t newSize) { writePos_ = std::min(newSize, data_.size()); }
    
private:
    std::vector<Byte> data_;
    size_t readPos_ = 0;
    size_t writePos_ = 0;
    bool overflowed_ = false;
    bool badRead_ = false;
};

// ============================================================================
// NetChannel - Reliable message channel over UDP
// ============================================================================

class NetChannel {
public:
    // Netchan state
    struct Stats {
        float frameLatency = 0.0f;   // Rolling average latency
        float frameRate = 0.0f;      // Rolling average frame rate
        int dropCount = 0;           // Dropped packets this level
        int goodCount = 0;           // Good packets this level
        int droppedSinceLastPacket = 0;  // Packets dropped between last and current
    };
    
    // Constructor
    NetChannel() = default;
    
    /// Setup the channel for communication
    void setup(NetSource source, const NetAddress& remote, int qport, int mtu = 0);
    
    /// Reset statistics (called at level change)
    void resetStats();
    
    // Transmission
    
    /// Transmit a message (may be queued for reliability)
    void transmit(Socket& socket, std::span<const Byte> data);
    
    /// Send out-of-band data (not sequenced, not reliable)
    static void outOfBand(Socket& socket, const NetAddress& to, std::span<const Byte> data);
    
    /// Send out-of-band printf-style message
    static void outOfBandPrint(Socket& socket, const NetAddress& to, std::string_view message);
    
    // Reception
    
    /// Process an incoming packet. Returns true if it was for this channel and valid.
    [[nodiscard]] bool process(std::span<const Byte> packet);
    
    // Rate limiting
    
    /// Can we send a packet now? (rate limiting)
    [[nodiscard]] bool canPacket() const;
    
    /// Can we send a reliable packet? (has pending reliable data)
    [[nodiscard]] bool canReliable() const;
    
    /// Set the rate limit (bytes per second)
    void setRate(double bytesPerSecond);
    
    /// Set duplicate packet count (0 = no duplicates)
    void setDupe(int dupe) { dupe_ = dupe; }
    
    // Accessors
    [[nodiscard]] NetSource source() const noexcept { return source_; }
    [[nodiscard]] const NetAddress& remoteAddress() const noexcept { return remoteAddress_; }
    [[nodiscard]] int qport() const noexcept { return qport_; }
    [[nodiscard]] bool hasFatalError() const noexcept { return fatalError_; }
    [[nodiscard]] const Stats& stats() const noexcept { return stats_; }
    [[nodiscard]] double lastReceived() const noexcept { return lastReceived_; }
    
    // Message buffer for building outgoing reliable messages
    MessageBuffer& message() { return message_; }
    const MessageBuffer& message() const { return message_; }
    
    // Sequence numbers (for debugging/display)
    [[nodiscard]] int incomingSequence() const noexcept { return incomingSequence_; }
    [[nodiscard]] int outgoingSequence() const noexcept { return outgoingSequence_; }
    
private:
    void transmitInternal(Socket& socket, std::span<const Byte> data);
    
    // Connection info
    NetSource source_ = NetSource::Client;
    NetAddress remoteAddress_;
    int qport_ = 0;
    int mtu_ = 0;
    
    // Error state
    bool fatalError_ = false;
    
    // Timing
    double lastReceived_ = 0.0;
    double clearTime_ = 0.0;  // Time when rate limit clears
    double rate_ = 1.0 / 2500.0;  // Seconds per byte (default ~20KB/s)
    
    // Packet duplication for lossy networks
    int dupe_ = 0;
    
    // Sequence numbers
    int incomingSequence_ = 0;
    int incomingAcknowledged_ = 0;
    int incomingReliableAcknowledged_ = 0;  // Single bit
    int incomingReliableSequence_ = 0;       // Single bit
    
    int outgoingSequence_ = 1;  // Start at 1, 0 is "uninitialized"
    int reliableSequence_ = 0;   // Single bit
    int lastReliableSequence_ = 0;
    
    // Reliable message handling
    MessageBuffer message_;  // Outgoing message being built
    MessageBuffer reliableBuffer_;  // Unacknowledged reliable data
    int reliableLength_ = 0;
    
    // Latency tracking
    std::array<int, protocol::MAX_LATENT> outgoingSize_{};
    std::array<double, protocol::MAX_LATENT> outgoingTime_{};
    
    // Statistics
    Stats stats_;
};

// ============================================================================
// Loopback - Local client-server communication
// ============================================================================

class Loopback {
public:
    static constexpr size_t MAX_MESSAGES = protocol::MAX_LOOPBACK;
    
    /// Send a loopback packet
    void send(NetSource from, std::span<const Byte> data);
    
    /// Get a loopback packet (if available)
    [[nodiscard]] std::optional<std::vector<Byte>> receive(NetSource to);
    
    /// Clear all pending messages
    void clear();
    
    /// Check if there are pending messages
    [[nodiscard]] bool hasPending(NetSource to) const;
    
private:
    struct Message {
        std::vector<Byte> data;
        bool valid = false;
    };
    
    std::array<Message, MAX_MESSAGES> clientMessages_{};
    std::array<Message, MAX_MESSAGES> serverMessages_{};
    uint32_t clientGet_ = 0, clientSend_ = 0;
    uint32_t serverGet_ = 0, serverSend_ = 0;
};

// ============================================================================
// Time function (must be provided by application)
// ============================================================================

using TimeFunc = std::function<double()>;

/// Set the time function used for network timing
void setTimeFunction(TimeFunc func);

/// Get current time (seconds)
[[nodiscard]] double getTime();

} // namespace ezquake::net
