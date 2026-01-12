/**
 * @file connection_state.hpp
 * @brief Client connection state machine enums and types
 * 
 * This file defines the connection states and related enums for managing
 * client-server connections.
 * 
 * Original C: cactive_t enum and connection handling in client.h, cl_main.c
 * 
 * @note State classes like DownloadState, DemoState, etc. are defined in
 *       core/client/client_persistent.hpp to avoid duplication.
 */

#pragma once

#include <cstdint>
#include <string_view>

namespace ezquake {

//=============================================================================
// Connection State Enum
//=============================================================================

/**
 * @brief Client connection state machine states.
 * 
 * Original: cactive_t enum in client.h
 */
enum class ConnectionState : uint8_t {
    Disconnected = 0,   ///< Full screen console with no connection
    DemoStart,          ///< Starting up a demo
    Connected,          ///< netchan_t established, waiting for svc_serverdata
    OnServer,           ///< Processing data lists, downloading, etc.
    Active              ///< Everything is loaded, frames can be rendered
};

/**
 * @brief Get string name of connection state.
 */
[[nodiscard]] constexpr std::string_view connectionStateName(ConnectionState state) noexcept {
    switch (state) {
        case ConnectionState::Disconnected: return "disconnected";
        case ConnectionState::DemoStart:    return "demostart";
        case ConnectionState::Connected:    return "connected";
        case ConnectionState::OnServer:     return "onserver";
        case ConnectionState::Active:       return "active";
    }
    return "unknown";
}

/**
 * @brief Check if state represents an active connection.
 */
[[nodiscard]] constexpr bool isConnected(ConnectionState state) noexcept {
    return state >= ConnectionState::Connected;
}

/**
 * @brief Check if state allows frame rendering.
 */
[[nodiscard]] constexpr bool canRenderFrames(ConnectionState state) noexcept {
    return state == ConnectionState::Active;
}

//=============================================================================
// Download Types
//=============================================================================

/**
 * @brief Types of downloads in progress.
 * 
 * Original: dltype_t enum in client.h
 */
enum class DownloadType : uint8_t {
    None = 0,
    Model,          ///< Model file (.mdl, .bsp, etc.)
    VwepModel,      ///< VWep (visible weapon) model
    Sound,          ///< Sound file
    Skin,           ///< Player skin
    Single          ///< Single file download (manual)
};

/**
 * @brief Get string name of download type.
 */
[[nodiscard]] constexpr std::string_view downloadTypeName(DownloadType type) noexcept {
    switch (type) {
        case DownloadType::None:      return "none";
        case DownloadType::Model:     return "model";
        case DownloadType::VwepModel: return "vwep";
        case DownloadType::Sound:     return "sound";
        case DownloadType::Skin:      return "skin";
        case DownloadType::Single:    return "file";
    }
    return "unknown";
}

/**
 * @brief Download method types.
 * 
 * Original: DL_NONE, DL_QW, DL_QWCHUNKS
 */
enum class DownloadMethod : uint8_t {
    None = 0,
    Standard,       ///< Standard QW downloads
    Chunked         ///< FTE chunked downloads (faster)
};

//=============================================================================
// Demo Seeking Types
//=============================================================================

/**
 * @brief Demo seeking mode types.
 * 
 * Original: demoseekingtype_t enum in client.h
 */
enum class DemoSeekingType : uint8_t {
    None = 0,               ///< Not seeking
    Normal,                 ///< demo_jump seeking
    DemoMark,               ///< demo_jump_mark seeking
    Status,                 ///< demo_jump_status seeking
    Found,                  ///< Mark/status found, should stop
    End,                    ///< demo_jump_end (intermission or 1 second before end)
    FoundNoRewind           ///< Found, no automatic rewind
};

/**
 * @brief Demo seeking status match types.
 * 
 * Original: demoseekingstatus_matchtype_t
 */
enum class DemoSeekMatchType : uint8_t {
    Equal = 0,
    NotEqual,
    LessThan,
    GreaterThan,
    BitOn,
    BitOff
};

//=============================================================================
// MVD Playback Types
//=============================================================================

/**
 * @brief MVD playback mode.
 * 
 * Original: MVD_FILE_PLAYBACK, QTV_PLAYBACK defines
 */
enum class MvdPlayback : uint8_t {
    None = 0,       ///< Not playing MVD
    File = 1,       ///< Playing MVD from file
    Qtv = 2         ///< Playing via QTV proxy
};

//=============================================================================
// Timedemo Mode
//=============================================================================

/**
 * @brief Timedemo mode types.
 * 
 * Original: TIMEDEMO_OFF, TIMEDEMO_CLASSIC, TIMEDEMO_FIXEDFPS
 */
enum class TimedemoMode : uint8_t {
    Off = 0,
    Classic = 1,
    FixedFps = 2
};

namespace timedemo {
    constexpr int DEFAULT_FPS = 308;
    constexpr int MIN_FPS = 20;
    constexpr int MAX_FPS = 10000;
} // namespace timedemo

//=============================================================================
// Connection Timing
//=============================================================================

/**
 * @brief Connection timing information.
 */
struct ConnectionTiming {
    double realtime = 0.0;          ///< Scaled by cl_demospeed
    double demotime = 0.0;          ///< Demo time, reset on demo start
    double trueFrametime = 0.0;     ///< Time since last frame
    double frametime = 0.0;         ///< Frame time scaled by cl_demospeed
    double demoPacketTime = 0.0;    ///< Timestamp of current demo packet
    int frameCount = 0;             ///< Incremented every frame, never reset
    
    void advanceFrame(double dt, double demoSpeed = 1.0) noexcept {
        trueFrametime = dt;
        frametime = dt * demoSpeed;
        realtime += frametime;
        demotime += frametime;
        ++frameCount;
    }
    
    void reset() noexcept {
        *this = ConnectionTiming{};
    }
};

//=============================================================================
// Protocol Extensions
//=============================================================================

/**
 * @brief Protocol extensions negotiated with server.
 */
struct ProtocolExtensions {
    uint32_t fte1 = 0;      ///< FTE protocol extensions 1
    uint32_t fte2 = 0;      ///< FTE protocol extensions 2
    uint32_t mvd1 = 0;      ///< MVD protocol extensions 1
    
    void clear() noexcept {
        fte1 = 0;
        fte2 = 0;
        mvd1 = 0;
    }
    
    [[nodiscard]] constexpr bool any() const noexcept {
        return fte1 != 0 || fte2 != 0 || mvd1 != 0;
    }
};

} // namespace ezquake
