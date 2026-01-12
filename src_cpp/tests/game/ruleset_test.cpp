/**
 * @file ruleset_test.cpp
 * @brief Unit tests for ruleset_types.hpp
 * 
 * Tests for ruleset modes, restrictions, configurations,
 * and competitive play settings.
 */

#include <gtest/gtest.h>
#include "../../core/game/ruleset_types.hpp"

namespace ezquake::ruleset {
namespace {

//=============================================================================
// Ruleset Limits Tests
//=============================================================================

TEST(RulesetLimitsTest, MTFLFlashValues) {
    EXPECT_GT(ruleset_limits::MTFL_FLASH_GAMMA, 0.0f);
    EXPECT_LE(ruleset_limits::MTFL_FLASH_GAMMA, 1.0f);
    EXPECT_EQ(ruleset_limits::MTFL_FLASH_CONTRAST, 1.0f);
}

TEST(RulesetLimitsTest, MaxFPSValues) {
    EXPECT_EQ(ruleset_limits::DEFAULT_MAX_FPS, 0.0f);  // No limit
    EXPECT_GT(ruleset_limits::SMACKDOWN_MAX_FPS, 0.0f);
    EXPECT_GT(ruleset_limits::MTFL_MAX_FPS, 0.0f);
    EXPECT_LT(ruleset_limits::MTFL_MAX_FPS, ruleset_limits::SMACKDOWN_MAX_FPS);
}

TEST(RulesetLimitsTest, CompetitiveMaxWaits) {
    EXPECT_EQ(ruleset_limits::DEFAULT_MAX_WAITS, 0);  // No limit
    EXPECT_GT(ruleset_limits::COMPETITIVE_MAX_WAITS, 0);
}

//=============================================================================
// Ruleset Mode Tests
//=============================================================================

TEST(RulesetModeTest, AllModesHaveNames) {
    EXPECT_EQ(rulesetName(RulesetMode::Default), "default");
    EXPECT_EQ(rulesetName(RulesetMode::Smackdown), "smackdown");
    EXPECT_EQ(rulesetName(RulesetMode::Thunderdome), "thunderdome");
    EXPECT_EQ(rulesetName(RulesetMode::QCon), "qcon");
    EXPECT_EQ(rulesetName(RulesetMode::MTFL), "mtfl");
    EXPECT_EQ(rulesetName(RulesetMode::Smackdrive), "smackdrive");
}

TEST(RulesetModeTest, ParseRuleset) {
    EXPECT_EQ(parseRuleset("default"), RulesetMode::Default);
    EXPECT_EQ(parseRuleset(""), RulesetMode::Default);
    EXPECT_EQ(parseRuleset("smackdown"), RulesetMode::Smackdown);
    EXPECT_EQ(parseRuleset("thunderdome"), RulesetMode::Thunderdome);
    EXPECT_EQ(parseRuleset("qcon"), RulesetMode::QCon);
    EXPECT_EQ(parseRuleset("mtfl"), RulesetMode::MTFL);
    EXPECT_EQ(parseRuleset("smackdrive"), RulesetMode::Smackdrive);
}

TEST(RulesetModeTest, ParseInvalidRuleset) {
    EXPECT_FALSE(parseRuleset("invalid").has_value());
    EXPECT_FALSE(parseRuleset("SMACKDOWN").has_value());  // Case sensitive
    EXPECT_FALSE(parseRuleset("tournament").has_value());
}

TEST(RulesetModeTest, IsCompetitive) {
    EXPECT_FALSE(isCompetitive(RulesetMode::Default));
    
    EXPECT_TRUE(isCompetitive(RulesetMode::Smackdown));
    EXPECT_TRUE(isCompetitive(RulesetMode::Thunderdome));
    EXPECT_TRUE(isCompetitive(RulesetMode::QCon));
    EXPECT_TRUE(isCompetitive(RulesetMode::MTFL));
    EXPECT_TRUE(isCompetitive(RulesetMode::Smackdrive));
}

//=============================================================================
// Restriction Set Tests
//=============================================================================

TEST(RestrictionSetTest, DefaultIsEmpty) {
    RestrictionSet restrictions;
    EXPECT_TRUE(restrictions.none());
    EXPECT_EQ(restrictions.count(), 0u);
}

TEST(RestrictionSetTest, SetAndTest) {
    RestrictionSet restrictions;
    restrictions.set(static_cast<size_t>(RestrictionFlag::Triggers));
    
    EXPECT_TRUE(restrictions.test(static_cast<size_t>(RestrictionFlag::Triggers)));
    EXPECT_FALSE(restrictions.test(static_cast<size_t>(RestrictionFlag::Packet)));
}

TEST(RestrictionSetTest, MultipleRestrictions) {
    RestrictionSet restrictions;
    restrictions.set(static_cast<size_t>(RestrictionFlag::Triggers));
    restrictions.set(static_cast<size_t>(RestrictionFlag::Exec));
    restrictions.set(static_cast<size_t>(RestrictionFlag::IPC));
    
    EXPECT_EQ(restrictions.count(), 3u);
}

//=============================================================================
// Ruleset Config Tests
//=============================================================================

TEST(RulesetConfigTest, DefaultConstruction) {
    RulesetConfig config;
    
    EXPECT_EQ(config.mode, RulesetMode::Default);
    EXPECT_TRUE(config.restrictions.none());
    EXPECT_EQ(config.maxFPS, 0.0f);
    EXPECT_EQ(config.maxSequentialWaits, 0);
}

TEST(RulesetConfigTest, RestrictAndAllow) {
    RulesetConfig config;
    
    config.restrict(RestrictionFlag::Triggers);
    EXPECT_TRUE(config.isRestricted(RestrictionFlag::Triggers));
    
    config.allow(RestrictionFlag::Triggers);
    EXPECT_FALSE(config.isRestricted(RestrictionFlag::Triggers));
}

TEST(RulesetConfigTest, ConvenienceMethods) {
    RulesetConfig config;
    
    config.restrict(RestrictionFlag::Triggers);
    EXPECT_TRUE(config.restrictsTriggers());
    
    config.restrict(RestrictionFlag::Particles);
    EXPECT_TRUE(config.restrictsParticles());
    
    config.restrict(RestrictionFlag::Exec);
    EXPECT_TRUE(config.restrictsExec());
    
    config.restrict(RestrictionFlag::Timerefresh);
    EXPECT_FALSE(config.allowsTimerefresh());
    
    config.restrict(RestrictionFlag::NoShadows);
    EXPECT_FALSE(config.allowsNoShadows());
}

TEST(RulesetConfigTest, FPSCap) {
    RulesetConfig config;
    
    EXPECT_FALSE(config.hasFPSCap());
    EXPECT_NEAR(config.effectiveFPS(1000.0f), 1000.0f, 0.01f);
    
    config.maxFPS = 500.0f;
    EXPECT_TRUE(config.hasFPSCap());
    EXPECT_NEAR(config.effectiveFPS(1000.0f), 500.0f, 0.01f);
    EXPECT_NEAR(config.effectiveFPS(250.0f), 250.0f, 0.01f);
}

TEST(RulesetConfigTest, ConsoleLogging) {
    RulesetConfig config;
    EXPECT_TRUE(config.canLogConsole());
    
    config.restrict(RestrictionFlag::ConsoleLogging);
    EXPECT_FALSE(config.canLogConsole());
}

//=============================================================================
// Factory Function Tests
//=============================================================================

TEST(RulesetFactoryTest, DefaultRuleset) {
    auto config = createDefaultRuleset();
    
    EXPECT_EQ(config.mode, RulesetMode::Default);
    EXPECT_TRUE(config.restrictions.none());
    EXPECT_EQ(config.maxFPS, 0.0f);
}

TEST(RulesetFactoryTest, SmackdownRuleset) {
    auto config = createSmackdownRuleset();
    
    EXPECT_EQ(config.mode, RulesetMode::Smackdown);
    EXPECT_EQ(config.maxFPS, ruleset_limits::SMACKDOWN_MAX_FPS);
    EXPECT_TRUE(config.restrictsTriggers());
    EXPECT_TRUE(config.restrictsExec());
}

TEST(RulesetFactoryTest, ThunderdomeRuleset) {
    auto config = createThunderdomeRuleset();
    
    EXPECT_EQ(config.mode, RulesetMode::Thunderdome);
    EXPECT_EQ(config.maxFPS, ruleset_limits::THUNDERDOME_MAX_FPS);
}

TEST(RulesetFactoryTest, QConRuleset) {
    auto config = createQConRuleset();
    
    EXPECT_EQ(config.mode, RulesetMode::QCon);
    EXPECT_EQ(config.maxFPS, ruleset_limits::QCON_MAX_FPS);
}

TEST(RulesetFactoryTest, MTFLRuleset) {
    auto config = createMTFLRuleset();
    
    EXPECT_EQ(config.mode, RulesetMode::MTFL);
    EXPECT_EQ(config.maxFPS, ruleset_limits::MTFL_MAX_FPS);
    EXPECT_NEAR(config.flashGamma, ruleset_limits::MTFL_FLASH_GAMMA, 0.001f);
    EXPECT_NEAR(config.flashContrast, ruleset_limits::MTFL_FLASH_CONTRAST, 0.001f);
    
    // MTFL has extra restrictions
    EXPECT_TRUE(config.restrictsParticles());
    EXPECT_FALSE(config.allowsNoShadows());
    EXPECT_TRUE(config.isRestricted(RestrictionFlag::HudPicChange));
    EXPECT_TRUE(config.isRestricted(RestrictionFlag::FullbrightSkins));
    EXPECT_TRUE(config.isRestricted(RestrictionFlag::ModelOutline));
}

TEST(RulesetFactoryTest, SmackdriveRuleset) {
    auto config = createSmackdriveRuleset();
    
    EXPECT_EQ(config.mode, RulesetMode::Smackdrive);
    EXPECT_EQ(config.maxFPS, ruleset_limits::SMACKDRIVE_MAX_FPS);
}

TEST(RulesetFactoryTest, CreateByMode) {
    auto defaultConfig = createRuleset(RulesetMode::Default);
    EXPECT_EQ(defaultConfig.mode, RulesetMode::Default);
    
    auto mtflConfig = createRuleset(RulesetMode::MTFL);
    EXPECT_EQ(mtflConfig.mode, RulesetMode::MTFL);
    EXPECT_EQ(mtflConfig.maxFPS, ruleset_limits::MTFL_MAX_FPS);
}

//=============================================================================
// Model Restriction Tests
//=============================================================================

TEST(ModelRestrictionTest, DefaultAllowsEverything) {
    auto config = createDefaultRuleset();
    auto restriction = ModelRestriction::fromConfig(config);
    
    EXPECT_TRUE(restriction.allowFullbright);
    EXPECT_TRUE(restriction.allowOutline);
    EXPECT_TRUE(restriction.allowExternalTexture);
    EXPECT_TRUE(restriction.allowLumaTexture);
    EXPECT_TRUE(restriction.allowSimpleTexture);
    EXPECT_TRUE(restriction.allowPowerupShell);
}

TEST(ModelRestrictionTest, MTFLRestrictions) {
    auto config = createMTFLRuleset();
    auto restriction = ModelRestriction::fromConfig(config);
    
    EXPECT_FALSE(restriction.allowFullbright);
    EXPECT_FALSE(restriction.allowOutline);
    EXPECT_FALSE(restriction.allowExternalTexture);
    EXPECT_FALSE(restriction.allowLumaTexture);
    EXPECT_FALSE(restriction.allowSimpleTexture);
    EXPECT_FALSE(restriction.allowPowerupShell);
}

//=============================================================================
// Cvar Validator Tests
//=============================================================================

TEST(CvarValidatorTest, DefaultAllowsEverything) {
    auto config = createDefaultRuleset();
    CvarValidator validator(config);
    
    float value = 1.0f;
    EXPECT_EQ(validator.validateFullbrightSkins(value), CvarChangeResult::Allowed);
    EXPECT_EQ(value, 1.0f);
}

TEST(CvarValidatorTest, MTFLClampsFullbrightSkins) {
    auto config = createMTFLRuleset();
    CvarValidator validator(config);
    
    float value = 1.0f;
    EXPECT_EQ(validator.validateFullbrightSkins(value), CvarChangeResult::Clamped);
    EXPECT_EQ(value, 0.0f);
}

TEST(CvarValidatorTest, MTFLClampsFakeShaft) {
    auto config = createMTFLRuleset();
    config.restrict(RestrictionFlag::FakeShaft);
    CvarValidator validator(config);
    
    float value = 1.0f;
    EXPECT_EQ(validator.validateFakeShaft(value), CvarChangeResult::Clamped);
    EXPECT_EQ(value, 0.0f);
}

TEST(CvarValidatorTest, AllowsZeroValues) {
    auto config = createMTFLRuleset();
    config.restrict(RestrictionFlag::FakeShaft);
    CvarValidator validator(config);
    
    float value = 0.0f;
    EXPECT_EQ(validator.validateFakeShaft(value), CvarChangeResult::Allowed);
    EXPECT_EQ(value, 0.0f);
}

//=============================================================================
// Ruleset State Tests
//=============================================================================

TEST(RulesetStateTest, DefaultConstruction) {
    RulesetState state;
    
    EXPECT_EQ(state.mode(), RulesetMode::Default);
    EXPECT_EQ(state.name(), "default");
    EXPECT_FALSE(state.isCompetitive());
}

TEST(RulesetStateTest, ConstructWithMode) {
    RulesetState state(RulesetMode::MTFL);
    
    EXPECT_EQ(state.mode(), RulesetMode::MTFL);
    EXPECT_EQ(state.name(), "mtfl");
    EXPECT_TRUE(state.isCompetitive());
    EXPECT_EQ(state.maxFPS(), ruleset_limits::MTFL_MAX_FPS);
}

TEST(RulesetStateTest, SetRuleset) {
    RulesetState state;
    
    state.setRuleset(RulesetMode::Smackdown);
    EXPECT_EQ(state.mode(), RulesetMode::Smackdown);
    EXPECT_EQ(state.name(), "smackdown");
    EXPECT_TRUE(state.isCompetitive());
    
    state.setRuleset(RulesetMode::Default);
    EXPECT_EQ(state.mode(), RulesetMode::Default);
    EXPECT_FALSE(state.isCompetitive());
}

TEST(RulesetStateTest, Validator) {
    RulesetState state(RulesetMode::MTFL);
    auto validator = state.validator();
    
    float value = 1.0f;
    EXPECT_EQ(validator.validateFullbrightSkins(value), CvarChangeResult::Clamped);
}

TEST(RulesetStateTest, MutableConfig) {
    RulesetState state;
    
    state.config().restrict(RestrictionFlag::Triggers);
    EXPECT_TRUE(state.config().restrictsTriggers());
}

//=============================================================================
// Ruleset Stats Tests
//=============================================================================

TEST(RulesetStatsTest, FromDefaultConfig) {
    auto config = createDefaultRuleset();
    auto stats = RulesetStats::from(config);
    
    EXPECT_EQ(stats.currentMode, RulesetMode::Default);
    EXPECT_EQ(stats.activeRestrictions, 0);
    EXPECT_EQ(stats.effectiveMaxFPS, 0.0f);
    EXPECT_FALSE(stats.isTournamentMode);
}

TEST(RulesetStatsTest, FromMTFLConfig) {
    auto config = createMTFLRuleset();
    auto stats = RulesetStats::from(config);
    
    EXPECT_EQ(stats.currentMode, RulesetMode::MTFL);
    EXPECT_GT(stats.activeRestrictions, 10);  // MTFL has many restrictions
    EXPECT_EQ(stats.effectiveMaxFPS, ruleset_limits::MTFL_MAX_FPS);
    EXPECT_TRUE(stats.isTournamentMode);
}

//=============================================================================
// Helper Function Tests
//=============================================================================

TEST(HelperTest, MTFLBanReason) {
    EXPECT_EQ(getMTFLBanReason(), "banned by MTFL ruleset");
}

TEST(HelperTest, CompetitiveBaseRestrictions) {
    RulesetConfig config;
    applyCompetitiveBase(config);
    
    EXPECT_TRUE(config.restrictsTriggers());
    EXPECT_TRUE(config.isRestricted(RestrictionFlag::Packet));
    EXPECT_TRUE(config.restrictsExec());
    EXPECT_TRUE(config.isRestricted(RestrictionFlag::IPC));
    EXPECT_TRUE(config.isRestricted(RestrictionFlag::SetCalc));
    EXPECT_TRUE(config.isRestricted(RestrictionFlag::TCL));
    EXPECT_FALSE(config.allowsTimerefresh());
    EXPECT_EQ(config.maxSequentialWaits, ruleset_limits::COMPETITIVE_MAX_WAITS);
}

//=============================================================================
// Locked/Limited Cvar Tests
//=============================================================================

TEST(LockedCvarTest, Construction) {
    LockedCvar locked("cl_maxfps", "77");
    
    EXPECT_EQ(locked.name, "cl_maxfps");
    EXPECT_EQ(locked.value, "77");
}

TEST(LimitedCvarMaxTest, Construction) {
    LimitedCvarMax limited("cl_maxfps", 500.0f);
    
    EXPECT_EQ(limited.name, "cl_maxfps");
    EXPECT_EQ(limited.maxValue, 500.0f);
}

TEST(LimitedCvarMinTest, Construction) {
    LimitedCvarMin limited("r_drawviewmodel", 0.5f);
    
    EXPECT_EQ(limited.name, "r_drawviewmodel");
    EXPECT_EQ(limited.minValue, 0.5f);
}

} // anonymous namespace
} // namespace ezquake::ruleset
