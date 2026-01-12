/**
 * @file message_reader.hpp
 * @brief Network message buffer reader
 * 
 * This file provides a type-safe message buffer reader for parsing QuakeWorld
 * network protocol messages. It handles byte ordering and provides convenient
 * read methods for all protocol data types.
 * 
 * Original C: com_msg.c MSG_Read* functions
 */

#pragma once

#include <cstdint>
#include <cstring>
#include <span>
#include <string>
#include <string_view>
#include <optional>
#include <cmath>
#include "../math/vec3.hpp"

namespace ezquake {

using math::Vec3;

/**
 * @brief Coordinate encoding mode.
 * 
 * QuakeWorld supports both 2-byte (fixed-point) and 4-byte (float) coordinates.
 */
enum class CoordMode {
    Short,      ///< 2-byte fixed-point (1/8 precision)
    Float       ///< 4-byte float
};

/**
 * @brief Angle encoding mode.
 */
enum class AngleMode {
    Byte,       ///< 1-byte (256 steps)
    Short       ///< 2-byte (65536 steps)
};

/**
 * @brief Network message buffer reader.
 * 
 * Provides safe reading of protocol messages with automatic endianness handling
 * and error detection.
 * 
 * Original: net_message global + MSG_Read* functions
 */
class MessageReader {
public:
    //=========================================================================
    // Constructors
    //=========================================================================
    
    /// Default constructor - empty buffer
    constexpr MessageReader() noexcept = default;
    
    /// Construct with data buffer
    explicit MessageReader(std::span<const uint8_t> data) noexcept
        : data_(data), position_(0), badRead_(false) {}
    
    /// Construct with raw pointer and size
    MessageReader(const uint8_t* data, size_t size) noexcept
        : data_(data, size), position_(0), badRead_(false) {}
    
    /// Construct with vector data
    explicit MessageReader(const std::vector<uint8_t>& data) noexcept
        : data_(data), position_(0), badRead_(false) {}
    
    //=========================================================================
    // Buffer Management
    //=========================================================================
    
    /**
     * @brief Reset reader to new data.
     */
    void reset(std::span<const uint8_t> data) noexcept {
        data_ = data;
        position_ = 0;
        badRead_ = false;
    }
    
    /**
     * @brief Reset reader to start of current buffer.
     */
    void rewind() noexcept {
        position_ = 0;
        badRead_ = false;
    }
    
    /**
     * @brief Get current read position.
     */
    [[nodiscard]] size_t position() const noexcept { return position_; }
    
    /**
     * @brief Get total buffer size.
     */
    [[nodiscard]] size_t size() const noexcept { return data_.size(); }
    
    /**
     * @brief Get remaining bytes to read.
     */
    [[nodiscard]] size_t remaining() const noexcept {
        return position_ < data_.size() ? data_.size() - position_ : 0;
    }
    
    /**
     * @brief Check if at end of message.
     */
    [[nodiscard]] bool atEnd() const noexcept {
        return position_ >= data_.size();
    }
    
    /**
     * @brief Check if a read error occurred.
     */
    [[nodiscard]] bool badRead() const noexcept { return badRead_; }
    
    /**
     * @brief Clear bad read flag.
     */
    void clearError() noexcept { badRead_ = false; }
    
    /**
     * @brief Seek to position.
     */
    void seek(size_t pos) noexcept {
        if (pos <= data_.size()) {
            position_ = pos;
        } else {
            badRead_ = true;
        }
    }
    
    /**
     * @brief Skip bytes.
     */
    void skip(size_t count) noexcept {
        if (position_ + count <= data_.size()) {
            position_ += count;
        } else {
            badRead_ = true;
        }
    }
    
    /**
     * @brief Get view of remaining data.
     */
    [[nodiscard]] std::span<const uint8_t> remainingData() const noexcept {
        if (position_ >= data_.size()) {
            return {};
        }
        return data_.subspan(position_);
    }
    
    //=========================================================================
    // Primitive Read Operations
    //=========================================================================
    
    /**
     * @brief Read a signed 8-bit integer (-128 to 127).
     */
    [[nodiscard]] int8_t readChar() noexcept {
        if (position_ + 1 > data_.size()) {
            badRead_ = true;
            return -1;
        }
        return static_cast<int8_t>(data_[position_++]);
    }
    
    /**
     * @brief Read an unsigned 8-bit integer (0 to 255).
     * 
     * Returns -1 on error to match original behavior.
     */
    [[nodiscard]] int readByte() noexcept {
        if (position_ + 1 > data_.size()) {
            badRead_ = true;
            return -1;
        }
        return static_cast<int>(data_[position_++]);
    }
    
    /**
     * @brief Read an unsigned 8-bit value.
     */
    [[nodiscard]] uint8_t readUByte() noexcept {
        if (position_ + 1 > data_.size()) {
            badRead_ = true;
            return 0;
        }
        return data_[position_++];
    }
    
    /**
     * @brief Read a signed 16-bit integer (little-endian).
     */
    [[nodiscard]] int16_t readShort() noexcept {
        if (position_ + 2 > data_.size()) {
            badRead_ = true;
            return -1;
        }
        int16_t value = static_cast<int16_t>(data_[position_] | 
                                              (data_[position_ + 1] << 8));
        position_ += 2;
        return value;
    }
    
    /**
     * @brief Read an unsigned 16-bit integer (little-endian).
     */
    [[nodiscard]] uint16_t readUShort() noexcept {
        if (position_ + 2 > data_.size()) {
            badRead_ = true;
            return 0;
        }
        uint16_t value = static_cast<uint16_t>(data_[position_] |
                                                (data_[position_ + 1] << 8));
        position_ += 2;
        return value;
    }
    
    /**
     * @brief Read a signed 32-bit integer (little-endian).
     */
    [[nodiscard]] int32_t readLong() noexcept {
        if (position_ + 4 > data_.size()) {
            badRead_ = true;
            return -1;
        }
        int32_t value = static_cast<int32_t>(
            data_[position_] |
            (data_[position_ + 1] << 8) |
            (data_[position_ + 2] << 16) |
            (data_[position_ + 3] << 24)
        );
        position_ += 4;
        return value;
    }
    
    /**
     * @brief Read an unsigned 32-bit integer (little-endian).
     */
    [[nodiscard]] uint32_t readULong() noexcept {
        if (position_ + 4 > data_.size()) {
            badRead_ = true;
            return 0;
        }
        uint32_t value = 
            data_[position_] |
            (static_cast<uint32_t>(data_[position_ + 1]) << 8) |
            (static_cast<uint32_t>(data_[position_ + 2]) << 16) |
            (static_cast<uint32_t>(data_[position_ + 3]) << 24);
        position_ += 4;
        return value;
    }
    
    /**
     * @brief Read a 32-bit float (little-endian).
     */
    [[nodiscard]] float readFloat() noexcept {
        if (position_ + 4 > data_.size()) {
            badRead_ = true;
            return -1.0f;
        }
        
        // Read as little-endian uint32 then reinterpret
        uint32_t bits = 
            data_[position_] |
            (static_cast<uint32_t>(data_[position_ + 1]) << 8) |
            (static_cast<uint32_t>(data_[position_ + 2]) << 16) |
            (static_cast<uint32_t>(data_[position_ + 3]) << 24);
        position_ += 4;
        
        float result;
        std::memcpy(&result, &bits, sizeof(float));
        return result;
    }
    
    //=========================================================================
    // String Read Operations
    //=========================================================================
    
    /**
     * @brief Read a null-terminated string.
     * 
     * @return String or empty string on error
     */
    [[nodiscard]] std::string readString() noexcept {
        std::string result;
        
        while (position_ < data_.size()) {
            char c = static_cast<char>(data_[position_++]);
            if (c == '\0') {
                return result;
            }
            result += c;
        }
        
        // Reached end without null terminator
        badRead_ = true;
        return result;
    }
    
    /**
     * @brief Read a string with maximum length.
     */
    [[nodiscard]] std::string readString(size_t maxLen) noexcept {
        std::string result;
        result.reserve(maxLen);
        
        size_t count = 0;
        while (position_ < data_.size() && count < maxLen) {
            char c = static_cast<char>(data_[position_++]);
            if (c == '\0') {
                return result;
            }
            result += c;
            ++count;
        }
        
        // Skip remaining until null
        while (position_ < data_.size() && data_[position_] != 0) {
            ++position_;
        }
        if (position_ < data_.size()) {
            ++position_;  // Skip null
        }
        
        return result;
    }
    
    /**
     * @brief Read a string until newline (for stufftext parsing).
     */
    [[nodiscard]] std::string readLine() noexcept {
        std::string result;
        
        while (position_ < data_.size()) {
            char c = static_cast<char>(data_[position_++]);
            if (c == '\n' || c == '\0') {
                return result;
            }
            result += c;
        }
        
        return result;
    }
    
    //=========================================================================
    // Coordinate/Angle Read Operations
    //=========================================================================
    
    /**
     * @brief Read a coordinate value.
     * 
     * @param mode Coordinate encoding mode
     */
    [[nodiscard]] float readCoord(CoordMode mode = CoordMode::Short) noexcept {
        if (mode == CoordMode::Float) {
            return readFloat();
        }
        // Short mode: 1/8 precision fixed point
        return static_cast<float>(readShort()) / 8.0f;
    }
    
    /**
     * @brief Read an angle value.
     * 
     * @param mode Angle encoding mode
     */
    [[nodiscard]] float readAngle(AngleMode mode = AngleMode::Byte) noexcept {
        if (mode == AngleMode::Short) {
            return static_cast<float>(readUShort()) * (360.0f / 65536.0f);
        }
        // Byte mode
        return static_cast<float>(readUByte()) * (360.0f / 256.0f);
    }
    
    /**
     * @brief Read a 16-bit high-precision angle.
     */
    [[nodiscard]] float readAngle16() noexcept {
        return static_cast<float>(readShort()) * (360.0f / 65536.0f);
    }
    
    /**
     * @brief Read a 3D coordinate vector.
     */
    [[nodiscard]] Vec3 readCoordVec(CoordMode mode = CoordMode::Short) noexcept {
        float x = readCoord(mode);
        float y = readCoord(mode);
        float z = readCoord(mode);
        return Vec3(x, y, z);
    }
    
    /**
     * @brief Read a 3D angle vector.
     */
    [[nodiscard]] Vec3 readAngleVec(AngleMode mode = AngleMode::Byte) noexcept {
        float pitch = readAngle(mode);
        float yaw = readAngle(mode);
        float roll = readAngle(mode);
        return Vec3(pitch, yaw, roll);
    }
    
    /**
     * @brief Read a 3D angle vector with 16-bit angles.
     */
    [[nodiscard]] Vec3 readAngle16Vec() noexcept {
        float pitch = readAngle16();
        float yaw = readAngle16();
        float roll = readAngle16();
        return Vec3(pitch, yaw, roll);
    }
    
    //=========================================================================
    // Raw Data Read Operations
    //=========================================================================
    
    /**
     * @brief Read raw bytes into buffer.
     * 
     * @return Number of bytes actually read
     */
    size_t readBytes(void* dest, size_t count) noexcept {
        size_t available = remaining();
        size_t toRead = std::min(count, available);
        
        if (toRead < count) {
            badRead_ = true;
        }
        
        if (toRead > 0) {
            std::memcpy(dest, &data_[position_], toRead);
            position_ += toRead;
        }
        
        return toRead;
    }
    
    /**
     * @brief Read raw bytes as span.
     */
    [[nodiscard]] std::span<const uint8_t> readBytesView(size_t count) noexcept {
        if (position_ + count > data_.size()) {
            badRead_ = true;
            return {};
        }
        
        auto result = data_.subspan(position_, count);
        position_ += count;
        return result;
    }
    
    //=========================================================================
    // Delta-Encoded Reads
    //=========================================================================
    
    /**
     * @brief Read entity update word and extract entity number and flags.
     */
    struct EntityUpdateWord {
        uint16_t entityNum;
        uint16_t flags;
    };
    
    [[nodiscard]] EntityUpdateWord readEntityWord() noexcept {
        uint16_t word = readUShort();
        return {
            .entityNum = static_cast<uint16_t>(word & 0x01FF),
            .flags = static_cast<uint16_t>(word & 0xFE00)
        };
    }
    
    /**
     * @brief Peek at next byte without advancing.
     */
    [[nodiscard]] int peekByte() const noexcept {
        if (position_ >= data_.size()) {
            return -1;
        }
        return static_cast<int>(data_[position_]);
    }
    
    /**
     * @brief Get raw data pointer at current position.
     */
    [[nodiscard]] const uint8_t* currentPtr() const noexcept {
        if (position_ >= data_.size()) {
            return nullptr;
        }
        return &data_[position_];
    }

private:
    std::span<const uint8_t> data_;
    size_t position_ = 0;
    bool badRead_ = false;
};

/**
 * @brief Network message buffer writer.
 * 
 * Provides type-safe writing of protocol messages with automatic endianness.
 * 
 * Original: sizebuf_t + MSG_Write* functions
 */
class MessageWriter {
public:
    //=========================================================================
    // Constructors
    //=========================================================================
    
    /// Default constructor - uses internal buffer
    MessageWriter() = default;
    
    /// Construct with pre-allocated capacity
    explicit MessageWriter(size_t capacity) {
        buffer_.reserve(capacity);
    }
    
    //=========================================================================
    // Buffer Management
    //=========================================================================
    
    /**
     * @brief Clear the buffer.
     */
    void clear() noexcept {
        buffer_.clear();
        overflow_ = false;
    }
    
    /**
     * @brief Get current buffer size.
     */
    [[nodiscard]] size_t size() const noexcept { return buffer_.size(); }
    
    /**
     * @brief Get buffer data.
     */
    [[nodiscard]] std::span<const uint8_t> data() const noexcept { return buffer_; }
    
    /**
     * @brief Get mutable buffer data.
     */
    [[nodiscard]] std::span<uint8_t> mutableData() noexcept { return buffer_; }
    
    /**
     * @brief Check if overflow occurred.
     */
    [[nodiscard]] bool overflow() const noexcept { return overflow_; }
    
    /**
     * @brief Set maximum size limit.
     */
    void setMaxSize(size_t max) noexcept { maxSize_ = max; }
    
    //=========================================================================
    // Primitive Write Operations
    //=========================================================================
    
    /**
     * @brief Write a signed 8-bit integer.
     */
    void writeChar(int8_t value) {
        writeByte(static_cast<uint8_t>(value));
    }
    
    /**
     * @brief Write an unsigned 8-bit integer.
     */
    void writeByte(uint8_t value) {
        if (maxSize_ > 0 && buffer_.size() >= maxSize_) {
            overflow_ = true;
            return;
        }
        buffer_.push_back(value);
    }
    
    /**
     * @brief Write a signed 16-bit integer (little-endian).
     */
    void writeShort(int16_t value) {
        writeByte(static_cast<uint8_t>(value & 0xFF));
        writeByte(static_cast<uint8_t>((value >> 8) & 0xFF));
    }
    
    /**
     * @brief Write an unsigned 16-bit integer (little-endian).
     */
    void writeUShort(uint16_t value) {
        writeByte(static_cast<uint8_t>(value & 0xFF));
        writeByte(static_cast<uint8_t>((value >> 8) & 0xFF));
    }
    
    /**
     * @brief Write a signed 32-bit integer (little-endian).
     */
    void writeLong(int32_t value) {
        writeByte(static_cast<uint8_t>(value & 0xFF));
        writeByte(static_cast<uint8_t>((value >> 8) & 0xFF));
        writeByte(static_cast<uint8_t>((value >> 16) & 0xFF));
        writeByte(static_cast<uint8_t>((value >> 24) & 0xFF));
    }
    
    /**
     * @brief Write a 32-bit float (little-endian).
     */
    void writeFloat(float value) {
        uint32_t bits;
        std::memcpy(&bits, &value, sizeof(float));
        writeByte(static_cast<uint8_t>(bits & 0xFF));
        writeByte(static_cast<uint8_t>((bits >> 8) & 0xFF));
        writeByte(static_cast<uint8_t>((bits >> 16) & 0xFF));
        writeByte(static_cast<uint8_t>((bits >> 24) & 0xFF));
    }
    
    /**
     * @brief Write a null-terminated string.
     */
    void writeString(std::string_view str) {
        for (char c : str) {
            writeByte(static_cast<uint8_t>(c));
        }
        writeByte(0);
    }
    
    /**
     * @brief Write a coordinate value.
     */
    void writeCoord(float value, CoordMode mode = CoordMode::Short) {
        if (mode == CoordMode::Float) {
            writeFloat(value);
        } else {
            // Short mode: 1/8 precision
            int16_t encoded;
            if (value >= 0) {
                encoded = static_cast<int16_t>(value * 8.0f + 0.5f);
            } else {
                encoded = static_cast<int16_t>(value * 8.0f - 0.5f);
            }
            writeShort(encoded);
        }
    }
    
    /**
     * @brief Write an angle value.
     */
    void writeAngle(float value, AngleMode mode = AngleMode::Byte) {
        if (mode == AngleMode::Short) {
            int16_t encoded;
            if (value >= 0) {
                encoded = static_cast<int16_t>(value * (65536.0f / 360.0f) + 0.5f) & 0xFFFF;
            } else {
                encoded = static_cast<int16_t>(value * (65536.0f / 360.0f) - 0.5f) & 0xFFFF;
            }
            writeShort(encoded);
        } else {
            uint8_t encoded;
            if (value >= 0) {
                encoded = static_cast<uint8_t>(value * (256.0f / 360.0f) + 0.5f) & 0xFF;
            } else {
                encoded = static_cast<uint8_t>(value * (256.0f / 360.0f) - 0.5f) & 0xFF;
            }
            writeByte(encoded);
        }
    }
    
    /**
     * @brief Write a 16-bit high-precision angle.
     */
    void writeAngle16(float value) {
        writeAngle(value, AngleMode::Short);
    }
    
    /**
     * @brief Write a 3D coordinate vector.
     */
    void writeCoordVec(const Vec3& v, CoordMode mode = CoordMode::Short) {
        writeCoord(v.x(), mode);
        writeCoord(v.y(), mode);
        writeCoord(v.z(), mode);
    }
    
    /**
     * @brief Write raw bytes.
     */
    void writeBytes(std::span<const uint8_t> bytes) {
        for (uint8_t b : bytes) {
            writeByte(b);
        }
    }

private:
    std::vector<uint8_t> buffer_;
    size_t maxSize_ = 0;
    bool overflow_ = false;
};

} // namespace ezquake
