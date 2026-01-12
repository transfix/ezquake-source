/*
 * ezQuake C++ Port - Client State
 * 
 * Per-level client state that is wiped at every server signon.
 * 
 * Copyright (C) 2024 ezQuake Team
 */

#pragma once

#include "core/types.hpp"
#include "core/math/vec3.hpp"
#include "core/client/client_types.hpp"
#include "core/client/player_state.hpp"
#include "core/client/frame.hpp"
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace ezquake {

//=============================================================================
// Server Info
//=============================================================================

/**
 * @brief Parsed server information from serverinfo string.
 */
class ServerInfo {
public:
    ServerInfo() noexcept = default;
    
    [[nodiscard]] int serverCount() const noexcept { return serverCount_; }
    [[nodiscard]] int protoVersion() const noexcept { return protoVersion_; }
    [[nodiscard]] int deathmatch() const noexcept { return deathmatch_; }
    [[nodiscard]] int teamplay() const noexcept { return teamplay_; }
    [[nodiscard]] client::GameType gameType() const noexcept { return gameType_; }
    [[nodiscard]] bool isTeamfortress() const noexcept { return teamfortress_; }
    [[nodiscard]] bool isVwepEnabled() const noexcept { return vwepEnabled_; }
    [[nodiscard]] int timelimit() const noexcept { return timelimit_; }
    [[nodiscard]] int fraglimit() const noexcept { return fraglimit_; }
    [[nodiscard]] float maxFps() const noexcept { return maxFps_; }
    [[nodiscard]] float minPitch() const noexcept { return minPitch_; }
    [[nodiscard]] float maxPitch() const noexcept { return maxPitch_; }
    [[nodiscard]] int fpd() const noexcept { return fpd_; }
    [[nodiscard]] int zExt() const noexcept { return zExt_; }
    [[nodiscard]] std::string_view rawInfo() const noexcept { return serverinfo_; }
    
    void setServerCount(int count) noexcept { serverCount_ = count; }
    void setProtoVersion(int version) noexcept { protoVersion_ = version; }
    void setDeathmatch(int dm) noexcept { deathmatch_ = dm; }
    void setTeamplay(int tp) noexcept { teamplay_ = tp; }
    void setGameType(client::GameType type) noexcept { gameType_ = type; }
    void setTeamfortress(bool tf) noexcept { teamfortress_ = tf; }
    void setVwepEnabled(bool enabled) noexcept { vwepEnabled_ = enabled; }
    void setTimelimit(int limit) noexcept { timelimit_ = limit; }
    void setFraglimit(int limit) noexcept { fraglimit_ = limit; }
    void setMaxFps(float fps) noexcept { maxFps_ = fps; }
    void setMinPitch(float pitch) noexcept { minPitch_ = pitch; }
    void setMaxPitch(float pitch) noexcept { maxPitch_ = pitch; }
    void setFpd(int fpd) noexcept { fpd_ = fpd; }
    void setZExt(int ext) noexcept { zExt_ = ext; }
    void setRawInfo(std::string_view info) { serverinfo_ = info; }
    
    void clear() {
        serverCount_ = 0;
        protoVersion_ = 0;
        deathmatch_ = 0;
        teamplay_ = 0;
        gameType_ = client::GameType::Deathmatch;
        teamfortress_ = false;
        vwepEnabled_ = false;
        timelimit_ = 0;
        fraglimit_ = 0;
        maxFps_ = 0.0f;
        minPitch_ = -70.0f;
        maxPitch_ = 80.0f;
        fpd_ = 0;
        zExt_ = 0;
        serverinfo_.clear();
    }
    
private:
    int serverCount_ = 0;
    int protoVersion_ = 0;
    int deathmatch_ = 0;
    int teamplay_ = 0;
    client::GameType gameType_ = client::GameType::Deathmatch;
    bool teamfortress_ = false;
    bool vwepEnabled_ = false;
    int timelimit_ = 0;
    int fraglimit_ = 0;
    float maxFps_ = 0.0f;
    float minPitch_ = -70.0f;
    float maxPitch_ = 80.0f;
    int fpd_ = 0;
    int zExt_ = 0;
    std::string serverinfo_;
};

//=============================================================================
// Sequence Tracking
//=============================================================================

/**
 * @brief Network sequence number tracking for delta compression.
 */
class SequenceState {
public:
    SequenceState() noexcept = default;
    
    [[nodiscard]] int parseCount() const noexcept { return parseCount_; }
    [[nodiscard]] int oldParseCount() const noexcept { return oldParseCount_; }
    [[nodiscard]] int parseCountMod() const noexcept { return parseCountMod_; }
    [[nodiscard]] double parseCountTime() const noexcept { return parseCountTime_; }
    [[nodiscard]] int validSequence() const noexcept { return validSequence_; }
    [[nodiscard]] int oldValidSequence() const noexcept { return oldValidSequence_; }
    [[nodiscard]] int deltaSequence() const noexcept { return deltaSequence_; }
    
    void setParseCount(int count, double time) noexcept {
        oldParseCount_ = parseCount_;
        parseCount_ = count;
        parseCountMod_ = count & (client::UPDATE_BACKUP - 1);
        parseCountTime_ = time;
    }
    
    void setValidSequence(int seq) noexcept {
        oldValidSequence_ = validSequence_;
        validSequence_ = seq;
    }
    
    void setDeltaSequence(int seq) noexcept { deltaSequence_ = seq; }
    
    [[nodiscard]] bool canRender() const noexcept { return validSequence_ != 0; }
    
    void clear() noexcept {
        parseCount_ = 0;
        oldParseCount_ = 0;
        parseCountMod_ = 0;
        parseCountTime_ = 0.0;
        validSequence_ = 0;
        oldValidSequence_ = 0;
        deltaSequence_ = 0;
    }
    
private:
    int parseCount_ = 0;
    int oldParseCount_ = 0;
    int parseCountMod_ = 0;
    double parseCountTime_ = 0.0;
    int validSequence_ = 0;
    int oldValidSequence_ = 0;
    int deltaSequence_ = 0;
};

//=============================================================================
// View State
//=============================================================================

/**
 * @brief Client view state including angles, position, and effects.
 */
class ViewState {
public:
    ViewState() noexcept = default;
    
    // View angles (sent to server each frame)
    [[nodiscard]] const Vec3& viewAngles() const noexcept { return viewAngles_; }
    [[nodiscard]] Vec3& viewAngles() noexcept { return viewAngles_; }
    void setViewAngles(const Vec3& angles) noexcept { viewAngles_ = angles; }
    
    // Simulated position/velocity (from client prediction)
    [[nodiscard]] const Vec3& simOrg() const noexcept { return simOrg_; }
    [[nodiscard]] const Vec3& simVel() const noexcept { return simVel_; }
    [[nodiscard]] const Vec3& simAngles() const noexcept { return simAngles_; }
    
    void setSimOrg(const Vec3& org) noexcept { simOrg_ = org; }
    void setSimVel(const Vec3& vel) noexcept { simVel_ = vel; }
    void setSimAngles(const Vec3& angles) noexcept { simAngles_ = angles; }
    
    // Movement state
    [[nodiscard]] bool onGround() const noexcept { return onGround_; }
    [[nodiscard]] float crouch() const noexcept { return crouch_; }
    [[nodiscard]] int waterLevel() const noexcept { return waterLevel_; }
    
    void setOnGround(bool grounded) noexcept { onGround_ = grounded; }
    void setCrouch(float amount) noexcept { crouch_ = amount; }
    void setWaterLevel(int level) noexcept { waterLevel_ = level; }
    
    // Pitch drift
    [[nodiscard]] float pitchVel() const noexcept { return pitchVel_; }
    [[nodiscard]] bool noDrift() const noexcept { return noDrift_; }
    [[nodiscard]] float driftMove() const noexcept { return driftMove_; }
    [[nodiscard]] double lastStop() const noexcept { return lastStop_; }
    
    void setPitchVel(float vel) noexcept { pitchVel_ = vel; }
    void setNoDrift(bool nodrift) noexcept { noDrift_ = nodrift; }
    void setDriftMove(float move) noexcept { driftMove_ = move; }
    void setLastStop(double time) noexcept { lastStop_ = time; }
    
    // View effects
    [[nodiscard]] float idealPunchAngle() const noexcept { return idealPunchAngle_; }
    [[nodiscard]] float punchAngle() const noexcept { return punchAngle_; }
    [[nodiscard]] float rollAngle() const noexcept { return rollAngle_; }
    [[nodiscard]] float hurtBlur() const noexcept { return hurtBlur_; }
    
    void setIdealPunchAngle(float angle) noexcept { idealPunchAngle_ = angle; }
    void setPunchAngle(float angle) noexcept { punchAngle_ = angle; }
    void setRollAngle(float angle) noexcept { rollAngle_ = angle; }
    void setHurtBlur(float blur) noexcept { hurtBlur_ = blur; }
    
    // Color shifts
    [[nodiscard]] const ColorShift& colorShift(client::ColorShiftType type) const noexcept {
        return colorShifts_[static_cast<std::size_t>(type)];
    }
    [[nodiscard]] ColorShift& colorShift(client::ColorShiftType type) noexcept {
        return colorShifts_[static_cast<std::size_t>(type)];
    }
    
    void clear() {
        viewAngles_ = Vec3{};
        simOrg_ = Vec3{};
        simVel_ = Vec3{};
        simAngles_ = Vec3{};
        onGround_ = false;
        crouch_ = 0.0f;
        waterLevel_ = 0;
        pitchVel_ = 0.0f;
        noDrift_ = false;
        driftMove_ = 0.0f;
        lastStop_ = 0.0;
        idealPunchAngle_ = 0.0f;
        punchAngle_ = 0.0f;
        rollAngle_ = 0.0f;
        hurtBlur_ = 0.0f;
        for (auto& shift : colorShifts_) {
            shift = ColorShift{};
        }
    }
    
private:
    Vec3 viewAngles_;
    Vec3 simOrg_;
    Vec3 simVel_;
    Vec3 simAngles_;
    
    bool onGround_ = false;
    float crouch_ = 0.0f;
    int waterLevel_ = 0;
    
    float pitchVel_ = 0.0f;
    bool noDrift_ = false;
    float driftMove_ = 0.0f;
    double lastStop_ = 0.0;
    
    float idealPunchAngle_ = 0.0f;
    float punchAngle_ = 0.0f;
    float rollAngle_ = 0.0f;
    float hurtBlur_ = 0.0f;
    
    std::array<ColorShift, static_cast<std::size_t>(client::ColorShiftType::Count)> colorShifts_;
};

//=============================================================================
// Movement Physics
//=============================================================================

/**
 * @brief Localized movement physics parameters from server.
 */
class MovementParams {
public:
    MovementParams() noexcept = default;
    
    [[nodiscard]] float gravity() const noexcept { return gravity_; }
    [[nodiscard]] float maxSpeed() const noexcept { return maxSpeed_; }
    [[nodiscard]] float bunnySpeedCap() const noexcept { return bunnySpeedCap_; }
    
    void setGravity(float g) noexcept { gravity_ = g; }
    void setMaxSpeed(float speed) noexcept { maxSpeed_ = speed; }
    void setBunnySpeedCap(float cap) noexcept { bunnySpeedCap_ = cap; }
    
    void clear() noexcept {
        gravity_ = 800.0f;
        maxSpeed_ = 320.0f;
        bunnySpeedCap_ = 0.0f;
    }
    
private:
    float gravity_ = 800.0f;
    float maxSpeed_ = 320.0f;
    float bunnySpeedCap_ = 0.0f;
};

//=============================================================================
// Spectator State
//=============================================================================

/**
 * @brief Spectator mode and camera tracking state.
 */
class SpectatorState {
public:
    SpectatorState() noexcept = default;
    
    [[nodiscard]] bool isSpectator() const noexcept { return spectator_; }
    [[nodiscard]] client::CameraMode cameraMode() const noexcept { return cameraMode_; }
    [[nodiscard]] int trackedPlayer() const noexcept { return trackedPlayer_; }
    [[nodiscard]] int idealTrack() const noexcept { return idealTrack_; }
    [[nodiscard]] bool isLocked() const noexcept { return locked_; }
    
    void setSpectator(bool spec) noexcept { spectator_ = spec; }
    void setCameraMode(client::CameraMode mode) noexcept { cameraMode_ = mode; }
    void setTrackedPlayer(int player) noexcept { trackedPlayer_ = player; }
    void setIdealTrack(int player) noexcept { idealTrack_ = player; }
    void setLocked(bool locked) noexcept { locked_ = locked; }
    
    [[nodiscard]] bool isTracking() const noexcept {
        return cameraMode_ == client::CameraMode::Track;
    }
    
    void clear() noexcept {
        spectator_ = false;
        cameraMode_ = client::CameraMode::None;
        trackedPlayer_ = 0;
        idealTrack_ = 0;
        locked_ = false;
    }
    
private:
    bool spectator_ = false;
    client::CameraMode cameraMode_ = client::CameraMode::None;
    int trackedPlayer_ = 0;
    int idealTrack_ = 0;
    bool locked_ = false;
};

//=============================================================================
// Game State Flags
//=============================================================================

/**
 * @brief Various game state flags.
 */
class GameStateFlags {
public:
    // Pause flags
    static constexpr int PAUSED_SERVER = 1;
    static constexpr int PAUSED_DEMO = 2;
    
    GameStateFlags() noexcept = default;
    
    [[nodiscard]] bool isPaused() const noexcept { return pauseFlags_ != 0; }
    [[nodiscard]] bool isServerPaused() const noexcept { return (pauseFlags_ & PAUSED_SERVER) != 0; }
    [[nodiscard]] bool isDemoPaused() const noexcept { return (pauseFlags_ & PAUSED_DEMO) != 0; }
    [[nodiscard]] int pauseFlags() const noexcept { return pauseFlags_; }
    
    void setPauseFlags(int flags) noexcept { pauseFlags_ = flags; }
    void addPauseFlag(int flag) noexcept { pauseFlags_ |= flag; }
    void removePauseFlag(int flag) noexcept { pauseFlags_ &= ~flag; }
    
    [[nodiscard]] bool standby() const noexcept { return standby_; }
    [[nodiscard]] bool countdown() const noexcept { return countdown_; }
    [[nodiscard]] bool racing() const noexcept { return racing_; }
    [[nodiscard]] int intermission() const noexcept { return intermission_; }
    
    void setStandby(bool val) noexcept { standby_ = val; }
    void setCountdown(bool val) noexcept { countdown_ = val; }
    void setRacing(bool val) noexcept { racing_ = val; }
    void setIntermission(int val) noexcept { intermission_ = val; }
    
    void clear() noexcept {
        pauseFlags_ = 0;
        standby_ = false;
        countdown_ = false;
        racing_ = false;
        intermission_ = 0;
    }
    
private:
    int pauseFlags_ = 0;
    bool standby_ = false;
    bool countdown_ = false;
    bool racing_ = false;
    int intermission_ = 0;
};

//=============================================================================
// ClientState - Per-level state wiped at every server signon
//=============================================================================

/**
 * @brief Per-level client state that is wiped at every server signon.
 * 
 * This contains all game state for the current level: player info,
 * view state, entity state, etc.
 * 
 * Original: clientState_t (cl)
 */
class ClientState {
public:
    ClientState() noexcept = default;
    
    // Timing
    [[nodiscard]] double time() const noexcept { return time_; }
    [[nodiscard]] double serverTime() const noexcept { return serverTime_; }
    [[nodiscard]] bool serverTimeWorks() const noexcept { return serverTimeWorks_; }
    [[nodiscard]] double gameTime() const noexcept { return gameTime_; }
    [[nodiscard]] double gameStartTime() const noexcept { return gameStartTime_; }
    [[nodiscard]] double gamePauseTime() const noexcept { return gamePauseTime_; }
    [[nodiscard]] double lastPingRequest() const noexcept { return lastPingRequest_; }
    
    void setTime(double t) noexcept { time_ = t; }
    void setServerTime(double t) noexcept { serverTime_ = t; }
    void setServerTimeWorks(bool works) noexcept { serverTimeWorks_ = works; }
    void setGameTime(double t) noexcept { gameTime_ = t; }
    void setGameStartTime(double t) noexcept { gameStartTime_ = t; }
    void setGamePauseTime(double t) noexcept { gamePauseTime_ = t; }
    void setLastPingRequest(double t) noexcept { lastPingRequest_ = t; }
    
    // Player identification
    [[nodiscard]] int playerNum() const noexcept { return playerNum_; }
    [[nodiscard]] int viewPlayerNum() const noexcept { return viewPlayerNum_; }
    void setPlayerNum(int num) noexcept { playerNum_ = num; }
    void setViewPlayerNum(int num) noexcept { viewPlayerNum_ = num; }
    
    // Level info
    [[nodiscard]] std::string_view levelName() const noexcept { return levelName_; }
    [[nodiscard]] int cdTrack() const noexcept { return cdTrack_; }
    [[nodiscard]] unsigned int mapChecksum() const noexcept { return mapChecksum_; }
    
    void setLevelName(std::string_view name) { levelName_ = name; }
    void setCdTrack(int track) noexcept { cdTrack_ = track; }
    void setMapChecksum(unsigned int checksum) noexcept { mapChecksum_ = checksum; }
    
    // Stats array
    [[nodiscard]] int stat(int index) const noexcept {
        if (index >= 0 && index < client::MAX_CL_STATS) {
            return stats_[static_cast<std::size_t>(index)];
        }
        return 0;
    }
    void setStat(int index, int value) noexcept {
        if (index >= 0 && index < client::MAX_CL_STATS) {
            stats_[static_cast<std::size_t>(index)] = value;
        }
    }
    
    // Sub-state accessors
    [[nodiscard]] ServerInfo& serverInfo() noexcept { return serverInfo_; }
    [[nodiscard]] const ServerInfo& serverInfo() const noexcept { return serverInfo_; }
    
    [[nodiscard]] SequenceState& sequence() noexcept { return sequence_; }
    [[nodiscard]] const SequenceState& sequence() const noexcept { return sequence_; }
    
    [[nodiscard]] ViewState& view() noexcept { return view_; }
    [[nodiscard]] const ViewState& view() const noexcept { return view_; }
    
    [[nodiscard]] MovementParams& movement() noexcept { return movement_; }
    [[nodiscard]] const MovementParams& movement() const noexcept { return movement_; }
    
    [[nodiscard]] SpectatorState& spectator() noexcept { return spectator_; }
    [[nodiscard]] const SpectatorState& spectator() const noexcept { return spectator_; }
    
    [[nodiscard]] GameStateFlags& gameState() noexcept { return gameState_; }
    [[nodiscard]] const GameStateFlags& gameState() const noexcept { return gameState_; }
    
    [[nodiscard]] FrameBuffer& frames() noexcept { return frames_; }
    [[nodiscard]] const FrameBuffer& frames() const noexcept { return frames_; }
    
    // Players
    [[nodiscard]] PlayerInfo& player(int index) noexcept {
        return players_[static_cast<std::size_t>(index)];
    }
    [[nodiscard]] const PlayerInfo& player(int index) const noexcept {
        return players_[static_cast<std::size_t>(index)];
    }
    
    // Static sounds
    [[nodiscard]] std::size_t numStaticSounds() const noexcept { return staticSounds_.size(); }
    void addStaticSound(const StaticSound& sound) {
        if (staticSounds_.size() < client::MAX_STATIC_SOUNDS) {
            staticSounds_.push_back(sound);
        }
    }
    [[nodiscard]] const StaticSound& staticSound(std::size_t index) const noexcept {
        return staticSounds_[index];
    }
    
    // Light styles
    void setLightStyle(int index, std::string_view pattern) {
        if (index >= 0 && index < static_cast<int>(lightStyles_.size())) {
            lightStyles_[static_cast<std::size_t>(index)].setPattern(pattern);
        }
    }
    [[nodiscard]] const LightStyle& lightStyle(int index) const noexcept {
        return lightStyles_[static_cast<std::size_t>(index)];
    }
    
    // Dynamic lights
    [[nodiscard]] DynamicLight& dynamicLight(int index) noexcept {
        return dlights_[static_cast<std::size_t>(index)];
    }
    [[nodiscard]] const DynamicLight& dynamicLight(int index) const noexcept {
        return dlights_[static_cast<std::size_t>(index)];
    }
    
    DynamicLight* allocDynamicLight(int key) noexcept {
        // First look for an existing light with this key
        for (auto& light : dlights_) {
            if (light.key() == key) {
                return &light;
            }
        }
        // Find a free slot (radius == 0)
        for (auto& light : dlights_) {
            if (light.radius() <= 0.0f) {
                light.clear();
                light.setKey(key);
                return &light;
            }
        }
        // All slots full, return first
        dlights_[0].clear();
        dlights_[0].setKey(key);
        return &dlights_[0];
    }
    
    // Clear all state (called on level change)
    void clear() {
        time_ = 0.0;
        serverTime_ = 0.0;
        serverTimeWorks_ = false;
        gameTime_ = 0.0;
        gameStartTime_ = 0.0;
        gamePauseTime_ = 0.0;
        lastPingRequest_ = 0.0;
        playerNum_ = 0;
        viewPlayerNum_ = 0;
        levelName_.clear();
        cdTrack_ = 0;
        mapChecksum_ = 0;
        stats_.fill(0);
        
        serverInfo_.clear();
        sequence_.clear();
        view_.clear();
        movement_.clear();
        spectator_.clear();
        gameState_.clear();
        frames_.clear();
        
        for (auto& player : players_) {
            player.clear();
        }
        
        staticSounds_.clear();
        
        for (auto& style : lightStyles_) {
            style = LightStyle{};
        }
        
        for (auto& light : dlights_) {
            light.clear();
        }
    }
    
private:
    // Timing
    double time_ = 0.0;
    double serverTime_ = 0.0;
    bool serverTimeWorks_ = false;
    double gameTime_ = 0.0;
    double gameStartTime_ = 0.0;
    double gamePauseTime_ = 0.0;
    double lastPingRequest_ = 0.0;
    
    // Player identification
    int playerNum_ = 0;
    int viewPlayerNum_ = 0;
    
    // Level info
    std::string levelName_;
    int cdTrack_ = 0;
    unsigned int mapChecksum_ = 0;
    
    // Stats
    std::array<int, client::MAX_CL_STATS> stats_ = {};
    
    // Sub-states
    ServerInfo serverInfo_;
    SequenceState sequence_;
    ViewState view_;
    MovementParams movement_;
    SpectatorState spectator_;
    GameStateFlags gameState_;
    FrameBuffer frames_;
    
    // Players
    std::array<PlayerInfo, client::MAX_CLIENTS> players_;
    
    // Static sounds
    std::vector<StaticSound> staticSounds_;
    
    // Light styles (MAX_LIGHTSTYLES = 64)
    std::array<LightStyle, 64> lightStyles_;
    
    // Dynamic lights
    std::array<DynamicLight, client::MAX_DLIGHTS> dlights_;
};

} // namespace ezquake
