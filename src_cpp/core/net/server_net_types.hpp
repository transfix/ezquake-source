/**
 * @file server_net_types.hpp
 * @brief Server networking type definitions
 * 
 * Converted from sv_send.c, sv_nchan.c, sv_ents.c, protocol.h
 * Part of ezQuake C to C++20 Conversion - Iteration 20
 */

#pragma once

#include <cstdint>
#include <string_view>
#include <span>
#include <array>
#include "core/math/vec3.hpp"

namespace ezquake::net {

using math::Vec3;

// =============================================================================
// Print Levels (from protocol.h)
// =============================================================================

/**
 * Print level for server messages
 * Used with svc_print to control message display
 */
enum class PrintLevel : uint8_t {
    Low    = 0,  // Low priority (debug/info)
    Medium = 1,  // Normal messages
    High   = 2,  // Important messages
    Chat   = 3   // Chat messages (also go to chat buffer)
};

[[nodiscard]] constexpr std::string_view printLevelName(PrintLevel level) noexcept {
    switch (level) {
        case PrintLevel::Low:    return "low";
        case PrintLevel::Medium: return "medium";
        case PrintLevel::High:   return "high";
        case PrintLevel::Chat:   return "chat";
        default:                 return "unknown";
    }
}

[[nodiscard]] constexpr bool isChatMessage(PrintLevel level) noexcept {
    return level == PrintLevel::Chat;
}

// =============================================================================
// Broadcast Print Flags (from server.h)
// =============================================================================

/**
 * Flags controlling broadcast print behavior
 */
namespace BroadcastFlags {
    constexpr uint32_t None           = 0;
    constexpr uint32_t IgnoreInDemo   = (1 << 0);  // Don't record in demo
    constexpr uint32_t IgnoreClients  = (1 << 1);  // Don't send to clients (demo only)
    constexpr uint32_t QtvOnly        = (1 << 2);  // Only to QTV stream, not file
    constexpr uint32_t IgnoreConsole  = (1 << 3);  // Don't print to server console
    
    [[nodiscard]] constexpr bool shouldPutInDemo(uint32_t flags) noexcept {
        return !(flags & IgnoreInDemo);
    }
    
    [[nodiscard]] constexpr bool shouldSendToClients(uint32_t flags) noexcept {
        return !(flags & IgnoreClients);
    }
    
    [[nodiscard]] constexpr bool shouldPutInConsole(uint32_t flags) noexcept {
        return !(flags & IgnoreConsole);
    }
}

// =============================================================================
// Sound Channels (from sv_send.c)
// =============================================================================

/**
 * Sound channel types for entity sounds
 */
enum class SoundChannel : uint8_t {
    Auto   = 0,  // Auto-allocate channel
    Weapon = 1,  // Weapon fire sounds
    Voice  = 2,  // Voice/vocalization
    Item   = 3,  // Item pickup
    Body   = 4   // Movement/body sounds
};

[[nodiscard]] constexpr std::string_view soundChannelName(SoundChannel chan) noexcept {
    switch (chan) {
        case SoundChannel::Auto:   return "auto";
        case SoundChannel::Weapon: return "weapon";
        case SoundChannel::Voice:  return "voice";
        case SoundChannel::Item:   return "item";
        case SoundChannel::Body:   return "body";
        default:                   return "unknown";
    }
}

// =============================================================================
// Sound Packet Flags (from protocol.h)
// =============================================================================

namespace SoundFlags {
    constexpr uint16_t Volume      = (1 << 15);  // Volume byte follows
    constexpr uint16_t Attenuation = (1 << 14);  // Attenuation byte follows
    
    constexpr uint8_t DefaultVolume = 255;
    constexpr float DefaultAttenuation = 1.0f;
}

// =============================================================================
// Protocol Version Constants (from protocol.h)
// =============================================================================

namespace protocol_constants {
    constexpr int32_t VERSION = 28;
    
    // Port defaults
    constexpr uint16_t PORT_MASTER  = 27000;
    constexpr uint16_t PORT_CLIENT  = 27001;
    constexpr uint16_t PORT_SERVER  = 27500;
    constexpr uint16_t PORT_QUAKETV = 27900;
    
    // Entity limits
    constexpr int32_t MAX_CLIENTS = 32;
    constexpr int32_t MAX_PACKET_ENTITIES = 64;
    constexpr int32_t MAX_PEXT256_PACKET_ENTITIES = 256;
    constexpr int32_t MAX_MVD_PACKET_ENTITIES = 300;
    
    // Update tracking
    constexpr int32_t UPDATE_BACKUP = 64;  // Must be power of two
    constexpr int32_t UPDATE_MASK = UPDATE_BACKUP - 1;
    
    // Back buffer limits
    constexpr int32_t MAX_BACK_BUFFERS = 8;
    constexpr int32_t MAX_REDIRECT_MESSAGES = 256;
    constexpr int32_t OUTPUT_BUFFER_SIZE = 8192;
    
    // Nails
    constexpr int32_t MAX_NAILS = 32;
    
    // View
    constexpr float DEFAULT_VIEWHEIGHT = 22.0f;
}

// =============================================================================
// Out-of-band Message IDs (from protocol.h)
// =============================================================================

namespace oob_message {
    // M = master, S = server, C = client, A = any
    constexpr char S2C_CHALLENGE   = 'c';
    constexpr char S2C_CONNECTION  = 'j';
    constexpr char A2A_PING        = 'k';
    constexpr char A2A_ACK         = 'l';
    constexpr char A2A_NACK        = 'm';
    constexpr char A2A_ECHO        = 'e';
    constexpr char A2C_PRINT       = 'n';
    constexpr char S2M_HEARTBEAT   = 'a';
    constexpr char A2C_CLIENT_CMD  = 'B';
    constexpr char S2M_SHUTDOWN    = 'C';
}

// =============================================================================
// Server to Client Messages (from protocol.h)
// =============================================================================

enum class ServerMessage : uint8_t {
    Bad             = 0,
    Nop             = 1,
    Disconnect      = 2,
    UpdateStat      = 3,
    // NQ: Version   = 4,
    // NQ: SetView   = 5,
    Sound           = 6,
    // NQ: Time      = 7,
    Print           = 8,
    StuffText       = 9,
    SetAngle        = 10,
    ServerData      = 11,
    LightStyle      = 12,
    // NQ: UpdateName = 13,
    UpdateFrags     = 14,
    // NQ: ClientData = 15,
    StopSound       = 16,
    // NQ: UpdateColors = 17,
    // NQ: Particle  = 18,
    Damage          = 19,
    SpawnStatic     = 20,
    // FTE: SpawnStatic2 = 21,
    SpawnBaseline   = 22,
    TempEntity      = 23,
    SetPause        = 24,
    // NQ: SignonNum = 25,
    CenterPrint     = 26,
    KilledMonster   = 27,
    FoundSecret     = 28,
    SpawnStaticSound = 29,
    Intermission    = 30,
    Finale          = 31,
    CdTrack         = 32,
    SellScreen      = 33,
    SmallKick       = 34,  // Also NQ: Cutscene
    BigKick         = 35,
    UpdatePing      = 36,
    UpdateEnterTime = 37,
    UpdateStatLong  = 38,
    MuzzleFlash     = 39,
    UpdateUserInfo  = 40,
    Download        = 41,
    PlayerInfo      = 42,
    Nails           = 43,
    ChokeCount      = 44,
    ModelList       = 45,
    SoundList       = 46,
    PacketEntities  = 47,
    DeltaPacketEntities = 48,
    MaxSpeed        = 49,
    EntGravity      = 50,
    SetInfo         = 51,
    ServerInfo      = 52,
    UpdatePL        = 53,
    Nails2          = 54
};

[[nodiscard]] constexpr std::string_view serverMessageName(ServerMessage msg) noexcept {
    switch (msg) {
        case ServerMessage::Bad:             return "svc_bad";
        case ServerMessage::Nop:             return "svc_nop";
        case ServerMessage::Disconnect:      return "svc_disconnect";
        case ServerMessage::UpdateStat:      return "svc_updatestat";
        case ServerMessage::Sound:           return "svc_sound";
        case ServerMessage::Print:           return "svc_print";
        case ServerMessage::StuffText:       return "svc_stufftext";
        case ServerMessage::SetAngle:        return "svc_setangle";
        case ServerMessage::ServerData:      return "svc_serverdata";
        case ServerMessage::LightStyle:      return "svc_lightstyle";
        case ServerMessage::UpdateFrags:     return "svc_updatefrags";
        case ServerMessage::StopSound:       return "svc_stopsound";
        case ServerMessage::Damage:          return "svc_damage";
        case ServerMessage::SpawnStatic:     return "svc_spawnstatic";
        case ServerMessage::SpawnBaseline:   return "svc_spawnbaseline";
        case ServerMessage::TempEntity:      return "svc_temp_entity";
        case ServerMessage::SetPause:        return "svc_setpause";
        case ServerMessage::CenterPrint:     return "svc_centerprint";
        case ServerMessage::KilledMonster:   return "svc_killedmonster";
        case ServerMessage::FoundSecret:     return "svc_foundsecret";
        case ServerMessage::SpawnStaticSound: return "svc_spawnstaticsound";
        case ServerMessage::Intermission:    return "svc_intermission";
        case ServerMessage::Finale:          return "svc_finale";
        case ServerMessage::CdTrack:         return "svc_cdtrack";
        case ServerMessage::SellScreen:      return "svc_sellscreen";
        case ServerMessage::SmallKick:       return "svc_smallkick";
        case ServerMessage::BigKick:         return "svc_bigkick";
        case ServerMessage::UpdatePing:      return "svc_updateping";
        case ServerMessage::UpdateEnterTime: return "svc_updateentertime";
        case ServerMessage::UpdateStatLong:  return "svc_updatestatlong";
        case ServerMessage::MuzzleFlash:     return "svc_muzzleflash";
        case ServerMessage::UpdateUserInfo:  return "svc_updateuserinfo";
        case ServerMessage::Download:        return "svc_download";
        case ServerMessage::PlayerInfo:      return "svc_playerinfo";
        case ServerMessage::Nails:           return "svc_nails";
        case ServerMessage::ChokeCount:      return "svc_chokecount";
        case ServerMessage::ModelList:       return "svc_modellist";
        case ServerMessage::SoundList:       return "svc_soundlist";
        case ServerMessage::PacketEntities:  return "svc_packetentities";
        case ServerMessage::DeltaPacketEntities: return "svc_deltapacketentities";
        case ServerMessage::MaxSpeed:        return "svc_maxspeed";
        case ServerMessage::EntGravity:      return "svc_entgravity";
        case ServerMessage::SetInfo:         return "svc_setinfo";
        case ServerMessage::ServerInfo:      return "svc_serverinfo";
        case ServerMessage::UpdatePL:        return "svc_updatepl";
        case ServerMessage::Nails2:          return "svc_nails2";
        default:                             return "svc_unknown";
    }
}

// =============================================================================
// Client to Server Messages (from protocol.h)
// =============================================================================

enum class ClientMessage : uint8_t {
    Bad       = 0,
    Nop       = 1,
    // 2 = doublemove (unused)
    Move      = 3,
    StringCmd = 4,
    Delta     = 5,
    TMove     = 6,  // Teleport request (spectator)
    Upload    = 7
};

[[nodiscard]] constexpr std::string_view clientMessageName(ClientMessage msg) noexcept {
    switch (msg) {
        case ClientMessage::Bad:       return "clc_bad";
        case ClientMessage::Nop:       return "clc_nop";
        case ClientMessage::Move:      return "clc_move";
        case ClientMessage::StringCmd: return "clc_stringcmd";
        case ClientMessage::Delta:     return "clc_delta";
        case ClientMessage::TMove:     return "clc_tmove";
        case ClientMessage::Upload:    return "clc_upload";
        default:                       return "clc_unknown";
    }
}

// =============================================================================
// Player Info Flags (from protocol.h)
// =============================================================================

namespace PlayerFlags {
    constexpr uint32_t MSec        = (1 << 0);
    constexpr uint32_t Command     = (1 << 1);
    constexpr uint32_t Velocity1   = (1 << 2);
    constexpr uint32_t Velocity2   = (1 << 3);
    constexpr uint32_t Velocity3   = (1 << 4);
    constexpr uint32_t Model       = (1 << 5);
    constexpr uint32_t SkinNum     = (1 << 6);
    constexpr uint32_t Effects     = (1 << 7);
    constexpr uint32_t WeaponFrame = (1 << 8);
    constexpr uint32_t Dead        = (1 << 9);
    constexpr uint32_t Gib         = (1 << 10);
    
    // PMC shift/mask
    constexpr int32_t PMC_SHIFT = 11;
    constexpr uint32_t PMC_MASK = 7;
    
    // ZQuake extensions
    constexpr uint32_t OnGround    = (1 << 14);
    constexpr uint32_t Solid       = (1 << 15);
}

// =============================================================================
// Entity Update Flags (from protocol.h)
// =============================================================================

namespace EntityUpdateFlags {
    // First 16 bits: entity number (9 bits) + flags (7 bits)
    constexpr uint16_t Origin1   = (1 << 9);
    constexpr uint16_t Origin2   = (1 << 10);
    constexpr uint16_t Origin3   = (1 << 11);
    constexpr uint16_t Angle2    = (1 << 12);
    constexpr uint16_t Frame     = (1 << 13);
    constexpr uint16_t Remove    = (1 << 14);
    constexpr uint16_t MoreBits  = (1 << 15);
    
    // Extended flags (if MoreBits set)
    constexpr uint8_t Angle1   = (1 << 0);
    constexpr uint8_t Angle3   = (1 << 1);
    constexpr uint8_t Model    = (1 << 2);
    constexpr uint8_t Colormap = (1 << 3);
    constexpr uint8_t Skin     = (1 << 4);
    constexpr uint8_t Effects  = (1 << 5);
    constexpr uint8_t Solid    = (1 << 6);
    
    constexpr uint16_t CheckMoreBits = ((1 << 9) - 1);
}

// =============================================================================
// Client Move Flags (from protocol.h)
// =============================================================================

namespace ClientMoveFlags {
    constexpr uint8_t Angle1  = (1 << 0);
    constexpr uint8_t Angle3  = (1 << 1);
    constexpr uint8_t Forward = (1 << 2);
    constexpr uint8_t Side    = (1 << 3);
    constexpr uint8_t Up      = (1 << 4);
    constexpr uint8_t Buttons = (1 << 5);
    constexpr uint8_t Impulse = (1 << 6);
    constexpr uint8_t Angle2  = (1 << 7);
}

// =============================================================================
// MVD Demo Flags (from protocol.h)
// =============================================================================

namespace MvdDemoFlags {
    constexpr uint16_t Origin      = 1;
    constexpr uint16_t Angles      = (1 << 3);
    constexpr uint16_t Effects     = (1 << 6);
    constexpr uint16_t SkinNum     = (1 << 7);
    constexpr uint16_t Dead        = (1 << 8);
    constexpr uint16_t Gib         = (1 << 9);
    constexpr uint16_t WeaponFrame = (1 << 10);
    constexpr uint16_t Model       = (1 << 11);
}

// =============================================================================
// Entity State Structure
// =============================================================================

/**
 * Entity state transmitted over network
 */
struct EntityState {
    int32_t number = 0;       // Edict index
    int32_t flags = 0;        // No lerp, etc.
    Vec3 origin{};
    Vec3 angles{};
    int32_t modelIndex = 0;
    int32_t frame = 0;
    int32_t colormap = 0;
    int32_t skinNum = 0;
    int32_t effects = 0;
    uint8_t trans = 255;      // FTE transparency
    
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return number > 0;
    }
    
    [[nodiscard]] constexpr bool isWorld() const noexcept {
        return number == 0;
    }
    
    void clear() noexcept {
        *this = EntityState{};
    }
};

// =============================================================================
// User Command Structure  
// =============================================================================

/**
 * User movement command sent from client
 */
struct UserCmd {
    uint8_t msec = 0;
    Vec3 angles{};
    int16_t forwardMove = 0;
    int16_t sideMove = 0;
    int16_t upMove = 0;
    uint8_t buttons = 0;
    uint8_t impulse = 0;
    
    [[nodiscard]] constexpr bool hasMovement() const noexcept {
        return forwardMove != 0 || sideMove != 0 || upMove != 0;
    }
    
    [[nodiscard]] constexpr bool hasButtons() const noexcept {
        return buttons != 0;
    }
    
    [[nodiscard]] constexpr bool hasImpulse() const noexcept {
        return impulse != 0;
    }
    
    void clear() noexcept {
        *this = UserCmd{};
    }
};

// =============================================================================
// Back Buffer for Reliable Messages
// =============================================================================

/**
 * Status of a reliable message back buffer
 */
struct BackBufferState {
    int32_t numBackBuffers = 0;
    std::array<int32_t, protocol_constants::MAX_BACK_BUFFERS> sizes{};
    
    [[nodiscard]] constexpr bool hasBackBuffers() const noexcept {
        return numBackBuffers > 0;
    }
    
    [[nodiscard]] constexpr bool isFull() const noexcept {
        return numBackBuffers >= protocol_constants::MAX_BACK_BUFFERS;
    }
    
    [[nodiscard]] constexpr int32_t totalSize() const noexcept {
        int32_t total = 0;
        for (int i = 0; i < numBackBuffers; ++i) {
            total += sizes[static_cast<size_t>(i)];
        }
        return total;
    }
    
    void clear() noexcept {
        numBackBuffers = 0;
        sizes.fill(0);
    }
};

// =============================================================================
// Delta Compression Helper
// =============================================================================

/**
 * Tracks what fields changed for delta compression
 */
struct EntityDelta {
    uint16_t flags = 0;       // Primary update flags
    uint8_t extraFlags = 0;   // Extended flags (if MoreBits)
    
    [[nodiscard]] constexpr bool hasOriginChange() const noexcept {
        return (flags & (EntityUpdateFlags::Origin1 | 
                        EntityUpdateFlags::Origin2 | 
                        EntityUpdateFlags::Origin3)) != 0;
    }
    
    [[nodiscard]] constexpr bool hasAngleChange() const noexcept {
        return (flags & EntityUpdateFlags::Angle2) != 0 ||
               (extraFlags & (EntityUpdateFlags::Angle1 | EntityUpdateFlags::Angle3)) != 0;
    }
    
    [[nodiscard]] constexpr bool hasModelChange() const noexcept {
        return (extraFlags & EntityUpdateFlags::Model) != 0;
    }
    
    [[nodiscard]] constexpr bool isRemove() const noexcept {
        return (flags & EntityUpdateFlags::Remove) != 0;
    }
    
    [[nodiscard]] constexpr bool needsMoreBits() const noexcept {
        return (flags & EntityUpdateFlags::MoreBits) != 0;
    }
    
    [[nodiscard]] constexpr bool isEmpty() const noexcept {
        return flags == 0 && extraFlags == 0;
    }
};

// =============================================================================
// Sound Event
// =============================================================================

/**
 * Server sound event to send to clients
 */
struct SoundEvent {
    int32_t entityNum = 0;
    SoundChannel channel = SoundChannel::Auto;
    int32_t soundIndex = 0;
    Vec3 origin{};
    uint8_t volume = SoundFlags::DefaultVolume;
    float attenuation = SoundFlags::DefaultAttenuation;
    bool usePhs = true;     // Use PHS for multicast
    bool reliable = false;   // Send reliably
    
    [[nodiscard]] constexpr bool hasCustomVolume() const noexcept {
        return volume != SoundFlags::DefaultVolume;
    }
    
    [[nodiscard]] constexpr bool hasCustomAttenuation() const noexcept {
        return attenuation != SoundFlags::DefaultAttenuation;
    }
    
    [[nodiscard]] uint16_t encodeChannel() const noexcept {
        uint16_t encoded = static_cast<uint16_t>((entityNum << 3) | 
                           static_cast<uint8_t>(channel));
        if (hasCustomVolume()) {
            encoded |= SoundFlags::Volume;
        }
        if (hasCustomAttenuation()) {
            encoded |= SoundFlags::Attenuation;
        }
        return encoded;
    }
};

// =============================================================================
// Nail Update (compressed projectile data)
// =============================================================================

/**
 * Compressed nail/projectile state for network
 * Uses 48-bit encoding: xyzpy 12 12 12 4 8
 */
struct NailUpdate {
    Vec3 origin{};
    float pitch = 0.0f;
    float yaw = 0.0f;
    uint8_t colormap = 0;  // For nails2 format
    
    /**
     * Encode position for network (12-bit per axis)
     */
    [[nodiscard]] std::array<uint8_t, 6> encode() const noexcept {
        std::array<uint8_t, 6> bits{};
        
        int x = (static_cast<int>(origin[0] + 4096 + 1) >> 1) & 4095;
        int y = (static_cast<int>(origin[1] + 4096 + 1) >> 1) & 4095;
        int z = (static_cast<int>(origin[2] + 4096 + 1) >> 1) & 4095;
        int p = static_cast<int>(pitch * (16.0f / 360.0f)) & 15;
        int yaw_enc = static_cast<int>(yaw * (256.0f / 360.0f)) & 255;
        
        bits[0] = static_cast<uint8_t>(x);
        bits[1] = static_cast<uint8_t>((x >> 8) | (y << 4));
        bits[2] = static_cast<uint8_t>(y >> 4);
        bits[3] = static_cast<uint8_t>(z);
        bits[4] = static_cast<uint8_t>((z >> 8) | (p << 4));
        bits[5] = static_cast<uint8_t>(yaw_enc);
        
        return bits;
    }
};

} // namespace ezquake::net
