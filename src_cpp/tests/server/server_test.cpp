/**
 * @file server_test.cpp
 * @brief Tests for server type definitions
 */

#include <gtest/gtest.h>
#include "core/server/server_types.hpp"

using namespace ezquake::server;

// =============================================================================
// ServerState Tests
// =============================================================================

TEST(ServerStateTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(ServerState::Dead), 0);
    EXPECT_EQ(static_cast<uint8_t>(ServerState::Loading), 1);
    EXPECT_EQ(static_cast<uint8_t>(ServerState::Active), 2);
}

TEST(ServerStateTest, Names) {
    EXPECT_EQ(serverStateName(ServerState::Dead), "dead");
    EXPECT_EQ(serverStateName(ServerState::Loading), "loading");
    EXPECT_EQ(serverStateName(ServerState::Active), "active");
}

TEST(ServerStateTest, IsServerRunning) {
    EXPECT_FALSE(isServerRunning(ServerState::Dead));
    EXPECT_FALSE(isServerRunning(ServerState::Loading));
    EXPECT_TRUE(isServerRunning(ServerState::Active));
}

TEST(ServerStateTest, CanPrecache) {
    EXPECT_FALSE(canPrecache(ServerState::Dead));
    EXPECT_TRUE(canPrecache(ServerState::Loading));
    EXPECT_FALSE(canPrecache(ServerState::Active));
}

// =============================================================================
// ClientState Tests
// =============================================================================

TEST(ClientStateTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(ClientState::Free), 0);
    EXPECT_EQ(static_cast<uint8_t>(ClientState::Zombie), 1);
    EXPECT_EQ(static_cast<uint8_t>(ClientState::Preconnected), 2);
    EXPECT_EQ(static_cast<uint8_t>(ClientState::Connected), 3);
    EXPECT_EQ(static_cast<uint8_t>(ClientState::Spawned), 4);
}

TEST(ClientStateTest, Names) {
    EXPECT_EQ(clientStateName(ClientState::Free), "free");
    EXPECT_EQ(clientStateName(ClientState::Zombie), "zombie");
    EXPECT_EQ(clientStateName(ClientState::Preconnected), "preconnected");
    EXPECT_EQ(clientStateName(ClientState::Connected), "connected");
    EXPECT_EQ(clientStateName(ClientState::Spawned), "spawned");
}

TEST(ClientStateTest, IsClientActive) {
    EXPECT_FALSE(isClientActive(ClientState::Free));
    EXPECT_FALSE(isClientActive(ClientState::Zombie));
    EXPECT_FALSE(isClientActive(ClientState::Preconnected));
    EXPECT_FALSE(isClientActive(ClientState::Connected));
    EXPECT_TRUE(isClientActive(ClientState::Spawned));
}

TEST(ClientStateTest, IsClientConnected) {
    EXPECT_FALSE(isClientConnected(ClientState::Free));
    EXPECT_FALSE(isClientConnected(ClientState::Zombie));
    EXPECT_FALSE(isClientConnected(ClientState::Preconnected));
    EXPECT_TRUE(isClientConnected(ClientState::Connected));
    EXPECT_TRUE(isClientConnected(ClientState::Spawned));
}

TEST(ClientStateTest, CanReuse) {
    EXPECT_TRUE(canReuse(ClientState::Free));
    EXPECT_FALSE(canReuse(ClientState::Zombie));
    EXPECT_FALSE(canReuse(ClientState::Spawned));
}

// =============================================================================
// MoveType Tests
// =============================================================================

TEST(MoveTypeTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(MoveType::None), 0);
    EXPECT_EQ(static_cast<uint8_t>(MoveType::Walk), 3);
    EXPECT_EQ(static_cast<uint8_t>(MoveType::Step), 4);
    EXPECT_EQ(static_cast<uint8_t>(MoveType::Fly), 5);
    EXPECT_EQ(static_cast<uint8_t>(MoveType::Toss), 6);
    EXPECT_EQ(static_cast<uint8_t>(MoveType::Push), 7);
    EXPECT_EQ(static_cast<uint8_t>(MoveType::NoClip), 8);
    EXPECT_EQ(static_cast<uint8_t>(MoveType::FlyMissile), 9);
    EXPECT_EQ(static_cast<uint8_t>(MoveType::Bounce), 10);
    EXPECT_EQ(static_cast<uint8_t>(MoveType::Lock), 15);
}

TEST(MoveTypeTest, Names) {
    EXPECT_EQ(moveTypeName(MoveType::None), "none");
    EXPECT_EQ(moveTypeName(MoveType::Walk), "walk");
    EXPECT_EQ(moveTypeName(MoveType::NoClip), "noclip");
    EXPECT_EQ(moveTypeName(MoveType::FlyMissile), "flymissile");
}

TEST(MoveTypeTest, AffectedByGravity) {
    EXPECT_FALSE(affectedByGravity(MoveType::None));
    EXPECT_FALSE(affectedByGravity(MoveType::Fly));
    EXPECT_FALSE(affectedByGravity(MoveType::NoClip));
    EXPECT_TRUE(affectedByGravity(MoveType::Walk));
    EXPECT_TRUE(affectedByGravity(MoveType::Step));
    EXPECT_TRUE(affectedByGravity(MoveType::Toss));
    EXPECT_TRUE(affectedByGravity(MoveType::Bounce));
}

TEST(MoveTypeTest, IsProjectileType) {
    EXPECT_FALSE(isProjectileType(MoveType::None));
    EXPECT_FALSE(isProjectileType(MoveType::Walk));
    EXPECT_TRUE(isProjectileType(MoveType::Toss));
    EXPECT_TRUE(isProjectileType(MoveType::FlyMissile));
    EXPECT_TRUE(isProjectileType(MoveType::Bounce));
}

// =============================================================================
// SolidType Tests
// =============================================================================

TEST(SolidTypeTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(SolidType::Not), 0);
    EXPECT_EQ(static_cast<uint8_t>(SolidType::Trigger), 1);
    EXPECT_EQ(static_cast<uint8_t>(SolidType::BBox), 2);
    EXPECT_EQ(static_cast<uint8_t>(SolidType::SlideBox), 3);
    EXPECT_EQ(static_cast<uint8_t>(SolidType::BSP), 4);
}

TEST(SolidTypeTest, Names) {
    EXPECT_EQ(solidTypeName(SolidType::Not), "not");
    EXPECT_EQ(solidTypeName(SolidType::Trigger), "trigger");
    EXPECT_EQ(solidTypeName(SolidType::BBox), "bbox");
    EXPECT_EQ(solidTypeName(SolidType::BSP), "bsp");
}

TEST(SolidTypeTest, IsBlocking) {
    EXPECT_FALSE(isBlocking(SolidType::Not));
    EXPECT_FALSE(isBlocking(SolidType::Trigger));
    EXPECT_TRUE(isBlocking(SolidType::BBox));
    EXPECT_FALSE(isBlocking(SolidType::SlideBox));
    EXPECT_TRUE(isBlocking(SolidType::BSP));
}

TEST(SolidTypeTest, IsTouchable) {
    EXPECT_FALSE(isTouchable(SolidType::Not));
    EXPECT_TRUE(isTouchable(SolidType::Trigger));
    EXPECT_TRUE(isTouchable(SolidType::BBox));
    EXPECT_TRUE(isTouchable(SolidType::BSP));
}

// =============================================================================
// EntityFlags Tests
// =============================================================================

TEST(EntityFlagsTest, Values) {
    EXPECT_EQ(EntityFlags::Fly, 1);
    EXPECT_EQ(EntityFlags::Swim, 2);
    EXPECT_EQ(EntityFlags::Client, 8);
    EXPECT_EQ(EntityFlags::Monster, 32);
    EXPECT_EQ(EntityFlags::OnGround, 512);
    EXPECT_EQ(EntityFlags::LaggedMove, 1 << 16);
}

TEST(EntityFlagsTest, HasFlag) {
    uint32_t flags = EntityFlags::Client | EntityFlags::OnGround;
    EXPECT_TRUE(EntityFlags::hasFlag(flags, EntityFlags::Client));
    EXPECT_TRUE(EntityFlags::hasFlag(flags, EntityFlags::OnGround));
    EXPECT_FALSE(EntityFlags::hasFlag(flags, EntityFlags::Monster));
}

TEST(EntityFlagsTest, IsPlayer) {
    EXPECT_TRUE(EntityFlags::isPlayer(EntityFlags::Client));
    EXPECT_TRUE(EntityFlags::isPlayer(EntityFlags::Client | EntityFlags::OnGround));
    EXPECT_FALSE(EntityFlags::isPlayer(EntityFlags::Monster));
}

TEST(EntityFlagsTest, CanFly) {
    EXPECT_TRUE(EntityFlags::canFly(EntityFlags::Fly));
    EXPECT_TRUE(EntityFlags::canFly(EntityFlags::Swim));
    EXPECT_TRUE(EntityFlags::canFly(EntityFlags::Fly | EntityFlags::Monster));
    EXPECT_FALSE(EntityFlags::canFly(EntityFlags::Client));
}

// =============================================================================
// SpawnFlags Tests
// =============================================================================

TEST(SpawnFlagsTest, Values) {
    EXPECT_EQ(SpawnFlags::NotEasy, 256);
    EXPECT_EQ(SpawnFlags::NotMedium, 512);
    EXPECT_EQ(SpawnFlags::NotHard, 1024);
    EXPECT_EQ(SpawnFlags::NotDeathmatch, 2048);
}

TEST(SpawnFlagsTest, ShouldSpawn) {
    // No flags - always spawn
    EXPECT_TRUE(SpawnFlags::shouldSpawn(0, 0, false));
    EXPECT_TRUE(SpawnFlags::shouldSpawn(0, 1, false));
    EXPECT_TRUE(SpawnFlags::shouldSpawn(0, 2, true));
    
    // Not easy - don't spawn on easy
    EXPECT_FALSE(SpawnFlags::shouldSpawn(SpawnFlags::NotEasy, 0, false));
    EXPECT_TRUE(SpawnFlags::shouldSpawn(SpawnFlags::NotEasy, 1, false));
    
    // Not medium - don't spawn on medium
    EXPECT_TRUE(SpawnFlags::shouldSpawn(SpawnFlags::NotMedium, 0, false));
    EXPECT_FALSE(SpawnFlags::shouldSpawn(SpawnFlags::NotMedium, 1, false));
    
    // Not deathmatch
    EXPECT_TRUE(SpawnFlags::shouldSpawn(SpawnFlags::NotDeathmatch, 0, false));
    EXPECT_FALSE(SpawnFlags::shouldSpawn(SpawnFlags::NotDeathmatch, 0, true));
}

// =============================================================================
// DamageType Tests
// =============================================================================

TEST(DamageTypeTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(DamageType::No), 0);
    EXPECT_EQ(static_cast<uint8_t>(DamageType::Yes), 1);
    EXPECT_EQ(static_cast<uint8_t>(DamageType::Aim), 2);
}

TEST(DamageTypeTest, CanBeDamaged) {
    EXPECT_FALSE(canBeDamaged(DamageType::No));
    EXPECT_TRUE(canBeDamaged(DamageType::Yes));
    EXPECT_TRUE(canBeDamaged(DamageType::Aim));
}

// =============================================================================
// MulticastType Tests
// =============================================================================

TEST(MulticastTypeTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(MulticastType::All), 0);
    EXPECT_EQ(static_cast<uint8_t>(MulticastType::PHS), 1);
    EXPECT_EQ(static_cast<uint8_t>(MulticastType::PVS), 2);
    EXPECT_EQ(static_cast<uint8_t>(MulticastType::AllReliable), 3);
    EXPECT_EQ(static_cast<uint8_t>(MulticastType::PHSReliable), 4);
    EXPECT_EQ(static_cast<uint8_t>(MulticastType::PVSReliable), 5);
}

TEST(MulticastTypeTest, Names) {
    EXPECT_EQ(multicastTypeName(MulticastType::All), "all");
    EXPECT_EQ(multicastTypeName(MulticastType::PVS), "pvs");
    EXPECT_EQ(multicastTypeName(MulticastType::AllReliable), "all_reliable");
}

TEST(MulticastTypeTest, IsReliable) {
    EXPECT_FALSE(isReliable(MulticastType::All));
    EXPECT_FALSE(isReliable(MulticastType::PVS));
    EXPECT_TRUE(isReliable(MulticastType::AllReliable));
    EXPECT_TRUE(isReliable(MulticastType::PHSReliable));
    EXPECT_TRUE(isReliable(MulticastType::PVSReliable));
}

TEST(MulticastTypeTest, UsesVisibility) {
    EXPECT_FALSE(usesVisibility(MulticastType::All));
    EXPECT_TRUE(usesVisibility(MulticastType::PVS));
    EXPECT_TRUE(usesVisibility(MulticastType::PHS));
    EXPECT_FALSE(usesVisibility(MulticastType::AllReliable));
    EXPECT_TRUE(usesVisibility(MulticastType::PVSReliable));
}

// =============================================================================
// IpFilterType Tests
// =============================================================================

TEST(IpFilterTypeTest, Names) {
    EXPECT_EQ(ipFilterTypeName(IpFilterType::Ban), "ban");
    EXPECT_EQ(ipFilterTypeName(IpFilterType::Safe), "safe");
}

// =============================================================================
// PenaltyType Tests
// =============================================================================

TEST(PenaltyTypeTest, Names) {
    EXPECT_EQ(penaltyTypeName(PenaltyType::Mute), "mute");
    EXPECT_EQ(penaltyTypeName(PenaltyType::Cuff), "cuff");
}

// =============================================================================
// RedirectType Tests
// =============================================================================

TEST(RedirectTypeTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(RedirectType::None), 0);
    EXPECT_EQ(static_cast<uint8_t>(RedirectType::Client), 1);
    EXPECT_EQ(static_cast<uint8_t>(RedirectType::Packet), 2);
    EXPECT_EQ(static_cast<uint8_t>(RedirectType::Mod), 3);
}

TEST(RedirectTypeTest, Names) {
    EXPECT_EQ(redirectTypeName(RedirectType::None), "none");
    EXPECT_EQ(redirectTypeName(RedirectType::Client), "client");
    EXPECT_EQ(redirectTypeName(RedirectType::Packet), "packet");
    EXPECT_EQ(redirectTypeName(RedirectType::Mod), "mod");
}

// =============================================================================
// DemoDestType Tests
// =============================================================================

TEST(DemoDestTypeTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(DemoDestType::None), 0);
    EXPECT_EQ(static_cast<uint8_t>(DemoDestType::File), 1);
    EXPECT_EQ(static_cast<uint8_t>(DemoDestType::BufferedFile), 2);
    EXPECT_EQ(static_cast<uint8_t>(DemoDestType::Stream), 3);
}

TEST(DemoDestTypeTest, IsFileBased) {
    EXPECT_FALSE(isFileBased(DemoDestType::None));
    EXPECT_TRUE(isFileBased(DemoDestType::File));
    EXPECT_TRUE(isFileBased(DemoDestType::BufferedFile));
    EXPECT_FALSE(isFileBased(DemoDestType::Stream));
}

// =============================================================================
// VoiceTarget Tests
// =============================================================================

TEST(VoiceTargetTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(VoiceTarget::Team), 0);
    EXPECT_EQ(static_cast<uint8_t>(VoiceTarget::All), 1);
    EXPECT_EQ(static_cast<uint8_t>(VoiceTarget::NonMuted), 2);
    EXPECT_EQ(static_cast<uint8_t>(VoiceTarget::PlayerSlot0), 3);
}

TEST(VoiceTargetTest, Names) {
    EXPECT_EQ(voiceTargetName(VoiceTarget::Team), "team");
    EXPECT_EQ(voiceTargetName(VoiceTarget::All), "all");
    EXPECT_EQ(voiceTargetName(VoiceTarget::NonMuted), "nonmuted");
}

// =============================================================================
// SpecPrint Tests
// =============================================================================

TEST(SpecPrintTest, Values) {
    EXPECT_EQ(SpecPrint::CenterPrint, 0x1);
    EXPECT_EQ(SpecPrint::Sprint, 0x2);
    EXPECT_EQ(SpecPrint::StuffCmd, 0x4);
}

// =============================================================================
// Server Constants Tests
// =============================================================================

TEST(ServerConstantsTest, Values) {
    EXPECT_EQ(server_constants::MAX_MASTERS, 8);
    EXPECT_EQ(server_constants::MAX_CLIENTS, 32);
    EXPECT_EQ(server_constants::MAX_MODELS, 512);
    EXPECT_EQ(server_constants::MAX_SOUNDS, 256);
    EXPECT_EQ(server_constants::MAX_EDICTS, 2048);
    EXPECT_EQ(server_constants::MAX_CHALLENGES, 1024);
    EXPECT_FLOAT_EQ(server_constants::STEPSIZE, 18.0f);
    EXPECT_EQ(server_constants::NUM_SPAWN_PARMS, 16);
}

TEST(ServerConstantsTest, Flags) {
    EXPECT_EQ(server_constants::SVF_SPEC_ONFULL, 1);
    EXPECT_EQ(server_constants::SVF_NO_SPEC_ONFULL, 2);
}

// =============================================================================
// IpFilter Tests
// =============================================================================

TEST(IpFilterTest, Default) {
    IpFilter filter;
    EXPECT_EQ(filter.mask, 0u);
    EXPECT_EQ(filter.compare, 0u);
    EXPECT_EQ(filter.type, IpFilterType::Ban);
    EXPECT_TRUE(filter.isBan());
    EXPECT_FALSE(filter.isSafe());
}

TEST(IpFilterTest, Matches) {
    IpFilter filter;
    filter.mask = 0xFFFFFF00;   // Match first 3 bytes
    filter.compare = 0xC0A80100; // 192.168.1.x
    
    EXPECT_TRUE(filter.matches(0xC0A80100));   // 192.168.1.0
    EXPECT_TRUE(filter.matches(0xC0A801FF));   // 192.168.1.255
    EXPECT_FALSE(filter.matches(0xC0A80200));  // 192.168.2.0
}

TEST(IpFilterTest, IsExpired) {
    IpFilter filter;
    filter.expireTime = 100.0;
    
    EXPECT_FALSE(filter.isExpired(50.0));
    EXPECT_FALSE(filter.isExpired(99.9));
    EXPECT_TRUE(filter.isExpired(100.0));
    EXPECT_TRUE(filter.isExpired(150.0));
    
    // Zero expire time = never expires
    filter.expireTime = 0.0;
    EXPECT_FALSE(filter.isExpired(1000000.0));
}

// =============================================================================
// PenaltyFilter Tests
// =============================================================================

TEST(PenaltyFilterTest, Default) {
    PenaltyFilter filter;
    EXPECT_EQ(filter.ip[0], 0);
    EXPECT_EQ(filter.type, PenaltyType::Mute);
    EXPECT_TRUE(filter.isMute());
    EXPECT_FALSE(filter.isCuff());
}

TEST(PenaltyFilterTest, MatchesIp) {
    PenaltyFilter filter;
    filter.ip = {192, 168, 1, 1};
    
    EXPECT_TRUE(filter.matchesIp({192, 168, 1, 1}));
    EXPECT_FALSE(filter.matchesIp({192, 168, 1, 2}));
}

// =============================================================================
// Challenge Tests
// =============================================================================

TEST(ChallengeTest, Default) {
    Challenge challenge;
    EXPECT_FALSE(challenge.isValid());
    EXPECT_EQ(challenge.challenge, 0);
}

TEST(ChallengeTest, IsExpired) {
    Challenge challenge;
    challenge.time = 100;
    challenge.challenge = 12345;
    
    // 30 second timeout
    EXPECT_FALSE(challenge.isExpired(110, 30));
    EXPECT_FALSE(challenge.isExpired(130, 30));
    EXPECT_TRUE(challenge.isExpired(131, 30));
    EXPECT_TRUE(challenge.isExpired(200, 30));
}

TEST(ChallengeTest, Clear) {
    Challenge challenge;
    challenge.ip = {192, 168, 1, 1};
    challenge.challenge = 12345;
    challenge.time = 100;
    
    challenge.clear();
    
    EXPECT_FALSE(challenge.isValid());
    EXPECT_EQ(challenge.challenge, 0);
}

// =============================================================================
// AntilagPosition Tests
// =============================================================================

TEST(AntilagPositionTest, Default) {
    AntilagPosition pos;
    EXPECT_FALSE(pos.isValid());
    EXPECT_DOUBLE_EQ(pos.localtime, 0.0);
}

TEST(AntilagPositionTest, Valid) {
    AntilagPosition pos;
    pos.localtime = 10.0;
    
    EXPECT_TRUE(pos.isValid());
    EXPECT_DOUBLE_EQ(pos.localtime, 10.0);
}

// =============================================================================
// ClientFrame Tests
// =============================================================================

TEST(ClientFrameTest, Default) {
    ClientFrame frame;
    EXPECT_FALSE(frame.wasSent());
    EXPECT_DOUBLE_EQ(frame.sentTime, 0.0);
}

TEST(ClientFrameTest, Clear) {
    ClientFrame frame;
    frame.sentTime = 10.0;
    frame.pingTime = 50.0f;
    frame.svTime = 10.0;
    
    frame.clear();
    
    EXPECT_FALSE(frame.wasSent());
    EXPECT_FLOAT_EQ(frame.pingTime, 0.0f);
}

// =============================================================================
// DemoClientFrame Tests
// =============================================================================

TEST(DemoClientFrameTest, Default) {
    DemoClientFrame frame;
    EXPECT_EQ(frame.parsecount, 0);
    EXPECT_FALSE(frame.fixangle);
}

TEST(DemoClientFrameTest, Clear) {
    DemoClientFrame frame;
    frame.parsecount = 100;
    frame.model = 5;
    frame.fixangle = true;
    
    frame.clear();
    
    EXPECT_EQ(frame.parsecount, 0);
    EXPECT_EQ(frame.model, 0);
    EXPECT_FALSE(frame.fixangle);
}

// =============================================================================
// ServerStats Tests
// =============================================================================

TEST(ServerStatsTest, Default) {
    ServerStats stats;
    EXPECT_DOUBLE_EQ(stats.active, 0.0);
    EXPECT_EQ(stats.packets, 0);
}

TEST(ServerStatsTest, TotalTime) {
    ServerStats stats;
    stats.active = 100.0;
    stats.idle = 50.0;
    stats.demo = 25.0;
    
    EXPECT_DOUBLE_EQ(stats.totalTime(), 175.0);
}

TEST(ServerStatsTest, ActivePercent) {
    ServerStats stats;
    stats.active = 80.0;
    stats.idle = 20.0;
    
    EXPECT_DOUBLE_EQ(stats.activePercent(), 80.0);
}

TEST(ServerStatsTest, Latch) {
    ServerStats stats;
    stats.active = 100.0;
    stats.packets = 500;
    
    stats.latch();
    
    EXPECT_DOUBLE_EQ(stats.latchedActive, 100.0);
    EXPECT_EQ(stats.latchedPackets, 500);
}

TEST(ServerStatsTest, Reset) {
    ServerStats stats;
    stats.active = 100.0;
    stats.packets = 500;
    
    stats.reset();
    
    EXPECT_DOUBLE_EQ(stats.active, 0.0);
    EXPECT_EQ(stats.packets, 0);
    // Latched values unaffected
}

// =============================================================================
// DateTime Tests
// =============================================================================

TEST(DateTimeTest, Default) {
    DateTime dt;
    EXPECT_EQ(dt.year, 1970);
    EXPECT_EQ(dt.mon, 1);
    EXPECT_EQ(dt.day, 1);
}

TEST(DateTimeTest, Format) {
    DateTime dt;
    dt.year = 2025;
    dt.mon = 6;
    dt.day = 15;
    dt.hour = 10;
    dt.min = 30;
    dt.sec = 45;
    
    std::string result = dt.format();
    EXPECT_EQ(result, "2025-06-15 10:30:45");
}

// =============================================================================
// EntityBaseline Tests
// =============================================================================

TEST(EntityBaselineTest, Default) {
    EntityBaseline baseline;
    EXPECT_EQ(baseline.number, 0);
    EXPECT_EQ(baseline.modelindex, 0);
    EXPECT_FALSE(baseline.isPlayer());
}

TEST(EntityBaselineTest, IsPlayer) {
    EntityBaseline baseline;
    baseline.colormap = 1;  // Player 1
    EXPECT_TRUE(baseline.isPlayer());
    
    baseline.colormap = 32;  // Player 32
    EXPECT_TRUE(baseline.isPlayer());
    
    baseline.colormap = 33;  // Too high
    EXPECT_FALSE(baseline.isPlayer());
    
    baseline.colormap = 0;  // Not a player
    EXPECT_FALSE(baseline.isPlayer());
}

TEST(EntityBaselineTest, Clear) {
    EntityBaseline baseline;
    baseline.number = 10;
    baseline.modelindex = 5;
    baseline.colormap = 1;
    
    baseline.clear();
    
    EXPECT_EQ(baseline.number, 0);
    EXPECT_EQ(baseline.modelindex, 0);
    EXPECT_EQ(baseline.colormap, 0);
}

// =============================================================================
// ServerTime Tests
// =============================================================================

TEST(ServerTimeTest, Default) {
    ServerTime st;
    EXPECT_DOUBLE_EQ(st.time, 0.0);
    EXPECT_FALSE(st.paused);
}

TEST(ServerTimeTest, DeltaTime) {
    ServerTime st;
    st.time = 10.0;
    st.oldTime = 9.9;
    
    EXPECT_NEAR(st.deltaTime(), 0.1, 0.0001);
}

TEST(ServerTimeTest, Advance) {
    ServerTime st;
    st.time = 10.0;
    
    st.advance(0.1);
    
    EXPECT_DOUBLE_EQ(st.oldTime, 10.0);
    EXPECT_NEAR(st.time, 10.1, 0.0001);
}

TEST(ServerTimeTest, PauseDuration) {
    ServerTime st;
    st.paused = true;
    st.pausedSince = 100.0;
    
    EXPECT_DOUBLE_EQ(st.pauseDuration(105.0), 5.0);
    EXPECT_DOUBLE_EQ(st.pauseDuration(100.0), 0.0);
    
    st.paused = false;
    EXPECT_DOUBLE_EQ(st.pauseDuration(105.0), 0.0);
}
