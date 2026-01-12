/**
 * @file config_types.hpp
 * @brief Configuration, teamplay, and scripting types
 * 
 * Converted from:
 * - config_manager.h: Config file management
 * - teamplay.h: Teamplay features and macros
 * - tp_triggers.h: PCRE trigger system
 */

#pragma once

#include <cstdint>
#include <array>
#include <string_view>
#include <bitset>
#include "core/math/vec3.hpp"

namespace ezquake::config {

// =============================================================================
// Config Constants
// =============================================================================

namespace config_limits {
    constexpr size_t MAX_LOC_NAME = 64;
    constexpr size_t MAX_MACRO_STRING = 2048;
    constexpr size_t MAX_CONFIG_PATH = 256;
    constexpr size_t MAX_TRIGGER_NAME = 64;
    
    // Default config filename
    constexpr std::string_view MAIN_CONFIG_FILENAME = "config.cfg";
}

// =============================================================================
// Message Type Flags
// =============================================================================

/**
 * @brief Message type classification flags
 */
namespace MessageType {
    constexpr uint32_t Unknown = 0;
    constexpr uint32_t Normal = 1;
    constexpr uint32_t Team = 2;
    constexpr uint32_t Spec = 4;
    constexpr uint32_t SpecTeam = 8;
    constexpr uint32_t QTV = 16;
    constexpr uint32_t All = Normal | Team | Spec | SpecTeam | QTV;
}

[[nodiscard]] constexpr bool isTeamMessage(uint32_t msgType) noexcept {
    return (msgType & (MessageType::Team | MessageType::SpecTeam)) != 0;
}

[[nodiscard]] constexpr bool isSpectatorMessage(uint32_t msgType) noexcept {
    return (msgType & (MessageType::Spec | MessageType::SpecTeam)) != 0;
}

// =============================================================================
// FPD (Fairplay Descriptor) Flags
// =============================================================================

/**
 * @brief Fairplay descriptor flags from Qizmo
 */
namespace FPDFlags {
    constexpr uint32_t NoTimers = 2;
    constexpr uint32_t NoSoundTriggers = 4;
    constexpr uint32_t NoForceSkin = 256;
    constexpr uint32_t NoForceColor = 512;
    constexpr uint32_t LimitPitch = (1 << 14);
    constexpr uint32_t LimitYaw = (1 << 15);
}

/**
 * @brief FPD settings container
 */
struct FairplaySettings {
    uint32_t flags{};
    
    [[nodiscard]] constexpr bool allowTimers() const noexcept {
        return (flags & FPDFlags::NoTimers) == 0;
    }
    
    [[nodiscard]] constexpr bool allowSoundTriggers() const noexcept {
        return (flags & FPDFlags::NoSoundTriggers) == 0;
    }
    
    [[nodiscard]] constexpr bool allowForceSkin() const noexcept {
        return (flags & FPDFlags::NoForceSkin) == 0;
    }
    
    [[nodiscard]] constexpr bool allowForceColor() const noexcept {
        return (flags & FPDFlags::NoForceColor) == 0;
    }
    
    [[nodiscard]] constexpr bool limitPitch() const noexcept {
        return (flags & FPDFlags::LimitPitch) != 0;
    }
    
    [[nodiscard]] constexpr bool limitYaw() const noexcept {
        return (flags & FPDFlags::LimitYaw) != 0;
    }
};

// =============================================================================
// Trigger Flags
// =============================================================================

/**
 * @brief PCRE trigger flags
 */
namespace TriggerFlags {
    constexpr uint32_t PrintLow = 1;
    constexpr uint32_t PrintMedium = 2;
    constexpr uint32_t PrintHigh = 4;
    constexpr uint32_t PrintChat = 8;
    constexpr uint32_t PrintCenter = 16;
    constexpr uint32_t PrintEcho = 32;
    constexpr uint32_t PrintInternal = 64;
    constexpr uint32_t PrintAll = PrintLow | PrintMedium | PrintHigh | PrintChat | PrintCenter;
    constexpr uint32_t Final = 256;
    constexpr uint32_t RemoveString = 512;
    constexpr uint32_t NoLog = 1024;
    constexpr uint32_t Enabled = 2048;
    constexpr uint32_t NoAction = 4096;
}

/**
 * @brief Trigger definition
 */
struct TriggerDef {
    std::string_view name;
    std::string_view pattern;
    uint32_t flags{TriggerFlags::Enabled};
    float minInterval{};   // Minimum time between triggers
    double lastTime{};     // Last trigger time
    int32_t counter{};     // Hit counter
    
    [[nodiscard]] constexpr bool isEnabled() const noexcept {
        return (flags & TriggerFlags::Enabled) != 0;
    }
    
    [[nodiscard]] constexpr bool isFinal() const noexcept {
        return (flags & TriggerFlags::Final) != 0;
    }
    
    [[nodiscard]] constexpr bool shouldRemoveString() const noexcept {
        return (flags & TriggerFlags::RemoveString) != 0;
    }
    
    [[nodiscard]] constexpr bool shouldLog() const noexcept {
        return (flags & TriggerFlags::NoLog) == 0;
    }
    
    [[nodiscard]] constexpr bool hasAction() const noexcept {
        return (flags & TriggerFlags::NoAction) == 0;
    }
    
    [[nodiscard]] bool canTrigger(double currentTime) const noexcept {
        if (!isEnabled()) return false;
        if (minInterval <= 0.0f) return true;
        return (currentTime - lastTime) >= static_cast<double>(minInterval);
    }
};

// =============================================================================
// Item Visibility
// =============================================================================

/**
 * @brief Item visibility check parameters
 */
struct ItemVisibility {
    math::Vec3 viewOrg;
    math::Vec3 forward;
    math::Vec3 right;
    math::Vec3 up;
    math::Vec3 entOrg;
    float radius{};
    math::Vec3 direction;
    float distance{};
};

// =============================================================================
// Item Flags (Teamplay)
// =============================================================================

/**
 * @brief Teamplay item flags
 */
namespace ItemFlags {
    constexpr uint32_t Quad = (1 << 0);
    constexpr uint32_t Pent = (1 << 1);
    constexpr uint32_t Ring = (1 << 2);
    constexpr uint32_t Suit = (1 << 3);
    constexpr uint32_t RA = (1 << 4);
    constexpr uint32_t YA = (1 << 5);
    constexpr uint32_t GA = (1 << 6);
    constexpr uint32_t MH = (1 << 7);
    constexpr uint32_t Health = (1 << 8);
    constexpr uint32_t LG = (1 << 9);
    constexpr uint32_t RL = (1 << 10);
    constexpr uint32_t GL = (1 << 11);
    constexpr uint32_t SNG = (1 << 12);
    constexpr uint32_t NG = (1 << 13);
    constexpr uint32_t SSG = (1 << 14);
    constexpr uint32_t Pack = (1 << 15);
    constexpr uint32_t Cells = (1 << 16);
    constexpr uint32_t Rockets = (1 << 17);
    constexpr uint32_t Nails = (1 << 18);
    constexpr uint32_t Shells = (1 << 19);
    constexpr uint32_t Flag = (1 << 20);
    constexpr uint32_t Sentry = (1 << 21);
    constexpr uint32_t Disp = (1 << 22);
    
    // Categories
    constexpr uint32_t AllPowerups = Quad | Pent | Ring | Suit;
    constexpr uint32_t AllArmor = RA | YA | GA;
    constexpr uint32_t AllWeapons = LG | RL | GL | SNG | NG | SSG;
    constexpr uint32_t AllAmmo = Cells | Rockets | Nails | Shells;
}

[[nodiscard]] constexpr const char* getItemName(uint32_t flag) noexcept {
    switch (flag) {
        case ItemFlags::Quad: return "quad";
        case ItemFlags::Pent: return "pent";
        case ItemFlags::Ring: return "ring";
        case ItemFlags::Suit: return "suit";
        case ItemFlags::RA: return "ra";
        case ItemFlags::YA: return "ya";
        case ItemFlags::GA: return "ga";
        case ItemFlags::MH: return "mh";
        case ItemFlags::LG: return "lg";
        case ItemFlags::RL: return "rl";
        case ItemFlags::GL: return "gl";
        case ItemFlags::SNG: return "sng";
        case ItemFlags::NG: return "ng";
        case ItemFlags::SSG: return "ssg";
        default: return "unknown";
    }
}

[[nodiscard]] constexpr bool isPowerup(uint32_t item) noexcept {
    return (item & ItemFlags::AllPowerups) != 0;
}

[[nodiscard]] constexpr bool isArmor(uint32_t item) noexcept {
    return (item & ItemFlags::AllArmor) != 0;
}

[[nodiscard]] constexpr bool isWeapon(uint32_t item) noexcept {
    return (item & ItemFlags::AllWeapons) != 0;
}

// =============================================================================
// Location Data
// =============================================================================

/**
 * @brief Location file entry
 */
struct LocationEntry {
    math::Vec3 coord;
    std::string_view name;
    
    [[nodiscard]] float distanceSquared(const math::Vec3& point) const noexcept {
        float dx = coord.x() - point.x();
        float dy = coord.y() - point.y();
        float dz = coord.z() - point.z();
        return dx*dx + dy*dy + dz*dz;
    }
};

// =============================================================================
// Skin Forcing
// =============================================================================

/**
 * @brief Skin forcing configuration
 */
struct SkinForcingConfig {
    std::string_view teamSkin;
    std::string_view enemySkin;
    std::string_view teamQuadSkin;
    std::string_view enemyQuadSkin;
    std::string_view teamPentSkin;
    std::string_view enemyPentSkin;
    std::string_view teamBothSkin;
    std::string_view enemyBothSkin;
    
    [[nodiscard]] constexpr bool hasSkinForcing() const noexcept {
        return !teamSkin.empty() || !enemySkin.empty();
    }
};

// =============================================================================
// Color Forcing
// =============================================================================

/**
 * @brief Color forcing configuration
 */
struct ColorForcingConfig {
    int8_t teamTopColor{-1};
    int8_t teamBottomColor{-1};
    int8_t enemyTopColor{-1};
    int8_t enemyBottomColor{-1};
    
    [[nodiscard]] constexpr bool hasTeamColor() const noexcept {
        return teamTopColor >= 0 || teamBottomColor >= 0;
    }
    
    [[nodiscard]] constexpr bool hasEnemyColor() const noexcept {
        return enemyTopColor >= 0 || enemyBottomColor >= 0;
    }
    
    [[nodiscard]] constexpr bool hasColorForcing() const noexcept {
        return hasTeamColor() || hasEnemyColor();
    }
};

// =============================================================================
// Config Save Options
// =============================================================================

/**
 * @brief What to include in config saves
 */
enum class ConfigSaveOption : uint8_t {
    All = 0,
    ChangedOnly = 1,
    NonDefault = 2
};

/**
 * @brief Config save state
 */
struct ConfigSaveSettings {
    bool saveUnchanged{true};
    bool legacyExec{false};
    ConfigSaveOption option{ConfigSaveOption::All};
    
    [[nodiscard]] constexpr bool shouldSave(bool isChanged, bool isDefault) const noexcept {
        switch (option) {
            case ConfigSaveOption::All:
                return true;
            case ConfigSaveOption::ChangedOnly:
                return isChanged;
            case ConfigSaveOption::NonDefault:
                return !isDefault;
        }
        return true;
    }
};

// =============================================================================
// Alias Definition
// =============================================================================

/**
 * @brief Command alias definition
 */
struct AliasDef {
    std::string_view name;
    std::string_view value;
    bool isArchived{false};  // Saved to config
    
    [[nodiscard]] constexpr bool isEmpty() const noexcept {
        return value.empty();
    }
};

// =============================================================================
// Bind Definition
// =============================================================================

/**
 * @brief Key binding definition
 */
struct BindDef {
    int32_t key{};
    std::string_view command;
    
    [[nodiscard]] constexpr bool isBound() const noexcept {
        return !command.empty();
    }
};

// =============================================================================
// Macro Expansion State
// =============================================================================

/**
 * @brief State for macro string parsing
 */
struct MacroState {
    int32_t recursionDepth{};
    bool inTeamMessage{false};
    bool parseFunChars{true};
    
    static constexpr int32_t MAX_RECURSION = 8;
    
    [[nodiscard]] constexpr bool canRecurse() const noexcept {
        return recursionDepth < MAX_RECURSION;
    }
    
    constexpr void enterRecursion() noexcept {
        ++recursionDepth;
    }
    
    constexpr void exitRecursion() noexcept {
        if (recursionDepth > 0) --recursionDepth;
    }
};

// =============================================================================
// Teamplay State
// =============================================================================

/**
 * @brief Teamplay state tracking
 */
struct TeamplayState {
    std::string_view playerName;
    std::string_view playerTeam;
    std::string_view mapName;
    int32_t playerCount{};
    bool needRefreshSkins{false};
    
    [[nodiscard]] constexpr bool hasTeam() const noexcept {
        return !playerTeam.empty();
    }
    
    [[nodiscard]] constexpr bool isTeamGame() const noexcept {
        return playerCount > 1 && hasTeam();
    }
};

// =============================================================================
// Sky/Map Group
// =============================================================================

/**
 * @brief Sky or map group definition
 */
struct GroupDef {
    std::string_view name;
    bool isSystem{false};  // Built-in vs user-defined
};

// =============================================================================
// Config Statistics
// =============================================================================

/**
 * @brief Configuration statistics
 */
struct ConfigStats {
    uint32_t totalCvars{};
    uint32_t changedCvars{};
    uint32_t totalAliases{};
    uint32_t totalBinds{};
    uint32_t totalTriggers{};
    
    [[nodiscard]] constexpr float changeRatio() const noexcept {
        if (totalCvars == 0) return 0.0f;
        return static_cast<float>(changedCvars) / static_cast<float>(totalCvars);
    }
};

} // namespace ezquake::config
