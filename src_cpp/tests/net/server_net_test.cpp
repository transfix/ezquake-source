/**
 * @file server_net_test.cpp
 * @brief Tests for server networking types
 * 
 * Part of ezQuake C to C++20 Conversion - Iteration 20
 */

#include <gtest/gtest.h>
#include "core/net/server_net.hpp"

using namespace ezquake::net;

// =============================================================================
// PrintLevel Tests
// =============================================================================

TEST(PrintLevelTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(PrintLevel::Low), 0);
    EXPECT_EQ(static_cast<uint8_t>(PrintLevel::Medium), 1);
    EXPECT_EQ(static_cast<uint8_t>(PrintLevel::High), 2);
    EXPECT_EQ(static_cast<uint8_t>(PrintLevel::Chat), 3);
}

TEST(PrintLevelTest, Names) {
    EXPECT_EQ(printLevelName(PrintLevel::Low), "low");
    EXPECT_EQ(printLevelName(PrintLevel::Medium), "medium");
    EXPECT_EQ(printLevelName(PrintLevel::High), "high");
    EXPECT_EQ(printLevelName(PrintLevel::Chat), "chat");
}

TEST(PrintLevelTest, IsChatMessage) {
    EXPECT_FALSE(isChatMessage(PrintLevel::Low));
    EXPECT_FALSE(isChatMessage(PrintLevel::Medium));
    EXPECT_FALSE(isChatMessage(PrintLevel::High));
    EXPECT_TRUE(isChatMessage(PrintLevel::Chat));
}

// =============================================================================
// BroadcastFlags Tests
// =============================================================================

TEST(BroadcastFlagsTest, Values) {
    EXPECT_EQ(BroadcastFlags::None, 0);
    EXPECT_EQ(BroadcastFlags::IgnoreInDemo, 1);
    EXPECT_EQ(BroadcastFlags::IgnoreClients, 2);
    EXPECT_EQ(BroadcastFlags::QtvOnly, 4);
    EXPECT_EQ(BroadcastFlags::IgnoreConsole, 8);
}

TEST(BroadcastFlagsTest, ShouldPutInDemo) {
    EXPECT_TRUE(BroadcastFlags::shouldPutInDemo(BroadcastFlags::None));
    EXPECT_FALSE(BroadcastFlags::shouldPutInDemo(BroadcastFlags::IgnoreInDemo));
    EXPECT_TRUE(BroadcastFlags::shouldPutInDemo(BroadcastFlags::IgnoreClients));
}

TEST(BroadcastFlagsTest, ShouldSendToClients) {
    EXPECT_TRUE(BroadcastFlags::shouldSendToClients(BroadcastFlags::None));
    EXPECT_TRUE(BroadcastFlags::shouldSendToClients(BroadcastFlags::IgnoreInDemo));
    EXPECT_FALSE(BroadcastFlags::shouldSendToClients(BroadcastFlags::IgnoreClients));
}

TEST(BroadcastFlagsTest, ShouldPutInConsole) {
    EXPECT_TRUE(BroadcastFlags::shouldPutInConsole(BroadcastFlags::None));
    EXPECT_FALSE(BroadcastFlags::shouldPutInConsole(BroadcastFlags::IgnoreConsole));
}

// =============================================================================
// SoundChannel Tests
// =============================================================================

TEST(SoundChannelTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(SoundChannel::Auto), 0);
    EXPECT_EQ(static_cast<uint8_t>(SoundChannel::Weapon), 1);
    EXPECT_EQ(static_cast<uint8_t>(SoundChannel::Voice), 2);
    EXPECT_EQ(static_cast<uint8_t>(SoundChannel::Item), 3);
    EXPECT_EQ(static_cast<uint8_t>(SoundChannel::Body), 4);
}

TEST(SoundChannelTest, Names) {
    EXPECT_EQ(soundChannelName(SoundChannel::Auto), "auto");
    EXPECT_EQ(soundChannelName(SoundChannel::Weapon), "weapon");
    EXPECT_EQ(soundChannelName(SoundChannel::Voice), "voice");
    EXPECT_EQ(soundChannelName(SoundChannel::Item), "item");
    EXPECT_EQ(soundChannelName(SoundChannel::Body), "body");
}

// =============================================================================
// SoundFlags Tests
// =============================================================================

TEST(SoundFlagsTest, Values) {
    EXPECT_EQ(SoundFlags::Volume, 0x8000);
    EXPECT_EQ(SoundFlags::Attenuation, 0x4000);
    EXPECT_EQ(SoundFlags::DefaultVolume, 255);
    EXPECT_FLOAT_EQ(SoundFlags::DefaultAttenuation, 1.0f);
}

// =============================================================================
// Server Protocol Constants Tests
// =============================================================================

TEST(ServerProtocolConstantsTest, Version) {
    EXPECT_EQ(protocol_constants::VERSION, 28);
}

TEST(ServerProtocolConstantsTest, Ports) {
    EXPECT_EQ(protocol_constants::PORT_MASTER, 27000);
    EXPECT_EQ(protocol_constants::PORT_CLIENT, 27001);
    EXPECT_EQ(protocol_constants::PORT_SERVER, 27500);
    EXPECT_EQ(protocol_constants::PORT_QUAKETV, 27900);
}

TEST(ServerProtocolConstantsTest, Limits) {
    EXPECT_EQ(protocol_constants::MAX_CLIENTS, 32);
    EXPECT_EQ(protocol_constants::MAX_PACKET_ENTITIES, 64);
    EXPECT_EQ(protocol_constants::MAX_MVD_PACKET_ENTITIES, 300);
    EXPECT_EQ(protocol_constants::UPDATE_BACKUP, 64);
    EXPECT_EQ(protocol_constants::UPDATE_MASK, 63);
}

// =============================================================================
// OOB Message Tests
// =============================================================================

TEST(OobMessageTest, Values) {
    EXPECT_EQ(oob_message::S2C_CHALLENGE, 'c');
    EXPECT_EQ(oob_message::S2C_CONNECTION, 'j');
    EXPECT_EQ(oob_message::A2A_PING, 'k');
    EXPECT_EQ(oob_message::A2A_ACK, 'l');
    EXPECT_EQ(oob_message::A2C_PRINT, 'n');
}

// =============================================================================
// ServerMessage Tests
// =============================================================================

TEST(ServerMessageTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(ServerMessage::Bad), 0);
    EXPECT_EQ(static_cast<uint8_t>(ServerMessage::Nop), 1);
    EXPECT_EQ(static_cast<uint8_t>(ServerMessage::Disconnect), 2);
    EXPECT_EQ(static_cast<uint8_t>(ServerMessage::Print), 8);
    EXPECT_EQ(static_cast<uint8_t>(ServerMessage::TempEntity), 23);
    EXPECT_EQ(static_cast<uint8_t>(ServerMessage::PacketEntities), 47);
}

TEST(ServerMessageTest, Names) {
    EXPECT_EQ(serverMessageName(ServerMessage::Bad), "svc_bad");
    EXPECT_EQ(serverMessageName(ServerMessage::Disconnect), "svc_disconnect");
    EXPECT_EQ(serverMessageName(ServerMessage::Print), "svc_print");
    EXPECT_EQ(serverMessageName(ServerMessage::TempEntity), "svc_temp_entity");
}

// =============================================================================
// ClientMessage Tests
// =============================================================================

TEST(ClientMessageTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(ClientMessage::Bad), 0);
    EXPECT_EQ(static_cast<uint8_t>(ClientMessage::Move), 3);
    EXPECT_EQ(static_cast<uint8_t>(ClientMessage::StringCmd), 4);
    EXPECT_EQ(static_cast<uint8_t>(ClientMessage::Delta), 5);
}

TEST(ClientMessageTest, Names) {
    EXPECT_EQ(clientMessageName(ClientMessage::Bad), "clc_bad");
    EXPECT_EQ(clientMessageName(ClientMessage::Move), "clc_move");
    EXPECT_EQ(clientMessageName(ClientMessage::StringCmd), "clc_stringcmd");
}

// =============================================================================
// PlayerFlags Tests
// =============================================================================

TEST(PlayerFlagsTest, Values) {
    EXPECT_EQ(PlayerFlags::MSec, 1);
    EXPECT_EQ(PlayerFlags::Command, 2);
    EXPECT_EQ(PlayerFlags::Model, 32);
    EXPECT_EQ(PlayerFlags::Dead, 512);
    EXPECT_EQ(PlayerFlags::Gib, 1024);
}

// =============================================================================
// EntityUpdateFlags Tests
// =============================================================================

TEST(EntityUpdateFlagsTest, Values) {
    EXPECT_EQ(EntityUpdateFlags::Origin1, 0x200);
    EXPECT_EQ(EntityUpdateFlags::Origin2, 0x400);
    EXPECT_EQ(EntityUpdateFlags::Origin3, 0x800);
    EXPECT_EQ(EntityUpdateFlags::Remove, 0x4000);
    EXPECT_EQ(EntityUpdateFlags::MoreBits, 0x8000);
}

TEST(EntityUpdateFlagsTest, ExtendedFlags) {
    EXPECT_EQ(EntityUpdateFlags::Angle1, 1);
    EXPECT_EQ(EntityUpdateFlags::Model, 4);
    EXPECT_EQ(EntityUpdateFlags::Colormap, 8);
}

// =============================================================================
// EntityState Tests
// =============================================================================

TEST(ServerEntityStateTest, Default) {
    EntityState state;
    EXPECT_EQ(state.number, 0);
    EXPECT_EQ(state.flags, 0);
    EXPECT_EQ(state.modelIndex, 0);
    EXPECT_EQ(state.frame, 0);
    EXPECT_EQ(state.trans, 255);
}

TEST(ServerEntityStateTest, IsValid) {
    EntityState state;
    EXPECT_FALSE(state.isValid());
    
    state.number = 1;
    EXPECT_TRUE(state.isValid());
}

TEST(ServerEntityStateTest, IsWorld) {
    EntityState state;
    EXPECT_TRUE(state.isWorld());
    
    state.number = 1;
    EXPECT_FALSE(state.isWorld());
}

TEST(ServerEntityStateTest, Clear) {
    EntityState state;
    state.number = 42;
    state.modelIndex = 5;
    state.trans = 128;
    
    state.clear();
    
    EXPECT_EQ(state.number, 0);
    EXPECT_EQ(state.modelIndex, 0);
    EXPECT_EQ(state.trans, 255);
}

// =============================================================================
// UserCmd Tests
// =============================================================================

TEST(ServerUserCmdTest, Default) {
    UserCmd cmd;
    EXPECT_EQ(cmd.msec, 0);
    EXPECT_EQ(cmd.forwardMove, 0);
    EXPECT_EQ(cmd.sideMove, 0);
    EXPECT_EQ(cmd.upMove, 0);
    EXPECT_EQ(cmd.buttons, 0);
    EXPECT_EQ(cmd.impulse, 0);
}

TEST(ServerUserCmdTest, HasMovement) {
    UserCmd cmd;
    EXPECT_FALSE(cmd.hasMovement());
    
    cmd.forwardMove = 100;
    EXPECT_TRUE(cmd.hasMovement());
    
    cmd.forwardMove = 0;
    cmd.sideMove = 50;
    EXPECT_TRUE(cmd.hasMovement());
}

TEST(ServerUserCmdTest, HasButtons) {
    UserCmd cmd;
    EXPECT_FALSE(cmd.hasButtons());
    
    cmd.buttons = 1;
    EXPECT_TRUE(cmd.hasButtons());
}

TEST(ServerUserCmdTest, HasImpulse) {
    UserCmd cmd;
    EXPECT_FALSE(cmd.hasImpulse());
    
    cmd.impulse = 10;
    EXPECT_TRUE(cmd.hasImpulse());
}

// =============================================================================
// BackBufferState Tests
// =============================================================================

TEST(BackBufferStateTest, Default) {
    BackBufferState state;
    EXPECT_EQ(state.numBackBuffers, 0);
    EXPECT_FALSE(state.hasBackBuffers());
    EXPECT_FALSE(state.isFull());
}

TEST(BackBufferStateTest, HasBackBuffers) {
    BackBufferState state;
    state.numBackBuffers = 1;
    EXPECT_TRUE(state.hasBackBuffers());
}

TEST(BackBufferStateTest, IsFull) {
    BackBufferState state;
    state.numBackBuffers = protocol_constants::MAX_BACK_BUFFERS;
    EXPECT_TRUE(state.isFull());
}

TEST(BackBufferStateTest, TotalSize) {
    BackBufferState state;
    state.numBackBuffers = 3;
    state.sizes[0] = 100;
    state.sizes[1] = 200;
    state.sizes[2] = 150;
    
    EXPECT_EQ(state.totalSize(), 450);
}

TEST(BackBufferStateTest, Clear) {
    BackBufferState state;
    state.numBackBuffers = 3;
    state.sizes[0] = 100;
    
    state.clear();
    
    EXPECT_EQ(state.numBackBuffers, 0);
    EXPECT_EQ(state.sizes[0], 0);
}

// =============================================================================
// EntityDelta Tests
// =============================================================================

TEST(EntityDeltaTest, Default) {
    EntityDelta delta;
    EXPECT_EQ(delta.flags, 0);
    EXPECT_EQ(delta.extraFlags, 0);
    EXPECT_TRUE(delta.isEmpty());
}

TEST(EntityDeltaTest, HasOriginChange) {
    EntityDelta delta;
    EXPECT_FALSE(delta.hasOriginChange());
    
    delta.flags = EntityUpdateFlags::Origin1;
    EXPECT_TRUE(delta.hasOriginChange());
    
    delta.flags = EntityUpdateFlags::Origin2 | EntityUpdateFlags::Origin3;
    EXPECT_TRUE(delta.hasOriginChange());
}

TEST(EntityDeltaTest, HasAngleChange) {
    EntityDelta delta;
    EXPECT_FALSE(delta.hasAngleChange());
    
    delta.flags = EntityUpdateFlags::Angle2;
    EXPECT_TRUE(delta.hasAngleChange());
    
    delta.flags = 0;
    delta.extraFlags = EntityUpdateFlags::Angle1;
    EXPECT_TRUE(delta.hasAngleChange());
}

TEST(EntityDeltaTest, HasModelChange) {
    EntityDelta delta;
    EXPECT_FALSE(delta.hasModelChange());
    
    delta.extraFlags = EntityUpdateFlags::Model;
    EXPECT_TRUE(delta.hasModelChange());
}

TEST(EntityDeltaTest, IsRemove) {
    EntityDelta delta;
    EXPECT_FALSE(delta.isRemove());
    
    delta.flags = EntityUpdateFlags::Remove;
    EXPECT_TRUE(delta.isRemove());
}

// =============================================================================
// SoundEvent Tests
// =============================================================================

TEST(SoundEventTest, Default) {
    SoundEvent event;
    EXPECT_EQ(event.entityNum, 0);
    EXPECT_EQ(event.channel, SoundChannel::Auto);
    EXPECT_EQ(event.soundIndex, 0);
    EXPECT_EQ(event.volume, 255);
    EXPECT_FLOAT_EQ(event.attenuation, 1.0f);
    EXPECT_TRUE(event.usePhs);
    EXPECT_FALSE(event.reliable);
}

TEST(SoundEventTest, HasCustomVolume) {
    SoundEvent event;
    EXPECT_FALSE(event.hasCustomVolume());
    
    event.volume = 200;
    EXPECT_TRUE(event.hasCustomVolume());
}

TEST(SoundEventTest, HasCustomAttenuation) {
    SoundEvent event;
    EXPECT_FALSE(event.hasCustomAttenuation());
    
    event.attenuation = 2.0f;
    EXPECT_TRUE(event.hasCustomAttenuation());
}

TEST(SoundEventTest, EncodeChannel) {
    SoundEvent event;
    event.entityNum = 5;
    event.channel = SoundChannel::Weapon;
    
    uint16_t encoded = event.encodeChannel();
    // Entity shifted left 3 bits + channel
    EXPECT_EQ(encoded & 0x7, 1);  // Channel = Weapon = 1
    EXPECT_EQ((encoded >> 3) & 0x1FF, 5);  // Entity = 5
}

TEST(SoundEventTest, EncodeChannelWithFlags) {
    SoundEvent event;
    event.entityNum = 1;
    event.channel = SoundChannel::Auto;
    event.volume = 128;  // Custom
    event.attenuation = 2.0f;  // Custom
    
    uint16_t encoded = event.encodeChannel();
    EXPECT_TRUE(encoded & SoundFlags::Volume);
    EXPECT_TRUE(encoded & SoundFlags::Attenuation);
}

// =============================================================================
// NailUpdate Tests
// =============================================================================

TEST(NailUpdateTest, Default) {
    NailUpdate nail;
    EXPECT_FLOAT_EQ(nail.origin[0], 0.0f);
    EXPECT_FLOAT_EQ(nail.pitch, 0.0f);
    EXPECT_FLOAT_EQ(nail.yaw, 0.0f);
    EXPECT_EQ(nail.colormap, 0);
}

TEST(NailUpdateTest, Encode) {
    NailUpdate nail;
    nail.origin = Vec3(0.0f, 0.0f, 0.0f);
    nail.pitch = 0.0f;
    nail.yaw = 0.0f;
    
    auto bits = nail.encode();
    
    // With origin at 0,0,0, encoded values should be around 2048 (center of 12-bit range)
    EXPECT_EQ(bits.size(), 6);
}

TEST(NailUpdateTest, EncodeNonZero) {
    NailUpdate nail;
    nail.origin = Vec3(100.0f, -100.0f, 50.0f);
    nail.pitch = 45.0f;
    nail.yaw = 90.0f;
    
    auto bits = nail.encode();
    
    // Just verify encoding produces 6 bytes
    EXPECT_EQ(bits.size(), 6);
}
