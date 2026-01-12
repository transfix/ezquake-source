/**
 * @file system_test.cpp
 * @brief Tests for system and platform types
 */

#include <gtest/gtest.h>
#include "../../core/system/system_types.hpp"

using namespace ezquake::sys;

// =============================================================================
// Limits Tests
// =============================================================================

TEST(SystemLimitsTest, Constants) {
    EXPECT_EQ(limits::MAX_DIRFILES, 4096);
    EXPECT_EQ(limits::MAX_DEMO_NAME, 196);
    EXPECT_GE(limits::MAX_PATH_LENGTH, 4096);
    EXPECT_GE(limits::MAX_FILENAME_LENGTH, 256);
}

// =============================================================================
// SystemTime Tests
// =============================================================================

TEST(SystemTimeTest, DefaultConstruction) {
    SystemTime time;
    EXPECT_EQ(time.year, 0);
    EXPECT_EQ(time.month, 0);
    EXPECT_EQ(time.day, 0);
    EXPECT_EQ(time.hour, 0);
    EXPECT_EQ(time.minute, 0);
    EXPECT_EQ(time.second, 0);
    EXPECT_EQ(time.milliseconds, 0);
}

TEST(SystemTimeTest, IsValid) {
    SystemTime time;
    EXPECT_FALSE(time.isValid());
    
    time.year = 2024;
    time.month = 6;
    time.day = 15;
    EXPECT_TRUE(time.isValid());
    
    // Invalid month
    time.month = 13;
    EXPECT_FALSE(time.isValid());
}

TEST(SystemTimeTest, Comparison) {
    SystemTime a = makeSystemTime(2024, 6, 15, 12, 30, 0);
    SystemTime b = makeSystemTime(2024, 6, 15, 12, 30, 0);
    SystemTime c = makeSystemTime(2024, 6, 15, 12, 31, 0);
    
    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_LT(a, c);
    EXPECT_LE(a, b);
    EXPECT_GT(c, a);
    EXPECT_GE(c, a);
}

TEST(SystemTimeTest, TotalSeconds) {
    SystemTime time;
    time.hour = 1;
    time.minute = 30;
    time.second = 45;
    
    EXPECT_EQ(time.totalSeconds(), 1 * 3600 + 30 * 60 + 45);
}

TEST(SystemTimeTest, TotalMilliseconds) {
    SystemTime time;
    time.hour = 0;
    time.minute = 0;
    time.second = 1;
    time.milliseconds = 500;
    
    EXPECT_EQ(time.totalMilliseconds(), 1500);
}

// =============================================================================
// TimerResolutionSession Tests
// =============================================================================

TEST(TimerResolutionSessionTest, DefaultConstruction) {
    TimerResolutionSession session;
    EXPECT_FALSE(session.isSet);
    EXPECT_EQ(session.intervalMs, 0u);
    EXPECT_FALSE(session.isActive());
}

TEST(TimerResolutionSessionTest, IsActive) {
    TimerResolutionSession session;
    EXPECT_FALSE(session.isActive());
    
    session.isSet = true;
    EXPECT_FALSE(session.isActive());  // No interval
    
    session.intervalMs = 1;
    EXPECT_TRUE(session.isActive());
}

TEST(TimerResolutionSessionTest, Reset) {
    TimerResolutionSession session;
    session.isSet = true;
    session.intervalMs = 10;
    
    session.reset();
    
    EXPECT_FALSE(session.isSet);
    EXPECT_EQ(session.intervalMs, 0u);
}

// =============================================================================
// FileInfo Tests
// =============================================================================

TEST(FileInfoTest, DefaultConstruction) {
    FileInfo info;
    EXPECT_EQ(info.size, 0);
    EXPECT_EQ(info.modTime, 0);
    EXPECT_FALSE(info.isDirectory);
    EXPECT_FALSE(info.isValid());
}

TEST(FileInfoTest, SetGetName) {
    FileInfo info;
    info.setName("test.dem");
    
    EXPECT_EQ(info.getName(), "test.dem");
    EXPECT_TRUE(info.isValid());
}

TEST(FileInfoTest, IsFile) {
    FileInfo info;
    info.setName("file.txt");
    
    EXPECT_TRUE(info.isFile());
    
    info.isDirectory = true;
    EXPECT_FALSE(info.isFile());
}

TEST(FileInfoTest, NameTruncation) {
    FileInfo info;
    std::string longName(limits::MAX_DEMO_NAME + 50, 'a');
    info.setName(longName);
    
    EXPECT_LT(info.getName().size(), limits::MAX_DEMO_NAME);
}

// =============================================================================
// OSType Tests
// =============================================================================

TEST(OSTypeTest, Names) {
    EXPECT_STREQ(getOSName(OSType::Windows), "Windows");
    EXPECT_STREQ(getOSName(OSType::Linux), "Linux");
    EXPECT_STREQ(getOSName(OSType::MacOS), "macOS");
    EXPECT_STREQ(getOSName(OSType::FreeBSD), "FreeBSD");
    EXPECT_STREQ(getOSName(OSType::Unknown), "Unknown");
}

TEST(OSTypeTest, GetCurrentOS) {
    OSType os = getCurrentOS();
    // Just verify it returns something valid
    EXPECT_NE(getOSName(os), nullptr);
}

// =============================================================================
// CPUArch Tests
// =============================================================================

TEST(CPUArchTest, Names) {
    EXPECT_STREQ(getCPUArchName(CPUArch::x86), "x86");
    EXPECT_STREQ(getCPUArchName(CPUArch::x86_64), "x86_64");
    EXPECT_STREQ(getCPUArchName(CPUArch::ARM), "ARM");
    EXPECT_STREQ(getCPUArchName(CPUArch::ARM64), "ARM64");
    EXPECT_STREQ(getCPUArchName(CPUArch::Unknown), "Unknown");
}

TEST(CPUArchTest, GetCurrentArch) {
    CPUArch arch = getCurrentCPUArch();
    EXPECT_NE(getCPUArchName(arch), nullptr);
}

// =============================================================================
// BuildType Tests
// =============================================================================

TEST(BuildTypeTest, Names) {
    EXPECT_STREQ(getBuildTypeName(BuildType::Debug), "Debug");
    EXPECT_STREQ(getBuildTypeName(BuildType::Release), "Release");
    EXPECT_STREQ(getBuildTypeName(BuildType::RelWithDebInfo), "RelWithDebInfo");
    EXPECT_STREQ(getBuildTypeName(BuildType::MinSizeRel), "MinSizeRel");
}

// =============================================================================
// PlatformInfo Tests
// =============================================================================

TEST(PlatformInfoTest, DefaultConstruction) {
    PlatformInfo info;
    EXPECT_EQ(info.os, getCurrentOS());
    EXPECT_EQ(info.arch, getCurrentCPUArch());
    EXPECT_EQ(info.is64Bit, sizeof(void*) == 8);
}

TEST(PlatformInfoTest, Is32Bit) {
    PlatformInfo info;
    info.is64Bit = true;
    EXPECT_FALSE(info.is32Bit());
    
    info.is64Bit = false;
    EXPECT_TRUE(info.is32Bit());
}

TEST(PlatformInfoTest, HasSIMD) {
    PlatformInfo info;
    EXPECT_FALSE(info.hasSIMD());
    
    info.hasSSE = true;
    EXPECT_TRUE(info.hasSIMD());
    
    info.hasSSE = false;
    info.hasAVX = true;
    EXPECT_TRUE(info.hasSIMD());
}

// =============================================================================
// ErrorLevel Tests
// =============================================================================

TEST(ErrorLevelTest, Names) {
    EXPECT_STREQ(getErrorLevelName(ErrorLevel::None), "none");
    EXPECT_STREQ(getErrorLevelName(ErrorLevel::Warning), "warning");
    EXPECT_STREQ(getErrorLevelName(ErrorLevel::Error), "error");
    EXPECT_STREQ(getErrorLevelName(ErrorLevel::Fatal), "fatal");
}

TEST(ErrorLevelTest, ShouldTerminate) {
    EXPECT_FALSE(shouldTerminate(ErrorLevel::None));
    EXPECT_FALSE(shouldTerminate(ErrorLevel::Warning));
    EXPECT_FALSE(shouldTerminate(ErrorLevel::Error));
    EXPECT_TRUE(shouldTerminate(ErrorLevel::Fatal));
}

// =============================================================================
// SystemError Tests
// =============================================================================

TEST(SystemErrorTest, DefaultConstruction) {
    SystemError err;
    EXPECT_EQ(err.level, ErrorLevel::None);
    EXPECT_EQ(err.code, 0);
    EXPECT_FALSE(err.hasError());
}

TEST(SystemErrorTest, Warning) {
    auto err = SystemError::warning("Test warning", 42);
    EXPECT_EQ(err.level, ErrorLevel::Warning);
    EXPECT_EQ(err.code, 42);
    EXPECT_EQ(err.getMessage(), "Test warning");
    EXPECT_TRUE(err.hasError());
}

TEST(SystemErrorTest, Error) {
    auto err = SystemError::error("Test error", 100);
    EXPECT_EQ(err.level, ErrorLevel::Error);
    EXPECT_EQ(err.code, 100);
    EXPECT_EQ(err.getMessage(), "Test error");
}

TEST(SystemErrorTest, Fatal) {
    auto err = SystemError::fatal("Test fatal");
    EXPECT_EQ(err.level, ErrorLevel::Fatal);
    EXPECT_EQ(err.getMessage(), "Test fatal");
}

// =============================================================================
// ThreadPriority Tests
// =============================================================================

TEST(ThreadPriorityTest, Names) {
    EXPECT_STREQ(getThreadPriorityName(ThreadPriority::Lowest), "lowest");
    EXPECT_STREQ(getThreadPriorityName(ThreadPriority::Normal), "normal");
    EXPECT_STREQ(getThreadPriorityName(ThreadPriority::Highest), "highest");
    EXPECT_STREQ(getThreadPriorityName(ThreadPriority::TimeCritical), "time_critical");
}

// =============================================================================
// ThreadInfo Tests
// =============================================================================

TEST(ThreadInfoTest, DefaultConstruction) {
    ThreadInfo info;
    EXPECT_EQ(info.id, 0u);
    EXPECT_EQ(info.priority, ThreadPriority::Normal);
    EXPECT_FALSE(info.isMainThread);
    EXPECT_FALSE(info.isValid());
}

TEST(ThreadInfoTest, IsValid) {
    ThreadInfo info;
    EXPECT_FALSE(info.isValid());
    
    info.id = 1;
    EXPECT_TRUE(info.isValid());
}

// =============================================================================
// PerformanceCounter Tests
// =============================================================================

TEST(PerformanceCounterTest, DefaultConstruction) {
    PerformanceCounter counter;
    EXPECT_EQ(counter.frequency, 0);
    EXPECT_EQ(counter.startCount, 0);
    EXPECT_EQ(counter.currentCount, 0);
}

TEST(PerformanceCounterTest, ElapsedSeconds) {
    PerformanceCounter counter;
    counter.frequency = 1000000;  // 1 MHz
    counter.startCount = 0;
    counter.currentCount = 1000000;  // 1 second elapsed
    
    EXPECT_DOUBLE_EQ(counter.elapsedSeconds(), 1.0);
}

TEST(PerformanceCounterTest, ElapsedMilliseconds) {
    PerformanceCounter counter;
    counter.frequency = 1000;  // 1 kHz
    counter.startCount = 0;
    counter.currentCount = 100;  // 100 ms elapsed
    
    EXPECT_DOUBLE_EQ(counter.elapsedMilliseconds(), 100.0);
}

TEST(PerformanceCounterTest, ElapsedMicroseconds) {
    PerformanceCounter counter;
    counter.frequency = 1000000;  // 1 MHz
    counter.startCount = 0;
    counter.currentCount = 500;  // 500 us elapsed
    
    EXPECT_DOUBLE_EQ(counter.elapsedMicroseconds(), 500.0);
}

TEST(PerformanceCounterTest, Reset) {
    PerformanceCounter counter;
    counter.frequency = 1000;
    counter.startCount = 0;
    counter.currentCount = 500;
    
    counter.reset();
    
    EXPECT_EQ(counter.startCount, 500);
    EXPECT_DOUBLE_EQ(counter.elapsedMilliseconds(), 0.0);
}

// =============================================================================
// FrameTiming Tests
// =============================================================================

TEST(FrameTimingTest, DefaultConstruction) {
    FrameTiming timing;
    EXPECT_DOUBLE_EQ(timing.realTime, 0.0);
    EXPECT_DOUBLE_EQ(timing.frameTime, 0.0);
    EXPECT_DOUBLE_EQ(timing.gameTime, 0.0);
    EXPECT_EQ(timing.frameCount, 0u);
}

TEST(FrameTimingTest, Update) {
    FrameTiming timing;
    
    // First frame at t=0.016 (60 FPS)
    timing.update(0.016, 0.016);
    
    EXPECT_DOUBLE_EQ(timing.realTime, 0.016);
    EXPECT_DOUBLE_EQ(timing.frameTime, 0.016);
    EXPECT_EQ(timing.frameCount, 1u);
    EXPECT_NEAR(timing.instantFps, 62.5, 0.1);
}

TEST(FrameTimingTest, Reset) {
    FrameTiming timing;
    timing.update(1.0, 1.0);
    
    timing.reset();
    
    EXPECT_DOUBLE_EQ(timing.realTime, 0.0);
    EXPECT_EQ(timing.frameCount, 0u);
}

// =============================================================================
// Helper Function Tests
// =============================================================================

TEST(SystemHelpersTest, MakeSystemTime) {
    auto time = makeSystemTime(2024, 6, 15, 12, 30, 45, 500);
    
    EXPECT_EQ(time.year, 2024);
    EXPECT_EQ(time.month, 6);
    EXPECT_EQ(time.day, 15);
    EXPECT_EQ(time.hour, 12);
    EXPECT_EQ(time.minute, 30);
    EXPECT_EQ(time.second, 45);
    EXPECT_EQ(time.milliseconds, 500);
}

TEST(SystemHelpersTest, DiffSeconds) {
    auto a = makeSystemTime(2024, 6, 15, 12, 0, 0);
    auto b = makeSystemTime(2024, 6, 15, 12, 0, 30);
    
    EXPECT_EQ(diffSeconds(b, a), 30);
    EXPECT_EQ(diffSeconds(a, b), -30);
}

TEST(SystemHelpersTest, FormatTimeHMS) {
    auto time = makeSystemTime(2024, 6, 15, 8, 5, 3);
    auto formatted = formatTimeHMS(time);
    
    EXPECT_STREQ(formatted.data(), "08:05:03");
}

TEST(SystemHelpersTest, FormatTimeHMSMidnight) {
    auto time = makeSystemTime(2024, 1, 1, 0, 0, 0);
    auto formatted = formatTimeHMS(time);
    
    EXPECT_STREQ(formatted.data(), "00:00:00");
}

TEST(SystemHelpersTest, FormatTimeHMSMax) {
    auto time = makeSystemTime(2024, 12, 31, 23, 59, 59);
    auto formatted = formatTimeHMS(time);
    
    EXPECT_STREQ(formatted.data(), "23:59:59");
}
