/**
 * @file tracker_test.cpp
 * @brief Unit tests for tracker types
 */

#include <gtest/gtest.h>
#include "../../core/tracker/tracker_types.hpp"

using namespace ezquake::tracker;

// ============================================================================
// SegmentFlag Tests
// ============================================================================

TEST(SegmentFlagTest, BitwiseOperations) {
    auto flags = SegmentFlag::Weapon | SegmentFlag::Highlight;
    
    EXPECT_TRUE(hasFlag(flags, SegmentFlag::Weapon));
    EXPECT_TRUE(hasFlag(flags, SegmentFlag::Highlight));
    EXPECT_FALSE(hasFlag(flags, SegmentFlag::Own));
}

TEST(SegmentFlagTest, None) {
    EXPECT_FALSE(hasFlag(SegmentFlag::None, SegmentFlag::Weapon));
}

// ============================================================================
// TrackerColor Tests
// ============================================================================

TEST(TrackerColorTest, DefaultConstruction) {
    TrackerColor color;
    EXPECT_TRUE(color.isWhite());
    EXPECT_FALSE(color.isTransparent());
}

TEST(TrackerColorTest, CustomColor) {
    TrackerColor color{255, 0, 0, 128};
    EXPECT_EQ(color.r, 255u);
    EXPECT_EQ(color.g, 0u);
    EXPECT_EQ(color.b, 0u);
    EXPECT_EQ(color.a, 128u);
    EXPECT_FALSE(color.isWhite());
}

TEST(TrackerColorTest, Transparency) {
    TrackerColor color{255, 255, 255, 0};
    EXPECT_TRUE(color.isTransparent());
}

TEST(TrackerColorTest, Equality) {
    TrackerColor a{100, 150, 200, 255};
    TrackerColor b{100, 150, 200, 255};
    TrackerColor c{100, 150, 200, 128};
    
    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

TEST(TrackerColorTest, StandardColors) {
    EXPECT_TRUE(colors::White.isWhite());
    EXPECT_EQ(colors::KilledYou.r, 255u);
    EXPECT_EQ(colors::KilledYou.g, 0u);
    EXPECT_EQ(colors::KilledYou.b, 0u);
}

// ============================================================================
// TrackerSegment Tests
// ============================================================================

TEST(TrackerSegmentTest, DefaultConstruction) {
    TrackerSegment seg;
    EXPECT_TRUE(seg.isEmpty());
    EXPECT_FALSE(seg.hasImage());
}

TEST(TrackerSegmentTest, TextConstruction) {
    TrackerSegment seg("Test text", colors::YouDied);
    EXPECT_EQ(seg.getText(), "Test text");
    EXPECT_EQ(seg.color, colors::YouDied);
    EXPECT_FALSE(seg.isEmpty());
}

TEST(TrackerSegmentTest, SetText) {
    TrackerSegment seg;
    seg.setText("Hello");
    EXPECT_EQ(seg.getText(), "Hello");
}

TEST(TrackerSegmentTest, ImageSegment) {
    TrackerSegment seg;
    seg.imageIndex = 5;
    
    EXPECT_TRUE(seg.hasImage());
    EXPECT_FALSE(seg.isEmpty());
}

TEST(TrackerSegmentTest, WeaponFlag) {
    TrackerSegment seg;
    seg.flags = SegmentFlag::Weapon;
    EXPECT_TRUE(seg.isWeapon());
}

// ============================================================================
// TrackerMessage Tests
// ============================================================================

TEST(TrackerMessageTest, DefaultConstruction) {
    TrackerMessage msg;
    EXPECT_TRUE(msg.isEmpty());
    EXPECT_EQ(msg.segmentCount, 0);
}

TEST(TrackerMessageTest, AddTextSegment) {
    TrackerMessage msg;
    EXPECT_TRUE(msg.addTextSegment("Hello", colors::White));
    
    EXPECT_FALSE(msg.isEmpty());
    EXPECT_EQ(msg.segmentCount, 1);
    EXPECT_EQ(msg.printableCharacters, 5);
}

TEST(TrackerMessageTest, AddImageSegment) {
    TrackerMessage msg;
    EXPECT_TRUE(msg.addImageSegment(3));
    
    EXPECT_EQ(msg.segmentCount, 1);
    EXPECT_EQ(msg.imageCharacters, 1);
    EXPECT_TRUE(msg.segments[0].hasImage());
}

TEST(TrackerMessageTest, MaxSegments) {
    TrackerMessage msg;
    
    for (int i = 0; i < tracker_limits::MAX_SEGMENTS_PER_LINE; ++i) {
        EXPECT_TRUE(msg.addTextSegment("x", colors::White));
    }
    
    EXPECT_FALSE(msg.addTextSegment("overflow", colors::White));
    EXPECT_EQ(msg.segmentCount, tracker_limits::MAX_SEGMENTS_PER_LINE);
}

TEST(TrackerMessageTest, IsActive) {
    TrackerMessage msg;
    msg.dieTime = 10.0f;
    
    EXPECT_TRUE(msg.isActive(5.0f));
    EXPECT_TRUE(msg.isActive(9.9f));
    EXPECT_FALSE(msg.isActive(10.0f));
    EXPECT_FALSE(msg.isActive(15.0f));
}

TEST(TrackerMessageTest, TotalCharacters) {
    TrackerMessage msg;
    msg.addTextSegment("Hello", colors::White);
    msg.addImageSegment(1);
    msg.addTextSegment("World", colors::White);
    
    EXPECT_EQ(msg.totalCharacters(), 10 + 1);
}

TEST(TrackerMessageTest, Clear) {
    TrackerMessage msg;
    msg.addTextSegment("Test", colors::White);
    msg.dieTime = 100.0f;
    msg.linked = true;
    
    msg.clear();
    
    EXPECT_TRUE(msg.isEmpty());
    EXPECT_FLOAT_EQ(msg.dieTime, 0.0f);
    EXPECT_FALSE(msg.linked);
}

// ============================================================================
// KillingStreak Tests
// ============================================================================

TEST(KillingStreakTest, Construction) {
    KillingStreak streak{10, "on a rampage", "rampage", "client/streak.wav"};
    
    EXPECT_EQ(streak.fragThreshold, 10);
    EXPECT_EQ(streak.displayString, "on a rampage");
    EXPECT_EQ(streak.internalName, "rampage");
}

TEST(KillingStreakTest, Matches) {
    KillingStreak streak{10, "rampage", "rampage", ""};
    
    EXPECT_FALSE(streak.matches(5));
    EXPECT_FALSE(streak.matches(9));
    EXPECT_TRUE(streak.matches(10));
    EXPECT_TRUE(streak.matches(15));
}

TEST(KillingStreakTest, DefaultStreaks) {
    auto streaks = createDefaultStreaks();
    
    EXPECT_EQ(streaks.size(), 6u);
    EXPECT_EQ(streaks[0].fragThreshold, 100);  // Highest first
    EXPECT_EQ(streaks[5].fragThreshold, 5);    // Lowest last
}

TEST(KillingStreakTest, FindStreak) {
    auto streaks = createDefaultStreaks();
    
    // No streak for low frags
    EXPECT_EQ(findStreak(streaks, 3), nullptr);
    
    // Find "killing spree" for 5+
    auto* streak5 = findStreak(streaks, 5);
    ASSERT_NE(streak5, nullptr);
    EXPECT_EQ(streak5->internalName, "spree");
    
    // Find higher streak for more frags
    auto* streak20 = findStreak(streaks, 20);
    ASSERT_NE(streak20, nullptr);
    EXPECT_EQ(streak20->internalName, "godlike");
}

// ============================================================================
// WeaponLabel Tests
// ============================================================================

TEST(WeaponLabelTest, DefaultConstruction) {
    WeaponLabel label;
    EXPECT_TRUE(label.label.empty());
    EXPECT_EQ(label.colorCount, 0);
}

TEST(WeaponLabelTest, AddColorSection) {
    WeaponLabel label("Rocket Launcher");
    label.addColorSection(0, colors::White);
    label.addColorSection(7, colors::KilledYou);
    
    EXPECT_EQ(label.colorCount, 2);
    EXPECT_EQ(label.colorStarts[0], 0);
    EXPECT_EQ(label.colorStarts[1], 7);
}

TEST(WeaponLabelTest, MaxColorSections) {
    WeaponLabel label;
    
    for (int i = 0; i < 6; ++i) {
        label.addColorSection(i * 10, colors::White);
    }
    
    EXPECT_EQ(label.colorCount, 4);  // Max is 4
}

// ============================================================================
// TrackerConfig Tests
// ============================================================================

TEST(TrackerConfigTest, Defaults) {
    TrackerConfig config;
    
    EXPECT_TRUE(config.enabled);
    EXPECT_TRUE(config.showFrags);
    EXPECT_FALSE(config.showStreaks);
    EXPECT_TRUE(config.alignRight);
    EXPECT_FLOAT_EQ(config.displayTime, 4.0f);
    EXPECT_EQ(config.maxMessages, 20);
}

// ============================================================================
// TrackerState Tests
// ============================================================================

TEST(TrackerStateTest, DefaultConstruction) {
    TrackerState state;
    
    EXPECT_EQ(state.activeTrack, 0);
    EXPECT_FALSE(state.streaks.empty());
    EXPECT_EQ(state.weaponImages[0], -1);
}

TEST(TrackerStateTest, NextMessage) {
    TrackerState state;
    state.setMaxMessages(5);
    
    auto* msg1 = state.nextMessage();
    ASSERT_NE(msg1, nullptr);
    EXPECT_EQ(state.activeTrack, 1);
    
    auto* msg2 = state.nextMessage();
    EXPECT_NE(msg1, msg2);
}

TEST(TrackerStateTest, SetMaxMessages) {
    TrackerState state;
    
    state.setMaxMessages(10);
    EXPECT_EQ(state.maxActiveTracks, 10);
    
    state.setMaxMessages(100);
    EXPECT_EQ(state.maxActiveTracks, tracker_limits::MAX_TRACKER_MESSAGES);
    
    state.setMaxMessages(-5);
    EXPECT_EQ(state.maxActiveTracks, 0);
}

TEST(TrackerStateTest, ActiveCount) {
    TrackerState state;
    state.setMaxMessages(10);
    
    auto* msg1 = state.nextMessage();
    msg1->dieTime = 100.0f;
    msg1->addTextSegment("Test", colors::White);
    
    auto* msg2 = state.nextMessage();
    msg2->dieTime = 50.0f;
    msg2->addTextSegment("Test2", colors::White);
    
    EXPECT_EQ(state.activeCount(40.0f), 2);
    EXPECT_EQ(state.activeCount(75.0f), 1);
    EXPECT_EQ(state.activeCount(150.0f), 0);
}

TEST(TrackerStateTest, ClearExpired) {
    TrackerState state;
    state.setMaxMessages(5);
    
    auto* msg = state.nextMessage();
    msg->dieTime = 10.0f;
    msg->addTextSegment("Will expire", colors::White);
    
    state.clearExpired(5.0f);
    EXPECT_FALSE(msg->isEmpty());  // Not expired yet
    
    state.clearExpired(15.0f);
    EXPECT_TRUE(msg->isEmpty());   // Now expired
}

TEST(TrackerStateTest, ClearAll) {
    TrackerState state;
    state.setMaxMessages(5);
    
    for (int i = 0; i < 3; ++i) {
        auto* msg = state.nextMessage();
        msg->dieTime = 100.0f;
        msg->addTextSegment("Test", colors::White);
    }
    
    state.clearAll();
    
    EXPECT_EQ(state.activeTrack, 0);
    EXPECT_EQ(state.activeCount(0.0f), 0);
}

// ============================================================================
// TrackerEventType Tests
// ============================================================================

TEST(TrackerEventTypeTest, Names) {
    EXPECT_EQ(trackerEventTypeName(TrackerEventType::Frag), "frag");
    EXPECT_EQ(trackerEventTypeName(TrackerEventType::Suicide), "suicide");
    EXPECT_EQ(trackerEventTypeName(TrackerEventType::FlagCapture), "flag_capture");
}

// ============================================================================
// TrackerEvent Tests
// ============================================================================

TEST(TrackerEventTest, DefaultConstruction) {
    TrackerEvent event;
    EXPECT_FALSE(event.isValid());
    EXPECT_FALSE(event.isFrag());
}

TEST(TrackerEventTest, FragEvent) {
    TrackerEvent event;
    event.type = TrackerEventType::Frag;
    event.attackerPlayer = 1;
    event.victimPlayer = 2;
    
    EXPECT_TRUE(event.isValid());
    EXPECT_TRUE(event.isFrag());
    EXPECT_FALSE(event.isNegative());
}

TEST(TrackerEventTest, DeathIsNegative) {
    TrackerEvent event;
    event.type = TrackerEventType::Death;
    
    EXPECT_TRUE(event.isNegative());
}

TEST(TrackerEventTest, TeamKillIsNegativeWhenOwn) {
    TrackerEvent event;
    event.type = TrackerEventType::TeamKill;
    event.isOwn = true;
    
    EXPECT_TRUE(event.isNegative());
    
    event.isOwn = false;
    EXPECT_FALSE(event.isNegative());
}

// ============================================================================
// Helper Function Tests
// ============================================================================

TEST(TrackerHelpersTest, ParseTrackerColor) {
    auto color = parseTrackerColor("090");
    ASSERT_TRUE(color.has_value());
    EXPECT_EQ(color->r, 0u);
    EXPECT_EQ(color->g, 255u);
    EXPECT_EQ(color->b, 0u);
    
    auto red = parseTrackerColor("900");
    ASSERT_TRUE(red.has_value());
    EXPECT_EQ(red->r, 255u);
    EXPECT_EQ(red->g, 0u);
}

TEST(TrackerHelpersTest, ParseTrackerColorInvalid) {
    EXPECT_FALSE(parseTrackerColor("ab").has_value());  // Too short
    EXPECT_FALSE(parseTrackerColor("xyz").has_value()); // Invalid chars
}

TEST(TrackerHelpersTest, ShouldFilterEvent) {
    TrackerConfig config;
    config.showFrags = true;
    config.showStreaks = false;
    config.showFlags = false;
    config.showPickups = false;
    
    EXPECT_FALSE(shouldFilterEvent(config, TrackerEventType::Frag));
    EXPECT_TRUE(shouldFilterEvent(config, TrackerEventType::StreakStart));
    EXPECT_TRUE(shouldFilterEvent(config, TrackerEventType::FlagCapture));
    EXPECT_TRUE(shouldFilterEvent(config, TrackerEventType::ItemPickup));
}

TEST(TrackerHelpersTest, CreateDefaultTrackerConfig) {
    auto config = createDefaultTrackerConfig();
    EXPECT_TRUE(config.enabled);
}
