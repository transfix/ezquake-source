/*
 * ezQuake C++ Port - Input System Types
 * 
 * Key codes, input enums, and related types.
 * 
 * Copyright (C) 2024 ezQuake Team
 */

#pragma once

#include <cstdint>
#include <string_view>
#include <optional>

namespace ezquake {
namespace input {

//=============================================================================
// Key Codes
//=============================================================================

/**
 * @brief Key codes for keyboard, mouse, and joystick input.
 * 
 * Values are compatible with the original C keynum_t enum.
 */
enum class KeyCode : std::int16_t {
    None = 0,
    
    // ASCII printable keys (passed as lowercase)
    Tab = 9,
    Enter = 13,
    Escape = 27,
    Space = 32,
    
    // Extended keys
    Backspace = 127,
    CapsLock = 128,
    PrintScreen = 129,
    ScrollLock = 130,
    Pause = 131,
    
    // Arrow keys
    UpArrow = 132,
    DownArrow = 133,
    LeftArrow = 134,
    RightArrow = 135,
    
    // Modifier keys
    Alt = 136,
    LeftAlt = 137,
    RightAlt = 138,
    Ctrl = 139,
    LeftCtrl = 140,
    RightCtrl = 141,
    Shift = 142,
    LeftShift = 143,
    RightShift = 144,
    
    // Function keys
    F1 = 148,
    F2 = 149,
    F3 = 150,
    F4 = 151,
    F5 = 152,
    F6 = 153,
    F7 = 154,
    F8 = 155,
    F9 = 156,
    F10 = 157,
    F11 = 158,
    F12 = 159,
    
    // Navigation keys
    Insert = 160,
    Delete = 161,
    PageDown = 162,
    PageUp = 163,
    Home = 164,
    End = 165,
    
    // Windows keys
    Win = 166,
    LeftWin = 167,
    RightWin = 168,
    Menu = 169,
    
    // Numpad
    NumLock = 170,
    NumpadSlash = 171,
    NumpadStar = 172,
    NumpadHome = 173,
    NumpadUp = 174,
    NumpadPgUp = 175,
    NumpadMinus = 176,
    NumpadLeft = 177,
    Numpad5 = 178,
    NumpadRight = 179,
    NumpadPlus = 180,
    NumpadEnd = 181,
    NumpadDown = 182,
    NumpadPgDn = 183,
    NumpadIns = 184,
    NumpadDel = 185,
    NumpadEnter = 186,
    
    // International keys
    Section = 187,
    Acute = 188,
    Diaeresis = 189,
    Aring = 190,
    Adiaeresis = 191,
    Odiaeresis = 192,
    WakeUp = 193,
    
    // Mouse buttons
    Mouse1 = 200,
    Mouse2 = 201,
    Mouse3 = 202,
    Mouse4 = 203,
    Mouse5 = 204,
    Mouse6 = 205,
    Mouse7 = 206,
    Mouse8 = 207,
    
    // Joystick buttons
    Joy1 = 208,
    Joy2 = 209,
    Joy3 = 210,
    Joy4 = 211,
    
    // Joystick POV/Hat
    JoyPovUp = 212,
    JoyPovRight = 213,
    JoyPovDown = 214,
    JoyPovLeft = 215,
    
    // Aux keys (for multi-button joysticks)
    Aux1 = 216,
    Aux2 = 217,
    Aux3 = 218,
    Aux4 = 219,
    Aux5 = 220,
    Aux6 = 221,
    Aux7 = 222,
    Aux8 = 223,
    Aux9 = 224,
    Aux10 = 225,
    Aux11 = 226,
    Aux12 = 227,
    Aux13 = 228,
    Aux14 = 229,
    Aux15 = 230,
    Aux16 = 231,
    Aux17 = 232,
    Aux18 = 233,
    Aux19 = 234,
    Aux20 = 235,
    Aux21 = 236,
    Aux22 = 237,
    Aux23 = 238,
    Aux24 = 239,
    Aux25 = 240,
    Aux26 = 241,
    Aux27 = 242,
    Aux28 = 243,
    
    // Mouse wheel
    MouseWheelUp = 244,
    MouseWheelDown = 245,
    
    // Unknown/maximum
    Unknown = 256,
    
    // Total number of possible keys
    MaxKeys = 512
};

/**
 * @brief Check if a key code is a mouse button.
 */
[[nodiscard]] constexpr bool isMouseButton(KeyCode key) noexcept {
    auto k = static_cast<int>(key);
    return k >= static_cast<int>(KeyCode::Mouse1) && 
           k <= static_cast<int>(KeyCode::Mouse8);
}

/**
 * @brief Check if a key code is a mouse wheel action.
 */
[[nodiscard]] constexpr bool isMouseWheel(KeyCode key) noexcept {
    return key == KeyCode::MouseWheelUp || key == KeyCode::MouseWheelDown;
}

/**
 * @brief Check if a key code is a joystick button.
 */
[[nodiscard]] constexpr bool isJoystickButton(KeyCode key) noexcept {
    auto k = static_cast<int>(key);
    return (k >= static_cast<int>(KeyCode::Joy1) && 
            k <= static_cast<int>(KeyCode::Aux28));
}

/**
 * @brief Check if a key code is a function key.
 */
[[nodiscard]] constexpr bool isFunctionKey(KeyCode key) noexcept {
    auto k = static_cast<int>(key);
    return k >= static_cast<int>(KeyCode::F1) && 
           k <= static_cast<int>(KeyCode::F12);
}

/**
 * @brief Check if a key code is a modifier key.
 */
[[nodiscard]] constexpr bool isModifierKey(KeyCode key) noexcept {
    auto k = static_cast<int>(key);
    return (k >= static_cast<int>(KeyCode::Alt) && 
            k <= static_cast<int>(KeyCode::RightShift));
}

/**
 * @brief Check if a key code is a numpad key.
 */
[[nodiscard]] constexpr bool isNumpadKey(KeyCode key) noexcept {
    auto k = static_cast<int>(key);
    return k >= static_cast<int>(KeyCode::NumLock) && 
           k <= static_cast<int>(KeyCode::NumpadEnter);
}

//=============================================================================
// Input Destination
//=============================================================================

/**
 * @brief Where keyboard input is currently being directed.
 */
enum class KeyDestination : std::uint8_t {
    Game,               // Normal gameplay
    Console,            // Console open
    Message,            // Chat message input
    Menu,               // Menu open
    HudEditor,          // HUD editor mode
    DemoControls,       // Demo control overlay
    StartupDemoMenu,    // Startup demo menu
    StartupDemoConsole, // Startup demo console
    StartupDemoGame,    // Startup demo game
    StartupDemoBrowser  // Startup demo browser
};

/**
 * @brief Check if destination is a startup demo mode.
 */
[[nodiscard]] constexpr bool isStartupDemoMode(KeyDestination dest) noexcept {
    return dest >= KeyDestination::StartupDemoMenu &&
           dest <= KeyDestination::StartupDemoBrowser;
}

//=============================================================================
// Chat Types
//=============================================================================

/**
 * @brief Type of chat message being composed.
 */
enum class ChatType : std::uint8_t {
    Say,        // say (mm1)
    SayTeam,    // say_team (mm2)
    Irc,        // IRC chat (if enabled)
    QtvToGame   // QTV to game chat
};

//=============================================================================
// Mouse State
//=============================================================================

/**
 * @brief Current state of the mouse pointer.
 */
struct MouseState {
    double x = 0.0;         // Current X position
    double y = 0.0;         // Current Y position
    double prevX = 0.0;     // Previous X position
    double prevY = 0.0;     // Previous Y position
    bool buttons[9] = {};   // Button states (1-8, 0 unused)
    int buttonDown = 0;     // Button just pressed (0 if none)
    int buttonUp = 0;       // Button just released (0 if none)
    
    [[nodiscard]] double deltaX() const noexcept { return x - prevX; }
    [[nodiscard]] double deltaY() const noexcept { return y - prevY; }
    
    [[nodiscard]] bool isButtonDown(int button) const noexcept {
        if (button >= 1 && button <= 8) {
            return buttons[button];
        }
        return false;
    }
    
    void clear() noexcept {
        x = y = prevX = prevY = 0.0;
        for (auto& b : buttons) b = false;
        buttonDown = buttonUp = 0;
    }
};

//=============================================================================
// Key Button State
//=============================================================================

/**
 * @brief Special key values for button tracking.
 */
inline constexpr int KEY_VOID = -1;   // No key (typed manually)
inline constexpr int KEY_NULL = -2;   // Null key marker

/**
 * @brief Tracks the state of a button that can be held by multiple keys.
 * 
 * Continuous button event tracking is complicated by the fact that two 
 * different input sources (say, mouse button 1 and the control key) can 
 * both press the same button, but the button should only be released 
 * when both pressing keys have been released.
 * 
 * State bits:
 * - bit 0: current state (1 = down)
 * - bit 1: edge triggered on up->down transition
 * - bit 2: edge triggered on down->up transition
 */
class KeyButton {
public:
    // State bit masks
    static constexpr int STATE_DOWN = 1;
    static constexpr int STATE_IMPULSE_DOWN = 2;
    static constexpr int STATE_IMPULSE_UP = 4;
    
    KeyButton() noexcept = default;
    
    /**
     * @brief Process a key down event.
     * @param keyNum The key number being pressed, or KEY_NULL for scripted.
     * @param currentTime Current time for downtime tracking.
     * @return true if state changed, false if key was already tracked.
     */
    bool keyDown(int keyNum, double currentTime) noexcept {
        if (keyNum != KEY_NULL) {
            // Check for repeating key
            if (keyNum == down_[0] || keyNum == down_[1]) {
                return false;
            }
            
            // Find empty slot
            if (down_[0] == 0) {
                down_[0] = keyNum;
            } else if (down_[1] == 0) {
                down_[1] = keyNum;
            } else {
                // Three keys down - ignore
                return false;
            }
        }
        
        if (state_ & STATE_DOWN) {
            return false; // Already down
        }
        
        state_ |= STATE_DOWN | STATE_IMPULSE_DOWN;
        downTime_ = currentTime;
        return true;
    }
    
    /**
     * @brief Process a key up event.
     * @param keyNum The key number being released, or KEY_VOID for manual.
     * @param currentTime Current time for uptime tracking.
     * @return true if the button is now fully up.
     */
    bool keyUp(int keyNum, double currentTime) noexcept {
        if (keyNum == KEY_VOID || keyNum == KEY_NULL) {
            // Manual/scripted release - clear all
            down_[0] = down_[1] = 0;
            state_ &= ~STATE_DOWN;
            state_ |= STATE_IMPULSE_UP;
            upTime_ = currentTime;
            return true;
        }
        
        // Find and clear the key
        if (down_[0] == keyNum) {
            down_[0] = 0;
        } else if (down_[1] == keyNum) {
            down_[1] = 0;
        } else {
            return true; // Key up without corresponding down
        }
        
        // Still held by another key?
        if (down_[0] != 0 || down_[1] != 0) {
            return false;
        }
        
        if (!(state_ & STATE_DOWN)) {
            return true; // Already up (shouldn't happen)
        }
        
        state_ &= ~STATE_DOWN;
        state_ |= STATE_IMPULSE_UP;
        upTime_ = currentTime;
        return true;
    }
    
    /**
     * @brief Check if the button is currently held down.
     */
    [[nodiscard]] bool isDown() const noexcept {
        return (state_ & STATE_DOWN) != 0;
    }
    
    /**
     * @brief Check if there was a down edge this frame.
     */
    [[nodiscard]] bool wasJustPressed() const noexcept {
        return (state_ & STATE_IMPULSE_DOWN) != 0;
    }
    
    /**
     * @brief Check if there was an up edge this frame.
     */
    [[nodiscard]] bool wasJustReleased() const noexcept {
        return (state_ & STATE_IMPULSE_UP) != 0;
    }
    
    /**
     * @brief Clear edge-triggered states (call after processing).
     */
    void clearEdges() noexcept {
        state_ &= ~(STATE_IMPULSE_DOWN | STATE_IMPULSE_UP);
    }
    
    /**
     * @brief Get the time when the button was last pressed down.
     */
    [[nodiscard]] double downTime() const noexcept { return downTime_; }
    
    /**
     * @brief Get the time when the button was last released.
     */
    [[nodiscard]] double upTime() const noexcept { return upTime_; }
    
    /**
     * @brief Get raw state bits.
     */
    [[nodiscard]] int state() const noexcept { return state_; }
    
    /**
     * @brief Get holding key in slot 0.
     */
    [[nodiscard]] int holdingKey0() const noexcept { return down_[0]; }
    
    /**
     * @brief Get holding key in slot 1.
     */
    [[nodiscard]] int holdingKey1() const noexcept { return down_[1]; }
    
    /**
     * @brief Clear all state.
     */
    void clear() noexcept {
        down_[0] = down_[1] = 0;
        state_ = 0;
        downTime_ = upTime_ = 0.0;
    }
    
    /**
     * @brief Calculate key state for movement commands.
     * 
     * Returns a value between 0.0 and 1.0 representing how "down" the
     * button was over the last frame. Used for smooth movement.
     * 
     * @param frameTime Duration of the current frame.
     * @param currentTime Current time.
     * @param isLookButton If true, uses simpler state calculation.
     */
    [[nodiscard]] float calculateKeyState(double frameTime, double currentTime, 
                                          bool isLookButton = false) const noexcept {
        if (isLookButton) {
            // Simple on/off for look buttons
            return isDown() ? 1.0f : 0.0f;
        }
        
        bool down = isDown();
        bool impulseDown = wasJustPressed();
        bool impulseUp = wasJustReleased();
        
        float val = 0.0f;
        
        if (impulseDown && impulseUp) {
            // Pressed and released this frame
            if (down) {
                val = 0.75f; // Held, released, pressed again
            } else {
                val = 0.25f; // Pressed, released
            }
        } else if (impulseDown) {
            // Just pressed
            if (down) {
                val = 0.5f;
            } else {
                val = 0.0f;
            }
        } else if (impulseUp) {
            // Just released
            if (down) {
                val = 1.0f;
            } else {
                val = 0.0f;
            }
        } else {
            // No change this frame
            val = down ? 1.0f : 0.0f;
        }
        
        return val;
    }
    
private:
    int down_[2] = {0, 0};   // Key numbers holding it down
    int state_ = 0;          // State bits
    double downTime_ = 0.0;  // Last keydown time
    double upTime_ = 0.0;    // Last keyup time
};

//=============================================================================
// Input Buttons (Named buttons for movement commands)
//=============================================================================

/**
 * @brief Named movement/action buttons.
 */
enum class InputButton : std::uint8_t {
    Forward,
    Back,
    MoveLeft,
    MoveRight,
    Left,       // Turn left
    Right,      // Turn right
    LookUp,
    LookDown,
    MoveUp,
    MoveDown,
    Speed,      // +speed (run)
    Strafe,     // +strafe
    Attack,
    Attack2,
    Jump,
    Use,
    MLook,      // Mouse look toggle
    KLook,      // Keyboard look toggle
    
    Count       // Number of buttons
};

/**
 * @brief Convert button enum to string name.
 */
[[nodiscard]] constexpr std::string_view buttonName(InputButton button) noexcept {
    switch (button) {
        case InputButton::Forward:   return "forward";
        case InputButton::Back:      return "back";
        case InputButton::MoveLeft:  return "moveleft";
        case InputButton::MoveRight: return "moveright";
        case InputButton::Left:      return "left";
        case InputButton::Right:     return "right";
        case InputButton::LookUp:    return "lookup";
        case InputButton::LookDown:  return "lookdown";
        case InputButton::MoveUp:    return "moveup";
        case InputButton::MoveDown:  return "movedown";
        case InputButton::Speed:     return "speed";
        case InputButton::Strafe:    return "strafe";
        case InputButton::Attack:    return "attack";
        case InputButton::Attack2:   return "attack2";
        case InputButton::Jump:      return "jump";
        case InputButton::Use:       return "use";
        case InputButton::MLook:     return "mlook";
        case InputButton::KLook:     return "klook";
        default:                     return "unknown";
    }
}

/**
 * @brief Parse button name to enum.
 */
[[nodiscard]] inline std::optional<InputButton> parseButtonName(std::string_view name) noexcept {
    if (name == "forward")   return InputButton::Forward;
    if (name == "back")      return InputButton::Back;
    if (name == "moveleft")  return InputButton::MoveLeft;
    if (name == "moveright") return InputButton::MoveRight;
    if (name == "left")      return InputButton::Left;
    if (name == "right")     return InputButton::Right;
    if (name == "lookup")    return InputButton::LookUp;
    if (name == "lookdown")  return InputButton::LookDown;
    if (name == "moveup")    return InputButton::MoveUp;
    if (name == "movedown")  return InputButton::MoveDown;
    if (name == "speed")     return InputButton::Speed;
    if (name == "strafe")    return InputButton::Strafe;
    if (name == "attack")    return InputButton::Attack;
    if (name == "attack2")   return InputButton::Attack2;
    if (name == "jump")      return InputButton::Jump;
    if (name == "use")       return InputButton::Use;
    if (name == "mlook")     return InputButton::MLook;
    if (name == "klook")     return InputButton::KLook;
    return std::nullopt;
}

} // namespace input
} // namespace ezquake
