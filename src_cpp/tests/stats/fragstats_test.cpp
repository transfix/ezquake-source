/**
 * @file fragstats_test.cpp
 * @brief Unit tests for frag statistics types
 */

#include <gtest/gtest.h>
#include "../../core/stats/fragstats_types.hpp"

using namespace ezquake::fragstats;

// ============================================================================
// FragMessageType Tests
// ============================================================================

TEST(FragMessageTypeTest, MessageTypeName) {
    EXPECT_EQ(messageTypeName(FragMessageType::Fragged), "fragged");
    EXPECT_EQ(messageTypeName(FragMessageType::Frags), "frags");
    EXPECT_EQ(messageTypeName(FragMessageType::Suicide), "suicide");
    EXPECT_EQ(messageTypeName(FragMessageType::FlagCapture), "flagcap");
}

TEST(FragMessageTypeTest, IsKillEvent) {
    EXPECT_TRUE(isKillEvent(FragMessageType::Frags));
    EXPECT_TRUE(isKillEvent(FragMessageType::Frag));
    EXPECT_TRUE(isKillEvent(FragMessageType::TeamKill));
    EXPECT_FALSE(isKillEvent(FragMessageType::Fragged));
    EXPECT_FALSE(isKillEvent(FragMessageType::Suicide));
    EXPECT_FALSE(isKillEvent(FragMessageType::FlagTouch));
}

TEST(FragMessageTypeTest, IsDeathEvent) {
    EXPECT_TRUE(isDeathEvent(FragMessageType::Fragged));
    EXPECT_TRUE(isDeathEvent(FragMessageType::Death));
    EXPECT_TRUE(isDeathEvent(FragMessageType::Suicide));
    EXPECT_TRUE(isDeathEvent(FragMessageType::TeamKilled));
    EXPECT_FALSE(isDeathEvent(FragMessageType::Frags));
    EXPECT_FALSE(isDeathEvent(FragMessageType::FlagCapture));
}

TEST(FragMessageTypeTest, IsFlagEvent) {
    EXPECT_TRUE(isFlagEvent(FragMessageType::FlagTouch));
    EXPECT_TRUE(isFlagEvent(FragMessageType::FlagDrop));
    EXPECT_TRUE(isFlagEvent(FragMessageType::FlagCapture));
    EXPECT_FALSE(isFlagEvent(FragMessageType::Frag));
    EXPECT_FALSE(isFlagEvent(FragMessageType::Suicide));
}

// ============================================================================
// WeaponClass Tests
// ============================================================================

TEST(WeaponClassTest, WeaponClassName) {
    EXPECT_EQ(weaponClassName(WeaponClass::Unknown), "unknown");
    EXPECT_EQ(weaponClassName(WeaponClass::Shotgun), "sg");
    EXPECT_EQ(weaponClassName(WeaponClass::RocketLauncher), "rl");
    EXPECT_EQ(weaponClassName(WeaponClass::LightningGun), "lg");
}

TEST(WeaponClassTest, WeaponClassFullName) {
    EXPECT_EQ(weaponClassFullName(WeaponClass::Shotgun), "Shotgun");
    EXPECT_EQ(weaponClassFullName(WeaponClass::RocketLauncher), "Rocket Launcher");
    EXPECT_EQ(weaponClassFullName(WeaponClass::Telefrag), "Telefrag");
}

TEST(WeaponClassTest, IsActualWeapon) {
    EXPECT_TRUE(isActualWeapon(WeaponClass::Axe));
    EXPECT_TRUE(isActualWeapon(WeaponClass::RocketLauncher));
    EXPECT_TRUE(isActualWeapon(WeaponClass::LightningGun));
    EXPECT_FALSE(isActualWeapon(WeaponClass::Unknown));
    EXPECT_FALSE(isActualWeapon(WeaponClass::Telefrag));
    EXPECT_FALSE(isActualWeapon(WeaponClass::Lava));
}

// ============================================================================
// WeaponClassInfo Tests
// ============================================================================

TEST(WeaponClassInfoTest, IsValid) {
    WeaponClassInfo info;
    EXPECT_FALSE(info.isValid());

    info.keyword = "rl";
    EXPECT_FALSE(info.isValid());

    info.name = "Rocket Launcher";
    EXPECT_TRUE(info.isValid());
}

TEST(WeaponClassInfoTest, DisplayName) {
    WeaponClassInfo info;
    info.name = "Rocket Launcher";
    info.shortName = "rl";

    EXPECT_EQ(info.displayName(false), "Rocket Launcher");
    EXPECT_EQ(info.displayName(true), "rl");
}

TEST(WeaponClassInfoTest, DisplayNameNoShort) {
    WeaponClassInfo info;
    info.name = "Rocket Launcher";

    EXPECT_EQ(info.displayName(true), "Rocket Launcher");
}

// ============================================================================
// FragMessage Tests
// ============================================================================

TEST(FragMessageTest, IsValid) {
    FragMessage msg;
    EXPECT_FALSE(msg.isValid());

    msg.message1 = "was fragged by";
    EXPECT_TRUE(msg.isValid());
}

// ============================================================================
// FragfileDefinition Tests
// ============================================================================

TEST(FragfileDefinitionTest, Reset) {
    FragfileDefinition def;
    def.active = true;
    def.title = "Test Fragfile";
    def.messages.push_back(FragMessage{});

    def.reset();

    EXPECT_FALSE(def.active);
    EXPECT_TRUE(def.title.empty());
    EXPECT_TRUE(def.messages.empty());
    EXPECT_EQ(def.weaponClasses.size(), 1u);  // Default "Unknown" class
}

TEST(FragfileDefinitionTest, GetWeaponClass) {
    FragfileDefinition def;
    def.reset();

    // Index 0 should be "Unknown"
    auto* wc = def.getWeaponClass(0);
    ASSERT_NE(wc, nullptr);
    EXPECT_EQ(wc->name, "Unknown");

    // Invalid index
    EXPECT_EQ(def.getWeaponClass(-1), nullptr);
    EXPECT_EQ(def.getWeaponClass(100), nullptr);
}

TEST(FragfileDefinitionTest, FindWeaponClass) {
    FragfileDefinition def;
    def.reset();

    WeaponClassInfo rl;
    rl.keyword = "rl";
    rl.name = "Rocket Launcher";
    def.weaponClasses.push_back(rl);

    auto idx = def.findWeaponClass("rl");
    ASSERT_TRUE(idx.has_value());
    EXPECT_EQ(*idx, 1);

    EXPECT_FALSE(def.findWeaponClass("nonexistent").has_value());
}

// ============================================================================
// WeaponFragStats Tests
// ============================================================================

TEST(WeaponFragStatsTest, DefaultValues) {
    WeaponFragStats stats;
    EXPECT_EQ(stats.kills, 0);
    EXPECT_EQ(stats.deaths, 0);
    EXPECT_EQ(stats.teamKills, 0);
}

TEST(WeaponFragStatsTest, NetFrags) {
    WeaponFragStats stats;
    stats.kills = 20;
    stats.teamKills = 2;
    stats.suicides = 3;
    EXPECT_EQ(stats.netFrags(), 15);  // 20 - 2 - 3
}

TEST(WeaponFragStatsTest, KillDeathRatio) {
    WeaponFragStats stats;
    stats.kills = 20;
    stats.deaths = 10;
    EXPECT_FLOAT_EQ(stats.killDeathRatio(), 2.0f);
}

TEST(WeaponFragStatsTest, KillDeathRatioZeroDeaths) {
    WeaponFragStats stats;
    stats.kills = 10;
    stats.deaths = 0;
    EXPECT_FLOAT_EQ(stats.killDeathRatio(), 10.0f);
}

TEST(WeaponFragStatsTest, Reset) {
    WeaponFragStats stats;
    stats.kills = 10;
    stats.deaths = 5;
    stats.reset();
    EXPECT_EQ(stats.kills, 0);
    EXPECT_EQ(stats.deaths, 0);
}

TEST(WeaponFragStatsTest, Add) {
    WeaponFragStats stats1;
    stats1.kills = 10;
    stats1.deaths = 5;

    WeaponFragStats stats2;
    stats2.kills = 5;
    stats2.deaths = 3;

    stats1.add(stats2);
    EXPECT_EQ(stats1.kills, 15);
    EXPECT_EQ(stats1.deaths, 8);
}

// ============================================================================
// PlayerFragStats Tests
// ============================================================================

TEST(PlayerFragStatsTest, TotalKills) {
    PlayerFragStats stats;
    stats.getWeaponStats(WeaponClass::RocketLauncher).kills = 10;
    stats.getWeaponStats(WeaponClass::LightningGun).kills = 5;
    EXPECT_EQ(stats.totalKills(), 15);
}

TEST(PlayerFragStatsTest, TotalDeaths) {
    PlayerFragStats stats;
    stats.getWeaponStats(WeaponClass::RocketLauncher).deaths = 5;
    stats.getWeaponStats(WeaponClass::LightningGun).deaths = 3;
    stats.environmentDeaths = 2;
    EXPECT_EQ(stats.totalDeaths(), 10);
}

TEST(PlayerFragStatsTest, RecordKill) {
    PlayerFragStats stats;
    stats.recordKill(WeaponClass::RocketLauncher);
    stats.recordKill(WeaponClass::RocketLauncher);
    stats.recordKill(WeaponClass::RocketLauncher, true);  // team kill

    EXPECT_EQ(stats.getWeaponStats(WeaponClass::RocketLauncher).kills, 2);
    EXPECT_EQ(stats.getWeaponStats(WeaponClass::RocketLauncher).teamKills, 1);
}

TEST(PlayerFragStatsTest, RecordDeath) {
    PlayerFragStats stats;
    stats.recordDeath(WeaponClass::RocketLauncher);
    stats.recordDeath(WeaponClass::RocketLauncher, true);  // team death
    stats.recordDeath(WeaponClass::RocketLauncher, false, true);  // suicide

    EXPECT_EQ(stats.getWeaponStats(WeaponClass::RocketLauncher).deaths, 1);
    EXPECT_EQ(stats.getWeaponStats(WeaponClass::RocketLauncher).teamDeaths, 1);
    EXPECT_EQ(stats.getWeaponStats(WeaponClass::RocketLauncher).suicides, 1);
}

TEST(PlayerFragStatsTest, CombinedStats) {
    PlayerFragStats stats;
    stats.getWeaponStats(WeaponClass::RocketLauncher).kills = 10;
    stats.getWeaponStats(WeaponClass::LightningGun).kills = 5;
    stats.getWeaponStats(WeaponClass::RocketLauncher).deaths = 3;

    auto combined = stats.combinedStats();
    EXPECT_EQ(combined.kills, 15);
    EXPECT_EQ(combined.deaths, 3);
}

TEST(PlayerFragStatsTest, Reset) {
    PlayerFragStats stats;
    stats.recordKill(WeaponClass::RocketLauncher);
    stats.flagCaptures = 3;
    stats.reset();
    EXPECT_EQ(stats.totalKills(), 0);
    EXPECT_EQ(stats.flagCaptures, 0);
}

// ============================================================================
// Quad Markers Tests
// ============================================================================

TEST(QuadMarkersTest, HasQuadMarker) {
    EXPECT_TRUE(hasQuadMarker("&c06fQ&r-rl"));
    EXPECT_TRUE(hasQuadMarker("&c06fQ&r-lg"));
    EXPECT_FALSE(hasQuadMarker("rl"));
    EXPECT_FALSE(hasQuadMarker("lg"));
}

// ============================================================================
// FragEvent Tests
// ============================================================================

TEST(FragEventTest, IsSuicide) {
    FragEvent event;
    event.type = FragMessageType::Suicide;
    EXPECT_TRUE(event.isSuicide());

    FragEvent event2;
    event2.killerPlayerId = 5;
    event2.victimPlayerId = 5;
    EXPECT_TRUE(event2.isSuicide());

    FragEvent event3;
    event3.killerPlayerId = 5;
    event3.victimPlayerId = 3;
    EXPECT_FALSE(event3.isSuicide());
}

TEST(FragEventTest, IsValid) {
    FragEvent event;
    EXPECT_FALSE(event.isValid());

    event.killerPlayerId = 0;
    EXPECT_TRUE(event.isValid());
}

// ============================================================================
// FragLog Tests
// ============================================================================

TEST(FragLogTest, AddEvent) {
    FragLog log;
    log.maxEvents = 3;

    FragEvent e1, e2, e3, e4;
    e1.killerPlayerId = 1;
    e2.killerPlayerId = 2;
    e3.killerPlayerId = 3;
    e4.killerPlayerId = 4;

    log.addEvent(e1);
    log.addEvent(e2);
    log.addEvent(e3);
    EXPECT_EQ(log.size(), 3u);

    log.addEvent(e4);  // Should evict oldest
    EXPECT_EQ(log.size(), 3u);
    EXPECT_EQ(log.events[0].killerPlayerId, 2);  // e1 was evicted
}

TEST(FragLogTest, GetEventsForPlayer) {
    FragLog log;

    FragEvent e1;
    e1.killerPlayerId = 1;
    e1.victimPlayerId = 2;

    FragEvent e2;
    e2.killerPlayerId = 3;
    e2.victimPlayerId = 1;

    FragEvent e3;
    e3.killerPlayerId = 4;
    e3.victimPlayerId = 5;

    log.addEvent(e1);
    log.addEvent(e2);
    log.addEvent(e3);

    auto player1Events = log.getEventsForPlayer(1);
    EXPECT_EQ(player1Events.size(), 2u);  // e1 (killer) and e2 (victim)

    auto player5Events = log.getEventsForPlayer(5);
    EXPECT_EQ(player5Events.size(), 1u);
}

TEST(FragLogTest, GetKillEvents) {
    FragLog log;

    FragEvent kill;
    kill.type = FragMessageType::Frag;
    kill.killerPlayerId = 1;

    FragEvent death;
    death.type = FragMessageType::Death;
    death.victimPlayerId = 2;

    FragEvent flag;
    flag.type = FragMessageType::FlagCapture;
    flag.killerPlayerId = 3;

    log.addEvent(kill);
    log.addEvent(death);
    log.addEvent(flag);

    auto kills = log.getKillEvents();
    EXPECT_EQ(kills.size(), 1u);
}

TEST(FragLogTest, Clear) {
    FragLog log;

    FragEvent e;
    e.killerPlayerId = 1;
    log.addEvent(e);
    log.addEvent(e);

    log.clear();
    EXPECT_EQ(log.size(), 0u);
}

// ============================================================================
// FragParserState Tests
// ============================================================================

TEST(FragParserStateTest, IsReady) {
    FragParserState parser;
    EXPECT_FALSE(parser.isReady());

    FragfileDefinition def;
    def.active = true;
    parser.fragfile = &def;
    parser.enabled = true;

    EXPECT_TRUE(parser.isReady());
}

TEST(FragParserStateTest, NotReadyWhenDisabled) {
    FragParserState parser;
    FragfileDefinition def;
    def.active = true;
    parser.fragfile = &def;
    parser.enabled = false;

    EXPECT_FALSE(parser.isReady());
}

// ============================================================================
// Factory Function Tests
// ============================================================================

TEST(FactoryTest, CreateDefaultWeaponClasses) {
    auto classes = createDefaultWeaponClasses();
    EXPECT_EQ(classes.size(), static_cast<size_t>(WeaponClass::Count));

    // Check specific entries
    EXPECT_EQ(classes[static_cast<size_t>(WeaponClass::Unknown)].keyword, "unknown");
    EXPECT_EQ(classes[static_cast<size_t>(WeaponClass::RocketLauncher)].keyword, "rl");
    EXPECT_EQ(classes[static_cast<size_t>(WeaponClass::RocketLauncher)].name, "Rocket Launcher");
}
