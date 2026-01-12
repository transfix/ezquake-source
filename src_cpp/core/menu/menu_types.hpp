/**
 * @file menu_types.hpp
 * @brief Menu system, key handling, and settings page types
 * 
 * Converted from:
 * - menu.h: Menu states and drawing
 * - keys.h: Key codes and input state
 * - settings.h: Settings page structures
 * - settings_page.h: Settings page API
 */

#pragma once

#include <cstdint>
#include <array>
#include <string_view>
#include <functional>

namespace ezquake::menu {

// =============================================================================
// Menu Constants
// =============================================================================

namespace menu_limits {
    constexpr int32_t OPT_PADDING = 4;
    constexpr int32_t CMDLINES = 256;         // 1 << 8
    constexpr int32_t MAXCMDLINE = 256;
    constexpr int32_t MAX_MOUSE_BUTTONS = 9;
    constexpr int32_t MAX_KEY_BINDINGS = 512; // UNKNOWN + 256
}

// =============================================================================
// Key Codes
// =============================================================================

/**
 * @brief Keyboard and input key codes
 */
enum class KeyCode : int32_t {
    // ASCII keys
    Tab = 9,
    Enter = 13,
    Escape = 27,
    Space = 32,
    Backspace = 127,
    
    // Lock keys
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
    LAlt = 137,
    RAlt = 138,
    Ctrl = 139,
    LCtrl = 140,
    RCtrl = 141,
    Shift = 142,
    LShift = 143,
    RShift = 144,
    
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
    LWin = 167,
    RWin = 168,
    Menu = 169,
    
    // Keypad
    KpNumLock = 170,
    KpSlash = 171,
    KpStar = 172,
    KpHome = 173,
    KpUpArrow = 174,
    KpPageUp = 175,
    KpMinus = 176,
    KpLeftArrow = 177,
    Kp5 = 178,
    KpRightArrow = 179,
    KpPlus = 180,
    KpEnd = 181,
    KpDownArrow = 182,
    KpPageDown = 183,
    KpInsert = 184,
    KpDelete = 185,
    KpEnter = 186,
    
    // Mouse buttons
    Mouse1 = 200,
    Mouse2 = 201,
    Mouse3 = 202,
    Mouse4 = 203,
    Mouse5 = 204,
    Mouse6 = 205,
    Mouse7 = 206,
    Mouse8 = 207,
    
    // Mouse wheel
    MWheelUp = 244,
    MWheelDown = 245,
    
    // Unknown
    Unknown = 256
};

[[nodiscard]] constexpr bool isMouseButton(KeyCode key) noexcept {
    return key >= KeyCode::Mouse1 && key <= KeyCode::Mouse8;
}

[[nodiscard]] constexpr bool isArrowKey(KeyCode key) noexcept {
    return key >= KeyCode::UpArrow && key <= KeyCode::RightArrow;
}

[[nodiscard]] constexpr bool isFunctionKey(KeyCode key) noexcept {
    return key >= KeyCode::F1 && key <= KeyCode::F12;
}

[[nodiscard]] constexpr bool isModifierKey(KeyCode key) noexcept {
    return (key >= KeyCode::Alt && key <= KeyCode::RShift);
}

// =============================================================================
// Key Destination
// =============================================================================

/**
 * @brief Where key input is routed
 */
enum class KeyDest : uint8_t {
    Game = 0,
    Console = 1,
    Message = 2,
    Menu = 3,
    HudEditor = 4,
    DemoControls = 5,
    StartupDemoMenu = 6,
    StartupDemoConsole = 7,
    StartupDemoGame = 8,
    StartupDemoBrowser = 9
};

[[nodiscard]] constexpr bool isStartupDemo(KeyDest dest) noexcept {
    return dest >= KeyDest::StartupDemoMenu && dest <= KeyDest::StartupDemoBrowser;
}

[[nodiscard]] constexpr const char* getKeyDestName(KeyDest dest) noexcept {
    switch (dest) {
        case KeyDest::Game: return "game";
        case KeyDest::Console: return "console";
        case KeyDest::Message: return "message";
        case KeyDest::Menu: return "menu";
        case KeyDest::HudEditor: return "hudeditor";
        case KeyDest::DemoControls: return "democontrols";
        default: return "unknown";
    }
}

// =============================================================================
// Menu State
// =============================================================================

/**
 * @brief Current menu state/page
 */
enum class MenuState : uint8_t {
    None = 0,
    Main = 1,
    Proxy = 2,
    SinglePlayer = 3,
    Load = 4,
    Save = 5,
    MultiPlayer = 6,
    Demos = 7,
    MultiPlayerSubmenu = 8,
    Options = 9,
    Help = 10,
    Quit = 11,
    InGame = 12
};

[[nodiscard]] constexpr const char* getMenuStateName(MenuState state) noexcept {
    switch (state) {
        case MenuState::None: return "None";
        case MenuState::Main: return "Main";
        case MenuState::Proxy: return "Proxy";
        case MenuState::SinglePlayer: return "SinglePlayer";
        case MenuState::Load: return "Load";
        case MenuState::Save: return "Save";
        case MenuState::MultiPlayer: return "MultiPlayer";
        case MenuState::Demos: return "Demos";
        case MenuState::MultiPlayerSubmenu: return "MultiPlayerSubmenu";
        case MenuState::Options: return "Options";
        case MenuState::Help: return "Help";
        case MenuState::Quit: return "Quit";
        case MenuState::InGame: return "InGame";
        default: return "Unknown";
    }
}

[[nodiscard]] constexpr bool isInMenu(MenuState state) noexcept {
    return state != MenuState::None;
}

// =============================================================================
// Chat Type
// =============================================================================

/**
 * @brief Chat message type
 */
enum class ChatType : uint8_t {
    MessageMode1 = 0,  // say
    MessageMode2 = 1,  // say_team
    IRC = 2,           // IRC chat (optional)
    QtvToGame = 3      // QTV to game
};

// =============================================================================
// Mouse State
// =============================================================================

/**
 * @brief Mouse pointer state for menu interaction
 */
struct MouseState {
    double x{};          // Current X position
    double y{};          // Current Y position
    double xOld{};       // Previous X position
    double yOld{};       // Previous Y position
    std::array<bool, 9> buttons{};  // Button states (1-8, 0 unused)
    int32_t buttonDown{};   // Button just pressed (0 = none)
    int32_t buttonUp{};     // Button just released (0 = none)
    
    [[nodiscard]] constexpr bool isButtonDown(int button) const noexcept {
        if (button < 1 || button > 8) return false;
        return buttons[static_cast<size_t>(button)];
    }
    
    [[nodiscard]] constexpr double deltaX() const noexcept {
        return x - xOld;
    }
    
    [[nodiscard]] constexpr double deltaY() const noexcept {
        return y - yOld;
    }
    
    [[nodiscard]] constexpr bool hasMoved() const noexcept {
        return deltaX() != 0.0 || deltaY() != 0.0;
    }
    
    [[nodiscard]] constexpr bool anyButtonDown() const noexcept {
        for (size_t i = 1; i <= 8; ++i) {
            if (buttons[i]) return true;
        }
        return false;
    }
};

// =============================================================================
// Setting Type
// =============================================================================

/**
 * @brief Type of setting control
 */
enum class SettingType : uint8_t {
    Separator = 0,     // Decorative separator
    Number = 1,        // Numeric slider (cvar)
    IntNumber = 2,     // Integer variable (non-cvar)
    Bool = 3,          // On/Off toggle
    Custom = 4,        // Custom read/write functions
    Named = 5,         // Named integers (0, 1, 2...)
    Enum = 6,          // Named enum values
    Action = 7,        // Function action
    String = 8,        // Editable string
    PlayerColor = 9,   // Player color picker
    Skin = 10,         // Player skin picker
    Bind = 11,         // Key binding
    AdvancedMark = 12, // Advanced section start
    BasicMark = 13,    // Basic section start
    Blank = 14         // Blank space
};

[[nodiscard]] constexpr const char* getSettingTypeName(SettingType type) noexcept {
    switch (type) {
        case SettingType::Separator: return "Separator";
        case SettingType::Number: return "Number";
        case SettingType::IntNumber: return "IntNumber";
        case SettingType::Bool: return "Bool";
        case SettingType::Custom: return "Custom";
        case SettingType::Named: return "Named";
        case SettingType::Enum: return "Enum";
        case SettingType::Action: return "Action";
        case SettingType::String: return "String";
        case SettingType::PlayerColor: return "PlayerColor";
        case SettingType::Skin: return "Skin";
        case SettingType::Bind: return "Bind";
        case SettingType::AdvancedMark: return "AdvancedMark";
        case SettingType::BasicMark: return "BasicMark";
        case SettingType::Blank: return "Blank";
        default: return "Unknown";
    }
}

[[nodiscard]] constexpr bool isDecorative(SettingType type) noexcept {
    return type == SettingType::Separator || 
           type == SettingType::Blank ||
           type == SettingType::AdvancedMark ||
           type == SettingType::BasicMark;
}

[[nodiscard]] constexpr bool isEditable(SettingType type) noexcept {
    return type == SettingType::Number ||
           type == SettingType::IntNumber ||
           type == SettingType::Bool ||
           type == SettingType::Named ||
           type == SettingType::Enum ||
           type == SettingType::String ||
           type == SettingType::PlayerColor ||
           type == SettingType::Skin ||
           type == SettingType::Bind;
}

// =============================================================================
// Settings Page Mode
// =============================================================================

/**
 * @brief Settings page interaction mode
 */
enum class SettingsPageMode : uint8_t {
    Normal = 0,       // Normal browsing
    Binding = 1,      // Waiting for key bind
    ViewHelp = 2,     // Viewing help text
    ChooseSkin = 3    // Choosing skin
};

// =============================================================================
// Setting Definition
// =============================================================================

/**
 * @brief Setting definition (simplified from C version)
 */
struct SettingDef {
    SettingType type{SettingType::Separator};
    std::string_view label;
    bool advanced{false};
    float min{};
    float max{};
    float step{};
    std::string_view varname;
    std::string_view description;
    int32_t top{};  // Rendering position
    
    [[nodiscard]] constexpr bool isAdvanced() const noexcept {
        return advanced;
    }
    
    [[nodiscard]] constexpr bool hasRange() const noexcept {
        return type == SettingType::Number || 
               type == SettingType::IntNumber ||
               type == SettingType::Named ||
               type == SettingType::Enum ||
               type == SettingType::PlayerColor;
    }
    
    [[nodiscard]] constexpr float clampValue(float value) const noexcept {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }
    
    [[nodiscard]] constexpr int32_t numSteps() const noexcept {
        if (step <= 0.0f) return 0;
        return static_cast<int32_t>((max - min) / step);
    }
};

// =============================================================================
// Settings Page State
// =============================================================================

/**
 * @brief Settings page state
 */
struct SettingsPageState {
    int32_t marked{};        // Currently selected item
    int32_t viewpoint{};     // Scroll offset
    int32_t width{};         // Last drawn width
    int32_t height{};        // Last drawn height
    int32_t count{};         // Number of settings
    SettingsPageMode mode{SettingsPageMode::Normal};
    bool mini{false};        // Minimalistic mode
    
    constexpr void reset() noexcept {
        marked = 0;
        viewpoint = 0;
        mode = SettingsPageMode::Normal;
    }
    
    [[nodiscard]] constexpr bool isBinding() const noexcept {
        return mode == SettingsPageMode::Binding;
    }
    
    [[nodiscard]] constexpr bool isViewingHelp() const noexcept {
        return mode == SettingsPageMode::ViewHelp;
    }
    
    constexpr void scrollTo(int32_t index) noexcept {
        if (index >= 0 && index < count) {
            marked = index;
        }
    }
};

// =============================================================================
// Key Binding
// =============================================================================

/**
 * @brief Key binding entry
 */
struct KeyBinding {
    KeyCode key{KeyCode::Unknown};
    std::string_view command;
    bool isActive{false};
    
    [[nodiscard]] constexpr bool isBound() const noexcept {
        return !command.empty();
    }
};

// =============================================================================
// Key State
// =============================================================================

/**
 * @brief Global key state tracking
 */
struct KeyState {
    std::array<bool, 512> down{};       // Key is currently down
    std::array<int32_t, 512> repeats{}; // Key repeat counter
    int32_t lastPress{};                // Last key pressed
    
    [[nodiscard]] constexpr bool isDown(KeyCode key) const noexcept {
        auto idx = static_cast<size_t>(key);
        return idx < down.size() && down[idx];
    }
    
    [[nodiscard]] constexpr int32_t repeatCount(KeyCode key) const noexcept {
        auto idx = static_cast<size_t>(key);
        return idx < repeats.size() ? repeats[idx] : 0;
    }
    
    constexpr void setDown(KeyCode key, bool isDown) noexcept {
        auto idx = static_cast<size_t>(key);
        if (idx < down.size()) {
            down[idx] = isDown;
            if (isDown) {
                lastPress = static_cast<int32_t>(key);
            }
        }
    }
    
    constexpr void clearAll() noexcept {
        down.fill(false);
        repeats.fill(0);
        lastPress = 0;
    }
};

// =============================================================================
// Menu Drawing Info
// =============================================================================

/**
 * @brief Menu drawing parameters
 */
struct MenuDrawInfo {
    int32_t x{};
    int32_t y{};
    int32_t width{};
    int32_t height{};
    int32_t yOffset{};  // m_yofs
    
    [[nodiscard]] constexpr int32_t centerX() const noexcept {
        return x + width / 2;
    }
    
    [[nodiscard]] constexpr int32_t centerY() const noexcept {
        return y + height / 2;
    }
    
    [[nodiscard]] constexpr bool containsPoint(int32_t px, int32_t py) const noexcept {
        return px >= x && px < x + width && py >= y && py < y + height;
    }
};

// =============================================================================
// Slider Control
// =============================================================================

/**
 * @brief Slider control state
 */
struct SliderControl {
    float value{};
    float min{};
    float max{};
    float step{};
    int32_t x{};
    int32_t y{};
    int32_t width{};
    
    [[nodiscard]] constexpr float normalizedValue() const noexcept {
        if (max <= min) return 0.0f;
        return (value - min) / (max - min);
    }
    
    constexpr void increment() noexcept {
        value += step;
        if (value > max) value = max;
    }
    
    constexpr void decrement() noexcept {
        value -= step;
        if (value < min) value = min;
    }
    
    [[nodiscard]] constexpr float valueFromPosition(int32_t px) const noexcept {
        if (width <= 0) return min;
        float ratio = static_cast<float>(px - x) / static_cast<float>(width);
        if (ratio < 0.0f) ratio = 0.0f;
        if (ratio > 1.0f) ratio = 1.0f;
        return min + ratio * (max - min);
    }
};

// =============================================================================
// Flashing Cursor
// =============================================================================

/**
 * @brief Animated cursor for menus
 */
struct FlashingCursor {
    float curtime{};
    
    [[nodiscard]] constexpr int32_t arrowFrame() const noexcept {
        return 12 + (static_cast<int32_t>(curtime * 4.0f) & 1);
    }
    
    [[nodiscard]] constexpr int32_t cursorFrame() const noexcept {
        return 10 + (static_cast<int32_t>(curtime * 4.0f) & 1);
    }
};

// =============================================================================
// Player Color
// =============================================================================

/**
 * @brief Player color selection
 */
struct PlayerColor {
    int32_t top{0};      // Top color (0-15 or -1 for unchanged)
    int32_t bottom{0};   // Bottom color (0-15 or -1 for unchanged)
    
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return top >= -1 && top <= 15 && bottom >= -1 && bottom <= 15;
    }
    
    [[nodiscard]] constexpr int32_t combined() const noexcept {
        return (top << 4) | (bottom & 0x0F);
    }
    
    static constexpr PlayerColor fromCombined(int32_t value) noexcept {
        return {(value >> 4) & 0x0F, value & 0x0F};
    }
};

} // namespace ezquake::menu
