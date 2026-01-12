// socket.cpp - Cross-platform socket implementation
// Part of ezQuake C++20 conversion - Iteration 7

#include "socket.hpp"
#include <cstring>
#include <algorithm>

// Platform-specific includes
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    
    #define SOCKET_ERROR_CODE WSAGetLastError()
    #define WOULD_BLOCK_ERROR WSAEWOULDBLOCK
    #define MSG_SIZE_ERROR WSAEMSGSIZE
    #define CONN_RESET_ERROR WSAECONNRESET
    #define CONN_REFUSED_ERROR WSAECONNREFUSED
    #define CONN_ABORTED_ERROR WSAECONNABORTED
    #define ADDR_NOT_AVAIL_ERROR WSAEADDRNOTAVAIL
    #define AF_NOT_SUPPORTED_ERROR WSAEAFNOSUPPORT
    #define INPROGRESS_ERROR WSAEINPROGRESS
    #define ALREADY_ERROR WSAEALREADY
    #define ISCONN_ERROR WSAEISCONN
    
    namespace { 
        inline int closeSocket(SOCKET s) { return ::closesocket(s); }
    }
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <sys/ioctl.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <errno.h>
    
    #define ioctlsocket ioctl
    #define SOCKET_ERROR_CODE errno
    #define WOULD_BLOCK_ERROR EWOULDBLOCK
    #define MSG_SIZE_ERROR EMSGSIZE
    #define CONN_RESET_ERROR ECONNRESET
    #define CONN_REFUSED_ERROR ECONNREFUSED
    #define CONN_ABORTED_ERROR ECONNABORTED
    #define ADDR_NOT_AVAIL_ERROR EADDRNOTAVAIL
    #define AF_NOT_SUPPORTED_ERROR EAFNOSUPPORT
    #define INPROGRESS_ERROR EINPROGRESS
    #define ALREADY_ERROR EALREADY
    #define ISCONN_ERROR EISCONN
    
    namespace { 
        inline int closeSocket(int s) { return ::close(s); }
    }
#endif

namespace ezquake::net {

// ============================================================================
// Platform Error Mapping
// ============================================================================

int getLastSocketError() {
    return SOCKET_ERROR_CODE;
}

NetError mapSystemError(int errorCode) {
    switch (errorCode) {
        case 0:
            return NetError::None;
        case WOULD_BLOCK_ERROR:
        #if !defined(_WIN32) && EAGAIN != EWOULDBLOCK
        case EAGAIN:
        #endif
            return NetError::WouldBlock;
        case MSG_SIZE_ERROR:
            return NetError::MessageTooLarge;
        case CONN_RESET_ERROR:
            return NetError::ConnectionReset;
        case CONN_REFUSED_ERROR:
            return NetError::ConnectionRefused;
        case CONN_ABORTED_ERROR:
            return NetError::ConnectionAborted;
        case ADDR_NOT_AVAIL_ERROR:
            return NetError::AddressNotAvailable;
        case AF_NOT_SUPPORTED_ERROR:
            return NetError::AddressFamilyNotSupported;
        case INPROGRESS_ERROR:
        case ALREADY_ERROR:
            return NetError::WouldBlock;
        case ISCONN_ERROR:
            return NetError::AlreadyConnected;
        default:
            return NetError::Unknown;
    }
}

// ============================================================================
// Helper Functions
// ============================================================================

namespace {

sockaddr_in toSockaddr(const NetAddress& addr) {
    sockaddr_in sa{};
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = addr.ipAsUint32();
    sa.sin_port = addr.portNetworkOrder();
    return sa;
}

NetAddress fromSockaddr(const sockaddr_in& sa) {
    return NetAddress::fromRaw(sa.sin_addr.s_addr, sa.sin_port);
}

bool applySocketOptions(Socket::Handle handle, bool tcp, const SocketOptions& options) {
    // Non-blocking mode
    if (options.nonBlocking) {
        #ifdef _WIN32
        u_long mode = 1;
        if (ioctlsocket(handle, FIONBIO, &mode) != 0) {
            return false;
        }
        #else
        int flags = fcntl(handle, F_GETFL, 0);
        if (flags < 0 || fcntl(handle, F_SETFL, flags | O_NONBLOCK) < 0) {
            return false;
        }
        #endif
    }
    
    // Reuse address
    if (options.reuseAddress) {
        int optval = 1;
        setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, 
                   reinterpret_cast<const char*>(&optval), sizeof(optval));
    }
    
    // Broadcast (UDP only)
    if (!tcp && options.broadcast) {
        int optval = 1;
        setsockopt(handle, SOL_SOCKET, SO_BROADCAST,
                   reinterpret_cast<const char*>(&optval), sizeof(optval));
    }
    
    // TCP-specific options
    if (tcp) {
        if (options.tcpNoDelay) {
            int optval = 1;
            setsockopt(handle, IPPROTO_TCP, TCP_NODELAY,
                       reinterpret_cast<const char*>(&optval), sizeof(optval));
        }
        
        if (options.tcpKeepAlive) {
            int optval = 1;
            setsockopt(handle, SOL_SOCKET, SO_KEEPALIVE,
                       reinterpret_cast<const char*>(&optval), sizeof(optval));
        }
    }
    
    // Buffer sizes
    if (options.receiveBufferSize > 0) {
        int size = options.receiveBufferSize;
        setsockopt(handle, SOL_SOCKET, SO_RCVBUF,
                   reinterpret_cast<const char*>(&size), sizeof(size));
    }
    
    if (options.sendBufferSize > 0) {
        int size = options.sendBufferSize;
        setsockopt(handle, SOL_SOCKET, SO_SNDBUF,
                   reinterpret_cast<const char*>(&size), sizeof(size));
    }
    
    return true;
}

} // anonymous namespace

// ============================================================================
// Socket Implementation
// ============================================================================

Socket::~Socket() {
    close();
}

Socket::Socket(Socket&& other) noexcept
    : handle_(other.handle_)
    , isTcp_(other.isTcp_)
    , localPort_(other.localPort_)
    , localAddress_(other.localAddress_)
{
    other.handle_ = INVALID_HANDLE;
    other.localPort_ = 0;
    other.localAddress_ = NetAddress();
}

Socket& Socket::operator=(Socket&& other) noexcept {
    if (this != &other) {
        close();
        handle_ = other.handle_;
        isTcp_ = other.isTcp_;
        localPort_ = other.localPort_;
        localAddress_ = other.localAddress_;
        
        other.handle_ = INVALID_HANDLE;
        other.localPort_ = 0;
        other.localAddress_ = NetAddress();
    }
    return *this;
}

NetResult<Socket> Socket::openUdp(uint16_t port, const SocketOptions& options) {
    Handle sock = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_HANDLE) {
        return unexpected(NetError::SocketCreationFailed);
    }
    
    if (!applySocketOptions(sock, false, options)) {
        closeSocket(sock);
        return unexpected(NetError::SocketCreationFailed);
    }
    
    // Bind to port
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    
    if (::bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        closeSocket(sock);
        return unexpected(NetError::BindFailed);
    }
    
    Socket s(sock, false);
    s.updateLocalAddress();
    return s;
}

NetResult<Socket> Socket::openTcpListener(uint16_t port, int backlog, const SocketOptions& options) {
    Handle sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_HANDLE) {
        return unexpected(NetError::SocketCreationFailed);
    }
    
    if (!applySocketOptions(sock, true, options)) {
        closeSocket(sock);
        return unexpected(NetError::SocketCreationFailed);
    }
    
    // Bind to port
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    
    if (::bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        closeSocket(sock);
        return unexpected(NetError::BindFailed);
    }
    
    if (::listen(sock, backlog) < 0) {
        closeSocket(sock);
        return unexpected(NetError::ListenFailed);
    }
    
    Socket s(sock, true);
    s.updateLocalAddress();
    return s;
}

NetResult<Socket> Socket::connectTcp(const NetAddress& remote, const SocketOptions& options) {
    if (!remote.isValid() || remote.type() == AddressType::Loopback) {
        return unexpected(NetError::InvalidAddress);
    }
    
    Handle sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_HANDLE) {
        return unexpected(NetError::SocketCreationFailed);
    }
    
    if (!applySocketOptions(sock, true, options)) {
        closeSocket(sock);
        return unexpected(NetError::SocketCreationFailed);
    }
    
    sockaddr_in addr = toSockaddr(remote);
    
    int result = ::connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    if (result < 0) {
        int err = getLastSocketError();
        // Non-blocking connect returns "would block" or "in progress"
        if (err != INPROGRESS_ERROR && err != WOULD_BLOCK_ERROR) {
            closeSocket(sock);
            return unexpected(mapSystemError(err));
        }
    }
    
    Socket s(sock, true);
    s.updateLocalAddress();
    return s;
}

NetResult<size_t> Socket::sendTo(const NetAddress& to, std::span<const Byte> data) {
    if (!isValid()) {
        return unexpected(NetError::NotConnected);
    }
    
    sockaddr_in addr = toSockaddr(to);
    
    auto sent = ::sendto(handle_, 
                         reinterpret_cast<const char*>(data.data()),
                         static_cast<int>(data.size()),
                         0,
                         reinterpret_cast<sockaddr*>(&addr),
                         sizeof(addr));
    
    if (sent < 0) {
        int err = getLastSocketError();
        if (err == WOULD_BLOCK_ERROR) {
            return unexpected(NetError::WouldBlock);
        }
        return unexpected(mapSystemError(err));
    }
    
    return static_cast<size_t>(sent);
}

NetResult<ReceivedPacket> Socket::receiveFrom(size_t maxSize) {
    if (!isValid()) {
        return unexpected(NetError::NotConnected);
    }
    
    std::vector<Byte> buffer(maxSize);
    sockaddr_in from{};
    socklen_t fromLen = sizeof(from);
    
    auto received = ::recvfrom(handle_,
                               reinterpret_cast<char*>(buffer.data()),
                               static_cast<int>(buffer.size()),
                               0,
                               reinterpret_cast<sockaddr*>(&from),
                               &fromLen);
    
    if (received < 0) {
        int err = getLastSocketError();
        if (err == WOULD_BLOCK_ERROR) {
            return unexpected(NetError::WouldBlock);
        }
        if (err == CONN_RESET_ERROR) {
            // ICMP port unreachable - ignore
            return unexpected(NetError::ConnectionReset);
        }
        return unexpected(mapSystemError(err));
    }
    
    buffer.resize(static_cast<size_t>(received));
    return ReceivedPacket(fromSockaddr(from), std::move(buffer));
}

bool Socket::hasData() const {
    if (!isValid()) {
        return false;
    }
    
    #ifdef _WIN32
    u_long available = 0;
    if (ioctlsocket(handle_, FIONREAD, &available) == 0) {
        return available > 0;
    }
    #else
    int available = 0;
    if (ioctl(handle_, FIONREAD, &available) == 0) {
        return available > 0;
    }
    #endif
    
    return false;
}

NetResult<std::pair<Socket, NetAddress>> Socket::accept() {
    if (!isValid() || !isTcp_) {
        return unexpected(NetError::NotConnected);
    }
    
    sockaddr_in from{};
    socklen_t fromLen = sizeof(from);
    
    Handle newSocket = ::accept(handle_, 
                                reinterpret_cast<sockaddr*>(&from), 
                                &fromLen);
    
    if (newSocket == INVALID_HANDLE) {
        int err = getLastSocketError();
        if (err == WOULD_BLOCK_ERROR) {
            return unexpected(NetError::WouldBlock);
        }
        return unexpected(mapSystemError(err));
    }
    
    Socket s(newSocket, true);
    s.updateLocalAddress();
    return std::make_pair(std::move(s), fromSockaddr(from));
}

NetResult<size_t> Socket::send(std::span<const Byte> data) {
    if (!isValid()) {
        return unexpected(NetError::NotConnected);
    }
    
    auto sent = ::send(handle_,
                       reinterpret_cast<const char*>(data.data()),
                       static_cast<int>(data.size()),
                       0);
    
    if (sent < 0) {
        int err = getLastSocketError();
        if (err == WOULD_BLOCK_ERROR) {
            return unexpected(NetError::WouldBlock);
        }
        return unexpected(mapSystemError(err));
    }
    
    return static_cast<size_t>(sent);
}

NetResult<std::vector<Byte>> Socket::receive(size_t maxSize) {
    if (!isValid()) {
        return unexpected(NetError::NotConnected);
    }
    
    std::vector<Byte> buffer(maxSize);
    
    auto received = ::recv(handle_,
                           reinterpret_cast<char*>(buffer.data()),
                           static_cast<int>(buffer.size()),
                           0);
    
    if (received < 0) {
        int err = getLastSocketError();
        if (err == WOULD_BLOCK_ERROR) {
            return unexpected(NetError::WouldBlock);
        }
        return unexpected(mapSystemError(err));
    }
    
    if (received == 0) {
        return unexpected(NetError::Shutdown);
    }
    
    buffer.resize(static_cast<size_t>(received));
    return buffer;
}

void Socket::close() {
    if (handle_ != INVALID_HANDLE) {
        closeSocket(handle_);
        handle_ = INVALID_HANDLE;
    }
    localPort_ = 0;
    localAddress_ = NetAddress();
}

NetResult<void> Socket::setNonBlocking(bool nonBlocking) {
    if (!isValid()) {
        return unexpected(NetError::NotConnected);
    }
    
    #ifdef _WIN32
    u_long mode = nonBlocking ? 1 : 0;
    if (ioctlsocket(handle_, FIONBIO, &mode) != 0) {
        return unexpected(mapSystemError(getLastSocketError()));
    }
    #else
    int flags = fcntl(handle_, F_GETFL, 0);
    if (flags < 0) {
        return unexpected(mapSystemError(errno));
    }
    
    if (nonBlocking) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }
    
    if (fcntl(handle_, F_SETFL, flags) < 0) {
        return unexpected(mapSystemError(errno));
    }
    #endif
    
    return {};
}

NetResult<void> Socket::setBroadcast(bool enable) {
    if (!isValid() || isTcp_) {
        return unexpected(NetError::NotConnected);
    }
    
    int optval = enable ? 1 : 0;
    if (setsockopt(handle_, SOL_SOCKET, SO_BROADCAST,
                   reinterpret_cast<const char*>(&optval), sizeof(optval)) < 0) {
        return unexpected(mapSystemError(getLastSocketError()));
    }
    
    return {};
}

NetResult<void> Socket::setKeepAlive(bool enable) {
    if (!isValid() || !isTcp_) {
        return unexpected(NetError::NotConnected);
    }
    
    int optval = enable ? 1 : 0;
    if (setsockopt(handle_, SOL_SOCKET, SO_KEEPALIVE,
                   reinterpret_cast<const char*>(&optval), sizeof(optval)) < 0) {
        return unexpected(mapSystemError(getLastSocketError()));
    }
    
    return {};
}

NetResult<void> Socket::setNoDelay(bool enable) {
    if (!isValid() || !isTcp_) {
        return unexpected(NetError::NotConnected);
    }
    
    int optval = enable ? 1 : 0;
    if (setsockopt(handle_, IPPROTO_TCP, TCP_NODELAY,
                   reinterpret_cast<const char*>(&optval), sizeof(optval)) < 0) {
        return unexpected(mapSystemError(getLastSocketError()));
    }
    
    return {};
}

NetResult<NetAddress> Socket::getLocalAddress() const {
    if (!isValid()) {
        return unexpected(NetError::NotConnected);
    }
    
    sockaddr_in addr{};
    socklen_t addrLen = sizeof(addr);
    
    if (getsockname(handle_, reinterpret_cast<sockaddr*>(&addr), &addrLen) < 0) {
        return unexpected(mapSystemError(getLastSocketError()));
    }
    
    return fromSockaddr(addr);
}

void Socket::updateLocalAddress() {
    auto result = getLocalAddress();
    if (result) {
        localAddress_ = *result;
        localPort_ = localAddress_.port();
    }
}

} // namespace ezquake::net
