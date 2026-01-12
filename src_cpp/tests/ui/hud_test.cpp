/**
 * @file hud_test.cpp
 * @brief Tests for HUD, Menu, and UI types
 */

#include <gtest/gtest.h>
#include "core/ui/hud_types.hpp"

using namespace ezquake::ui;

// =============================================================================
// HUD Limits Tests
// =============================================================================

TEST(HudLimitsTest, Constants) {
    EXPECT_EQ(hud_limits::MAX_ELEMENTS, 256);
    EXPECT_EQ(hud_limits::MAX_PARAMS, 32);
    EXPECT_EQ(hud_limits::REGEXP_OFFSET_COUNT, 20);
    EXPECT_EQ(hud_limits::LETTER_WIDTH, 8);
    EXPECT_EQ(hud_limits::LETTER_HEIGHT, 8);
}

// =============================================================================
// HUD Flags Tests
// =============================================================================

TEST(HudFlagsTest, Values) {
    EXPECT_EQ(HudFlags::NoDraw, 1u << 0);
    EXPECT_EQ(HudFlags::NoShow, 1u << 1);
    EXPECT_EQ(HudFlags::OnIntermission, 1u << 6);
    EXPECT_EQ(HudFlags::Opacity, 1u << 11);
}

TEST(HudFlagsTest, HasFlag) {
    uint32_t flags = HudFlags::NoDraw | HudFlags::OnIntermission;
    
    EXPECT_TRUE(HudFlags::hasFlag(flags, HudFlags::NoDraw));
    EXPECT_TRUE(HudFlags::hasFlag(flags, HudFlags::OnIntermission));
    EXPECT_FALSE(HudFlags::hasFlag(flags, HudFlags::NoShow));
}

TEST(HudFlagsTest, Names) {
    EXPECT_STREQ(HudFlags::getFlagName(HudFlags::NoDraw), "NoDraw");
    EXPECT_STREQ(HudFlags::getFlagName(HudFlags::OnIntermission), "OnIntermission");
    EXPECT_STREQ(HudFlags::getFlagName(HudFlags::Inventory), "NoGrow");
}

// =============================================================================
// HUD Placement Tests
// =============================================================================

TEST(HudPlacementTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(HudPlacement::Screen), 1);
    EXPECT_EQ(static_cast<uint8_t>(HudPlacement::Sbar), 4);
    EXPECT_EQ(static_cast<uint8_t>(HudPlacement::HFree), 9);
}

TEST(HudPlacementTest, Names) {
    EXPECT_STREQ(getPlacementName(HudPlacement::Screen), "Screen");
    EXPECT_STREQ(getPlacementName(HudPlacement::Sbar), "Sbar");
    EXPECT_STREQ(getPlacementName(HudPlacement::View), "View");
}

// =============================================================================
// HUD Alignment Tests
// =============================================================================

TEST(HudAlignmentTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(HudAlign::Left), 1);
    EXPECT_EQ(static_cast<uint8_t>(HudAlign::Top), 1);
    EXPECT_EQ(static_cast<uint8_t>(HudAlign::Center), 2);
    EXPECT_EQ(static_cast<uint8_t>(HudAlign::Right), 3);
    EXPECT_EQ(static_cast<uint8_t>(HudAlign::Bottom), 3);
}

TEST(HudAlignmentTest, Names) {
    EXPECT_STREQ(getAlignmentName(HudAlign::Left), "Left");
    EXPECT_STREQ(getAlignmentName(HudAlign::Center), "Center");
    EXPECT_STREQ(getAlignmentName(HudAlign::Console), "Console");
}

// =============================================================================
// HUD Rect Tests
// =============================================================================

TEST(HudRectTest, DefaultConstruction) {
    HudRect rect;
    EXPECT_EQ(rect.x, 0);
    EXPECT_EQ(rect.y, 0);
    EXPECT_EQ(rect.width, 0);
    EXPECT_EQ(rect.height, 0);
}

TEST(HudRectTest, Dimensions) {
    HudRect rect{10, 20, 100, 50};
    EXPECT_EQ(rect.right(), 110);
    EXPECT_EQ(rect.bottom(), 70);
}

TEST(HudRectTest, IsEmpty) {
    HudRect empty;
    EXPECT_TRUE(empty.isEmpty());
    
    HudRect nonEmpty{0, 0, 10, 10};
    EXPECT_FALSE(nonEmpty.isEmpty());
}

TEST(HudRectTest, Contains) {
    HudRect rect{10, 10, 100, 50};
    
    EXPECT_TRUE(rect.contains(10, 10));   // Top-left
    EXPECT_TRUE(rect.contains(50, 30));   // Center
    EXPECT_TRUE(rect.contains(109, 59));  // Just inside bottom-right
    EXPECT_FALSE(rect.contains(110, 60)); // Outside
    EXPECT_FALSE(rect.contains(5, 30));   // Left of rect
}

// =============================================================================
// HUD Frame Tests
// =============================================================================

TEST(HudFrameTest, Padding) {
    HudFrame frame{5, 10, 3, 7};
    EXPECT_EQ(frame.horizontalPadding(), 15);
    EXPECT_EQ(frame.verticalPadding(), 10);
}

// =============================================================================
// HUD Color Tests
// =============================================================================

TEST(HudColorTest, DefaultConstruction) {
    HudColor color;
    EXPECT_EQ(color.r, 255);
    EXPECT_EQ(color.g, 255);
    EXPECT_EQ(color.b, 255);
    EXPECT_EQ(color.a, 255);
}

TEST(HudColorTest, Presets) {
    auto white = HudColor::white();
    EXPECT_EQ(white.r, 255);
    EXPECT_EQ(white.a, 255);
    
    auto transparent = HudColor::transparent();
    EXPECT_EQ(transparent.a, 0);
    
    auto red = HudColor::red();
    EXPECT_EQ(red.r, 255);
    EXPECT_EQ(red.g, 0);
}

TEST(HudColorTest, WithOpacity) {
    HudColor color(255, 128, 64, 255);
    auto faded = color.withOpacity(0.5f);
    
    EXPECT_EQ(faded.r, 255);
    EXPECT_EQ(faded.g, 128);
    EXPECT_EQ(faded.b, 64);
    EXPECT_EQ(faded.a, 127);
}

// =============================================================================
// Menu State Tests
// =============================================================================

TEST(MenuStateTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(MenuState::None), 0);
    EXPECT_EQ(static_cast<uint8_t>(MenuState::Main), 1);
    EXPECT_EQ(static_cast<uint8_t>(MenuState::InGame), 12);
}

TEST(MenuStateTest, Names) {
    EXPECT_STREQ(getMenuStateName(MenuState::None), "None");
    EXPECT_STREQ(getMenuStateName(MenuState::Main), "Main");
    EXPECT_STREQ(getMenuStateName(MenuState::Options), "Options");
}

TEST(MenuStateTest, IsActive) {
    EXPECT_FALSE(isMenuActive(MenuState::None));
    EXPECT_TRUE(isMenuActive(MenuState::Main));
    EXPECT_TRUE(isMenuActive(MenuState::InGame));
}

// =============================================================================
// Slider Range Tests
// =============================================================================

TEST(SliderRangeTest, Normalized) {
    SliderRange slider{0.0f, 100.0f, 10.0f, 50.0f};
    EXPECT_NEAR(slider.normalized(), 0.5f, 0.001f);
}

TEST(SliderRangeTest, SetNormalized) {
    SliderRange slider{0.0f, 100.0f, 10.0f, 0.0f};
    slider.setNormalized(0.75f);
    EXPECT_NEAR(slider.current, 75.0f, 0.001f);
}

TEST(SliderRangeTest, Clamp) {
    SliderRange slider{0.0f, 100.0f, 10.0f, 150.0f};
    slider.clamp();
    EXPECT_EQ(slider.current, 100.0f);
    
    slider.current = -50.0f;
    slider.clamp();
    EXPECT_EQ(slider.current, 0.0f);
}

TEST(SliderRangeTest, IncrementDecrement) {
    SliderRange slider{0.0f, 100.0f, 10.0f, 50.0f};
    
    slider.increment();
    EXPECT_NEAR(slider.current, 60.0f, 0.001f);
    
    slider.decrement();
    slider.decrement();
    EXPECT_NEAR(slider.current, 40.0f, 0.001f);
}

// =============================================================================
// Scrollbar State Tests
// =============================================================================

TEST(ScrollbarStateTest, Normalized) {
    ScrollbarState scrollbar{50.0, 8, 100, false};
    EXPECT_NEAR(scrollbar.normalized(), 0.5, 0.001);
}

TEST(ScrollbarStateTest, SetNormalized) {
    ScrollbarState scrollbar{};
    scrollbar.setNormalized(0.25);
    EXPECT_NEAR(scrollbar.position, 25.0, 0.001);
}

TEST(ScrollbarStateTest, Clamp) {
    ScrollbarState scrollbar{150.0, 8, 100, false};
    scrollbar.clamp();
    EXPECT_EQ(scrollbar.position, 100.0);
    
    scrollbar.position = -50.0;
    scrollbar.clamp();
    EXPECT_EQ(scrollbar.position, 0.0);
}

// =============================================================================
// TextBox Spec Tests
// =============================================================================

TEST(TextBoxSpecTest, PixelDimensions) {
    TextBoxSpec spec{10, 20, 10, 5, false};
    
    // 10 chars * 8 + 16 padding = 96
    EXPECT_EQ(spec.pixelWidth(), 96);
    
    // 5 lines * 8 + 16 padding = 56
    EXPECT_EQ(spec.pixelHeight(), 56);
}

// =============================================================================
// Mouse State Tests
// =============================================================================

TEST(MouseStateTest, ButtonPressed) {
    MouseState state;
    state.button = MouseButton::Left;
    state.buttonDown = true;
    
    EXPECT_TRUE(state.isButtonPressed());
    EXPECT_FALSE(state.isButtonReleased());
}

TEST(MouseStateTest, HasScroll) {
    MouseState state;
    EXPECT_FALSE(state.hasScroll());
    
    state.scrollDelta = 1;
    EXPECT_TRUE(state.hasScroll());
}

// =============================================================================
// Draw Constants Tests
// =============================================================================

TEST(DrawConstantsTest, Padding) {
    EXPECT_EQ(draw_constants::OPT_PADDING, 4);
}

TEST(DrawConstantsTest, FlashingChars) {
    // Should alternate between two values
    int arrow1 = draw_constants::flashingArrow(0.0f);
    int arrow2 = draw_constants::flashingArrow(0.25f);
    
    EXPECT_GE(arrow1, 12);
    EXPECT_LE(arrow1, 13);
    EXPECT_GE(arrow2, 12);
    EXPECT_LE(arrow2, 13);
}

// =============================================================================
// HUD Draw Info Tests
// =============================================================================

TEST(HudDrawInfoTest, ContentArea) {
    HudDrawInfo info;
    info.bounds = {10, 10, 100, 80};
    info.frame = {5, 10, 5, 10};
    
    auto content = info.contentArea();
    EXPECT_EQ(content.x, 15);      // 10 + 5
    EXPECT_EQ(content.y, 15);      // 10 + 5
    EXPECT_EQ(content.width, 85);  // 100 - 15
    EXPECT_EQ(content.height, 65); // 80 - 15
}

// =============================================================================
// Stack Calculation Tests
// =============================================================================

TEST(StackCalculationTest, NoArmor) {
    float stack = calculateStack(100.0f, 0.0f, 0.0f);
    EXPECT_NEAR(stack, 100.0f, 0.001f);
}

TEST(StackCalculationTest, GreenArmor) {
    // 100 health + 100 armor at 30% = 100 + 100/0.3 = 433.33
    float stack = calculateStack(100.0f, 100.0f, 0.3f);
    EXPECT_NEAR(stack, 433.33f, 0.1f);
}

TEST(StackCalculationTest, RedArmor) {
    // 100 health + 200 armor at 80% = 100 + 200/0.8 = 350
    float stack = calculateStack(100.0f, 200.0f, 0.8f);
    EXPECT_NEAR(stack, 350.0f, 0.1f);
}

// =============================================================================
// Armor Type Tests
// =============================================================================

TEST(ArmorTypeTest, NoArmor) {
    EXPECT_NEAR(getArmorType(0), 0.0f, 0.001f);
}

TEST(ArmorTypeTest, GreenArmor) {
    EXPECT_NEAR(getArmorType(0x2000), 0.3f, 0.001f);
}

TEST(ArmorTypeTest, YellowArmor) {
    EXPECT_NEAR(getArmorType(0x4000), 0.6f, 0.001f);
}

TEST(ArmorTypeTest, RedArmor) {
    EXPECT_NEAR(getArmorType(0x8000), 0.8f, 0.001f);
}

TEST(ArmorTypeTest, RedArmorPriority) {
    // Red armor takes priority when multiple flags set
    EXPECT_NEAR(getArmorType(0x8000 | 0x4000 | 0x2000), 0.8f, 0.001f);
}

// =============================================================================
// Scoreboard Flags Tests
// =============================================================================

TEST(ScoreboardFlagsTest, Values) {
    EXPECT_EQ(ScoreboardFlags::All, 0xFFFFFFFF);
    EXPECT_EQ(ScoreboardFlags::SortTeams, 1u);
    EXPECT_EQ(ScoreboardFlags::SortPlayers, 2u);
    EXPECT_EQ(ScoreboardFlags::Update, 4u);
    EXPECT_EQ(ScoreboardFlags::AvgPing, 8u);
}

// =============================================================================
// HUD Number Style Tests
// =============================================================================

TEST(HudNumberStyleTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(HudNumberStyle::Normal), 0);
    EXPECT_EQ(static_cast<uint8_t>(HudNumberStyle::Large), 1);
    EXPECT_EQ(static_cast<uint8_t>(HudNumberStyle::Small), 2);
    EXPECT_EQ(static_cast<uint8_t>(HudNumberStyle::Proportional), 3);
}

// =============================================================================
// HUD Min State Tests
// =============================================================================

TEST(HudMinStateTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(HudMinState::Disconnected), 0);
    EXPECT_EQ(static_cast<uint8_t>(HudMinState::Connected), 1);
    EXPECT_EQ(static_cast<uint8_t>(HudMinState::OnServer), 2);
    EXPECT_EQ(static_cast<uint8_t>(HudMinState::Active), 3);
}

// =============================================================================
// Sorted Team Info Tests
// =============================================================================

TEST(SortedTeamInfoTest, DefaultValues) {
    SortedTeamInfo team;
    EXPECT_EQ(team.frags, 0);
    EXPECT_EQ(team.playerCount, 0);
    EXPECT_EQ(team.rlCount, 0);
    EXPECT_EQ(team.stack, 0);
}

// =============================================================================
// Sorted Player Info Tests
// =============================================================================

TEST(SortedPlayerInfoTest, DefaultValues) {
    SortedPlayerInfo player;
    EXPECT_EQ(player.playerNum, -1);
    EXPECT_EQ(player.team, nullptr);
}
