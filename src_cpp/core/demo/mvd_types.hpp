/**
 * @file mvd_types.hpp
 * @brief MVD (Multi-View Demo) and Autotrack type definitions
 *
 * This file contains C++20 implementations of types from mvd_utils_common.h
 * and mvd_autotrack.c for MVD playback analysis and automatic player tracking.
 */

#pragma once

#include <array>
#include <bitset>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include "../math/vec3.hpp"

namespace ezquake::mvd {

// ============================================================================
// Forward Declarations
// ============================================================================

struct MvdPlayerInfo;
struct MvdGameInfo;
struct MvdWeaponInfo;

// ============================================================================
// MVD Constants
// ============================================================================

namespace mvd_limits {
    inline constexpr int MAX_CLIENTS = 32;
    inline constexpr int MAX_MEGAS_PER_PLAYER = 4;
    inline constexpr int AMMO_TYPES = 4;
    inline constexpr int MAX_RUNS = 512;
    inline constexpr int PL_VALUES_COUNT = 14;
    inline constexpr int MVD_INFO_TYPES = 15;
    inline constexpr int MAX_WEAPONS = 8;
    inline constexpr int MAX_HOSTNAME_LENGTH = 1024;
    inline constexpr int MAX_TEAMNAME_LENGTH = 1024;
    inline constexpr int MAX_MAPNAME_LENGTH = 1024;
}

// ============================================================================
// MVD Item/Weapon Info Index
// ============================================================================

/**
 * @brief Info type indices for MVD stats tracking
 *
 * Based on mvd_utils_common.h defines.
 * Used to index into item stats arrays.
 */
enum class MvdInfoType : int {
    Axe = 0,
    Shotgun = 1,
    SuperShotgun = 2,
    Nailgun = 3,
    SuperNailgun = 4,
    GrenadeLauncher = 5,
    RocketLauncher = 6,
    LightningGun = 7,
    Ring = 8,
    Quad = 9,
    Pent = 10,
    GreenArmor = 11,
    YellowArmor = 12,
    RedArmor = 13,
    MegaHealth = 14,

    Count = 15
};

/**
 * @brief Check if info type represents a weapon
 */
[[nodiscard]] constexpr bool isWeapon(MvdInfoType type) noexcept {
    return static_cast<int>(type) >= static_cast<int>(MvdInfoType::SuperShotgun) &&
           static_cast<int>(type) <= static_cast<int>(MvdInfoType::LightningGun);
}

/**
 * @brief Check if info type represents a powerup
 */
[[nodiscard]] constexpr bool isPowerup(MvdInfoType type) noexcept {
    return type == MvdInfoType::Quad ||
           type == MvdInfoType::Pent ||
           type == MvdInfoType::Ring;
}

/**
 * @brief Check if info type represents armor
 */
[[nodiscard]] constexpr bool isArmor(MvdInfoType type) noexcept {
    return type == MvdInfoType::GreenArmor ||
           type == MvdInfoType::YellowArmor ||
           type == MvdInfoType::RedArmor;
}

// ============================================================================
// Game Type
// ============================================================================

/**
 * @brief MVD game type enumeration
 *
 * Based on gt_* defines from mvd_utils_common.h.
 */
enum class GameType : int {
    OneVsOne = 0,   // gt_1on1
    TwoVsTwo = 1,   // gt_2on2
    ThreeVsThree = 2, // gt_3on3
    FourVsFour = 3, // gt_4on4
    Unknown = 4,

    Count = 5
};

/**
 * @brief Get the display name for a game type
 */
[[nodiscard]] constexpr std::string_view gameTypeName(GameType type) noexcept {
    switch (type) {
        case GameType::OneVsOne: return "1on1";
        case GameType::TwoVsTwo: return "2on2";
        case GameType::ThreeVsThree: return "3on3";
        case GameType::FourVsFour: return "4on4";
        case GameType::Unknown: return "unknown";
        default: return "unknown";
    }
}

/**
 * @brief Get player count per team for game type
 */
[[nodiscard]] constexpr int playersPerTeam(GameType type) noexcept {
    switch (type) {
        case GameType::OneVsOne: return 1;
        case GameType::TwoVsTwo: return 2;
        case GameType::ThreeVsThree: return 3;
        case GameType::FourVsFour: return 4;
        default: return 0;
    }
}

// ============================================================================
// MVD Event Types
// ============================================================================

/**
 * @brief MVD event type enumeration
 *
 * Based on mvd_event_t.type values from mvd_utils_common.h.
 */
enum class MvdEventType : int {
    Spawn = 0,
    Death = 1,
    Kill = 2,
    TeamKill = 3,
    ItemTook = 4,
    PowerupEnd = 5,

    Count = 6
};

/**
 * @brief Get display name for event type
 */
[[nodiscard]] constexpr std::string_view eventTypeName(MvdEventType type) noexcept {
    switch (type) {
        case MvdEventType::Spawn: return "spawn";
        case MvdEventType::Death: return "death";
        case MvdEventType::Kill: return "kill";
        case MvdEventType::TeamKill: return "teamkill";
        case MvdEventType::ItemTook: return "took";
        case MvdEventType::PowerupEnd: return "powerup_end";
        default: return "unknown";
    }
}

// ============================================================================
// AddClock Mode
// ============================================================================

/**
 * @brief When to add respawn clock to weapon/item display
 */
enum class AddClockMode : int {
    Never = 0,
    Always = 1,
    DeathMatchMode1 = 2  // DMM1 - standard deathmatch
};

// ============================================================================
// Kill Statistics
// ============================================================================

/**
 * @brief Per-weapon kill statistics
 *
 * Based on mvd_ks_w_t from mvd_utils_common.h.
 */
struct WeaponKillStats {
    int kills = 0;
    int teamKills = 0;
    int lastKills = 0;

    [[nodiscard]] constexpr int totalKills() const noexcept {
        return kills + teamKills;
    }

    [[nodiscard]] constexpr bool hasKills() const noexcept {
        return kills > 0 || teamKills > 0;
    }

    constexpr void reset() noexcept {
        kills = 0;
        teamKills = 0;
        lastKills = 0;
    }
};

/**
 * @brief Complete kill statistics with powerup combinations
 *
 * Based on mvd_ks_t from mvd_utils_common.h.
 * Tracks kills for each weapon and powerup combination.
 */
struct KillStats {
    std::array<WeaponKillStats, 8> normal{};  // axe through lg
    std::array<WeaponKillStats, 7> quad{};    // with quad
    std::array<WeaponKillStats, 7> pent{};    // with pent
    std::array<WeaponKillStats, 7> ring{};    // with ring
    std::array<WeaponKillStats, 7> quadPent{};
    std::array<WeaponKillStats, 7> quadRing{};
    std::array<WeaponKillStats, 7> pentRing{};
    std::array<WeaponKillStats, 7> quadPentRing{};

    /**
     * @brief Get total kills across all weapons
     */
    [[nodiscard]] int totalKills() const noexcept {
        int total = 0;
        for (const auto& ws : normal) {
            total += ws.kills;
        }
        return total;
    }

    /**
     * @brief Get total teamkills across all weapons
     */
    [[nodiscard]] int totalTeamKills() const noexcept {
        int total = 0;
        for (const auto& ws : normal) {
            total += ws.teamKills;
        }
        return total;
    }

    /**
     * @brief Reset all kill statistics
     */
    void reset() noexcept {
        for (auto& ws : normal) ws.reset();
        for (auto& ws : quad) ws.reset();
        for (auto& ws : pent) ws.reset();
        for (auto& ws : ring) ws.reset();
        for (auto& ws : quadPent) ws.reset();
        for (auto& ws : quadRing) ws.reset();
        for (auto& ws : pentRing) ws.reset();
        for (auto& ws : quadPentRing) ws.reset();
    }
};

// ============================================================================
// Death/Alive Statistics
// ============================================================================

/**
 * @brief Death and alive time statistics
 *
 * Based on mvd_ds_t from mvd_utils_common.h.
 */
struct DeathAliveStats {
    bool isDead = false;
    int deathCount = 0;
    double aliveTimeStart = 0.0;
    double aliveTime = 0.0;

    /**
     * @brief Mark player as spawned (alive)
     */
    void onSpawn(double time) noexcept {
        isDead = false;
        aliveTimeStart = time;
    }

    /**
     * @brief Mark player as dead
     */
    void onDeath(double time) noexcept {
        if (!isDead) {
            isDead = true;
            deathCount++;
            aliveTime += time - aliveTimeStart;
        }
    }

    /**
     * @brief Get average life duration
     */
    [[nodiscard]] double averageLifeTime() const noexcept {
        if (deathCount == 0) return 0.0;
        return aliveTime / deathCount;
    }

    void reset() noexcept {
        isDead = false;
        deathCount = 0;
        aliveTimeStart = 0.0;
        aliveTime = 0.0;
    }
};

// ============================================================================
// Run Statistics
// ============================================================================

/**
 * @brief Single run statistics (spawn to death)
 *
 * Based on mvd_runs_t from mvd_utils_common.h.
 */
struct RunStats {
    double time = 0.0;
    int frags = 0;
    int teamFrags = 0;
    double startTime = 0.0;

    [[nodiscard]] double duration() const noexcept {
        return time - startTime;
    }

    [[nodiscard]] double fragsPerSecond() const noexcept {
        double dur = duration();
        if (dur <= 0.0) return 0.0;
        return static_cast<double>(frags) / dur;
    }

    void reset() noexcept {
        time = 0.0;
        frags = 0;
        teamFrags = 0;
        startTime = 0.0;
    }
};

/**
 * @brief Average run statistics
 *
 * Based on mvd_avgruns_t from mvd_utils_common.h.
 */
struct AverageRunStats {
    double avgTime = 0.0;
    double avgFrags = 0.0;
    double avgTeamFrags = 0.0;

    void reset() noexcept {
        avgTime = 0.0;
        avgFrags = 0.0;
        avgTeamFrags = 0.0;
    }
};

/**
 * @brief Complete average run statistics with powerup breakdown
 *
 * Based on mvd_avgruns_all_t from mvd_utils_common.h.
 */
struct AverageRunStatsAll {
    AverageRunStats all{};
    AverageRunStats quad{};   // not implemented in original
    AverageRunStats pent{};   // not implemented in original
    AverageRunStats ring{};   // not implemented in original

    void reset() noexcept {
        all.reset();
        quad.reset();
        pent.reset();
        ring.reset();
    }
};

// ============================================================================
// Powerup/Item Statistics
// ============================================================================

/**
 * @brief Powerup/item possession statistics
 *
 * Based on mvd_pw_t from mvd_utils_common.h.
 */
struct ItemPossessionStats {
    bool has = false;
    double startTime = 0.0;
    double currentTime = 0.0;
    int count = 0;
    int lost = 0;
    int mention = 0;
    std::vector<RunStats> runs{};
    int currentRun = 0;

    /**
     * @brief Total time held
     */
    [[nodiscard]] double totalTime() const noexcept {
        return currentTime;
    }

    /**
     * @brief Average hold time per pickup
     */
    [[nodiscard]] double averageHoldTime() const noexcept {
        if (count == 0) return 0.0;
        return currentTime / count;
    }

    /**
     * @brief Pickup to loss ratio
     */
    [[nodiscard]] double retentionRate() const noexcept {
        if (count == 0) return 0.0;
        return 1.0 - (static_cast<double>(lost) / count);
    }

    void reset() noexcept {
        has = false;
        startTime = 0.0;
        currentTime = 0.0;
        count = 0;
        lost = 0;
        mention = 0;
        runs.clear();
        currentRun = 0;
    }
};

// ============================================================================
// MVD Event
// ============================================================================

/**
 * @brief Single MVD event record
 *
 * Based on mvd_event_t from mvd_utils_common.h.
 */
struct MvdEvent {
    MvdEventType type = MvdEventType::Spawn;
    double time = 0.0;
    math::Vec3 location{};
    int info = 0;   // item on tooks, lfw on deaths/kills
    int killedId = -1;  // userid of killed player

    /**
     * @brief Check if event is a frag event
     */
    [[nodiscard]] constexpr bool isFragEvent() const noexcept {
        return type == MvdEventType::Kill || type == MvdEventType::TeamKill;
    }

    /**
     * @brief Check if event involves an item
     */
    [[nodiscard]] constexpr bool isItemEvent() const noexcept {
        return type == MvdEventType::ItemTook || type == MvdEventType::PowerupEnd;
    }
};

// ============================================================================
// MVD Player Info
// ============================================================================

/**
 * @brief Complete MVD player statistics
 *
 * Based on mvd_info_t from mvd_utils_common.h.
 */
struct MvdPlayerStats {
    float value = 0.0f;
    int lastFiredWeapon = 0;
    DeathAliveStats deathAliveStats{};
    std::array<ItemPossessionStats, mvd_limits::MVD_INFO_TYPES> itemStats{};
    std::array<uint16_t, mvd_limits::AMMO_TYPES> ammoStats{};
    std::vector<RunStats> runs{};
    AverageRunStatsAll runStats{};
    KillStats killStats{};
    int spawnTelefrags = 0;
    int teamSpawnTelefrags = 0;
    int teamFrags = 0;
    int lastFrags = 0;
    int currentRun = 0;
    int firstRun = 0;
    bool initialized = false;

    /**
     * @brief Calculate total frags (including team frags adjustment)
     */
    [[nodiscard]] int totalFrags() const noexcept {
        return killStats.totalKills();
    }

    /**
     * @brief Calculate efficiency (kills / deaths)
     */
    [[nodiscard]] double efficiency() const noexcept {
        int deaths = deathAliveStats.deathCount;
        if (deaths == 0) return 0.0;
        return static_cast<double>(killStats.totalKills()) / deaths;
    }

    /**
     * @brief Reset all statistics
     */
    void reset() noexcept {
        value = 0.0f;
        lastFiredWeapon = 0;
        deathAliveStats.reset();
        for (auto& item : itemStats) item.reset();
        for (auto& ammo : ammoStats) ammo = 0;
        runs.clear();
        runStats.reset();
        killStats.reset();
        spawnTelefrags = 0;
        teamSpawnTelefrags = 0;
        teamFrags = 0;
        lastFrags = 0;
        currentRun = 0;
        firstRun = 0;
        initialized = false;
    }
};

/**
 * @brief Complete MVD tracking info for a player
 *
 * Based on mvd_new_info_t from mvd_utils_common.h.
 */
struct MvdPlayerInfo {
    int id = -1;
    float value = 0.0f;
    std::vector<MvdEvent> events{};
    int lastWeaponFired = 0;
    MvdPlayerStats stats{};
    std::array<math::Vec3, mvd_limits::MAX_MEGAS_PER_PLAYER> megaLocations{};

    /**
     * @brief Check if player has any recorded events
     */
    [[nodiscard]] bool hasEvents() const noexcept {
        return !events.empty();
    }

    /**
     * @brief Get last event of specific type
     */
    [[nodiscard]] std::optional<MvdEvent> lastEventOfType(MvdEventType type) const noexcept {
        for (auto it = events.rbegin(); it != events.rend(); ++it) {
            if (it->type == type) {
                return *it;
            }
        }
        return std::nullopt;
    }

    void reset() noexcept {
        id = -1;
        value = 0.0f;
        events.clear();
        lastWeaponFired = 0;
        stats.reset();
        for (auto& loc : megaLocations) {
            loc = math::Vec3{};
        }
    }
};

// ============================================================================
// MVD Game Info
// ============================================================================

/**
 * @brief MVD game/match information
 *
 * Based on mvd_cg_info_s from mvd_utils_common.h.
 */
struct MvdGameInfo {
    std::string mapName{};
    std::string team1{};
    std::string team2{};
    std::string hostname{};
    GameType gameType = GameType::Unknown;
    int timeLimit = 0;
    int playerCount = 0;
    int deathmatch = 0;

    /**
     * @brief Check if game is a team game
     */
    [[nodiscard]] bool isTeamGame() const noexcept {
        return gameType != GameType::OneVsOne;
    }

    /**
     * @brief Get expected player count based on game type
     */
    [[nodiscard]] int expectedPlayers() const noexcept {
        return playersPerTeam(gameType) * 2;
    }

    void reset() noexcept {
        mapName.clear();
        team1.clear();
        team2.clear();
        hostname.clear();
        gameType = GameType::Unknown;
        timeLimit = 0;
        playerCount = 0;
        deathmatch = 0;
    }
};

// ============================================================================
// MVD Weapon Info
// ============================================================================

/**
 * @brief Weapon display information for MVD
 *
 * Based on mvd_wp_info_t from mvd_utils_common.h.
 */
struct MvdWeaponInfo {
    MvdInfoType id = MvdInfoType::Axe;
    std::string name{};
    uint32_t itemFlag = 0;
    std::string coloredName{};
    int modelHint = 0;
    int skinNumber = 0;
    uint8_t colorR = 255;
    uint8_t colorG = 255;
    uint8_t colorB = 255;
    std::string colorString{};
    std::string coloredPackName{};
    AddClockMode addClock = AddClockMode::Never;
    bool showHeld = false;

    /**
     * @brief Get RGB color as 32-bit value
     */
    [[nodiscard]] constexpr uint32_t colorRGB() const noexcept {
        return (static_cast<uint32_t>(colorR) << 16) |
               (static_cast<uint32_t>(colorG) << 8) |
               static_cast<uint32_t>(colorB);
    }
};

// ============================================================================
// Autotrack Configuration
// ============================================================================

/**
 * @brief Autotrack value weights
 *
 * Based on pl_values array from mvd_autotrack.c.
 */
struct AutotrackWeights {
    float axe = 1.0f;
    float shotgun = 2.0f;
    float superShotgun = 3.0f;
    float nailgun = 2.0f;
    float superNailgun = 3.0f;
    float grenadeLauncher = 5.0f;
    float rocketLauncher = 8.0f;
    float lightningGun = 8.0f;
    float greenArmor = 1.0f;
    float yellowArmor = 2.0f;
    float redArmor = 3.0f;
    float ring = 0.0f;
    float quad = 0.0f;
    float pent = 0.0f;

    /**
     * @brief Get weapon value by index (0=axe through 7=lg)
     */
    [[nodiscard]] float weaponValue(int index) const noexcept {
        switch (index) {
            case 0: return axe;
            case 1: return shotgun;
            case 2: return superShotgun;
            case 3: return nailgun;
            case 4: return superNailgun;
            case 5: return grenadeLauncher;
            case 6: return rocketLauncher;
            case 7: return lightningGun;
            default: return 0.0f;
        }
    }

    /**
     * @brief Get armor value by type (0=green, 1=yellow, 2=red)
     */
    [[nodiscard]] float armorValue(int type) const noexcept {
        switch (type) {
            case 0: return greenArmor;
            case 1: return yellowArmor;
            case 2: return redArmor;
            default: return 0.0f;
        }
    }

    /**
     * @brief Parse values from string (space-separated floats)
     */
    bool parseFromString(std::string_view values);

    /**
     * @brief Create default 1v1 weights
     */
    static AutotrackWeights create1on1() noexcept {
        return AutotrackWeights{
            1, 2, 3, 2, 3, 5, 8, 8,
            1, 2, 3,
            0, 0, 0
        };
    }

    /**
     * @brief Create default 2v2 weights
     */
    static AutotrackWeights create2on2() noexcept {
        return AutotrackWeights{
            1, 2, 3, 2, 3, 5, 8, 8,
            1, 2, 3,
            500, 900, 1000
        };
    }

    /**
     * @brief Create default 4v4 weights
     */
    static AutotrackWeights create4on4() noexcept {
        return AutotrackWeights{
            1, 2, 4, 2, 4, 6, 10, 10,
            1, 2, 3,
            500, 900, 1000
        };
    }
};

/**
 * @brief Autotrack configuration for a game type
 */
struct AutotrackConfig {
    std::string formula{"%a * %A + 50 * %W + %p + %f"};
    AutotrackWeights weights{};
    bool lockTeam = false;
    bool instant = false;

    /**
     * @brief Create default config for game type
     */
    static AutotrackConfig createForGameType(GameType type) noexcept {
        AutotrackConfig config;
        switch (type) {
            case GameType::OneVsOne:
                config.weights = AutotrackWeights::create1on1();
                break;
            case GameType::TwoVsTwo:
                config.weights = AutotrackWeights::create2on2();
                break;
            case GameType::FourVsFour:
            case GameType::ThreeVsThree:
            default:
                config.weights = AutotrackWeights::create4on4();
                break;
        }
        return config;
    }
};

/**
 * @brief Multitrack configuration for multiple camera views
 */
struct MultitrackConfig {
    std::array<std::string, 4> formulas{{
        "%f",   // frags
        "%W",   // best weapon
        "%h",   // health
        "%A"    // armor
    }};
    std::array<AutotrackWeights, 4> weights{};
    std::array<int, 4> trackedPlayerIds{-1, -1, -1, -1};

    /**
     * @brief Reset tracked players
     */
    void resetTracking() noexcept {
        for (auto& id : trackedPlayerIds) {
            id = -1;
        }
    }
};

// ============================================================================
// Item Tracking
// ============================================================================

/**
 * @brief Simple item took/lost tracking
 *
 * Based on items_t from mvd_utils_common.h.
 */
struct ItemTrackingStats {
    int took = 0;
    int lost = 0;

    [[nodiscard]] constexpr int net() const noexcept {
        return took - lost;
    }

    void reset() noexcept {
        took = 0;
        lost = 0;
    }
};

// ============================================================================
// MVD State Manager
// ============================================================================

/**
 * @brief Complete MVD playback state
 */
struct MvdState {
    std::array<MvdPlayerInfo, mvd_limits::MAX_CLIENTS> players{};
    MvdGameInfo gameInfo{};
    AutotrackConfig autotrackConfig{};
    MultitrackConfig multitrackConfig{};
    int currentTrackTarget = -1;
    int lastTrackTarget = -1;
    bool autotrackEnabled = false;

    /**
     * @brief Get player by ID
     */
    [[nodiscard]] MvdPlayerInfo* getPlayer(int id) noexcept {
        if (id < 0 || id >= mvd_limits::MAX_CLIENTS) {
            return nullptr;
        }
        return &players[static_cast<size_t>(id)];
    }

    [[nodiscard]] const MvdPlayerInfo* getPlayer(int id) const noexcept {
        if (id < 0 || id >= mvd_limits::MAX_CLIENTS) {
            return nullptr;
        }
        return &players[static_cast<size_t>(id)];
    }

    /**
     * @brief Find best player to track based on current config
     */
    [[nodiscard]] int findBestTrackTarget() const noexcept;

    /**
     * @brief Reset all state
     */
    void reset() noexcept {
        for (auto& player : players) {
            player.reset();
        }
        gameInfo.reset();
        autotrackConfig = AutotrackConfig{};
        multitrackConfig = MultitrackConfig{};
        currentTrackTarget = -1;
        lastTrackTarget = -1;
        autotrackEnabled = false;
    }
};

// ============================================================================
// Factory Functions
// ============================================================================

/**
 * @brief Create default weapon info table
 */
[[nodiscard]] std::array<MvdWeaponInfo, mvd_limits::MVD_INFO_TYPES> createDefaultWeaponInfo();

/**
 * @brief Create game type info from player count
 */
[[nodiscard]] constexpr GameType gameTypeFromPlayerCount(int count) noexcept {
    if (count <= 2) return GameType::OneVsOne;
    if (count <= 4) return GameType::TwoVsTwo;
    if (count <= 6) return GameType::ThreeVsThree;
    if (count <= 8) return GameType::FourVsFour;
    return GameType::Unknown;
}

} // namespace ezquake::mvd
