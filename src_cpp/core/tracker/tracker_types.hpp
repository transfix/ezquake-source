/**
 * @file tracker_types.hpp
 * @brief Frag/kill tracker type definitions
 * 
 * This module provides types for the frag/kill tracker HUD element
 * that displays kills, deaths, streaks, and pickups.
 * 
 * Replaces:
 * - vx_tracker.c: trackmsg_t, killing_streak_t, weapon_label_t
 */

#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <string_view>
#include <array>
#include <vector>
#include <optional>

namespace ezquake::tracker {

// ============================================================================
// Constants
// ============================================================================

namespace tracker_limits {
    constexpr int32_t MAX_TRACKER_MESSAGES = 30;
    constexpr int32_t MAX_SEGMENTS_PER_LINE = 6;
    constexpr int32_t MAX_IMAGES_PER_LINE = 2;
    constexpr int32_t MAX_TRACKER_MSG_LEN = 500;
    constexpr int32_t MAX_IMAGENAME = 32;
    constexpr int32_t MAX_IMAGES_PER_WEAPON = 4;
    constexpr int32_t MAX_WEAPON_CLASSES = 16;
    constexpr int32_t SEGMENT_TEXT_LEN = 64;
}

// ============================================================================
// Segment Flags
// ============================================================================

/**
 * @brief Flags for tracker message segments
 */
enum class SegmentFlag : uint8_t {
    None = 0,
    Weapon = 1,         ///< Segment represents a weapon
    Highlight = 2,      ///< Should be highlighted
    Own = 4,            ///< Your own frag/death
    TeamKill = 8        ///< Teamkill indicator
};

[[nodiscard]] constexpr SegmentFlag operator|(SegmentFlag a, SegmentFlag b) noexcept {
    return static_cast<SegmentFlag>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

[[nodiscard]] constexpr SegmentFlag operator&(SegmentFlag a, SegmentFlag b) noexcept {
    return static_cast<SegmentFlag>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

[[nodiscard]] constexpr bool hasFlag(SegmentFlag flags, SegmentFlag flag) noexcept {
    return (static_cast<uint8_t>(flags) & static_cast<uint8_t>(flag)) != 0;
}

// ============================================================================
// Text Alignment
// ============================================================================

/**
 * @brief Text alignment for segments
 */
enum class TrackerAlignment : uint8_t {
    Left,
    Center,
    Right
};

// ============================================================================
// Color Types
// ============================================================================

/**
 * @brief RGBA color for tracker segments
 */
struct TrackerColor {
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    uint8_t a = 255;
    
    constexpr TrackerColor() = default;
    
    constexpr TrackerColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}
    
    [[nodiscard]] constexpr bool isWhite() const noexcept {
        return r == 255 && g == 255 && b == 255;
    }
    
    [[nodiscard]] constexpr bool isTransparent() const noexcept {
        return a == 0;
    }
    
    constexpr bool operator==(const TrackerColor& other) const noexcept = default;
};

// Standard tracker colors
namespace colors {
    constexpr TrackerColor White{255, 255, 255, 255};
    constexpr TrackerColor YouDied{255, 170, 0, 255};      // 960 - suicides
    constexpr TrackerColor Streak{255, 113, 0, 255};       // 940 - streaks
    constexpr TrackerColor KilledYou{255, 0, 0, 255};      // 900 - someone killed you
    constexpr TrackerColor TeamKill{77, 227, 0, 255};      // 380 - teamkills
    constexpr TrackerColor Good{0, 255, 0, 255};           // Good news
    constexpr TrackerColor Bad{255, 0, 0, 255};            // Bad news
}

// ============================================================================
// Message Segment
// ============================================================================

/**
 * @brief Single segment of a tracker message
 */
struct TrackerSegment {
    std::array<char, tracker_limits::SEGMENT_TEXT_LEN> text{};
    TrackerColor color{colors::White};
    SegmentFlag flags = SegmentFlag::None;
    TrackerAlignment alignment = TrackerAlignment::Left;
    int32_t imageIndex = -1;    ///< Index into image array, -1 if none
    
    TrackerSegment() = default;
    
    explicit TrackerSegment(std::string_view str, TrackerColor clr = colors::White) 
        : color(clr) {
        setText(str);
    }
    
    void setText(std::string_view str) {
        size_t len = std::min(str.size(), text.size() - 1);
        std::copy_n(str.begin(), len, text.begin());
        text[len] = '\0';
    }
    
    [[nodiscard]] std::string_view getText() const noexcept {
        return text.data();
    }
    
    [[nodiscard]] bool isEmpty() const noexcept {
        return text[0] == '\0' && imageIndex < 0;
    }
    
    [[nodiscard]] bool hasImage() const noexcept {
        return imageIndex >= 0;
    }
    
    [[nodiscard]] bool isWeapon() const noexcept {
        return hasFlag(flags, SegmentFlag::Weapon);
    }
};

// ============================================================================
// Tracker Message
// ============================================================================

/**
 * @brief Complete tracker message with multiple segments
 * 
 * Replaces C: trackmsg_t
 */
struct TrackerMessage {
    std::array<TrackerSegment, tracker_limits::MAX_SEGMENTS_PER_LINE> segments{};
    int32_t segmentCount = 0;
    float dieTime = 0.0f;           ///< Time when message expires
    bool linked = false;            ///< Delete subsequent message at same time
    int32_t printableCharacters = 0; ///< Count for positioning
    int32_t imageCharacters = 0;    ///< Image count for sizing
    bool pad = false;               ///< Add padding
    
    TrackerMessage() = default;
    
    [[nodiscard]] bool isActive(float currentTime) const noexcept {
        return dieTime > currentTime;
    }
    
    [[nodiscard]] bool isEmpty() const noexcept {
        return segmentCount == 0;
    }
    
    [[nodiscard]] int32_t totalCharacters() const noexcept {
        return printableCharacters + imageCharacters;
    }
    
    TrackerSegment* addSegment() {
        if (segmentCount >= tracker_limits::MAX_SEGMENTS_PER_LINE) {
            return nullptr;
        }
        return &segments[static_cast<size_t>(segmentCount++)];
    }
    
    bool addTextSegment(std::string_view text, const TrackerColor& color, SegmentFlag flags = SegmentFlag::None) {
        auto* seg = addSegment();
        if (!seg) return false;
        
        seg->setText(text);
        seg->color = color;
        seg->flags = flags;
        printableCharacters += static_cast<int32_t>(text.size());
        return true;
    }
    
    bool addImageSegment(int32_t imageIndex) {
        auto* seg = addSegment();
        if (!seg) return false;
        
        seg->imageIndex = imageIndex;
        ++imageCharacters;
        return true;
    }
    
    void clear() {
        for (auto& seg : segments) {
            seg = TrackerSegment{};
        }
        segmentCount = 0;
        dieTime = 0.0f;
        linked = false;
        printableCharacters = 0;
        imageCharacters = 0;
        pad = false;
    }
};

// ============================================================================
// Killing Streak
// ============================================================================

/**
 * @brief Killing streak definition
 * 
 * Replaces C: killing_streak_t
 */
struct KillingStreak {
    int32_t fragThreshold = 0;      ///< Minimum frags for this streak
    std::string displayString;       ///< Display text (e.g., "on a killing spree")
    std::string internalName;        ///< Internal identifier
    std::string soundFile;           ///< Sound file to play
    
    KillingStreak() = default;
    
    KillingStreak(int32_t threshold, std::string_view display, 
                  std::string_view name, std::string_view sound)
        : fragThreshold(threshold)
        , displayString(display)
        , internalName(name)
        , soundFile(sound) {}
    
    [[nodiscard]] bool matches(int32_t fragCount) const noexcept {
        return fragCount >= fragThreshold;
    }
};

/**
 * @brief Create default streak definitions
 */
[[nodiscard]] inline std::vector<KillingStreak> createDefaultStreaks() {
    return {
        {100, "teh chet",           "0wnhack",     "client/streakx6.wav"},
        {50,  "the master now",     "master",      "client/streakx5.wav"},
        {20,  "godlike",            "godlike",     "client/streakx4.wav"},
        {15,  "unstoppable",        "unstoppable", "client/streakx3.wav"},
        {10,  "on a rampage",       "rampage",     "client/streakx2.wav"},
        {5,   "on a killing spree", "spree",       "client/streakx1.wav"},
    };
}

/**
 * @brief Find streak for given frag count
 */
[[nodiscard]] inline const KillingStreak* findStreak(
    const std::vector<KillingStreak>& streaks,
    int32_t fragCount
) noexcept {
    for (const auto& streak : streaks) {
        if (streak.matches(fragCount)) {
            return &streak;
        }
    }
    return nullptr;
}

// ============================================================================
// Weapon Label
// ============================================================================

/**
 * @brief Weapon display label with color coding
 * 
 * Replaces C: weapon_label_t
 */
struct WeaponLabel {
    std::string label;
    std::array<int32_t, 4> colorStarts{};  ///< Start positions for color changes
    std::array<TrackerColor, 4> colors{};   ///< Colors for each section
    int32_t colorCount = 0;
    
    WeaponLabel() = default;
    
    explicit WeaponLabel(std::string_view lbl) : label(lbl) {}
    
    void addColorSection(int32_t start, const TrackerColor& color) {
        if (colorCount < 4) {
            colorStarts[static_cast<size_t>(colorCount)] = start;
            colors[static_cast<size_t>(colorCount)] = color;
            ++colorCount;
        }
    }
};

// ============================================================================
// Tracker State
// ============================================================================

/**
 * @brief Tracker display configuration
 */
struct TrackerConfig {
    bool enabled = true;
    bool showFrags = true;
    bool showStreaks = false;
    bool showFlags = false;
    bool showPickups = false;
    bool alignRight = true;
    bool proportional = false;
    bool showInConsole = false;
    float displayTime = 4.0f;       ///< How long messages display
    int32_t maxMessages = 20;       ///< Maximum concurrent messages
    float scale = 1.0f;
    float imageScale = 1.0f;
    int32_t nameWidth = 0;          ///< Fixed name width (0 = auto)
    int32_t xOffset = 0;
    int32_t yOffset = 0;
    TrackerColor frameColor{0, 0, 0, 0};
    
    // Color configuration
    TrackerColor colorGood{0, 255, 0, 255};
    TrackerColor colorBad{255, 0, 0, 255};
    TrackerColor colorTeamKillGood{255, 255, 0, 255};
    TrackerColor colorTeamKillBad{0, 0, 255, 255};
    TrackerColor colorMyFrag{0, 255, 0, 255};
    TrackerColor colorFragOnMe{255, 0, 0, 255};
    TrackerColor colorSuicide{255, 0, 0, 255};
    
    // String configuration
    std::string stringSuicides = " (suicides)";
    std::string stringDied = " (died)";
    std::string stringTeammate = "teammate";
    std::string stringEnemy = "enemy";
    std::string stringOwnFragPrefix = "You fragged ";
};

/**
 * @brief Tracker display state
 */
struct TrackerState {
    std::array<TrackerMessage, tracker_limits::MAX_TRACKER_MESSAGES> messages{};
    int32_t activeTrack = 0;        ///< Current write position
    int32_t maxActiveTracks = 0;    ///< Configured maximum
    std::vector<KillingStreak> streaks;
    std::array<int32_t, tracker_limits::MAX_WEAPON_CLASSES> weaponImages{};
    std::array<WeaponLabel, tracker_limits::MAX_WEAPON_CLASSES> weaponLabels{};
    TrackerConfig config;
    
    TrackerState() {
        streaks = createDefaultStreaks();
        weaponImages.fill(-1);
    }
    
    [[nodiscard]] TrackerMessage* nextMessage() noexcept {
        if (maxActiveTracks <= 0) return nullptr;
        
        auto* msg = &messages[static_cast<size_t>(activeTrack)];
        activeTrack = (activeTrack + 1) % maxActiveTracks;
        return msg;
    }
    
    [[nodiscard]] int32_t activeCount(float currentTime) const noexcept {
        int32_t count = 0;
        for (const auto& msg : messages) {
            if (msg.isActive(currentTime)) ++count;
        }
        return count;
    }
    
    void clearExpired(float currentTime) {
        for (auto& msg : messages) {
            if (!msg.isActive(currentTime)) {
                msg.clear();
            }
        }
    }
    
    void clearAll() {
        for (auto& msg : messages) {
            msg.clear();
        }
        activeTrack = 0;
    }
    
    void setMaxMessages(int32_t max) noexcept {
        maxActiveTracks = std::clamp(max, 0, tracker_limits::MAX_TRACKER_MESSAGES);
    }
};

// ============================================================================
// Tracker Event Types
// ============================================================================

/**
 * @brief Type of tracker event
 */
enum class TrackerEventType : uint8_t {
    None,
    Frag,           ///< Player killed player
    Suicide,        ///< Player killed self
    TeamKill,       ///< Killed teammate
    Death,          ///< You died
    StreakStart,    ///< Streak started
    StreakEnd,      ///< Streak ended
    FlagPickup,     ///< Flag picked up
    FlagCapture,    ///< Flag captured
    FlagDrop,       ///< Flag dropped
    ItemPickup      ///< Item picked up
};

[[nodiscard]] constexpr std::string_view trackerEventTypeName(TrackerEventType type) noexcept {
    switch (type) {
        case TrackerEventType::None:        return "none";
        case TrackerEventType::Frag:        return "frag";
        case TrackerEventType::Suicide:     return "suicide";
        case TrackerEventType::TeamKill:    return "teamkill";
        case TrackerEventType::Death:       return "death";
        case TrackerEventType::StreakStart: return "streak_start";
        case TrackerEventType::StreakEnd:   return "streak_end";
        case TrackerEventType::FlagPickup:  return "flag_pickup";
        case TrackerEventType::FlagCapture: return "flag_capture";
        case TrackerEventType::FlagDrop:    return "flag_drop";
        case TrackerEventType::ItemPickup:  return "item_pickup";
        default: return "unknown";
    }
}

/**
 * @brief Tracker event for processing
 */
struct TrackerEvent {
    TrackerEventType type = TrackerEventType::None;
    int32_t attackerPlayer = -1;
    int32_t victimPlayer = -1;
    int32_t weaponIndex = -1;
    int32_t itemIndex = -1;
    std::string attackerName;
    std::string victimName;
    bool isOwn = false;             ///< Involves local player
    bool isTeammate = false;        ///< Involves teammate
    float timestamp = 0.0f;
    
    [[nodiscard]] bool isValid() const noexcept {
        return type != TrackerEventType::None;
    }
    
    [[nodiscard]] bool isFrag() const noexcept {
        return type == TrackerEventType::Frag || 
               type == TrackerEventType::TeamKill;
    }
    
    [[nodiscard]] bool isNegative() const noexcept {
        return type == TrackerEventType::Death ||
               type == TrackerEventType::Suicide ||
               (type == TrackerEventType::TeamKill && isOwn);
    }
};

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * @brief Parse tracker color from string (0-9 format)
 */
[[nodiscard]] inline std::optional<TrackerColor> parseTrackerColor(std::string_view str) {
    if (str.size() < 3) return std::nullopt;
    
    auto charToValue = [](char c) -> int {
        if (c >= '0' && c <= '9') return (c - '0') * 255 / 9;
        return -1;
    };
    
    int r = charToValue(str[0]);
    int g = charToValue(str[1]);
    int b = charToValue(str[2]);
    
    if (r < 0 || g < 0 || b < 0) return std::nullopt;
    
    return TrackerColor{
        static_cast<uint8_t>(r),
        static_cast<uint8_t>(g),
        static_cast<uint8_t>(b),
        255
    };
}

/**
 * @brief Create default tracker configuration
 */
[[nodiscard]] inline TrackerConfig createDefaultTrackerConfig() {
    return TrackerConfig{};
}

/**
 * @brief Check if event should be filtered based on config
 */
[[nodiscard]] inline bool shouldFilterEvent(
    const TrackerConfig& config,
    TrackerEventType type
) noexcept {
    switch (type) {
        case TrackerEventType::Frag:
        case TrackerEventType::Suicide:
        case TrackerEventType::TeamKill:
        case TrackerEventType::Death:
            return !config.showFrags;
        case TrackerEventType::StreakStart:
        case TrackerEventType::StreakEnd:
            return !config.showStreaks;
        case TrackerEventType::FlagPickup:
        case TrackerEventType::FlagCapture:
        case TrackerEventType::FlagDrop:
            return !config.showFlags;
        case TrackerEventType::ItemPickup:
            return !config.showPickups;
        default:
            return true;
    }
}

} // namespace ezquake::tracker
