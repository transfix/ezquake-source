/**
 * @file mvd_test.cpp
 * @brief Unit tests for MVD (Multi-View Demo) types
 */

#include <gtest/gtest.h>
#include "../../core/demo/mvd_types.hpp"

using namespace ezquake::mvd;

// ============================================================================
// MvdInfoType Tests
// ============================================================================

TEST(MvdInfoTypeTest, IsWeapon) {
    // Weapons: SSG through LG (indices 2-7)
    EXPECT_FALSE(isWeapon(MvdInfoType::Axe));
    EXPECT_FALSE(isWeapon(MvdInfoType::Shotgun));
    EXPECT_TRUE(isWeapon(MvdInfoType::SuperShotgun));
    EXPECT_TRUE(isWeapon(MvdInfoType::Nailgun));
    EXPECT_TRUE(isWeapon(MvdInfoType::SuperNailgun));
    EXPECT_TRUE(isWeapon(MvdInfoType::GrenadeLauncher));
    EXPECT_TRUE(isWeapon(MvdInfoType::RocketLauncher));
    EXPECT_TRUE(isWeapon(MvdInfoType::LightningGun));
    EXPECT_FALSE(isWeapon(MvdInfoType::Ring));
    EXPECT_FALSE(isWeapon(MvdInfoType::Quad));
}

TEST(MvdInfoTypeTest, IsPowerup) {
    EXPECT_FALSE(isPowerup(MvdInfoType::Axe));
    EXPECT_FALSE(isPowerup(MvdInfoType::RocketLauncher));
    EXPECT_TRUE(isPowerup(MvdInfoType::Ring));
    EXPECT_TRUE(isPowerup(MvdInfoType::Quad));
    EXPECT_TRUE(isPowerup(MvdInfoType::Pent));
    EXPECT_FALSE(isPowerup(MvdInfoType::RedArmor));
}

TEST(MvdInfoTypeTest, IsArmor) {
    EXPECT_FALSE(isArmor(MvdInfoType::Axe));
    EXPECT_FALSE(isArmor(MvdInfoType::Quad));
    EXPECT_TRUE(isArmor(MvdInfoType::GreenArmor));
    EXPECT_TRUE(isArmor(MvdInfoType::YellowArmor));
    EXPECT_TRUE(isArmor(MvdInfoType::RedArmor));
    EXPECT_FALSE(isArmor(MvdInfoType::MegaHealth));
}

// ============================================================================
// GameType Tests
// ============================================================================

TEST(GameTypeTest, GameTypeName) {
    EXPECT_EQ(gameTypeName(GameType::OneVsOne), "1on1");
    EXPECT_EQ(gameTypeName(GameType::TwoVsTwo), "2on2");
    EXPECT_EQ(gameTypeName(GameType::ThreeVsThree), "3on3");
    EXPECT_EQ(gameTypeName(GameType::FourVsFour), "4on4");
    EXPECT_EQ(gameTypeName(GameType::Unknown), "unknown");
}

TEST(GameTypeTest, PlayersPerTeam) {
    EXPECT_EQ(playersPerTeam(GameType::OneVsOne), 1);
    EXPECT_EQ(playersPerTeam(GameType::TwoVsTwo), 2);
    EXPECT_EQ(playersPerTeam(GameType::ThreeVsThree), 3);
    EXPECT_EQ(playersPerTeam(GameType::FourVsFour), 4);
    EXPECT_EQ(playersPerTeam(GameType::Unknown), 0);
}

TEST(GameTypeTest, GameTypeFromPlayerCount) {
    EXPECT_EQ(gameTypeFromPlayerCount(2), GameType::OneVsOne);
    EXPECT_EQ(gameTypeFromPlayerCount(4), GameType::TwoVsTwo);
    EXPECT_EQ(gameTypeFromPlayerCount(6), GameType::ThreeVsThree);
    EXPECT_EQ(gameTypeFromPlayerCount(8), GameType::FourVsFour);
    EXPECT_EQ(gameTypeFromPlayerCount(16), GameType::Unknown);
}

// ============================================================================
// MvdEventType Tests
// ============================================================================

TEST(MvdEventTypeTest, EventTypeName) {
    EXPECT_EQ(eventTypeName(MvdEventType::Spawn), "spawn");
    EXPECT_EQ(eventTypeName(MvdEventType::Death), "death");
    EXPECT_EQ(eventTypeName(MvdEventType::Kill), "kill");
    EXPECT_EQ(eventTypeName(MvdEventType::TeamKill), "teamkill");
    EXPECT_EQ(eventTypeName(MvdEventType::ItemTook), "took");
    EXPECT_EQ(eventTypeName(MvdEventType::PowerupEnd), "powerup_end");
}

// ============================================================================
// WeaponKillStats Tests
// ============================================================================

TEST(WeaponKillStatsTest, DefaultValues) {
    WeaponKillStats stats;
    EXPECT_EQ(stats.kills, 0);
    EXPECT_EQ(stats.teamKills, 0);
    EXPECT_EQ(stats.lastKills, 0);
    EXPECT_FALSE(stats.hasKills());
}

TEST(WeaponKillStatsTest, TotalKills) {
    WeaponKillStats stats;
    stats.kills = 10;
    stats.teamKills = 2;
    EXPECT_EQ(stats.totalKills(), 12);
    EXPECT_TRUE(stats.hasKills());
}

TEST(WeaponKillStatsTest, Reset) {
    WeaponKillStats stats;
    stats.kills = 10;
    stats.teamKills = 2;
    stats.lastKills = 5;
    stats.reset();
    EXPECT_EQ(stats.kills, 0);
    EXPECT_EQ(stats.teamKills, 0);
    EXPECT_EQ(stats.lastKills, 0);
}

// ============================================================================
// KillStats Tests
// ============================================================================

TEST(KillStatsTest, TotalKills) {
    KillStats stats;
    stats.normal[0].kills = 2;  // axe
    stats.normal[6].kills = 10; // rl
    stats.normal[7].kills = 5;  // lg
    EXPECT_EQ(stats.totalKills(), 17);
}

TEST(KillStatsTest, TotalTeamKills) {
    KillStats stats;
    stats.normal[6].teamKills = 1;
    stats.normal[7].teamKills = 2;
    EXPECT_EQ(stats.totalTeamKills(), 3);
}

TEST(KillStatsTest, Reset) {
    KillStats stats;
    stats.normal[0].kills = 5;
    stats.quad[0].kills = 3;
    stats.reset();
    EXPECT_EQ(stats.totalKills(), 0);
}

// ============================================================================
// DeathAliveStats Tests
// ============================================================================

TEST(DeathAliveStatsTest, SpawnAndDeath) {
    DeathAliveStats stats;
    EXPECT_FALSE(stats.isDead);
    EXPECT_EQ(stats.deathCount, 0);

    stats.onSpawn(0.0);
    EXPECT_FALSE(stats.isDead);

    stats.onDeath(30.0);
    EXPECT_TRUE(stats.isDead);
    EXPECT_EQ(stats.deathCount, 1);
    EXPECT_DOUBLE_EQ(stats.aliveTime, 30.0);
}

TEST(DeathAliveStatsTest, MultipleSpawnsDeath) {
    DeathAliveStats stats;
    stats.onSpawn(0.0);
    stats.onDeath(30.0);
    stats.onSpawn(35.0);
    stats.onDeath(65.0);

    EXPECT_EQ(stats.deathCount, 2);
    EXPECT_DOUBLE_EQ(stats.aliveTime, 60.0);  // 30 + 30
    EXPECT_DOUBLE_EQ(stats.averageLifeTime(), 30.0);
}

TEST(DeathAliveStatsTest, Reset) {
    DeathAliveStats stats;
    stats.onSpawn(0.0);
    stats.onDeath(30.0);
    stats.reset();
    EXPECT_FALSE(stats.isDead);
    EXPECT_EQ(stats.deathCount, 0);
    EXPECT_DOUBLE_EQ(stats.aliveTime, 0.0);
}

// ============================================================================
// RunStats Tests
// ============================================================================

TEST(RunStatsTest, Duration) {
    RunStats run;
    run.startTime = 10.0;
    run.time = 40.0;
    EXPECT_DOUBLE_EQ(run.duration(), 30.0);
}

TEST(RunStatsTest, FragsPerSecond) {
    RunStats run;
    run.startTime = 0.0;
    run.time = 60.0;
    run.frags = 6;
    EXPECT_DOUBLE_EQ(run.fragsPerSecond(), 0.1);
}

TEST(RunStatsTest, FragsPerSecondZeroDuration) {
    RunStats run;
    run.startTime = 10.0;
    run.time = 10.0;
    run.frags = 5;
    EXPECT_DOUBLE_EQ(run.fragsPerSecond(), 0.0);
}

// ============================================================================
// ItemPossessionStats Tests
// ============================================================================

TEST(ItemPossessionStatsTest, DefaultValues) {
    ItemPossessionStats stats;
    EXPECT_FALSE(stats.has);
    EXPECT_EQ(stats.count, 0);
    EXPECT_EQ(stats.lost, 0);
}

TEST(ItemPossessionStatsTest, AverageHoldTime) {
    ItemPossessionStats stats;
    stats.count = 4;
    stats.currentTime = 120.0;
    EXPECT_DOUBLE_EQ(stats.averageHoldTime(), 30.0);
}

TEST(ItemPossessionStatsTest, RetentionRate) {
    ItemPossessionStats stats;
    stats.count = 10;
    stats.lost = 3;
    EXPECT_DOUBLE_EQ(stats.retentionRate(), 0.7);
}

TEST(ItemPossessionStatsTest, RetentionRateZeroCount) {
    ItemPossessionStats stats;
    EXPECT_DOUBLE_EQ(stats.retentionRate(), 0.0);
}

// ============================================================================
// MvdEvent Tests
// ============================================================================

TEST(MvdEventTest, IsFragEvent) {
    MvdEvent killEvent;
    killEvent.type = MvdEventType::Kill;
    EXPECT_TRUE(killEvent.isFragEvent());

    MvdEvent tkEvent;
    tkEvent.type = MvdEventType::TeamKill;
    EXPECT_TRUE(tkEvent.isFragEvent());

    MvdEvent spawnEvent;
    spawnEvent.type = MvdEventType::Spawn;
    EXPECT_FALSE(spawnEvent.isFragEvent());
}

TEST(MvdEventTest, IsItemEvent) {
    MvdEvent tookEvent;
    tookEvent.type = MvdEventType::ItemTook;
    EXPECT_TRUE(tookEvent.isItemEvent());

    MvdEvent endEvent;
    endEvent.type = MvdEventType::PowerupEnd;
    EXPECT_TRUE(endEvent.isItemEvent());

    MvdEvent killEvent;
    killEvent.type = MvdEventType::Kill;
    EXPECT_FALSE(killEvent.isItemEvent());
}

// ============================================================================
// MvdPlayerStats Tests
// ============================================================================

TEST(MvdPlayerStatsTest, Efficiency) {
    MvdPlayerStats stats;
    stats.killStats.normal[6].kills = 20;  // 20 RL kills
    stats.deathAliveStats.deathCount = 10;
    EXPECT_DOUBLE_EQ(stats.efficiency(), 2.0);
}

TEST(MvdPlayerStatsTest, EfficiencyZeroDeaths) {
    MvdPlayerStats stats;
    stats.killStats.normal[6].kills = 20;
    EXPECT_DOUBLE_EQ(stats.efficiency(), 0.0);
}

TEST(MvdPlayerStatsTest, Reset) {
    MvdPlayerStats stats;
    stats.killStats.normal[6].kills = 20;
    stats.initialized = true;
    stats.reset();
    EXPECT_EQ(stats.totalFrags(), 0);
    EXPECT_FALSE(stats.initialized);
}

// ============================================================================
// MvdPlayerInfo Tests
// ============================================================================

TEST(MvdPlayerInfoTest, HasEvents) {
    MvdPlayerInfo info;
    EXPECT_FALSE(info.hasEvents());

    info.events.push_back(MvdEvent{});
    EXPECT_TRUE(info.hasEvents());
}

TEST(MvdPlayerInfoTest, LastEventOfType) {
    MvdPlayerInfo info;

    MvdEvent spawn1;
    spawn1.type = MvdEventType::Spawn;
    spawn1.time = 0.0;

    MvdEvent kill1;
    kill1.type = MvdEventType::Kill;
    kill1.time = 10.0;

    MvdEvent spawn2;
    spawn2.type = MvdEventType::Spawn;
    spawn2.time = 35.0;

    info.events.push_back(spawn1);
    info.events.push_back(kill1);
    info.events.push_back(spawn2);

    auto lastSpawn = info.lastEventOfType(MvdEventType::Spawn);
    ASSERT_TRUE(lastSpawn.has_value());
    EXPECT_DOUBLE_EQ(lastSpawn->time, 35.0);

    auto lastKill = info.lastEventOfType(MvdEventType::Kill);
    ASSERT_TRUE(lastKill.has_value());
    EXPECT_DOUBLE_EQ(lastKill->time, 10.0);

    auto lastDeath = info.lastEventOfType(MvdEventType::Death);
    EXPECT_FALSE(lastDeath.has_value());
}

TEST(MvdPlayerInfoTest, Reset) {
    MvdPlayerInfo info;
    info.id = 5;
    info.value = 100.0f;
    info.events.push_back(MvdEvent{});
    info.reset();
    EXPECT_EQ(info.id, -1);
    EXPECT_FLOAT_EQ(info.value, 0.0f);
    EXPECT_FALSE(info.hasEvents());
}

// ============================================================================
// MvdGameInfo Tests
// ============================================================================

TEST(MvdGameInfoTest, IsTeamGame) {
    MvdGameInfo info;
    info.gameType = GameType::OneVsOne;
    EXPECT_FALSE(info.isTeamGame());

    info.gameType = GameType::TwoVsTwo;
    EXPECT_TRUE(info.isTeamGame());

    info.gameType = GameType::FourVsFour;
    EXPECT_TRUE(info.isTeamGame());
}

TEST(MvdGameInfoTest, ExpectedPlayers) {
    MvdGameInfo info;
    info.gameType = GameType::OneVsOne;
    EXPECT_EQ(info.expectedPlayers(), 2);

    info.gameType = GameType::TwoVsTwo;
    EXPECT_EQ(info.expectedPlayers(), 4);

    info.gameType = GameType::FourVsFour;
    EXPECT_EQ(info.expectedPlayers(), 8);
}

TEST(MvdGameInfoTest, Reset) {
    MvdGameInfo info;
    info.mapName = "dm6";
    info.team1 = "Red";
    info.reset();
    EXPECT_TRUE(info.mapName.empty());
    EXPECT_TRUE(info.team1.empty());
}

// ============================================================================
// MvdWeaponInfo Tests
// ============================================================================

TEST(MvdWeaponInfoTest, ColorRGB) {
    MvdWeaponInfo info;
    info.colorR = 255;
    info.colorG = 128;
    info.colorB = 64;
    EXPECT_EQ(info.colorRGB(), 0xFF8040u);
}

TEST(MvdWeaponInfoTest, DefaultColor) {
    MvdWeaponInfo info;
    EXPECT_EQ(info.colorRGB(), 0xFFFFFFu);  // white
}

// ============================================================================
// AutotrackWeights Tests
// ============================================================================

TEST(AutotrackWeightsTest, WeaponValue) {
    AutotrackWeights weights;
    weights.axe = 1.0f;
    weights.lightningGun = 10.0f;

    EXPECT_FLOAT_EQ(weights.weaponValue(0), 1.0f);
    EXPECT_FLOAT_EQ(weights.weaponValue(7), 10.0f);
    EXPECT_FLOAT_EQ(weights.weaponValue(-1), 0.0f);
    EXPECT_FLOAT_EQ(weights.weaponValue(8), 0.0f);
}

TEST(AutotrackWeightsTest, ArmorValue) {
    AutotrackWeights weights;
    weights.greenArmor = 1.0f;
    weights.yellowArmor = 2.0f;
    weights.redArmor = 3.0f;

    EXPECT_FLOAT_EQ(weights.armorValue(0), 1.0f);
    EXPECT_FLOAT_EQ(weights.armorValue(1), 2.0f);
    EXPECT_FLOAT_EQ(weights.armorValue(2), 3.0f);
    EXPECT_FLOAT_EQ(weights.armorValue(3), 0.0f);
}

TEST(AutotrackWeightsTest, Create1on1) {
    auto weights = AutotrackWeights::create1on1();
    EXPECT_FLOAT_EQ(weights.axe, 1.0f);
    EXPECT_FLOAT_EQ(weights.rocketLauncher, 8.0f);
    EXPECT_FLOAT_EQ(weights.quad, 0.0f);  // 1v1 no powerup tracking
}

TEST(AutotrackWeightsTest, Create4on4) {
    auto weights = AutotrackWeights::create4on4();
    EXPECT_FLOAT_EQ(weights.rocketLauncher, 10.0f);
    EXPECT_FLOAT_EQ(weights.ring, 500.0f);  // 4v4 high powerup values
    EXPECT_FLOAT_EQ(weights.quad, 900.0f);
    EXPECT_FLOAT_EQ(weights.pent, 1000.0f);
}

// ============================================================================
// AutotrackConfig Tests
// ============================================================================

TEST(AutotrackConfigTest, CreateForGameType) {
    auto config1v1 = AutotrackConfig::createForGameType(GameType::OneVsOne);
    EXPECT_FLOAT_EQ(config1v1.weights.quad, 0.0f);

    auto config4v4 = AutotrackConfig::createForGameType(GameType::FourVsFour);
    EXPECT_FLOAT_EQ(config4v4.weights.ring, 500.0f);
    EXPECT_FLOAT_EQ(config4v4.weights.quad, 900.0f);
}

TEST(AutotrackConfigTest, DefaultFormula) {
    AutotrackConfig config;
    EXPECT_FALSE(config.formula.empty());
    EXPECT_NE(config.formula.find("%A"), std::string::npos);  // armor
}

// ============================================================================
// MultitrackConfig Tests
// ============================================================================

TEST(MultitrackConfigTest, DefaultFormulas) {
    MultitrackConfig config;
    EXPECT_EQ(config.formulas[0], "%f");
    EXPECT_EQ(config.formulas[1], "%W");
    EXPECT_EQ(config.formulas[2], "%h");
    EXPECT_EQ(config.formulas[3], "%A");
}

TEST(MultitrackConfigTest, ResetTracking) {
    MultitrackConfig config;
    config.trackedPlayerIds[0] = 1;
    config.trackedPlayerIds[1] = 5;
    config.resetTracking();
    for (size_t i = 0; i < 4; ++i) {
        EXPECT_EQ(config.trackedPlayerIds[i], -1);
    }
}

// ============================================================================
// ItemTrackingStats Tests
// ============================================================================

TEST(ItemTrackingStatsTest, Net) {
    ItemTrackingStats stats;
    stats.took = 10;
    stats.lost = 3;
    EXPECT_EQ(stats.net(), 7);
}

TEST(ItemTrackingStatsTest, Reset) {
    ItemTrackingStats stats;
    stats.took = 10;
    stats.lost = 3;
    stats.reset();
    EXPECT_EQ(stats.took, 0);
    EXPECT_EQ(stats.lost, 0);
}

// ============================================================================
// MvdState Tests
// ============================================================================

TEST(MvdStateTest, GetPlayer) {
    MvdState state;
    state.players[5].id = 5;
    state.players[5].value = 100.0f;

    auto* player = state.getPlayer(5);
    ASSERT_NE(player, nullptr);
    EXPECT_EQ(player->id, 5);
    EXPECT_FLOAT_EQ(player->value, 100.0f);
}

TEST(MvdStateTest, GetPlayerOutOfRange) {
    MvdState state;
    EXPECT_EQ(state.getPlayer(-1), nullptr);
    EXPECT_EQ(state.getPlayer(mvd_limits::MAX_CLIENTS), nullptr);
}

TEST(MvdStateTest, ConstGetPlayer) {
    MvdState state;
    state.players[5].id = 5;

    const MvdState& constState = state;
    auto* player = constState.getPlayer(5);
    ASSERT_NE(player, nullptr);
    EXPECT_EQ(player->id, 5);
}

TEST(MvdStateTest, Reset) {
    MvdState state;
    state.players[0].id = 0;
    state.players[0].value = 50.0f;
    state.gameInfo.mapName = "dm6";
    state.autotrackEnabled = true;
    state.currentTrackTarget = 3;

    state.reset();

    EXPECT_EQ(state.players[0].id, -1);
    EXPECT_FLOAT_EQ(state.players[0].value, 0.0f);
    EXPECT_TRUE(state.gameInfo.mapName.empty());
    EXPECT_FALSE(state.autotrackEnabled);
    EXPECT_EQ(state.currentTrackTarget, -1);
}

// ============================================================================
// AddClockMode Tests
// ============================================================================

TEST(AddClockModeTest, Values) {
    EXPECT_EQ(static_cast<int>(AddClockMode::Never), 0);
    EXPECT_EQ(static_cast<int>(AddClockMode::Always), 1);
    EXPECT_EQ(static_cast<int>(AddClockMode::DeathMatchMode1), 2);
}

// ============================================================================
// AverageRunStats Tests
// ============================================================================

TEST(AverageRunStatsTest, Reset) {
    AverageRunStats stats;
    stats.avgTime = 60.0;
    stats.avgFrags = 5.0;
    stats.avgTeamFrags = 2.0;
    stats.reset();
    EXPECT_DOUBLE_EQ(stats.avgTime, 0.0);
    EXPECT_DOUBLE_EQ(stats.avgFrags, 0.0);
    EXPECT_DOUBLE_EQ(stats.avgTeamFrags, 0.0);
}

TEST(AverageRunStatsAllTest, Reset) {
    AverageRunStatsAll stats;
    stats.all.avgTime = 60.0;
    stats.quad.avgFrags = 3.0;
    stats.reset();
    EXPECT_DOUBLE_EQ(stats.all.avgTime, 0.0);
    EXPECT_DOUBLE_EQ(stats.quad.avgFrags, 0.0);
}
