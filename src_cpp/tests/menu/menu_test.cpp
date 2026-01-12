/**
 * @file menu_test.cpp
 * @brief Tests for menu system, key handling, and settings page types
 */

#include <gtest/gtest.h>
#include "core/menu/menu_types.hpp"

using namespace ezquake::menu;

// =============================================================================
// Menu Limits Tests
// =============================================================================

TEST(MenuLimitsTest, Constants) {
    EXPECT_EQ(menu_limits::OPT_PADDING, 4);
    EXPECT_EQ(menu_limits::CMDLINES, 256);
    EXPECT_EQ(menu_limits::MAXCMDLINE, 256);
    EXPECT_EQ(menu_limits::MAX_MOUSE_BUTTONS, 9);
}

// =============================================================================
// Key Code Tests
// =============================================================================

TEST(KeyCodeTest, BasicValues) {
    EXPECT_EQ(static_cast<int32_t>(KeyCode::Tab), 9);
    EXPECT_EQ(static_cast<int32_t>(KeyCode::Enter), 13);
    EXPECT_EQ(static_cast<int32_t>(KeyCode::Escape), 27);
    EXPECT_EQ(static_cast<int32_t>(KeyCode::Space), 32);
}

TEST(KeyCodeTest, FunctionKeys) {
    EXPECT_EQ(static_cast<int32_t>(KeyCode::F1), 148);
    EXPECT_EQ(static_cast<int32_t>(KeyCode::F12), 159);
}

TEST(KeyCodeTest, MouseButtons) {
    EXPECT_EQ(static_cast<int32_t>(KeyCode::Mouse1), 200);
    EXPECT_EQ(static_cast<int32_t>(KeyCode::Mouse8), 207);
}

TEST(KeyCodeTest, IsMouseButton) {
    EXPECT_TRUE(isMouseButton(KeyCode::Mouse1));
    EXPECT_TRUE(isMouseButton(KeyCode::Mouse5));
    EXPECT_FALSE(isMouseButton(KeyCode::Space));
    EXPECT_FALSE(isMouseButton(KeyCode::F1));
}

TEST(KeyCodeTest, IsArrowKey) {
    EXPECT_TRUE(isArrowKey(KeyCode::UpArrow));
    EXPECT_TRUE(isArrowKey(KeyCode::RightArrow));
    EXPECT_FALSE(isArrowKey(KeyCode::Space));
}

TEST(KeyCodeTest, IsFunctionKey) {
    EXPECT_TRUE(isFunctionKey(KeyCode::F1));
    EXPECT_TRUE(isFunctionKey(KeyCode::F12));
    EXPECT_FALSE(isFunctionKey(KeyCode::Escape));
}

TEST(KeyCodeTest, IsModifierKey) {
    EXPECT_TRUE(isModifierKey(KeyCode::Alt));
    EXPECT_TRUE(isModifierKey(KeyCode::Ctrl));
    EXPECT_TRUE(isModifierKey(KeyCode::Shift));
    EXPECT_FALSE(isModifierKey(KeyCode::Space));
}

// =============================================================================
// Key Destination Tests
// =============================================================================

TEST(KeyDestTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(KeyDest::Game), 0);
    EXPECT_EQ(static_cast<uint8_t>(KeyDest::Console), 1);
    EXPECT_EQ(static_cast<uint8_t>(KeyDest::Menu), 3);
}

TEST(KeyDestTest, IsStartupDemo) {
    EXPECT_FALSE(isStartupDemo(KeyDest::Game));
    EXPECT_FALSE(isStartupDemo(KeyDest::Console));
    EXPECT_TRUE(isStartupDemo(KeyDest::StartupDemoMenu));
    EXPECT_TRUE(isStartupDemo(KeyDest::StartupDemoBrowser));
}

TEST(KeyDestTest, Names) {
    EXPECT_STREQ(getKeyDestName(KeyDest::Game), "game");
    EXPECT_STREQ(getKeyDestName(KeyDest::Console), "console");
    EXPECT_STREQ(getKeyDestName(KeyDest::Menu), "menu");
}

// =============================================================================
// Menu State Tests
// =============================================================================

TEST(MenuPageStateTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(MenuState::None), 0);
    EXPECT_EQ(static_cast<uint8_t>(MenuState::Main), 1);
    EXPECT_EQ(static_cast<uint8_t>(MenuState::Options), 9);
}

TEST(MenuPageStateTest, Names) {
    EXPECT_STREQ(getMenuStateName(MenuState::None), "None");
    EXPECT_STREQ(getMenuStateName(MenuState::Main), "Main");
    EXPECT_STREQ(getMenuStateName(MenuState::Options), "Options");
}

TEST(MenuPageStateTest, IsInMenu) {
    EXPECT_FALSE(isInMenu(MenuState::None));
    EXPECT_TRUE(isInMenu(MenuState::Main));
    EXPECT_TRUE(isInMenu(MenuState::Options));
}

// =============================================================================
// Chat Type Tests
// =============================================================================

TEST(ChatTypeTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(ChatType::MessageMode1), 0);
    EXPECT_EQ(static_cast<uint8_t>(ChatType::MessageMode2), 1);
    EXPECT_EQ(static_cast<uint8_t>(ChatType::QtvToGame), 3);
}

// =============================================================================
// Mouse State Tests
// =============================================================================

TEST(MenuMouseStateTest, IsButtonDown) {
    MouseState state;
    EXPECT_FALSE(state.isButtonDown(1));
    
    state.buttons[1] = true;
    EXPECT_TRUE(state.isButtonDown(1));
    EXPECT_FALSE(state.isButtonDown(2));
}

TEST(MenuMouseStateTest, InvalidButtonIndex) {
    MouseState state;
    EXPECT_FALSE(state.isButtonDown(0));   // Button 0 is unused
    EXPECT_FALSE(state.isButtonDown(9));   // Out of range
    EXPECT_FALSE(state.isButtonDown(-1));  // Negative
}

TEST(MenuMouseStateTest, Delta) {
    MouseState state;
    state.x = 100;
    state.y = 200;
    state.xOld = 90;
    state.yOld = 180;
    
    EXPECT_DOUBLE_EQ(state.deltaX(), 10);
    EXPECT_DOUBLE_EQ(state.deltaY(), 20);
}

TEST(MenuMouseStateTest, HasMoved) {
    MouseState state;
    EXPECT_FALSE(state.hasMoved());
    
    state.x = 1;
    EXPECT_TRUE(state.hasMoved());
}

TEST(MenuMouseStateTest, AnyButtonDown) {
    MouseState state;
    EXPECT_FALSE(state.anyButtonDown());
    
    state.buttons[5] = true;
    EXPECT_TRUE(state.anyButtonDown());
}

// =============================================================================
// Setting Type Tests
// =============================================================================

TEST(SettingTypeTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(SettingType::Separator), 0);
    EXPECT_EQ(static_cast<uint8_t>(SettingType::Number), 1);
    EXPECT_EQ(static_cast<uint8_t>(SettingType::Bool), 3);
    EXPECT_EQ(static_cast<uint8_t>(SettingType::Bind), 11);
}

TEST(SettingTypeTest, Names) {
    EXPECT_STREQ(getSettingTypeName(SettingType::Number), "Number");
    EXPECT_STREQ(getSettingTypeName(SettingType::Bool), "Bool");
    EXPECT_STREQ(getSettingTypeName(SettingType::String), "String");
}

TEST(SettingTypeTest, IsDecorative) {
    EXPECT_TRUE(isDecorative(SettingType::Separator));
    EXPECT_TRUE(isDecorative(SettingType::Blank));
    EXPECT_TRUE(isDecorative(SettingType::AdvancedMark));
    EXPECT_FALSE(isDecorative(SettingType::Number));
    EXPECT_FALSE(isDecorative(SettingType::Bool));
}

TEST(SettingTypeTest, IsEditable) {
    EXPECT_TRUE(isEditable(SettingType::Number));
    EXPECT_TRUE(isEditable(SettingType::Bool));
    EXPECT_TRUE(isEditable(SettingType::String));
    EXPECT_FALSE(isEditable(SettingType::Separator));
    EXPECT_FALSE(isEditable(SettingType::Blank));
}

// =============================================================================
// Settings Page Mode Tests
// =============================================================================

TEST(SettingsPageModeTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(SettingsPageMode::Normal), 0);
    EXPECT_EQ(static_cast<uint8_t>(SettingsPageMode::Binding), 1);
    EXPECT_EQ(static_cast<uint8_t>(SettingsPageMode::ViewHelp), 2);
}

// =============================================================================
// Setting Definition Tests
// =============================================================================

TEST(SettingDefTest, HasRange) {
    SettingDef numSetting;
    numSetting.type = SettingType::Number;
    EXPECT_TRUE(numSetting.hasRange());
    
    SettingDef boolSetting;
    boolSetting.type = SettingType::Bool;
    EXPECT_FALSE(boolSetting.hasRange());
}

TEST(SettingDefTest, ClampValue) {
    SettingDef setting;
    setting.min = 0.0f;
    setting.max = 100.0f;
    
    EXPECT_FLOAT_EQ(setting.clampValue(50.0f), 50.0f);
    EXPECT_FLOAT_EQ(setting.clampValue(-10.0f), 0.0f);
    EXPECT_FLOAT_EQ(setting.clampValue(150.0f), 100.0f);
}

TEST(SettingDefTest, NumSteps) {
    SettingDef setting;
    setting.min = 0.0f;
    setting.max = 100.0f;
    setting.step = 10.0f;
    
    EXPECT_EQ(setting.numSteps(), 10);
}

// =============================================================================
// Settings Page State Tests
// =============================================================================

TEST(SettingsPageStateTest, Reset) {
    SettingsPageState state;
    state.marked = 5;
    state.viewpoint = 10;
    state.mode = SettingsPageMode::Binding;
    
    state.reset();
    
    EXPECT_EQ(state.marked, 0);
    EXPECT_EQ(state.viewpoint, 0);
    EXPECT_EQ(state.mode, SettingsPageMode::Normal);
}

TEST(SettingsPageStateTest, IsBinding) {
    SettingsPageState state;
    EXPECT_FALSE(state.isBinding());
    
    state.mode = SettingsPageMode::Binding;
    EXPECT_TRUE(state.isBinding());
}

TEST(SettingsPageStateTest, ScrollTo) {
    SettingsPageState state;
    state.count = 10;
    
    state.scrollTo(5);
    EXPECT_EQ(state.marked, 5);
    
    state.scrollTo(15);  // Out of range
    EXPECT_EQ(state.marked, 5);  // Unchanged
}

// =============================================================================
// Key Binding Tests
// =============================================================================

TEST(KeyBindingTest, IsBound) {
    KeyBinding binding;
    EXPECT_FALSE(binding.isBound());
    
    binding.command = "+forward";
    EXPECT_TRUE(binding.isBound());
}

// =============================================================================
// Key State Tests
// =============================================================================

TEST(KeyStateTest, IsDown) {
    KeyState state;
    EXPECT_FALSE(state.isDown(KeyCode::Space));
    
    state.setDown(KeyCode::Space, true);
    EXPECT_TRUE(state.isDown(KeyCode::Space));
    EXPECT_EQ(state.lastPress, static_cast<int32_t>(KeyCode::Space));
}

TEST(KeyStateTest, ClearAll) {
    KeyState state;
    state.setDown(KeyCode::Space, true);
    state.setDown(KeyCode::Enter, true);
    
    state.clearAll();
    
    EXPECT_FALSE(state.isDown(KeyCode::Space));
    EXPECT_FALSE(state.isDown(KeyCode::Enter));
    EXPECT_EQ(state.lastPress, 0);
}

// =============================================================================
// Menu Draw Info Tests
// =============================================================================

TEST(MenuDrawInfoTest, CenterX) {
    MenuDrawInfo info;
    info.x = 100;
    info.width = 200;
    
    EXPECT_EQ(info.centerX(), 200);
}

TEST(MenuDrawInfoTest, ContainsPoint) {
    MenuDrawInfo info;
    info.x = 100;
    info.y = 100;
    info.width = 200;
    info.height = 150;
    
    EXPECT_TRUE(info.containsPoint(150, 150));
    EXPECT_TRUE(info.containsPoint(100, 100));
    EXPECT_FALSE(info.containsPoint(50, 50));
    EXPECT_FALSE(info.containsPoint(300, 300));
}

// =============================================================================
// Slider Control Tests
// =============================================================================

TEST(SliderControlTest, NormalizedValue) {
    SliderControl slider;
    slider.min = 0.0f;
    slider.max = 100.0f;
    slider.value = 50.0f;
    
    EXPECT_FLOAT_EQ(slider.normalizedValue(), 0.5f);
}

TEST(SliderControlTest, Increment) {
    SliderControl slider;
    slider.min = 0.0f;
    slider.max = 100.0f;
    slider.step = 10.0f;
    slider.value = 50.0f;
    
    slider.increment();
    EXPECT_FLOAT_EQ(slider.value, 60.0f);
    
    slider.value = 95.0f;
    slider.increment();
    EXPECT_FLOAT_EQ(slider.value, 100.0f);  // Clamped to max
}

TEST(SliderControlTest, Decrement) {
    SliderControl slider;
    slider.min = 0.0f;
    slider.max = 100.0f;
    slider.step = 10.0f;
    slider.value = 50.0f;
    
    slider.decrement();
    EXPECT_FLOAT_EQ(slider.value, 40.0f);
    
    slider.value = 5.0f;
    slider.decrement();
    EXPECT_FLOAT_EQ(slider.value, 0.0f);  // Clamped to min
}

TEST(SliderControlTest, ValueFromPosition) {
    SliderControl slider;
    slider.x = 100;
    slider.width = 200;
    slider.min = 0.0f;
    slider.max = 100.0f;
    
    EXPECT_FLOAT_EQ(slider.valueFromPosition(100), 0.0f);
    EXPECT_FLOAT_EQ(slider.valueFromPosition(200), 50.0f);
    EXPECT_FLOAT_EQ(slider.valueFromPosition(300), 100.0f);
}

// =============================================================================
// Flashing Cursor Tests
// =============================================================================

TEST(FlashingCursorTest, ArrowFrame) {
    FlashingCursor cursor;
    cursor.curtime = 0.0f;
    EXPECT_EQ(cursor.arrowFrame(), 12);
    
    cursor.curtime = 0.25f;
    EXPECT_EQ(cursor.arrowFrame(), 13);
}

TEST(FlashingCursorTest, CursorFrame) {
    FlashingCursor cursor;
    cursor.curtime = 0.0f;
    EXPECT_EQ(cursor.cursorFrame(), 10);
    
    cursor.curtime = 0.25f;
    EXPECT_EQ(cursor.cursorFrame(), 11);
}

// =============================================================================
// Player Color Tests
// =============================================================================

TEST(PlayerColorTest, IsValid) {
    PlayerColor color;
    EXPECT_TRUE(color.isValid());
    
    color.top = 16;  // Out of range
    EXPECT_FALSE(color.isValid());
}

TEST(PlayerColorTest, Combined) {
    PlayerColor color;
    color.top = 4;
    color.bottom = 13;
    
    EXPECT_EQ(color.combined(), 0x4D);  // (4 << 4) | 13
}

TEST(PlayerColorTest, FromCombined) {
    auto color = PlayerColor::fromCombined(0x4D);
    
    EXPECT_EQ(color.top, 4);
    EXPECT_EQ(color.bottom, 13);
}
