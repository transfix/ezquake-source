/**
 * @file config_test.cpp
 * @brief Tests for configuration, teamplay, and scripting types
 */

#include <gtest/gtest.h>
#include "core/config/config_types.hpp"

using namespace ezquake::config;

// =============================================================================
// Config Limits Tests
// =============================================================================

TEST(ConfigLimitsTest, Constants) {
    EXPECT_EQ(config_limits::MAX_LOC_NAME, 64);
    EXPECT_EQ(config_limits::MAX_MACRO_STRING, 2048);
    EXPECT_EQ(config_limits::MAX_CONFIG_PATH, 256);
}

TEST(ConfigLimitsTest, MainConfigFilename) {
    EXPECT_EQ(config_limits::MAIN_CONFIG_FILENAME, "config.cfg");
}

// =============================================================================
// Message Type Tests
// =============================================================================

TEST(MessageTypeTest, Values) {
    EXPECT_EQ(MessageType::Unknown, 0);
    EXPECT_EQ(MessageType::Normal, 1);
    EXPECT_EQ(MessageType::Team, 2);
    EXPECT_EQ(MessageType::Spec, 4);
}

TEST(MessageTypeTest, IsTeamMessage) {
    EXPECT_FALSE(isTeamMessage(MessageType::Normal));
    EXPECT_TRUE(isTeamMessage(MessageType::Team));
    EXPECT_TRUE(isTeamMessage(MessageType::SpecTeam));
    EXPECT_FALSE(isTeamMessage(MessageType::Spec));
}

TEST(MessageTypeTest, IsSpectatorMessage) {
    EXPECT_FALSE(isSpectatorMessage(MessageType::Normal));
    EXPECT_FALSE(isSpectatorMessage(MessageType::Team));
    EXPECT_TRUE(isSpectatorMessage(MessageType::Spec));
    EXPECT_TRUE(isSpectatorMessage(MessageType::SpecTeam));
}

// =============================================================================
// FPD Flags Tests
// =============================================================================

TEST(FPDFlagsTest, Values) {
    EXPECT_EQ(FPDFlags::NoTimers, 2);
    EXPECT_EQ(FPDFlags::NoSoundTriggers, 4);
    EXPECT_EQ(FPDFlags::NoForceSkin, 256);
    EXPECT_EQ(FPDFlags::NoForceColor, 512);
}

// =============================================================================
// Fairplay Settings Tests
// =============================================================================

TEST(FairplaySettingsTest, AllowTimers) {
    FairplaySettings settings;
    EXPECT_TRUE(settings.allowTimers());
    
    settings.flags = FPDFlags::NoTimers;
    EXPECT_FALSE(settings.allowTimers());
}

TEST(FairplaySettingsTest, AllowSoundTriggers) {
    FairplaySettings settings;
    EXPECT_TRUE(settings.allowSoundTriggers());
    
    settings.flags = FPDFlags::NoSoundTriggers;
    EXPECT_FALSE(settings.allowSoundTriggers());
}

TEST(FairplaySettingsTest, LimitPitch) {
    FairplaySettings settings;
    EXPECT_FALSE(settings.limitPitch());
    
    settings.flags = FPDFlags::LimitPitch;
    EXPECT_TRUE(settings.limitPitch());
}

// =============================================================================
// Trigger Flags Tests
// =============================================================================

TEST(TriggerFlagsTest, Values) {
    EXPECT_EQ(TriggerFlags::PrintLow, 1);
    EXPECT_EQ(TriggerFlags::PrintHigh, 4);
    EXPECT_EQ(TriggerFlags::Enabled, 2048);
}

// =============================================================================
// Trigger Definition Tests
// =============================================================================

TEST(TriggerDefTest, IsEnabled) {
    TriggerDef trigger;
    trigger.flags = TriggerFlags::Enabled;
    EXPECT_TRUE(trigger.isEnabled());
    
    trigger.flags = 0;
    EXPECT_FALSE(trigger.isEnabled());
}

TEST(TriggerDefTest, IsFinal) {
    TriggerDef trigger;
    EXPECT_FALSE(trigger.isFinal());
    
    trigger.flags = TriggerFlags::Final;
    EXPECT_TRUE(trigger.isFinal());
}

TEST(TriggerDefTest, ShouldLog) {
    TriggerDef trigger;
    EXPECT_TRUE(trigger.shouldLog());
    
    trigger.flags = TriggerFlags::NoLog;
    EXPECT_FALSE(trigger.shouldLog());
}

TEST(TriggerDefTest, CanTrigger) {
    TriggerDef trigger;
    trigger.flags = TriggerFlags::Enabled;
    trigger.minInterval = 0.0f;
    
    EXPECT_TRUE(trigger.canTrigger(0.0));
    
    trigger.minInterval = 1.0f;
    trigger.lastTime = 0.0;
    
    EXPECT_FALSE(trigger.canTrigger(0.5));
    EXPECT_TRUE(trigger.canTrigger(1.5));
}

// =============================================================================
// Item Flags Tests
// =============================================================================

TEST(ItemFlagsTest, Values) {
    EXPECT_EQ(ItemFlags::Quad, 1);
    EXPECT_EQ(ItemFlags::Pent, 2);
    EXPECT_EQ(ItemFlags::RA, 16);
    EXPECT_EQ(ItemFlags::RL, 1024);
}

TEST(ItemFlagsTest, Categories) {
    EXPECT_EQ(ItemFlags::AllPowerups, ItemFlags::Quad | ItemFlags::Pent | 
              ItemFlags::Ring | ItemFlags::Suit);
    EXPECT_EQ(ItemFlags::AllArmor, ItemFlags::RA | ItemFlags::YA | ItemFlags::GA);
}

TEST(ItemFlagsTest, IsPowerup) {
    EXPECT_TRUE(isPowerup(ItemFlags::Quad));
    EXPECT_TRUE(isPowerup(ItemFlags::Pent));
    EXPECT_FALSE(isPowerup(ItemFlags::RA));
    EXPECT_FALSE(isPowerup(ItemFlags::RL));
}

TEST(ItemFlagsTest, IsArmor) {
    EXPECT_TRUE(isArmor(ItemFlags::RA));
    EXPECT_TRUE(isArmor(ItemFlags::YA));
    EXPECT_FALSE(isArmor(ItemFlags::Quad));
    EXPECT_FALSE(isArmor(ItemFlags::RL));
}

TEST(ItemFlagsTest, IsWeapon) {
    EXPECT_TRUE(isWeapon(ItemFlags::RL));
    EXPECT_TRUE(isWeapon(ItemFlags::LG));
    EXPECT_FALSE(isWeapon(ItemFlags::RA));
    EXPECT_FALSE(isWeapon(ItemFlags::Quad));
}

TEST(ItemFlagsTest, GetItemName) {
    EXPECT_STREQ(getItemName(ItemFlags::Quad), "quad");
    EXPECT_STREQ(getItemName(ItemFlags::RL), "rl");
    EXPECT_STREQ(getItemName(ItemFlags::RA), "ra");
}

// =============================================================================
// Location Entry Tests
// =============================================================================

TEST(LocationEntryTest, DistanceSquared) {
    LocationEntry loc;
    loc.coord = ezquake::math::Vec3{0.0f, 0.0f, 0.0f};
    
    ezquake::math::Vec3 point{3.0f, 4.0f, 0.0f};
    
    EXPECT_FLOAT_EQ(loc.distanceSquared(point), 25.0f);  // 3² + 4² = 25
}

// =============================================================================
// Skin Forcing Config Tests
// =============================================================================

TEST(SkinForcingConfigTest, HasSkinForcing) {
    SkinForcingConfig config;
    EXPECT_FALSE(config.hasSkinForcing());
    
    config.teamSkin = "base";
    EXPECT_TRUE(config.hasSkinForcing());
}

// =============================================================================
// Color Forcing Config Tests
// =============================================================================

TEST(ColorForcingConfigTest, HasTeamColor) {
    ColorForcingConfig config;
    EXPECT_FALSE(config.hasTeamColor());
    
    config.teamTopColor = 4;
    EXPECT_TRUE(config.hasTeamColor());
}

TEST(ColorForcingConfigTest, HasEnemyColor) {
    ColorForcingConfig config;
    EXPECT_FALSE(config.hasEnemyColor());
    
    config.enemyTopColor = 13;
    EXPECT_TRUE(config.hasEnemyColor());
}

TEST(ColorForcingConfigTest, HasColorForcing) {
    ColorForcingConfig config;
    EXPECT_FALSE(config.hasColorForcing());
    
    config.teamTopColor = 4;
    EXPECT_TRUE(config.hasColorForcing());
}

// =============================================================================
// Config Save Settings Tests
// =============================================================================

TEST(ConfigSaveSettingsTest, ShouldSaveAll) {
    ConfigSaveSettings settings;
    settings.option = ConfigSaveOption::All;
    
    EXPECT_TRUE(settings.shouldSave(false, false));
    EXPECT_TRUE(settings.shouldSave(false, true));
    EXPECT_TRUE(settings.shouldSave(true, false));
}

TEST(ConfigSaveSettingsTest, ShouldSaveChangedOnly) {
    ConfigSaveSettings settings;
    settings.option = ConfigSaveOption::ChangedOnly;
    
    EXPECT_FALSE(settings.shouldSave(false, false));
    EXPECT_TRUE(settings.shouldSave(true, false));
}

TEST(ConfigSaveSettingsTest, ShouldSaveNonDefault) {
    ConfigSaveSettings settings;
    settings.option = ConfigSaveOption::NonDefault;
    
    EXPECT_TRUE(settings.shouldSave(false, false));
    EXPECT_FALSE(settings.shouldSave(false, true));
}

// =============================================================================
// Alias Definition Tests
// =============================================================================

TEST(AliasDefTest, IsEmpty) {
    AliasDef alias;
    EXPECT_TRUE(alias.isEmpty());
    
    alias.value = "+forward;+jump";
    EXPECT_FALSE(alias.isEmpty());
}

// =============================================================================
// Bind Definition Tests
// =============================================================================

TEST(BindDefTest, IsBound) {
    BindDef bind;
    EXPECT_FALSE(bind.isBound());
    
    bind.command = "+attack";
    EXPECT_TRUE(bind.isBound());
}

// =============================================================================
// Macro State Tests
// =============================================================================

TEST(MacroStateTest, CanRecurse) {
    MacroState state;
    EXPECT_TRUE(state.canRecurse());
    
    for (int i = 0; i < MacroState::MAX_RECURSION; ++i) {
        state.enterRecursion();
    }
    
    EXPECT_FALSE(state.canRecurse());
}

TEST(MacroStateTest, EnterExitRecursion) {
    MacroState state;
    EXPECT_EQ(state.recursionDepth, 0);
    
    state.enterRecursion();
    EXPECT_EQ(state.recursionDepth, 1);
    
    state.enterRecursion();
    EXPECT_EQ(state.recursionDepth, 2);
    
    state.exitRecursion();
    EXPECT_EQ(state.recursionDepth, 1);
}

// =============================================================================
// Teamplay State Tests
// =============================================================================

TEST(TeamplayStateTest, HasTeam) {
    TeamplayState state;
    EXPECT_FALSE(state.hasTeam());
    
    state.playerTeam = "red";
    EXPECT_TRUE(state.hasTeam());
}

TEST(TeamplayStateTest, IsTeamGame) {
    TeamplayState state;
    EXPECT_FALSE(state.isTeamGame());
    
    state.playerTeam = "red";
    state.playerCount = 1;
    EXPECT_FALSE(state.isTeamGame());
    
    state.playerCount = 4;
    EXPECT_TRUE(state.isTeamGame());
}

// =============================================================================
// Config Stats Tests
// =============================================================================

TEST(ConfigStatsTest, ChangeRatio) {
    ConfigStats stats;
    EXPECT_FLOAT_EQ(stats.changeRatio(), 0.0f);
    
    stats.totalCvars = 100;
    stats.changedCvars = 25;
    
    EXPECT_FLOAT_EQ(stats.changeRatio(), 0.25f);
}
