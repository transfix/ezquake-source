/*
 * ezQuake C++ Port - Browser Types Tests
 *
 * Unit tests for core/browser/browser_types.hpp
 */

#include <gtest/gtest.h>
#include "core/browser/browser_types.hpp"

using namespace ezquake;

//=============================================================================
// Server Occupancy Tests
//=============================================================================

TEST(ServerOccupancyTest, HasPlayers) {
    EXPECT_FALSE(hasPlayers(ServerOccupancy::Empty));
    EXPECT_TRUE(hasPlayers(ServerOccupancy::NotEmpty));
    EXPECT_TRUE(hasPlayers(ServerOccupancy::Full));
}

TEST(ServerOccupancyTest, HasRoom) {
    EXPECT_TRUE(hasRoom(ServerOccupancy::Empty));
    EXPECT_TRUE(hasRoom(ServerOccupancy::NotEmpty));
    EXPECT_FALSE(hasRoom(ServerOccupancy::Full));
}

//=============================================================================
// Source Type Tests
//=============================================================================

TEST(SourceTypeTest, Names) {
    EXPECT_EQ(sourceTypeName(SourceType::Master), "master");
    EXPECT_EQ(sourceTypeName(SourceType::File), "file");
    EXPECT_EQ(sourceTypeName(SourceType::Url), "url");
    EXPECT_EQ(sourceTypeName(SourceType::Dummy), "dummy");
}

TEST(SourceTypeTest, IsNetwork) {
    EXPECT_TRUE(isNetworkSource(SourceType::Master));
    EXPECT_FALSE(isNetworkSource(SourceType::File));
    EXPECT_TRUE(isNetworkSource(SourceType::Url));
    EXPECT_FALSE(isNetworkSource(SourceType::Dummy));
}

//=============================================================================
// Browser Trigger Tests
//=============================================================================

TEST(BrowserTriggerTest, Flags) {
    EXPECT_EQ(BrowserTrigger::None, 0);
    EXPECT_EQ(BrowserTrigger::RefreshDone, 1);
    EXPECT_EQ(BrowserTrigger::SourcesUpdated, 2);
    EXPECT_EQ(BrowserTrigger::NotifyPingTree, 4);
    
    int combined = BrowserTrigger::RefreshDone | BrowserTrigger::SourcesUpdated;
    EXPECT_EQ(combined, 3);
}

//=============================================================================
// BrowserPlayerInfo Tests
//=============================================================================

TEST(BrowserPlayerInfoTest, DefaultConstruction) {
    BrowserPlayerInfo p;
    EXPECT_EQ(p.id, 0);
    EXPECT_EQ(p.frags, 0);
    EXPECT_FALSE(p.isSpectator);
    EXPECT_TRUE(p.isActive());
}

TEST(BrowserPlayerInfoTest, SetName) {
    BrowserPlayerInfo p;
    p.setName("TestPlayer");
    EXPECT_EQ(p.getName(), "TestPlayer");
}

TEST(BrowserPlayerInfoTest, SetTeam) {
    BrowserPlayerInfo p;
    p.setTeam("red");
    EXPECT_EQ(p.getTeam(), "red");
}

TEST(BrowserPlayerInfoTest, SpectatorNotActive) {
    BrowserPlayerInfo p;
    p.isSpectator = true;
    EXPECT_FALSE(p.isActive());
}

//=============================================================================
// Net Address Tests
//=============================================================================

TEST(BrowserNetAddressTest, DefaultConstruction) {
    NetAddress addr;
    EXPECT_EQ(addr.port, 0);
    EXPECT_FALSE(addr.isValid());
}

TEST(BrowserNetAddressTest, ValueConstruction) {
    NetAddress addr(192, 168, 1, 100, 27500);
    EXPECT_EQ(addr.ip[0], 192);
    EXPECT_EQ(addr.ip[1], 168);
    EXPECT_EQ(addr.ip[2], 1);
    EXPECT_EQ(addr.ip[3], 100);
    EXPECT_EQ(addr.port, 27500);
    EXPECT_TRUE(addr.isValid());
}

TEST(BrowserNetAddressTest, ToString) {
    NetAddress addr(127, 0, 0, 1, 27500);
    EXPECT_EQ(addr.toString(), "127.0.0.1:27500");
}

TEST(BrowserNetAddressTest, Equality) {
    NetAddress a(192, 168, 1, 1, 27500);
    NetAddress b(192, 168, 1, 1, 27500);
    NetAddress c(192, 168, 1, 2, 27500);
    
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
}

//=============================================================================
// Server Data Tests
//=============================================================================

TEST(ServerDataTest, DefaultConstruction) {
    ServerData s;
    EXPECT_FALSE(s.passedFilters);
    EXPECT_EQ(s.ping, 999);
    EXPECT_EQ(s.occupancy, ServerOccupancy::Empty);
    EXPECT_FALSE(s.isResponding());
}

TEST(ServerDataTest, SetGetValue) {
    ServerData s;
    s.setValue("hostname", "Test Server");
    s.setValue("map", "dm3");
    
    auto hostname = s.getValue("hostname");
    EXPECT_TRUE(hostname.has_value());
    EXPECT_EQ(*hostname, "Test Server");
    
    auto map = s.getValue("map");
    EXPECT_TRUE(map.has_value());
    EXPECT_EQ(*map, "dm3");
    
    auto missing = s.getValue("nonexistent");
    EXPECT_FALSE(missing.has_value());
}

TEST(ServerDataTest, PlayerCount) {
    ServerData s;
    s.players.resize(4);
    s.players[0].isSpectator = false;
    s.players[1].isSpectator = false;
    s.players[2].isSpectator = true;
    s.players[3].isSpectator = true;
    
    EXPECT_EQ(s.playerCount(), 2);
    EXPECT_EQ(s.totalCount(), 4);
}

TEST(ServerDataTest, IsProxy) {
    ServerData s;
    EXPECT_FALSE(s.isProxy());
    
    s.isQizmo = true;
    EXPECT_TRUE(s.isProxy());
    
    s.isQizmo = false;
    s.isQwfwd = true;
    EXPECT_TRUE(s.isProxy());
}

TEST(ServerDataTest, IsResponding) {
    ServerData s;
    s.ping = 999;
    EXPECT_FALSE(s.isResponding());
    
    s.ping = 50;
    EXPECT_TRUE(s.isResponding());
}

TEST(ServerDataTest, Reset) {
    ServerData s;
    s.ping = 50;
    s.players.resize(5);
    s.passedFilters = true;
    
    s.reset();
    
    EXPECT_EQ(s.ping, 999);
    EXPECT_TRUE(s.players.empty());
    EXPECT_FALSE(s.passedFilters);
}

//=============================================================================
// Source Time Tests
//=============================================================================

TEST(SourceTimeTest, DefaultConstruction) {
    SourceTime t;
    EXPECT_FALSE(t.isSet());
    EXPECT_EQ(t.toString(), "never");
}

TEST(SourceTimeTest, SetValue) {
    SourceTime t;
    t.year = 2024;
    t.month = 12;
    t.day = 25;
    t.hour = 14;
    t.minute = 30;
    t.second = 0;
    
    EXPECT_TRUE(t.isSet());
    EXPECT_EQ(t.toString(), "2024-12-25 14:30:00");
}

//=============================================================================
// Source Data Tests
//=============================================================================

TEST(SourceDataTest, DefaultConstruction) {
    SourceData s;
    EXPECT_EQ(s.type, SourceType::Dummy);
    EXPECT_FALSE(s.isChecked);
    EXPECT_FALSE(s.hasServers());
}

TEST(SourceDataTest, SetName) {
    SourceData s;
    s.setName("Master Server");
    EXPECT_EQ(s.getName(), "Master Server");
}

TEST(SourceDataTest, GetAddressString) {
    SourceData s;
    
    s.type = SourceType::Master;
    s.masterAddress = NetAddress(192, 168, 1, 1, 27000);
    EXPECT_EQ(s.getAddressString(), "192.168.1.1:27000");
    
    s.type = SourceType::File;
    s.filename = "servers.txt";
    EXPECT_EQ(s.getAddressString(), "servers.txt");
    
    s.type = SourceType::Url;
    s.url = "http://example.com/servers";
    EXPECT_EQ(s.getAddressString(), "http://example.com/servers");
}

//=============================================================================
// Player Host Tests
//=============================================================================

TEST(PlayerHostTest, SetName) {
    PlayerHost h;
    h.setName("Player1");
    EXPECT_EQ(h.getName(), "Player1");
}

//=============================================================================
// Browser Window Tests
//=============================================================================

TEST(BrowserWindowTest, DefaultConstruction) {
    BrowserWindow w;
    EXPECT_EQ(w.x, 0);
    EXPECT_EQ(w.width, 0);
    EXPECT_FALSE(w.isValid());
}

TEST(BrowserWindowTest, ValueConstruction) {
    BrowserWindow w(10, 20, 640, 480);
    EXPECT_EQ(w.x, 10);
    EXPECT_EQ(w.y, 20);
    EXPECT_EQ(w.width, 640);
    EXPECT_EQ(w.height, 480);
    EXPECT_TRUE(w.isValid());
    EXPECT_EQ(w.area(), 640 * 480);
}

//=============================================================================
// Server Entry Tests
//=============================================================================

TEST(BrowserServerEntryTest, DefaultConstruction) {
    ServerEntry e;
    EXPECT_FALSE(e.isValid());
}

TEST(BrowserServerEntryTest, ValueConstruction) {
    ServerEntry e("192.168.1.1:27500", "My Server");
    EXPECT_TRUE(e.isValid());
    EXPECT_EQ(e.server, "192.168.1.1:27500");
    EXPECT_EQ(e.description, "My Server");
}

//=============================================================================
// Ping Result Tests
//=============================================================================

TEST(PingResultTest, DefaultConstruction) {
    PingResult r;
    EXPECT_FALSE(r.success);
    EXPECT_FALSE(r.isValid());
}

TEST(PingResultTest, Success) {
    NetAddress addr(192, 168, 1, 1, 27500);
    PingResult r(addr, 50, true);
    EXPECT_TRUE(r.success);
    EXPECT_TRUE(r.isValid());
    EXPECT_EQ(r.ping, 50);
}

TEST(PingResultTest, Failure) {
    NetAddress addr(192, 168, 1, 1, 27500);
    PingResult r(addr, 999, false);
    EXPECT_FALSE(r.isValid());
}

//=============================================================================
// Browser State Tests
//=============================================================================

TEST(BrowserStateTest, DefaultConstruction) {
    BrowserState s;
    EXPECT_FALSE(s.hasPendingWork());
    EXPECT_EQ(s.queuedTriggers, 0);
}

TEST(BrowserStateTest, PendingWork) {
    BrowserState s;
    s.resortServers = true;
    EXPECT_TRUE(s.hasPendingWork());
}

TEST(BrowserStateTest, Triggers) {
    BrowserState s;
    
    EXPECT_FALSE(s.hasTrigger(BrowserTrigger::RefreshDone));
    
    s.addTrigger(BrowserTrigger::RefreshDone);
    EXPECT_TRUE(s.hasTrigger(BrowserTrigger::RefreshDone));
    EXPECT_FALSE(s.hasTrigger(BrowserTrigger::SourcesUpdated));
    
    s.clearTrigger(BrowserTrigger::RefreshDone);
    EXPECT_FALSE(s.hasTrigger(BrowserTrigger::RefreshDone));
}

TEST(BrowserStateTest, Reset) {
    BrowserState s;
    s.resortServers = true;
    s.pingProgress = 0.5;
    s.queuedTriggers = BrowserTrigger::All;
    
    s.reset();
    
    EXPECT_FALSE(s.resortServers);
    EXPECT_EQ(s.pingProgress, 0.0);
    EXPECT_EQ(s.queuedTriggers, 0);
}

//=============================================================================
// Filter Settings Tests
//=============================================================================

TEST(FilterSettingsTest, DefaultNoFilters) {
    FilterSettings f;
    EXPECT_FALSE(f.hasActiveFilters());
}

TEST(FilterSettingsTest, ActiveFilters) {
    FilterSettings f;
    f.hideEmpty = true;
    EXPECT_TRUE(f.hasActiveFilters());
}

TEST(FilterSettingsTest, PassesEmpty) {
    FilterSettings f;
    ServerData s;
    s.occupancy = ServerOccupancy::Empty;
    s.ping = 50;
    
    EXPECT_TRUE(f.passes(s));
    
    f.hideEmpty = true;
    EXPECT_FALSE(f.passes(s));
}

TEST(FilterSettingsTest, PassesFull) {
    FilterSettings f;
    ServerData s;
    s.occupancy = ServerOccupancy::Full;
    s.ping = 50;
    
    EXPECT_TRUE(f.passes(s));
    
    f.hideFull = true;
    EXPECT_FALSE(f.passes(s));
}

TEST(FilterSettingsTest, PassesHighPing) {
    FilterSettings f;
    f.hideHighPing = true;
    f.pingLimit = 100;
    
    ServerData s;
    s.occupancy = ServerOccupancy::NotEmpty;
    s.ping = 50;
    EXPECT_TRUE(f.passes(s));
    
    s.ping = 150;
    EXPECT_FALSE(f.passes(s));
}

TEST(FilterSettingsTest, PassesProxy) {
    FilterSettings f;
    ServerData s;
    s.occupancy = ServerOccupancy::NotEmpty;
    s.ping = 50;
    s.isQizmo = true;
    
    EXPECT_TRUE(f.passes(s));
    
    f.showProxies = false;
    EXPECT_FALSE(f.passes(s));
}

//=============================================================================
// Browser Stats Tests
//=============================================================================

TEST(BrowserStatsTest, FilterRatio) {
    BrowserStats s;
    s.totalServers = 100;
    s.filteredServers = 75;
    
    EXPECT_FLOAT_EQ(s.filterRatio(), 0.75f);
}

TEST(BrowserStatsTest, FilterRatioEmpty) {
    BrowserStats s;
    s.totalServers = 0;
    EXPECT_FLOAT_EQ(s.filterRatio(), 1.0f);
}

//=============================================================================
// Limits Tests
//=============================================================================

TEST(BrowserLimitsTest, Values) {
    EXPECT_EQ(browser_limits::MAX_SOURCES, 200);
    EXPECT_EQ(browser_limits::MAX_SERVERS, 1000);
    EXPECT_EQ(browser_limits::MAX_PLAYERS_PER_SERVER, 128);
    EXPECT_EQ(browser_limits::MAX_SERVER_LIST, 512);
}
