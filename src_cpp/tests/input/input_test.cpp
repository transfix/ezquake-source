/*
 * ezQuake C++ Port - Input System Unit Tests
 * 
 * Copyright (C) 2024 ezQuake Team
 */

#include <gtest/gtest.h>
#include "core/input/input.hpp"

using namespace ezquake;
using namespace ezquake::input;

//=============================================================================
// KeyCode Tests
//=============================================================================

TEST(KeyCodeTest, MouseButtonDetection) {
    EXPECT_TRUE(isMouseButton(KeyCode::Mouse1));
    EXPECT_TRUE(isMouseButton(KeyCode::Mouse2));
    EXPECT_TRUE(isMouseButton(KeyCode::Mouse8));
    EXPECT_FALSE(isMouseButton(KeyCode::Space));
    EXPECT_FALSE(isMouseButton(KeyCode::Joy1));
}

TEST(KeyCodeTest, MouseWheelDetection) {
    EXPECT_TRUE(isMouseWheel(KeyCode::MouseWheelUp));
    EXPECT_TRUE(isMouseWheel(KeyCode::MouseWheelDown));
    EXPECT_FALSE(isMouseWheel(KeyCode::Mouse1));
    EXPECT_FALSE(isMouseWheel(KeyCode::Space));
}

TEST(KeyCodeTest, JoystickDetection) {
    EXPECT_TRUE(isJoystickButton(KeyCode::Joy1));
    EXPECT_TRUE(isJoystickButton(KeyCode::Joy4));
    EXPECT_TRUE(isJoystickButton(KeyCode::Aux1));
    EXPECT_TRUE(isJoystickButton(KeyCode::Aux28));
    EXPECT_FALSE(isJoystickButton(KeyCode::Mouse1));
}

TEST(KeyCodeTest, FunctionKeyDetection) {
    EXPECT_TRUE(isFunctionKey(KeyCode::F1));
    EXPECT_TRUE(isFunctionKey(KeyCode::F12));
    EXPECT_FALSE(isFunctionKey(KeyCode::Space));
    EXPECT_FALSE(isFunctionKey(KeyCode::Escape));
}

TEST(KeyCodeTest, ModifierKeyDetection) {
    EXPECT_TRUE(isModifierKey(KeyCode::Alt));
    EXPECT_TRUE(isModifierKey(KeyCode::Ctrl));
    EXPECT_TRUE(isModifierKey(KeyCode::Shift));
    EXPECT_TRUE(isModifierKey(KeyCode::LeftAlt));
    EXPECT_TRUE(isModifierKey(KeyCode::RightShift));
    EXPECT_FALSE(isModifierKey(KeyCode::Space));
}

TEST(KeyCodeTest, NumpadDetection) {
    EXPECT_TRUE(isNumpadKey(KeyCode::NumLock));
    EXPECT_TRUE(isNumpadKey(KeyCode::NumpadEnter));
    EXPECT_TRUE(isNumpadKey(KeyCode::Numpad5));
    EXPECT_FALSE(isNumpadKey(KeyCode::Enter));
}

//=============================================================================
// KeyDestination Tests
//=============================================================================

TEST(KeyDestinationTest, StartupDemoMode) {
    EXPECT_FALSE(isStartupDemoMode(KeyDestination::Game));
    EXPECT_FALSE(isStartupDemoMode(KeyDestination::Console));
    EXPECT_FALSE(isStartupDemoMode(KeyDestination::Menu));
    EXPECT_TRUE(isStartupDemoMode(KeyDestination::StartupDemoMenu));
    EXPECT_TRUE(isStartupDemoMode(KeyDestination::StartupDemoBrowser));
}

//=============================================================================
// MouseState Tests
//=============================================================================

TEST(MouseStateTest, DefaultState) {
    MouseState mouse;
    
    EXPECT_DOUBLE_EQ(mouse.x, 0.0);
    EXPECT_DOUBLE_EQ(mouse.y, 0.0);
    EXPECT_DOUBLE_EQ(mouse.deltaX(), 0.0);
    EXPECT_DOUBLE_EQ(mouse.deltaY(), 0.0);
    EXPECT_FALSE(mouse.isButtonDown(1));
}

TEST(MouseStateTest, Delta) {
    MouseState mouse;
    mouse.prevX = 100.0;
    mouse.prevY = 200.0;
    mouse.x = 150.0;
    mouse.y = 180.0;
    
    EXPECT_DOUBLE_EQ(mouse.deltaX(), 50.0);
    EXPECT_DOUBLE_EQ(mouse.deltaY(), -20.0);
}

TEST(MouseStateTest, Buttons) {
    MouseState mouse;
    
    mouse.buttons[1] = true;
    mouse.buttons[3] = true;
    
    EXPECT_TRUE(mouse.isButtonDown(1));
    EXPECT_FALSE(mouse.isButtonDown(2));
    EXPECT_TRUE(mouse.isButtonDown(3));
    EXPECT_FALSE(mouse.isButtonDown(0));  // Invalid
    EXPECT_FALSE(mouse.isButtonDown(9));  // Out of range
}

TEST(MouseStateTest, Clear) {
    MouseState mouse;
    mouse.x = 100.0;
    mouse.buttons[1] = true;
    
    mouse.clear();
    
    EXPECT_DOUBLE_EQ(mouse.x, 0.0);
    EXPECT_FALSE(mouse.isButtonDown(1));
}

//=============================================================================
// KeyButton Tests
//=============================================================================

TEST(KeyButtonTest, DefaultState) {
    KeyButton button;
    
    EXPECT_FALSE(button.isDown());
    EXPECT_FALSE(button.wasJustPressed());
    EXPECT_FALSE(button.wasJustReleased());
    EXPECT_EQ(button.state(), 0);
}

TEST(KeyButtonTest, SingleKeyDown) {
    KeyButton button;
    
    bool changed = button.keyDown(42, 1.0);
    
    EXPECT_TRUE(changed);
    EXPECT_TRUE(button.isDown());
    EXPECT_TRUE(button.wasJustPressed());
    EXPECT_FALSE(button.wasJustReleased());
    EXPECT_DOUBLE_EQ(button.downTime(), 1.0);
    EXPECT_EQ(button.holdingKey0(), 42);
}

TEST(KeyButtonTest, SingleKeyUp) {
    KeyButton button;
    button.keyDown(42, 1.0);
    
    bool isUp = button.keyUp(42, 2.0);
    
    EXPECT_TRUE(isUp);
    EXPECT_FALSE(button.isDown());
    EXPECT_TRUE(button.wasJustReleased());
    EXPECT_DOUBLE_EQ(button.upTime(), 2.0);
}

TEST(KeyButtonTest, TwoKeysHolding) {
    KeyButton button;
    
    button.keyDown(10, 1.0);
    button.keyDown(20, 1.5);
    
    EXPECT_TRUE(button.isDown());
    EXPECT_EQ(button.holdingKey0(), 10);
    EXPECT_EQ(button.holdingKey1(), 20);
    
    // Release first key - still held by second
    bool isUp = button.keyUp(10, 2.0);
    EXPECT_FALSE(isUp);
    EXPECT_TRUE(button.isDown());
    
    // Release second key - now up
    isUp = button.keyUp(20, 2.5);
    EXPECT_TRUE(isUp);
    EXPECT_FALSE(button.isDown());
}

TEST(KeyButtonTest, RepeatingKeyIgnored) {
    KeyButton button;
    
    button.keyDown(42, 1.0);
    bool changed = button.keyDown(42, 1.5);  // Same key again
    
    EXPECT_FALSE(changed);
    EXPECT_EQ(button.holdingKey1(), 0);  // Not stored twice
}

TEST(KeyButtonTest, VoidKeyReleasesAll) {
    KeyButton button;
    button.keyDown(10, 1.0);
    button.keyDown(20, 1.5);
    
    bool isUp = button.keyUp(KEY_VOID, 2.0);
    
    EXPECT_TRUE(isUp);
    EXPECT_FALSE(button.isDown());
    EXPECT_EQ(button.holdingKey0(), 0);
    EXPECT_EQ(button.holdingKey1(), 0);
}

TEST(KeyButtonTest, ClearEdges) {
    KeyButton button;
    button.keyDown(42, 1.0);
    
    EXPECT_TRUE(button.wasJustPressed());
    
    button.clearEdges();
    
    EXPECT_FALSE(button.wasJustPressed());
    EXPECT_TRUE(button.isDown());  // Still down
}

TEST(KeyButtonTest, CalculateKeyState) {
    KeyButton button;
    
    // Not pressed
    EXPECT_FLOAT_EQ(button.calculateKeyState(0.016, 1.0), 0.0f);
    
    // Just pressed
    button.keyDown(42, 1.0);
    EXPECT_FLOAT_EQ(button.calculateKeyState(0.016, 1.016), 0.5f);
    
    // Held (after clearing edges)
    button.clearEdges();
    EXPECT_FLOAT_EQ(button.calculateKeyState(0.016, 1.032), 1.0f);
}

TEST(KeyButtonTest, Clear) {
    KeyButton button;
    button.keyDown(42, 1.0);
    
    button.clear();
    
    EXPECT_FALSE(button.isDown());
    EXPECT_EQ(button.holdingKey0(), 0);
    EXPECT_EQ(button.state(), 0);
}

//=============================================================================
// InputButton Tests
//=============================================================================

TEST(InputButtonTest, ButtonName) {
    EXPECT_EQ(buttonName(InputButton::Forward), "forward");
    EXPECT_EQ(buttonName(InputButton::Attack), "attack");
    EXPECT_EQ(buttonName(InputButton::Jump), "jump");
    EXPECT_EQ(buttonName(InputButton::MLook), "mlook");
}

TEST(InputButtonTest, ParseButtonName) {
    EXPECT_EQ(parseButtonName("forward"), InputButton::Forward);
    EXPECT_EQ(parseButtonName("attack"), InputButton::Attack);
    EXPECT_EQ(parseButtonName("jump"), InputButton::Jump);
    EXPECT_EQ(parseButtonName("invalid"), std::nullopt);
}

//=============================================================================
// KeyNames Tests
//=============================================================================

TEST(KeyNamesTest, SpecialKeyToCode) {
    EXPECT_EQ(stringToKeyCode("SPACE"), KeyCode::Space);
    EXPECT_EQ(stringToKeyCode("ENTER"), KeyCode::Enter);
    EXPECT_EQ(stringToKeyCode("ESCAPE"), KeyCode::Escape);
    EXPECT_EQ(stringToKeyCode("TAB"), KeyCode::Tab);
    EXPECT_EQ(stringToKeyCode("MOUSE1"), KeyCode::Mouse1);
    EXPECT_EQ(stringToKeyCode("F1"), KeyCode::F1);
    EXPECT_EQ(stringToKeyCode("MWHEELUP"), KeyCode::MouseWheelUp);
}

TEST(KeyNamesTest, CaseInsensitive) {
    EXPECT_EQ(stringToKeyCode("space"), KeyCode::Space);
    EXPECT_EQ(stringToKeyCode("Space"), KeyCode::Space);
    EXPECT_EQ(stringToKeyCode("SPACE"), KeyCode::Space);
    EXPECT_EQ(stringToKeyCode("mouse1"), KeyCode::Mouse1);
}

TEST(KeyNamesTest, SingleCharacter) {
    auto aKey = stringToKeyCode("a");
    EXPECT_TRUE(aKey.has_value());
    EXPECT_EQ(static_cast<int>(*aKey), 'a');
    
    auto zKey = stringToKeyCode("Z");
    EXPECT_TRUE(zKey.has_value());
    EXPECT_EQ(static_cast<int>(*zKey), 'z');  // Lowercase
    
    auto numKey = stringToKeyCode("5");
    EXPECT_TRUE(numKey.has_value());
    EXPECT_EQ(static_cast<int>(*numKey), '5');
}

TEST(KeyNamesTest, InvalidKey) {
    EXPECT_EQ(stringToKeyCode(""), std::nullopt);
    EXPECT_EQ(stringToKeyCode("INVALID_KEY_NAME"), std::nullopt);
}

TEST(KeyNamesTest, KeyCodeToString) {
    EXPECT_EQ(keyCodeToString(KeyCode::Space), "SPACE");
    EXPECT_EQ(keyCodeToString(KeyCode::Enter), "ENTER");
    EXPECT_EQ(keyCodeToString(KeyCode::Mouse1), "MOUSE1");
    EXPECT_EQ(keyCodeToString(KeyCode::F5), "F5");
    
    // ASCII character
    EXPECT_EQ(keyCodeToString(static_cast<KeyCode>('a')), "A");
}

TEST(KeyNamesTest, RoundTrip) {
    // Key name -> code -> name should be consistent
    auto code = stringToKeyCode("MOUSE2");
    ASSERT_TRUE(code.has_value());
    EXPECT_EQ(keyCodeToString(*code), "MOUSE2");
    
    code = stringToKeyCode("F10");
    ASSERT_TRUE(code.has_value());
    EXPECT_EQ(keyCodeToString(*code), "F10");
}

TEST(KeyNamesTest, IsBindable) {
    EXPECT_TRUE(isBindableKey(KeyCode::Space));
    EXPECT_TRUE(isBindableKey(KeyCode::Mouse1));
    EXPECT_TRUE(isBindableKey(KeyCode::F1));
    EXPECT_FALSE(isBindableKey(KeyCode::Escape));
    EXPECT_FALSE(isBindableKey(static_cast<KeyCode>(-1)));
}

//=============================================================================
// InputState Tests
//=============================================================================

TEST(InputStateTest, DefaultState) {
    InputState state;
    
    EXPECT_FALSE(state.button(InputButton::Forward).isDown());
    EXPECT_FALSE(state.isKeyDown(KeyCode::Space));
    EXPECT_TRUE(state.binding(KeyCode::Space).empty());
    EXPECT_EQ(state.destination(), KeyDestination::Game);
    EXPECT_EQ(state.impulse(), 0);
}

TEST(InputStateTest, ButtonDownUp) {
    InputState state;
    
    state.buttonDown(InputButton::Forward, 42, 1.0);
    EXPECT_TRUE(state.button(InputButton::Forward).isDown());
    
    state.buttonUp(InputButton::Forward, 42, 2.0);
    EXPECT_FALSE(state.button(InputButton::Forward).isDown());
}

TEST(InputStateTest, KeyDownState) {
    InputState state;
    
    state.setKeyDown(KeyCode::Space, true);
    EXPECT_TRUE(state.isKeyDown(KeyCode::Space));
    EXPECT_EQ(state.lastKeyPress(), static_cast<int>(KeyCode::Space));
    
    state.setKeyDown(KeyCode::Space, false);
    EXPECT_FALSE(state.isKeyDown(KeyCode::Space));
}

TEST(InputStateTest, KeyDownByInt) {
    InputState state;
    
    state.setKeyDown(32, true);  // Space
    EXPECT_TRUE(state.isKeyDown(32));
    EXPECT_TRUE(state.isKeyDown(KeyCode::Space));
}

TEST(InputStateTest, KeyBindings) {
    InputState state;
    
    state.setBinding(KeyCode::Space, "+jump");
    EXPECT_EQ(state.binding(KeyCode::Space).command(), "+jump");
    EXPECT_FALSE(state.binding(KeyCode::Space).empty());
    
    state.unbind(KeyCode::Space);
    EXPECT_TRUE(state.binding(KeyCode::Space).empty());
}

TEST(InputStateTest, UnbindAll) {
    InputState state;
    
    state.setBinding(KeyCode::Space, "+jump");
    state.setBinding(KeyCode::Mouse1, "+attack");
    
    state.unbindAll();
    
    EXPECT_TRUE(state.binding(KeyCode::Space).empty());
    EXPECT_TRUE(state.binding(KeyCode::Mouse1).empty());
}

TEST(InputStateTest, Destination) {
    InputState state;
    
    EXPECT_TRUE(state.isGameMode());
    EXPECT_FALSE(state.isConsoleMode());
    
    state.setDestination(KeyDestination::Console);
    EXPECT_FALSE(state.isGameMode());
    EXPECT_TRUE(state.isConsoleMode());
    
    state.setDestination(KeyDestination::Menu);
    EXPECT_TRUE(state.isMenuMode());
}

TEST(InputStateTest, Impulse) {
    InputState state;
    
    state.setImpulse(7);
    EXPECT_EQ(state.impulse(), 7);
    
    state.clearImpulse();
    EXPECT_EQ(state.impulse(), 0);
}

TEST(InputStateTest, MouseState) {
    InputState state;
    
    state.mouseState().x = 100.0;
    state.mouseState().y = 200.0;
    
    EXPECT_DOUBLE_EQ(state.mouseState().x, 100.0);
    EXPECT_DOUBLE_EQ(state.mouseState().y, 200.0);
}

TEST(InputStateTest, IsMouseLookActive) {
    InputState state;
    
    // Default: freelook is true
    EXPECT_TRUE(state.isMouseLookActive());
    
    // Disable freelook
    state.mouseConfig().freeLook = false;
    EXPECT_FALSE(state.isMouseLookActive());
    
    // Enable via +mlook button
    state.buttonDown(InputButton::MLook, 42, 1.0);
    EXPECT_TRUE(state.isMouseLookActive());
}

TEST(InputStateTest, ClearButtonEdges) {
    InputState state;
    
    state.buttonDown(InputButton::Forward, 42, 1.0);
    state.buttonDown(InputButton::Attack, 43, 1.0);
    
    EXPECT_TRUE(state.button(InputButton::Forward).wasJustPressed());
    EXPECT_TRUE(state.button(InputButton::Attack).wasJustPressed());
    
    state.clearButtonEdges();
    
    EXPECT_FALSE(state.button(InputButton::Forward).wasJustPressed());
    EXPECT_FALSE(state.button(InputButton::Attack).wasJustPressed());
    EXPECT_TRUE(state.button(InputButton::Forward).isDown());  // Still down
}

TEST(InputStateTest, ClearAllButtons) {
    InputState state;
    
    state.buttonDown(InputButton::Forward, 42, 1.0);
    
    state.clearAllButtons();
    
    EXPECT_FALSE(state.button(InputButton::Forward).isDown());
}

TEST(InputStateTest, ClearKeyStates) {
    InputState state;
    
    state.setKeyDown(KeyCode::Space, true);
    state.incrementKeyRepeat(KeyCode::Space);
    
    state.clearKeyStates();
    
    EXPECT_FALSE(state.isKeyDown(KeyCode::Space));
    EXPECT_EQ(state.keyRepeat(KeyCode::Space), 0);
    EXPECT_EQ(state.lastKeyPress(), 0);
}

TEST(InputStateTest, FullClear) {
    InputState state;
    
    state.buttonDown(InputButton::Forward, 42, 1.0);
    state.setKeyDown(KeyCode::Space, true);
    state.setBinding(KeyCode::Mouse1, "+attack");
    state.setDestination(KeyDestination::Console);
    state.setImpulse(5);
    
    state.clear();
    
    EXPECT_FALSE(state.button(InputButton::Forward).isDown());
    EXPECT_FALSE(state.isKeyDown(KeyCode::Space));
    EXPECT_TRUE(state.binding(KeyCode::Mouse1).empty());
    EXPECT_EQ(state.destination(), KeyDestination::Game);
    EXPECT_EQ(state.impulse(), 0);
}

//=============================================================================
// MovementSpeeds Tests
//=============================================================================

TEST(MovementSpeedsTest, Defaults) {
    MovementSpeeds speeds;
    
    EXPECT_FLOAT_EQ(speeds.forwardSpeed, 400.0f);
    EXPECT_FLOAT_EQ(speeds.backSpeed, 400.0f);
    EXPECT_FLOAT_EQ(speeds.sideSpeed, 400.0f);
    EXPECT_FLOAT_EQ(speeds.upSpeed, 400.0f);
    EXPECT_FLOAT_EQ(speeds.speedKey, 2.0f);
}

//=============================================================================
// MouseConfig Tests
//=============================================================================

TEST(MouseConfigTest, Defaults) {
    MouseConfig config;
    
    EXPECT_FLOAT_EQ(config.sensitivity, 12.0f);
    EXPECT_FLOAT_EQ(config.pitchScale, 0.022f);
    EXPECT_FLOAT_EQ(config.yawScale, 0.022f);
    EXPECT_TRUE(config.freeLook);
    EXPECT_FALSE(config.lookSpring);
}
