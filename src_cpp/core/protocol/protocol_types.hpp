/**
 * @file protocol_types.hpp
 * @brief QuakeWorld protocol types and constants
 * 
 * This file defines the protocol constants for the QuakeWorld network protocol
 * including server commands (svc_*), client commands (clc_*), and various flags.
 * 
 * Original C: protocol.h, cl_parse.c svc_strings[]
 */

#pragma once

#include <cstdint>
#include <string_view>
#include <array>

namespace ezquake {

//=============================================================================
// Protocol Version Constants
//=============================================================================

namespace protocol {

/// Standard QuakeWorld protocol version
inline constexpr int VERSION = 28;

/// FTE protocol extension magic numbers
inline constexpr uint32_t VERSION_FTE  = (('F' << 0) + ('T' << 8) + ('E' << 16) + ('X' << 24));
inline constexpr uint32_t VERSION_FTE2 = (('F' << 0) + ('T' << 8) + ('E' << 16) + ('2' << 24));
inline constexpr uint32_t VERSION_MVD1 = (('M' << 0) + ('V' << 8) + ('D' << 16) + ('1' << 24));

/// Default ports
inline constexpr int PORT_MASTER   = 27000;
inline constexpr int PORT_CLIENT   = 27001;
inline constexpr int PORT_SERVER   = 27500;
inline constexpr int PORT_QUAKETV  = 27900;

} // namespace protocol

//=============================================================================
// Server Commands (svc_*)
//=============================================================================

/**
 * @brief Server-to-client message types.
 * 
 * These are the command bytes sent by the server to clients.
 * Original: svc_* defines in protocol.h
 */
enum class ServerCommand : uint8_t {
    Bad                 = 0,
    Nop                 = 1,
    Disconnect          = 2,
    UpdateStat          = 3,    ///< [byte] [byte]
    NQVersion           = 4,    ///< NetQuake: [long] server version
    NQSetView           = 5,    ///< NetQuake: [short] entity number
    Sound               = 6,    ///< <see code>
    NQTime              = 7,    ///< NetQuake: [float] server time
    Print               = 8,    ///< [byte] id [string] null terminated
    Stufftext           = 9,    ///< [string] stuffed into console
    SetAngle            = 10,   ///< [angle3] view angle
    
    ServerData          = 11,   ///< [long] protocol ...
    LightStyle          = 12,   ///< [byte] [string]
    NQUpdateName        = 13,   ///< NetQuake: [byte] [string]
    UpdateFrags         = 14,   ///< [byte] [short]
    NQClientData        = 15,   ///< NetQuake: <shortbits + data>
    StopSound           = 16,   ///< <see code>
    NQUpdateColors      = 17,   ///< NetQuake: [byte] [byte] [byte]
    NQParticle          = 18,   ///< NetQuake: [vec3] <variable>
    Damage              = 19,
    
    SpawnStatic         = 20,
    FTESpawnStatic2     = 21,   ///< FTE extension
    SpawnBaseline       = 22,
    
    TempEntity          = 23,   ///< variable
    SetPause            = 24,   ///< [byte] on / off
    NQSignOnNum         = 25,   ///< NetQuake: [byte]
    
    CenterPrint         = 26,   ///< [string]
    
    KilledMonster       = 27,
    FoundSecret         = 28,
    
    SpawnStaticSound    = 29,   ///< [coord3] [byte] samp [byte] vol [byte] aten
    
    Intermission        = 30,   ///< [vec3_t] origin [vec3_t] angle
    Finale              = 31,   ///< [string] text
    
    CDTrack             = 32,   ///< [byte] track
    SellScreen          = 33,
    NQCutscene          = 34,   ///< NetQuake: same as SmallKick
    
    SmallKick           = 34,   ///< set client punchangle to 2
    BigKick             = 35,   ///< set client punchangle to 4
    
    UpdatePing          = 36,   ///< [byte] [short]
    UpdateEnterTime     = 37,   ///< [byte] [float]
    
    UpdateStatLong      = 38,   ///< [byte] [long]
    
    MuzzleFlash         = 39,   ///< [short] entity
    
    UpdateUserInfo      = 40,   ///< [byte] slot [long] uid [string] userinfo
    
    Download            = 41,   ///< [short] size [size bytes]
    PlayerInfo          = 42,   ///< variable
    Nails               = 43,   ///< [byte] num [48 bits] xyzpy
    ChokeCount          = 44,   ///< [byte] packets choked
    ModelList           = 45,   ///< [strings]
    SoundList           = 46,   ///< [strings]
    PacketEntities      = 47,   ///< [...]
    DeltaPacketEntities = 48,   ///< [...]
    MaxSpeed            = 49,   ///< maxspeed change
    EntGravity          = 50,   ///< gravity change
    SetInfo             = 51,   ///< setinfo on a client
    ServerInfo          = 52,   ///< serverinfo
    UpdatePL            = 53,   ///< [byte] [byte]
    Nails2              = 54,   ///< MVD: [byte] num [52 bits]
    
    // FTE extensions
    FTEModelListShort   = 60,   ///< FTE: short model indices
    FTESpawnBaseline2   = 66,
    QizmoVoice          = 83,
    FTEVoiceChat        = 84,   ///< FTE voice chat
};

/**
 * @brief Get human-readable name for a server command.
 */
[[nodiscard]] constexpr std::string_view serverCommandName(ServerCommand cmd) noexcept {
    switch (cmd) {
        case ServerCommand::Bad:                return "svc_bad";
        case ServerCommand::Nop:                return "svc_nop";
        case ServerCommand::Disconnect:         return "svc_disconnect";
        case ServerCommand::UpdateStat:         return "svc_updatestat";
        case ServerCommand::NQVersion:          return "svc_version";
        case ServerCommand::NQSetView:          return "svc_setview";
        case ServerCommand::Sound:              return "svc_sound";
        case ServerCommand::NQTime:             return "svc_time";
        case ServerCommand::Print:              return "svc_print";
        case ServerCommand::Stufftext:          return "svc_stufftext";
        case ServerCommand::SetAngle:           return "svc_setangle";
        case ServerCommand::ServerData:         return "svc_serverdata";
        case ServerCommand::LightStyle:         return "svc_lightstyle";
        case ServerCommand::NQUpdateName:       return "svc_updatename";
        case ServerCommand::UpdateFrags:        return "svc_updatefrags";
        case ServerCommand::NQClientData:       return "svc_clientdata";
        case ServerCommand::StopSound:          return "svc_stopsound";
        case ServerCommand::NQUpdateColors:     return "svc_updatecolors";
        case ServerCommand::NQParticle:         return "svc_particle";
        case ServerCommand::Damage:             return "svc_damage";
        case ServerCommand::SpawnStatic:        return "svc_spawnstatic";
        case ServerCommand::FTESpawnStatic2:    return "svc_fte_spawnstatic2";
        case ServerCommand::SpawnBaseline:      return "svc_spawnbaseline";
        case ServerCommand::TempEntity:         return "svc_temp_entity";
        case ServerCommand::SetPause:           return "svc_setpause";
        case ServerCommand::NQSignOnNum:        return "svc_signonnum";
        case ServerCommand::CenterPrint:        return "svc_centerprint";
        case ServerCommand::KilledMonster:      return "svc_killedmonster";
        case ServerCommand::FoundSecret:        return "svc_foundsecret";
        case ServerCommand::SpawnStaticSound:   return "svc_spawnstaticsound";
        case ServerCommand::Intermission:       return "svc_intermission";
        case ServerCommand::Finale:             return "svc_finale";
        case ServerCommand::CDTrack:            return "svc_cdtrack";
        case ServerCommand::SellScreen:         return "svc_sellscreen";
        case ServerCommand::SmallKick:          return "svc_smallkick";
        case ServerCommand::BigKick:            return "svc_bigkick";
        case ServerCommand::UpdatePing:         return "svc_updateping";
        case ServerCommand::UpdateEnterTime:    return "svc_updateentertime";
        case ServerCommand::UpdateStatLong:     return "svc_updatestatlong";
        case ServerCommand::MuzzleFlash:        return "svc_muzzleflash";
        case ServerCommand::UpdateUserInfo:     return "svc_updateuserinfo";
        case ServerCommand::Download:           return "svc_download";
        case ServerCommand::PlayerInfo:         return "svc_playerinfo";
        case ServerCommand::Nails:              return "svc_nails";
        case ServerCommand::ChokeCount:         return "svc_chokecount";
        case ServerCommand::ModelList:          return "svc_modellist";
        case ServerCommand::SoundList:          return "svc_soundlist";
        case ServerCommand::PacketEntities:     return "svc_packetentities";
        case ServerCommand::DeltaPacketEntities: return "svc_deltapacketentities";
        case ServerCommand::MaxSpeed:           return "svc_maxspeed";
        case ServerCommand::EntGravity:         return "svc_entgravity";
        case ServerCommand::SetInfo:            return "svc_setinfo";
        case ServerCommand::ServerInfo:         return "svc_serverinfo";
        case ServerCommand::UpdatePL:           return "svc_updatepl";
        case ServerCommand::Nails2:             return "svc_nails2";
        case ServerCommand::FTEModelListShort:  return "svc_fte_modellistshort";
        case ServerCommand::FTESpawnBaseline2:  return "svc_fte_spawnbaseline2";
        case ServerCommand::QizmoVoice:         return "svc_qizmovoice";
        case ServerCommand::FTEVoiceChat:       return "svc_fte_voicechat";
        default:                                return "svc_unknown";
    }
}

//=============================================================================
// Client Commands (clc_*)
//=============================================================================

/**
 * @brief Client-to-server message types.
 * 
 * Original: clc_* defines in protocol.h
 */
enum class ClientCommand : uint8_t {
    Bad         = 0,
    Nop         = 1,
    // DoubleMove = 2,  // Unused
    Move        = 3,    ///< [usercmd_t]
    StringCmd   = 4,    ///< [string] message
    Delta       = 5,    ///< [byte] sequence, requests delta
    TMove       = 6,    ///< teleport request (spectator)
    Upload      = 7,
    
    // FTE/MVD extensions
    VoiceChat   = 83,   ///< FTE voice chat
    MVDWeapon   = 200,  ///< MVD server-side weapon
    MVDDebug    = 201,  ///< MVD debug info
};

/**
 * @brief Get human-readable name for a client command.
 */
[[nodiscard]] constexpr std::string_view clientCommandName(ClientCommand cmd) noexcept {
    switch (cmd) {
        case ClientCommand::Bad:        return "clc_bad";
        case ClientCommand::Nop:        return "clc_nop";
        case ClientCommand::Move:       return "clc_move";
        case ClientCommand::StringCmd:  return "clc_stringcmd";
        case ClientCommand::Delta:      return "clc_delta";
        case ClientCommand::TMove:      return "clc_tmove";
        case ClientCommand::Upload:     return "clc_upload";
        case ClientCommand::VoiceChat:  return "clc_voicechat";
        case ClientCommand::MVDWeapon:  return "clc_mvd_weapon";
        case ClientCommand::MVDDebug:   return "clc_mvd_debug";
        default:                        return "clc_unknown";
    }
}

//=============================================================================
// Player Info Flags (PF_*)
//=============================================================================

/**
 * @brief Player info update flags.
 * 
 * These flags indicate which fields are included in a player info update.
 * Original: PF_* defines in protocol.h
 */
namespace PlayerFlags {
    inline constexpr uint32_t MSEC        = (1 << 0);
    inline constexpr uint32_t COMMAND     = (1 << 1);
    inline constexpr uint32_t VELOCITY1   = (1 << 2);
    inline constexpr uint32_t VELOCITY2   = (1 << 3);
    inline constexpr uint32_t VELOCITY3   = (1 << 4);
    inline constexpr uint32_t MODEL       = (1 << 5);
    inline constexpr uint32_t SKINNUM     = (1 << 6);
    inline constexpr uint32_t EFFECTS     = (1 << 7);
    inline constexpr uint32_t WEAPONFRAME = (1 << 8);   ///< Only sent for view player
    inline constexpr uint32_t DEAD        = (1 << 9);   ///< Don't block movement
    inline constexpr uint32_t GIB         = (1 << 10);  ///< Offset view height
    
    // Move type is encoded in bits 11-13
    inline constexpr int PMC_SHIFT = 11;
    inline constexpr uint32_t PMC_MASK = 7;
    
    // ZQuake extensions (offset depends on FTE_PEXT_TRANS)
    inline constexpr uint32_t ONGROUND    = (1 << 14);  ///< Without FTE_PEXT_TRANS
    inline constexpr uint32_t SOLID       = (1 << 15);  ///< Without FTE_PEXT_TRANS
    inline constexpr uint32_t ONGROUND_FTE = (1 << 22); ///< With FTE_PEXT_TRANS
    inline constexpr uint32_t SOLID_FTE   = (1 << 23);  ///< With FTE_PEXT_TRANS
    
    /**
     * @brief Extract player move type from flags.
     */
    [[nodiscard]] constexpr int extractMoveType(uint32_t flags) noexcept {
        return static_cast<int>((flags >> PMC_SHIFT) & PMC_MASK);
    }
}

//=============================================================================
// Entity Update Flags (U_*)
//=============================================================================

/**
 * @brief Entity update flags.
 * 
 * The first 16 bits of a packetentities update holds 9 bits of entity number
 * and 7 bits of flags.
 * Original: U_* defines in protocol.h
 */
namespace EntityFlags {
    inline constexpr uint16_t ORIGIN1   = (1 << 9);
    inline constexpr uint16_t ORIGIN2   = (1 << 10);
    inline constexpr uint16_t ORIGIN3   = (1 << 11);
    inline constexpr uint16_t ANGLE2    = (1 << 12);
    inline constexpr uint16_t FRAME     = (1 << 13);
    inline constexpr uint16_t REMOVE    = (1 << 14);    ///< Remove entity
    inline constexpr uint16_t MOREBITS  = (1 << 15);
    
    // Extended flags (after MOREBITS)
    inline constexpr uint8_t ANGLE1     = (1 << 0);
    inline constexpr uint8_t ANGLE3     = (1 << 1);
    inline constexpr uint8_t MODEL      = (1 << 2);
    inline constexpr uint8_t COLORMAP   = (1 << 3);
    inline constexpr uint8_t SKIN       = (1 << 4);
    inline constexpr uint8_t EFFECTS    = (1 << 5);
    inline constexpr uint8_t SOLID      = (1 << 6);
    inline constexpr uint8_t CHECKMORE  = (1 << 7);     ///< More extended flags
    
    // FTE extended flags
    inline constexpr uint8_t FTE_MODELDBL    = (1 << 0);
    inline constexpr uint8_t FTE_ENTITYDBL   = (1 << 1);
    inline constexpr uint8_t FTE_ENTITYDBL2  = (1 << 2);
    inline constexpr uint8_t FTE_SCALE       = (1 << 4);
    inline constexpr uint8_t FTE_ALPHA       = (1 << 5);
    inline constexpr uint8_t FTE_FATNESS     = (1 << 6);
    inline constexpr uint8_t FTE_COLOURMOD   = (1 << 7);
    
    /// Mask for entity number (lower 9 bits)
    inline constexpr uint16_t ENTITY_MASK = 0x01FF;
    
    /**
     * @brief Extract entity number from update word.
     */
    [[nodiscard]] constexpr int extractEntityNum(uint16_t word) noexcept {
        return word & ENTITY_MASK;
    }
}

//=============================================================================
// Command Move Flags (CM_*)
//=============================================================================

/**
 * @brief User command delta flags.
 * 
 * Original: CM_* defines in protocol.h
 */
namespace CommandFlags {
    inline constexpr uint8_t ANGLE1     = (1 << 0);
    inline constexpr uint8_t ANGLE3     = (1 << 1);
    inline constexpr uint8_t FORWARD    = (1 << 2);
    inline constexpr uint8_t SIDE       = (1 << 3);
    inline constexpr uint8_t UP         = (1 << 4);
    inline constexpr uint8_t BUTTONS    = (1 << 5);
    inline constexpr uint8_t IMPULSE    = (1 << 6);
    inline constexpr uint8_t ANGLE2     = (1 << 7);
}

//=============================================================================
// MVD Demo Flags (DF_*)
//=============================================================================

/**
 * @brief MVD demo player flags.
 * 
 * Original: DF_* defines in protocol.h
 */
namespace DemoFlags {
    inline constexpr uint16_t ORIGIN      = 1;
    inline constexpr uint16_t ANGLES      = (1 << 3);
    inline constexpr uint16_t EFFECTS     = (1 << 6);
    inline constexpr uint16_t SKINNUM     = (1 << 7);
    inline constexpr uint16_t DEAD        = (1 << 8);
    inline constexpr uint16_t GIB         = (1 << 9);
    inline constexpr uint16_t WEAPONFRAME = (1 << 10);
    inline constexpr uint16_t MODEL       = (1 << 11);
}

//=============================================================================
// ZQuake Protocol Extensions (Z_EXT_*)
//=============================================================================

/**
 * @brief ZQuake protocol extensions.
 * 
 * These are negotiated via the *z_ext serverinfo key.
 * Original: Z_EXT_* defines in protocol.h
 */
namespace ZExtensions {
    inline constexpr uint32_t PM_TYPE       = (1 << 0);  ///< Basic PM_TYPE
    inline constexpr uint32_t PM_TYPE_NEW   = (1 << 1);  ///< PM_FLY, PM_SPECTATOR
    inline constexpr uint32_t VIEWHEIGHT    = (1 << 2);  ///< STAT_VIEWHEIGHT
    inline constexpr uint32_t SERVERTIME    = (1 << 3);  ///< STAT_TIME
    inline constexpr uint32_t PITCHLIMITS   = (1 << 4);  ///< maxpitch/minpitch
    inline constexpr uint32_t JOIN_OBSERVE  = (1 << 5);  ///< join/observe commands
    inline constexpr uint32_t PF_ONGROUND   = (1 << 6);  ///< PF_ONGROUND valid
    inline constexpr uint32_t VWEP          = (1 << 7);  ///< ZQ_VWEP extension
    inline constexpr uint32_t PF_SOLID      = (1 << 8);
    
    /// Extensions supported by client
    inline constexpr uint32_t CLIENT = PM_TYPE | PM_TYPE_NEW | VIEWHEIGHT | 
                                       SERVERTIME | PITCHLIMITS | JOIN_OBSERVE |
                                       PF_ONGROUND | VWEP | PF_SOLID;
}

//=============================================================================
// FTE Protocol Extensions
//=============================================================================

/**
 * @brief FTE protocol extensions (first set).
 */
namespace FTEExtensions {
    inline constexpr uint32_t TRANS              = 0x00000008;  ///< .alpha support
    inline constexpr uint32_t HLBSP              = 0x00000200;  ///< Half-Life BSP
    inline constexpr uint32_t MODELDBL           = 0x00001000;  ///< 2-byte models
    inline constexpr uint32_t ENTITYDBL          = 0x00002000;  ///< 1024 entities
    inline constexpr uint32_t ENTITYDBL2         = 0x00004000;
    inline constexpr uint32_t FLOATCOORDS        = 0x00008000;  ///< Float origins
    inline constexpr uint32_t SPAWNSTATIC2       = 0x00400000;  ///< Entity delta
    inline constexpr uint32_t COLOURMOD          = 0x00080000;  ///< Color mod
    inline constexpr uint32_t PACKETENTITIES_256 = 0x01000000;  ///< 256 packet ents
    inline constexpr uint32_t CHUNKEDDOWNLOADS   = 0x20000000;  ///< Chunked DL
}

/**
 * @brief FTE protocol extensions (second set).
 */
namespace FTEExtensions2 {
    inline constexpr uint32_t VOICECHAT = 0x00000002;
}

//=============================================================================
// MVD Protocol Extensions
//=============================================================================

/**
 * @brief MVD protocol extensions.
 */
namespace MVDExtensions {
    inline constexpr uint32_t FLOATCOORDS      = (1 << 0);
    inline constexpr uint32_t HIGHLAGTELEPORT  = (1 << 1);
    inline constexpr uint32_t DEBUG_WEAPON     = (1 << 3);
    inline constexpr uint32_t DEBUG_ANTILAG    = (1 << 4);
    inline constexpr uint32_t HIDDEN_MESSAGES  = (1 << 5);
}

//=============================================================================
// Print Levels
//=============================================================================

/**
 * @brief Print message levels for svc_print.
 */
enum class PrintLevel : uint8_t {
    Low    = 0,     ///< Pickup messages
    Medium = 1,     ///< Death messages
    High   = 2,     ///< Critical messages
    Chat   = 3,     ///< Chat messages
};

//=============================================================================
// Temp Entity Types
//=============================================================================

/**
 * @brief Temporary entity types for svc_temp_entity.
 */
enum class TempEntityType : uint8_t {
    Spike           = 0,
    SuperSpike      = 1,
    Gunshot         = 2,
    Explosion       = 3,
    TarExplosion    = 4,
    Lightning1      = 5,
    Lightning2      = 6,
    WizSpike        = 7,
    KnightSpike     = 8,
    Lightning3      = 9,
    LavaSplash      = 10,
    Teleport        = 11,
    Blood           = 12,
    LightningBlood  = 13,
    
    // Extended types
    Explosion3      = 16,   ///< Colored explosion
    RailTrail       = 17,   ///< Q2-style rail
    Beam            = 100,  ///< Grappling hook
};

//=============================================================================
// Sound Flags
//=============================================================================

/**
 * @brief Sound flags for svc_sound.
 */
namespace SoundFlags {
    inline constexpr uint8_t VOLUME     = (1 << 0);
    inline constexpr uint8_t ATTENUATION = (1 << 1);
    inline constexpr uint8_t LOOPING    = (1 << 2);
}

//=============================================================================
// Network Limits
//=============================================================================

namespace NetworkLimits {
    inline constexpr int MAX_CLIENTS           = 32;
    inline constexpr int MAX_MODELS            = 512;   ///< With extensions
    inline constexpr int MAX_SOUNDS            = 512;
    inline constexpr int MAX_LIGHTSTYLES       = 64;
    inline constexpr int MAX_PACKET_ENTITIES   = 64;
    inline constexpr int MAX_MVD_PACKET_ENTITIES = 300;
    inline constexpr int MAX_EDICTS            = 2048;  ///< With FTE extensions
    inline constexpr int UPDATE_BACKUP         = 64;
    inline constexpr int UPDATE_MASK           = UPDATE_BACKUP - 1;
    inline constexpr int MAX_MSGLEN            = 65536;
    inline constexpr int MAX_INFO_STRING       = 1024;
}

} // namespace ezquake
