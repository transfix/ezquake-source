/**
 * @file fragstats_types.hpp
 * @brief Frag statistics and fragfile type definitions
 *
 * This file contains C++20 implementations of types from fragstats.c
 * for frag/kill tracking, weapon classification, and fragfile parsing.
 */

#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace ezquake::fragstats {

// ============================================================================
// FragStats Constants
// ============================================================================

namespace fragstats_limits {
    inline constexpr int MAX_FRAG_DEFINITIONS = 256;
    inline constexpr int MAX_WEAPON_CLASSES = 32;
    inline constexpr int MAX_FRAGMSG_LENGTH = 256;
    inline constexpr int MAX_KEYWORD_LENGTH = 64;
    inline constexpr int MAX_AUTHOR_LENGTH = 128;
    inline constexpr int MAX_DESCRIPTION_LENGTH = 512;
}

// ============================================================================
// Message Type
// ============================================================================

/**
 * @brief Frag message type enumeration
 *
 * Based on msgtype_t from fragstats.c.
 */
enum class FragMessageType : int {
    Fragged = 0,        // mt_fragged - killed by someone
    Frags = 1,          // mt_frags - you fragged someone
    TeamKills = 2,      // mt_tkills - you team killed
    TeamKilled = 3,     // mt_tkilled - team killed by someone

    Death = 4,          // mt_death - generic death
    Suicide = 5,        // mt_suicide - self kill
    Frag = 6,           // mt_frag - specific frag event
    TeamKill = 7,       // mt_tkill - specific team kill
    TeamKilledUnknown = 8, // mt_tkilled_unk - team killed by unknown
    FlagTouch = 9,      // mt_flagtouch - flag pickup
    FlagDrop = 10,      // mt_flagdrop - flag dropped
    FlagCapture = 11,   // mt_flagcap - flag captured

    Count = 12
};

/**
 * @brief Get display name for message type
 */
[[nodiscard]] constexpr std::string_view messageTypeName(FragMessageType type) noexcept {
    switch (type) {
        case FragMessageType::Fragged: return "fragged";
        case FragMessageType::Frags: return "frags";
        case FragMessageType::TeamKills: return "teamkills";
        case FragMessageType::TeamKilled: return "teamkilled";
        case FragMessageType::Death: return "death";
        case FragMessageType::Suicide: return "suicide";
        case FragMessageType::Frag: return "frag";
        case FragMessageType::TeamKill: return "teamkill";
        case FragMessageType::TeamKilledUnknown: return "teamkilled_unknown";
        case FragMessageType::FlagTouch: return "flagtouch";
        case FragMessageType::FlagDrop: return "flagdrop";
        case FragMessageType::FlagCapture: return "flagcap";
        default: return "unknown";
    }
}

/**
 * @brief Check if message type is a kill event
 */
[[nodiscard]] constexpr bool isKillEvent(FragMessageType type) noexcept {
    return type == FragMessageType::Frags ||
           type == FragMessageType::Frag ||
           type == FragMessageType::TeamKill ||
           type == FragMessageType::TeamKills;
}

/**
 * @brief Check if message type is a death event
 */
[[nodiscard]] constexpr bool isDeathEvent(FragMessageType type) noexcept {
    return type == FragMessageType::Fragged ||
           type == FragMessageType::Death ||
           type == FragMessageType::Suicide ||
           type == FragMessageType::TeamKilled ||
           type == FragMessageType::TeamKilledUnknown;
}

/**
 * @brief Check if message type is a flag event
 */
[[nodiscard]] constexpr bool isFlagEvent(FragMessageType type) noexcept {
    return type == FragMessageType::FlagTouch ||
           type == FragMessageType::FlagDrop ||
           type == FragMessageType::FlagCapture;
}

// ============================================================================
// Weapon Class
// ============================================================================

/**
 * @brief Standard weapon class indices
 */
enum class WeaponClass : int {
    Unknown = 0,
    Axe = 1,
    Shotgun = 2,
    SuperShotgun = 3,
    Nailgun = 4,
    SuperNailgun = 5,
    GrenadeLauncher = 6,
    RocketLauncher = 7,
    LightningGun = 8,

    // Extended
    Telefrag = 9,
    Squish = 10,
    Lava = 11,
    Slime = 12,
    Drown = 13,
    Fall = 14,

    Count = 15
};

/**
 * @brief Get weapon class name
 */
[[nodiscard]] constexpr std::string_view weaponClassName(WeaponClass wclass) noexcept {
    switch (wclass) {
        case WeaponClass::Unknown: return "unknown";
        case WeaponClass::Axe: return "axe";
        case WeaponClass::Shotgun: return "sg";
        case WeaponClass::SuperShotgun: return "ssg";
        case WeaponClass::Nailgun: return "ng";
        case WeaponClass::SuperNailgun: return "sng";
        case WeaponClass::GrenadeLauncher: return "gl";
        case WeaponClass::RocketLauncher: return "rl";
        case WeaponClass::LightningGun: return "lg";
        case WeaponClass::Telefrag: return "tele";
        case WeaponClass::Squish: return "squish";
        case WeaponClass::Lava: return "lava";
        case WeaponClass::Slime: return "slime";
        case WeaponClass::Drown: return "drown";
        case WeaponClass::Fall: return "fall";
        default: return "unknown";
    }
}

/**
 * @brief Get weapon class full name
 */
[[nodiscard]] constexpr std::string_view weaponClassFullName(WeaponClass wclass) noexcept {
    switch (wclass) {
        case WeaponClass::Unknown: return "Unknown";
        case WeaponClass::Axe: return "Axe";
        case WeaponClass::Shotgun: return "Shotgun";
        case WeaponClass::SuperShotgun: return "Super Shotgun";
        case WeaponClass::Nailgun: return "Nailgun";
        case WeaponClass::SuperNailgun: return "Super Nailgun";
        case WeaponClass::GrenadeLauncher: return "Grenade Launcher";
        case WeaponClass::RocketLauncher: return "Rocket Launcher";
        case WeaponClass::LightningGun: return "Lightning Gun";
        case WeaponClass::Telefrag: return "Telefrag";
        case WeaponClass::Squish: return "Squished";
        case WeaponClass::Lava: return "Lava";
        case WeaponClass::Slime: return "Slime";
        case WeaponClass::Drown: return "Drowned";
        case WeaponClass::Fall: return "Falling";
        default: return "Unknown";
    }
}

/**
 * @brief Check if weapon class is an actual weapon
 */
[[nodiscard]] constexpr bool isActualWeapon(WeaponClass wclass) noexcept {
    return wclass >= WeaponClass::Axe && wclass <= WeaponClass::LightningGun;
}

// ============================================================================
// Weapon Class Info
// ============================================================================

/**
 * @brief Weapon class definition
 *
 * Based on wclass_t from fragstats.c.
 */
struct WeaponClassInfo {
    std::string keyword{};
    std::string name{};
    std::string shortName{};
    std::string imageName{};
    std::string colorName{};
    int colorNameRgbOffset = 0;
    bool colorNameSkip = false;

    /**
     * @brief Check if weapon class info is valid
     */
    [[nodiscard]] bool isValid() const noexcept {
        return !keyword.empty() && !name.empty();
    }

    /**
     * @brief Get display name (short or full)
     */
    [[nodiscard]] const std::string& displayName(bool useShort = false) const noexcept {
        if (useShort && !shortName.empty()) {
            return shortName;
        }
        return name;
    }
};

// ============================================================================
// Frag Message Definition
// ============================================================================

/**
 * @brief Single frag message pattern
 *
 * Based on fragmsg_t from fragstats.c.
 */
struct FragMessage {
    std::string message1{};     // First part of message pattern
    std::string message2{};     // Second part (after player name)
    FragMessageType type = FragMessageType::Frag;
    int weaponClassIndex = 0;   // Index into weapon classes

    /**
     * @brief Check if message matches a death string
     */
    [[nodiscard]] bool matches(std::string_view deathMessage) const noexcept;

    /**
     * @brief Check if this is a valid message definition
     */
    [[nodiscard]] bool isValid() const noexcept {
        return !message1.empty();
    }
};

// ============================================================================
// Fragfile Definition
// ============================================================================

/**
 * @brief Complete fragfile definition
 *
 * Based on fragdef_t from fragstats.c.
 */
struct FragfileDefinition {
    bool active = false;
    bool flagAlerts = false;

    int version = 0;
    std::string gamedir{};

    std::string title{};
    std::string description{};
    std::string author{};
    std::string email{};
    std::string webpage{};

    std::vector<FragMessage> messages{};
    std::vector<WeaponClassInfo> weaponClasses{};

    /**
     * @brief Find message matching death string
     */
    [[nodiscard]] std::optional<size_t> findMessage(std::string_view deathMessage) const noexcept;

    /**
     * @brief Get weapon class by index
     */
    [[nodiscard]] const WeaponClassInfo* getWeaponClass(int index) const noexcept {
        if (index < 0 || static_cast<size_t>(index) >= weaponClasses.size()) {
            return nullptr;
        }
        return &weaponClasses[static_cast<size_t>(index)];
    }

    /**
     * @brief Find weapon class by keyword
     */
    [[nodiscard]] std::optional<int> findWeaponClass(std::string_view keyword) const noexcept {
        for (size_t i = 0; i < weaponClasses.size(); ++i) {
            if (weaponClasses[i].keyword == keyword) {
                return static_cast<int>(i);
            }
        }
        return std::nullopt;
    }

    /**
     * @brief Reset to empty state
     */
    void reset() noexcept {
        active = false;
        flagAlerts = false;
        version = 0;
        gamedir.clear();
        title.clear();
        description.clear();
        author.clear();
        email.clear();
        webpage.clear();
        messages.clear();
        weaponClasses.clear();

        // Add default unknown weapon class
        WeaponClassInfo unknown;
        unknown.name = "Unknown";
        weaponClasses.push_back(std::move(unknown));
    }
};

// ============================================================================
// Frag Statistics
// ============================================================================

/**
 * @brief Per-weapon frag statistics
 */
struct WeaponFragStats {
    int kills = 0;
    int deaths = 0;
    int teamKills = 0;
    int teamDeaths = 0;
    int suicides = 0;

    /**
     * @brief Get total frags (kills - teamkills - suicides)
     */
    [[nodiscard]] constexpr int netFrags() const noexcept {
        return kills - teamKills - suicides;
    }

    /**
     * @brief Get kill/death ratio
     */
    [[nodiscard]] float killDeathRatio() const noexcept {
        if (deaths == 0) return static_cast<float>(kills);
        return static_cast<float>(kills) / static_cast<float>(deaths);
    }

    /**
     * @brief Reset all stats
     */
    void reset() noexcept {
        kills = 0;
        deaths = 0;
        teamKills = 0;
        teamDeaths = 0;
        suicides = 0;
    }

    /**
     * @brief Add another weapon stats
     */
    void add(const WeaponFragStats& other) noexcept {
        kills += other.kills;
        deaths += other.deaths;
        teamKills += other.teamKills;
        teamDeaths += other.teamDeaths;
        suicides += other.suicides;
    }
};

/**
 * @brief Complete player frag statistics
 */
struct PlayerFragStats {
    std::array<WeaponFragStats, static_cast<size_t>(WeaponClass::Count)> byWeapon{};
    int flagTouches = 0;
    int flagDrops = 0;
    int flagCaptures = 0;
    int telefrags = 0;
    int environmentDeaths = 0;  // lava, slime, fall, drown

    /**
     * @brief Get total kills across all weapons
     */
    [[nodiscard]] int totalKills() const noexcept {
        int total = 0;
        for (const auto& ws : byWeapon) {
            total += ws.kills;
        }
        return total;
    }

    /**
     * @brief Get total deaths across all weapons
     */
    [[nodiscard]] int totalDeaths() const noexcept {
        int total = 0;
        for (const auto& ws : byWeapon) {
            total += ws.deaths;
        }
        return total + environmentDeaths;
    }

    /**
     * @brief Get total suicides
     */
    [[nodiscard]] int totalSuicides() const noexcept {
        int total = 0;
        for (const auto& ws : byWeapon) {
            total += ws.suicides;
        }
        return total;
    }

    /**
     * @brief Get total team kills
     */
    [[nodiscard]] int totalTeamKills() const noexcept {
        int total = 0;
        for (const auto& ws : byWeapon) {
            total += ws.teamKills;
        }
        return total;
    }

    /**
     * @brief Get combined stats for all weapons
     */
    [[nodiscard]] WeaponFragStats combinedStats() const noexcept {
        WeaponFragStats combined;
        for (const auto& ws : byWeapon) {
            combined.add(ws);
        }
        return combined;
    }

    /**
     * @brief Get stats for a specific weapon
     */
    [[nodiscard]] const WeaponFragStats& getWeaponStats(WeaponClass wclass) const noexcept {
        return byWeapon[static_cast<size_t>(wclass)];
    }

    [[nodiscard]] WeaponFragStats& getWeaponStats(WeaponClass wclass) noexcept {
        return byWeapon[static_cast<size_t>(wclass)];
    }

    /**
     * @brief Record a kill
     */
    void recordKill(WeaponClass weapon, bool isTeamKill = false) noexcept {
        auto& ws = getWeaponStats(weapon);
        if (isTeamKill) {
            ws.teamKills++;
        } else {
            ws.kills++;
        }
    }

    /**
     * @brief Record a death
     */
    void recordDeath(WeaponClass weapon, bool isTeamDeath = false, bool isSuicide = false) noexcept {
        auto& ws = getWeaponStats(weapon);
        if (isSuicide) {
            ws.suicides++;
        } else if (isTeamDeath) {
            ws.teamDeaths++;
        } else {
            ws.deaths++;
        }
    }

    /**
     * @brief Reset all stats
     */
    void reset() noexcept {
        for (auto& ws : byWeapon) {
            ws.reset();
        }
        flagTouches = 0;
        flagDrops = 0;
        flagCaptures = 0;
        telefrags = 0;
        environmentDeaths = 0;
    }
};

// ============================================================================
// Quad Damage Tracking
// ============================================================================

/**
 * @brief Quad damage weapon string markers
 */
namespace quad_markers {
    inline constexpr std::string_view SG_QUAD = "&c06fQ&r-sg";
    inline constexpr std::string_view SSG_QUAD = "&c06fQ&r-ssg";
    inline constexpr std::string_view SNG_QUAD = "&c06fQ&r-sng";
    inline constexpr std::string_view RL_QUAD = "&c06fQ&r-rl";
    inline constexpr std::string_view LG_QUAD = "&c06fQ&r-lg";
    inline constexpr std::string_view QUAD_PREFIX = "&c06fQ&r-";
}

/**
 * @brief Check if weapon string indicates quad damage
 */
[[nodiscard]] inline bool hasQuadMarker(std::string_view weaponStr) noexcept {
    return weaponStr.find(quad_markers::QUAD_PREFIX) != std::string_view::npos;
}

// ============================================================================
// Frag Event
// ============================================================================

/**
 * @brief Single frag event record
 */
struct FragEvent {
    double time = 0.0;
    int killerPlayerId = -1;
    int victimPlayerId = -1;
    FragMessageType type = FragMessageType::Frag;
    WeaponClass weapon = WeaponClass::Unknown;
    bool hadQuad = false;
    bool wasTeamKill = false;
    std::string message{};

    /**
     * @brief Check if this is a self-kill
     */
    [[nodiscard]] constexpr bool isSuicide() const noexcept {
        return type == FragMessageType::Suicide ||
               (killerPlayerId == victimPlayerId && killerPlayerId >= 0);
    }

    /**
     * @brief Check if this is a valid event
     */
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return killerPlayerId >= 0 || victimPlayerId >= 0;
    }
};

// ============================================================================
// Frag Parser State
// ============================================================================

/**
 * @brief Frag message parser state
 */
struct FragParserState {
    FragfileDefinition* fragfile = nullptr;
    bool enabled = true;
    bool showMessages = true;
    bool useImages = false;

    /**
     * @brief Parse a death message
     */
    [[nodiscard]] std::optional<FragEvent> parse(
        std::string_view message,
        double time,
        int killerPlayerId,
        int victimPlayerId
    ) const noexcept;

    /**
     * @brief Check if parser is ready
     */
    [[nodiscard]] bool isReady() const noexcept {
        return enabled && fragfile != nullptr && fragfile->active;
    }
};

// ============================================================================
// Frag Log
// ============================================================================

/**
 * @brief Frag event log for match history
 */
struct FragLog {
    std::vector<FragEvent> events{};
    size_t maxEvents = 1000;

    /**
     * @brief Add event to log
     */
    void addEvent(FragEvent event) noexcept {
        if (events.size() >= maxEvents && !events.empty()) {
            events.erase(events.begin());
        }
        events.push_back(std::move(event));
    }

    /**
     * @brief Get events for a specific player
     */
    [[nodiscard]] std::vector<const FragEvent*> getEventsForPlayer(int playerId) const noexcept {
        std::vector<const FragEvent*> result;
        for (const auto& event : events) {
            if (event.killerPlayerId == playerId || event.victimPlayerId == playerId) {
                result.push_back(&event);
            }
        }
        return result;
    }

    /**
     * @brief Get kill events only
     */
    [[nodiscard]] std::vector<const FragEvent*> getKillEvents() const noexcept {
        std::vector<const FragEvent*> result;
        for (const auto& event : events) {
            if (isKillEvent(event.type)) {
                result.push_back(&event);
            }
        }
        return result;
    }

    /**
     * @brief Clear all events
     */
    void clear() noexcept {
        events.clear();
    }

    /**
     * @brief Get event count
     */
    [[nodiscard]] size_t size() const noexcept {
        return events.size();
    }
};

// ============================================================================
// Factory Functions
// ============================================================================

/**
 * @brief Create default weapon class list
 */
[[nodiscard]] inline std::vector<WeaponClassInfo> createDefaultWeaponClasses() {
    std::vector<WeaponClassInfo> classes;
    classes.reserve(static_cast<size_t>(WeaponClass::Count));

    for (int i = 0; i < static_cast<int>(WeaponClass::Count); ++i) {
        WeaponClass wc = static_cast<WeaponClass>(i);
        WeaponClassInfo info;
        info.keyword = std::string(weaponClassName(wc));
        info.name = std::string(weaponClassFullName(wc));
        info.shortName = std::string(weaponClassName(wc));
        classes.push_back(std::move(info));
    }

    return classes;
}

} // namespace ezquake::fragstats
