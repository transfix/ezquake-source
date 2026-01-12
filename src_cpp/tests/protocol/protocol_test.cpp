/**
 * @file protocol_test.cpp
 * @brief Tests for protocol module
 * 
 * Comprehensive tests for protocol types, message reading/writing,
 * and entity state handling.
 */

#include <gtest/gtest.h>
#include "core/protocol/protocol.hpp"
#include <vector>
#include <cstring>

using namespace ezquake;
using namespace ezquake::math;

//=============================================================================
// Protocol Constants Tests
//=============================================================================

TEST(ProtocolConstantsTest, Version) {
    EXPECT_EQ(protocol::VERSION, 28);
}

TEST(ProtocolConstantsTest, Ports) {
    EXPECT_EQ(protocol::PORT_MASTER, 27000);
    EXPECT_EQ(protocol::PORT_CLIENT, 27001);
    EXPECT_EQ(protocol::PORT_SERVER, 27500);
    EXPECT_EQ(protocol::PORT_QUAKETV, 27900);
}

TEST(ProtocolConstantsTest, FTEMagic) {
    // Verify the magic numbers are calculated correctly
    uint32_t ftex = (('F' << 0) + ('T' << 8) + ('E' << 16) + ('X' << 24));
    EXPECT_EQ(protocol::VERSION_FTE, ftex);
}

//=============================================================================
// Server Command Tests
//=============================================================================

TEST(ServerCommandTest, BasicCommands) {
    EXPECT_EQ(static_cast<uint8_t>(ServerCommand::Bad), 0);
    EXPECT_EQ(static_cast<uint8_t>(ServerCommand::Nop), 1);
    EXPECT_EQ(static_cast<uint8_t>(ServerCommand::Disconnect), 2);
    EXPECT_EQ(static_cast<uint8_t>(ServerCommand::Print), 8);
    EXPECT_EQ(static_cast<uint8_t>(ServerCommand::ServerData), 11);
}

TEST(ServerCommandTest, EntityCommands) {
    EXPECT_EQ(static_cast<uint8_t>(ServerCommand::SpawnStatic), 20);
    EXPECT_EQ(static_cast<uint8_t>(ServerCommand::SpawnBaseline), 22);
    EXPECT_EQ(static_cast<uint8_t>(ServerCommand::PacketEntities), 47);
    EXPECT_EQ(static_cast<uint8_t>(ServerCommand::DeltaPacketEntities), 48);
}

TEST(ServerCommandTest, Names) {
    EXPECT_EQ(serverCommandName(ServerCommand::Bad), "svc_bad");
    EXPECT_EQ(serverCommandName(ServerCommand::Print), "svc_print");
    EXPECT_EQ(serverCommandName(ServerCommand::ServerData), "svc_serverdata");
    EXPECT_EQ(serverCommandName(ServerCommand::PacketEntities), "svc_packetentities");
}

TEST(ServerCommandTest, UnknownName) {
    auto name = serverCommandName(static_cast<ServerCommand>(255));
    EXPECT_EQ(name, "svc_unknown");
}

//=============================================================================
// Client Command Tests
//=============================================================================

TEST(ClientCommandTest, BasicCommands) {
    EXPECT_EQ(static_cast<uint8_t>(ClientCommand::Bad), 0);
    EXPECT_EQ(static_cast<uint8_t>(ClientCommand::Move), 3);
    EXPECT_EQ(static_cast<uint8_t>(ClientCommand::StringCmd), 4);
    EXPECT_EQ(static_cast<uint8_t>(ClientCommand::Delta), 5);
}

TEST(ClientCommandTest, Names) {
    EXPECT_EQ(clientCommandName(ClientCommand::Move), "clc_move");
    EXPECT_EQ(clientCommandName(ClientCommand::StringCmd), "clc_stringcmd");
}

//=============================================================================
// Player Flags Tests
//=============================================================================

TEST(PlayerFlagsTest, BasicFlags) {
    EXPECT_EQ(PlayerFlags::MSEC, 1u);
    EXPECT_EQ(PlayerFlags::COMMAND, 2u);
    EXPECT_EQ(PlayerFlags::VELOCITY1, 4u);
    EXPECT_EQ(PlayerFlags::MODEL, 32u);
    EXPECT_EQ(PlayerFlags::DEAD, 512u);
}

TEST(PlayerFlagsTest, ExtractMoveType) {
    // Normal (0) in bits 11-13
    uint32_t normal = 0;
    EXPECT_EQ(PlayerFlags::extractMoveType(normal), 0);
    
    // Spectator (3) in bits 11-13 = 3 << 11 = 0x1800
    uint32_t spectator = 3 << PlayerFlags::PMC_SHIFT;
    EXPECT_EQ(PlayerFlags::extractMoveType(spectator), 3);
    
    // Fly (4) in bits 11-13
    uint32_t fly = 4 << PlayerFlags::PMC_SHIFT;
    EXPECT_EQ(PlayerFlags::extractMoveType(fly), 4);
}

//=============================================================================
// Entity Flags Tests
//=============================================================================

TEST(EntityFlagsTest, UpdateFlags) {
    EXPECT_EQ(EntityFlags::ORIGIN1, 0x0200u);
    EXPECT_EQ(EntityFlags::ORIGIN2, 0x0400u);
    EXPECT_EQ(EntityFlags::ORIGIN3, 0x0800u);
    EXPECT_EQ(EntityFlags::REMOVE, 0x4000u);
    EXPECT_EQ(EntityFlags::MOREBITS, 0x8000u);
}

TEST(EntityFlagsTest, ExtractEntityNum) {
    // Entity 123 with ORIGIN1 flag
    uint16_t word = 123 | EntityFlags::ORIGIN1;
    EXPECT_EQ(EntityFlags::extractEntityNum(word), 123);
    
    // Entity 511 (max with 9 bits)
    word = 511 | EntityFlags::MOREBITS;
    EXPECT_EQ(EntityFlags::extractEntityNum(word), 511);
}

//=============================================================================
// Command Flags Tests
//=============================================================================

TEST(CommandFlagsTest, MoveFlags) {
    EXPECT_EQ(CommandFlags::ANGLE1, 1u);
    EXPECT_EQ(CommandFlags::ANGLE3, 2u);
    EXPECT_EQ(CommandFlags::FORWARD, 4u);
    EXPECT_EQ(CommandFlags::SIDE, 8u);
    EXPECT_EQ(CommandFlags::BUTTONS, 32u);
    EXPECT_EQ(CommandFlags::IMPULSE, 64u);
    EXPECT_EQ(CommandFlags::ANGLE2, 128u);
}

//=============================================================================
// ZQuake Extensions Tests
//=============================================================================

TEST(ZExtensionsTest, Extensions) {
    EXPECT_EQ(ZExtensions::PM_TYPE, 1u);
    EXPECT_EQ(ZExtensions::VIEWHEIGHT, 4u);
    EXPECT_EQ(ZExtensions::VWEP, 128u);
    
    // Client extensions should include all standard ones
    EXPECT_TRUE((ZExtensions::CLIENT & ZExtensions::PM_TYPE) != 0);
    EXPECT_TRUE((ZExtensions::CLIENT & ZExtensions::VWEP) != 0);
}

//=============================================================================
// Print Level Tests
//=============================================================================

TEST(PrintLevelTest, Levels) {
    EXPECT_EQ(static_cast<uint8_t>(PrintLevel::Low), 0);
    EXPECT_EQ(static_cast<uint8_t>(PrintLevel::Medium), 1);
    EXPECT_EQ(static_cast<uint8_t>(PrintLevel::High), 2);
    EXPECT_EQ(static_cast<uint8_t>(PrintLevel::Chat), 3);
}

//=============================================================================
// Temp Entity Type Tests
//=============================================================================

TEST(TempEntityTypeTest, Types) {
    EXPECT_EQ(static_cast<uint8_t>(TempEntityType::Spike), 0);
    EXPECT_EQ(static_cast<uint8_t>(TempEntityType::Explosion), 3);
    EXPECT_EQ(static_cast<uint8_t>(TempEntityType::Teleport), 11);
    EXPECT_EQ(static_cast<uint8_t>(TempEntityType::Blood), 12);
}

//=============================================================================
// Network Limits Tests
//=============================================================================

TEST(NetworkLimitsTest, Limits) {
    EXPECT_EQ(NetworkLimits::MAX_CLIENTS, 32);
    EXPECT_EQ(NetworkLimits::MAX_MODELS, 512);
    EXPECT_EQ(NetworkLimits::UPDATE_BACKUP, 64);
    EXPECT_EQ(NetworkLimits::UPDATE_MASK, 63);
}

//=============================================================================
// Message Reader Tests - Primitives
//=============================================================================

TEST(MessageReaderTest, EmptyBuffer) {
    MessageReader reader;
    EXPECT_EQ(reader.size(), 0);
    EXPECT_TRUE(reader.atEnd());
    EXPECT_EQ(reader.readByte(), -1);
    EXPECT_TRUE(reader.badRead());
}

TEST(MessageReaderTest, ReadByte) {
    std::vector<uint8_t> data = {0x00, 0x7F, 0xFF};
    MessageReader reader(data);
    
    EXPECT_EQ(reader.readByte(), 0);
    EXPECT_EQ(reader.readByte(), 127);
    EXPECT_EQ(reader.readByte(), 255);
    EXPECT_TRUE(reader.atEnd());
    EXPECT_FALSE(reader.badRead());
}

TEST(MessageReaderTest, ReadChar) {
    std::vector<uint8_t> data = {0x00, 0x7F, 0xFF, 0x80};
    MessageReader reader(data);
    
    EXPECT_EQ(reader.readChar(), 0);
    EXPECT_EQ(reader.readChar(), 127);
    EXPECT_EQ(reader.readChar(), -1);
    EXPECT_EQ(reader.readChar(), -128);
}

TEST(MessageReaderTest, ReadShort) {
    // Little-endian: 0x1234 stored as [0x34, 0x12]
    std::vector<uint8_t> data = {0x34, 0x12, 0xFF, 0xFF};
    MessageReader reader(data);
    
    EXPECT_EQ(reader.readShort(), 0x1234);
    EXPECT_EQ(reader.readShort(), -1);  // 0xFFFF as signed
}

TEST(MessageReaderTest, ReadLong) {
    // Little-endian: 0x12345678 stored as [0x78, 0x56, 0x34, 0x12]
    std::vector<uint8_t> data = {0x78, 0x56, 0x34, 0x12};
    MessageReader reader(data);
    
    EXPECT_EQ(reader.readLong(), 0x12345678);
}

TEST(MessageReaderTest, ReadFloat) {
    // 1.0f in little-endian IEEE 754
    std::vector<uint8_t> data = {0x00, 0x00, 0x80, 0x3F};
    MessageReader reader(data);
    
    EXPECT_FLOAT_EQ(reader.readFloat(), 1.0f);
}

TEST(MessageReaderTest, ReadString) {
    std::vector<uint8_t> data = {'H', 'e', 'l', 'l', 'o', '\0', 'W'};
    MessageReader reader(data);
    
    EXPECT_EQ(reader.readString(), "Hello");
    EXPECT_EQ(reader.position(), 6);
}

TEST(MessageReaderTest, ReadStringMaxLen) {
    std::vector<uint8_t> data = {'L', 'o', 'n', 'g', 's', 't', 'r', '\0'};
    MessageReader reader(data);
    
    EXPECT_EQ(reader.readString(3), "Lon");
}

TEST(MessageReaderTest, ReadLine) {
    std::vector<uint8_t> data = {'l', 'i', 'n', 'e', '1', '\n', 'l', 'i', 'n', 'e', '2'};
    MessageReader reader(data);
    
    EXPECT_EQ(reader.readLine(), "line1");
    EXPECT_EQ(reader.readLine(), "line2");
}

//=============================================================================
// Message Reader Tests - Coordinates
//=============================================================================

TEST(MessageReaderTest, ReadCoordShort) {
    // 100.0 * 8 = 800 = 0x0320
    std::vector<uint8_t> data = {0x20, 0x03};
    MessageReader reader(data);
    
    EXPECT_FLOAT_EQ(reader.readCoord(CoordMode::Short), 100.0f);
}

TEST(MessageReaderTest, ReadCoordFloat) {
    // 100.0f in little-endian
    std::vector<uint8_t> data = {0x00, 0x00, 0xC8, 0x42};
    MessageReader reader(data);
    
    EXPECT_FLOAT_EQ(reader.readCoord(CoordMode::Float), 100.0f);
}

TEST(MessageReaderTest, ReadAngleByte) {
    // 90 degrees = 64/256 * 360 = 90
    std::vector<uint8_t> data = {64};
    MessageReader reader(data);
    
    EXPECT_FLOAT_EQ(reader.readAngle(AngleMode::Byte), 90.0f);
}

TEST(MessageReaderTest, ReadAngleShort) {
    // 180 degrees = 32768/65536 * 360 = 180
    std::vector<uint8_t> data = {0x00, 0x80};
    MessageReader reader(data);
    
    EXPECT_FLOAT_EQ(reader.readAngle(AngleMode::Short), 180.0f);
}

TEST(MessageReaderTest, ReadCoordVec) {
    // Three short coords: (8.0, 16.0, 24.0)
    std::vector<uint8_t> data = {
        0x40, 0x00,  // 64/8 = 8.0
        0x80, 0x00,  // 128/8 = 16.0
        0xC0, 0x00   // 192/8 = 24.0
    };
    MessageReader reader(data);
    
    Vec3 v = reader.readCoordVec();
    EXPECT_FLOAT_EQ(v.x(), 8.0f);
    EXPECT_FLOAT_EQ(v.y(), 16.0f);
    EXPECT_FLOAT_EQ(v.z(), 24.0f);
}

TEST(MessageReaderTest, ReadAngleVec) {
    // Three byte angles: (0, 90, 180)
    std::vector<uint8_t> data = {0, 64, 128};
    MessageReader reader(data);
    
    Vec3 v = reader.readAngleVec();
    EXPECT_FLOAT_EQ(v.x(), 0.0f);
    EXPECT_FLOAT_EQ(v.y(), 90.0f);
    EXPECT_FLOAT_EQ(v.z(), 180.0f);
}

//=============================================================================
// Message Reader Tests - Buffer Operations
//=============================================================================

TEST(MessageReaderTest, Position) {
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    MessageReader reader(data);
    
    EXPECT_EQ(reader.position(), 0);
    reader.readByte();
    EXPECT_EQ(reader.position(), 1);
    reader.readShort();
    EXPECT_EQ(reader.position(), 3);
}

TEST(MessageReaderTest, Seek) {
    std::vector<uint8_t> data = {10, 20, 30, 40, 50};
    MessageReader reader(data);
    
    reader.seek(2);
    EXPECT_EQ(reader.readByte(), 30);
    
    reader.seek(0);
    EXPECT_EQ(reader.readByte(), 10);
}

TEST(MessageReaderTest, Skip) {
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    MessageReader reader(data);
    
    reader.skip(3);
    EXPECT_EQ(reader.readByte(), 4);
}

TEST(MessageReaderTest, Remaining) {
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    MessageReader reader(data);
    
    EXPECT_EQ(reader.remaining(), 5);
    reader.readShort();
    EXPECT_EQ(reader.remaining(), 3);
}

TEST(MessageReaderTest, Rewind) {
    std::vector<uint8_t> data = {1, 2, 3};
    MessageReader reader(data);
    
    reader.readByte();
    reader.readByte();
    EXPECT_EQ(reader.position(), 2);
    
    reader.rewind();
    EXPECT_EQ(reader.position(), 0);
    EXPECT_EQ(reader.readByte(), 1);
}

TEST(MessageReaderTest, PeekByte) {
    std::vector<uint8_t> data = {42, 43};
    MessageReader reader(data);
    
    EXPECT_EQ(reader.peekByte(), 42);
    EXPECT_EQ(reader.position(), 0);
    reader.readByte();
    EXPECT_EQ(reader.peekByte(), 43);
}

TEST(MessageReaderTest, ReadBytes) {
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    MessageReader reader(data);
    
    uint8_t buf[3];
    size_t read = reader.readBytes(buf, 3);
    
    EXPECT_EQ(read, 3);
    EXPECT_EQ(buf[0], 1);
    EXPECT_EQ(buf[1], 2);
    EXPECT_EQ(buf[2], 3);
}

TEST(MessageReaderTest, ReadBytesView) {
    std::vector<uint8_t> data = {10, 20, 30, 40, 50};
    MessageReader reader(data);
    
    auto view = reader.readBytesView(3);
    EXPECT_EQ(view.size(), 3);
    EXPECT_EQ(view[0], 10);
    EXPECT_EQ(view[1], 20);
    EXPECT_EQ(view[2], 30);
}

TEST(MessageReaderTest, EntityWord) {
    // Entity 100 with ORIGIN1 | ORIGIN2 flags
    uint16_t word = 100 | EntityFlags::ORIGIN1 | EntityFlags::ORIGIN2;
    std::vector<uint8_t> data = {
        static_cast<uint8_t>(word & 0xFF),
        static_cast<uint8_t>((word >> 8) & 0xFF)
    };
    MessageReader reader(data);
    
    auto result = reader.readEntityWord();
    EXPECT_EQ(result.entityNum, 100);
    EXPECT_TRUE((result.flags & EntityFlags::ORIGIN1) != 0);
    EXPECT_TRUE((result.flags & EntityFlags::ORIGIN2) != 0);
}

//=============================================================================
// Message Writer Tests
//=============================================================================

TEST(MessageWriterTest, Empty) {
    MessageWriter writer;
    EXPECT_EQ(writer.size(), 0);
    EXPECT_FALSE(writer.overflow());
}

TEST(MessageWriterTest, WriteByte) {
    MessageWriter writer;
    writer.writeByte(0x42);
    writer.writeByte(0xFF);
    
    EXPECT_EQ(writer.size(), 2);
    EXPECT_EQ(writer.data()[0], 0x42);
    EXPECT_EQ(writer.data()[1], 0xFF);
}

TEST(MessageWriterTest, WriteShort) {
    MessageWriter writer;
    writer.writeShort(0x1234);
    
    EXPECT_EQ(writer.size(), 2);
    EXPECT_EQ(writer.data()[0], 0x34);  // Little-endian low byte
    EXPECT_EQ(writer.data()[1], 0x12);  // Little-endian high byte
}

TEST(MessageWriterTest, WriteLong) {
    MessageWriter writer;
    writer.writeLong(0x12345678);
    
    EXPECT_EQ(writer.size(), 4);
    EXPECT_EQ(writer.data()[0], 0x78);
    EXPECT_EQ(writer.data()[1], 0x56);
    EXPECT_EQ(writer.data()[2], 0x34);
    EXPECT_EQ(writer.data()[3], 0x12);
}

TEST(MessageWriterTest, WriteFloat) {
    MessageWriter writer;
    writer.writeFloat(1.0f);
    
    EXPECT_EQ(writer.size(), 4);
    // 1.0f in little-endian IEEE 754
    EXPECT_EQ(writer.data()[0], 0x00);
    EXPECT_EQ(writer.data()[1], 0x00);
    EXPECT_EQ(writer.data()[2], 0x80);
    EXPECT_EQ(writer.data()[3], 0x3F);
}

TEST(MessageWriterTest, WriteString) {
    MessageWriter writer;
    writer.writeString("Hi");
    
    EXPECT_EQ(writer.size(), 3);
    EXPECT_EQ(writer.data()[0], 'H');
    EXPECT_EQ(writer.data()[1], 'i');
    EXPECT_EQ(writer.data()[2], '\0');
}

TEST(MessageWriterTest, WriteCoord) {
    MessageWriter writer;
    writer.writeCoord(100.0f, CoordMode::Short);
    
    // 100.0 * 8 = 800 = 0x0320
    EXPECT_EQ(writer.size(), 2);
    EXPECT_EQ(writer.data()[0], 0x20);
    EXPECT_EQ(writer.data()[1], 0x03);
}

TEST(MessageWriterTest, WriteAngle) {
    MessageWriter writer;
    writer.writeAngle(90.0f, AngleMode::Byte);
    
    // 90 degrees = 64/256 of full rotation
    EXPECT_EQ(writer.size(), 1);
    EXPECT_EQ(writer.data()[0], 64);
}

TEST(MessageWriterTest, RoundTrip) {
    MessageWriter writer;
    writer.writeByte(42);
    writer.writeShort(-1234);
    writer.writeLong(0x12345678);
    writer.writeFloat(3.14159f);
    writer.writeString("test");
    
    MessageReader reader(writer.data());
    EXPECT_EQ(reader.readByte(), 42);
    EXPECT_EQ(reader.readShort(), -1234);
    EXPECT_EQ(reader.readLong(), 0x12345678);
    EXPECT_NEAR(reader.readFloat(), 3.14159f, 0.00001f);
    EXPECT_EQ(reader.readString(), "test");
}

TEST(MessageWriterTest, Clear) {
    MessageWriter writer;
    writer.writeByte(1);
    writer.writeByte(2);
    EXPECT_EQ(writer.size(), 2);
    
    writer.clear();
    EXPECT_EQ(writer.size(), 0);
}

TEST(MessageWriterTest, Overflow) {
    MessageWriter writer;
    writer.setMaxSize(2);
    
    writer.writeByte(1);
    writer.writeByte(2);
    EXPECT_FALSE(writer.overflow());
    
    writer.writeByte(3);
    EXPECT_TRUE(writer.overflow());
    EXPECT_EQ(writer.size(), 2);  // Third byte not written
}

//=============================================================================
// Entity State Tests
//=============================================================================

TEST(ProtocolEntityStateTest, DefaultConstruction) {
    NetworkEntityState state;
    
    EXPECT_EQ(state.number(), 0);
    EXPECT_FALSE(state.isValid());
    EXPECT_FLOAT_EQ(state.origin().x(), 0.0f);
    EXPECT_EQ(state.modelIndex(), 0);
    EXPECT_EQ(state.alpha(), 255);  // Default opaque
}

TEST(ProtocolEntityStateTest, WithNumber) {
    NetworkEntityState state(123);
    
    EXPECT_EQ(state.number(), 123);
    EXPECT_TRUE(state.isValid());
}

TEST(ProtocolEntityStateTest, SetOrigin) {
    NetworkEntityState state;
    state.setOrigin(Vec3(100.0f, 200.0f, 300.0f));
    
    EXPECT_FLOAT_EQ(state.origin().x(), 100.0f);
    EXPECT_FLOAT_EQ(state.origin().y(), 200.0f);
    EXPECT_FLOAT_EQ(state.origin().z(), 300.0f);
}

TEST(ProtocolEntityStateTest, SetAngles) {
    NetworkEntityState state;
    state.setAngles(Vec3(10.0f, 20.0f, 30.0f));
    
    EXPECT_FLOAT_EQ(state.angles().x(), 10.0f);
    EXPECT_FLOAT_EQ(state.angles().y(), 20.0f);
    EXPECT_FLOAT_EQ(state.angles().z(), 30.0f);
}

TEST(ProtocolEntityStateTest, SetModelAndFrame) {
    NetworkEntityState state;
    state.setModelIndex(42);
    state.setFrame(10);
    
    EXPECT_EQ(state.modelIndex(), 42);
    EXPECT_EQ(state.frame(), 10);
}

TEST(ProtocolEntityStateTest, SetAppearance) {
    NetworkEntityState state;
    state.setColormap(1);
    state.setSkinnum(2);
    state.setEffects(3);
    
    EXPECT_EQ(state.colormap(), 1);
    EXPECT_EQ(state.skinnum(), 2);
    EXPECT_EQ(state.effects(), 3);
}

TEST(ProtocolEntityStateTest, FTEExtensions) {
    NetworkEntityState state;
    state.setAlpha(128);
    state.setScale(32);
    state.setColormod({64, 64, 64});
    
    EXPECT_EQ(state.alpha(), 128);
    EXPECT_EQ(state.scale(), 32);
    EXPECT_EQ(state.colormod()[0], 64);
}

TEST(ProtocolEntityStateTest, DiffFlags) {
    NetworkEntityState a, b;
    
    // Same state = no diff
    EXPECT_EQ(a.diffFlags(b), 0u);
    
    // Different origin.x
    b.setOrigin(Vec3(1.0f, 0.0f, 0.0f));
    EXPECT_TRUE((a.diffFlags(b) & EntityFlags::ORIGIN1) != 0);
    
    // Different model
    b.setOrigin(Vec3());
    b.setModelIndex(5);
    EXPECT_TRUE((a.diffFlags(b) & EntityFlags::MODEL) != 0);
}

TEST(ProtocolEntityStateTest, Clear) {
    NetworkEntityState state(123);
    state.setOrigin(Vec3(100.0f, 200.0f, 300.0f));
    state.setModelIndex(42);
    
    state.clear();
    
    EXPECT_EQ(state.number(), 0);
    EXPECT_FALSE(state.isValid());
    EXPECT_FLOAT_EQ(state.origin().x(), 0.0f);
}

TEST(ProtocolEntityStateTest, CopyFrom) {
    NetworkEntityState a(100);
    a.setOrigin(Vec3(1.0f, 2.0f, 3.0f));
    a.setModelIndex(42);
    
    NetworkEntityState b;
    b.copyFrom(a);
    
    EXPECT_EQ(b.number(), 100);
    EXPECT_FLOAT_EQ(b.origin().x(), 1.0f);
    EXPECT_EQ(b.modelIndex(), 42);
}

//=============================================================================
// Packet Entities Tests
//=============================================================================

TEST(ProtocolPacketEntitiesTest, Empty) {
    NetworkPacketEntities packet;
    
    EXPECT_EQ(packet.count(), 0);
}

TEST(ProtocolPacketEntitiesTest, AddEntities) {
    NetworkPacketEntities packet;
    
    NetworkEntityState e1(1);
    NetworkEntityState e2(2);
    NetworkEntityState e3(3);
    
    EXPECT_TRUE(packet.add(e1));
    EXPECT_TRUE(packet.add(e2));
    EXPECT_TRUE(packet.add(e3));
    
    EXPECT_EQ(packet.count(), 3);
    EXPECT_EQ(packet[0].number(), 1);
    EXPECT_EQ(packet[1].number(), 2);
    EXPECT_EQ(packet[2].number(), 3);
}

TEST(ProtocolPacketEntitiesTest, Find) {
    NetworkPacketEntities packet;
    
    NetworkEntityState e1(10);
    e1.setModelIndex(42);
    packet.add(e1);
    
    NetworkEntityState e2(20);
    e2.setModelIndex(84);
    packet.add(e2);
    
    auto* found = packet.find(20);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->modelIndex(), 84);
    
    EXPECT_EQ(packet.find(999), nullptr);
}

TEST(ProtocolPacketEntitiesTest, Clear) {
    NetworkPacketEntities packet;
    packet.add(NetworkEntityState(1));
    packet.add(NetworkEntityState(2));
    EXPECT_EQ(packet.count(), 2);
    
    packet.clear();
    EXPECT_EQ(packet.count(), 0);
}

TEST(ProtocolPacketEntitiesTest, Iteration) {
    NetworkPacketEntities packet;
    packet.add(NetworkEntityState(1));
    packet.add(NetworkEntityState(2));
    packet.add(NetworkEntityState(3));
    
    int sum = 0;
    for (const auto& ent : packet) {
        sum += ent.number();
    }
    EXPECT_EQ(sum, 6);  // 1 + 2 + 3
}

//=============================================================================
// Network Player State Tests
//=============================================================================

TEST(NetworkPlayerStateTest, DefaultConstruction) {
    NetworkPlayerState state;
    
    EXPECT_FLOAT_EQ(state.origin().x(), 0.0f);
    EXPECT_FLOAT_EQ(state.velocity().x(), 0.0f);
    EXPECT_EQ(state.modelIndex(), 0);
    EXPECT_EQ(state.flags(), 0u);
}

TEST(NetworkPlayerStateTest, Position) {
    NetworkPlayerState state;
    state.setOrigin(Vec3(100.0f, 200.0f, 300.0f));
    state.setVelocity(Vec3(10.0f, 20.0f, 30.0f));
    
    EXPECT_FLOAT_EQ(state.origin().z(), 300.0f);
    EXPECT_FLOAT_EQ(state.velocity().y(), 20.0f);
}

TEST(NetworkPlayerStateTest, Angles) {
    NetworkPlayerState state;
    state.setViewAngles(Vec3(10.0f, 90.0f, 0.0f));
    state.setCmdAngles(Vec3(0.0f, 45.0f, 0.0f));
    
    EXPECT_FLOAT_EQ(state.viewAngles().y(), 90.0f);
    EXPECT_FLOAT_EQ(state.cmdAngles().y(), 45.0f);
}

TEST(NetworkPlayerStateTest, Flags) {
    NetworkPlayerState state;
    
    state.setFlags(PlayerFlags::DEAD | PlayerFlags::ONGROUND);
    
    EXPECT_TRUE(state.isDead());
    EXPECT_TRUE(state.onGround());
    EXPECT_FALSE(state.isGib());
}

TEST(NetworkPlayerStateTest, MoveType) {
    NetworkPlayerState state;
    
    // Set spectator mode (3) in move type bits
    state.setFlags(3 << PlayerFlags::PMC_SHIFT);
    
    EXPECT_EQ(state.moveType(), 3);
}

TEST(NetworkPlayerStateTest, UserCommand) {
    NetworkPlayerState state;
    state.setForwardMove(400);
    state.setSideMove(-200);
    state.setUpMove(0);
    state.setButtons(1);
    state.setImpulse(8);
    
    EXPECT_EQ(state.forwardMove(), 400);
    EXPECT_EQ(state.sideMove(), -200);
    EXPECT_EQ(state.upMove(), 0);
    EXPECT_EQ(state.buttons(), 1);
    EXPECT_EQ(state.impulse(), 8);
}

//=============================================================================
// Entity Baselines Tests
//=============================================================================

TEST(EntityBaselinesTest, GetSet) {
    NetworkEntityBaselines baselines;
    
    NetworkEntityState state(42);
    state.setModelIndex(10);
    state.setOrigin(Vec3(1.0f, 2.0f, 3.0f));
    
    baselines.set(42, state);
    
    const auto& retrieved = baselines.get(42);
    EXPECT_EQ(retrieved.number(), 42);
    EXPECT_EQ(retrieved.modelIndex(), 10);
    EXPECT_FLOAT_EQ(retrieved.origin().x(), 1.0f);
}

TEST(EntityBaselinesTest, Clear) {
    NetworkEntityBaselines baselines;
    
    NetworkEntityState state(100);
    state.setModelIndex(5);
    baselines.set(100, state);
    
    baselines.clear();
    
    EXPECT_EQ(baselines.get(100).modelIndex(), 0);
}

TEST(EntityBaselinesTest, Wrapping) {
    NetworkEntityBaselines baselines;
    
    // Entity numbers wrap at MAX_EDICTS
    NetworkEntityState state1(0);
    state1.setModelIndex(1);
    baselines.set(0, state1);
    
    // Should wrap
    NetworkEntityState state2(NetworkLimits::MAX_EDICTS);
    state2.setModelIndex(2);
    baselines.set(NetworkLimits::MAX_EDICTS, state2);
    
    // Both should refer to same slot
    EXPECT_EQ(baselines.get(0).modelIndex(), 2);
}

//=============================================================================
// Integration Tests
//=============================================================================

TEST(ProtocolIntegrationTest, WriteReadEntityUpdate) {
    // Simulate writing an entity update
    MessageWriter writer;
    
    // Entity 50 with origin change
    uint16_t word = 50 | EntityFlags::ORIGIN1 | EntityFlags::ORIGIN2 | EntityFlags::ORIGIN3;
    writer.writeUShort(word);
    writer.writeCoord(100.0f);
    writer.writeCoord(200.0f);
    writer.writeCoord(300.0f);
    
    // Read it back
    MessageReader reader(writer.data());
    
    auto ew = reader.readEntityWord();
    EXPECT_EQ(ew.entityNum, 50);
    
    float x = reader.readCoord();
    float y = reader.readCoord();
    float z = reader.readCoord();
    
    EXPECT_FLOAT_EQ(x, 100.0f);
    EXPECT_FLOAT_EQ(y, 200.0f);
    EXPECT_FLOAT_EQ(z, 300.0f);
}

TEST(ProtocolIntegrationTest, ServerMessageSimulation) {
    // Simulate a server message with multiple commands
    MessageWriter writer;
    
    // svc_print
    writer.writeByte(static_cast<uint8_t>(ServerCommand::Print));
    writer.writeByte(static_cast<uint8_t>(PrintLevel::High));
    writer.writeString("Test message");
    
    // svc_centerprint
    writer.writeByte(static_cast<uint8_t>(ServerCommand::CenterPrint));
    writer.writeString("Center text");
    
    // Read it back
    MessageReader reader(writer.data());
    
    auto cmd1 = static_cast<ServerCommand>(reader.readByte());
    EXPECT_EQ(cmd1, ServerCommand::Print);
    auto level = static_cast<PrintLevel>(reader.readByte());
    EXPECT_EQ(level, PrintLevel::High);
    EXPECT_EQ(reader.readString(), "Test message");
    
    auto cmd2 = static_cast<ServerCommand>(reader.readByte());
    EXPECT_EQ(cmd2, ServerCommand::CenterPrint);
    EXPECT_EQ(reader.readString(), "Center text");
    
    EXPECT_TRUE(reader.atEnd());
}

TEST(ProtocolIntegrationTest, CoordPrecision) {
    // Test coordinate precision at various values
    std::vector<float> testValues = {
        0.0f, 1.0f, -1.0f, 0.125f, -0.125f,  // Exact values
        100.5f, -100.5f,                       // Common game coords
        4095.875f, -4095.875f                  // Near max range
    };
    
    for (float original : testValues) {
        MessageWriter writer;
        writer.writeCoord(original, CoordMode::Short);
        
        MessageReader reader(writer.data());
        float result = reader.readCoord(CoordMode::Short);
        
        // Short coords have 1/8 precision
        EXPECT_NEAR(result, original, 0.0625f) << "Failed for value: " << original;
    }
}

// Main is provided by test_main.cpp
