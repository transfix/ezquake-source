// netchan.cpp - Reliable network channel implementation
// Part of ezQuake C++20 conversion - Iteration 7

#include "netchan.hpp"
#include <cstring>
#include <algorithm>

namespace ezquake::net {

// ============================================================================
// Time Function
// ============================================================================

namespace {
    TimeFunc g_timeFunc = []() { return 0.0; };
}

void setTimeFunction(TimeFunc func) {
    if (func) {
        g_timeFunc = std::move(func);
    }
}

double getTime() {
    return g_timeFunc();
}

// ============================================================================
// MessageBuffer Implementation
// ============================================================================

MessageBuffer::MessageBuffer() 
    : data_(DEFAULT_SIZE) 
{}

MessageBuffer::MessageBuffer(size_t capacity) 
    : data_(capacity) 
{}

MessageBuffer::MessageBuffer(std::span<const Byte> data)
    : data_(data.begin(), data.end())
    , writePos_(data.size())
{}

void MessageBuffer::writeByte(uint8_t value) {
    if (writePos_ >= data_.size()) {
        overflowed_ = true;
        return;
    }
    data_[writePos_++] = value;
}

void MessageBuffer::writeShort(int16_t value) {
    if (writePos_ + 2 > data_.size()) {
        overflowed_ = true;
        return;
    }
    data_[writePos_++] = static_cast<Byte>(value & 0xFF);
    data_[writePos_++] = static_cast<Byte>((value >> 8) & 0xFF);
}

void MessageBuffer::writeLong(int32_t value) {
    if (writePos_ + 4 > data_.size()) {
        overflowed_ = true;
        return;
    }
    data_[writePos_++] = static_cast<Byte>(value & 0xFF);
    data_[writePos_++] = static_cast<Byte>((value >> 8) & 0xFF);
    data_[writePos_++] = static_cast<Byte>((value >> 16) & 0xFF);
    data_[writePos_++] = static_cast<Byte>((value >> 24) & 0xFF);
}

void MessageBuffer::writeFloat(float value) {
    union {
        float f;
        int32_t i;
    } dat;
    dat.f = value;
    writeLong(dat.i);
}

void MessageBuffer::writeString(std::string_view str) {
    for (char c : str) {
        writeByte(static_cast<uint8_t>(c));
    }
    writeByte(0);  // Null terminator
}

void MessageBuffer::writeData(std::span<const Byte> data) {
    if (writePos_ + data.size() > data_.size()) {
        overflowed_ = true;
        return;
    }
    std::memcpy(&data_[writePos_], data.data(), data.size());
    writePos_ += data.size();
}

void MessageBuffer::writeCoord(float value) {
    // Quake coordinate: 1/8 unit precision
    writeShort(static_cast<int16_t>(value * 8.0f));
}

void MessageBuffer::writeAngle(float value) {
    // Quake angle: 256 steps per 360 degrees
    writeByte(static_cast<uint8_t>(static_cast<int>(value * 256.0f / 360.0f) & 255));
}

NetResult<uint8_t> MessageBuffer::readByte() {
    if (readPos_ >= writePos_) {
        badRead_ = true;
        return unexpected(NetError::ReceiveFailed);
    }
    return data_[readPos_++];
}

NetResult<int16_t> MessageBuffer::readShort() {
    if (readPos_ + 2 > writePos_) {
        badRead_ = true;
        return unexpected(NetError::ReceiveFailed);
    }
    int16_t value = static_cast<int16_t>(data_[readPos_]) |
                    (static_cast<int16_t>(data_[readPos_ + 1]) << 8);
    readPos_ += 2;
    return value;
}

NetResult<int32_t> MessageBuffer::readLong() {
    if (readPos_ + 4 > writePos_) {
        badRead_ = true;
        return unexpected(NetError::ReceiveFailed);
    }
    int32_t value = static_cast<int32_t>(data_[readPos_]) |
                    (static_cast<int32_t>(data_[readPos_ + 1]) << 8) |
                    (static_cast<int32_t>(data_[readPos_ + 2]) << 16) |
                    (static_cast<int32_t>(data_[readPos_ + 3]) << 24);
    readPos_ += 4;
    return value;
}

NetResult<float> MessageBuffer::readFloat() {
    auto result = readLong();
    if (!result) {
        return unexpected(result.error());
    }
    union {
        int32_t i;
        float f;
    } dat;
    dat.i = *result;
    return dat.f;
}

NetResult<std::string> MessageBuffer::readString() {
    std::string result;
    while (readPos_ < writePos_) {
        char c = static_cast<char>(data_[readPos_++]);
        if (c == 0) {
            return result;
        }
        result += c;
    }
    badRead_ = true;
    return unexpected(NetError::ReceiveFailed);
}

NetResult<std::vector<Byte>> MessageBuffer::readData(size_t count) {
    if (readPos_ + count > writePos_) {
        badRead_ = true;
        return unexpected(NetError::ReceiveFailed);
    }
    auto startIt = data_.begin() + static_cast<std::ptrdiff_t>(readPos_);
    auto endIt = startIt + static_cast<std::ptrdiff_t>(count);
    std::vector<Byte> result(startIt, endIt);
    readPos_ += count;
    return result;
}

NetResult<float> MessageBuffer::readCoord() {
    auto result = readShort();
    if (!result) {
        return unexpected(result.error());
    }
    return static_cast<float>(*result) * (1.0f / 8.0f);
}

NetResult<float> MessageBuffer::readAngle() {
    auto result = readByte();
    if (!result) {
        return unexpected(result.error());
    }
    return static_cast<float>(*result) * (360.0f / 256.0f);
}

void MessageBuffer::clear() {
    readPos_ = 0;
    writePos_ = 0;
    overflowed_ = false;
    badRead_ = false;
}

void MessageBuffer::beginReading() {
    readPos_ = 0;
    badRead_ = false;
}

void MessageBuffer::setData(std::span<const Byte> newData) {
    if (newData.size() > data_.size()) {
        data_.resize(newData.size());
    }
    std::memcpy(data_.data(), newData.data(), newData.size());
    writePos_ = newData.size();
    readPos_ = 0;
    overflowed_ = false;
    badRead_ = false;
}

// ============================================================================
// NetChannel Implementation
// ============================================================================

void NetChannel::setup(NetSource source, const NetAddress& remote, int qport, int mtu) {
    source_ = source;
    remoteAddress_ = remote;
    qport_ = qport;
    mtu_ = mtu > 0 ? mtu : static_cast<int>(protocol::MAX_MSGLEN);
    
    fatalError_ = false;
    lastReceived_ = getTime();
    clearTime_ = 0.0;
    
    incomingSequence_ = 0;
    incomingAcknowledged_ = 0;
    incomingReliableAcknowledged_ = 0;
    incomingReliableSequence_ = 0;
    
    outgoingSequence_ = 1;
    reliableSequence_ = 0;
    lastReliableSequence_ = 0;
    
    message_.clear();
    reliableBuffer_.clear();
    reliableLength_ = 0;
    
    stats_ = Stats{};
    
    outgoingSize_.fill(0);
    outgoingTime_.fill(0.0);
}

void NetChannel::resetStats() {
    stats_.dropCount = 0;
    stats_.goodCount = 0;
}

void NetChannel::transmit(Socket& socket, std::span<const Byte> data) {
    transmitInternal(socket, data);
    
    // Send duplicates if configured
    for (int i = 0; i < dupe_ && dupe_ > 0; ++i) {
        transmitInternal(socket, data);
    }
}

void NetChannel::transmitInternal(Socket& socket, std::span<const Byte> data) {
    // Build the packet header
    MessageBuffer sendBuffer;
    
    // Sequence number with reliable bit
    uint32_t seq = static_cast<uint32_t>(outgoingSequence_);
    
    // If we have unacknowledged reliable data, resend it
    bool sendReliable = false;
    if (reliableLength_ > 0) {
        if (incomingAcknowledged_ > lastReliableSequence_ ||
            incomingReliableAcknowledged_ != reliableSequence_) {
            sendReliable = true;
        }
    }
    
    // If there's pending reliable data in message buffer and no unacked reliable
    if (!sendReliable && message_.size() > 0 && reliableLength_ == 0) {
        // Move message to reliable buffer
        reliableBuffer_.setData(message_.span());
        reliableLength_ = static_cast<int>(message_.size());
        message_.clear();
        
        reliableSequence_ ^= 1;
        sendReliable = true;
    }
    
    if (sendReliable) {
        seq |= protocol::SEQUENCE_RELIABLE_BIT;
    }
    
    // Write sequence numbers
    sendBuffer.writeLong(static_cast<int32_t>(seq));
    sendBuffer.writeLong(incomingSequence_ | 
                         (incomingReliableSequence_ << 31));
    
    // Write qport for client
    if (source_ == NetSource::Client) {
        sendBuffer.writeShort(static_cast<int16_t>(qport_));
    }
    
    // Write reliable data if any
    if (sendReliable && reliableLength_ > 0) {
        sendBuffer.writeData(std::span<const Byte>(
            reliableBuffer_.data(), 
            static_cast<size_t>(reliableLength_)));
        lastReliableSequence_ = outgoingSequence_;
    }
    
    // Write unreliable data
    if (!data.empty()) {
        sendBuffer.writeData(data);
    }
    
    // Track for latency calculation
    size_t seqMod = static_cast<size_t>(outgoingSequence_) & (protocol::MAX_LATENT - 1);
    outgoingSize_[seqMod] = static_cast<int>(sendBuffer.size());
    outgoingTime_[seqMod] = getTime();
    
    // Send it (ignore result - UDP is fire and forget)
    (void)socket.sendTo(remoteAddress_, sendBuffer.span());
    
    // Update rate limiting
    double now = getTime();
    if (clearTime_ < now) {
        clearTime_ = now;
    }
    clearTime_ += static_cast<double>(sendBuffer.size()) * rate_;
    
    ++outgoingSequence_;
}

void NetChannel::outOfBand(Socket& socket, const NetAddress& to, std::span<const Byte> data) {
    MessageBuffer buffer;
    buffer.writeLong(-1);  // Sequence -1 = out of band
    buffer.writeData(data);
    (void)socket.sendTo(to, buffer.span());
}

void NetChannel::outOfBandPrint(Socket& socket, const NetAddress& to, std::string_view message) {
    MessageBuffer buffer;
    buffer.writeLong(-1);  // Sequence -1 = out of band
    buffer.writeString(message);
    (void)socket.sendTo(to, buffer.span());
}

bool NetChannel::process(std::span<const Byte> packet) {
    if (packet.size() < 8) {
        return false;
    }
    
    MessageBuffer msg(packet);
    msg.beginReading();
    
    // Read sequence numbers
    auto seqResult = msg.readLong();
    auto ackResult = msg.readLong();
    
    if (!seqResult || !ackResult) {
        return false;
    }
    
    uint32_t sequence = static_cast<uint32_t>(*seqResult);
    uint32_t sequenceAck = static_cast<uint32_t>(*ackResult);
    
    // Out of band?
    if (sequence == 0xFFFFFFFF) {
        // This should be handled by the caller before process()
        return false;
    }
    
    bool reliableMessage = (sequence & protocol::SEQUENCE_RELIABLE_BIT) != 0;
    int reliableAck = static_cast<int>((sequenceAck >> 31) & 1);
    
    sequence &= ~protocol::SEQUENCE_RELIABLE_BIT;
    sequenceAck &= ~protocol::SEQUENCE_RELIABLE_BIT;
    
    // Discard stale or duplicated packets
    if (static_cast<int>(sequence) <= incomingSequence_) {
        return false;
    }
    
    // Count dropped packets
    stats_.droppedSinceLastPacket = static_cast<int>(sequence) - incomingSequence_ - 1;
    if (stats_.droppedSinceLastPacket > 0) {
        stats_.dropCount += stats_.droppedSinceLastPacket;
    }
    
    // Update latency stats
    if (static_cast<int>(sequenceAck) > 0) {
        size_t ackMod = sequenceAck & (protocol::MAX_LATENT - 1);
        if (outgoingTime_[ackMod] > 0.0) {
            float latency = static_cast<float>(getTime() - outgoingTime_[ackMod]);
            stats_.frameLatency = stats_.frameLatency * protocol::OLD_AVG + 
                                  latency * (1.0f - protocol::OLD_AVG);
        }
    }
    
    // If reliable was acked, clear the reliable buffer
    if (reliableAck == reliableSequence_) {
        reliableLength_ = 0;
    }
    
    // Update sequence tracking
    incomingSequence_ = static_cast<int>(sequence);
    incomingAcknowledged_ = static_cast<int>(sequenceAck);
    incomingReliableAcknowledged_ = reliableAck;
    
    if (reliableMessage) {
        incomingReliableSequence_ ^= 1;
    }
    
    lastReceived_ = getTime();
    ++stats_.goodCount;
    
    return true;
}

bool NetChannel::canPacket() const {
    return clearTime_ < getTime();
}

bool NetChannel::canReliable() const {
    // Can send reliable if we have pending reliable data and it hasn't been acked
    if (reliableLength_ > 0) {
        return incomingAcknowledged_ > lastReliableSequence_ ||
               incomingReliableAcknowledged_ != reliableSequence_;
    }
    // Or if there's new reliable data to send
    return message_.size() > 0;
}

void NetChannel::setRate(double bytesPerSecond) {
    if (bytesPerSecond < 500.0) {
        bytesPerSecond = 500.0;  // Minimum rate
    }
    if (bytesPerSecond > 1000000.0) {
        bytesPerSecond = 1000000.0;  // Maximum rate
    }
    rate_ = 1.0 / bytesPerSecond;
}

// ============================================================================
// Loopback Implementation
// ============================================================================

void Loopback::send(NetSource from, std::span<const Byte> data) {
    // Send to the opposite endpoint
    auto& messages = (from == NetSource::Client) ? serverMessages_ : clientMessages_;
    auto& sendIdx = (from == NetSource::Client) ? serverSend_ : clientSend_;
    
    size_t idx = sendIdx & (MAX_MESSAGES - 1);
    messages[idx].data.assign(data.begin(), data.end());
    messages[idx].valid = true;
    ++sendIdx;
}

std::optional<std::vector<Byte>> Loopback::receive(NetSource to) {
    auto& messages = (to == NetSource::Client) ? clientMessages_ : serverMessages_;
    auto& getIdx = (to == NetSource::Client) ? clientGet_ : serverGet_;
    auto& sendIdx = (to == NetSource::Client) ? clientSend_ : serverSend_;
    
    // Check for overflow
    if (sendIdx - getIdx > MAX_MESSAGES) {
        getIdx = static_cast<uint32_t>(sendIdx - MAX_MESSAGES);
    }
    
    if (getIdx >= sendIdx) {
        return std::nullopt;
    }
    
    size_t idx = getIdx & (MAX_MESSAGES - 1);
    if (!messages[idx].valid) {
        return std::nullopt;
    }
    
    auto result = std::move(messages[idx].data);
    messages[idx].valid = false;
    ++getIdx;
    
    return result;
}

void Loopback::clear() {
    for (auto& msg : clientMessages_) {
        msg.valid = false;
        msg.data.clear();
    }
    for (auto& msg : serverMessages_) {
        msg.valid = false;
        msg.data.clear();
    }
    clientGet_ = clientSend_ = 0;
    serverGet_ = serverSend_ = 0;
}

bool Loopback::hasPending(NetSource to) const {
    const auto& getIdx = (to == NetSource::Client) ? clientGet_ : serverGet_;
    const auto& sendIdx = (to == NetSource::Client) ? clientSend_ : serverSend_;
    return getIdx < sendIdx;
}

} // namespace ezquake::net
