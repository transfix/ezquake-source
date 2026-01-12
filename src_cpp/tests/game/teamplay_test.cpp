/**
 * @file teamplay_test.cpp
 * @brief Unit tests for teamplay_types.hpp
 * 
 * Tests for item tracking, skin forcing, message types,
 * and teamplay state management.
 */

#include <gtest/gtest.h>
#include "../../core/game/teamplay_types.hpp"

namespace ezquake::teamplay {
namespace {

//=============================================================================
// Teamplay Limits Tests
//=============================================================================

TEST(TeamplayLimitsTest, ConstantsAreValid) {
    EXPECT_GT(teamplay_limits::MAX_LOC_NAME, 0);
    EXPECT_GT(teamplay_limits::MAX_MACRO_STRING, 0);
    EXPECT_EQ(teamplay_limits::NUM_ITEMFLAGS, 32);
}

//=============================================================================
// Item Flag Tests
//=============================================================================

TEST(ItemFlagTest, BitValues) {
    EXPECT_EQ(itemFlagBit(ItemFlag::Quad), 1u);
    EXPECT_EQ(itemFlagBit(ItemFlag::Pent), 2u);
    EXPECT_EQ(itemFlagBit(ItemFlag::Ring), 4u);
    EXPECT_EQ(itemFlagBit(ItemFlag::Suit), 8u);
    EXPECT_EQ(itemFlagBit(ItemFlag::RedArmor), 16u);
}

TEST(ItemFlagTest, AllItemsHaveNames) {
    EXPECT_EQ(itemName(ItemFlag::Quad), "quad");
    EXPECT_EQ(itemName(ItemFlag::Pent), "pent");
    EXPECT_EQ(itemName(ItemFlag::Ring), "ring");
    EXPECT_EQ(itemName(ItemFlag::RedArmor), "ra");
    EXPECT_EQ(itemName(ItemFlag::YellowArmor), "ya");
    EXPECT_EQ(itemName(ItemFlag::GreenArmor), "ga");
    EXPECT_EQ(itemName(ItemFlag::MegaHealth), "mega");
    EXPECT_EQ(itemName(ItemFlag::LightningGun), "lg");
    EXPECT_EQ(itemName(ItemFlag::RocketLauncher), "rl");
    EXPECT_EQ(itemName(ItemFlag::Flag), "flag");
}

TEST(ItemFlagTest, ItemFlagsSet) {
    ItemFlags flags;
    flags.set(static_cast<size_t>(ItemFlag::Quad));
    flags.set(static_cast<size_t>(ItemFlag::RocketLauncher));
    
    EXPECT_TRUE(flags.test(static_cast<size_t>(ItemFlag::Quad)));
    EXPECT_TRUE(flags.test(static_cast<size_t>(ItemFlag::RocketLauncher)));
    EXPECT_FALSE(flags.test(static_cast<size_t>(ItemFlag::Pent)));
}

//=============================================================================
// Item Mask Tests
//=============================================================================

TEST(ItemMaskTest, PowerupsMask) {
    uint32_t powerups = item_masks::POWERUPS;
    
    EXPECT_TRUE((powerups & itemFlagBit(ItemFlag::Quad)) != 0);
    EXPECT_TRUE((powerups & itemFlagBit(ItemFlag::Pent)) != 0);
    EXPECT_TRUE((powerups & itemFlagBit(ItemFlag::Ring)) != 0);
    EXPECT_TRUE((powerups & itemFlagBit(ItemFlag::Suit)) != 0);
    EXPECT_FALSE((powerups & itemFlagBit(ItemFlag::RedArmor)) != 0);
}

TEST(ItemMaskTest, WeaponsMask) {
    uint32_t weapons = item_masks::WEAPONS;
    
    EXPECT_TRUE((weapons & itemFlagBit(ItemFlag::LightningGun)) != 0);
    EXPECT_TRUE((weapons & itemFlagBit(ItemFlag::RocketLauncher)) != 0);
    EXPECT_TRUE((weapons & itemFlagBit(ItemFlag::GrenadeLauncher)) != 0);
    EXPECT_TRUE((weapons & itemFlagBit(ItemFlag::SuperNailgun)) != 0);
    EXPECT_TRUE((weapons & itemFlagBit(ItemFlag::SuperShotgun)) != 0);
    EXPECT_FALSE((weapons & itemFlagBit(ItemFlag::Nailgun)) != 0);  // Not in mask
}

TEST(ItemMaskTest, ArmorMask) {
    uint32_t armor = item_masks::ARMOR;
    
    EXPECT_TRUE((armor & itemFlagBit(ItemFlag::RedArmor)) != 0);
    EXPECT_TRUE((armor & itemFlagBit(ItemFlag::YellowArmor)) != 0);
    EXPECT_TRUE((armor & itemFlagBit(ItemFlag::GreenArmor)) != 0);
}

TEST(ItemMaskTest, AmmoMask) {
    uint32_t ammo = item_masks::AMMO;
    
    EXPECT_TRUE((ammo & itemFlagBit(ItemFlag::Cells)) != 0);
    EXPECT_TRUE((ammo & itemFlagBit(ItemFlag::Rockets)) != 0);
    EXPECT_TRUE((ammo & itemFlagBit(ItemFlag::Nails)) != 0);
    EXPECT_TRUE((ammo & itemFlagBit(ItemFlag::Shells)) != 0);
}

TEST(ItemMaskTest, HelperFunctions) {
    EXPECT_TRUE(isPowerup(itemFlagBit(ItemFlag::Quad)));
    EXPECT_TRUE(isPowerup(itemFlagBit(ItemFlag::Pent)));
    EXPECT_FALSE(isPowerup(itemFlagBit(ItemFlag::RedArmor)));
    
    EXPECT_TRUE(isWeapon(itemFlagBit(ItemFlag::RocketLauncher)));
    EXPECT_FALSE(isWeapon(itemFlagBit(ItemFlag::Quad)));
    
    EXPECT_TRUE(isArmor(itemFlagBit(ItemFlag::RedArmor)));
    EXPECT_FALSE(isArmor(itemFlagBit(ItemFlag::Quad)));
    
    EXPECT_TRUE(isAmmo(itemFlagBit(ItemFlag::Rockets)));
    EXPECT_FALSE(isAmmo(itemFlagBit(ItemFlag::RocketLauncher)));
    
    EXPECT_TRUE(isPlayer(itemFlagBit(ItemFlag::Teammate)));
    EXPECT_TRUE(isPlayer(itemFlagBit(ItemFlag::Enemy)));
    EXPECT_FALSE(isPlayer(itemFlagBit(ItemFlag::Quad)));
}

//=============================================================================
// FPD Flag Tests
//=============================================================================

TEST(FPDFlagTest, HasFPD) {
    uint16_t fpd = static_cast<uint16_t>(FPDFlag::NoTimers) | 
                   static_cast<uint16_t>(FPDFlag::NoForceSkin);
    
    EXPECT_TRUE(hasFPD(fpd, FPDFlag::NoTimers));
    EXPECT_TRUE(hasFPD(fpd, FPDFlag::NoForceSkin));
    EXPECT_FALSE(hasFPD(fpd, FPDFlag::NoSoundTriggers));
    EXPECT_FALSE(hasFPD(fpd, FPDFlag::NoForceColor));
}

//=============================================================================
// Message Type Tests
//=============================================================================

TEST(MessageTypeTest, Names) {
    EXPECT_EQ(messageTypeName(MessageType::Normal), "normal");
    EXPECT_EQ(messageTypeName(MessageType::Team), "team");
    EXPECT_EQ(messageTypeName(MessageType::Spectator), "spec");
    EXPECT_EQ(messageTypeName(MessageType::SpectatorTeam), "specteam");
    EXPECT_EQ(messageTypeName(MessageType::QTV), "qtv");
    EXPECT_EQ(messageTypeName(MessageType::Unknown), "unknown");
}

TEST(MessageTypeTest, IsTeamMessage) {
    EXPECT_TRUE(isTeamMessage(MessageType::Team));
    EXPECT_TRUE(isTeamMessage(MessageType::SpectatorTeam));
    EXPECT_FALSE(isTeamMessage(MessageType::Normal));
    EXPECT_FALSE(isTeamMessage(MessageType::Spectator));
}

TEST(MessageTypeTest, IsSpectatorMessage) {
    EXPECT_TRUE(isSpectatorMessage(MessageType::Spectator));
    EXPECT_TRUE(isSpectatorMessage(MessageType::SpectatorTeam));
    EXPECT_FALSE(isSpectatorMessage(MessageType::Normal));
    EXPECT_FALSE(isSpectatorMessage(MessageType::Team));
}

//=============================================================================
// Item Visibility Tests
//=============================================================================

TEST(ItemVisibilityTest, Calculate) {
    ItemVisibility vis;
    vis.viewOrigin = math::Vec3(0, 0, 0);
    vis.entityOrigin = math::Vec3(100, 0, 0);
    vis.forward = math::Vec3(1, 0, 0);
    
    vis.calculate();
    
    EXPECT_NEAR(vis.distance, 100.0f, 0.01f);
    EXPECT_NEAR(vis.direction.x(), 1.0f, 0.01f);
    EXPECT_NEAR(vis.direction.y(), 0.0f, 0.01f);
}

TEST(ItemVisibilityTest, IsInFront) {
    ItemVisibility vis;
    vis.forward = math::Vec3(1, 0, 0);
    
    vis.direction = math::Vec3(1, 0, 0);
    EXPECT_TRUE(vis.isInFront());
    
    vis.direction = math::Vec3(-1, 0, 0);
    EXPECT_FALSE(vis.isInFront());
}

TEST(ItemVisibilityTest, AngleToEntity) {
    ItemVisibility vis;
    vis.forward = math::Vec3(1, 0, 0);
    
    // Directly in front
    vis.direction = math::Vec3(1, 0, 0);
    EXPECT_NEAR(vis.angleToEntity(), 0.0f, 0.1f);
    
    // 90 degrees to the side
    vis.direction = math::Vec3(0, 1, 0);
    EXPECT_NEAR(vis.angleToEntity(), 90.0f, 0.1f);
    
    // Behind
    vis.direction = math::Vec3(-1, 0, 0);
    EXPECT_NEAR(vis.angleToEntity(), 180.0f, 0.1f);
}

//=============================================================================
// Location Data Tests
//=============================================================================

TEST(LocationDataTest, Construction) {
    LocationData loc(math::Vec3(100, 200, 300), "red armor");
    
    EXPECT_NEAR(loc.coord.x(), 100.0f, 0.01f);
    EXPECT_NEAR(loc.coord.y(), 200.0f, 0.01f);
    EXPECT_NEAR(loc.coord.z(), 300.0f, 0.01f);
    EXPECT_EQ(loc.name, "red armor");
}

TEST(LocationDataTest, DistanceTo) {
    LocationData loc(math::Vec3(100, 0, 0), "test");
    
    math::Vec3 point(0, 0, 0);
    EXPECT_NEAR(loc.distanceTo(point), 100.0f, 0.01f);
}

//=============================================================================
// Teamplay State Tests
//=============================================================================

TEST(TeamplayStateTest, DefaultConstruction) {
    TeamplayState state;
    
    EXPECT_EQ(state.health, 0);
    EXPECT_EQ(state.items, 0u);
    EXPECT_EQ(state.numEnemies, 0);
    EXPECT_EQ(state.numFriendlies, 0);
}

TEST(TeamplayStateTest, Reset) {
    TeamplayState state;
    state.health = 100;
    state.items = 0xFFFFFFFF;
    state.numEnemies = 5;
    state.tookFlag = 1;
    
    state.reset();
    
    EXPECT_EQ(state.health, 0);
    EXPECT_EQ(state.items, 0u);
    EXPECT_EQ(state.numEnemies, 0);
    EXPECT_EQ(state.tookFlag, 0);
}

TEST(TeamplayStateTest, TookEmpty) {
    TeamplayState state;
    double timeout = 5.0;
    
    // No took time set
    EXPECT_TRUE(state.tookEmpty(10.0, timeout));
    
    // Recent took
    state.tookTime = 8.0;
    EXPECT_FALSE(state.tookEmpty(10.0, timeout));
    
    // Old took (expired)
    EXPECT_TRUE(state.tookEmpty(20.0, timeout));
}

TEST(TeamplayStateTest, HasPowerups) {
    TeamplayState state;
    
    EXPECT_FALSE(state.hasPowerups());
    
    state.items = itemFlagBit(ItemFlag::Quad);
    EXPECT_TRUE(state.hasPowerups());
}

TEST(TeamplayStateTest, ItemsChanged) {
    TeamplayState state;
    state.items = 100;
    state.oldItems = 100;
    EXPECT_FALSE(state.itemsChanged());
    
    state.items = 200;
    EXPECT_TRUE(state.itemsChanged());
}

TEST(TeamplayStateTest, NewAndLostItems) {
    TeamplayState state;
    state.oldItems = 0b0011;  // Had items 0 and 1
    state.items = 0b0110;     // Now have items 1 and 2
    
    EXPECT_EQ(state.newItems(), 0b0100u);  // Gained item 2
    EXPECT_EQ(state.lostItems(), 0b0001u); // Lost item 0
}

//=============================================================================
// Skin Force Type Tests
//=============================================================================

TEST(SkinForceTypeTest, Names) {
    EXPECT_EQ(skinForceTypeName(SkinForceType::Normal), "normal");
    EXPECT_EQ(skinForceTypeName(SkinForceType::Team), "team");
    EXPECT_EQ(skinForceTypeName(SkinForceType::Enemy), "enemy");
    EXPECT_EQ(skinForceTypeName(SkinForceType::TeamQuad), "team_quad");
    EXPECT_EQ(skinForceTypeName(SkinForceType::EnemyBoth), "enemy_both");
}

TEST(SkinForceTypeTest, IsTeamSkin) {
    EXPECT_TRUE(isTeamSkin(SkinForceType::Team));
    EXPECT_TRUE(isTeamSkin(SkinForceType::TeamQuad));
    EXPECT_TRUE(isTeamSkin(SkinForceType::TeamPent));
    EXPECT_TRUE(isTeamSkin(SkinForceType::TeamBoth));
    
    EXPECT_FALSE(isTeamSkin(SkinForceType::Normal));
    EXPECT_FALSE(isTeamSkin(SkinForceType::Enemy));
    EXPECT_FALSE(isTeamSkin(SkinForceType::EnemyQuad));
}

TEST(SkinForceTypeTest, IsEnemySkin) {
    EXPECT_TRUE(isEnemySkin(SkinForceType::Enemy));
    EXPECT_TRUE(isEnemySkin(SkinForceType::EnemyQuad));
    EXPECT_TRUE(isEnemySkin(SkinForceType::EnemyPent));
    EXPECT_TRUE(isEnemySkin(SkinForceType::EnemyBoth));
    
    EXPECT_FALSE(isEnemySkin(SkinForceType::Normal));
    EXPECT_FALSE(isEnemySkin(SkinForceType::Team));
}

TEST(SkinForceTypeTest, HasPowerupSkin) {
    EXPECT_TRUE(hasPowerupSkin(SkinForceType::TeamQuad));
    EXPECT_TRUE(hasPowerupSkin(SkinForceType::EnemyQuad));
    EXPECT_TRUE(hasPowerupSkin(SkinForceType::TeamPent));
    EXPECT_TRUE(hasPowerupSkin(SkinForceType::TeamBoth));
    
    EXPECT_FALSE(hasPowerupSkin(SkinForceType::Normal));
    EXPECT_FALSE(hasPowerupSkin(SkinForceType::Team));
    EXPECT_FALSE(hasPowerupSkin(SkinForceType::Enemy));
}

//=============================================================================
// Skin Force Config Tests
//=============================================================================

TEST(SkinForceConfigTest, DetermineSkinType) {
    // No powerups
    EXPECT_EQ(SkinForceConfig::determineSkinType(true, false, false), SkinForceType::Team);
    EXPECT_EQ(SkinForceConfig::determineSkinType(false, false, false), SkinForceType::Enemy);
    
    // Quad only
    EXPECT_EQ(SkinForceConfig::determineSkinType(true, true, false), SkinForceType::TeamQuad);
    EXPECT_EQ(SkinForceConfig::determineSkinType(false, true, false), SkinForceType::EnemyQuad);
    
    // Pent only
    EXPECT_EQ(SkinForceConfig::determineSkinType(true, false, true), SkinForceType::TeamPent);
    EXPECT_EQ(SkinForceConfig::determineSkinType(false, false, true), SkinForceType::EnemyPent);
    
    // Both powerups
    EXPECT_EQ(SkinForceConfig::determineSkinType(true, true, true), SkinForceType::TeamBoth);
    EXPECT_EQ(SkinForceConfig::determineSkinType(false, true, true), SkinForceType::EnemyBoth);
}

TEST(SkinForceConfigTest, GetSkin) {
    SkinForceConfig config;
    config.teamSkin = "blue_player";
    config.enemySkin = "red_player";
    config.teamQuadSkin = "blue_quad";
    
    EXPECT_EQ(config.getSkin(SkinForceType::Team), "blue_player");
    EXPECT_EQ(config.getSkin(SkinForceType::Enemy), "red_player");
    EXPECT_EQ(config.getSkin(SkinForceType::TeamQuad), "blue_quad");
    EXPECT_EQ(config.getSkin(SkinForceType::Normal), "");
}

//=============================================================================
// Color Force Config Tests
//=============================================================================

TEST(ColorForceConfigTest, DefaultNoForcing) {
    ColorForceConfig config;
    
    EXPECT_FALSE(config.forcingTeamColors());
    EXPECT_FALSE(config.forcingEnemyColors());
    EXPECT_FALSE(config.forcingAnyColors());
}

TEST(ColorForceConfigTest, TeamColorsSet) {
    ColorForceConfig config;
    config.teamTopColor = 13;
    
    EXPECT_TRUE(config.forcingTeamColors());
    EXPECT_FALSE(config.forcingEnemyColors());
    EXPECT_TRUE(config.forcingAnyColors());
}

TEST(ColorForceConfigTest, EnemyColorsSet) {
    ColorForceConfig config;
    config.enemyTopColor = 4;
    config.enemyBottomColor = 4;
    
    EXPECT_FALSE(config.forcingTeamColors());
    EXPECT_TRUE(config.forcingEnemyColors());
    EXPECT_TRUE(config.forcingAnyColors());
}

//=============================================================================
// Teamplay Config Tests
//=============================================================================

TEST(TeamplayConfigTest, CanForceSkins) {
    TeamplayConfig config;
    
    // No FPD restrictions
    EXPECT_TRUE(config.canForceSkins(0));
    
    // FPD restricts skin forcing
    EXPECT_FALSE(config.canForceSkins(static_cast<uint16_t>(FPDFlag::NoForceSkin)));
}

TEST(TeamplayConfigTest, CanForceColors) {
    TeamplayConfig config;
    
    // No FPD restrictions
    EXPECT_TRUE(config.canForceColors(0));
    
    // FPD restricts color forcing
    EXPECT_FALSE(config.canForceColors(static_cast<uint16_t>(FPDFlag::NoForceColor)));
}

//=============================================================================
// Need Assessment Tests
//=============================================================================

TEST(NeedAssessmentTest, HasAnyNeeds) {
    NeedAssessment need;
    EXPECT_FALSE(need.hasAnyNeeds());
    
    need.needsHealth = true;
    EXPECT_TRUE(need.hasAnyNeeds());
}

TEST(NeedAssessmentTest, Priority) {
    NeedAssessment need;
    EXPECT_EQ(need.priority(), 0);
    
    need.needsAmmo = true;
    EXPECT_EQ(need.priority(), 1);
    
    need.needsArmor = true;
    EXPECT_EQ(need.priority(), 3);
    
    need.needsHealth = true;
    EXPECT_EQ(need.priority(), 7);
    
    need.needsWeapon = true;
    EXPECT_EQ(need.priority(), 10);
}

TEST(NeedCalculationTest, LowHealth) {
    auto need = calculateNeed(30, 100, 100, 100, 100, 100, 
                               itemFlagBit(ItemFlag::RocketLauncher) | 
                               itemFlagBit(ItemFlag::LightningGun));
    
    EXPECT_TRUE(need.needsHealth);
    EXPECT_FALSE(need.needsArmor);
    EXPECT_FALSE(need.needsWeapon);
}

TEST(NeedCalculationTest, LowArmor) {
    auto need = calculateNeed(100, 50, 100, 100, 100, 100,
                               itemFlagBit(ItemFlag::RocketLauncher) |
                               itemFlagBit(ItemFlag::LightningGun));
    
    EXPECT_FALSE(need.needsHealth);
    EXPECT_TRUE(need.needsArmor);
}

TEST(NeedCalculationTest, NeedWeapon) {
    auto need = calculateNeed(100, 100, 100, 100, 100, 100, 0);
    
    EXPECT_TRUE(need.needsWeapon);
    EXPECT_TRUE((need.needFlags & itemFlagBit(ItemFlag::RocketLauncher)) != 0);
    EXPECT_TRUE((need.needFlags & itemFlagBit(ItemFlag::LightningGun)) != 0);
}

TEST(NeedCalculationTest, LowAmmo) {
    auto need = calculateNeed(100, 100, 10, 2, 100, 100,
                               itemFlagBit(ItemFlag::RocketLauncher) |
                               itemFlagBit(ItemFlag::LightningGun));
    
    EXPECT_TRUE(need.needsAmmo);
    EXPECT_TRUE((need.needFlags & itemFlagBit(ItemFlag::Rockets)) != 0);
    EXPECT_TRUE((need.needFlags & itemFlagBit(ItemFlag::Cells)) != 0);
}

TEST(NeedCalculationTest, AllGood) {
    auto need = calculateNeed(100, 200, 100, 100, 100, 100,
                               itemFlagBit(ItemFlag::RocketLauncher) |
                               itemFlagBit(ItemFlag::LightningGun));
    
    EXPECT_FALSE(need.needsHealth);
    EXPECT_FALSE(need.needsArmor);
    EXPECT_FALSE(need.needsWeapon);
    EXPECT_FALSE(need.needsAmmo);
    EXPECT_FALSE(need.hasAnyNeeds());
}

} // anonymous namespace
} // namespace ezquake::teamplay
