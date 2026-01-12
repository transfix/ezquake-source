/**
 * @file trigger_test.cpp
 * @brief Unit tests for regex trigger and message trigger types
 */

#include <gtest/gtest.h>
#include "../../core/trigger/trigger_types.hpp"

using namespace ezquake::trigger;

// ============================================================================
// TriggerFlag Tests
// ============================================================================

TEST(TriggerFlagTest, BitwiseOr) {
    auto flags = TriggerFlag::PrintLow | TriggerFlag::PrintHigh;
    EXPECT_TRUE(hasFlag(flags, TriggerFlag::PrintLow));
    EXPECT_TRUE(hasFlag(flags, TriggerFlag::PrintHigh));
    EXPECT_FALSE(hasFlag(flags, TriggerFlag::PrintChat));
}

TEST(TriggerFlagTest, BitwiseAnd) {
    auto flags = TriggerFlag::PrintLow | TriggerFlag::PrintHigh | TriggerFlag::PrintChat;
    auto masked = flags & TriggerFlag::PrintHigh;
    EXPECT_TRUE(hasFlag(masked, TriggerFlag::PrintHigh));
    EXPECT_FALSE(hasFlag(masked, TriggerFlag::PrintLow));
}

TEST(TriggerFlagTest, BitwiseNot) {
    auto allFlags = TriggerFlag::PrintAll | TriggerFlag::Enabled;
    auto withoutChat = allFlags & ~TriggerFlag::PrintChat;
    EXPECT_TRUE(hasFlag(withoutChat, TriggerFlag::PrintLow));
    EXPECT_FALSE(hasFlag(withoutChat, TriggerFlag::PrintChat));
}

TEST(TriggerFlagTest, HasFlag) {
    EXPECT_TRUE(hasFlag(TriggerFlag::Enabled, TriggerFlag::Enabled));
    EXPECT_FALSE(hasFlag(TriggerFlag::Enabled, TriggerFlag::Final));
    EXPECT_FALSE(hasFlag(TriggerFlag::None, TriggerFlag::Enabled));
}

TEST(TriggerFlagTest, PrintAllIncludes) {
    EXPECT_TRUE(hasFlag(TriggerFlag::PrintAll, TriggerFlag::PrintLow));
    EXPECT_TRUE(hasFlag(TriggerFlag::PrintAll, TriggerFlag::PrintMedium));
    EXPECT_TRUE(hasFlag(TriggerFlag::PrintAll, TriggerFlag::PrintHigh));
    EXPECT_TRUE(hasFlag(TriggerFlag::PrintAll, TriggerFlag::PrintChat));
    EXPECT_TRUE(hasFlag(TriggerFlag::PrintAll, TriggerFlag::PrintCenter));
    EXPECT_FALSE(hasFlag(TriggerFlag::PrintAll, TriggerFlag::PrintInternal));
}

// ============================================================================
// MessageType Tests
// ============================================================================

TEST(MessageTypeTest, Names) {
    EXPECT_EQ(messageTypeName(MessageType::Low), "low");
    EXPECT_EQ(messageTypeName(MessageType::High), "high");
    EXPECT_EQ(messageTypeName(MessageType::Chat), "chat");
    EXPECT_EQ(messageTypeName(MessageType::Center), "center");
}

TEST(MessageTypeTest, ToFlag) {
    EXPECT_EQ(messageTypeToFlag(MessageType::Low), TriggerFlag::PrintLow);
    EXPECT_EQ(messageTypeToFlag(MessageType::High), TriggerFlag::PrintHigh);
    EXPECT_EQ(messageTypeToFlag(MessageType::Chat), TriggerFlag::PrintChat);
    EXPECT_EQ(messageTypeToFlag(MessageType::Internal), TriggerFlag::PrintInternal);
}

// ============================================================================
// TriggerMatchResult Tests
// ============================================================================

TEST(TriggerMatchResultTest, NoMatch) {
    auto result = TriggerMatchResult::noMatch();
    EXPECT_FALSE(result.matched);
    EXPECT_FALSE(result.shouldHide);
    EXPECT_FALSE(result.isFinal);
}

TEST(TriggerMatchResultTest, WasActionable) {
    TriggerMatchResult result;
    result.matched = true;
    result.shouldHide = false;
    EXPECT_TRUE(result.wasActionable());

    result.shouldHide = true;
    EXPECT_FALSE(result.wasActionable());

    result.matched = false;
    EXPECT_FALSE(result.wasActionable());
}

// ============================================================================
// RegexTrigger Tests
// ============================================================================

TEST(RegexTriggerTest, DefaultConstruction) {
    RegexTrigger trigger;
    EXPECT_TRUE(trigger.name.empty());
    EXPECT_TRUE(trigger.pattern.empty());
    EXPECT_FALSE(trigger.isValid());
}

TEST(RegexTriggerTest, IsEnabled) {
    RegexTrigger trigger;
    trigger.flags = TriggerFlag::Enabled;
    EXPECT_TRUE(trigger.isEnabled());

    trigger.flags = TriggerFlag::None;
    EXPECT_FALSE(trigger.isEnabled());
}

TEST(RegexTriggerTest, IsValid) {
    RegexTrigger trigger;
    trigger.name = "test";
    EXPECT_FALSE(trigger.isValid());  // No pattern

    trigger.pattern = ".*";
    EXPECT_TRUE(trigger.isValid());
}

TEST(RegexTriggerTest, MatchesMessageType) {
    RegexTrigger trigger;
    trigger.flags = TriggerFlag::PrintChat | TriggerFlag::PrintHigh;

    EXPECT_TRUE(trigger.matchesMessageType(MessageType::Chat));
    EXPECT_TRUE(trigger.matchesMessageType(MessageType::High));
    EXPECT_FALSE(trigger.matchesMessageType(MessageType::Low));
}

TEST(RegexTriggerTest, CanTrigger) {
    RegexTrigger trigger;
    trigger.flags = TriggerFlag::Enabled;
    trigger.minInterval = 0.0f;
    EXPECT_TRUE(trigger.canTrigger(0.0));

    trigger.minInterval = 1.0f;
    trigger.lastTriggerTime = 0.0;
    EXPECT_FALSE(trigger.canTrigger(0.5));  // Too soon
    EXPECT_TRUE(trigger.canTrigger(1.5));   // OK
}

TEST(RegexTriggerTest, CanTriggerDisabled) {
    RegexTrigger trigger;
    trigger.flags = TriggerFlag::None;  // Not enabled
    EXPECT_FALSE(trigger.canTrigger(0.0));
}

TEST(RegexTriggerTest, RecordTrigger) {
    RegexTrigger trigger;
    EXPECT_EQ(trigger.triggerCount, 0);

    trigger.recordTrigger(5.0);
    EXPECT_EQ(trigger.triggerCount, 1);
    EXPECT_DOUBLE_EQ(trigger.lastTriggerTime, 5.0);

    trigger.recordTrigger(10.0);
    EXPECT_EQ(trigger.triggerCount, 2);
}

TEST(RegexTriggerTest, Reset) {
    RegexTrigger trigger;
    trigger.lastTriggerTime = 100.0;
    trigger.triggerCount = 50;

    trigger.reset();
    EXPECT_DOUBLE_EQ(trigger.lastTriggerTime, 0.0);
    EXPECT_EQ(trigger.triggerCount, 0);
}

TEST(RegexTriggerTest, SetEnabled) {
    RegexTrigger trigger;
    trigger.flags = TriggerFlag::None;

    trigger.setEnabled(true);
    EXPECT_TRUE(trigger.isEnabled());

    trigger.setEnabled(false);
    EXPECT_FALSE(trigger.isEnabled());
}

// ============================================================================
// InternalTrigger Tests
// ============================================================================

TEST(InternalTriggerTest, IsValid) {
    InternalTrigger trigger;
    EXPECT_FALSE(trigger.isValid());

    trigger.pattern = ".*";
    EXPECT_FALSE(trigger.isValid());  // No callback

    trigger.callback = [](std::string_view) {};
    EXPECT_TRUE(trigger.isValid());
}

TEST(InternalTriggerTest, MatchesMessageType) {
    InternalTrigger trigger;
    trigger.flags = TriggerFlag::PrintInternal;

    EXPECT_TRUE(trigger.matchesMessageType(MessageType::Internal));
    EXPECT_FALSE(trigger.matchesMessageType(MessageType::Chat));
}

// ============================================================================
// TriggerState Tests
// ============================================================================

TEST(TriggerStateTest, UserTriggerCount) {
    TriggerState state;
    EXPECT_EQ(state.userTriggerCount(), 0u);

    state.userTriggers.push_back(RegexTrigger{});
    state.userTriggers.push_back(RegexTrigger{});
    EXPECT_EQ(state.userTriggerCount(), 2u);
}

TEST(TriggerStateTest, EnabledUserTriggerCount) {
    TriggerState state;

    RegexTrigger enabled;
    enabled.flags = TriggerFlag::Enabled;

    RegexTrigger disabled;
    disabled.flags = TriggerFlag::None;

    state.userTriggers.push_back(enabled);
    state.userTriggers.push_back(disabled);
    state.userTriggers.push_back(enabled);

    EXPECT_EQ(state.enabledUserTriggerCount(), 2u);
}

TEST(TriggerStateTest, FindUserTrigger) {
    TriggerState state;

    RegexTrigger t1;
    t1.name = "test1";

    RegexTrigger t2;
    t2.name = "test2";

    state.userTriggers.push_back(t1);
    state.userTriggers.push_back(t2);

    auto* found = state.findUserTrigger("test1");
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->name, "test1");

    EXPECT_EQ(state.findUserTrigger("nonexistent"), nullptr);
}

TEST(TriggerStateTest, RemoveUserTrigger) {
    TriggerState state;

    RegexTrigger t;
    t.name = "removeme";
    state.userTriggers.push_back(t);

    EXPECT_TRUE(state.removeUserTrigger("removeme"));
    EXPECT_EQ(state.userTriggerCount(), 0u);

    EXPECT_FALSE(state.removeUserTrigger("nonexistent"));
}

TEST(TriggerStateTest, ResetAllTriggerTimes) {
    TriggerState state;

    RegexTrigger t1, t2;
    t1.lastTriggerTime = 100.0;
    t2.lastTriggerTime = 200.0;

    state.userTriggers.push_back(t1);
    state.userTriggers.push_back(t2);

    state.resetAllTriggerTimes();

    EXPECT_DOUBLE_EQ(state.userTriggers[0].lastTriggerTime, 0.0);
    EXPECT_DOUBLE_EQ(state.userTriggers[1].lastTriggerTime, 0.0);
}

TEST(TriggerStateTest, Clear) {
    TriggerState state;
    state.userTriggers.push_back(RegexTrigger{});
    state.internalTriggers.push_back(InternalTrigger{});

    state.clearUserTriggers();
    EXPECT_EQ(state.userTriggerCount(), 0u);
    EXPECT_EQ(state.internalTriggerCount(), 1u);

    state.clearInternalTriggers();
    EXPECT_EQ(state.internalTriggerCount(), 0u);
}

TEST(TriggerStateTest, Reset) {
    TriggerState state;
    state.userTriggers.push_back(RegexTrigger{});
    state.enabled = false;
    state.processing = true;

    state.reset();

    EXPECT_EQ(state.userTriggerCount(), 0u);
    EXPECT_TRUE(state.enabled);
    EXPECT_FALSE(state.processing);
}

// ============================================================================
// MessageTrigger Tests
// ============================================================================

TEST(MessageTriggerTest, IsValid) {
    MessageTrigger trigger;
    EXPECT_FALSE(trigger.isValid());

    trigger.pattern = "test";
    EXPECT_FALSE(trigger.isValid());

    trigger.action = "echo matched";
    EXPECT_TRUE(trigger.isValid());
}

// ============================================================================
// TriggerStats Tests
// ============================================================================

TEST(TriggerStatsTest, RecordMessage) {
    TriggerStats stats;
    EXPECT_EQ(stats.messagesProcessed, 0u);

    stats.recordMessage();
    stats.recordMessage();
    EXPECT_EQ(stats.messagesProcessed, 2u);
}

TEST(TriggerStatsTest, RecordMatch) {
    TriggerStats stats;

    TriggerMatchResult noMatch;
    noMatch.matched = false;
    stats.recordMatch(noMatch);
    EXPECT_EQ(stats.triggersMatched, 0u);

    TriggerMatchResult match;
    match.matched = true;
    match.shouldHide = true;
    match.shouldNotLog = true;
    stats.recordMatch(match);

    EXPECT_EQ(stats.triggersMatched, 1u);
    EXPECT_EQ(stats.messagesHidden, 1u);
    EXPECT_EQ(stats.messagesNotLogged, 1u);
}

TEST(TriggerStatsTest, MatchRate) {
    TriggerStats stats;
    EXPECT_DOUBLE_EQ(stats.matchRate(), 0.0);

    stats.messagesProcessed = 100;
    stats.triggersMatched = 25;
    EXPECT_DOUBLE_EQ(stats.matchRate(), 0.25);
}

TEST(TriggerStatsTest, Reset) {
    TriggerStats stats;
    stats.messagesProcessed = 100;
    stats.triggersMatched = 50;

    stats.reset();
    EXPECT_EQ(stats.messagesProcessed, 0u);
    EXPECT_EQ(stats.triggersMatched, 0u);
}

// ============================================================================
// Helper Function Tests
// ============================================================================

TEST(TriggerHelpersTest, ParseTriggerFlags) {
    auto flags = parseTriggerFlags("lhc");
    EXPECT_TRUE(hasFlag(flags, TriggerFlag::PrintLow));
    EXPECT_TRUE(hasFlag(flags, TriggerFlag::PrintHigh));
    EXPECT_TRUE(hasFlag(flags, TriggerFlag::PrintChat));
    EXPECT_FALSE(hasFlag(flags, TriggerFlag::PrintMedium));
}

TEST(TriggerHelpersTest, ParseTriggerFlagsBehavior) {
    auto flags = parseTriggerFlags("frn");
    EXPECT_TRUE(hasFlag(flags, TriggerFlag::Final));
    EXPECT_TRUE(hasFlag(flags, TriggerFlag::RemoveString));
    EXPECT_TRUE(hasFlag(flags, TriggerFlag::NoLog));
}

TEST(TriggerHelpersTest, TriggerFlagsToString) {
    auto flags = TriggerFlag::PrintLow | TriggerFlag::PrintHigh | TriggerFlag::Final;
    auto str = triggerFlagsToString(flags);
    EXPECT_NE(str.find('l'), std::string::npos);
    EXPECT_NE(str.find('h'), std::string::npos);
    EXPECT_NE(str.find('f'), std::string::npos);
    EXPECT_EQ(str.find('c'), std::string::npos);  // No chat
}

TEST(TriggerHelpersTest, CreateTrigger) {
    auto trigger = createTrigger("mytrigger", ".*pattern.*");
    EXPECT_EQ(trigger.name, "mytrigger");
    EXPECT_EQ(trigger.pattern, ".*pattern.*");
    EXPECT_TRUE(trigger.isEnabled());
}

TEST(TriggerHelpersTest, CreateTriggerCustomFlags) {
    auto trigger = createTrigger("test", "pat", TriggerFlag::PrintChat);
    EXPECT_FALSE(trigger.isEnabled());  // No Enabled flag
    EXPECT_TRUE(trigger.matchesMessageType(MessageType::Chat));
}

TEST(TriggerHelpersTest, CreateMatchResult) {
    RegexTrigger trigger;
    trigger.flags = TriggerFlag::RemoveString | TriggerFlag::NoLog | TriggerFlag::Final;

    auto result = createMatchResult(trigger, true, 5);
    EXPECT_TRUE(result.matched);
    EXPECT_TRUE(result.shouldHide);
    EXPECT_TRUE(result.shouldNotLog);
    EXPECT_TRUE(result.isFinal);
    EXPECT_EQ(result.triggerIndex, 5);
}

TEST(TriggerHelpersTest, CreateMatchResultNoMatch) {
    RegexTrigger trigger;
    trigger.flags = TriggerFlag::RemoveString;

    auto result = createMatchResult(trigger, false);
    EXPECT_FALSE(result.matched);
    EXPECT_FALSE(result.shouldHide);  // Not set when not matched
}
