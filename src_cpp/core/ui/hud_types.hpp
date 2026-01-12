/**
 * @file hud_types.hpp
 * @brief HUD, Menu, and UI type definitions
 *
 * Converted from:
 * - hud.h - HUD element definitions
 * - hud_common.h - Common HUD types
 * - menu.h - Menu state types
 * - Ctrl.h - UI control types
 */

#pragma once

#include <array>
#include <cstdint>
#include <string_view>
#include <functional>
#include <optional>

namespace ezquake::ui {

// =============================================================================
// HUD Constants
// =============================================================================

namespace hud_limits {
    /// Maximum HUD elements
    inline constexpr std::size_t MAX_ELEMENTS = 256;
    
    /// Maximum parameters per HUD element
    inline constexpr std::size_t MAX_PARAMS = 32;
    
    /// Regex offset count for HUD
    inline constexpr std::size_t REGEXP_OFFSET_COUNT = 20;
    
    /// Letter dimensions for text
    inline constexpr int LETTER_WIDTH = 8;
    inline constexpr int LETTER_HEIGHT = 8;
}

// =============================================================================
// HUD Flags
// =============================================================================

namespace HudFlags {
    inline constexpr uint32_t NoDraw          = 1u << 0;   // Don't draw normally
    inline constexpr uint32_t NoShow          = 1u << 1;   // Doesn't support show/hide
    inline constexpr uint32_t NoPosX          = 1u << 2;   // No X positioning
    inline constexpr uint32_t NoPosY          = 1u << 3;   // No Y positioning
    inline constexpr uint32_t NoFrame         = 1u << 4;   // Don't add frame
    inline constexpr uint32_t OnDialog        = 1u << 5;   // Draw on dialog
    inline constexpr uint32_t OnIntermission  = 1u << 6;   // Draw on intermission
    inline constexpr uint32_t OnFinale        = 1u << 7;   // Draw on finale
    inline constexpr uint32_t OnScores        = 1u << 8;   // Draw on +showscores
    inline constexpr uint32_t NoGrow          = 1u << 9;   // No frame grow
    inline constexpr uint32_t PlusMinus       = 1u << 10;  // Auto add +/- commands
    inline constexpr uint32_t Opacity         = 1u << 11;  // Supports opacity
    
    /// Inventory items (sbar elements)
    inline constexpr uint32_t Inventory = NoGrow;
    
    /// Check if flag is set
    [[nodiscard]] constexpr bool hasFlag(uint32_t flags, uint32_t flag) noexcept {
        return (flags & flag) != 0;
    }
    
    /// Get display name for flags
    [[nodiscard]] constexpr const char* getFlagName(uint32_t flag) noexcept {
        switch (flag) {
            case NoDraw:         return "NoDraw";
            case NoShow:         return "NoShow";
            case NoPosX:         return "NoPosX";
            case NoPosY:         return "NoPosY";
            case NoFrame:        return "NoFrame";
            case OnDialog:       return "OnDialog";
            case OnIntermission: return "OnIntermission";
            case OnFinale:       return "OnFinale";
            case OnScores:       return "OnScores";
            case NoGrow:         return "NoGrow";
            case PlusMinus:      return "PlusMinus";
            case Opacity:        return "Opacity";
            default:             return "Unknown";
        }
    }
}

// =============================================================================
// HUD Placement
// =============================================================================

/**
 * @brief HUD element placement mode
 */
enum class HudPlacement : uint8_t {
    Screen = 1,     // Relative to screen
    Top,            // Relative to top
    View,           // Relative to view
    Sbar,           // Relative to status bar
    Ibar,           // Relative to inventory bar
    Hbar,           // Relative to health bar
    SFree,          // Free status bar placement
    IFree,          // Free inventory placement
    HFree           // Free health placement
};

/**
 * @brief Get display name for placement
 */
[[nodiscard]] constexpr const char* getPlacementName(HudPlacement p) noexcept {
    switch (p) {
        case HudPlacement::Screen: return "Screen";
        case HudPlacement::Top:    return "Top";
        case HudPlacement::View:   return "View";
        case HudPlacement::Sbar:   return "Sbar";
        case HudPlacement::Ibar:   return "Ibar";
        case HudPlacement::Hbar:   return "Hbar";
        case HudPlacement::SFree:  return "SFree";
        case HudPlacement::IFree:  return "IFree";
        case HudPlacement::HFree:  return "HFree";
        default:                   return "Unknown";
    }
}

// =============================================================================
// HUD Alignment
// =============================================================================

/**
 * @brief HUD element alignment
 */
enum class HudAlign : uint8_t {
    Left = 1,       // Left/Top alignment
    Top = 1,        // Top (same as left for Y)
    Center = 2,     // Center alignment
    Right = 3,      // Right/Bottom alignment
    Bottom = 3,     // Bottom (same as right for Y)
    Before = 4,     // Before parent element
    After = 5,      // After parent element
    Console = 6     // Console-relative
};

/**
 * @brief Get display name for alignment
 */
[[nodiscard]] constexpr const char* getAlignmentName(HudAlign a) noexcept {
    switch (a) {
        case HudAlign::Left:    return "Left";
        case HudAlign::Center:  return "Center";
        case HudAlign::Right:   return "Right";
        case HudAlign::Before:  return "Before";
        case HudAlign::After:   return "After";
        case HudAlign::Console: return "Console";
        default:                return "Unknown";
    }
}

// =============================================================================
// HUD Scoreboard Flags
// =============================================================================

namespace ScoreboardFlags {
    inline constexpr uint32_t All         = 0xFFFFFFFF;
    inline constexpr uint32_t SortTeams   = 1u << 0;
    inline constexpr uint32_t SortPlayers = 1u << 1;
    inline constexpr uint32_t Update      = 1u << 2;
    inline constexpr uint32_t AvgPing     = 1u << 3;
}

// =============================================================================
// HUD Show Conditions
// =============================================================================

namespace HudShowCondition {
    inline constexpr int OnlyInTeamplay      = 1;
    inline constexpr int OnlyInDemoPlayback  = 2;
}

// =============================================================================
// HUD Rect (Position and Size)
// =============================================================================

/**
 * @brief Rectangle for HUD element positioning
 */
struct HudRect {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    
    [[nodiscard]] constexpr int right() const noexcept { return x + width; }
    [[nodiscard]] constexpr int bottom() const noexcept { return y + height; }
    [[nodiscard]] constexpr bool isEmpty() const noexcept { 
        return width <= 0 || height <= 0; 
    }
    [[nodiscard]] constexpr bool contains(int px, int py) const noexcept {
        return px >= x && px < right() && py >= y && py < bottom();
    }
};

/**
 * @brief Frame/border offsets for HUD elements
 */
struct HudFrame {
    int left = 0;
    int right = 0;
    int top = 0;
    int bottom = 0;
    
    [[nodiscard]] constexpr int horizontalPadding() const noexcept {
        return left + right;
    }
    [[nodiscard]] constexpr int verticalPadding() const noexcept {
        return top + bottom;
    }
};

// =============================================================================
// HUD Color
// =============================================================================

/**
 * @brief RGBA color for HUD elements
 */
struct HudColor {
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    uint8_t a = 255;
    
    constexpr HudColor() noexcept = default;
    constexpr HudColor(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_ = 255) noexcept
        : r(r_), g(g_), b(b_), a(a_) {}
    
    /// Standard colors
    [[nodiscard]] static constexpr HudColor white() noexcept { return {255, 255, 255, 255}; }
    [[nodiscard]] static constexpr HudColor black() noexcept { return {0, 0, 0, 255}; }
    [[nodiscard]] static constexpr HudColor red() noexcept { return {255, 0, 0, 255}; }
    [[nodiscard]] static constexpr HudColor green() noexcept { return {0, 255, 0, 255}; }
    [[nodiscard]] static constexpr HudColor blue() noexcept { return {0, 0, 255, 255}; }
    [[nodiscard]] static constexpr HudColor yellow() noexcept { return {255, 255, 0, 255}; }
    [[nodiscard]] static constexpr HudColor transparent() noexcept { return {0, 0, 0, 0}; }
    
    /// Apply opacity (0.0 - 1.0)
    [[nodiscard]] constexpr HudColor withOpacity(float opacity) const noexcept {
        return HudColor(r, g, b, static_cast<uint8_t>(a * opacity));
    }
    
    constexpr bool operator==(const HudColor& other) const noexcept = default;
};

// =============================================================================
// Menu State
// =============================================================================

/**
 * @brief Menu state enumeration
 */
enum class MenuState : uint8_t {
    None = 0,           // No menu active
    Main,               // Main menu
    Proxy,              // Proxy menu
    SinglePlayer,       // Single player menu
    Load,               // Load game menu
    Save,               // Save game menu
    Multiplayer,        // Multiplayer menu
    Demos,              // Demo menu
    MultiplayerSubmenu, // Multiplayer submenu
    Options,            // Options menu
    Help,               // Help menu
    Quit,               // Quit dialog
    InGame              // In-game menu
};

/**
 * @brief Get display name for menu state
 */
[[nodiscard]] constexpr const char* getMenuStateName(MenuState state) noexcept {
    switch (state) {
        case MenuState::None:               return "None";
        case MenuState::Main:               return "Main";
        case MenuState::Proxy:              return "Proxy";
        case MenuState::SinglePlayer:       return "SinglePlayer";
        case MenuState::Load:               return "Load";
        case MenuState::Save:               return "Save";
        case MenuState::Multiplayer:        return "Multiplayer";
        case MenuState::Demos:              return "Demos";
        case MenuState::MultiplayerSubmenu: return "MultiplayerSubmenu";
        case MenuState::Options:            return "Options";
        case MenuState::Help:               return "Help";
        case MenuState::Quit:               return "Quit";
        case MenuState::InGame:             return "InGame";
        default:                            return "Unknown";
    }
}

/**
 * @brief Check if a menu is active
 */
[[nodiscard]] constexpr bool isMenuActive(MenuState state) noexcept {
    return state != MenuState::None;
}

// =============================================================================
// UI Control Types
// =============================================================================

/**
 * @brief Slider range specification
 */
struct SliderRange {
    float min = 0.0f;
    float max = 1.0f;
    float step = 0.1f;
    float current = 0.0f;
    
    /// Get normalized position (0-1)
    [[nodiscard]] constexpr float normalized() const noexcept {
        if (max <= min) return 0.0f;
        return (current - min) / (max - min);
    }
    
    /// Set from normalized position (0-1)
    constexpr void setNormalized(float norm) noexcept {
        current = min + norm * (max - min);
        clamp();
    }
    
    /// Clamp current to range
    constexpr void clamp() noexcept {
        if (current < min) current = min;
        if (current > max) current = max;
    }
    
    /// Increment by step
    constexpr void increment() noexcept {
        current += step;
        clamp();
    }
    
    /// Decrement by step
    constexpr void decrement() noexcept {
        current -= step;
        clamp();
    }
};

/**
 * @brief Scrollbar state
 */
struct ScrollbarState {
    double position = 0.0;      // Current position (0-100%)
    int width = 8;              // Scrollbar width in pixels
    int height = 100;           // Content height in pixels
    bool mouseLocked = false;   // Mouse grabbed by scrollbar
    
    /// Get normalized position (0-1)
    [[nodiscard]] constexpr double normalized() const noexcept {
        return position / 100.0;
    }
    
    /// Set from normalized position
    constexpr void setNormalized(double norm) noexcept {
        position = norm * 100.0;
        clamp();
    }
    
    /// Clamp position to valid range
    constexpr void clamp() noexcept {
        if (position < 0.0) position = 0.0;
        if (position > 100.0) position = 100.0;
    }
};

// =============================================================================
// Team/Player Sorting (for scoreboards)
// =============================================================================

/**
 * @brief Sorted team information for scoreboards
 */
struct SortedTeamInfo {
    std::string_view name;
    int frags = 0;
    int minPing = 0;
    int avgPing = 0;
    int maxPing = 0;
    int playerCount = 0;
    int topColor = 0;           // Team top color
    int bottomColor = 0;        // Team bottom color
    
    // Weapon counts
    int rlCount = 0;            // Number of rocket launchers
    int lgCount = 0;            // Number of lightning guns
    int weapCount = 0;          // Players with weapons
    
    // Item pickups
    int raCount = 0;            // Red armor taken
    int yaCount = 0;            // Yellow armor taken
    int gaCount = 0;            // Green armor taken
    int mhCount = 0;            // Mega health taken
    int quadCount = 0;          // Quad damage taken
    int pentCount = 0;          // Pentagram taken
    int ringCount = 0;          // Ring taken
    
    int stack = 0;              // Total damage team can take
    
    // Last pickup times
    float raLastTime = 0.0f;
    float yaLastTime = 0.0f;
    float gaLastTime = 0.0f;
    float mhLastTime = 0.0f;
    float quadLastTime = 0.0f;
    float pentLastTime = 0.0f;
    float ringLastTime = 0.0f;
};

/**
 * @brief Sorted player information for scoreboards
 */
struct SortedPlayerInfo {
    int playerNum = -1;
    const SortedTeamInfo* team = nullptr;
};

// =============================================================================
// HUD Number Style
// =============================================================================

/**
 * @brief Style for drawing numbers on HUD
 */
enum class HudNumberStyle : uint8_t {
    Normal = 0,     // Standard digits
    Large,          // Large status bar digits
    Small,          // Small 8x8 digits
    Proportional    // Proportional font
};

// =============================================================================
// HUD Element State
// =============================================================================

/**
 * @brief Minimum client state required to draw HUD element
 */
enum class HudMinState : uint8_t {
    Disconnected = 0,   // Draw always
    Connected,          // Need connection
    OnServer,           // Need to be on server
    Active              // Need active gameplay
};

// =============================================================================
// Text Box Specification
// =============================================================================

/**
 * @brief Specification for text box drawing
 */
struct TextBoxSpec {
    int x = 0;
    int y = 0;
    int width = 0;      // In characters
    int lines = 0;      // Number of lines
    bool red = false;   // Use red text color
    
    /// Get pixel width
    [[nodiscard]] constexpr int pixelWidth() const noexcept {
        return width * hud_limits::LETTER_WIDTH + 16; // Add padding
    }
    
    /// Get pixel height
    [[nodiscard]] constexpr int pixelHeight() const noexcept {
        return lines * hud_limits::LETTER_HEIGHT + 16; // Add padding
    }
};

// =============================================================================
// UI Mouse State
// =============================================================================

/**
 * @brief Mouse button state
 */
enum class MouseButton : uint8_t {
    None = 0,
    Left = 1,
    Right = 2,
    Middle = 3,
    Button4 = 4,
    Button5 = 5
};

/**
 * @brief Mouse state for UI interactions
 */
struct MouseState {
    int x = 0;
    int y = 0;
    int deltaX = 0;
    int deltaY = 0;
    int scrollDelta = 0;
    MouseButton button = MouseButton::None;
    bool buttonDown = false;
    bool buttonUp = false;
    
    [[nodiscard]] constexpr bool isButtonPressed() const noexcept {
        return buttonDown;
    }
    
    [[nodiscard]] constexpr bool isButtonReleased() const noexcept {
        return buttonUp;
    }
    
    [[nodiscard]] constexpr bool hasScroll() const noexcept {
        return scrollDelta != 0;
    }
};

// =============================================================================
// Drawing Constants
// =============================================================================

namespace draw_constants {
    /// Option padding for menus
    inline constexpr int OPT_PADDING = 4;
    
    /// Flashing arrow characters (for menus)
    [[nodiscard]] inline int flashingArrow(float curtime) noexcept {
        return 12 + (static_cast<int>(curtime * 4) & 1);
    }
    
    [[nodiscard]] inline int flashingCursor(float curtime) noexcept {
        return 10 + (static_cast<int>(curtime * 4) & 1);
    }
}

// =============================================================================
// HUD Element Draw Info
// =============================================================================

/**
 * @brief Information for drawing a HUD element
 */
struct HudDrawInfo {
    HudRect bounds{};           // Element bounds
    HudFrame frame{};           // Frame offsets
    HudColor frameColor{};      // Frame color
    float opacity = 1.0f;       // Element opacity
    bool visible = true;        // Should be drawn
    bool frameVisible = true;   // Frame should be drawn
    int drawOrder = 0;          // Draw order (higher = later)
    int lastDrawSequence = 0;   // Last successful draw frame
    
    /// Get content area (inside frame)
    [[nodiscard]] constexpr HudRect contentArea() const noexcept {
        return HudRect{
            bounds.x + frame.left,
            bounds.y + frame.top,
            bounds.width - frame.horizontalPadding(),
            bounds.height - frame.verticalPadding()
        };
    }
};

// =============================================================================
// Stack Calculation
// =============================================================================

/**
 * @brief Calculate total stack (health + effective armor)
 *
 * @param health Current health
 * @param armorValue Armor points
 * @param armorType Armor absorption rate (0-1)
 * @return Total effective health
 */
[[nodiscard]] constexpr float calculateStack(float health, float armorValue, 
                                              float armorType) noexcept {
    if (armorType <= 0.0f) return health;
    // Armor absorbs armorType fraction of damage
    // Effective health = health + armor / armorType
    return health + (armorValue / armorType);
}

/**
 * @brief Convert item flags to armor type percentage
 *
 * @param items Item flags from stats
 * @return Armor absorption rate (0.0, 0.3, 0.6, or 0.8)
 */
[[nodiscard]] constexpr float getArmorType(int items) noexcept {
    // IT_ARMOR1 = 0x2000, IT_ARMOR2 = 0x4000, IT_ARMOR3 = 0x8000
    if (items & 0x8000) return 0.8f;  // Red armor
    if (items & 0x4000) return 0.6f;  // Yellow armor
    if (items & 0x2000) return 0.3f;  // Green armor
    return 0.0f;                       // No armor
}

} // namespace ezquake::ui
