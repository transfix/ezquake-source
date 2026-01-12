/**
 * @file teamplay_types.hpp
 * @brief Teamplay, item tracking, and skin forcing types
 * 
 * Modern C++20 implementation of teamplay enhancement types including
 * item visibility, location tracking, skin forcing, and team communication.
 * 
 * @note Iteration 37: Teamplay Types
 */

#pragma once

#include <array>
#include <cstdint>
#include <string_view>
#include <optional>
#include <bitset>
#include <string>

#include "../math/vec3.hpp"

namespace ezquake::teamplay {

//=============================================================================
// Teamplay Constants
//=============================================================================

namespace teamplay_limits {
    inline constexpr int MAX_LOC_NAME = 64;
    inline constexpr int MAX_MACRO_STRING = 2048;
    inline constexpr int NUM_ITEMFLAGS = 32;
    
    // Message types
    inline constexpr int MSGTYPE_UNKNOWN = 0;
    inline constexpr int MSGTYPE_NORMAL = 1;
    inline constexpr int MSGTYPE_TEAM = 2;
    inline constexpr int MSGTYPE_SPEC = 4;
    inline constexpr int MSGTYPE_SPECTEAM = 8;
    inline constexpr int MSGTYPE_QTV = 16;
}

//=============================================================================
// Item Flags
//=============================================================================

/**
 * @brief Item flag bits for tracking game items
 * 
 * Converted from: teamplay.h it_* defines
 */
enum class ItemFlag : uint32_t {
    Quad = 0,
    Pent,
    Ring,
    Suit,
    RedArmor,
    YellowArmor,
    GreenArmor,
    MegaHealth,
    Health,
    LightningGun,
    RocketLauncher,
    GrenadeLauncher,
    SuperNailgun,
    Nailgun,
    SuperShotgun,
    Pack,
    Cells,
    Rockets,
    Nails,
    Shells,
    Flag,
    Teammate,
    Enemy,
    Eyes,
    Sentry,
    Dispenser,
    Quaded,
    Pented,
    Rune1,
    Rune2,
    Rune3,
    Rune4,
    
    Count = 32
};

/**
 * @brief Bitset for item flags
 */
using ItemFlags = std::bitset<static_cast<size_t>(ItemFlag::Count)>;

/**
 * @brief Convert ItemFlag to bit value
 */
[[nodiscard]] constexpr uint32_t itemFlagBit(ItemFlag flag) noexcept {
    return 1u << static_cast<uint32_t>(flag);
}

/**
 * @brief Get human-readable name for item
 */
[[nodiscard]] constexpr std::string_view itemName(ItemFlag flag) noexcept {
    switch (flag) {
        case ItemFlag::Quad:            return "quad";
        case ItemFlag::Pent:            return "pent";
        case ItemFlag::Ring:            return "ring";
        case ItemFlag::Suit:            return "suit";
        case ItemFlag::RedArmor:        return "ra";
        case ItemFlag::YellowArmor:     return "ya";
        case ItemFlag::GreenArmor:      return "ga";
        case ItemFlag::MegaHealth:      return "mega";
        case ItemFlag::Health:          return "health";
        case ItemFlag::LightningGun:    return "lg";
        case ItemFlag::RocketLauncher:  return "rl";
        case ItemFlag::GrenadeLauncher: return "gl";
        case ItemFlag::SuperNailgun:    return "sng";
        case ItemFlag::Nailgun:         return "ng";
        case ItemFlag::SuperShotgun:    return "ssg";
        case ItemFlag::Pack:            return "pack";
        case ItemFlag::Cells:           return "cells";
        case ItemFlag::Rockets:         return "rockets";
        case ItemFlag::Nails:           return "nails";
        case ItemFlag::Shells:          return "shells";
        case ItemFlag::Flag:            return "flag";
        case ItemFlag::Teammate:        return "teammate";
        case ItemFlag::Enemy:           return "enemy";
        case ItemFlag::Eyes:            return "eyes";
        case ItemFlag::Sentry:          return "sentry";
        case ItemFlag::Dispenser:       return "dispenser";
        case ItemFlag::Quaded:          return "quaded";
        case ItemFlag::Pented:          return "pented";
        case ItemFlag::Rune1:           return "rune1";
        case ItemFlag::Rune2:           return "rune2";
        case ItemFlag::Rune3:           return "rune3";
        case ItemFlag::Rune4:           return "rune4";
        default:                        return "unknown";
    }
}

//=============================================================================
// Item Flag Combinations
//=============================================================================

namespace item_masks {
    // Powerups
    inline constexpr uint32_t RUNES = itemFlagBit(ItemFlag::Rune1) | 
                                      itemFlagBit(ItemFlag::Rune2) |
                                      itemFlagBit(ItemFlag::Rune3) | 
                                      itemFlagBit(ItemFlag::Rune4);
    
    inline constexpr uint32_t POWERUPS = itemFlagBit(ItemFlag::Quad) | 
                                         itemFlagBit(ItemFlag::Pent) |
                                         itemFlagBit(ItemFlag::Ring) | 
                                         itemFlagBit(ItemFlag::Suit);
    
    inline constexpr uint32_t WEAPONS = itemFlagBit(ItemFlag::LightningGun) | 
                                        itemFlagBit(ItemFlag::RocketLauncher) |
                                        itemFlagBit(ItemFlag::GrenadeLauncher) | 
                                        itemFlagBit(ItemFlag::SuperNailgun) |
                                        itemFlagBit(ItemFlag::SuperShotgun);
    
    inline constexpr uint32_t ARMOR = itemFlagBit(ItemFlag::RedArmor) | 
                                      itemFlagBit(ItemFlag::YellowArmor) |
                                      itemFlagBit(ItemFlag::GreenArmor);
    
    inline constexpr uint32_t AMMO = itemFlagBit(ItemFlag::Cells) | 
                                     itemFlagBit(ItemFlag::Rockets) |
                                     itemFlagBit(ItemFlag::Nails) | 
                                     itemFlagBit(ItemFlag::Shells);
    
    inline constexpr uint32_t PLAYERS = itemFlagBit(ItemFlag::Teammate) | 
                                        itemFlagBit(ItemFlag::Enemy) |
                                        itemFlagBit(ItemFlag::Eyes);
}

[[nodiscard]] constexpr bool isPowerup(uint32_t items) noexcept {
    return (items & item_masks::POWERUPS) != 0;
}

[[nodiscard]] constexpr bool isWeapon(uint32_t items) noexcept {
    return (items & item_masks::WEAPONS) != 0;
}

[[nodiscard]] constexpr bool isArmor(uint32_t items) noexcept {
    return (items & item_masks::ARMOR) != 0;
}

[[nodiscard]] constexpr bool isAmmo(uint32_t items) noexcept {
    return (items & item_masks::AMMO) != 0;
}

[[nodiscard]] constexpr bool isPlayer(uint32_t items) noexcept {
    return (items & item_masks::PLAYERS) != 0;
}

//=============================================================================
// FPD (Fake Player Detection) Flags
//=============================================================================

/**
 * @brief FPD restriction flags
 * 
 * Converted from: teamplay.h FPD_* defines
 */
enum class FPDFlag : uint16_t {
    NoTimers = 2,           ///< Disable powerup timer
    NoSoundTriggers = 4,    ///< Disable sound triggers
    NoForceSkin = 256,      ///< Disable skin forcing
    NoForceColor = 512,     ///< Disable color forcing
    LimitPitch = 1 << 14,   ///< Limit pitch angle
    LimitYaw = 1 << 15,     ///< Limit yaw angle
};

/**
 * @brief Check if FPD flag is set
 */
[[nodiscard]] constexpr bool hasFPD(uint16_t fpd, FPDFlag flag) noexcept {
    return (fpd & static_cast<uint16_t>(flag)) != 0;
}

//=============================================================================
// Message Type
//=============================================================================

/**
 * @brief Chat message type for categorization
 */
enum class MessageType : uint8_t {
    Unknown = 0,
    Normal = 1,
    Team = 2,
    Spectator = 4,
    SpectatorTeam = 8,
    QTV = 16
};

[[nodiscard]] constexpr std::string_view messageTypeName(MessageType type) noexcept {
    switch (type) {
        case MessageType::Normal:         return "normal";
        case MessageType::Team:           return "team";
        case MessageType::Spectator:      return "spec";
        case MessageType::SpectatorTeam:  return "specteam";
        case MessageType::QTV:            return "qtv";
        default:                          return "unknown";
    }
}

[[nodiscard]] constexpr bool isTeamMessage(MessageType type) noexcept {
    return type == MessageType::Team || type == MessageType::SpectatorTeam;
}

[[nodiscard]] constexpr bool isSpectatorMessage(MessageType type) noexcept {
    return type == MessageType::Spectator || type == MessageType::SpectatorTeam;
}

//=============================================================================
// Item Visibility
//=============================================================================

/**
 * @brief Item visibility check parameters
 * 
 * Converted from: teamplay.h item_vis_t
 */
struct ItemVisibility {
    math::Vec3 viewOrigin{};
    math::Vec3 forward{};
    math::Vec3 right{};
    math::Vec3 up{};
    math::Vec3 entityOrigin{};
    float radius = 0.0f;
    math::Vec3 direction{};     ///< Direction to entity
    float distance = 0.0f;      ///< Distance to entity
    
    /**
     * @brief Calculate direction and distance from viewOrigin to entityOrigin
     */
    void calculate() noexcept {
        math::Vec3 diff = entityOrigin - viewOrigin;
        distance = diff.length();
        if (distance > 0.0f) {
            direction = diff.normalized();
        } else {
            direction = math::Vec3(1, 0, 0);
        }
    }
    
    /**
     * @brief Check if entity is in front of view
     */
    [[nodiscard]] bool isInFront() const noexcept {
        return direction.dot(forward) > 0.0f;
    }
    
    /**
     * @brief Get angle to entity (in degrees)
     */
    [[nodiscard]] float angleToEntity() const noexcept {
        float dot = direction.dot(forward);
        // Clamp to valid range for acos
        dot = std::max(-1.0f, std::min(1.0f, dot));
        return std::acos(dot) * 57.29578f;  // Convert to degrees
    }
};

//=============================================================================
// Location Data
//=============================================================================

/**
 * @brief Location entry for .loc files
 * 
 * Converted from: teamplay.h locdata_t
 */
struct LocationData {
    math::Vec3 coord{};
    std::string name;
    
    LocationData() = default;
    LocationData(const math::Vec3& c, std::string_view n) 
        : coord(c), name(n) {}
    
    /**
     * @brief Calculate distance to point
     */
    [[nodiscard]] float distanceTo(const math::Vec3& point) const noexcept {
        return (coord - point).length();
    }
};

//=============================================================================
// Teamplay Variables State
//=============================================================================

/**
 * @brief Teamplay tracking state (cleared on new map)
 * 
 * Converted from: teamplay.h tvars_t
 */
struct TeamplayState {
    // Player state
    int health = 0;
    uint32_t items = 0;
    uint32_t oldItems = 0;
    int activeWeapon = 0;
    
    // Timing
    double deathTriggerTime = 0.0;
    float skinsReplyTime = 0.0f;
    float versionReplyTime = 0.0f;
    
    // Locations
    std::array<char, teamplay_limits::MAX_LOC_NAME> lastDeathLoc{};
    std::array<char, teamplay_limits::MAX_LOC_NAME> tookLoc{};
    std::array<char, teamplay_limits::MAX_LOC_NAME> pointLoc{};
    std::array<char, teamplay_limits::MAX_LOC_NAME> nearestItemLoc{};
    std::array<char, teamplay_limits::MAX_LOC_NAME> lastReportedLoc{};
    std::array<char, teamplay_limits::MAX_LOC_NAME> lastDropLoc{};
    
    // Item tracking
    std::array<char, 32> tookName{};
    int tookFlag = 0;
    double tookTime = 0.0;
    
    // Point tracking
    double pointTime = 0.0;
    std::array<char, 64> pointName{};
    int pointFlag = 0;
    int pointType = 0;
    
    // Drop tracking
    double lastDropTime = 0.0;
    
    // Trigger tracking
    std::array<char, 256> lastTriggerMatch{};
    
    // Need tracking
    uint32_t needFlags = 0;
    
    // Player counts
    int numEnemies = 0;
    int numFriendlies = 0;
    int lastNumEnemies = 0;
    int lastNumFriendlies = 0;
    
    // Enemy powerup tracking
    uint32_t enemyPowerups = 0;
    double enemyPowerupsTime = 0.0;
    
    /**
     * @brief Reset state for new map
     */
    void reset() noexcept {
        health = 0;
        items = 0;
        oldItems = 0;
        activeWeapon = 0;
        deathTriggerTime = 0.0;
        skinsReplyTime = 0.0f;
        versionReplyTime = 0.0f;
        tookFlag = 0;
        tookTime = 0.0;
        pointTime = 0.0;
        pointFlag = 0;
        pointType = 0;
        lastDropTime = 0.0;
        needFlags = 0;
        numEnemies = 0;
        numFriendlies = 0;
        lastNumEnemies = 0;
        lastNumFriendlies = 0;
        enemyPowerups = 0;
        enemyPowerupsTime = 0.0;
        
        lastDeathLoc.fill('\0');
        tookLoc.fill('\0');
        pointLoc.fill('\0');
        nearestItemLoc.fill('\0');
        lastReportedLoc.fill('\0');
        lastDropLoc.fill('\0');
        tookName.fill('\0');
        pointName.fill('\0');
        lastTriggerMatch.fill('\0');
    }
    
    /**
     * @brief Check if took info is still valid
     */
    [[nodiscard]] bool tookEmpty(double currentTime, double timeout) const noexcept {
        return tookTime == 0.0 || currentTime > tookTime + timeout;
    }
    
    /**
     * @brief Check if any powerups are active
     */
    [[nodiscard]] bool hasPowerups() const noexcept {
        return isPowerup(items);
    }
    
    /**
     * @brief Check if items changed since last frame
     */
    [[nodiscard]] bool itemsChanged() const noexcept {
        return items != oldItems;
    }
    
    /**
     * @brief Get newly picked up items
     */
    [[nodiscard]] uint32_t newItems() const noexcept {
        return items & ~oldItems;
    }
    
    /**
     * @brief Get lost items
     */
    [[nodiscard]] uint32_t lostItems() const noexcept {
        return oldItems & ~items;
    }
};

//=============================================================================
// Skin Forcing
//=============================================================================

/**
 * @brief Skin type for forcing
 */
enum class SkinForceType : uint8_t {
    Normal = 0,     ///< Normal skin
    Team,           ///< Team skin
    Enemy,          ///< Enemy skin
    TeamQuad,       ///< Team with quad
    EnemyQuad,      ///< Enemy with quad
    TeamPent,       ///< Team with pent
    EnemyPent,      ///< Enemy with pent
    TeamBoth,       ///< Team with both powerups
    EnemyBoth,      ///< Enemy with both powerups
    
    Count
};

[[nodiscard]] constexpr std::string_view skinForceTypeName(SkinForceType type) noexcept {
    switch (type) {
        case SkinForceType::Normal:    return "normal";
        case SkinForceType::Team:      return "team";
        case SkinForceType::Enemy:     return "enemy";
        case SkinForceType::TeamQuad:  return "team_quad";
        case SkinForceType::EnemyQuad: return "enemy_quad";
        case SkinForceType::TeamPent:  return "team_pent";
        case SkinForceType::EnemyPent: return "enemy_pent";
        case SkinForceType::TeamBoth:  return "team_both";
        case SkinForceType::EnemyBoth: return "enemy_both";
        default:                       return "unknown";
    }
}

[[nodiscard]] constexpr bool isTeamSkin(SkinForceType type) noexcept {
    return type == SkinForceType::Team ||
           type == SkinForceType::TeamQuad ||
           type == SkinForceType::TeamPent ||
           type == SkinForceType::TeamBoth;
}

[[nodiscard]] constexpr bool isEnemySkin(SkinForceType type) noexcept {
    return type == SkinForceType::Enemy ||
           type == SkinForceType::EnemyQuad ||
           type == SkinForceType::EnemyPent ||
           type == SkinForceType::EnemyBoth;
}

[[nodiscard]] constexpr bool hasPowerupSkin(SkinForceType type) noexcept {
    return type == SkinForceType::TeamQuad ||
           type == SkinForceType::EnemyQuad ||
           type == SkinForceType::TeamPent ||
           type == SkinForceType::EnemyPent ||
           type == SkinForceType::TeamBoth ||
           type == SkinForceType::EnemyBoth;
}

/**
 * @brief Skin forcing configuration
 */
struct SkinForceConfig {
    std::string teamSkin;
    std::string enemySkin;
    std::string teamQuadSkin;
    std::string enemyQuadSkin;
    std::string teamPentSkin;
    std::string enemyPentSkin;
    std::string teamBothSkin;
    std::string enemyBothSkin;
    
    /**
     * @brief Get skin name for specific type
     */
    [[nodiscard]] std::string_view getSkin(SkinForceType type) const noexcept {
        switch (type) {
            case SkinForceType::Team:      return teamSkin;
            case SkinForceType::Enemy:     return enemySkin;
            case SkinForceType::TeamQuad:  return teamQuadSkin;
            case SkinForceType::EnemyQuad: return enemyQuadSkin;
            case SkinForceType::TeamPent:  return teamPentSkin;
            case SkinForceType::EnemyPent: return enemyPentSkin;
            case SkinForceType::TeamBoth:  return teamBothSkin;
            case SkinForceType::EnemyBoth: return enemyBothSkin;
            default:                       return "";
        }
    }
    
    /**
     * @brief Determine skin type for player
     */
    [[nodiscard]] static SkinForceType determineSkinType(bool isTeammate, bool hasQuad, bool hasPent) noexcept {
        if (hasQuad && hasPent) {
            return isTeammate ? SkinForceType::TeamBoth : SkinForceType::EnemyBoth;
        }
        if (hasQuad) {
            return isTeammate ? SkinForceType::TeamQuad : SkinForceType::EnemyQuad;
        }
        if (hasPent) {
            return isTeammate ? SkinForceType::TeamPent : SkinForceType::EnemyPent;
        }
        return isTeammate ? SkinForceType::Team : SkinForceType::Enemy;
    }
};

//=============================================================================
// Color Forcing
//=============================================================================

/**
 * @brief Team/enemy color forcing
 */
struct ColorForceConfig {
    int teamTopColor = -1;      ///< -1 = don't force
    int teamBottomColor = -1;
    int enemyTopColor = -1;
    int enemyBottomColor = -1;
    
    [[nodiscard]] bool forcingTeamColors() const noexcept {
        return teamTopColor >= 0 || teamBottomColor >= 0;
    }
    
    [[nodiscard]] bool forcingEnemyColors() const noexcept {
        return enemyTopColor >= 0 || enemyBottomColor >= 0;
    }
    
    [[nodiscard]] bool forcingAnyColors() const noexcept {
        return forcingTeamColors() || forcingEnemyColors();
    }
};

//=============================================================================
// Teamplay Config
//=============================================================================

/**
 * @brief Complete teamplay configuration
 */
struct TeamplayConfig {
    // Skin forcing
    SkinForceConfig skinConfig;
    ColorForceConfig colorConfig;
    
    // Message parsing
    bool parseSay = true;
    int noFake = 0;             ///< cl_nofake level
    
    // Team identification
    std::string lockTeam;       ///< Forced team name
    bool teamLockSpecified = false;
    
    /**
     * @brief Check if skin forcing is enabled
     */
    [[nodiscard]] bool canForceSkins(uint16_t fpd) const noexcept {
        return !hasFPD(fpd, FPDFlag::NoForceSkin);
    }
    
    /**
     * @brief Check if color forcing is enabled
     */
    [[nodiscard]] bool canForceColors(uint16_t fpd) const noexcept {
        return !hasFPD(fpd, FPDFlag::NoForceColor);
    }
};

//=============================================================================
// Need Calculation
//=============================================================================

/**
 * @brief Player need assessment
 */
struct NeedAssessment {
    bool needsHealth = false;
    bool needsArmor = false;
    bool needsAmmo = false;
    bool needsWeapon = false;
    uint32_t needFlags = 0;
    
    [[nodiscard]] bool hasAnyNeeds() const noexcept {
        return needsHealth || needsArmor || needsAmmo || needsWeapon;
    }
    
    [[nodiscard]] int priority() const noexcept {
        // Higher = more urgent
        int p = 0;
        if (needsHealth) p += 4;
        if (needsArmor) p += 2;
        if (needsAmmo) p += 1;
        if (needsWeapon) p += 3;
        return p;
    }
};

/**
 * @brief Calculate need based on current stats
 */
[[nodiscard]] inline NeedAssessment calculateNeed(int health, int armor, 
                                                   int cells, int rockets,
                                                   int nails, int shells,
                                                   uint32_t weapons) noexcept {
    NeedAssessment need;
    
    // Health need
    if (health < 50) {
        need.needsHealth = true;
        need.needFlags |= itemFlagBit(ItemFlag::Health);
        if (health < 80) {
            need.needFlags |= itemFlagBit(ItemFlag::MegaHealth);
        }
    }
    
    // Armor need
    if (armor < 100) {
        need.needsArmor = true;
        need.needFlags |= item_masks::ARMOR;
    }
    
    // Weapon need (want RL or LG)
    bool hasRL = (weapons & itemFlagBit(ItemFlag::RocketLauncher)) != 0;
    bool hasLG = (weapons & itemFlagBit(ItemFlag::LightningGun)) != 0;
    if (!hasRL) {
        need.needsWeapon = true;
        need.needFlags |= itemFlagBit(ItemFlag::RocketLauncher);
    }
    if (!hasLG) {
        need.needsWeapon = true;
        need.needFlags |= itemFlagBit(ItemFlag::LightningGun);
    }
    
    // Ammo need
    if (hasRL && rockets < 5) {
        need.needsAmmo = true;
        need.needFlags |= itemFlagBit(ItemFlag::Rockets);
    }
    if (hasLG && cells < 20) {
        need.needsAmmo = true;
        need.needFlags |= itemFlagBit(ItemFlag::Cells);
    }
    
    return need;
}

} // namespace ezquake::teamplay
