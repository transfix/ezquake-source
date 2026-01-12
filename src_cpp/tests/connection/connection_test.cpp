/**
 * @file connection_test.cpp
 * @brief Comprehensive tests for the connection subsystem
 * 
 * Tests covering:
 * - Connection state machine enums
 * - Download types and methods
 * - Demo seeking types
 * - Server list operations
 * - File persistence
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <filesystem>
#include <fstream>
#include "../../src_cpp/core/connection/connection.hpp"

namespace ezquake::test {

using namespace testing;

//=============================================================================
// Connection State Tests
//=============================================================================

class ConnectionStateTest : public Test {
protected:
    void SetUp() override {}
};

TEST_F(ConnectionStateTest, StateEnumValuesMatchOriginal) {
    // Original C values: ca_disconnected=0, ca_demostart, ca_connected, ca_onserver, ca_active
    EXPECT_EQ(static_cast<uint8_t>(ConnectionState::Disconnected), 0);
    EXPECT_EQ(static_cast<uint8_t>(ConnectionState::DemoStart), 1);
    EXPECT_EQ(static_cast<uint8_t>(ConnectionState::Connected), 2);
    EXPECT_EQ(static_cast<uint8_t>(ConnectionState::OnServer), 3);
    EXPECT_EQ(static_cast<uint8_t>(ConnectionState::Active), 4);
}

TEST_F(ConnectionStateTest, StateNameReturnsCorrectStrings) {
    EXPECT_EQ(connectionStateName(ConnectionState::Disconnected), "disconnected");
    EXPECT_EQ(connectionStateName(ConnectionState::DemoStart), "demostart");
    EXPECT_EQ(connectionStateName(ConnectionState::Connected), "connected");
    EXPECT_EQ(connectionStateName(ConnectionState::OnServer), "onserver");
    EXPECT_EQ(connectionStateName(ConnectionState::Active), "active");
}

TEST_F(ConnectionStateTest, IsConnectedHelperFunction) {
    EXPECT_FALSE(isConnected(ConnectionState::Disconnected));
    EXPECT_FALSE(isConnected(ConnectionState::DemoStart));
    EXPECT_TRUE(isConnected(ConnectionState::Connected));
    EXPECT_TRUE(isConnected(ConnectionState::OnServer));
    EXPECT_TRUE(isConnected(ConnectionState::Active));
}

TEST_F(ConnectionStateTest, CanRenderFramesHelperFunction) {
    EXPECT_FALSE(canRenderFrames(ConnectionState::Disconnected));
    EXPECT_FALSE(canRenderFrames(ConnectionState::DemoStart));
    EXPECT_FALSE(canRenderFrames(ConnectionState::Connected));
    EXPECT_FALSE(canRenderFrames(ConnectionState::OnServer));
    EXPECT_TRUE(canRenderFrames(ConnectionState::Active));
}

//=============================================================================
// Download Type Tests
//=============================================================================

class DownloadTypeTest : public Test {};

TEST_F(DownloadTypeTest, DownloadTypeEnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(DownloadType::None), 0);
    EXPECT_EQ(static_cast<uint8_t>(DownloadType::Model), 1);
    EXPECT_EQ(static_cast<uint8_t>(DownloadType::VwepModel), 2);
    EXPECT_EQ(static_cast<uint8_t>(DownloadType::Sound), 3);
    EXPECT_EQ(static_cast<uint8_t>(DownloadType::Skin), 4);
    EXPECT_EQ(static_cast<uint8_t>(DownloadType::Single), 5);
}

TEST_F(DownloadTypeTest, DownloadTypeNames) {
    EXPECT_EQ(downloadTypeName(DownloadType::None), "none");
    EXPECT_EQ(downloadTypeName(DownloadType::Model), "model");
    EXPECT_EQ(downloadTypeName(DownloadType::VwepModel), "vwep");
    EXPECT_EQ(downloadTypeName(DownloadType::Sound), "sound");
    EXPECT_EQ(downloadTypeName(DownloadType::Skin), "skin");
    EXPECT_EQ(downloadTypeName(DownloadType::Single), "file");
}

TEST_F(DownloadTypeTest, DownloadMethodValues) {
    EXPECT_EQ(static_cast<uint8_t>(DownloadMethod::None), 0);
    EXPECT_EQ(static_cast<uint8_t>(DownloadMethod::Standard), 1);
    EXPECT_EQ(static_cast<uint8_t>(DownloadMethod::Chunked), 2);
}

//=============================================================================
// Demo Seeking Type Tests
//=============================================================================

class DemoSeekingTypeTest : public Test {};

TEST_F(DemoSeekingTypeTest, SeekingTypeValues) {
    EXPECT_EQ(static_cast<uint8_t>(DemoSeekingType::None), 0);
    EXPECT_EQ(static_cast<uint8_t>(DemoSeekingType::Normal), 1);
    EXPECT_EQ(static_cast<uint8_t>(DemoSeekingType::DemoMark), 2);
    EXPECT_EQ(static_cast<uint8_t>(DemoSeekingType::Status), 3);
    EXPECT_EQ(static_cast<uint8_t>(DemoSeekingType::Found), 4);
    EXPECT_EQ(static_cast<uint8_t>(DemoSeekingType::End), 5);
    EXPECT_EQ(static_cast<uint8_t>(DemoSeekingType::FoundNoRewind), 6);
}

TEST_F(DemoSeekingTypeTest, SeekMatchTypeValues) {
    EXPECT_EQ(static_cast<uint8_t>(DemoSeekMatchType::Equal), 0);
    EXPECT_EQ(static_cast<uint8_t>(DemoSeekMatchType::NotEqual), 1);
    EXPECT_EQ(static_cast<uint8_t>(DemoSeekMatchType::LessThan), 2);
    EXPECT_EQ(static_cast<uint8_t>(DemoSeekMatchType::GreaterThan), 3);
    EXPECT_EQ(static_cast<uint8_t>(DemoSeekMatchType::BitOn), 4);
    EXPECT_EQ(static_cast<uint8_t>(DemoSeekMatchType::BitOff), 5);
}

//=============================================================================
// MVD Playback Tests
//=============================================================================

class MvdPlaybackTest : public Test {};

TEST_F(MvdPlaybackTest, PlaybackModeValues) {
    EXPECT_EQ(static_cast<uint8_t>(MvdPlayback::None), 0);
    EXPECT_EQ(static_cast<uint8_t>(MvdPlayback::File), 1);
    EXPECT_EQ(static_cast<uint8_t>(MvdPlayback::Qtv), 2);
}

//=============================================================================
// Timedemo Mode Tests
//=============================================================================

class TimedemoModeTest : public Test {};

TEST_F(TimedemoModeTest, ModeValues) {
    EXPECT_EQ(static_cast<uint8_t>(TimedemoMode::Off), 0);
    EXPECT_EQ(static_cast<uint8_t>(TimedemoMode::Classic), 1);
    EXPECT_EQ(static_cast<uint8_t>(TimedemoMode::FixedFps), 2);
}

TEST_F(TimedemoModeTest, ConstantsAreCorrect) {
    EXPECT_EQ(timedemo::DEFAULT_FPS, 308);
    EXPECT_EQ(timedemo::MIN_FPS, 20);
    EXPECT_EQ(timedemo::MAX_FPS, 10000);
}

//=============================================================================
// Connection Timing Tests
//=============================================================================

class ConnectionTimingTest : public Test {
protected:
    ConnectionTiming timing_;
};

TEST_F(ConnectionTimingTest, DefaultInitialization) {
    EXPECT_DOUBLE_EQ(timing_.realtime, 0.0);
    EXPECT_DOUBLE_EQ(timing_.demotime, 0.0);
    EXPECT_DOUBLE_EQ(timing_.trueFrametime, 0.0);
    EXPECT_DOUBLE_EQ(timing_.frametime, 0.0);
    EXPECT_EQ(timing_.frameCount, 0);
}

TEST_F(ConnectionTimingTest, AdvanceFrameNormalSpeed) {
    timing_.advanceFrame(0.016); // ~60 FPS
    
    EXPECT_DOUBLE_EQ(timing_.trueFrametime, 0.016);
    EXPECT_DOUBLE_EQ(timing_.frametime, 0.016);
    EXPECT_DOUBLE_EQ(timing_.realtime, 0.016);
    EXPECT_EQ(timing_.frameCount, 1);
}

TEST_F(ConnectionTimingTest, AdvanceFrameWithDemoSpeed) {
    timing_.advanceFrame(0.016, 2.0); // 2x demo speed
    
    EXPECT_DOUBLE_EQ(timing_.trueFrametime, 0.016);
    EXPECT_DOUBLE_EQ(timing_.frametime, 0.032);
    EXPECT_DOUBLE_EQ(timing_.realtime, 0.032);
}

TEST_F(ConnectionTimingTest, AdvanceFrameAccumulates) {
    timing_.advanceFrame(0.016);
    timing_.advanceFrame(0.016);
    timing_.advanceFrame(0.016);
    
    EXPECT_DOUBLE_EQ(timing_.realtime, 0.048);
    EXPECT_EQ(timing_.frameCount, 3);
}

TEST_F(ConnectionTimingTest, ResetClearsAll) {
    timing_.advanceFrame(0.016);
    timing_.reset();
    
    EXPECT_DOUBLE_EQ(timing_.realtime, 0.0);
    EXPECT_EQ(timing_.frameCount, 0);
}

//=============================================================================
// Protocol Extensions Tests
//=============================================================================

class ProtocolExtensionsTest : public Test {
protected:
    ProtocolExtensions ext_;
};

TEST_F(ProtocolExtensionsTest, DefaultIsEmpty) {
    EXPECT_EQ(ext_.fte1, 0u);
    EXPECT_EQ(ext_.fte2, 0u);
    EXPECT_EQ(ext_.mvd1, 0u);
    EXPECT_FALSE(ext_.any());
}

TEST_F(ProtocolExtensionsTest, AnyReturnsTrueIfSet) {
    ext_.fte1 = 0x1;
    EXPECT_TRUE(ext_.any());
    
    ext_.clear();
    ext_.fte2 = 0x1;
    EXPECT_TRUE(ext_.any());
    
    ext_.clear();
    ext_.mvd1 = 0x1;
    EXPECT_TRUE(ext_.any());
}

TEST_F(ProtocolExtensionsTest, Clear) {
    ext_.fte1 = 0xFFFFFFFF;
    ext_.fte2 = 0xFFFFFFFF;
    ext_.mvd1 = 0xFFFFFFFF;
    
    ext_.clear();
    
    EXPECT_EQ(ext_.fte1, 0u);
    EXPECT_EQ(ext_.fte2, 0u);
    EXPECT_EQ(ext_.mvd1, 0u);
}

//=============================================================================
// Server Entry Tests
//=============================================================================

class ServerEntryTest : public Test {
protected:
    ServerEntry entry_;
};

TEST_F(ServerEntryTest, DefaultConstructor) {
    EXPECT_TRUE(entry_.isEmpty());
    EXPECT_FALSE(entry_.isValid());
    EXPECT_TRUE(entry_.address().empty());
    EXPECT_TRUE(entry_.description().empty());
}

TEST_F(ServerEntryTest, ConstructWithAddress) {
    ServerEntry e("192.168.1.1:27500");
    
    EXPECT_FALSE(e.isEmpty());
    EXPECT_TRUE(e.isValid());
    EXPECT_EQ(e.address(), "192.168.1.1:27500");
    EXPECT_FALSE(e.hasDescription());
}

TEST_F(ServerEntryTest, ConstructWithAddressAndDescription) {
    ServerEntry e("192.168.1.1:27500", "My Server");
    
    EXPECT_EQ(e.address(), "192.168.1.1:27500");
    EXPECT_EQ(e.description(), "My Server");
    EXPECT_TRUE(e.hasDescription());
}

TEST_F(ServerEntryTest, SetAddress) {
    entry_.setAddress("10.0.0.1:27500");
    EXPECT_EQ(entry_.address(), "10.0.0.1:27500");
}

TEST_F(ServerEntryTest, SetDescription) {
    entry_.setDescription("Test Server");
    EXPECT_EQ(entry_.description(), "Test Server");
}

TEST_F(ServerEntryTest, SetBoth) {
    entry_.set("host.com:27500", "Best Server");
    
    EXPECT_EQ(entry_.address(), "host.com:27500");
    EXPECT_EQ(entry_.description(), "Best Server");
}

TEST_F(ServerEntryTest, Clear) {
    entry_.set("server.com", "desc");
    entry_.clear();
    
    EXPECT_TRUE(entry_.isEmpty());
}

TEST_F(ServerEntryTest, Equality) {
    ServerEntry a("192.168.1.1:27500", "Server A");
    ServerEntry b("192.168.1.1:27500", "Server B");
    ServerEntry c("192.168.1.2:27500", "Server A");
    
    // Equality is based on address only
    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

TEST_F(ServerEntryTest, MatchesAddress) {
    entry_.set("quake.server.com:27500", "My Server");
    
    EXPECT_TRUE(entry_.matchesAddress("quake.server.com:27500"));
    EXPECT_FALSE(entry_.matchesAddress("other.server.com:27500"));
}

TEST_F(ServerEntryTest, DisplayStringWithoutDescription) {
    entry_.setAddress("192.168.1.1:27500");
    EXPECT_EQ(entry_.displayString(), "192.168.1.1:27500");
}

TEST_F(ServerEntryTest, DisplayStringWithDescription) {
    entry_.set("192.168.1.1:27500", "My Server");
    EXPECT_EQ(entry_.displayString(), "192.168.1.1:27500 - My Server");
}

TEST_F(ServerEntryTest, ToFileFormat) {
    entry_.set("192.168.1.1:27500", "My Server");
    EXPECT_EQ(entry_.toFileFormat(), "192.168.1.1:27500\tMy Server");
}

TEST_F(ServerEntryTest, ToFileFormatNoDescription) {
    entry_.setAddress("192.168.1.1:27500");
    EXPECT_EQ(entry_.toFileFormat(), "192.168.1.1:27500");
}

TEST_F(ServerEntryTest, FromFileFormat) {
    auto e = ServerEntry::fromFileFormat("192.168.1.1:27500\tMy Server");
    
    EXPECT_EQ(e.address(), "192.168.1.1:27500");
    EXPECT_EQ(e.description(), "My Server");
}

TEST_F(ServerEntryTest, FromFileFormatNoDescription) {
    auto e = ServerEntry::fromFileFormat("192.168.1.1:27500");
    
    EXPECT_EQ(e.address(), "192.168.1.1:27500");
    EXPECT_FALSE(e.hasDescription());
}

//=============================================================================
// Server List Tests
//=============================================================================

class ServerListTest : public Test {
protected:
    ServerList list_;
};

TEST_F(ServerListTest, DefaultConstruction) {
    EXPECT_TRUE(list_.empty());
    EXPECT_EQ(list_.size(), 0u);
    EXPECT_EQ(list_.capacity(), server_list::MAX_ENTRIES);
    EXPECT_FALSE(list_.full());
}

TEST_F(ServerListTest, CustomCapacity) {
    ServerList small(10);
    EXPECT_EQ(small.capacity(), 10u);
}

TEST_F(ServerListTest, AddEntry) {
    EXPECT_TRUE(list_.add("server1.com:27500", "Server 1"));
    
    EXPECT_EQ(list_.size(), 1u);
    EXPECT_EQ(list_[0].address(), "server1.com:27500");
}

TEST_F(ServerListTest, AddMultipleEntries) {
    list_.add("server1.com:27500");
    list_.add("server2.com:27500");
    list_.add("server3.com:27500");
    
    EXPECT_EQ(list_.size(), 3u);
}

TEST_F(ServerListTest, AddWhenFull) {
    ServerList small(2);
    EXPECT_TRUE(small.add("server1.com"));
    EXPECT_TRUE(small.add("server2.com"));
    EXPECT_FALSE(small.add("server3.com")); // Should fail
    
    EXPECT_EQ(small.size(), 2u);
}

TEST_F(ServerListTest, AddOrUpdateNew) {
    EXPECT_TRUE(list_.addOrUpdate("server.com:27500", "New Server"));
    EXPECT_EQ(list_.size(), 1u);
}

TEST_F(ServerListTest, AddOrUpdateExisting) {
    list_.add("server.com:27500", "Old Description");
    list_.addOrUpdate("server.com:27500", "New Description");
    
    EXPECT_EQ(list_.size(), 1u);
    EXPECT_EQ(list_[0].description(), "New Description");
}

TEST_F(ServerListTest, SetAtIndex) {
    list_.set(5, "server.com:27500", "Server");
    
    EXPECT_EQ(list_.size(), 6u); // Entries 0-5
    EXPECT_EQ(list_[5].address(), "server.com:27500");
}

TEST_F(ServerListTest, Insert) {
    list_.add("server1.com");
    list_.add("server3.com");
    list_.insert(1, "server2.com");
    
    EXPECT_EQ(list_.size(), 3u);
    EXPECT_EQ(list_[0].address(), "server1.com");
    EXPECT_EQ(list_[1].address(), "server2.com");
    EXPECT_EQ(list_[2].address(), "server3.com");
}

TEST_F(ServerListTest, Remove) {
    list_.add("server1.com");
    list_.add("server2.com");
    list_.add("server3.com");
    
    EXPECT_TRUE(list_.remove("server2.com"));
    
    EXPECT_EQ(list_.size(), 2u);
    EXPECT_FALSE(list_.contains("server2.com"));
}

TEST_F(ServerListTest, RemoveNonExistent) {
    list_.add("server1.com");
    EXPECT_FALSE(list_.remove("nonexistent.com"));
}

TEST_F(ServerListTest, RemoveAt) {
    list_.add("server1.com");
    list_.add("server2.com");
    
    EXPECT_TRUE(list_.removeAt(0));
    
    EXPECT_EQ(list_.size(), 1u);
    EXPECT_EQ(list_[0].address(), "server2.com");
}

TEST_F(ServerListTest, ResetAt) {
    list_.add("server1.com", "desc");
    list_.resetAt(0);
    
    EXPECT_EQ(list_.size(), 1u); // Still there, just empty
    EXPECT_TRUE(list_[0].isEmpty());
}

TEST_F(ServerListTest, Swap) {
    list_.add("server1.com", "First");
    list_.add("server2.com", "Second");
    
    EXPECT_TRUE(list_.swap(0, 1));
    
    EXPECT_EQ(list_[0].address(), "server2.com");
    EXPECT_EQ(list_[1].address(), "server1.com");
}

TEST_F(ServerListTest, Move) {
    list_.add("server1.com");
    list_.add("server2.com");
    list_.add("server3.com");
    
    EXPECT_TRUE(list_.move(2, 0));
    
    EXPECT_EQ(list_[0].address(), "server3.com");
    EXPECT_EQ(list_[1].address(), "server1.com");
    EXPECT_EQ(list_[2].address(), "server2.com");
}

TEST_F(ServerListTest, Clear) {
    list_.add("server1.com");
    list_.add("server2.com");
    list_.clear();
    
    EXPECT_TRUE(list_.empty());
}

TEST_F(ServerListTest, Compact) {
    list_.add("server1.com");
    list_.add("");  // Empty
    list_.add("server2.com");
    list_.add("");  // Empty
    
    list_.compact();
    
    EXPECT_EQ(list_.size(), 2u);
    EXPECT_EQ(list_[0].address(), "server1.com");
    EXPECT_EQ(list_[1].address(), "server2.com");
}

TEST_F(ServerListTest, Find) {
    list_.add("server1.com");
    list_.add("server2.com");
    
    auto it = list_.find("server2.com");
    ASSERT_NE(it, list_.end());
    EXPECT_EQ(it->address(), "server2.com");
}

TEST_F(ServerListTest, FindNotFound) {
    list_.add("server1.com");
    
    auto it = list_.find("nonexistent.com");
    EXPECT_EQ(it, list_.end());
}

TEST_F(ServerListTest, Contains) {
    list_.add("server1.com");
    
    EXPECT_TRUE(list_.contains("server1.com"));
    EXPECT_FALSE(list_.contains("server2.com"));
}

TEST_F(ServerListTest, IndexOf) {
    list_.add("server1.com");
    list_.add("server2.com");
    list_.add("server3.com");
    
    EXPECT_EQ(list_.indexOf("server1.com"), 0);
    EXPECT_EQ(list_.indexOf("server2.com"), 1);
    EXPECT_EQ(list_.indexOf("server3.com"), 2);
    EXPECT_EQ(list_.indexOf("nonexistent.com"), -1);
}

TEST_F(ServerListTest, GetByIndex) {
    list_.add("server1.com");
    
    auto* entry = list_.get(0);
    ASSERT_NE(entry, nullptr);
    EXPECT_EQ(entry->address(), "server1.com");
    
    EXPECT_EQ(list_.get(99), nullptr);
}

TEST_F(ServerListTest, AtByIndex) {
    list_.add("server1.com");
    
    auto opt = list_.at(0);
    ASSERT_TRUE(opt.has_value());
    EXPECT_EQ(opt->get().address(), "server1.com");
    
    EXPECT_FALSE(list_.at(99).has_value());
}

TEST_F(ServerListTest, SortByAddress) {
    list_.add("charlie.com");
    list_.add("alpha.com");
    list_.add("bravo.com");
    
    list_.sortByAddress();
    
    EXPECT_EQ(list_[0].address(), "alpha.com");
    EXPECT_EQ(list_[1].address(), "bravo.com");
    EXPECT_EQ(list_[2].address(), "charlie.com");
}

TEST_F(ServerListTest, SortByDescription) {
    list_.add("server1.com", "Zulu");
    list_.add("server2.com", "Alpha");
    list_.add("server3.com", "Mike");
    
    list_.sortByDescription();
    
    EXPECT_EQ(list_[0].description(), "Alpha");
    EXPECT_EQ(list_[1].description(), "Mike");
    EXPECT_EQ(list_[2].description(), "Zulu");
}

TEST_F(ServerListTest, CountValid) {
    list_.add("server1.com");
    list_.add("");
    list_.add("server2.com");
    
    EXPECT_EQ(list_.countValid(), 2u);
}

TEST_F(ServerListTest, CountWithDescriptions) {
    list_.add("server1.com", "With desc");
    list_.add("server2.com");
    list_.add("server3.com", "Also with desc");
    
    EXPECT_EQ(list_.countWithDescriptions(), 2u);
}

TEST_F(ServerListTest, ModifiedFlag) {
    EXPECT_FALSE(list_.isModified());
    
    list_.add("server.com");
    EXPECT_TRUE(list_.isModified());
    
    list_.clearModified();
    EXPECT_FALSE(list_.isModified());
    
    list_.remove("server.com");
    EXPECT_TRUE(list_.isModified());
}

TEST_F(ServerListTest, Iterator) {
    list_.add("server1.com");
    list_.add("server2.com");
    list_.add("server3.com");
    
    int count = 0;
    for (const auto& entry : list_) {
        EXPECT_FALSE(entry.isEmpty());
        ++count;
    }
    EXPECT_EQ(count, 3);
}

TEST_F(ServerListTest, FindIf) {
    list_.add("server1.com", "FFA");
    list_.add("server2.com", "CTF");
    list_.add("server3.com", "FFA");
    
    auto ffa = list_.findIf([](const ServerEntry& e) {
        return e.description() == "FFA";
    });
    
    EXPECT_EQ(ffa.size(), 2u);
}

//=============================================================================
// Server List File I/O Tests
//=============================================================================

class ServerListFileTest : public Test {
protected:
    ServerList list_;
    std::filesystem::path tempDir_;
    std::filesystem::path tempFile_;
    
    void SetUp() override {
        tempDir_ = std::filesystem::temp_directory_path() / "ezquake_test";
        std::filesystem::create_directories(tempDir_);
        tempFile_ = tempDir_ / "servers.txt";
    }
    
    void TearDown() override {
        std::filesystem::remove_all(tempDir_);
    }
};

TEST_F(ServerListFileTest, SaveAndLoad) {
    list_.add("server1.com:27500", "First Server");
    list_.add("server2.com:27500", "Second Server");
    list_.add("server3.com:27500");
    
    EXPECT_TRUE(list_.save(tempFile_));
    
    ServerList loaded;
    EXPECT_EQ(loaded.load(tempFile_), 3);
    
    EXPECT_EQ(loaded.size(), 3u);
    EXPECT_EQ(loaded[0].address(), "server1.com:27500");
    EXPECT_EQ(loaded[0].description(), "First Server");
    EXPECT_EQ(loaded[2].address(), "server3.com:27500");
    EXPECT_FALSE(loaded[2].hasDescription());
}

TEST_F(ServerListFileTest, LoadNonExistentFile) {
    EXPECT_EQ(list_.load(tempDir_ / "nonexistent.txt"), -1);
}

TEST_F(ServerListFileTest, LoadSkipsEmptyLines) {
    std::ofstream file(tempFile_);
    file << "server1.com\n";
    file << "\n";
    file << "server2.com\n";
    file << "   \n";
    file << "server3.com\n";
    file.close();
    
    EXPECT_EQ(list_.load(tempFile_), 3);
}

TEST_F(ServerListFileTest, LoadSkipsComments) {
    std::ofstream file(tempFile_);
    file << "# This is a comment\n";
    file << "server1.com\n";
    file << "// Another comment\n";
    file << "server2.com\n";
    file.close();
    
    EXPECT_EQ(list_.load(tempFile_), 2);
}

TEST_F(ServerListFileTest, LoadHandlesTrailingWhitespace) {
    std::ofstream file(tempFile_);
    file << "server1.com\t   \n";
    file << "server2.com\r\n";
    file.close();
    
    EXPECT_EQ(list_.load(tempFile_), 2);
    EXPECT_EQ(list_[0].address(), "server1.com");
}

TEST_F(ServerListFileTest, ModifiedFlagAfterLoad) {
    list_.add("temp.com");
    list_.save(tempFile_);
    
    ServerList loaded;
    loaded.load(tempFile_);
    
    EXPECT_FALSE(loaded.isModified());
}

//=============================================================================
// Categorized Server List Tests
//=============================================================================

class CategorizedServerListTest : public Test {
protected:
    CategorizedServerList catList_{ServerCategory::Favorites, "favorites"};
};

TEST_F(CategorizedServerListTest, CategoryAndName) {
    EXPECT_EQ(catList_.category(), ServerCategory::Favorites);
    EXPECT_EQ(catList_.name(), "favorites");
}

TEST_F(CategorizedServerListTest, ListAccess) {
    catList_.list().add("server.com", "My Favorite");
    EXPECT_EQ(catList_.list().size(), 1u);
}

//=============================================================================
// Server List Manager Tests
//=============================================================================

class ServerListManagerTest : public Test {
protected:
    ServerListManager manager_;
    std::filesystem::path tempDir_;
    
    void SetUp() override {
        tempDir_ = std::filesystem::temp_directory_path() / "ezquake_manager_test";
        std::filesystem::create_directories(tempDir_);
    }
    
    void TearDown() override {
        std::filesystem::remove_all(tempDir_);
    }
};

TEST_F(ServerListManagerTest, HasFavoritesAndRecent) {
    EXPECT_TRUE(manager_.favorites().empty());
    EXPECT_TRUE(manager_.recent().empty());
}

TEST_F(ServerListManagerTest, AddToFavorites) {
    manager_.favorites().add("favorite.com", "My Fav");
    EXPECT_TRUE(manager_.isFavorite("favorite.com"));
}

TEST_F(ServerListManagerTest, ToggleFavorite) {
    EXPECT_TRUE(manager_.toggleFavorite("server.com", "Test"));
    EXPECT_TRUE(manager_.isFavorite("server.com"));
    
    EXPECT_FALSE(manager_.toggleFavorite("server.com"));
    EXPECT_FALSE(manager_.isFavorite("server.com"));
}

TEST_F(ServerListManagerTest, AddToRecent) {
    manager_.addToRecent("server1.com", "First");
    manager_.addToRecent("server2.com", "Second");
    manager_.addToRecent("server3.com", "Third");
    
    EXPECT_EQ(manager_.recent().size(), 3u);
    // Most recent should be first
    EXPECT_EQ(manager_.recent()[0].address(), "server3.com");
}

TEST_F(ServerListManagerTest, AddToRecentMovesDuplicateToFront) {
    manager_.addToRecent("server1.com");
    manager_.addToRecent("server2.com");
    manager_.addToRecent("server1.com"); // Add again
    
    EXPECT_EQ(manager_.recent().size(), 2u);
    EXPECT_EQ(manager_.recent()[0].address(), "server1.com");
}

TEST_F(ServerListManagerTest, GetListByCategory) {
    auto* favs = manager_.getList(ServerCategory::Favorites);
    ASSERT_NE(favs, nullptr);
    
    favs->add("test.com");
    EXPECT_TRUE(manager_.isFavorite("test.com"));
}

TEST_F(ServerListManagerTest, AddCustomList) {
    manager_.addCustomList("lan_servers");
    
    auto* custom = manager_.getList(ServerCategory::Custom);
    ASSERT_NE(custom, nullptr);
}

TEST_F(ServerListManagerTest, SaveAndLoadAll) {
    manager_.favorites().add("fav.com", "Favorite");
    manager_.addToRecent("recent.com", "Recent");
    
    manager_.saveAll(tempDir_);
    
    ServerListManager loaded;
    loaded.loadAll(tempDir_);
    
    EXPECT_TRUE(loaded.isFavorite("fav.com"));
    EXPECT_EQ(loaded.recent().size(), 1u);
}

//=============================================================================
// Server List Constants Tests
//=============================================================================

class ServerListConstantsTest : public Test {};

TEST_F(ServerListConstantsTest, MaxServerListEntries) {
    EXPECT_EQ(server_list::MAX_ENTRIES, 512u);
}

TEST_F(ServerListConstantsTest, DefaultFilename) {
    EXPECT_EQ(server_list::DEFAULT_FILENAME, "servers.txt");
}

TEST_F(ServerListConstantsTest, FieldSeparator) {
    EXPECT_EQ(server_list::FIELD_SEPARATOR, '\t');
}

} // namespace ezquake::test
