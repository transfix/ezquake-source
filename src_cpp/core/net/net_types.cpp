// net_types.cpp - Network type implementations
// Part of ezQuake C++20 conversion - Iteration 7

#include "net_types.hpp"
#include <charconv>
#include <cstring>
#include <sstream>

namespace ezquake::net {

// ============================================================================
// NetAddress Implementation
// ============================================================================

NetResult<NetAddress> NetAddress::parse(std::string_view str) {
    if (str.empty()) {
        return unexpected(NetError::InvalidAddress);
    }
    
    // Handle "local" and "loopback" special cases
    if (str == "local" || str == "loopback") {
        return loopback();
    }
    
    // Find port separator
    std::string_view host = str;
    uint16_t port = 0;
    
    auto colonPos = str.rfind(':');
    if (colonPos != std::string_view::npos) {
        host = str.substr(0, colonPos);
        auto portStr = str.substr(colonPos + 1);
        
        // Parse port
        int portValue = 0;
        auto result = std::from_chars(portStr.data(), portStr.data() + portStr.size(), portValue);
        if (result.ec != std::errc{} || portValue < 0 || portValue > 65535) {
            return unexpected(NetError::InvalidAddress);
        }
        port = static_cast<uint16_t>(portValue);
    }
    
    // Parse IPv4 address (simple dot-notation parsing)
    uint8_t octets[4] = {0, 0, 0, 0};
    int octetIndex = 0;
    int currentValue = 0;
    bool hasDigit = false;
    
    for (size_t i = 0; i <= host.size(); ++i) {
        char c = (i < host.size()) ? host[i] : '.';
        
        if (c >= '0' && c <= '9') {
            currentValue = currentValue * 10 + (c - '0');
            if (currentValue > 255) {
                return unexpected(NetError::InvalidAddress);
            }
            hasDigit = true;
        } else if (c == '.') {
            if (!hasDigit || octetIndex >= 4) {
                return unexpected(NetError::InvalidAddress);
            }
            octets[octetIndex++] = static_cast<uint8_t>(currentValue);
            currentValue = 0;
            hasDigit = false;
        } else {
            // Invalid character - might be a hostname, needs DNS resolution
            // For now, we only support IP addresses in the C++ layer
            // DNS resolution is done via the C bridge
            return unexpected(NetError::InvalidAddress);
        }
    }
    
    if (octetIndex != 4) {
        return unexpected(NetError::InvalidAddress);
    }
    
    return ipv4(octets[0], octets[1], octets[2], octets[3], port);
}

std::string NetAddress::toString() const {
    if (type_ == AddressType::Invalid) {
        return "invalid";
    }
    
    if (type_ == AddressType::Loopback) {
        if (port_ > 0) {
            return "loopback:" + std::to_string(port_);
        }
        return "loopback";
    }
    
    std::ostringstream oss;
    oss << static_cast<int>(ip_[0]) << '.'
        << static_cast<int>(ip_[1]) << '.'
        << static_cast<int>(ip_[2]) << '.'
        << static_cast<int>(ip_[3]);
    
    if (port_ > 0) {
        oss << ':' << port_;
    }
    
    return oss.str();
}

std::string NetAddress::toBaseString() const {
    if (type_ == AddressType::Invalid) {
        return "invalid";
    }
    
    if (type_ == AddressType::Loopback) {
        return "loopback";
    }
    
    std::ostringstream oss;
    oss << static_cast<int>(ip_[0]) << '.'
        << static_cast<int>(ip_[1]) << '.'
        << static_cast<int>(ip_[2]) << '.'
        << static_cast<int>(ip_[3]);
    
    return oss.str();
}

} // namespace ezquake::net
