// net_test.cpp - Network module unit tests
// Part of ezQuake C++20 conversion - Iteration 7

#include <gtest/gtest.h>
#include "../core/net/net.hpp"
#include <thread>
#include <chrono>
#include <atomic>

using namespace ezquake;
using namespace ezquake::net;

// ============================================================================
// NetError Tests
// ============================================================================

TEST(NetErrorTest, ErrorToString) {
    EXPECT_EQ(toString(NetError::None), "No error");
    EXPECT_EQ(toString(NetError::InvalidAddress), "Invalid address");
    EXPECT_EQ(toString(NetError::WouldBlock), "Operation would block");
    EXPECT_EQ(toString(NetError::ConnectionRefused), "Connection refused");
}

// ============================================================================
// NetAddress Tests
// ============================================================================

TEST(NetAddressTest, DefaultConstruction) {
    NetAddress addr;
    EXPECT_FALSE(addr.isValid());
    EXPECT_EQ(addr.type(), AddressType::Invalid);
}

TEST(NetAddressTest, Loopback) {
    auto addr = NetAddress::loopback();
    EXPECT_TRUE(addr.isValid());
    EXPECT_TRUE(addr.isLoopback());
    EXPECT_TRUE(addr.isLocal());
    EXPECT_EQ(addr.type(), AddressType::Loopback);
    EXPECT_EQ(addr.toString(), "loopback");
}

TEST(NetAddressTest, IPv4Construction) {
    auto addr = NetAddress::ipv4(192, 168, 1, 1, 27500);
    EXPECT_TRUE(addr.isValid());
    EXPECT_FALSE(addr.isLoopback());
    EXPECT_TRUE(addr.isLocal());  // 192.168.x.x is local
    EXPECT_EQ(addr.type(), AddressType::IPv4);
    EXPECT_EQ(addr.port(), 27500);
    EXPECT_EQ(addr.ip(0), 192);
    EXPECT_EQ(addr.ip(1), 168);
    EXPECT_EQ(addr.ip(2), 1);
    EXPECT_EQ(addr.ip(3), 1);
}

TEST(NetAddressTest, IPv4Parse) {
    auto result = NetAddress::parse("192.168.1.1:27500");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->port(), 27500);
    EXPECT_EQ(result->ip(0), 192);
    EXPECT_EQ(result->ip(1), 168);
}

TEST(NetAddressTest, IPv4ParseNoPort) {
    auto result = NetAddress::parse("10.0.0.1");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->port(), 0);
    EXPECT_EQ(result->ip(0), 10);
    EXPECT_TRUE(result->isLocal());  // 10.x.x.x is local
}

TEST(NetAddressTest, LoopbackParse) {
    auto result = NetAddress::parse("local");
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->isLoopback());
    
    auto result2 = NetAddress::parse("loopback");
    ASSERT_TRUE(result2.has_value());
    EXPECT_TRUE(result2->isLoopback());
}

TEST(NetAddressTest, InvalidParse) {
    EXPECT_FALSE(NetAddress::parse("").has_value());
    EXPECT_FALSE(NetAddress::parse("256.1.1.1").has_value());
    EXPECT_FALSE(NetAddress::parse("1.1.1").has_value());
    EXPECT_FALSE(NetAddress::parse("abc").has_value());
}

TEST(NetAddressTest, ToString) {
    auto addr = NetAddress::ipv4(8, 8, 8, 8, 53);
    EXPECT_EQ(addr.toString(), "8.8.8.8:53");
    EXPECT_EQ(addr.toBaseString(), "8.8.8.8");
}

TEST(NetAddressTest, Comparison) {
    auto a = NetAddress::ipv4(192, 168, 1, 1, 27500);
    auto b = NetAddress::ipv4(192, 168, 1, 1, 27500);
    auto c = NetAddress::ipv4(192, 168, 1, 1, 27501);
    auto d = NetAddress::ipv4(192, 168, 1, 2, 27500);
    
    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);  // Different port
    EXPECT_NE(a, d);  // Different IP
    
    EXPECT_TRUE(a.sameHost(b));
    EXPECT_TRUE(a.sameHost(c));  // Same host, different port
    EXPECT_FALSE(a.sameHost(d));
}

TEST(NetAddressTest, LocalAddresses) {
    EXPECT_TRUE(NetAddress::ipv4(127, 0, 0, 1).isLoopback());
    EXPECT_TRUE(NetAddress::ipv4(127, 1, 2, 3).isLoopback());
    EXPECT_TRUE(NetAddress::ipv4(10, 0, 0, 1).isLocal());
    EXPECT_TRUE(NetAddress::ipv4(172, 16, 0, 1).isLocal());
    EXPECT_TRUE(NetAddress::ipv4(172, 31, 255, 255).isLocal());
    EXPECT_FALSE(NetAddress::ipv4(172, 32, 0, 1).isLocal());
    EXPECT_TRUE(NetAddress::ipv4(192, 168, 0, 1).isLocal());
    EXPECT_FALSE(NetAddress::ipv4(8, 8, 8, 8).isLocal());
}

TEST(NetAddressTest, Broadcast) {
    EXPECT_TRUE(NetAddress::ipv4(255, 255, 255, 255).isBroadcast());
    EXPECT_FALSE(NetAddress::ipv4(255, 255, 255, 0).isBroadcast());
    EXPECT_FALSE(NetAddress::ipv4(192, 168, 1, 255).isBroadcast());
}

TEST(NetAddressTest, NetworkByteOrder) {
    auto addr = NetAddress::ipv4(192, 168, 1, 1, 27500);
    
    // IP should be 192.168.1.1 = 0xC0A80101 in network order (big endian)
    // But we store as little endian on x86: 0x0101A8C0
    uint32_t ip = addr.ipAsUint32();
    EXPECT_EQ((ip >> 0) & 0xFF, 192);
    EXPECT_EQ((ip >> 8) & 0xFF, 168);
    EXPECT_EQ((ip >> 16) & 0xFF, 1);
    EXPECT_EQ((ip >> 24) & 0xFF, 1);
}

// ============================================================================
// MessageBuffer Tests
// ============================================================================

TEST(MessageBufferTest, DefaultConstruction) {
    MessageBuffer buf;
    EXPECT_EQ(buf.size(), 0);
    EXPECT_EQ(buf.capacity(), MessageBuffer::DEFAULT_SIZE);
    EXPECT_FALSE(buf.hasOverflowed());
}

TEST(MessageBufferTest, WriteByte) {
    MessageBuffer buf;
    buf.writeByte(0x42);
    EXPECT_EQ(buf.size(), 1);
    EXPECT_EQ(buf.data()[0], 0x42);
}

TEST(MessageBufferTest, ReadByte) {
    MessageBuffer buf;
    buf.writeByte(0x42);
    buf.beginReading();
    auto result = buf.readByte();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 0x42);
}

TEST(MessageBufferTest, WriteShort) {
    MessageBuffer buf;
    buf.writeShort(0x1234);
    EXPECT_EQ(buf.size(), 2);
    buf.beginReading();
    auto result = buf.readShort();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 0x1234);
}

TEST(MessageBufferTest, WriteLong) {
    MessageBuffer buf;
    buf.writeLong(0x12345678);
    EXPECT_EQ(buf.size(), 4);
    buf.beginReading();
    auto result = buf.readLong();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 0x12345678);
}

TEST(MessageBufferTest, WriteFloat) {
    MessageBuffer buf;
    buf.writeFloat(3.14159f);
    EXPECT_EQ(buf.size(), 4);
    buf.beginReading();
    auto result = buf.readFloat();
    ASSERT_TRUE(result.has_value());
    EXPECT_FLOAT_EQ(*result, 3.14159f);
}

TEST(MessageBufferTest, WriteString) {
    MessageBuffer buf;
    buf.writeString("Hello");
    EXPECT_EQ(buf.size(), 6);  // 5 chars + null
    buf.beginReading();
    auto result = buf.readString();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "Hello");
}

TEST(MessageBufferTest, WriteData) {
    MessageBuffer buf;
    std::vector<Byte> data = {0x01, 0x02, 0x03, 0x04};
    buf.writeData(data);
    EXPECT_EQ(buf.size(), 4);
    buf.beginReading();
    auto result = buf.readData(4);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, data);
}

TEST(MessageBufferTest, WriteCoord) {
    MessageBuffer buf;
    buf.writeCoord(128.5f);
    buf.beginReading();
    auto result = buf.readCoord();
    ASSERT_TRUE(result.has_value());
    EXPECT_FLOAT_EQ(*result, 128.5f);
}

TEST(MessageBufferTest, WriteAngle) {
    MessageBuffer buf;
    buf.writeAngle(90.0f);
    buf.beginReading();
    auto result = buf.readAngle();
    ASSERT_TRUE(result.has_value());
    // Angle encoding has limited precision
    EXPECT_NEAR(*result, 90.0f, 2.0f);
}

TEST(MessageBufferTest, MixedReadWrite) {
    MessageBuffer buf;
    buf.writeByte(0x01);
    buf.writeShort(0x0203);
    buf.writeLong(0x04050607);
    buf.writeString("Test");
    buf.writeFloat(1.5f);
    
    buf.beginReading();
    
    EXPECT_EQ(*buf.readByte(), 0x01);
    EXPECT_EQ(*buf.readShort(), 0x0203);
    EXPECT_EQ(*buf.readLong(), 0x04050607);
    EXPECT_EQ(*buf.readString(), "Test");
    EXPECT_FLOAT_EQ(*buf.readFloat(), 1.5f);
}

TEST(MessageBufferTest, ReadPastEnd) {
    MessageBuffer buf;
    buf.writeByte(0x01);
    buf.beginReading();
    
    EXPECT_TRUE(buf.readByte().has_value());
    EXPECT_FALSE(buf.readByte().has_value());
    EXPECT_TRUE(buf.hasBadRead());
}

TEST(MessageBufferTest, Clear) {
    MessageBuffer buf;
    buf.writeByte(0x01);
    buf.writeByte(0x02);
    EXPECT_EQ(buf.size(), 2);
    
    buf.clear();
    EXPECT_EQ(buf.size(), 0);
    EXPECT_FALSE(buf.hasOverflowed());
}

TEST(MessageBufferTest, Eof) {
    MessageBuffer buf;
    buf.writeByte(0x01);
    buf.beginReading();
    
    EXPECT_FALSE(buf.eof());
    buf.readByte();
    EXPECT_TRUE(buf.eof());
}

// ============================================================================
// Loopback Tests
// ============================================================================

TEST(LoopbackTest, SendReceive) {
    Loopback loop;
    
    std::vector<Byte> data = {0x01, 0x02, 0x03};
    loop.send(NetSource::Client, data);
    
    EXPECT_TRUE(loop.hasPending(NetSource::Server));
    EXPECT_FALSE(loop.hasPending(NetSource::Client));
    
    auto received = loop.receive(NetSource::Server);
    ASSERT_TRUE(received.has_value());
    EXPECT_EQ(*received, data);
    
    EXPECT_FALSE(loop.hasPending(NetSource::Server));
}

TEST(LoopbackTest, MultipleMessages) {
    Loopback loop;
    
    for (int i = 0; i < 4; ++i) {
        std::vector<Byte> data = {static_cast<Byte>(i)};
        loop.send(NetSource::Client, data);
    }
    
    for (int i = 0; i < 4; ++i) {
        auto received = loop.receive(NetSource::Server);
        ASSERT_TRUE(received.has_value());
        EXPECT_EQ(received->size(), 1);
        EXPECT_EQ((*received)[0], static_cast<Byte>(i));
    }
    
    EXPECT_FALSE(loop.receive(NetSource::Server).has_value());
}

TEST(LoopbackTest, Clear) {
    Loopback loop;
    
    std::vector<Byte> data = {0x01};
    loop.send(NetSource::Client, data);
    
    loop.clear();
    
    EXPECT_FALSE(loop.hasPending(NetSource::Server));
    EXPECT_FALSE(loop.receive(NetSource::Server).has_value());
}

TEST(LoopbackTest, BothDirections) {
    Loopback loop;
    
    std::vector<Byte> clientData = {0xC1};
    std::vector<Byte> serverData = {0x5E};
    
    loop.send(NetSource::Client, clientData);
    loop.send(NetSource::Server, serverData);
    
    auto toServer = loop.receive(NetSource::Server);
    auto toClient = loop.receive(NetSource::Client);
    
    ASSERT_TRUE(toServer.has_value());
    ASSERT_TRUE(toClient.has_value());
    EXPECT_EQ(*toServer, clientData);
    EXPECT_EQ(*toClient, serverData);
}

// ============================================================================
// NetChannel Tests
// ============================================================================

class NetChannelTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up a fake time function
        currentTime_ = 0.0;
        setTimeFunction([this]() { return currentTime_; });
    }
    
    void advanceTime(double seconds) {
        currentTime_ += seconds;
    }
    
    double currentTime_ = 0.0;
};

TEST_F(NetChannelTest, Setup) {
    NetChannel chan;
    auto addr = NetAddress::ipv4(192, 168, 1, 1, 27500);
    chan.setup(NetSource::Client, addr, 12345);
    
    EXPECT_EQ(chan.source(), NetSource::Client);
    EXPECT_EQ(chan.remoteAddress(), addr);
    EXPECT_EQ(chan.qport(), 12345);
    EXPECT_FALSE(chan.hasFatalError());
    EXPECT_EQ(chan.outgoingSequence(), 1);
}

TEST_F(NetChannelTest, MessageBuffer) {
    NetChannel chan;
    auto addr = NetAddress::ipv4(192, 168, 1, 1, 27500);
    chan.setup(NetSource::Client, addr, 12345);
    
    chan.message().writeByte(0x01);
    chan.message().writeString("Test");
    
    EXPECT_GT(chan.message().size(), 0);
    
    chan.message().clear();
    EXPECT_EQ(chan.message().size(), 0);
}

TEST_F(NetChannelTest, RateLimiting) {
    NetChannel chan;
    auto addr = NetAddress::ipv4(192, 168, 1, 1, 27500);
    
    // Advance time past 0 so clearTime < currentTime
    advanceTime(1.0);
    
    chan.setup(NetSource::Client, addr, 12345);
    chan.setRate(1000.0);  // 1000 bytes/sec
    
    // Should be able to send immediately after setup
    EXPECT_TRUE(chan.canPacket());
}

TEST_F(NetChannelTest, Stats) {
    NetChannel chan;
    auto addr = NetAddress::ipv4(192, 168, 1, 1, 27500);
    chan.setup(NetSource::Client, addr, 12345);
    
    EXPECT_EQ(chan.stats().dropCount, 0);
    EXPECT_EQ(chan.stats().goodCount, 0);
    
    chan.resetStats();
    EXPECT_EQ(chan.stats().dropCount, 0);
}

// ============================================================================
// Socket Tests (require actual network stack)
// ============================================================================

TEST(SocketTest, OpenUdpInvalidPort) {
    // Try to open on a privileged port (should work on most systems if run as root,
    // but we're testing the API, not permissions)
    auto result = Socket::openUdp(0);  // Port 0 = any available port
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->isValid());
    EXPECT_FALSE(result->isTcp());
    EXPECT_GT(result->localPort(), 0);
}

TEST(SocketTest, SocketMove) {
    auto result = Socket::openUdp(0);
    ASSERT_TRUE(result.has_value());
    
    Socket s1 = std::move(*result);
    EXPECT_TRUE(s1.isValid());
    
    Socket s2 = std::move(s1);
    EXPECT_FALSE(s1.isValid());
    EXPECT_TRUE(s2.isValid());
}

TEST(SocketTest, SocketClose) {
    auto result = Socket::openUdp(0);
    ASSERT_TRUE(result.has_value());
    
    Socket& s = *result;
    EXPECT_TRUE(s.isValid());
    
    s.close();
    EXPECT_FALSE(s.isValid());
}

TEST(SocketTest, SendToReceiveFrom) {
    // Open two sockets
    auto s1Result = Socket::openUdp(0);
    auto s2Result = Socket::openUdp(0);
    ASSERT_TRUE(s1Result.has_value());
    ASSERT_TRUE(s2Result.has_value());
    
    Socket& sender = *s1Result;
    Socket& receiver = *s2Result;
    
    // Get receiver's address
    auto receiverAddr = NetAddress::ipv4(127, 0, 0, 1, receiver.localPort());
    
    // Send data
    std::vector<Byte> sendData = {0x01, 0x02, 0x03, 0x04, 0x05};
    auto sendResult = sender.sendTo(receiverAddr, sendData);
    ASSERT_TRUE(sendResult.has_value());
    EXPECT_EQ(*sendResult, sendData.size());
    
    // Give it a moment
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    // Receive data
    auto recvResult = receiver.receiveFrom();
    ASSERT_TRUE(recvResult.has_value());
    EXPECT_EQ(recvResult->data, sendData);
    EXPECT_TRUE(recvResult->from.isLoopback() || recvResult->from.ip(0) == 127);
}

TEST(SocketTest, NonBlockingReceive) {
    auto result = Socket::openUdp(0);
    ASSERT_TRUE(result.has_value());
    
    // Non-blocking receive should return WouldBlock when no data
    auto recvResult = result->receiveFrom();
    EXPECT_FALSE(recvResult.has_value());
    EXPECT_EQ(recvResult.error(), NetError::WouldBlock);
}

TEST(SocketTest, HasData) {
    auto s1Result = Socket::openUdp(0);
    auto s2Result = Socket::openUdp(0);
    ASSERT_TRUE(s1Result.has_value());
    ASSERT_TRUE(s2Result.has_value());
    
    Socket& sender = *s1Result;
    Socket& receiver = *s2Result;
    
    EXPECT_FALSE(receiver.hasData());
    
    auto receiverAddr = NetAddress::ipv4(127, 0, 0, 1, receiver.localPort());
    std::vector<Byte> data = {0x01};
    sender.sendTo(receiverAddr, data);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    EXPECT_TRUE(receiver.hasData());
}

TEST(SocketTest, SetNonBlocking) {
    auto result = Socket::openUdp(0);
    ASSERT_TRUE(result.has_value());
    
    auto nbResult = result->setNonBlocking(true);
    EXPECT_TRUE(nbResult.has_value());
    
    nbResult = result->setNonBlocking(false);
    EXPECT_TRUE(nbResult.has_value());
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST(NetIntegrationTest, LoopbackWithNetChannel) {
    // Set up time function
    double currentTime = 0.0;
    setTimeFunction([&currentTime]() { return currentTime; });
    
    Loopback loopback;
    
    // Create client and server channels
    NetChannel clientChan;
    NetChannel serverChan;
    
    auto serverAddr = NetAddress::loopback();
    auto clientAddr = NetAddress::loopback();
    
    clientChan.setup(NetSource::Client, serverAddr, 12345);
    serverChan.setup(NetSource::Server, clientAddr, 0);
    
    // Client builds a message
    clientChan.message().writeByte(0x42);
    clientChan.message().writeString("Hello Server!");
    
    EXPECT_GT(clientChan.message().size(), 0);
}

// ============================================================================
// Thread Safety Tests
// ============================================================================

TEST(SocketThreadSafetyTest, ConcurrentSendReceive) {
    auto s1Result = Socket::openUdp(0);
    auto s2Result = Socket::openUdp(0);
    ASSERT_TRUE(s1Result.has_value());
    ASSERT_TRUE(s2Result.has_value());
    
    Socket sender = std::move(*s1Result);
    Socket receiver = std::move(*s2Result);
    
    auto receiverAddr = NetAddress::ipv4(127, 0, 0, 1, receiver.localPort());
    
    std::atomic<int> sendCount{0};
    std::atomic<int> recvCount{0};
    std::atomic<bool> stopReceiver{false};
    
    // Sender thread
    std::thread senderThread([&]() {
        for (int i = 0; i < 100; ++i) {
            std::vector<Byte> data = {static_cast<Byte>(i)};
            if (sender.sendTo(receiverAddr, data).has_value()) {
                ++sendCount;
            }
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
        stopReceiver = true;
    });
    
    // Receiver thread
    std::thread receiverThread([&]() {
        while (!stopReceiver || receiver.hasData()) {
            auto result = receiver.receiveFrom();
            if (result.has_value()) {
                ++recvCount;
            }
            std::this_thread::sleep_for(std::chrono::microseconds(50));
        }
    });
    
    senderThread.join();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));  // Let receiver finish
    receiverThread.join();
    
    EXPECT_EQ(sendCount, 100);
    EXPECT_GT(recvCount, 0);  // Should receive most packets (UDP may drop some)
}
