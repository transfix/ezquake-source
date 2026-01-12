// net_types.hpp - Network type definitions
// Part of ezQuake C++20 conversion - Iteration 7

#pragma once

#include "../types.hpp"
#include <string>
#include <string_view>
#include <array>
#include <cstdint>
#include <variant>
#include <optional>

namespace ezquake::net {

// ============================================================================
// Error Handling
// ============================================================================

enum class NetError {
    None = 0,
    InvalidAddress,
    SocketCreationFailed,
    BindFailed,
    ListenFailed,
    ConnectFailed,
    SendFailed,
    ReceiveFailed,
    WouldBlock,
    ConnectionReset,
    ConnectionRefused,
    ConnectionAborted,
    AddressNotAvailable,
    AddressFamilyNotSupported,
    MessageTooLarge,
    Timeout,
    HostNotFound,
    NetworkUnreachable,
    AlreadyConnected,
    NotConnected,
    Shutdown,
    Unknown
};

/// Convert NetError to human-readable string
[[nodiscard]] constexpr std::string_view toString(NetError error) noexcept {
    switch (error) {
        case NetError::None: return "No error";
        case NetError::InvalidAddress: return "Invalid address";
        case NetError::SocketCreationFailed: return "Socket creation failed";
        case NetError::BindFailed: return "Bind failed";
        case NetError::ListenFailed: return "Listen failed";
        case NetError::ConnectFailed: return "Connect failed";
        case NetError::SendFailed: return "Send failed";
        case NetError::ReceiveFailed: return "Receive failed";
        case NetError::WouldBlock: return "Operation would block";
        case NetError::ConnectionReset: return "Connection reset";
        case NetError::ConnectionRefused: return "Connection refused";
        case NetError::ConnectionAborted: return "Connection aborted";
        case NetError::AddressNotAvailable: return "Address not available";
        case NetError::AddressFamilyNotSupported: return "Address family not supported";
        case NetError::MessageTooLarge: return "Message too large";
        case NetError::Timeout: return "Operation timed out";
        case NetError::HostNotFound: return "Host not found";
        case NetError::NetworkUnreachable: return "Network unreachable";
        case NetError::AlreadyConnected: return "Already connected";
        case NetError::NotConnected: return "Not connected";
        case NetError::Shutdown: return "Socket shutdown";
        case NetError::Unknown: return "Unknown error";
    }
    return "Unknown error";
}

// ============================================================================
// Expected<T, E> - Result type (same as fs module for consistency)
// ============================================================================

template<typename E>
class Unexpected {
public:
    constexpr explicit Unexpected(const E& error) : error_(error) {}
    constexpr explicit Unexpected(E&& error) : error_(std::move(error)) {}
    
    [[nodiscard]] constexpr const E& error() const& noexcept { return error_; }
    [[nodiscard]] constexpr E& error() & noexcept { return error_; }
    [[nodiscard]] constexpr E&& error() && noexcept { return std::move(error_); }
    
private:
    E error_;
};

template<typename E>
Unexpected(E) -> Unexpected<E>;

template<typename E>
[[nodiscard]] constexpr Unexpected<std::decay_t<E>> unexpected(E&& error) {
    return Unexpected<std::decay_t<E>>(std::forward<E>(error));
}

template<typename T, typename E>
class Expected {
public:
    // Constructors
    constexpr Expected() requires std::default_initializable<T>
        : data_(T{}) {}
    
    constexpr Expected(const T& value) : data_(value) {}
    constexpr Expected(T&& value) : data_(std::move(value)) {}
    
    constexpr Expected(const Unexpected<E>& error) : data_(error.error()) {}
    constexpr Expected(Unexpected<E>&& error) : data_(std::move(error).error()) {}
    
    // Observers
    [[nodiscard]] constexpr bool has_value() const noexcept {
        return std::holds_alternative<T>(data_);
    }
    
    [[nodiscard]] constexpr explicit operator bool() const noexcept {
        return has_value();
    }
    
    [[nodiscard]] constexpr const T& value() const& {
        if (!has_value()) {
            throw std::runtime_error("Expected has no value");
        }
        return std::get<T>(data_);
    }
    
    [[nodiscard]] constexpr T& value() & {
        if (!has_value()) {
            throw std::runtime_error("Expected has no value");
        }
        return std::get<T>(data_);
    }
    
    [[nodiscard]] constexpr T&& value() && {
        if (!has_value()) {
            throw std::runtime_error("Expected has no value");
        }
        return std::get<T>(std::move(data_));
    }
    
    [[nodiscard]] constexpr const E& error() const& {
        if (has_value()) {
            throw std::runtime_error("Expected has value, not error");
        }
        return std::get<E>(data_);
    }
    
    [[nodiscard]] constexpr E& error() & {
        if (has_value()) {
            throw std::runtime_error("Expected has value, not error");
        }
        return std::get<E>(data_);
    }
    
    // Convenience
    [[nodiscard]] constexpr const T* operator->() const noexcept {
        return &std::get<T>(data_);
    }
    
    [[nodiscard]] constexpr T* operator->() noexcept {
        return &std::get<T>(data_);
    }
    
    [[nodiscard]] constexpr const T& operator*() const& noexcept {
        return std::get<T>(data_);
    }
    
    [[nodiscard]] constexpr T& operator*() & noexcept {
        return std::get<T>(data_);
    }
    
    template<typename U>
    [[nodiscard]] constexpr T value_or(U&& default_value) const& {
        if (has_value()) {
            return value();
        }
        return static_cast<T>(std::forward<U>(default_value));
    }
    
private:
    std::variant<T, E> data_;
};

// Specialization for void
template<typename E>
class Expected<void, E> {
public:
    constexpr Expected() : has_value_(true) {}
    
    constexpr Expected(const Unexpected<E>& error) 
        : has_value_(false), error_(error.error()) {}
    constexpr Expected(Unexpected<E>&& error) 
        : has_value_(false), error_(std::move(error).error()) {}
    
    [[nodiscard]] constexpr bool has_value() const noexcept { return has_value_; }
    [[nodiscard]] constexpr explicit operator bool() const noexcept { return has_value_; }
    
    [[nodiscard]] constexpr const E& error() const& {
        if (has_value_) {
            throw std::runtime_error("Expected has value, not error");
        }
        return error_;
    }
    
private:
    bool has_value_;
    E error_{};
};

// Convenience type aliases
template<typename T>
using NetResult = Expected<T, NetError>;

// ============================================================================
// Network Source (Client vs Server)
// ============================================================================

enum class NetSource : uint8_t {
    Client = 0,
    Server = 1
};

[[nodiscard]] constexpr std::string_view toString(NetSource source) noexcept {
    switch (source) {
        case NetSource::Client: return "client";
        case NetSource::Server: return "server";
    }
    return "unknown";
}

// ============================================================================
// Address Types
// ============================================================================

enum class AddressType : uint8_t {
    Invalid = 0,
    Loopback,
    IPv4,
    IPv6  // Future support
};

/// Network address (IPv4 for now, IPv6 planned)
class NetAddress {
public:
    /// Default constructor creates an invalid address
    constexpr NetAddress() noexcept = default;
    
    /// Create loopback address
    [[nodiscard]] static constexpr NetAddress loopback() noexcept {
        NetAddress addr;
        addr.type_ = AddressType::Loopback;
        return addr;
    }
    
    /// Create IPv4 address from octets and port
    [[nodiscard]] static constexpr NetAddress ipv4(
        uint8_t a, uint8_t b, uint8_t c, uint8_t d, 
        uint16_t port = 0) noexcept 
    {
        NetAddress addr;
        addr.type_ = AddressType::IPv4;
        addr.ip_[0] = a;
        addr.ip_[1] = b;
        addr.ip_[2] = c;
        addr.ip_[3] = d;
        addr.port_ = port;
        return addr;
    }
    
    /// Create address from 32-bit IP and port (network byte order)
    [[nodiscard]] static constexpr NetAddress fromRaw(
        uint32_t ip_network_order, 
        uint16_t port_network_order) noexcept 
    {
        NetAddress addr;
        addr.type_ = AddressType::IPv4;
        addr.ip_[0] = static_cast<uint8_t>((ip_network_order >> 0) & 0xFF);
        addr.ip_[1] = static_cast<uint8_t>((ip_network_order >> 8) & 0xFF);
        addr.ip_[2] = static_cast<uint8_t>((ip_network_order >> 16) & 0xFF);
        addr.ip_[3] = static_cast<uint8_t>((ip_network_order >> 24) & 0xFF);
        // Port is stored in host byte order
        addr.port_ = static_cast<uint16_t>(
            ((port_network_order >> 8) & 0xFF) | 
            ((port_network_order & 0xFF) << 8));
        return addr;
    }
    
    /// Parse address from string (e.g., "192.168.1.1:27500" or "localhost")
    [[nodiscard]] static NetResult<NetAddress> parse(std::string_view str);
    
    // Type checks
    [[nodiscard]] constexpr bool isValid() const noexcept { 
        return type_ != AddressType::Invalid; 
    }
    [[nodiscard]] constexpr bool isLoopback() const noexcept { 
        return type_ == AddressType::Loopback || 
               (type_ == AddressType::IPv4 && ip_[0] == 127); 
    }
    [[nodiscard]] constexpr bool isLocal() const noexcept {
        if (type_ == AddressType::Loopback) return true;
        if (type_ != AddressType::IPv4) return false;
        // 127.x.x.x, 10.x.x.x, 192.168.x.x, 172.16-31.x.x
        return ip_[0] == 127 || ip_[0] == 10 ||
               (ip_[0] == 192 && ip_[1] == 168) ||
               (ip_[0] == 172 && ip_[1] >= 16 && ip_[1] <= 31);
    }
    [[nodiscard]] constexpr bool isBroadcast() const noexcept {
        return type_ == AddressType::IPv4 &&
               ip_[0] == 255 && ip_[1] == 255 && 
               ip_[2] == 255 && ip_[3] == 255;
    }
    
    // Accessors
    [[nodiscard]] constexpr AddressType type() const noexcept { return type_; }
    [[nodiscard]] constexpr uint16_t port() const noexcept { return port_; }
    [[nodiscard]] constexpr std::array<uint8_t, 4> ip() const noexcept { 
        return {ip_[0], ip_[1], ip_[2], ip_[3]}; 
    }
    [[nodiscard]] constexpr uint8_t ip(size_t index) const noexcept {
        return index < 4 ? ip_[index] : 0;
    }
    
    /// Get IP as 32-bit value (network byte order)
    [[nodiscard]] constexpr uint32_t ipAsUint32() const noexcept {
        return static_cast<uint32_t>(ip_[0]) |
               (static_cast<uint32_t>(ip_[1]) << 8) |
               (static_cast<uint32_t>(ip_[2]) << 16) |
               (static_cast<uint32_t>(ip_[3]) << 24);
    }
    
    /// Get port in network byte order
    [[nodiscard]] constexpr uint16_t portNetworkOrder() const noexcept {
        return static_cast<uint16_t>(((port_ >> 8) & 0xFF) | ((port_ & 0xFF) << 8));
    }
    
    // Modifiers
    constexpr void setPort(uint16_t port) noexcept { port_ = port; }
    
    // Comparison
    [[nodiscard]] constexpr bool operator==(const NetAddress& other) const noexcept {
        if (type_ != other.type_) return false;
        if (type_ == AddressType::Loopback) return port_ == other.port_;
        if (type_ == AddressType::Invalid) return true;
        return ip_[0] == other.ip_[0] && ip_[1] == other.ip_[1] &&
               ip_[2] == other.ip_[2] && ip_[3] == other.ip_[3] &&
               port_ == other.port_;
    }
    
    [[nodiscard]] constexpr bool operator!=(const NetAddress& other) const noexcept {
        return !(*this == other);
    }
    
    /// Compare base address (ignoring port)
    [[nodiscard]] constexpr bool sameHost(const NetAddress& other) const noexcept {
        if (type_ != other.type_) return false;
        if (type_ == AddressType::Loopback) return true;
        if (type_ == AddressType::Invalid) return true;
        return ip_[0] == other.ip_[0] && ip_[1] == other.ip_[1] &&
               ip_[2] == other.ip_[2] && ip_[3] == other.ip_[3];
    }
    
    /// Convert to string (e.g., "192.168.1.1:27500" or "loopback")
    [[nodiscard]] std::string toString() const;
    
    /// Convert to string without port
    [[nodiscard]] std::string toBaseString() const;
    
private:
    AddressType type_ = AddressType::Invalid;
    uint8_t ip_[4] = {0, 0, 0, 0};
    uint16_t port_ = 0;
};

// ============================================================================
// Protocol Constants
// ============================================================================

namespace protocol {
    // Port definitions
    constexpr uint16_t PORT_ANY = 0xFFFF;
    constexpr uint16_t PORT_CLIENT_DEFAULT = 27001;
    constexpr uint16_t PORT_SERVER_DEFAULT = 27500;
    
    // Packet sizes
    constexpr size_t MAX_MSGLEN = 8192;
    constexpr size_t MAX_UDP_PACKET = MAX_MSGLEN * 2;
    
    // Sequence masks
    constexpr uint32_t SEQUENCE_RELIABLE_BIT = 0x80000000;
    
    // Loopback buffer size (must be power of 2)
    constexpr size_t MAX_LOOPBACK = 4;
    
    // Latency tracking
    constexpr size_t MAX_LATENT = 32;
    
    // Rate limiting
    constexpr float OLD_AVG = 0.99f;  // EMA smoothing factor
}

} // namespace ezquake::net
