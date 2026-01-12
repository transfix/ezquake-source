/**
 * @file logging_test.cpp
 * @brief Unit tests for logging and ignore types
 */

#include <gtest/gtest.h>
#include "../../core/logging/logging_types.hpp"

using namespace ezquake::logging;

// ============================================================================
// LogType Tests
// ============================================================================

TEST(LogTypeTest, Names) {
    EXPECT_EQ(logTypeName(LogType::Console), "console");
    EXPECT_EQ(logTypeName(LogType::Error), "error");
    EXPECT_EQ(logTypeName(LogType::Frag), "frag");
    EXPECT_EQ(logTypeName(LogType::Player), "player");
}

TEST(LogTypeTest, DefaultFilenames) {
    EXPECT_EQ(logTypeDefaultFilename(LogType::Console), "console.log");
    EXPECT_EQ(logTypeDefaultFilename(LogType::Error), "error.log");
    EXPECT_EQ(logTypeDefaultFilename(LogType::Frag), "frag.log");
}

// ============================================================================
// LogLevel Tests
// ============================================================================

TEST(LogLevelTest, Names) {
    EXPECT_EQ(logLevelName(LogLevel::Debug), "debug");
    EXPECT_EQ(logLevelName(LogLevel::Info), "info");
    EXPECT_EQ(logLevelName(LogLevel::Warning), "warning");
    EXPECT_EQ(logLevelName(LogLevel::Error), "error");
    EXPECT_EQ(logLevelName(LogLevel::Critical), "critical");
}

TEST(LogLevelTest, ShouldLog) {
    EXPECT_TRUE(shouldLog(LogLevel::Error, LogLevel::Info));
    EXPECT_TRUE(shouldLog(LogLevel::Info, LogLevel::Info));
    EXPECT_FALSE(shouldLog(LogLevel::Debug, LogLevel::Info));
    EXPECT_FALSE(shouldLog(LogLevel::Info, LogLevel::None));
}

// ============================================================================
// LogEntry Tests
// ============================================================================

TEST(LogEntryTest, IsValid) {
    LogEntry entry;
    EXPECT_FALSE(entry.isValid());

    entry.message = "Test message";
    EXPECT_TRUE(entry.isValid());
}

TEST(LogEntryTest, IsError) {
    LogEntry entry;
    entry.level = LogLevel::Info;
    EXPECT_FALSE(entry.isError());

    entry.level = LogLevel::Error;
    EXPECT_TRUE(entry.isError());

    entry.level = LogLevel::Critical;
    EXPECT_TRUE(entry.isError());
}

// ============================================================================
// LogConfig Tests
// ============================================================================

TEST(LogConfigTest, IsOpen) {
    LogConfig config;
    EXPECT_FALSE(config.isOpen());

    int dummy = 0;
    config.fileHandle = &dummy;
    EXPECT_TRUE(config.isOpen());
}

TEST(LogConfigTest, ShouldLog) {
    LogConfig config;
    config.enabled = true;
    config.minLevel = LogLevel::Warning;

    EXPECT_TRUE(config.shouldLog(LogLevel::Error));
    EXPECT_TRUE(config.shouldLog(LogLevel::Warning));
    EXPECT_FALSE(config.shouldLog(LogLevel::Info));
}

TEST(LogConfigTest, ShouldLogDisabled) {
    LogConfig config;
    config.enabled = false;
    config.minLevel = LogLevel::Debug;

    EXPECT_FALSE(config.shouldLog(LogLevel::Critical));
}

// ============================================================================
// LogManagerState Tests
// ============================================================================

TEST(LogManagerStateTest, GetLog) {
    LogManagerState state;
    
    auto* consoleLog = state.getLog(LogType::Console);
    ASSERT_NE(consoleLog, nullptr);

    auto* errorLog = state.getLog(LogType::Error);
    ASSERT_NE(errorLog, nullptr);
    EXPECT_NE(consoleLog, errorLog);
}

TEST(LogManagerStateTest, EnableDisableAll) {
    LogManagerState state;
    
    state.enableAll();
    EXPECT_EQ(state.enabledCount(), logging_limits::MAX_LOG_TYPES);

    state.disableAll();
    EXPECT_EQ(state.enabledCount(), 0u);
}

TEST(LogManagerStateTest, Reset) {
    LogManagerState state;
    state.enableAll();
    state.globalEnabled = false;

    state.reset();
    EXPECT_EQ(state.enabledCount(), 0u);
    EXPECT_TRUE(state.globalEnabled);
}

// ============================================================================
// FloodEntry Tests
// ============================================================================

TEST(FloodEntryTest, IsEmpty) {
    FloodEntry entry;
    EXPECT_TRUE(entry.isEmpty());

    entry.data = "test";
    EXPECT_FALSE(entry.isEmpty());
}

TEST(FloodEntryTest, Clear) {
    FloodEntry entry;
    entry.data = "message";
    entry.timestamp = 5.0f;

    entry.clear();
    EXPECT_TRUE(entry.isEmpty());
    EXPECT_FLOAT_EQ(entry.timestamp, 0.0f);
}

// ============================================================================
// FloodProtection Tests
// ============================================================================

TEST(FloodProtectionTest, IsFlooding) {
    FloodProtection flood;
    flood.enabled = true;
    flood.duration = 4.0f;

    flood.recordMessage("test message", 1.0f);

    EXPECT_TRUE(flood.isFlooding("test message", 2.0f));   // Within duration
    EXPECT_FALSE(flood.isFlooding("test message", 10.0f)); // Outside duration
    EXPECT_FALSE(flood.isFlooding("other message", 2.0f)); // Different message
}

TEST(FloodProtectionTest, IsFloodingDisabled) {
    FloodProtection flood;
    flood.enabled = false;

    flood.recordMessage("test", 1.0f);
    EXPECT_FALSE(flood.isFlooding("test", 2.0f));
}

TEST(FloodProtectionTest, CircularBuffer) {
    FloodProtection flood;
    flood.enabled = true;
    flood.duration = 100.0f;  // Large duration

    // Fill buffer
    for (int i = 0; i < logging_limits::FLOOD_LIST_SIZE + 5; ++i) {
        flood.recordMessage("msg" + std::to_string(i), static_cast<float>(i));
    }

    // Old messages should be evicted
    EXPECT_FALSE(flood.isFlooding("msg0", 50.0f));
    // Recent messages should still be there
    EXPECT_TRUE(flood.isFlooding("msg" + std::to_string(logging_limits::FLOOD_LIST_SIZE + 4), 50.0f));
}

TEST(FloodProtectionTest, Clear) {
    FloodProtection flood;
    flood.recordMessage("test", 1.0f);

    flood.clear();
    EXPECT_EQ(flood.currentIndex, 0u);
}

TEST(FloodProtectionTest, Reset) {
    FloodProtection flood;
    flood.enabled = true;
    flood.duration = 10.0f;
    flood.recordMessage("test", 1.0f);

    flood.reset();
    EXPECT_FALSE(flood.enabled);
    EXPECT_FLOAT_EQ(flood.duration, logging_limits::DEFAULT_FLOOD_DURATION);
}

// ============================================================================
// IgnoreMode Tests
// ============================================================================

TEST(IgnoreModeTest, Names) {
    EXPECT_EQ(ignoreModeName(IgnoreMode::Disabled), "disabled");
    EXPECT_EQ(ignoreModeName(IgnoreMode::Normal), "normal");
    EXPECT_EQ(ignoreModeName(IgnoreMode::Absolute), "absolute");
}

// ============================================================================
// IgnoreEntry Tests
// ============================================================================

TEST(IgnoreEntryTest, IsSet) {
    IgnoreEntry entry;
    EXPECT_FALSE(entry.isSet());

    entry.playerSlot = 0;
    EXPECT_TRUE(entry.isSet());
}

TEST(IgnoreEntryTest, IsIgnored) {
    IgnoreEntry entry;
    entry.playerSlot = 5;

    EXPECT_FALSE(entry.isIgnored());

    entry.ignored = true;
    EXPECT_TRUE(entry.isIgnored());

    entry.ignored = false;
    entry.voiceIgnored = true;
    EXPECT_TRUE(entry.isIgnored());
}

// ============================================================================
// IgnoreState Tests
// ============================================================================

TEST(IgnoreStateTest, SetPlayerIgnored) {
    IgnoreState state;

    EXPECT_TRUE(state.setPlayerIgnored(5, true));
    EXPECT_TRUE(state.isPlayerIgnored(5));
    EXPECT_FALSE(state.isPlayerIgnored(0));
}

TEST(IgnoreStateTest, TeamIgnore) {
    IgnoreState state;

    EXPECT_TRUE(state.addTeamIgnore("evil"));
    EXPECT_TRUE(state.isTeamIgnored("evil"));
    EXPECT_FALSE(state.isTeamIgnored("good"));

    EXPECT_TRUE(state.removeTeamIgnore("evil"));
    EXPECT_FALSE(state.isTeamIgnored("evil"));
}

TEST(IgnoreStateTest, MaxTeamIgnores) {
    IgnoreState state;

    for (int i = 0; i < logging_limits::MAX_TEAM_IGNORE; ++i) {
        EXPECT_TRUE(state.addTeamIgnore("team" + std::to_string(i)));
    }

    EXPECT_FALSE(state.addTeamIgnore("overflow"));
}

TEST(IgnoreStateTest, DuplicateTeamIgnore) {
    IgnoreState state;

    EXPECT_TRUE(state.addTeamIgnore("team"));
    EXPECT_FALSE(state.addTeamIgnore("team"));  // Duplicate
}

TEST(IgnoreStateTest, IgnoredPlayerCount) {
    IgnoreState state;

    state.setPlayerIgnored(1, true);
    state.setPlayerIgnored(3, true);
    state.setPlayerIgnored(5, false);

    EXPECT_EQ(state.ignoredPlayerCount(), 2u);
}

TEST(IgnoreStateTest, ClearPlayerIgnores) {
    IgnoreState state;

    state.setPlayerIgnored(0, true);
    state.setPlayerIgnored(5, true);

    state.clearPlayerIgnores();
    EXPECT_EQ(state.ignoredPlayerCount(), 0u);
}

TEST(IgnoreStateTest, Reset) {
    IgnoreState state;
    state.setPlayerIgnored(0, true);
    state.addTeamIgnore("team");
    state.mode = IgnoreMode::Absolute;
    state.ignoreOpponents = true;

    state.reset();

    EXPECT_EQ(state.ignoredPlayerCount(), 0u);
    EXPECT_TRUE(state.teamIgnores.empty());
    EXPECT_EQ(state.mode, IgnoreMode::Disabled);
    EXPECT_FALSE(state.ignoreOpponents);
}

// ============================================================================
// ConsoleLogDest Tests
// ============================================================================

TEST(ConsoleLogDestTest, BitwiseOr) {
    auto dest = ConsoleLogDest::File | ConsoleLogDest::Screen;
    EXPECT_TRUE(hasDestination(dest, ConsoleLogDest::File));
    EXPECT_TRUE(hasDestination(dest, ConsoleLogDest::Screen));
}

TEST(ConsoleLogDestTest, HasDestination) {
    EXPECT_TRUE(hasDestination(ConsoleLogDest::Both, ConsoleLogDest::File));
    EXPECT_TRUE(hasDestination(ConsoleLogDest::Both, ConsoleLogDest::Screen));
    EXPECT_FALSE(hasDestination(ConsoleLogDest::File, ConsoleLogDest::Screen));
}

// ============================================================================
// ConsoleLogConfig Tests
// ============================================================================

TEST(ConsoleLogConfigTest, ShouldLogToFile) {
    ConsoleLogConfig config;
    config.enabled = true;
    config.destination = ConsoleLogDest::File;

    EXPECT_TRUE(config.shouldLogToFile());
    EXPECT_FALSE(config.shouldLogToScreen());
}

TEST(ConsoleLogConfigTest, ShouldLogToBoth) {
    ConsoleLogConfig config;
    config.enabled = true;
    config.destination = ConsoleLogDest::Both;

    EXPECT_TRUE(config.shouldLogToFile());
    EXPECT_TRUE(config.shouldLogToScreen());
}

TEST(ConsoleLogConfigTest, DisabledLogsNothing) {
    ConsoleLogConfig config;
    config.enabled = false;
    config.destination = ConsoleLogDest::Both;

    EXPECT_FALSE(config.shouldLogToFile());
    EXPECT_FALSE(config.shouldLogToScreen());
}

// ============================================================================
// Factory Function Tests
// ============================================================================

TEST(LoggingFactoryTest, CreateDefaultLogConfigs) {
    auto configs = createDefaultLogConfigs();
    
    EXPECT_EQ(configs.size(), static_cast<size_t>(logging_limits::MAX_LOG_TYPES));
    EXPECT_EQ(configs[0].filename, "console.log");
    EXPECT_FALSE(configs[0].messageOn.empty());
}

TEST(LoggingFactoryTest, CreateDefaultIgnoreState) {
    auto state = createDefaultIgnoreState(32);
    
    EXPECT_EQ(state.playerIgnores.size(), 32u);
    EXPECT_EQ(state.mode, IgnoreMode::Disabled);
}

TEST(LoggingFactoryTest, FormatLogEntry) {
    LogEntry entry;
    entry.message = "Test message";
    entry.timestamp = 65.5;  // 1:05

    auto formatted = formatLogEntry(entry, true);
    EXPECT_NE(formatted.find("01:05"), std::string::npos);
    EXPECT_NE(formatted.find("Test message"), std::string::npos);
}

TEST(LoggingFactoryTest, FormatLogEntryNoTimestamp) {
    LogEntry entry;
    entry.message = "Test";
    entry.timestamp = 100.0;

    auto formatted = formatLogEntry(entry, false);
    EXPECT_EQ(formatted, "Test");
}
