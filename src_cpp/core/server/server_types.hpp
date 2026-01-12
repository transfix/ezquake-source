/**
 * @file server_types.hpp
 * @brief Server type definitions for ezQuake C++ core
 * 
 * This file contains type-safe equivalents for server structures
 * from server.h, sv_main.c, sv_init.c, sv_user.c, sv_phys.c.
 */

#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include "core/math/vec3.hpp"

namespace ezquake::server {

// Use Vec3 from ezquake::math namespace
using Vec3 = ezquake::math::Vec3;

// =============================================================================
// Server State (from server.h)
// =============================================================================

/**
 * @brief Server state enumeration
 * 
 * Matches server_state_t from server.h:
 * - ss_dead: no map loaded
 * - ss_loading: spawning level edicts
 * - ss_active: actively running
 */
enum class ServerState : uint8_t {
    Dead = 0,      // ss_dead - no map loaded
    Loading = 1,   // ss_loading - spawning level edicts
    Active = 2     // ss_active - actively running
};

[[nodiscard]] constexpr std::string_view serverStateName(ServerState state) noexcept {
    switch (state) {
        case ServerState::Dead: return "dead";
        case ServerState::Loading: return "loading";
        case ServerState::Active: return "active";
        default: return "unknown";
    }
}

[[nodiscard]] constexpr bool isServerRunning(ServerState state) noexcept {
    return state == ServerState::Active;
}

[[nodiscard]] constexpr bool canPrecache(ServerState state) noexcept {
    // Precache commands only valid during load
    return state == ServerState::Loading;
}

// =============================================================================
// Client State (from server.h)
// =============================================================================

/**
 * @brief Server-side client state enumeration
 * 
 * Matches sv_client_state_t from server.h
 */
enum class ClientState : uint8_t {
    Free = 0,          // cs_free - can be reused for a new connection
    Zombie = 1,        // cs_zombie - disconnected, don't reuse yet
    Preconnected = 2,  // cs_preconnected - assigned but login not settled
    Connected = 3,     // cs_connected - assigned to client_t, not in game
    Spawned = 4        // cs_spawned - fully in game
};

[[nodiscard]] constexpr std::string_view clientStateName(ClientState state) noexcept {
    switch (state) {
        case ClientState::Free: return "free";
        case ClientState::Zombie: return "zombie";
        case ClientState::Preconnected: return "preconnected";
        case ClientState::Connected: return "connected";
        case ClientState::Spawned: return "spawned";
        default: return "unknown";
    }
}

[[nodiscard]] constexpr bool isClientActive(ClientState state) noexcept {
    return state == ClientState::Spawned;
}

[[nodiscard]] constexpr bool isClientConnected(ClientState state) noexcept {
    return state >= ClientState::Connected;
}

[[nodiscard]] constexpr bool canReuse(ClientState state) noexcept {
    return state == ClientState::Free;
}

// =============================================================================
// Move Types (from server.h)
// =============================================================================

/**
 * @brief Entity move type enumeration
 * 
 * Matches MOVETYPE_* defines from server.h.
 * These control how physics treats the entity.
 */
enum class MoveType : uint8_t {
    None = 0,          // MOVETYPE_NONE - never moves
    AngleNoClip = 1,   // MOVETYPE_ANGLENOCLIP
    AngleClip = 2,     // MOVETYPE_ANGLECLIP
    Walk = 3,          // MOVETYPE_WALK - gravity applies
    Step = 4,          // MOVETYPE_STEP - gravity, special edge handling (monsters)
    Fly = 5,           // MOVETYPE_FLY
    Toss = 6,          // MOVETYPE_TOSS - gravity applies
    Push = 7,          // MOVETYPE_PUSH - no clip to world, push and crush
    NoClip = 8,        // MOVETYPE_NOCLIP
    FlyMissile = 9,    // MOVETYPE_FLYMISSILE - extra size to monsters
    Bounce = 10,       // MOVETYPE_BOUNCE
    Lock = 15          // MOVETYPE_LOCK - server controls view angles
};

[[nodiscard]] constexpr std::string_view moveTypeName(MoveType type) noexcept {
    switch (type) {
        case MoveType::None: return "none";
        case MoveType::AngleNoClip: return "anglenoclip";
        case MoveType::AngleClip: return "angleclip";
        case MoveType::Walk: return "walk";
        case MoveType::Step: return "step";
        case MoveType::Fly: return "fly";
        case MoveType::Toss: return "toss";
        case MoveType::Push: return "push";
        case MoveType::NoClip: return "noclip";
        case MoveType::FlyMissile: return "flymissile";
        case MoveType::Bounce: return "bounce";
        case MoveType::Lock: return "lock";
        default: return "unknown";
    }
}

[[nodiscard]] constexpr bool affectedByGravity(MoveType type) noexcept {
    return type == MoveType::Walk || 
           type == MoveType::Step ||
           type == MoveType::Toss ||
           type == MoveType::Bounce;
}

[[nodiscard]] constexpr bool isProjectileType(MoveType type) noexcept {
    return type == MoveType::FlyMissile || type == MoveType::Toss || type == MoveType::Bounce;
}

// =============================================================================
// Solid Types (from server.h)
// =============================================================================

/**
 * @brief Entity solid type enumeration
 * 
 * Matches SOLID_* defines from server.h.
 * Controls collision behavior.
 */
enum class SolidType : uint8_t {
    Not = 0,       // SOLID_NOT - no interaction with other objects
    Trigger = 1,   // SOLID_TRIGGER - touch on edge, not blocking
    BBox = 2,      // SOLID_BBOX - touch on edge, block
    SlideBox = 3,  // SOLID_SLIDEBOX - touch on edge, not onground
    BSP = 4        // SOLID_BSP - bsp clip, touch on edge, block
};

[[nodiscard]] constexpr std::string_view solidTypeName(SolidType type) noexcept {
    switch (type) {
        case SolidType::Not: return "not";
        case SolidType::Trigger: return "trigger";
        case SolidType::BBox: return "bbox";
        case SolidType::SlideBox: return "slidebox";
        case SolidType::BSP: return "bsp";
        default: return "unknown";
    }
}

[[nodiscard]] constexpr bool isBlocking(SolidType type) noexcept {
    return type == SolidType::BBox || type == SolidType::BSP;
}

[[nodiscard]] constexpr bool isTouchable(SolidType type) noexcept {
    return type != SolidType::Not;
}

// =============================================================================
// Entity Flags (from server.h)
// =============================================================================

/**
 * @brief Entity flags bitmask
 * 
 * Matches FL_* defines from server.h.
 */
namespace EntityFlags {
    constexpr uint32_t Fly = 1 << 0;             // FL_FLY
    constexpr uint32_t Swim = 1 << 1;            // FL_SWIM
    constexpr uint32_t Glimpse = 1 << 2;         // FL_GLIMPSE
    constexpr uint32_t Client = 1 << 3;          // FL_CLIENT
    constexpr uint32_t InWater = 1 << 4;         // FL_INWATER
    constexpr uint32_t Monster = 1 << 5;         // FL_MONSTER
    constexpr uint32_t GodMode = 1 << 6;         // FL_GODMODE
    constexpr uint32_t NoTarget = 1 << 7;        // FL_NOTARGET
    constexpr uint32_t Item = 1 << 8;            // FL_ITEM
    constexpr uint32_t OnGround = 1 << 9;        // FL_ONGROUND
    constexpr uint32_t PartialGround = 1 << 10;  // FL_PARTIALGROUND
    constexpr uint32_t WaterJump = 1 << 11;      // FL_WATERJUMP
    constexpr uint32_t LaggedMove = 1 << 16;     // FL_LAGGEDMOVE (sv_antilag)
    
    [[nodiscard]] constexpr bool hasFlag(uint32_t flags, uint32_t flag) noexcept {
        return (flags & flag) != 0;
    }
    
    [[nodiscard]] constexpr bool isPlayer(uint32_t flags) noexcept {
        return hasFlag(flags, Client);
    }
    
    [[nodiscard]] constexpr bool isMonster(uint32_t flags) noexcept {
        return hasFlag(flags, Monster);
    }
    
    [[nodiscard]] constexpr bool canFly(uint32_t flags) noexcept {
        return hasFlag(flags, Fly | Swim);
    }
}

// =============================================================================
// Spawn Flags (from server.h)
// =============================================================================

/**
 * @brief Entity spawn flags bitmask
 * 
 * Matches SPAWNFLAG_* defines from server.h.
 */
namespace SpawnFlags {
    constexpr uint32_t NotEasy = 256;            // SPAWNFLAG_NOT_EASY
    constexpr uint32_t NotMedium = 512;          // SPAWNFLAG_NOT_MEDIUM
    constexpr uint32_t NotHard = 1024;           // SPAWNFLAG_NOT_HARD
    constexpr uint32_t NotDeathmatch = 2048;     // SPAWNFLAG_NOT_DEATHMATCH
    
    /**
     * @brief Check if entity should spawn based on skill level
     */
    [[nodiscard]] constexpr bool shouldSpawn(uint32_t spawnflags, int skill, bool deathmatch) noexcept {
        if (deathmatch && (spawnflags & NotDeathmatch))
            return false;
        if (skill == 0 && (spawnflags & NotEasy))
            return false;
        if (skill == 1 && (spawnflags & NotMedium))
            return false;
        if (skill >= 2 && (spawnflags & NotHard))
            return false;
        return true;
    }
}

// =============================================================================
// Damage Types (from server.h)
// =============================================================================

/**
 * @brief Damage type enumeration
 * 
 * Matches DAMAGE_* defines from server.h (edict->deadflag values).
 */
enum class DamageType : uint8_t {
    No = 0,    // DAMAGE_NO - cannot be damaged
    Yes = 1,   // DAMAGE_YES - can be damaged
    Aim = 2    // DAMAGE_AIM - special aiming behavior
};

[[nodiscard]] constexpr std::string_view damageTypeName(DamageType type) noexcept {
    switch (type) {
        case DamageType::No: return "no";
        case DamageType::Yes: return "yes";
        case DamageType::Aim: return "aim";
        default: return "unknown";
    }
}

[[nodiscard]] constexpr bool canBeDamaged(DamageType type) noexcept {
    return type != DamageType::No;
}

// =============================================================================
// Multicast Types (from server.h)
// =============================================================================

/**
 * @brief Multicast destination enumeration
 * 
 * Matches MULTICAST_* defines from server.h.
 * Controls which clients receive a multicast message.
 */
enum class MulticastType : uint8_t {
    All = 0,           // MULTICAST_ALL - all clients
    PHS = 1,           // MULTICAST_PHS - potentially hearable set
    PVS = 2,           // MULTICAST_PVS - potentially visible set
    AllReliable = 3,   // MULTICAST_ALL_R - all, reliable
    PHSReliable = 4,   // MULTICAST_PHS_R - PHS, reliable
    PVSReliable = 5,   // MULTICAST_PVS_R - PVS, reliable
    KTX1Ext = 6,       // MULTICAST_KTX1_EXT - only KTX1 protocol extension
    MVDHidden = 7      // MULTICAST_MVD_HIDDEN - MVD stream only
};

[[nodiscard]] constexpr std::string_view multicastTypeName(MulticastType type) noexcept {
    switch (type) {
        case MulticastType::All: return "all";
        case MulticastType::PHS: return "phs";
        case MulticastType::PVS: return "pvs";
        case MulticastType::AllReliable: return "all_reliable";
        case MulticastType::PHSReliable: return "phs_reliable";
        case MulticastType::PVSReliable: return "pvs_reliable";
        case MulticastType::KTX1Ext: return "ktx1_ext";
        case MulticastType::MVDHidden: return "mvd_hidden";
        default: return "unknown";
    }
}

[[nodiscard]] constexpr bool isReliable(MulticastType type) noexcept {
    return type == MulticastType::AllReliable ||
           type == MulticastType::PHSReliable ||
           type == MulticastType::PVSReliable;
}

[[nodiscard]] constexpr bool usesVisibility(MulticastType type) noexcept {
    return type == MulticastType::PVS || 
           type == MulticastType::PVSReliable ||
           type == MulticastType::PHS ||
           type == MulticastType::PHSReliable;
}

// =============================================================================
// IP Filter Types (from sv_main.c)
// =============================================================================

/**
 * @brief IP filter type enumeration
 * 
 * Matches ipfiltertype_t from sv_main.c.
 */
enum class IpFilterType : uint8_t {
    Ban = 0,   // ipft_ban
    Safe = 1   // ipft_safe
};

[[nodiscard]] constexpr std::string_view ipFilterTypeName(IpFilterType type) noexcept {
    switch (type) {
        case IpFilterType::Ban: return "ban";
        case IpFilterType::Safe: return "safe";
        default: return "unknown";
    }
}

/**
 * @brief Penalty filter type enumeration
 * 
 * Matches filtertype_t from sv_main.c.
 */
enum class PenaltyType : uint8_t {
    Mute = 0,  // ft_mute
    Cuff = 1   // ft_cuff
};

[[nodiscard]] constexpr std::string_view penaltyTypeName(PenaltyType type) noexcept {
    switch (type) {
        case PenaltyType::Mute: return "mute";
        case PenaltyType::Cuff: return "cuff";
        default: return "unknown";
    }
}

// =============================================================================
// Redirect Types (from sv_send.c)
// =============================================================================

/**
 * @brief Redirect destination enumeration
 * 
 * Matches redirect_t from server.h.
 */
enum class RedirectType : uint8_t {
    None = 0,    // RD_NONE
    Client = 1,  // RD_CLIENT
    Packet = 2,  // RD_PACKET
    Mod = 3      // RD_MOD
};

[[nodiscard]] constexpr std::string_view redirectTypeName(RedirectType type) noexcept {
    switch (type) {
        case RedirectType::None: return "none";
        case RedirectType::Client: return "client";
        case RedirectType::Packet: return "packet";
        case RedirectType::Mod: return "mod";
        default: return "unknown";
    }
}

// =============================================================================
// Demo Destination Types (from server.h)
// =============================================================================

/**
 * @brief Demo destination type enumeration
 * 
 * Matches desttype_t from server.h.
 */
enum class DemoDestType : uint8_t {
    None = 0,          // DEST_NONE
    File = 1,          // DEST_FILE
    BufferedFile = 2,  // DEST_BUFFEREDFILE
    Stream = 3         // DEST_STREAM
};

[[nodiscard]] constexpr std::string_view demoDestTypeName(DemoDestType type) noexcept {
    switch (type) {
        case DemoDestType::None: return "none";
        case DemoDestType::File: return "file";
        case DemoDestType::BufferedFile: return "buffered_file";
        case DemoDestType::Stream: return "stream";
        default: return "unknown";
    }
}

[[nodiscard]] constexpr bool isFileBased(DemoDestType type) noexcept {
    return type == DemoDestType::File || type == DemoDestType::BufferedFile;
}

// =============================================================================
// Voice Chat Types (from server.h, FTE_PEXT2_VOICECHAT)
// =============================================================================

/**
 * @brief Voice target enumeration
 * 
 * Matches voice_target enum from server.h.
 */
enum class VoiceTarget : uint8_t {
    Team = 0,         // VT_TEAM
    All = 1,          // VT_ALL
    NonMuted = 2,     // VT_NONMUTED
    PlayerSlot0 = 3   // VT_PLAYERSLOT0 (base for player+N)
};

[[nodiscard]] constexpr std::string_view voiceTargetName(VoiceTarget target) noexcept {
    switch (target) {
        case VoiceTarget::Team: return "team";
        case VoiceTarget::All: return "all";
        case VoiceTarget::NonMuted: return "nonmuted";
        case VoiceTarget::PlayerSlot0: return "player";
        default: return "unknown";
    }
}

// =============================================================================
// Spec Print Flags (from server.h)
// =============================================================================

/**
 * @brief Spectator print flags
 * 
 * Matches SPECPRINT_* defines from server.h.
 */
namespace SpecPrint {
    constexpr int CenterPrint = 0x1;  // SPECPRINT_CENTERPRINT
    constexpr int Sprint = 0x2;       // SPECPRINT_SPRINT
    constexpr int StuffCmd = 0x4;     // SPECPRINT_STUFFCMD
}

// =============================================================================
// Server Constants
// =============================================================================

namespace server_constants {
    // Max counts from server.h
    constexpr int MAX_MASTERS = 8;
    constexpr int MAX_SIGNON_BUFFERS = 16;
    constexpr int MAX_CLIENTS = 32;  // Standard QW max
    constexpr int MAX_MODELS = 512;
    constexpr int MAX_SOUNDS = 256;
    constexpr int MAX_LIGHTSTYLES = 64;
    constexpr int MAX_VWEP_MODELS = 32;
    constexpr int MAX_STATIC_ENTITIES = 512;
    constexpr int MAX_EDICTS = 2048;
    
    // Signon buffer
    constexpr int MAX_DATAGRAM = 1400;
    constexpr int MAX_MSGLEN = 65536;
    constexpr int MSG_BUF_SIZE = 65536;
    constexpr int MAP_NAME_LEN = 64;
    constexpr int MAX_QPATH = 64;
    constexpr int MODEL_NAME_LEN = 6;
    
    // Networking
    constexpr int MAX_CHALLENGES = 1024;
    constexpr int MAX_DELAYED_PACKETS = 1024;
    constexpr int UPDATE_BACKUP = 64;
    
    // Physics step size
    constexpr float STEPSIZE = 18.0f;
    
    // Spawn params
    constexpr int NUM_SPAWN_PARMS = 16;
    
    // Antilag
    constexpr int MAX_ANTILAG_POSITIONS = 128;
    
    // Back buffers
    constexpr int MAX_BACK_BUFFERS = 128;
    constexpr int MAX_STUFFTEXT = 256;
    
    // Login
    constexpr int CLIENT_LOGIN_LEN = 16;
    constexpr int CLIENT_NAME_LEN = 32;
    constexpr int LOGIN_CHALLENGE_LENGTH = 128;
    constexpr int LOGIN_FLAG_LENGTH = 8;
    constexpr int LOGIN_MIN_RETRY_TIME = 5;
    
    // Server flags
    constexpr int SVF_SPEC_ONFULL = 1 << 0;
    constexpr int SVF_NO_SPEC_ONFULL = 1 << 1;
    
    // Localinfo
    constexpr int MAX_LOCALINFOS = 10000;
    constexpr int LOCALINFO_MAPS_LIST_START = 1000;
    constexpr int LOCALINFO_MAPS_LIST_END = 4999;
    
    // Redirect
    constexpr int MAX_REDIRECTMESSAGES = 128;
    constexpr int OUTPUTBUF_SIZE = 8000;
    
    // Penalty filters
    constexpr int MAX_PENFILTERS = 512;
    
    // QTV/Demo
    constexpr int MAX_PROXY_INBUFFER = 4096;
    constexpr int MAX_MVD_SIZE = 65536;
}

// =============================================================================
// IP Filter Structure
// =============================================================================

/**
 * @brief IP filter entry
 * 
 * Matches ipfilter_t from sv_main.c.
 */
struct IpFilter {
    uint32_t mask = 0;
    uint32_t compare = 0;
    int level = 0;
    double expireTime = 0.0;  // for ban expiration
    IpFilterType type = IpFilterType::Ban;
    
    [[nodiscard]] constexpr bool matches(uint32_t ip) const noexcept {
        return (ip & mask) == compare;
    }
    
    [[nodiscard]] constexpr bool isExpired(double currentTime) const noexcept {
        return expireTime > 0.0 && currentTime >= expireTime;
    }
    
    [[nodiscard]] constexpr bool isBan() const noexcept {
        return type == IpFilterType::Ban;
    }
    
    [[nodiscard]] constexpr bool isSafe() const noexcept {
        return type == IpFilterType::Safe;
    }
};

// =============================================================================
// Penalty Filter Structure
// =============================================================================

/**
 * @brief Penalty filter entry
 * 
 * Matches penfilter_t from sv_main.c.
 */
struct PenaltyFilter {
    std::array<uint8_t, 4> ip = {0, 0, 0, 0};
    double expireTime = 0.0;
    PenaltyType type = PenaltyType::Mute;
    
    [[nodiscard]] constexpr bool isExpired(double currentTime) const noexcept {
        return currentTime >= expireTime;
    }
    
    [[nodiscard]] constexpr bool isMute() const noexcept {
        return type == PenaltyType::Mute;
    }
    
    [[nodiscard]] constexpr bool isCuff() const noexcept {
        return type == PenaltyType::Cuff;
    }
    
    [[nodiscard]] bool matchesIp(const std::array<uint8_t, 4>& other) const noexcept {
        return ip == other;
    }
};

// =============================================================================
// Challenge Structure
// =============================================================================

/**
 * @brief Connection challenge entry
 * 
 * Matches challenge_t from server.h.
 */
struct Challenge {
    std::array<uint8_t, 4> ip = {0, 0, 0, 0};
    uint16_t port = 0;
    int challenge = 0;
    int time = 0;  // time challenge was generated
    
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return challenge != 0;
    }
    
    [[nodiscard]] constexpr bool isExpired(int currentTime, int timeout) const noexcept {
        return currentTime - time > timeout;
    }
    
    void clear() noexcept {
        ip = {0, 0, 0, 0};
        port = 0;
        challenge = 0;
        time = 0;
    }
};

// =============================================================================
// Antilag Position Structure
// =============================================================================

/**
 * @brief Antilag position history entry
 * 
 * Matches antilag_position_t from server.h.
 */
struct AntilagPosition {
    double localtime = 0.0;
    Vec3 origin;
    
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return localtime > 0.0;
    }
};

// =============================================================================
// Client Frame Structure (simplified)
// =============================================================================

/**
 * @brief Per-frame client data
 * 
 * Simplified from client_frame_t in server.h.
 */
struct ClientFrame {
    double sentTime = 0.0;
    float pingTime = 0.0f;
    double svTime = 0.0;  // sv_antilag timestamp
    
    [[nodiscard]] constexpr bool wasSent() const noexcept {
        return sentTime > 0.0;
    }
    
    void clear() noexcept {
        sentTime = 0.0;
        pingTime = 0.0f;
        svTime = 0.0;
    }
};

// =============================================================================
// Demo Client Structure
// =============================================================================

/**
 * @brief Per-client demo frame data
 * 
 * Matches demo_client_t from server.h.
 */
struct DemoClientFrame {
    int parsecount = 0;
    Vec3 origin;
    Vec3 angles;
    int weaponframe = 0;
    int frame = 0;
    int skinnum = 0;
    int model = 0;
    int effects = 0;
    int flags = 0;
    bool fixangle = false;
    float sec = 0.0f;
    
    void clear() noexcept {
        parsecount = 0;
        origin = Vec3{};
        angles = Vec3{};
        weaponframe = 0;
        frame = 0;
        skinnum = 0;
        model = 0;
        effects = 0;
        flags = 0;
        fixangle = false;
        sec = 0.0f;
    }
};

// =============================================================================
// Server Statistics Structure
// =============================================================================

/**
 * @brief Server performance statistics
 * 
 * Matches svstats_t from server.h.
 */
struct ServerStats {
    double active = 0.0;
    double idle = 0.0;
    double demo = 0.0;
    int count = 0;
    int packets = 0;
    
    // Latched values for display
    double latchedActive = 0.0;
    double latchedIdle = 0.0;
    double latchedDemo = 0.0;
    int latchedPackets = 0;
    
    void latch() noexcept {
        latchedActive = active;
        latchedIdle = idle;
        latchedDemo = demo;
        latchedPackets = packets;
    }
    
    void reset() noexcept {
        active = 0.0;
        idle = 0.0;
        demo = 0.0;
        count = 0;
        packets = 0;
    }
    
    [[nodiscard]] double totalTime() const noexcept {
        return active + idle + demo;
    }
    
    [[nodiscard]] double activePercent() const noexcept {
        double total = totalTime();
        return total > 0.0 ? (active / total) * 100.0 : 0.0;
    }
};

// =============================================================================
// Date/Time Structure (from sv_main.c)
// =============================================================================

/**
 * @brief Date/time structure for logging
 * 
 * Matches date_t from server.h.
 */
struct DateTime {
    int sec = 0;
    int min = 0;
    int hour = 0;
    int day = 1;
    int mon = 1;
    int year = 1970;
    
    [[nodiscard]] std::string format(std::string_view fmt = "%Y-%m-%d %H:%M:%S") const;
    
    static DateTime now();
};

// Simple format implementation (can be expanded)
inline std::string DateTime::format(std::string_view /* fmt */) const {
    char buf[128];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
             year, mon, day, hour, min, sec);
    return std::string(buf);
}

// =============================================================================
// Entity Baseline Info
// =============================================================================

/**
 * @brief Entity baseline state for delta compression
 * 
 * Simplified from entity_state_t baseline fields.
 */
struct EntityBaseline {
    int number = 0;
    Vec3 origin;
    Vec3 angles;
    int frame = 0;
    int skinnum = 0;
    int colormap = 0;
    int modelindex = 0;
    uint8_t trans = 0;
    std::array<uint8_t, 3> colourmod = {255, 255, 255};
    
    void clear() noexcept {
        number = 0;
        origin = Vec3{};
        angles = Vec3{};
        frame = 0;
        skinnum = 0;
        colormap = 0;
        modelindex = 0;
        trans = 0;
        colourmod = {255, 255, 255};
    }
    
    [[nodiscard]] constexpr bool isPlayer() const noexcept {
        return colormap > 0 && colormap <= server_constants::MAX_CLIENTS;
    }
};

// =============================================================================
// Server Time Info
// =============================================================================

/**
 * @brief Server timing information
 */
struct ServerTime {
    double time = 0.0;          // sv.time
    double oldTime = 0.0;       // sv.old_time
    double physicsTime = 0.0;   // sv.physicstime
    double pausedSince = 0.0;   // when pause started
    bool paused = false;
    
    [[nodiscard]] double deltaTime() const noexcept {
        return time - oldTime;
    }
    
    [[nodiscard]] double pauseDuration(double currentTime) const noexcept {
        if (!paused || pausedSince <= 0.0) return 0.0;
        return currentTime - pausedSince;
    }
    
    void advance(double dt) noexcept {
        oldTime = time;
        time += dt;
    }
};

} // namespace quake::server
