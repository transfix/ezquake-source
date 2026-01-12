/*
 * ezQuake C++ Port - Server Browser Types
 *
 * Modern C++20 replacements for EX_browser.h structures.
 * Covers server browser, player lists, and source management.
 *
 * Original C sources:
 *   - EX_browser.h (server browser structures)
 *   - cl_slist.h (server list)
 *
 * Copyright (C) 2011 azazello and ezQuake team
 * Copyright (C) 2024 ezQuake Team
 */

#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <optional>

namespace ezquake {

//=============================================================================
// Constants
//=============================================================================

namespace browser_limits {

inline constexpr int MAX_UNBOUND = 200;
inline constexpr int MAX_SOURCES = 200;
inline constexpr int MAX_SERVERS = 1000;
inline constexpr int MAX_KEYS = 100;
inline constexpr int MAX_PLAYERS_PER_SERVER = 128;
inline constexpr int MAX_SERVER_LIST = 512;
inline constexpr int MAX_SOURCE_NAME = 25;
inline constexpr int MAX_PLAYER_NAME = 21;
inline constexpr int MAX_TEAM_NAME = 21;
inline constexpr int MAX_SKIN_NAME = 21;

// Column widths for display
inline constexpr int COL_PING = 3;
inline constexpr int COL_IP = 21;
inline constexpr int COL_NAME = 60;
inline constexpr int COL_MAP = 8;
inline constexpr int COL_GAMEDIR = 8;
inline constexpr int COL_PLAYERS = 5;
inline constexpr int COL_FRAGLIMIT = 2;
inline constexpr int COL_TIMELIMIT = 2;

} // namespace browser_limits

//=============================================================================
// ServerOccupancy - Server fullness state
//=============================================================================

/**
 * @brief Server occupancy state.
 * 
 * Original: server_occupancy
 */
enum class ServerOccupancy : std::uint8_t {
    Empty = 0,
    NotEmpty = 1,
    Full = 2
};

/**
 * @brief Check if server has any players.
 */
[[nodiscard]] constexpr bool hasPlayers(ServerOccupancy occ) noexcept {
    return occ != ServerOccupancy::Empty;
}

/**
 * @brief Check if server can accept more players.
 */
[[nodiscard]] constexpr bool hasRoom(ServerOccupancy occ) noexcept {
    return occ != ServerOccupancy::Full;
}

//=============================================================================
// SourceType - Server source types
//=============================================================================

/**
 * @brief Type of server source.
 * 
 * Original: sb_source_type_t
 */
enum class SourceType : std::uint8_t {
    Master = 0,     // Master server query
    File = 1,       // Local file list
    Url = 2,        // HTTP URL source
    Dummy = 3       // Placeholder/empty
};

/**
 * @brief Get name of source type.
 */
[[nodiscard]] constexpr std::string_view sourceTypeName(SourceType type) noexcept {
    switch (type) {
        case SourceType::Master: return "master";
        case SourceType::File:   return "file";
        case SourceType::Url:    return "url";
        case SourceType::Dummy:  return "dummy";
    }
    return "unknown";
}

/**
 * @brief Check if source type is network-based.
 */
[[nodiscard]] constexpr bool isNetworkSource(SourceType type) noexcept {
    return type == SourceType::Master || type == SourceType::Url;
}

//=============================================================================
// BrowserTrigger - Browser event triggers
//=============================================================================

/**
 * @brief Browser event trigger flags.
 * 
 * Original: SB_TRIGGER_* defines
 */
namespace BrowserTrigger {
    inline constexpr int None = 0;
    inline constexpr int RefreshDone = 1;
    inline constexpr int SourcesUpdated = 2;
    inline constexpr int NotifyPingTree = 4;
    inline constexpr int All = RefreshDone | SourcesUpdated | NotifyPingTree;
}

//=============================================================================
// BrowserPlayerInfo - Player on a server
//=============================================================================

/**
 * @brief Information about a player on a server.
 * 
 * Original: playerinfo
 */
struct BrowserPlayerInfo {
    int id = 0;
    int frags = 0;
    int time = 0;         // Time on server (minutes)
    int ping = 0;
    std::array<char, browser_limits::MAX_PLAYER_NAME> name{};
    std::array<char, browser_limits::MAX_SKIN_NAME> skin{};
    std::array<char, browser_limits::MAX_TEAM_NAME> team{};
    int topColor = 0;
    int bottomColor = 0;
    bool isSpectator = false;
    
    /**
     * @brief Get player name as string_view.
     */
    [[nodiscard]] std::string_view getName() const {
        return name.data();
    }
    
    /**
     * @brief Get team name as string_view.
     */
    [[nodiscard]] std::string_view getTeam() const {
        return team.data();
    }
    
    /**
     * @brief Set player name.
     */
    void setName(std::string_view n) {
        size_t len = std::min(n.size(), name.size() - 1);
        for (size_t i = 0; i < len; ++i) name[i] = n[i];
        name[len] = '\0';
    }
    
    /**
     * @brief Set team name.
     */
    void setTeam(std::string_view t) {
        size_t len = std::min(t.size(), team.size() - 1);
        for (size_t i = 0; i < len; ++i) team[i] = t[i];
        team[len] = '\0';
    }
    
    /**
     * @brief Check if player is active (not spectating).
     */
    [[nodiscard]] constexpr bool isActive() const noexcept {
        return !isSpectator;
    }
};

//=============================================================================
// ServerColumns - Display columns for server list
//=============================================================================

/**
 * @brief Pre-formatted display columns for server.
 * 
 * Original: columns
 */
struct ServerColumns {
    std::array<char, browser_limits::COL_PING + 1> ping{};
    std::array<char, browser_limits::COL_PING + 1> bestPing{};
    std::array<char, browser_limits::COL_IP + 1> ip{};
    std::array<char, browser_limits::COL_NAME + 1> name{};
    std::array<char, browser_limits::COL_MAP + 1> map{};
    std::array<char, browser_limits::COL_GAMEDIR + 1> gamedir{};
    std::array<char, browser_limits::COL_PLAYERS + 1> players{};
    std::array<char, browser_limits::COL_FRAGLIMIT + 1> fraglimit{};
    std::array<char, browser_limits::COL_TIMELIMIT + 1> timelimit{};
    
    /**
     * @brief Get server name as string_view.
     */
    [[nodiscard]] std::string_view getName() const {
        return name.data();
    }
    
    /**
     * @brief Get map name as string_view.
     */
    [[nodiscard]] std::string_view getMap() const {
        return map.data();
    }
};

//=============================================================================
// ServerKeyValue - Key-value pair for serverinfo
//=============================================================================

/**
 * @brief Key-value pair for server info.
 */
struct ServerKeyValue {
    std::string key;
    std::string value;
    
    ServerKeyValue() = default;
    ServerKeyValue(std::string_view k, std::string_view v)
        : key(k), value(v) {}
};

//=============================================================================
// NetAddress - Network address (simplified)
//=============================================================================

/**
 * @brief Simplified network address for server browser.
 */
struct NetAddress {
    std::array<std::uint8_t, 4> ip{};
    std::uint16_t port = 0;
    
    constexpr NetAddress() noexcept = default;
    constexpr NetAddress(std::uint8_t a, std::uint8_t b, std::uint8_t c, std::uint8_t d, std::uint16_t p) noexcept
        : ip{a, b, c, d}, port(p) {}
    
    /**
     * @brief Format as string (cached).
     */
    [[nodiscard]] std::string toString() const {
        return std::to_string(ip[0]) + "." + std::to_string(ip[1]) + "." +
               std::to_string(ip[2]) + "." + std::to_string(ip[3]) + ":" +
               std::to_string(port);
    }
    
    /**
     * @brief Compare addresses for equality.
     */
    [[nodiscard]] constexpr bool operator==(const NetAddress& other) const noexcept {
        return ip == other.ip && port == other.port;
    }
    
    /**
     * @brief Check if address is valid (non-zero).
     */
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return port != 0 && (ip[0] != 0 || ip[1] != 0 || ip[2] != 0 || ip[3] != 0);
    }
};

//=============================================================================
// ServerData - Full server information
//=============================================================================

/**
 * @brief Complete server data entry.
 * 
 * Original: server_data
 */
struct ServerData {
    bool passedFilters = false;
    int ping = 999;
    int bestPing = 999;
    NetAddress address{};
    ServerColumns display{};
    std::vector<ServerKeyValue> info;  // Serverinfo key-values
    std::vector<BrowserPlayerInfo> players;
    ServerOccupancy occupancy = ServerOccupancy::Empty;
    int spectatorCount = 0;
    bool isQizmo = false;       // Qizmo proxy
    bool isQwfwd = false;       // QWfwd proxy
    bool supportsTeams = false; // Server supports team per player
    
    /**
     * @brief Get value for a serverinfo key.
     */
    [[nodiscard]] std::optional<std::string_view> getValue(std::string_view key) const {
        for (const auto& kv : info) {
            if (kv.key == key) {
                return kv.value;
            }
        }
        return std::nullopt;
    }
    
    /**
     * @brief Set a serverinfo value.
     */
    void setValue(std::string_view key, std::string_view value) {
        for (auto& kv : info) {
            if (kv.key == key) {
                kv.value = value;
                return;
            }
        }
        info.emplace_back(key, value);
    }
    
    /**
     * @brief Get player count (excluding spectators).
     */
    [[nodiscard]] int playerCount() const noexcept {
        int count = 0;
        for (const auto& p : players) {
            if (!p.isSpectator) count++;
        }
        return count;
    }
    
    /**
     * @brief Get total count (players + spectators).
     */
    [[nodiscard]] int totalCount() const noexcept {
        return static_cast<int>(players.size());
    }
    
    /**
     * @brief Check if server is a proxy.
     */
    [[nodiscard]] constexpr bool isProxy() const noexcept {
        return isQizmo || isQwfwd;
    }
    
    /**
     * @brief Check if server is responding.
     */
    [[nodiscard]] constexpr bool isResponding() const noexcept {
        return ping < 999;
    }
    
    /**
     * @brief Reset server to default state.
     */
    void reset() {
        passedFilters = false;
        ping = 999;
        bestPing = 999;
        display = ServerColumns{};
        info.clear();
        players.clear();
        occupancy = ServerOccupancy::Empty;
        spectatorCount = 0;
        isQizmo = false;
        isQwfwd = false;
        supportsTeams = false;
    }
};

//=============================================================================
// SourceTime - Source update timestamp
//=============================================================================

/**
 * @brief Timestamp for source updates.
 */
struct SourceTime {
    std::uint16_t year = 0;
    std::uint8_t month = 0;
    std::uint8_t day = 0;
    std::uint8_t hour = 0;
    std::uint8_t minute = 0;
    std::uint8_t second = 0;
    
    /**
     * @brief Check if timestamp is set.
     */
    [[nodiscard]] constexpr bool isSet() const noexcept {
        return year != 0;
    }
    
    /**
     * @brief Format as string.
     */
    [[nodiscard]] std::string toString() const {
        if (!isSet()) return "never";
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%04u-%02u-%02u %02u:%02u:%02u",
                      year, month, day, hour, minute, second);
        return buf;
    }
};

//=============================================================================
// SourceData - Server source information
//=============================================================================

/**
 * @brief Server source entry.
 * 
 * Original: source_data
 */
struct SourceData {
    SourceType type = SourceType::Dummy;
    
    // Address storage (union in original)
    NetAddress masterAddress{};
    std::string filename;
    std::string url;
    
    std::array<char, browser_limits::MAX_SOURCE_NAME + 1> name{};
    SourceTime lastUpdate{};
    
    std::vector<ServerData> servers;
    
    bool isChecked = false;     // Use this source
    int uniqueOrder = 0;        // Order in file for sorting
    
    /**
     * @brief Get source name as string_view.
     */
    [[nodiscard]] std::string_view getName() const {
        return name.data();
    }
    
    /**
     * @brief Set source name.
     */
    void setName(std::string_view n) {
        size_t len = std::min(n.size(), name.size() - 1);
        for (size_t i = 0; i < len; ++i) name[i] = n[i];
        name[len] = '\0';
    }
    
    /**
     * @brief Get server count.
     */
    [[nodiscard]] size_t serverCount() const noexcept {
        return servers.size();
    }
    
    /**
     * @brief Check if source has any servers.
     */
    [[nodiscard]] bool hasServers() const noexcept {
        return !servers.empty();
    }
    
    /**
     * @brief Get address string based on type.
     */
    [[nodiscard]] std::string getAddressString() const {
        switch (type) {
            case SourceType::Master:
                return masterAddress.toString();
            case SourceType::File:
                return filename;
            case SourceType::Url:
                return url;
            default:
                return "";
        }
    }
    
    /**
     * @brief Clear all servers.
     */
    void clearServers() {
        servers.clear();
    }
};

//=============================================================================
// PlayerHost - Player with their server
//=============================================================================

/**
 * @brief Player entry with associated server reference.
 * 
 * Original: player_host
 */
struct PlayerHost {
    std::array<char, browser_limits::MAX_PLAYER_NAME> name{};
    std::size_t serverIndex = 0;  // Index into server list
    
    /**
     * @brief Get player name as string_view.
     */
    [[nodiscard]] std::string_view getName() const {
        return name.data();
    }
    
    /**
     * @brief Set player name.
     */
    void setName(std::string_view n) {
        size_t len = std::min(n.size(), name.size() - 1);
        for (size_t i = 0; i < len; ++i) name[i] = n[i];
        name[len] = '\0';
    }
};

//=============================================================================
// BrowserWindow - Browser window dimensions
//=============================================================================

/**
 * @brief Browser window position and size.
 * 
 * Original: serverbrowser_window_t
 */
struct BrowserWindow {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    
    constexpr BrowserWindow() noexcept = default;
    constexpr BrowserWindow(int x_, int y_, int w_, int h_) noexcept
        : x(x_), y(y_), width(w_), height(h_) {}
    
    /**
     * @brief Check if window has valid dimensions.
     */
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return width > 0 && height > 0;
    }
    
    /**
     * @brief Get area in pixels.
     */
    [[nodiscard]] constexpr int area() const noexcept {
        return width * height;
    }
};

//=============================================================================
// ServerEntry - Simple server entry (slist)
//=============================================================================

/**
 * @brief Simple server list entry.
 * 
 * Original: server_entry_t
 */
struct ServerEntry {
    std::string server;
    std::string description;
    
    ServerEntry() = default;
    ServerEntry(std::string_view s, std::string_view d)
        : server(s), description(d) {}
    
    /**
     * @brief Check if entry is valid (has server address).
     */
    [[nodiscard]] bool isValid() const noexcept {
        return !server.empty();
    }
};

//=============================================================================
// PingResult - Ping test result
//=============================================================================

/**
 * @brief Result of a ping test.
 */
struct PingResult {
    NetAddress address{};
    int ping = 999;
    bool success = false;
    
    constexpr PingResult() noexcept = default;
    constexpr PingResult(const NetAddress& addr, int p, bool ok) noexcept
        : address(addr), ping(p), success(ok) {}
    
    /**
     * @brief Check if ping was successful.
     */
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return success && ping < 999;
    }
};

//=============================================================================
// BrowserState - Browser UI state
//=============================================================================

/**
 * @brief Current state of the server browser.
 */
struct BrowserState {
    int serverListPosition = 0;
    int sourceListPosition = 0;
    int playerListPosition = 0;
    
    bool resortServers = false;
    bool rebuildServerList = false;
    bool resortAllPlayers = false;
    bool rebuildAllPlayers = false;
    bool resortSources = false;
    
    bool pingInProgress = false;
    double pingProgress = 0.0;
    
    bool sourcesUpdating = false;
    bool abortPing = false;
    bool fullSourceUpdate = false;
    
    int queuedTriggers = 0;
    
    /**
     * @brief Check if any work is pending.
     */
    [[nodiscard]] constexpr bool hasPendingWork() const noexcept {
        return resortServers || rebuildServerList || 
               resortAllPlayers || rebuildAllPlayers ||
               pingInProgress || sourcesUpdating;
    }
    
    /**
     * @brief Check if a specific trigger is queued.
     */
    [[nodiscard]] constexpr bool hasTrigger(int trigger) const noexcept {
        return (queuedTriggers & trigger) != 0;
    }
    
    /**
     * @brief Add a trigger to the queue.
     */
    constexpr void addTrigger(int trigger) noexcept {
        queuedTriggers |= trigger;
    }
    
    /**
     * @brief Clear a trigger from the queue.
     */
    constexpr void clearTrigger(int trigger) noexcept {
        queuedTriggers &= ~trigger;
    }
    
    /**
     * @brief Reset all state flags.
     */
    void reset() {
        serverListPosition = 0;
        sourceListPosition = 0;
        playerListPosition = 0;
        resortServers = false;
        rebuildServerList = false;
        resortAllPlayers = false;
        rebuildAllPlayers = false;
        resortSources = false;
        pingInProgress = false;
        pingProgress = 0.0;
        sourcesUpdating = false;
        abortPing = false;
        fullSourceUpdate = false;
        queuedTriggers = 0;
    }
};

//=============================================================================
// SortMode - Server sorting modes
//=============================================================================

/**
 * @brief Server list sorting mode.
 */
enum class ServerSortMode : std::uint8_t {
    None = 0,
    ByPing = 1,
    ByName = 2,
    ByMap = 3,
    ByPlayers = 4,
    ByGamedir = 5,
    ByAddress = 6
};

/**
 * @brief Player list sorting mode.
 */
enum class PlayerSortMode : std::uint8_t {
    None = 0,
    ByName = 1,
    ByFrags = 2,
    ByPing = 3,
    ByTime = 4,
    ByTeam = 5
};

//=============================================================================
// FilterSettings - Server filter configuration
//=============================================================================

/**
 * @brief Server filter settings.
 */
struct FilterSettings {
    bool hideEmpty = false;
    bool hideNotEmpty = false;
    bool hideFull = false;
    bool hideDead = false;
    bool hideHighPing = false;
    int pingLimit = 200;
    bool showProxies = true;
    
    /**
     * @brief Check if a server passes filters.
     */
    [[nodiscard]] bool passes(const ServerData& server) const noexcept {
        if (hideEmpty && server.occupancy == ServerOccupancy::Empty) return false;
        if (hideNotEmpty && server.occupancy == ServerOccupancy::NotEmpty) return false;
        if (hideFull && server.occupancy == ServerOccupancy::Full) return false;
        if (hideDead && !server.isResponding()) return false;
        if (hideHighPing && server.ping > pingLimit) return false;
        if (!showProxies && server.isProxy()) return false;
        return true;
    }
    
    /**
     * @brief Check if any filters are active.
     */
    [[nodiscard]] constexpr bool hasActiveFilters() const noexcept {
        return hideEmpty || hideNotEmpty || hideFull || 
               hideDead || hideHighPing || !showProxies;
    }
};

//=============================================================================
// BrowserStats - Browser statistics
//=============================================================================

/**
 * @brief Statistics about the server browser.
 */
struct BrowserStats {
    int totalServers = 0;
    int filteredServers = 0;
    int totalPlayers = 0;
    int totalSources = 0;
    int activeSources = 0;
    
    /**
     * @brief Calculate filter efficiency.
     */
    [[nodiscard]] constexpr float filterRatio() const noexcept {
        return totalServers > 0 ? 
            static_cast<float>(filteredServers) / static_cast<float>(totalServers) : 1.0f;
    }
};

} // namespace ezquake
