// net.hpp - Network module convenience header
// Part of ezQuake C++20 conversion - Iteration 7

#pragma once

#include "net_types.hpp"
#include "socket.hpp"
#include "netchan.hpp"

// This header provides access to all network functionality:
//
// ezquake::net::NetAddress     - Network address (IPv4)
// ezquake::net::NetError       - Network error codes
// ezquake::net::NetResult<T>   - Expected<T, NetError>
// ezquake::net::NetSource      - Client or Server
// ezquake::net::Socket         - UDP/TCP socket wrapper
// ezquake::net::NetChannel     - Reliable message channel
// ezquake::net::MessageBuffer  - Read/write buffer for messages
// ezquake::net::Loopback       - Local client-server communication
//
// Example usage:
//
//   using namespace ezquake::net;
//
//   // Parse an address
//   auto addr = NetAddress::parse("192.168.1.1:27500");
//   if (addr) {
//       std::cout << "Address: " << addr->toString() << std::endl;
//   }
//
//   // Open a UDP socket
//   auto socket = Socket::openUdp(27001);
//   if (socket) {
//       // Send data
//       std::vector<Byte> data = {0x01, 0x02, 0x03};
//       socket->sendTo(*addr, data);
//
//       // Receive data
//       auto packet = socket->receiveFrom();
//       if (packet) {
//           std::cout << "Got " << packet->data.size() << " bytes from " 
//                     << packet->from.toString() << std::endl;
//       }
//   }
//
//   // Use NetChannel for reliable messaging
//   NetChannel chan;
//   chan.setup(NetSource::Client, *addr, 12345);
//   
//   // Build a message
//   chan.message().writeByte(0x01);
//   chan.message().writeString("Hello");
//   
//   // Transmit
//   chan.transmit(*socket, {});
