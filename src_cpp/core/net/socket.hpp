// socket.hpp - Cross-platform socket abstraction
// Part of ezQuake C++20 conversion - Iteration 7

#pragma once

#include "net_types.hpp"
#include <span>
#include <vector>
#include <memory>
#include <mutex>
#include <optional>

namespace ezquake::net {

// Forward declarations
class Socket;

// ============================================================================
// Socket Options
// ============================================================================

struct SocketOptions {
    bool nonBlocking = true;
    bool broadcast = false;
    bool reuseAddress = true;
    bool tcpNoDelay = false;       // For TCP sockets
    bool tcpKeepAlive = false;     // For TCP sockets
    int receiveBufferSize = 0;     // 0 = use system default
    int sendBufferSize = 0;        // 0 = use system default
};

// ============================================================================
// Received Packet
// ============================================================================

struct ReceivedPacket {
    NetAddress from;
    std::vector<Byte> data;
    
    ReceivedPacket() = default;
    ReceivedPacket(NetAddress addr, std::vector<Byte> bytes)
        : from(std::move(addr)), data(std::move(bytes)) {}
};

// ============================================================================
// Socket - RAII UDP/TCP socket wrapper
// ============================================================================

class Socket {
public:
    // Use system-specific socket handle type
    #ifdef _WIN32
    using Handle = uint64_t;  // SOCKET is 64-bit on Windows x64
    #else
    using Handle = int;
    #endif
    
    static constexpr Handle INVALID_HANDLE = static_cast<Handle>(-1);
    
    // Constructors
    Socket() = default;
    ~Socket();
    
    // Move-only
    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    
    // Factory methods for creating sockets
    
    /// Open a UDP socket, optionally bound to a port
    [[nodiscard]] static NetResult<Socket> openUdp(
        uint16_t port = 0, 
        const SocketOptions& options = {});
    
    /// Open a TCP socket for listening
    [[nodiscard]] static NetResult<Socket> openTcpListener(
        uint16_t port, 
        int backlog = 5,
        const SocketOptions& options = {});
    
    /// Connect to a remote TCP server
    [[nodiscard]] static NetResult<Socket> connectTcp(
        const NetAddress& remote,
        const SocketOptions& options = {});
    
    // Status
    [[nodiscard]] bool isValid() const noexcept { return handle_ != INVALID_HANDLE; }
    [[nodiscard]] explicit operator bool() const noexcept { return isValid(); }
    [[nodiscard]] Handle handle() const noexcept { return handle_; }
    [[nodiscard]] bool isTcp() const noexcept { return isTcp_; }
    [[nodiscard]] uint16_t localPort() const noexcept { return localPort_; }
    [[nodiscard]] NetAddress localAddress() const noexcept { return localAddress_; }
    
    // UDP Operations
    
    /// Send data to a specific address (UDP)
    [[nodiscard]] NetResult<size_t> sendTo(
        const NetAddress& to, 
        std::span<const Byte> data);
    
    /// Receive data (UDP), returns packet with source address
    [[nodiscard]] NetResult<ReceivedPacket> receiveFrom(size_t maxSize = protocol::MAX_UDP_PACKET);
    
    /// Check if data is available to read
    [[nodiscard]] bool hasData() const;
    
    // TCP Operations
    
    /// Accept an incoming connection (TCP listener)
    [[nodiscard]] NetResult<std::pair<Socket, NetAddress>> accept();
    
    /// Send data on connected socket (TCP)
    [[nodiscard]] NetResult<size_t> send(std::span<const Byte> data);
    
    /// Receive data on connected socket (TCP)
    [[nodiscard]] NetResult<std::vector<Byte>> receive(size_t maxSize = protocol::MAX_MSGLEN);
    
    // Common Operations
    
    /// Close the socket
    void close();
    
    /// Set socket to non-blocking mode
    NetResult<void> setNonBlocking(bool nonBlocking);
    
    /// Enable/disable broadcast (UDP only)
    NetResult<void> setBroadcast(bool enable);
    
    /// Set TCP keep-alive (TCP only)
    NetResult<void> setKeepAlive(bool enable);
    
    /// Set TCP no-delay (TCP only)
    NetResult<void> setNoDelay(bool enable);
    
    /// Get the local address this socket is bound to
    [[nodiscard]] NetResult<NetAddress> getLocalAddress() const;
    
private:
    explicit Socket(Handle h, bool tcp = false) 
        : handle_(h), isTcp_(tcp) {}
    
    void updateLocalAddress();
    
    Handle handle_ = INVALID_HANDLE;
    bool isTcp_ = false;
    uint16_t localPort_ = 0;
    NetAddress localAddress_;
};

// ============================================================================
// Platform-specific error mapping
// ============================================================================

/// Convert platform errno/WSAGetLastError to NetError
[[nodiscard]] NetError mapSystemError(int errorCode);

/// Get the last socket error for the current platform
[[nodiscard]] int getLastSocketError();

} // namespace ezquake::net
