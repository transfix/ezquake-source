/**
 * @file demo_test.cpp
 * @brief Comprehensive tests for the demo subsystem
 * 
 * Tests covering:
 * - Demo message types
 * - Demo format detection
 * - Playback state machine
 * - Seeking functionality
 * - Recording functionality
 * - Timedemo stats
 * - Demo cache
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <filesystem>
#include <fstream>
#include "../../src_cpp/core/demo/demo.hpp"

namespace ezquake::test {

using namespace testing;

//=============================================================================
// Demo Message Type Tests
//=============================================================================

class DemoMessageTypeTest : public Test {};

TEST_F(DemoMessageTypeTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(DemoMessageType::Cmd), 0);
    EXPECT_EQ(static_cast<uint8_t>(DemoMessageType::Read), 1);
    EXPECT_EQ(static_cast<uint8_t>(DemoMessageType::Set), 2);
    EXPECT_EQ(static_cast<uint8_t>(DemoMessageType::Multiple), 3);
    EXPECT_EQ(static_cast<uint8_t>(DemoMessageType::Single), 4);
    EXPECT_EQ(static_cast<uint8_t>(DemoMessageType::Stats), 5);
    EXPECT_EQ(static_cast<uint8_t>(DemoMessageType::All), 6);
}

TEST_F(DemoMessageTypeTest, Names) {
    EXPECT_EQ(demoMessageTypeName(DemoMessageType::Cmd), "cmd");
    EXPECT_EQ(demoMessageTypeName(DemoMessageType::Read), "read");
    EXPECT_EQ(demoMessageTypeName(DemoMessageType::Set), "set");
    EXPECT_EQ(demoMessageTypeName(DemoMessageType::Multiple), "multiple");
    EXPECT_EQ(demoMessageTypeName(DemoMessageType::Single), "single");
    EXPECT_EQ(demoMessageTypeName(DemoMessageType::Stats), "stats");
    EXPECT_EQ(demoMessageTypeName(DemoMessageType::All), "all");
}

TEST_F(DemoMessageTypeTest, IsMvdMessageType) {
    EXPECT_FALSE(isMvdMessageType(DemoMessageType::Cmd));
    EXPECT_FALSE(isMvdMessageType(DemoMessageType::Read));
    EXPECT_FALSE(isMvdMessageType(DemoMessageType::Set));
    EXPECT_TRUE(isMvdMessageType(DemoMessageType::Multiple));
    EXPECT_TRUE(isMvdMessageType(DemoMessageType::Single));
    EXPECT_TRUE(isMvdMessageType(DemoMessageType::Stats));
    EXPECT_TRUE(isMvdMessageType(DemoMessageType::All));
}

//=============================================================================
// Demo Format Tests
//=============================================================================

class DemoFormatTest : public Test {};

TEST_F(DemoFormatTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(DemoFormat::None), 0);
    EXPECT_EQ(static_cast<uint8_t>(DemoFormat::Qwd), 1);
    EXPECT_EQ(static_cast<uint8_t>(DemoFormat::Mvd), 2);
    EXPECT_EQ(static_cast<uint8_t>(DemoFormat::Qwz), 3);
    EXPECT_EQ(static_cast<uint8_t>(DemoFormat::Dem), 4);
    EXPECT_EQ(static_cast<uint8_t>(DemoFormat::Qtv), 5);
}

TEST_F(DemoFormatTest, Extensions) {
    EXPECT_EQ(demoFormatExtension(DemoFormat::Qwd), ".qwd");
    EXPECT_EQ(demoFormatExtension(DemoFormat::Mvd), ".mvd");
    EXPECT_EQ(demoFormatExtension(DemoFormat::Qwz), ".qwz");
    EXPECT_EQ(demoFormatExtension(DemoFormat::Dem), ".dem");
    EXPECT_EQ(demoFormatExtension(DemoFormat::Qtv), "");
    EXPECT_EQ(demoFormatExtension(DemoFormat::None), "");
}

TEST_F(DemoFormatTest, Names) {
    EXPECT_EQ(demoFormatName(DemoFormat::None), "none");
    EXPECT_EQ(demoFormatName(DemoFormat::Qwd), "QWD");
    EXPECT_EQ(demoFormatName(DemoFormat::Mvd), "MVD");
    EXPECT_EQ(demoFormatName(DemoFormat::Qwz), "QWZ");
    EXPECT_EQ(demoFormatName(DemoFormat::Dem), "DEM");
    EXPECT_EQ(demoFormatName(DemoFormat::Qtv), "QTV");
}

TEST_F(DemoFormatTest, DetectFromFilename) {
    EXPECT_EQ(detectDemoFormat("demo.qwd"), DemoFormat::Qwd);
    EXPECT_EQ(detectDemoFormat("demo.QWD"), DemoFormat::Qwd);
    EXPECT_EQ(detectDemoFormat("demo.mvd"), DemoFormat::Mvd);
    EXPECT_EQ(detectDemoFormat("demo.MVD"), DemoFormat::Mvd);
    EXPECT_EQ(detectDemoFormat("demo.qwz"), DemoFormat::Qwz);
    EXPECT_EQ(detectDemoFormat("demo.dem"), DemoFormat::Dem);
    EXPECT_EQ(detectDemoFormat("demo.txt"), DemoFormat::None);
    EXPECT_EQ(detectDemoFormat("abc"), DemoFormat::None);
}

//=============================================================================
// Demo Playback Mode Tests
//=============================================================================

class DemoPlaybackModeTest : public Test {};

TEST_F(DemoPlaybackModeTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(DemoPlaybackMode::None), 0);
    EXPECT_EQ(static_cast<uint8_t>(DemoPlaybackMode::Qwd), 1);
    EXPECT_EQ(static_cast<uint8_t>(DemoPlaybackMode::Nq), 2);
    EXPECT_EQ(static_cast<uint8_t>(DemoPlaybackMode::MvdFile), 3);
    EXPECT_EQ(static_cast<uint8_t>(DemoPlaybackMode::MvdQtv), 4);
}

TEST_F(DemoPlaybackModeTest, IsPlayingDemo) {
    EXPECT_FALSE(isPlayingDemo(DemoPlaybackMode::None));
    EXPECT_TRUE(isPlayingDemo(DemoPlaybackMode::Qwd));
    EXPECT_TRUE(isPlayingDemo(DemoPlaybackMode::Nq));
    EXPECT_TRUE(isPlayingDemo(DemoPlaybackMode::MvdFile));
    EXPECT_TRUE(isPlayingDemo(DemoPlaybackMode::MvdQtv));
}

TEST_F(DemoPlaybackModeTest, IsMvdPlayback) {
    EXPECT_FALSE(isMvdPlayback(DemoPlaybackMode::None));
    EXPECT_FALSE(isMvdPlayback(DemoPlaybackMode::Qwd));
    EXPECT_FALSE(isMvdPlayback(DemoPlaybackMode::Nq));
    EXPECT_TRUE(isMvdPlayback(DemoPlaybackMode::MvdFile));
    EXPECT_TRUE(isMvdPlayback(DemoPlaybackMode::MvdQtv));
}

TEST_F(DemoPlaybackModeTest, IsQtvPlayback) {
    EXPECT_FALSE(isQtvPlayback(DemoPlaybackMode::None));
    EXPECT_FALSE(isQtvPlayback(DemoPlaybackMode::MvdFile));
    EXPECT_TRUE(isQtvPlayback(DemoPlaybackMode::MvdQtv));
}

//=============================================================================
// Demo Seek Type Tests
//=============================================================================

class DemoSeekTypeTest : public Test {};

TEST_F(DemoSeekTypeTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(DemoSeekType::None), 0);
    EXPECT_EQ(static_cast<uint8_t>(DemoSeekType::Normal), 1);
    EXPECT_EQ(static_cast<uint8_t>(DemoSeekType::DemoMark), 2);
    EXPECT_EQ(static_cast<uint8_t>(DemoSeekType::Status), 3);
    EXPECT_EQ(static_cast<uint8_t>(DemoSeekType::Found), 4);
    EXPECT_EQ(static_cast<uint8_t>(DemoSeekType::End), 5);
    EXPECT_EQ(static_cast<uint8_t>(DemoSeekType::FoundNoRewind), 6);
}

TEST_F(DemoSeekTypeTest, IsSeeking) {
    EXPECT_FALSE(isSeeking(DemoSeekType::None));
    EXPECT_TRUE(isSeeking(DemoSeekType::Normal));
    EXPECT_TRUE(isSeeking(DemoSeekType::DemoMark));
    EXPECT_TRUE(isSeeking(DemoSeekType::Status));
}

TEST_F(DemoSeekTypeTest, IsSeekingComplete) {
    EXPECT_FALSE(isSeekingComplete(DemoSeekType::None));
    EXPECT_FALSE(isSeekingComplete(DemoSeekType::Normal));
    EXPECT_TRUE(isSeekingComplete(DemoSeekType::Found));
    EXPECT_TRUE(isSeekingComplete(DemoSeekType::FoundNoRewind));
}

//=============================================================================
// Seek Condition Tests
//=============================================================================

class SeekConditionTest : public Test {};

TEST_F(SeekConditionTest, EqualMatch) {
    SeekCondition cond{SeekMatchType::Equal, 0, 100};
    EXPECT_TRUE(cond.evaluate(100));
    EXPECT_FALSE(cond.evaluate(99));
}

TEST_F(SeekConditionTest, NotEqualMatch) {
    SeekCondition cond{SeekMatchType::NotEqual, 0, 100};
    EXPECT_TRUE(cond.evaluate(99));
    EXPECT_FALSE(cond.evaluate(100));
}

TEST_F(SeekConditionTest, LessThanMatch) {
    SeekCondition cond{SeekMatchType::LessThan, 0, 100};
    EXPECT_TRUE(cond.evaluate(99));
    EXPECT_FALSE(cond.evaluate(100));
    EXPECT_FALSE(cond.evaluate(101));
}

TEST_F(SeekConditionTest, GreaterThanMatch) {
    SeekCondition cond{SeekMatchType::GreaterThan, 0, 100};
    EXPECT_FALSE(cond.evaluate(99));
    EXPECT_FALSE(cond.evaluate(100));
    EXPECT_TRUE(cond.evaluate(101));
}

TEST_F(SeekConditionTest, BitOnMatch) {
    SeekCondition cond{SeekMatchType::BitOn, 0, 0x04};
    EXPECT_TRUE(cond.evaluate(0x04));
    EXPECT_TRUE(cond.evaluate(0xFF));
    EXPECT_FALSE(cond.evaluate(0x03));
}

TEST_F(SeekConditionTest, BitOffMatch) {
    SeekCondition cond{SeekMatchType::BitOff, 0, 0x04};
    EXPECT_FALSE(cond.evaluate(0x04));
    EXPECT_TRUE(cond.evaluate(0x03));
}

//=============================================================================
// Seek Status Tests
//=============================================================================

class SeekStatusTest : public Test {
protected:
    SeekStatus status_;
};

TEST_F(SeekStatusTest, DefaultState) {
    EXPECT_FALSE(status_.nonMatchingFound);
    EXPECT_FALSE(status_.hasConditions());
}

TEST_F(SeekStatusTest, AddConditions) {
    status_.conditions.push_back({SeekMatchType::Equal, 0, 100});
    EXPECT_TRUE(status_.hasConditions());
}

TEST_F(SeekStatusTest, Clear) {
    status_.nonMatchingFound = true;
    status_.conditions.push_back({SeekMatchType::Equal, 0, 100});
    
    status_.clear();
    
    EXPECT_FALSE(status_.nonMatchingFound);
    EXPECT_FALSE(status_.hasConditions());
}

//=============================================================================
// Demo Timing Tests
//=============================================================================

class DemoTimingTest : public Test {
protected:
    DemoTiming timing_;
};

TEST_F(DemoTimingTest, DefaultState) {
    EXPECT_DOUBLE_EQ(timing_.oldTime, 0.0);
    EXPECT_DOUBLE_EQ(timing_.nextTime, 0.0);
    EXPECT_DOUBLE_EQ(timing_.startTime, 0.0);
    EXPECT_DOUBLE_EQ(timing_.speed, 1.0);
    EXPECT_FLOAT_EQ(timing_.length, 0.0f);
}

TEST_F(DemoTimingTest, ProgressPercent) {
    timing_.startTime = 0.0;
    timing_.length = 100.0f;
    
    EXPECT_FLOAT_EQ(timing_.progressPercent(0.0), 0.0f);
    EXPECT_FLOAT_EQ(timing_.progressPercent(50.0), 50.0f);
    EXPECT_FLOAT_EQ(timing_.progressPercent(100.0), 100.0f);
}

TEST_F(DemoTimingTest, ProgressPercentClamped) {
    timing_.length = 100.0f;
    
    EXPECT_FLOAT_EQ(timing_.progressPercent(-10.0), 0.0f);
    EXPECT_FLOAT_EQ(timing_.progressPercent(200.0), 100.0f);
}

TEST_F(DemoTimingTest, ElapsedTime) {
    timing_.startTime = 10.0;
    
    EXPECT_DOUBLE_EQ(timing_.elapsedTime(10.0), 0.0);
    EXPECT_DOUBLE_EQ(timing_.elapsedTime(15.0), 5.0);
}

TEST_F(DemoTimingTest, IsPaused) {
    EXPECT_FALSE(timing_.isPaused());
    
    timing_.speed = 0.0;
    EXPECT_TRUE(timing_.isPaused());
}

TEST_F(DemoTimingTest, SetSpeedClamped) {
    timing_.setSpeed(-1.0);
    EXPECT_DOUBLE_EQ(timing_.speed, 0.0);
    
    timing_.setSpeed(200.0);
    EXPECT_DOUBLE_EQ(timing_.speed, 100.0);
    
    timing_.setSpeed(2.5);
    EXPECT_DOUBLE_EQ(timing_.speed, 2.5);
}

TEST_F(DemoTimingTest, Reset) {
    timing_.oldTime = 5.0;
    timing_.speed = 2.0;
    timing_.length = 100.0f;
    
    timing_.reset();
    
    EXPECT_DOUBLE_EQ(timing_.oldTime, 0.0);
    EXPECT_DOUBLE_EQ(timing_.speed, 1.0);
    EXPECT_FLOAT_EQ(timing_.length, 0.0f);
}

//=============================================================================
// Timedemo Stats Tests
//=============================================================================

class TimedemoStatsTest : public Test {
protected:
    TimedemoStats stats_;
};

TEST_F(TimedemoStatsTest, DefaultState) {
    EXPECT_EQ(stats_.mode, TimedemoMode::Off);
    EXPECT_FALSE(stats_.isActive());
}

TEST_F(TimedemoStatsTest, IsActive) {
    stats_.mode = TimedemoMode::Classic;
    EXPECT_TRUE(stats_.isActive());
    
    stats_.mode = TimedemoMode::FixedFps;
    EXPECT_TRUE(stats_.isActive());
}

TEST_F(TimedemoStatsTest, RecordFrametime) {
    stats_.recordFrametime(5.0, 100); // 5ms frame, frame 100
    
    EXPECT_EQ(stats_.frametimeStats[50], 1); // 5.0 * 10 = 50
    EXPECT_EQ(stats_.maxFrametimeMs, 5);
    EXPECT_EQ(stats_.maxFrametimeFrame, 100);
}

TEST_F(TimedemoStatsTest, RecordMultipleFrametimes) {
    stats_.recordFrametime(5.0, 1);
    stats_.recordFrametime(10.0, 2);
    stats_.recordFrametime(5.0, 3);
    
    EXPECT_EQ(stats_.frametimeStats[50], 2); // Two 5ms frames
    EXPECT_EQ(stats_.frametimeStats[100], 1); // One 10ms frame
    EXPECT_EQ(stats_.maxFrametimeMs, 10);
    EXPECT_EQ(stats_.maxFrametimeFrame, 2);
}

TEST_F(TimedemoStatsTest, CalculateFps) {
    stats_.startTime = 1.0;  // Non-zero start time
    stats_.startFrame = 0;
    
    double fps = stats_.calculateFps(100, 2.0); // 100 frames in 1 second
    EXPECT_DOUBLE_EQ(fps, 100.0);
}

TEST_F(TimedemoStatsTest, Reset) {
    stats_.mode = TimedemoMode::Classic;
    stats_.maxFrametimeMs = 100;
    
    stats_.reset();
    
    EXPECT_EQ(stats_.mode, TimedemoMode::Off);
    EXPECT_EQ(stats_.maxFrametimeMs, 0);
}

TEST_F(TimedemoStatsTest, Constants) {
    EXPECT_EQ(timedemo::DEFAULT_FPS, 308);
    EXPECT_EQ(timedemo::MIN_FPS, 20);
    EXPECT_EQ(timedemo::MAX_FPS, 10000);
}

//=============================================================================
// Recording State Tests
//=============================================================================

class RecordingStateTest : public Test {
protected:
    RecordingState state_;
};

TEST_F(RecordingStateTest, DefaultState) {
    EXPECT_EQ(state_.mode, RecordingMode::None);
    EXPECT_FALSE(state_.isRecording());
    EXPECT_TRUE(state_.filename.empty());
}

TEST_F(RecordingStateTest, IsRecording) {
    state_.mode = RecordingMode::Manual;
    EXPECT_TRUE(state_.isRecording());
    
    state_.mode = RecordingMode::Auto;
    EXPECT_TRUE(state_.isRecording());
}

TEST_F(RecordingStateTest, Reset) {
    state_.mode = RecordingMode::Manual;
    state_.filename = "test.qwd";
    state_.bytesWritten = 1000;
    
    state_.reset();
    
    EXPECT_EQ(state_.mode, RecordingMode::None);
    EXPECT_TRUE(state_.filename.empty());
    EXPECT_EQ(state_.bytesWritten, 0u);
}

//=============================================================================
// Playback State Tests
//=============================================================================

class PlaybackStateTest : public Test {
protected:
    PlaybackState state_;
};

TEST_F(PlaybackStateTest, DefaultState) {
    EXPECT_EQ(state_.mode, DemoPlaybackMode::None);
    EXPECT_FALSE(state_.isPlaying());
    EXPECT_FALSE(state_.isSeeking());
    EXPECT_FALSE(state_.isPaused());
}

TEST_F(PlaybackStateTest, IsPlaying) {
    state_.mode = DemoPlaybackMode::Qwd;
    EXPECT_TRUE(state_.isPlaying());
}

TEST_F(PlaybackStateTest, IsSeeking) {
    state_.seekType = DemoSeekType::Normal;
    EXPECT_TRUE(state_.isSeeking());
}

TEST_F(PlaybackStateTest, IsPaused) {
    state_.mode = DemoPlaybackMode::Qwd;
    state_.timing.speed = 0.0;
    state_.seekType = DemoSeekType::None;
    state_.timedemo.mode = TimedemoMode::Off;
    
    EXPECT_TRUE(state_.isPaused());
}

TEST_F(PlaybackStateTest, NotPausedWhileSeeking) {
    state_.mode = DemoPlaybackMode::Qwd;
    state_.timing.speed = 0.0;
    state_.seekType = DemoSeekType::Normal;
    
    EXPECT_FALSE(state_.isPaused());
}

TEST_F(PlaybackStateTest, Reset) {
    state_.mode = DemoPlaybackMode::Qwd;
    state_.filename = "test.mvd";
    state_.seekType = DemoSeekType::Normal;
    
    state_.reset();
    
    EXPECT_EQ(state_.mode, DemoPlaybackMode::None);
    EXPECT_TRUE(state_.filename.empty());
    EXPECT_EQ(state_.seekType, DemoSeekType::None);
}

//=============================================================================
// QTV Info Tests
//=============================================================================

class QtvInfoTest : public Test {
protected:
    QtvInfo info_;
};

TEST_F(QtvInfoTest, DefaultState) {
    EXPECT_EQ(info_.state, QtvState::Disconnected);
    EXPECT_FALSE(info_.isConnected());
    EXPECT_FALSE(info_.isPlaying());
}

TEST_F(QtvInfoTest, IsConnected) {
    info_.state = QtvState::Connecting;
    EXPECT_TRUE(info_.isConnected());
}

TEST_F(QtvInfoTest, IsPlaying) {
    info_.state = QtvState::Playing;
    EXPECT_TRUE(info_.isPlaying());
}

TEST_F(QtvInfoTest, BufferProgress) {
    info_.bufferSize = 50;
    info_.bufferTarget = 100;
    
    EXPECT_FLOAT_EQ(info_.bufferProgress(), 50.0f);
}

TEST_F(QtvInfoTest, BufferProgressNoTarget) {
    info_.bufferTarget = 0;
    EXPECT_FLOAT_EQ(info_.bufferProgress(), 100.0f);
}

TEST_F(QtvInfoTest, Reset) {
    info_.state = QtvState::Playing;
    info_.source = "qtv://server:27999";
    
    info_.reset();
    
    EXPECT_EQ(info_.state, QtvState::Disconnected);
    EXPECT_TRUE(info_.source.empty());
}

//=============================================================================
// NQ Demo State Tests
//=============================================================================

class NqDemoStateTest : public Test {
protected:
    NqDemoState state_;
};

TEST_F(NqDemoStateTest, DefaultState) {
    EXPECT_EQ(state_.signon, 0);
    EXPECT_TRUE(state_.standardQuake);
    EXPECT_FALSE(state_.isSignonComplete());
}

TEST_F(NqDemoStateTest, IsSignonComplete) {
    state_.signon = 3;
    EXPECT_FALSE(state_.isSignonComplete());
    
    state_.signon = 4;
    EXPECT_TRUE(state_.isSignonComplete());
}

TEST_F(NqDemoStateTest, Constants) {
    EXPECT_EQ(nq_demo::PROTOCOL_VERSION, 15);
    EXPECT_EQ(nq_demo::MAX_CLIENTS, 16);
    EXPECT_EQ(nq_demo::SIGNONS, 4);
    EXPECT_EQ(nq_demo::MAX_EDICTS, 600);
}

TEST_F(NqDemoStateTest, UpdateFlags) {
    EXPECT_EQ(nq_demo::update::MOREBITS, (1 << 0));
    EXPECT_EQ(nq_demo::update::ORIGIN1, (1 << 1));
    EXPECT_EQ(nq_demo::update::MODEL, (1 << 10));
    EXPECT_EQ(nq_demo::update::LONGENTITY, (1 << 14));
}

//=============================================================================
// Demo Cache Tests
//=============================================================================

class DemoCacheTest : public Test {
protected:
    DemoCache cache_;
};

TEST_F(DemoCacheTest, DefaultState) {
    EXPECT_FALSE(cache_.isEnabled());
    EXPECT_EQ(cache_.size(), 0u);
    EXPECT_EQ(cache_.capacity(), 0u);
}

TEST_F(DemoCacheTest, Resize) {
    cache_.resize(1024);
    
    EXPECT_TRUE(cache_.isEnabled());
    EXPECT_GE(cache_.capacity(), 1024u);
}

TEST_F(DemoCacheTest, ResizeMinimumSize) {
    cache_.resize(100); // Less than MIN_SIZE
    
    EXPECT_GE(cache_.capacity(), demo_cache::MIN_SIZE);
}

TEST_F(DemoCacheTest, Write) {
    cache_.resize(1024);
    
    uint8_t data[] = {1, 2, 3, 4, 5};
    size_t written = cache_.write(data, 5);
    
    EXPECT_EQ(written, 5u);
    EXPECT_EQ(cache_.size(), 5u);
}

TEST_F(DemoCacheTest, WriteMultiple) {
    cache_.resize(1024);
    
    uint8_t data[] = {1, 2, 3};
    cache_.write(data, 3);
    cache_.write(data, 3);
    
    EXPECT_EQ(cache_.size(), 6u);
}

TEST_F(DemoCacheTest, Clear) {
    cache_.resize(1024);
    uint8_t data[] = {1, 2, 3};
    cache_.write(data, 3);
    
    cache_.clear();
    
    EXPECT_EQ(cache_.size(), 0u);
    EXPECT_TRUE(cache_.isEnabled());
}

TEST_F(DemoCacheTest, Available) {
    cache_.resize(demo_cache::MIN_SIZE);
    
    EXPECT_EQ(cache_.available(), cache_.capacity());
    
    uint8_t data[100];
    cache_.write(data, 100);
    
    EXPECT_EQ(cache_.available(), cache_.capacity() - 100);
}

TEST_F(DemoCacheTest, Flush) {
    cache_.resize(demo_cache::MIN_SIZE);
    uint8_t data[100];
    cache_.write(data, 100);
    
    cache_.flush(50);
    
    EXPECT_EQ(cache_.size(), 50u);
}

TEST_F(DemoCacheTest, FlushAll) {
    cache_.resize(demo_cache::MIN_SIZE);
    uint8_t data[100];
    cache_.write(data, 100);
    
    cache_.flush(100);
    
    EXPECT_EQ(cache_.size(), 0u);
}

TEST_F(DemoCacheTest, Constants) {
    EXPECT_EQ(demo_cache::MIN_SIZE, 2u * 1024 * 1024);
    EXPECT_EQ(demo_cache::FLUSH_SIZE, 1u * 1024 * 1024);
}

//=============================================================================
// DemoUserCmd Tests
//=============================================================================

class DemoUserCmdTest : public Test {
protected:
    DemoUserCmd cmd_;
};

TEST_F(DemoUserCmdTest, DefaultState) {
    EXPECT_FLOAT_EQ(cmd_.angles[0], 0.0f);
    EXPECT_EQ(cmd_.forwardMove, 0);
    EXPECT_EQ(cmd_.buttons, 0);
}

TEST_F(DemoUserCmdTest, SerializeDeserialize) {
    cmd_.angles[0] = 10.0f;
    cmd_.angles[1] = 20.0f;
    cmd_.angles[2] = 30.0f;
    cmd_.forwardMove = 400;
    cmd_.sideMove = -200;
    cmd_.upMove = 100;
    cmd_.buttons = 0x03;
    cmd_.impulse = 1;
    cmd_.msec = 16;
    
    auto data = cmd_.serialize();
    auto result = DemoUserCmd::deserialize(data.data(), data.size());
    
    ASSERT_TRUE(result.has_value());
    EXPECT_FLOAT_EQ(result->angles[0], 10.0f);
    EXPECT_FLOAT_EQ(result->angles[1], 20.0f);
    EXPECT_FLOAT_EQ(result->angles[2], 30.0f);
    EXPECT_EQ(result->forwardMove, 400);
    EXPECT_EQ(result->sideMove, -200);
    EXPECT_EQ(result->upMove, 100);
    EXPECT_EQ(result->buttons, 0x03);
    EXPECT_EQ(result->impulse, 1);
    EXPECT_EQ(result->msec, 16);
}

TEST_F(DemoUserCmdTest, DeserializeTooShort) {
    uint8_t data[10] = {0};
    auto result = DemoUserCmd::deserialize(data, 10);
    
    EXPECT_FALSE(result.has_value());
}

//=============================================================================
// DemoReadResult Tests
//=============================================================================

class DemoReadResultTest : public Test {};

TEST_F(DemoReadResultTest, OkResult) {
    std::vector<uint8_t> data = {1, 2, 3};
    auto result = DemoReadResult::ok(DemoMessageType::Read, 1.5f, data, 0);
    
    EXPECT_TRUE(result.success);
    EXPECT_FALSE(result.endOfDemo);
    EXPECT_EQ(result.type, DemoMessageType::Read);
    EXPECT_FLOAT_EQ(result.time, 1.5f);
    EXPECT_EQ(result.data.size(), 3u);
}

TEST_F(DemoReadResultTest, EofResult) {
    auto result = DemoReadResult::eof();
    
    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.endOfDemo);
}

TEST_F(DemoReadResultTest, ErrorResult) {
    auto result = DemoReadResult::error();
    
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.endOfDemo);
}

//=============================================================================
// DemoPlaylist Tests
//=============================================================================

class DemoPlaylistTest : public Test {
protected:
    DemoPlaylist playlist_;
};

TEST_F(DemoPlaylistTest, DefaultState) {
    EXPECT_TRUE(playlist_.empty());
    EXPECT_EQ(playlist_.size(), 0u);
    EXPECT_EQ(playlist_.index(), 0u);
    EXPECT_FALSE(playlist_.current().has_value());
}

TEST_F(DemoPlaylistTest, Add) {
    EXPECT_TRUE(playlist_.add("demo1.qwd"));
    EXPECT_TRUE(playlist_.add("demo2.qwd"));
    
    EXPECT_EQ(playlist_.size(), 2u);
    EXPECT_FALSE(playlist_.empty());
}

TEST_F(DemoPlaylistTest, Current) {
    playlist_.add("demo1.qwd");
    playlist_.add("demo2.qwd");
    
    auto current = playlist_.current();
    ASSERT_TRUE(current.has_value());
    EXPECT_EQ(*current, "demo1.qwd");
}

TEST_F(DemoPlaylistTest, Next) {
    playlist_.add("demo1.qwd");
    playlist_.add("demo2.qwd");
    
    EXPECT_TRUE(playlist_.next());
    EXPECT_EQ(playlist_.index(), 1u);
    EXPECT_EQ(*playlist_.current(), "demo2.qwd");
    
    EXPECT_FALSE(playlist_.next()); // At end
}

TEST_F(DemoPlaylistTest, Previous) {
    playlist_.add("demo1.qwd");
    playlist_.add("demo2.qwd");
    playlist_.next();
    
    EXPECT_TRUE(playlist_.previous());
    EXPECT_EQ(playlist_.index(), 0u);
    
    EXPECT_FALSE(playlist_.previous()); // At start
}

TEST_F(DemoPlaylistTest, AtEnd) {
    // Empty playlist is at end
    EXPECT_TRUE(playlist_.atEnd());
    
    playlist_.add("demo1.qwd");
    EXPECT_FALSE(playlist_.atEnd()); // index 0, size 1 - not past end
}

TEST_F(DemoPlaylistTest, Clear) {
    playlist_.add("demo1.qwd");
    playlist_.add("demo2.qwd");
    playlist_.next();
    
    playlist_.clear();
    
    EXPECT_TRUE(playlist_.empty());
    EXPECT_EQ(playlist_.index(), 0u);
}

TEST_F(DemoPlaylistTest, MaxDemos) {
    EXPECT_EQ(DemoPlaylist::MAX_DEMOS, 8u);
    EXPECT_EQ(DemoPlaylist::MAX_NAME_LENGTH, 16u);
    
    for (size_t i = 0; i < DemoPlaylist::MAX_DEMOS; ++i) {
        EXPECT_TRUE(playlist_.add("demo"));
    }
    EXPECT_FALSE(playlist_.add("one_more")); // Should fail
}

TEST_F(DemoPlaylistTest, NameTruncation) {
    playlist_.add("this_is_a_very_long_demo_name.qwd");
    
    auto current = playlist_.current();
    ASSERT_TRUE(current.has_value());
    EXPECT_LE(current->size(), DemoPlaylist::MAX_NAME_LENGTH);
}

//=============================================================================
// AutoRecorder Tests
//=============================================================================

class AutoRecorderTest : public Test {
protected:
    AutoRecorder recorder_;
};

TEST_F(AutoRecorderTest, DefaultState) {
    EXPECT_FALSE(recorder_.isEnabled());
    EXPECT_FALSE(recorder_.isRecording());
}

TEST_F(AutoRecorderTest, Enable) {
    recorder_.enable(true);
    EXPECT_TRUE(recorder_.isEnabled());
    
    recorder_.enable(false);
    EXPECT_FALSE(recorder_.isEnabled());
}

TEST_F(AutoRecorderTest, GenerateFilename) {
    auto name = AutoRecorder::generateFilename("dm4", "MyServer");
    
    EXPECT_TRUE(name.ends_with(".qwd"));
    EXPECT_TRUE(name.find("dm4") != std::string::npos);
    EXPECT_TRUE(name.find("MyServer") != std::string::npos);
}

TEST_F(AutoRecorderTest, GenerateFilenameNoServer) {
    auto name = AutoRecorder::generateFilename("dm6", "");
    
    EXPECT_TRUE(name.ends_with(".qwd"));
    EXPECT_TRUE(name.find("dm6") != std::string::npos);
}

} // namespace ezquake::test
