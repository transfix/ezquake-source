/*
 * ezQuake C++ Port - Client Types Unit Tests
 * 
 * Copyright (C) 2024 ezQuake Team
 */

#include <gtest/gtest.h>
#include "core/client/client_types.hpp"
#include "core/client/player_state.hpp"
#include "core/client/frame.hpp"

using namespace ezquake;

//=============================================================================
// UserCommand Tests
//=============================================================================

TEST(UserCommandTest, DefaultConstruction) {
    UserCommand cmd;
    
    EXPECT_EQ(cmd.msec(), 0);
    EXPECT_EQ(cmd.forwardMove(), 0);
    EXPECT_EQ(cmd.sideMove(), 0);
    EXPECT_EQ(cmd.upMove(), 0);
    EXPECT_EQ(cmd.buttons(), client::Button::None);
    EXPECT_EQ(cmd.impulse(), 0);
    EXPECT_TRUE(cmd.empty());
}

TEST(UserCommandTest, Mutators) {
    UserCommand cmd;
    
    cmd.setMsec(100);
    cmd.setAngles(Vec3{10.0f, 20.0f, 0.0f});
    cmd.setForwardMove(400);
    cmd.setSideMove(-200);
    cmd.setUpMove(100);
    cmd.setButtons(client::Button::Attack);
    cmd.setImpulse(7);
    
    EXPECT_EQ(cmd.msec(), 100);
    EXPECT_FLOAT_EQ(cmd.angles().x(), 10.0f);
    EXPECT_FLOAT_EQ(cmd.angles().y(), 20.0f);
    EXPECT_EQ(cmd.forwardMove(), 400);
    EXPECT_EQ(cmd.sideMove(), -200);
    EXPECT_EQ(cmd.upMove(), 100);
    EXPECT_TRUE(cmd.isAttacking());
    EXPECT_FALSE(cmd.isJumping());
    EXPECT_EQ(cmd.impulse(), 7);
    EXPECT_FALSE(cmd.empty());
}

TEST(UserCommandTest, ButtonFlags) {
    UserCommand cmd;
    
    cmd.setButtons(client::Button::Attack | client::Button::Jump);
    
    EXPECT_TRUE(cmd.isAttacking());
    EXPECT_TRUE(cmd.isJumping());
    EXPECT_FALSE(cmd.isUsing());
    EXPECT_FALSE(cmd.isAttacking2());
}

TEST(UserCommandTest, BuilderPattern) {
    UserCommand cmd = UserCommand{}
        .withMsec(50)
        .withForwardMove(400)
        .withSideMove(100)
        .withButtons(client::Button::Jump);
    
    EXPECT_EQ(cmd.msec(), 50);
    EXPECT_EQ(cmd.forwardMove(), 400);
    EXPECT_EQ(cmd.sideMove(), 100);
    EXPECT_TRUE(cmd.isJumping());
}

TEST(UserCommandTest, Clear) {
    UserCommand cmd;
    cmd.setMsec(100);
    cmd.setForwardMove(400);
    cmd.setButtons(client::Button::Attack);
    
    cmd.clear();
    
    EXPECT_EQ(cmd.msec(), 0);
    EXPECT_EQ(cmd.forwardMove(), 0);
    EXPECT_EQ(cmd.buttons(), client::Button::None);
    EXPECT_TRUE(cmd.empty());
}

TEST(UserCommandTest, Equality) {
    UserCommand cmd1;
    cmd1.setMsec(100);
    cmd1.setForwardMove(400);
    
    UserCommand cmd2;
    cmd2.setMsec(100);
    cmd2.setForwardMove(400);
    
    UserCommand cmd3;
    cmd3.setMsec(50);
    cmd3.setForwardMove(400);
    
    EXPECT_EQ(cmd1, cmd2);
    EXPECT_NE(cmd1, cmd3);
}

//=============================================================================
// ColorShift Tests
//=============================================================================

TEST(ColorShiftTest, DefaultConstruction) {
    ColorShift shift;
    
    EXPECT_EQ(shift.red(), 0);
    EXPECT_EQ(shift.green(), 0);
    EXPECT_EQ(shift.blue(), 0);
    EXPECT_FLOAT_EQ(shift.percent(), 0.0f);
    EXPECT_FALSE(shift.isActive());
}

TEST(ColorShiftTest, ParameterizedConstruction) {
    ColorShift shift(255, 0, 0, 100.0f);
    
    EXPECT_EQ(shift.red(), 255);
    EXPECT_EQ(shift.green(), 0);
    EXPECT_EQ(shift.blue(), 0);
    EXPECT_FLOAT_EQ(shift.percent(), 100.0f);
    EXPECT_TRUE(shift.isActive());
}

TEST(ColorShiftTest, Decay) {
    ColorShift shift(255, 0, 0, 100.0f);
    
    shift.decay(25.0f);
    EXPECT_FLOAT_EQ(shift.percent(), 75.0f);
    EXPECT_TRUE(shift.isActive());
    
    shift.decay(100.0f);
    EXPECT_FLOAT_EQ(shift.percent(), 0.0f);
    EXPECT_FALSE(shift.isActive());
}

//=============================================================================
// DynamicLight Tests
//=============================================================================

TEST(DynamicLightTest, DefaultConstruction) {
    DynamicLight light;
    
    EXPECT_EQ(light.key(), 0);
    EXPECT_FLOAT_EQ(light.radius(), 0.0f);
    EXPECT_EQ(light.type(), client::DynamicLightType::Default);
}

TEST(DynamicLightTest, Properties) {
    DynamicLight light;
    
    light.setKey(42);
    light.setOrigin(Vec3{100.0f, 200.0f, 50.0f});
    light.setRadius(300.0f);
    light.setDecay(100.0f);
    light.setDieTime(5.0f);
    light.setType(client::DynamicLightType::Explosion);
    light.setColor(255, 200, 100);
    
    EXPECT_EQ(light.key(), 42);
    EXPECT_FLOAT_EQ(light.origin().x(), 100.0f);
    EXPECT_FLOAT_EQ(light.radius(), 300.0f);
    EXPECT_EQ(light.type(), client::DynamicLightType::Explosion);
    EXPECT_EQ(light.colorRed(), 255);
    EXPECT_EQ(light.colorGreen(), 200);
    EXPECT_EQ(light.colorBlue(), 100);
}

TEST(DynamicLightTest, Update) {
    DynamicLight light;
    light.setRadius(300.0f);
    light.setDecay(100.0f);
    
    light.update(1.0f, 0.0f);  // 1 second
    EXPECT_FLOAT_EQ(light.radius(), 200.0f);
    
    light.update(2.0f, 0.0f);  // 2 more seconds
    EXPECT_FLOAT_EQ(light.radius(), 0.0f);  // Clamped at 0
}

TEST(DynamicLightTest, IsActive) {
    DynamicLight light;
    light.setRadius(100.0f);
    light.setDieTime(5.0f);
    
    EXPECT_TRUE(light.isActive(0.0f));
    EXPECT_TRUE(light.isActive(4.9f));
    EXPECT_FALSE(light.isActive(5.1f));
    
    light.setDieTime(0.0f);  // Never dies
    EXPECT_TRUE(light.isActive(1000.0f));
}

//=============================================================================
// StaticSound Tests
//=============================================================================

TEST(StaticSoundTest, DefaultConstruction) {
    StaticSound sound;
    
    EXPECT_EQ(sound.soundNum(), 0);
    EXPECT_EQ(sound.volume(), 0);
    EXPECT_EQ(sound.attenuation(), 0);
}

TEST(StaticSoundTest, ParameterizedConstruction) {
    StaticSound sound(Vec3{100.0f, 200.0f, 50.0f}, 5, 255, 1);
    
    EXPECT_FLOAT_EQ(sound.origin().x(), 100.0f);
    EXPECT_EQ(sound.soundNum(), 5);
    EXPECT_EQ(sound.volume(), 255);
    EXPECT_EQ(sound.attenuation(), 1);
}

//=============================================================================
// LightStyle Tests
//=============================================================================

TEST(LightStyleTest, DefaultConstruction) {
    LightStyle style;
    
    EXPECT_TRUE(style.empty());
    EXPECT_EQ(style.length(), 0);
    EXPECT_EQ(style.valueAt(0.0), 255);  // Empty pattern = full brightness
}

TEST(LightStyleTest, SetPattern) {
    LightStyle style;
    style.setPattern("mmmaaazzzaaammmzzz");
    
    EXPECT_FALSE(style.empty());
    EXPECT_EQ(style.pattern(), "mmmaaazzzaaammmzzz");
}

TEST(LightStyleTest, ValueAtTime) {
    LightStyle style;
    style.setPattern("az");  // 'a' = 0, 'z' = 25*22 = 550 (clamped in practice)
    
    // Pattern repeats every 0.2 seconds (2 chars * 0.1s per char)
    EXPECT_EQ(style.valueAt(0.0), 0);    // 'a'
    EXPECT_EQ(style.valueAt(0.1), 550);  // 'z'
}

//=============================================================================
// PlayerState Tests
//=============================================================================

TEST(PlayerStateTest, DefaultConstruction) {
    PlayerState state;
    
    EXPECT_EQ(state.messageNum(), 0);
    EXPECT_DOUBLE_EQ(state.stateTime(), 0.0);
    EXPECT_FALSE(state.onGround());
    EXPECT_FALSE(state.isDead());
    EXPECT_FALSE(state.isSpectator());
    EXPECT_EQ(state.alpha(), 255);
}

TEST(PlayerStateTest, SetOriginVelocity) {
    PlayerState state;
    
    state.setOrigin(Vec3{100.0f, 200.0f, 50.0f});
    state.setVelocity(Vec3{300.0f, 0.0f, -100.0f});
    
    EXPECT_FLOAT_EQ(state.origin().x(), 100.0f);
    EXPECT_FLOAT_EQ(state.origin().y(), 200.0f);
    EXPECT_FLOAT_EQ(state.velocity().x(), 300.0f);
    EXPECT_FLOAT_EQ(state.velocity().z(), -100.0f);
}

TEST(PlayerStateTest, MoveType) {
    PlayerState state;
    
    state.setMoveType(PlayerState::MoveType::Dead);
    EXPECT_TRUE(state.isDead());
    EXPECT_FALSE(state.isSpectator());
    
    state.setMoveType(PlayerState::MoveType::Spectator);
    EXPECT_FALSE(state.isDead());
    EXPECT_TRUE(state.isSpectator());
}

TEST(PlayerStateTest, GroundAndJump) {
    PlayerState state;
    
    state.setOnGround(true);
    state.setJumpHeld(true);
    state.setJumpMsec(50);
    
    EXPECT_TRUE(state.onGround());
    EXPECT_TRUE(state.jumpHeld());
    EXPECT_EQ(state.jumpMsec(), 50);
}

TEST(PlayerStateTest, Clear) {
    PlayerState state;
    state.setOrigin(Vec3{100.0f, 200.0f, 50.0f});
    state.setOnGround(true);
    state.setMoveType(PlayerState::MoveType::Fly);
    
    state.clear();
    
    EXPECT_FLOAT_EQ(state.origin().x(), 0.0f);
    EXPECT_FALSE(state.onGround());
    EXPECT_EQ(state.moveType(), PlayerState::MoveType::Normal);
}

//=============================================================================
// PlayerInfo Tests
//=============================================================================

TEST(PlayerInfoTest, DefaultConstruction) {
    PlayerInfo info;
    
    EXPECT_EQ(info.userId(), 0);
    EXPECT_TRUE(info.name().empty());
    EXPECT_EQ(info.frags(), 0);
    EXPECT_EQ(info.ping(), 0);
    EXPECT_FALSE(info.isSpectator());
}

TEST(PlayerInfoTest, SetName) {
    PlayerInfo info;
    
    info.setName("Player1");
    EXPECT_EQ(info.name(), "Player1");
    
    info.setTeam("red");
    EXPECT_EQ(info.team(), "red");
}

TEST(PlayerInfoTest, ScoreboardStats) {
    PlayerInfo info;
    
    info.setFrags(10);
    info.setPing(25);
    info.setPacketLoss(2);
    info.setEnterTime(123.5f);
    
    EXPECT_EQ(info.frags(), 10);
    EXPECT_EQ(info.ping(), 25);
    EXPECT_EQ(info.packetLoss(), 2);
    EXPECT_FLOAT_EQ(info.enterTime(), 123.5f);
}

TEST(PlayerInfoTest, Colors) {
    PlayerInfo info;
    
    info.setTopColor(4);
    info.setBottomColor(13);
    info.setRealTopColor(1);
    info.setRealBottomColor(2);
    
    EXPECT_EQ(info.topColor(), 4);
    EXPECT_EQ(info.bottomColor(), 13);
    EXPECT_EQ(info.realTopColor(), 1);
    EXPECT_EQ(info.realBottomColor(), 2);
}

TEST(PlayerInfoTest, Stats) {
    PlayerInfo info;
    
    info.setStat(0, 100);   // STAT_HEALTH
    info.setStat(3, 50);    // STAT_AMMO
    
    EXPECT_EQ(info.stat(0), 100);
    EXPECT_EQ(info.stat(3), 50);
    EXPECT_EQ(info.stat(99), 0);  // Out of bounds
}

TEST(PlayerInfoTest, Clear) {
    PlayerInfo info;
    info.setName("Player1");
    info.setFrags(10);
    info.setDead(true);
    
    info.clear();
    
    EXPECT_TRUE(info.name().empty());
    EXPECT_EQ(info.frags(), 0);
    EXPECT_FALSE(info.isDead());
}

//=============================================================================
// EntityState Tests
//=============================================================================

TEST(EntityStateTest, DefaultConstruction) {
    EntityState state;
    
    EXPECT_EQ(state.number(), 0);
    EXPECT_EQ(state.modelIndex(), 0);
    EXPECT_EQ(state.frame(), 0);
}

TEST(EntityStateTest, Properties) {
    EntityState state;
    
    state.setNumber(42);
    state.setOrigin(Vec3{100.0f, 200.0f, 50.0f});
    state.setAngles(Vec3{0.0f, 90.0f, 0.0f});
    state.setModelIndex(5);
    state.setFrame(10);
    state.setEffects(0x08);
    
    EXPECT_EQ(state.number(), 42);
    EXPECT_FLOAT_EQ(state.origin().x(), 100.0f);
    EXPECT_FLOAT_EQ(state.angles().y(), 90.0f);
    EXPECT_EQ(state.modelIndex(), 5);
    EXPECT_EQ(state.frame(), 10);
    EXPECT_EQ(state.effects(), 0x08);
}

//=============================================================================
// CEntity Tests
//=============================================================================

TEST(CEntityTest, DefaultConstruction) {
    CEntity entity;
    
    EXPECT_EQ(entity.sequence(), 0);
    EXPECT_DOUBLE_EQ(entity.startLerp(), 0.0);
}

TEST(CEntityTest, BaselineAndCurrent) {
    CEntity entity;
    
    EntityState baseline;
    baseline.setNumber(1);
    baseline.setModelIndex(5);
    entity.setBaseline(baseline);
    
    EntityState current;
    current.setNumber(1);
    current.setOrigin(Vec3{100.0f, 0.0f, 0.0f});
    entity.setCurrent(current);
    
    EXPECT_EQ(entity.baseline().modelIndex(), 5);
    EXPECT_FLOAT_EQ(entity.current().origin().x(), 100.0f);
}

TEST(CEntityTest, Interpolation) {
    CEntity entity;
    
    entity.setOldOrigin(Vec3{0.0f, 0.0f, 0.0f});
    
    EntityState current;
    current.setOrigin(Vec3{100.0f, 0.0f, 0.0f});
    entity.setCurrent(current);
    
    entity.setStartLerp(0.0);
    entity.setDeltaLerp(1.0);  // 1 second lerp
    
    entity.interpolate(0.5);  // Half way
    EXPECT_FLOAT_EQ(entity.lerpOrigin().x(), 50.0f);
    
    entity.interpolate(1.0);  // End
    EXPECT_FLOAT_EQ(entity.lerpOrigin().x(), 100.0f);
}

//=============================================================================
// PacketEntities Tests
//=============================================================================

TEST(PacketEntitiesTest, DefaultConstruction) {
    PacketEntities entities;
    
    EXPECT_TRUE(entities.empty());
    EXPECT_EQ(entities.count(), 0);
}

TEST(PacketEntitiesTest, AddAndFind) {
    PacketEntities entities;
    
    EntityState state1;
    state1.setNumber(1);
    state1.setModelIndex(5);
    entities.add(state1);
    
    EntityState state2;
    state2.setNumber(5);
    state2.setModelIndex(10);
    entities.add(state2);
    
    EXPECT_EQ(entities.count(), 2);
    
    const EntityState* found = entities.find(5);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->modelIndex(), 10);
    
    EXPECT_EQ(entities.find(99), nullptr);
}

TEST(PacketEntitiesTest, Clear) {
    PacketEntities entities;
    
    EntityState state;
    state.setNumber(1);
    entities.add(state);
    
    entities.clear();
    EXPECT_TRUE(entities.empty());
}

//=============================================================================
// Frame Tests
//=============================================================================

TEST(FrameTest, DefaultConstruction) {
    Frame frame;
    
    EXPECT_DOUBLE_EQ(frame.sentTime(), 0.0);
    EXPECT_DOUBLE_EQ(frame.receivedTime(), -1.0);
    EXPECT_TRUE(frame.isValid());
    EXPECT_LT(frame.rtt(), 0.0);  // Not received yet
}

TEST(FrameTest, Timing) {
    Frame frame;
    
    frame.setSentTime(100.0);
    frame.setReceivedTime(100.05);
    
    EXPECT_DOUBLE_EQ(frame.sentTime(), 100.0);
    EXPECT_DOUBLE_EQ(frame.receivedTime(), 100.05);
    EXPECT_NEAR(frame.rtt(), 0.05, 0.0001);
}

TEST(FrameTest, Command) {
    Frame frame;
    
    UserCommand cmd;
    cmd.setMsec(100);
    cmd.setForwardMove(400);
    frame.setCommand(cmd);
    
    EXPECT_EQ(frame.command().msec(), 100);
    EXPECT_EQ(frame.command().forwardMove(), 400);
}

TEST(FrameTest, PlayerStates) {
    Frame frame;
    
    frame.playerState(0).setOrigin(Vec3{100.0f, 0.0f, 0.0f});
    frame.playerState(1).setOrigin(Vec3{200.0f, 0.0f, 0.0f});
    
    EXPECT_FLOAT_EQ(frame.playerState(0).origin().x(), 100.0f);
    EXPECT_FLOAT_EQ(frame.playerState(1).origin().x(), 200.0f);
}

TEST(FrameTest, PacketEntities) {
    Frame frame;
    
    EntityState state;
    state.setNumber(42);
    frame.packetEntities().add(state);
    
    EXPECT_EQ(frame.packetEntities().count(), 1);
}

TEST(FrameTest, Validity) {
    Frame frame;
    
    EXPECT_TRUE(frame.isValid());
    
    frame.setInvalid(true);
    EXPECT_FALSE(frame.isValid());
    EXPECT_TRUE(frame.isInvalid());
}

//=============================================================================
// FrameBuffer Tests
//=============================================================================

TEST(FrameBufferTest, Size) {
    FrameBuffer buffer;
    
    EXPECT_EQ(buffer.size(), client::UPDATE_BACKUP);
}

TEST(FrameBufferTest, SequenceAccess) {
    FrameBuffer buffer;
    
    // Access by sequence number (wraps around)
    buffer[0].setSentTime(1.0);
    buffer[64].setSentTime(2.0);  // Same as [0] if UPDATE_BACKUP is 64
    
    EXPECT_DOUBLE_EQ(buffer[0].sentTime(), 2.0);  // Overwritten
}

TEST(FrameBufferTest, Clear) {
    FrameBuffer buffer;
    
    buffer[0].setSentTime(100.0);
    buffer.clear();
    
    EXPECT_DOUBLE_EQ(buffer[0].sentTime(), 0.0);
}

//=============================================================================
// Button Flags Tests
//=============================================================================

TEST(ButtonFlagsTest, BitwiseOr) {
    client::Button buttons = client::Button::Attack | client::Button::Jump;
    
    EXPECT_TRUE(client::hasButton(buttons, client::Button::Attack));
    EXPECT_TRUE(client::hasButton(buttons, client::Button::Jump));
    EXPECT_FALSE(client::hasButton(buttons, client::Button::Use));
}

TEST(ButtonFlagsTest, BitwiseAnd) {
    client::Button buttons = client::Button::Attack | client::Button::Jump | client::Button::Use;
    client::Button masked = buttons & client::Button::Jump;
    
    EXPECT_EQ(masked, client::Button::Jump);
}

//=============================================================================
// ClientPersistent Tests
//=============================================================================

#include "core/client/client_persistent.hpp"

TEST(DownloadStateTest, DefaultState) {
    DownloadState download;
    
    EXPECT_FALSE(download.isActive());
    EXPECT_EQ(download.type(), client::DownloadType::None);
    EXPECT_EQ(download.percent(), 0);
}

TEST(DownloadStateTest, StartDownload) {
    DownloadState download;
    
    download.start(client::DownloadType::Model, "progs/player.mdl", 0.0);
    
    EXPECT_TRUE(download.isActive());
    EXPECT_EQ(download.type(), client::DownloadType::Model);
    EXPECT_EQ(download.name(), "progs/player.mdl");
    EXPECT_EQ(download.percent(), 0);
}

TEST(DownloadStateTest, Progress) {
    DownloadState download;
    download.start(client::DownloadType::Model, "test.mdl", 0.0);
    
    download.updateProgress(50, 10000);
    EXPECT_EQ(download.percent(), 50);
    
    download.updateProgress(75, 10000);
    EXPECT_EQ(download.percent(), 75);
}

TEST(DownloadStateTest, Complete) {
    DownloadState download;
    download.start(client::DownloadType::Model, "test.mdl", 0.0);
    download.updateProgress(100, 0);
    download.finish();
    
    EXPECT_FALSE(download.isActive());
    EXPECT_EQ(download.type(), client::DownloadType::None);
}

TEST(DemoStateTest, DefaultState) {
    DemoState demo;
    
    EXPECT_FALSE(demo.isRecording());
    EXPECT_FALSE(demo.isPlaying());
    EXPECT_FALSE(demo.isSeeking());
}

TEST(DemoStateTest, Recording) {
    DemoState demo;
    
    demo.startRecording("mydemo.mvd");
    
    EXPECT_TRUE(demo.isRecording());
    EXPECT_FALSE(demo.isPlaying());
    EXPECT_EQ(demo.name(), "mydemo.mvd");
}

TEST(DemoStateTest, Playing) {
    DemoState demo;
    
    demo.startPlayback("replay.mvd");
    
    EXPECT_FALSE(demo.isRecording());
    EXPECT_TRUE(demo.isPlaying());
    EXPECT_EQ(demo.name(), "replay.mvd");
}

TEST(DemoStateTest, Seeking) {
    DemoState demo;
    demo.startPlayback("replay.mvd");
    
    demo.startSeeking(client::DemoSeekType::Normal);
    
    EXPECT_TRUE(demo.isSeeking());
    EXPECT_EQ(demo.seekType(), client::DemoSeekType::Normal);
    
    demo.stopSeeking();
    EXPECT_FALSE(demo.isSeeking());
}

TEST(PerformanceStatsTest, FpsTracking) {
    PerformanceStats stats;
    
    // Update with 60 fps
    stats.updateFps(60.0);
    
    EXPECT_DOUBLE_EQ(stats.fps(), 60.0);
}

TEST(PerformanceStatsTest, Latency) {
    PerformanceStats stats;
    
    stats.updateLatency(50.0f);
    
    EXPECT_FLOAT_EQ(stats.latency(), 50.0f);
}

TEST(QtvStateTest, ServerVersion) {
    QtvState qtv;
    
    EXPECT_FLOAT_EQ(qtv.serverVersion(), 0.0f);
    
    qtv.setServerVersion(1.5f);
    
    EXPECT_FLOAT_EQ(qtv.serverVersion(), 1.5f);
}

TEST(QtvStateTest, Source) {
    QtvState qtv;
    
    qtv.setSource("qtv.example.com:12345");
    
    EXPECT_EQ(qtv.source(), "qtv.example.com:12345");
}

TEST(ClientPersistentTest, DefaultState) {
    ClientPersistent persistent;
    
    EXPECT_EQ(persistent.state(), client::ConnectionState::Disconnected);
    EXPECT_FALSE(persistent.download().isActive());
    EXPECT_FALSE(persistent.demo().isRecording());
}

TEST(ClientPersistentTest, ConnectionCycle) {
    ClientPersistent persistent;
    
    persistent.setState(client::ConnectionState::Connected);
    EXPECT_EQ(persistent.state(), client::ConnectionState::Connected);
    
    persistent.setState(client::ConnectionState::Active);
    EXPECT_EQ(persistent.state(), client::ConnectionState::Active);
    
    persistent.setState(client::ConnectionState::Disconnected);
    EXPECT_EQ(persistent.state(), client::ConnectionState::Disconnected);
}

TEST(ClientPersistentTest, Clear) {
    ClientPersistent persistent;
    
    persistent.setState(client::ConnectionState::Active);
    persistent.download().start(client::DownloadType::Model, "test.mdl", 1000);
    
    persistent.clear();
    
    EXPECT_EQ(persistent.state(), client::ConnectionState::Disconnected);
    EXPECT_FALSE(persistent.download().isActive());
}

//=============================================================================
// ClientState Tests
//=============================================================================

#include "core/client/client_state.hpp"

TEST(ServerInfoTest, DefaultState) {
    ServerInfo info;
    
    EXPECT_EQ(info.serverCount(), 0);
    EXPECT_EQ(info.protoVersion(), 0);
    EXPECT_EQ(info.deathmatch(), 0);
    EXPECT_EQ(info.teamplay(), 0);
}

TEST(ServerInfoTest, Setters) {
    ServerInfo info;
    
    info.setServerCount(42);
    info.setProtoVersion(28);
    info.setDeathmatch(1);
    info.setTeamplay(2);
    info.setGameType(client::GameType::Deathmatch);
    info.setTimelimit(20);
    info.setFraglimit(50);
    
    EXPECT_EQ(info.serverCount(), 42);
    EXPECT_EQ(info.protoVersion(), 28);
    EXPECT_EQ(info.deathmatch(), 1);
    EXPECT_EQ(info.teamplay(), 2);
    EXPECT_EQ(info.gameType(), client::GameType::Deathmatch);
    EXPECT_EQ(info.timelimit(), 20);
    EXPECT_EQ(info.fraglimit(), 50);
}

TEST(SequenceStateTest, ParseCount) {
    SequenceState seq;
    
    EXPECT_EQ(seq.parseCount(), 0);
    EXPECT_FALSE(seq.canRender());
    
    seq.setParseCount(100, 5.0);
    
    EXPECT_EQ(seq.parseCount(), 100);
    EXPECT_EQ(seq.oldParseCount(), 0);
    EXPECT_EQ(seq.parseCountMod(), 100 % client::UPDATE_BACKUP);
    EXPECT_DOUBLE_EQ(seq.parseCountTime(), 5.0);
}

TEST(SequenceStateTest, ValidSequence) {
    SequenceState seq;
    
    seq.setValidSequence(50);
    EXPECT_TRUE(seq.canRender());
    EXPECT_EQ(seq.validSequence(), 50);
    
    seq.setValidSequence(51);
    EXPECT_EQ(seq.oldValidSequence(), 50);
    EXPECT_EQ(seq.validSequence(), 51);
}

TEST(ViewStateTest, Angles) {
    ViewState view;
    
    view.setViewAngles(Vec3{10.0f, 20.0f, 0.0f});
    
    EXPECT_FLOAT_EQ(view.viewAngles().x(), 10.0f);
    EXPECT_FLOAT_EQ(view.viewAngles().y(), 20.0f);
}

TEST(ViewStateTest, SimulatedState) {
    ViewState view;
    
    view.setSimOrg(Vec3{100.0f, 200.0f, 50.0f});
    view.setSimVel(Vec3{300.0f, 0.0f, 0.0f});
    view.setOnGround(true);
    
    EXPECT_FLOAT_EQ(view.simOrg().x(), 100.0f);
    EXPECT_FLOAT_EQ(view.simVel().x(), 300.0f);
    EXPECT_TRUE(view.onGround());
}

TEST(ViewStateTest, ColorShift) {
    ViewState view;
    
    view.colorShift(client::ColorShiftType::Damage).setColor(255, 0, 0);
    view.colorShift(client::ColorShiftType::Damage).setPercent(128.0f);
    
    EXPECT_EQ(view.colorShift(client::ColorShiftType::Damage).red(), 255);
    EXPECT_TRUE(view.colorShift(client::ColorShiftType::Damage).isActive());
}

TEST(MovementParamsTest, Defaults) {
    MovementParams params;
    
    EXPECT_FLOAT_EQ(params.gravity(), 800.0f);
    EXPECT_FLOAT_EQ(params.maxSpeed(), 320.0f);
    EXPECT_FLOAT_EQ(params.bunnySpeedCap(), 0.0f);
}

TEST(MovementParamsTest, Setters) {
    MovementParams params;
    
    params.setGravity(600.0f);
    params.setMaxSpeed(400.0f);
    params.setBunnySpeedCap(550.0f);
    
    EXPECT_FLOAT_EQ(params.gravity(), 600.0f);
    EXPECT_FLOAT_EQ(params.maxSpeed(), 400.0f);
    EXPECT_FLOAT_EQ(params.bunnySpeedCap(), 550.0f);
}

TEST(SpectatorStateTest, DefaultState) {
    SpectatorState spec;
    
    EXPECT_FALSE(spec.isSpectator());
    EXPECT_EQ(spec.cameraMode(), client::CameraMode::None);
    EXPECT_FALSE(spec.isTracking());
}

TEST(SpectatorStateTest, Tracking) {
    SpectatorState spec;
    
    spec.setSpectator(true);
    spec.setCameraMode(client::CameraMode::Track);
    spec.setTrackedPlayer(5);
    
    EXPECT_TRUE(spec.isSpectator());
    EXPECT_TRUE(spec.isTracking());
    EXPECT_EQ(spec.trackedPlayer(), 5);
}

TEST(GameStateFlagsTest, Pause) {
    GameStateFlags flags;
    
    EXPECT_FALSE(flags.isPaused());
    
    flags.addPauseFlag(GameStateFlags::PAUSED_SERVER);
    EXPECT_TRUE(flags.isPaused());
    EXPECT_TRUE(flags.isServerPaused());
    EXPECT_FALSE(flags.isDemoPaused());
    
    flags.addPauseFlag(GameStateFlags::PAUSED_DEMO);
    EXPECT_TRUE(flags.isDemoPaused());
    
    flags.removePauseFlag(GameStateFlags::PAUSED_SERVER);
    EXPECT_TRUE(flags.isPaused());  // Still paused by demo
    EXPECT_FALSE(flags.isServerPaused());
}

TEST(GameStateFlagsTest, Intermission) {
    GameStateFlags flags;
    
    EXPECT_EQ(flags.intermission(), 0);
    
    flags.setIntermission(1);
    EXPECT_EQ(flags.intermission(), 1);
}

TEST(ClientStateTest, DefaultState) {
    ClientState state;
    
    EXPECT_DOUBLE_EQ(state.time(), 0.0);
    EXPECT_EQ(state.playerNum(), 0);
    EXPECT_TRUE(state.levelName().empty());
}

TEST(ClientStateTest, Timing) {
    ClientState state;
    
    state.setTime(100.5);
    state.setServerTime(100.0);
    state.setGameTime(50.0);
    
    EXPECT_DOUBLE_EQ(state.time(), 100.5);
    EXPECT_DOUBLE_EQ(state.serverTime(), 100.0);
    EXPECT_DOUBLE_EQ(state.gameTime(), 50.0);
}

TEST(ClientStateTest, PlayerNum) {
    ClientState state;
    
    state.setPlayerNum(5);
    state.setViewPlayerNum(10);
    
    EXPECT_EQ(state.playerNum(), 5);
    EXPECT_EQ(state.viewPlayerNum(), 10);
}

TEST(ClientStateTest, LevelInfo) {
    ClientState state;
    
    state.setLevelName("e1m1");
    state.setCdTrack(4);
    state.setMapChecksum(0x12345678);
    
    EXPECT_EQ(state.levelName(), "e1m1");
    EXPECT_EQ(state.cdTrack(), 4);
    EXPECT_EQ(state.mapChecksum(), 0x12345678);
}

TEST(ClientStateTest, Stats) {
    ClientState state;
    
    state.setStat(0, 100);  // Health
    state.setStat(1, 50);   // Armor
    state.setStat(2, 7);    // Weapons
    
    EXPECT_EQ(state.stat(0), 100);
    EXPECT_EQ(state.stat(1), 50);
    EXPECT_EQ(state.stat(2), 7);
    EXPECT_EQ(state.stat(99), 0);  // Out of bounds returns 0
}

TEST(ClientStateTest, SubStateAccess) {
    ClientState state;
    
    state.serverInfo().setServerCount(1);
    state.sequence().setValidSequence(100);
    state.view().setOnGround(true);
    state.movement().setMaxSpeed(400.0f);
    state.spectator().setSpectator(true);
    state.gameState().setIntermission(1);
    
    EXPECT_EQ(state.serverInfo().serverCount(), 1);
    EXPECT_EQ(state.sequence().validSequence(), 100);
    EXPECT_TRUE(state.view().onGround());
    EXPECT_FLOAT_EQ(state.movement().maxSpeed(), 400.0f);
    EXPECT_TRUE(state.spectator().isSpectator());
    EXPECT_EQ(state.gameState().intermission(), 1);
}

TEST(ClientStateTest, Players) {
    ClientState state;
    
    state.player(0).setName("Player1");
    state.player(0).setFrags(10);
    state.player(1).setName("Player2");
    state.player(1).setFrags(5);
    
    EXPECT_EQ(state.player(0).name(), "Player1");
    EXPECT_EQ(state.player(0).frags(), 10);
    EXPECT_EQ(state.player(1).name(), "Player2");
    EXPECT_EQ(state.player(1).frags(), 5);
}

TEST(ClientStateTest, StaticSounds) {
    ClientState state;
    
    EXPECT_EQ(state.numStaticSounds(), 0);
    
    state.addStaticSound(StaticSound{Vec3{100.0f, 0.0f, 0.0f}, 1, 255, 1});
    state.addStaticSound(StaticSound{Vec3{200.0f, 0.0f, 0.0f}, 2, 128, 2});
    
    EXPECT_EQ(state.numStaticSounds(), 2);
    EXPECT_FLOAT_EQ(state.staticSound(0).origin().x(), 100.0f);
    EXPECT_EQ(state.staticSound(0).soundNum(), 1);
}

TEST(ClientStateTest, LightStyles) {
    ClientState state;
    
    state.setLightStyle(0, "mmaaaamm");
    
    EXPECT_EQ(state.lightStyle(0).pattern(), "mmaaaamm");
    EXPECT_FALSE(state.lightStyle(0).empty());
}

TEST(ClientStateTest, DynamicLights) {
    ClientState state;
    
    auto* light = state.allocDynamicLight(1);
    EXPECT_NE(light, nullptr);
    
    light->setOrigin(Vec3{100.0f, 0.0f, 0.0f});
    light->setRadius(200.0f);
    
    EXPECT_EQ(light->key(), 1);
    EXPECT_FLOAT_EQ(light->radius(), 200.0f);
    
    // Same key should return same light
    auto* light2 = state.allocDynamicLight(1);
    EXPECT_EQ(light2, light);
}

TEST(ClientStateTest, Clear) {
    ClientState state;
    
    state.setTime(100.0);
    state.setPlayerNum(5);
    state.setLevelName("e1m1");
    state.setStat(0, 100);
    state.serverInfo().setServerCount(1);
    state.addStaticSound(StaticSound{Vec3{}, 1, 255, 1});
    
    state.clear();
    
    EXPECT_DOUBLE_EQ(state.time(), 0.0);
    EXPECT_EQ(state.playerNum(), 0);
    EXPECT_TRUE(state.levelName().empty());
    EXPECT_EQ(state.stat(0), 0);
    EXPECT_EQ(state.serverInfo().serverCount(), 0);
    EXPECT_EQ(state.numStaticSounds(), 0);
}
