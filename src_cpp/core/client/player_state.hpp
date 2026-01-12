/*
 * ezQuake C++ Port - Player State Types
 * 
 * Player state for prediction and rendering.
 * 
 * Copyright (C) 2024 ezQuake Team
 */

#pragma once

#include "core/types.hpp"
#include "core/math/vec3.hpp"
#include "core/client/client_types.hpp"
#include <array>
#include <cstdint>
#include <string>
#include <string_view>

namespace ezquake {

//=============================================================================
// PlayerState - Per-frame player state for prediction/rendering
//=============================================================================

/**
 * @brief Player state at a specific point in time.
 * 
 * This is the information needed by a player entity to do move prediction
 * and to generate a drawable entity.
 * 
 * Original: player_state_t
 */
class PlayerState {
public:
    // Physics movement types
    enum class MoveType : std::uint8_t {
        Normal = 0,
        Fly,
        NoClip,
        Dead,
        Spectator
    };
    
    // Antilag debug flags
    enum class AntilagFlag : std::uint8_t {
        None = 0,
        RewindPresent = 1,
        ClientPresent = 2,
        PositionSet = 4
    };
    
    constexpr PlayerState() noexcept = default;
    
    // Message sequence
    [[nodiscard]] constexpr int messageNum() const noexcept { return messageNum_; }
    constexpr void setMessageNum(int num) noexcept { messageNum_ = num; }
    
    // State time
    [[nodiscard]] constexpr double stateTime() const noexcept { return stateTime_; }
    constexpr void setStateTime(double time) noexcept { stateTime_ = time; }
    
    // Last command for prediction
    [[nodiscard]] constexpr const UserCommand& command() const noexcept { return command_; }
    [[nodiscard]] constexpr UserCommand& command() noexcept { return command_; }
    constexpr void setCommand(const UserCommand& cmd) noexcept { command_ = cmd; }
    
    // Origin/Position
    [[nodiscard]] constexpr const Vec3& origin() const noexcept { return origin_; }
    constexpr void setOrigin(const Vec3& origin) noexcept { origin_ = origin; }
    
    // View angles (only for demos, not from server)
    [[nodiscard]] constexpr const Vec3& viewAngles() const noexcept { return viewAngles_; }
    constexpr void setViewAngles(const Vec3& angles) noexcept { viewAngles_ = angles; }
    
    // Velocity
    [[nodiscard]] constexpr const Vec3& velocity() const noexcept { return velocity_; }
    constexpr void setVelocity(const Vec3& vel) noexcept { velocity_ = vel; }
    
    // Model info
    [[nodiscard]] constexpr int modelIndex() const noexcept { return modelIndex_; }
    [[nodiscard]] constexpr int frame() const noexcept { return frame_; }
    [[nodiscard]] constexpr int skinNum() const noexcept { return skinNum_; }
    [[nodiscard]] constexpr int weaponFrame() const noexcept { return weaponFrame_; }
    [[nodiscard]] constexpr int effects() const noexcept { return effects_; }
    [[nodiscard]] constexpr int flags() const noexcept { return flags_; }
    
    constexpr void setModelIndex(int index) noexcept { modelIndex_ = index; }
    constexpr void setFrame(int frame) noexcept { frame_ = frame; }
    constexpr void setSkinNum(int skin) noexcept { skinNum_ = skin; }
    constexpr void setWeaponFrame(int frame) noexcept { weaponFrame_ = frame; }
    constexpr void setEffects(int effects) noexcept { effects_ = effects; }
    constexpr void setFlags(int flags) noexcept { flags_ = flags; }
    
    // Transparency (FTE extension)
    [[nodiscard]] constexpr std::uint8_t alpha() const noexcept { return alpha_; }
    constexpr void setAlpha(std::uint8_t alpha) noexcept { alpha_ = alpha; }
    
    // Color mod (FTE extension)
    [[nodiscard]] constexpr std::uint8_t colorModR() const noexcept { return colorMod_[0]; }
    [[nodiscard]] constexpr std::uint8_t colorModG() const noexcept { return colorMod_[1]; }
    [[nodiscard]] constexpr std::uint8_t colorModB() const noexcept { return colorMod_[2]; }
    constexpr void setColorMod(std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept {
        colorMod_[0] = r;
        colorMod_[1] = g;
        colorMod_[2] = b;
    }
    
    // VWep (visible weapon) index
    [[nodiscard]] constexpr std::uint8_t vwIndex() const noexcept { return vwIndex_; }
    constexpr void setVwIndex(std::uint8_t index) noexcept { vwIndex_ = index; }
    
    // Movement type
    [[nodiscard]] constexpr MoveType moveType() const noexcept { return moveType_; }
    constexpr void setMoveType(MoveType type) noexcept { moveType_ = type; }
    
    // Water jump time
    [[nodiscard]] constexpr float waterJumpTime() const noexcept { return waterJumpTime_; }
    constexpr void setWaterJumpTime(float time) noexcept { waterJumpTime_ = time; }
    
    // Ground state
    [[nodiscard]] constexpr bool onGround() const noexcept { return onGround_; }
    constexpr void setOnGround(bool grounded) noexcept { onGround_ = grounded; }
    
    // Jump state
    [[nodiscard]] constexpr bool jumpHeld() const noexcept { return jumpHeld_; }
    [[nodiscard]] constexpr int jumpMsec() const noexcept { return jumpMsec_; }
    constexpr void setJumpHeld(bool held) noexcept { jumpHeld_ = held; }
    constexpr void setJumpMsec(int msec) noexcept { jumpMsec_ = msec; }
    
    // Antilag positions
    [[nodiscard]] constexpr const Vec3& currentOrigin() const noexcept { return currentOrigin_; }
    [[nodiscard]] constexpr const Vec3& rewindOrigin() const noexcept { return rewindOrigin_; }
    [[nodiscard]] constexpr const Vec3& clientOrigin() const noexcept { return clientOrigin_; }
    [[nodiscard]] constexpr int antilagFlags() const noexcept { return antilagFlags_; }
    
    constexpr void setCurrentOrigin(const Vec3& origin) noexcept { currentOrigin_ = origin; }
    constexpr void setRewindOrigin(const Vec3& origin) noexcept { rewindOrigin_ = origin; }
    constexpr void setClientOrigin(const Vec3& origin) noexcept { clientOrigin_ = origin; }
    constexpr void setAntilagFlags(int flags) noexcept { antilagFlags_ = flags; }
    
    // Convenience methods
    [[nodiscard]] constexpr bool isDead() const noexcept {
        return moveType_ == MoveType::Dead;
    }
    
    [[nodiscard]] constexpr bool isSpectator() const noexcept {
        return moveType_ == MoveType::Spectator;
    }
    
    // Clear all state
    void clear() noexcept {
        messageNum_ = 0;
        stateTime_ = 0.0;
        command_.clear();
        origin_ = Vec3{};
        viewAngles_ = Vec3{};
        velocity_ = Vec3{};
        weaponFrame_ = 0;
        modelIndex_ = 0;
        frame_ = 0;
        skinNum_ = 0;
        effects_ = 0;
        flags_ = 0;
        alpha_ = 255;
        colorMod_ = {255, 255, 255};
        vwIndex_ = 0;
        moveType_ = MoveType::Normal;
        waterJumpTime_ = 0.0f;
        onGround_ = false;
        jumpHeld_ = false;
        jumpMsec_ = 0;
        currentOrigin_ = Vec3{};
        rewindOrigin_ = Vec3{};
        clientOrigin_ = Vec3{};
        antilagFlags_ = 0;
    }
    
private:
    int messageNum_ = 0;                    // Not all players updated each frame
    double stateTime_ = 0.0;                // Not same as packet time
    UserCommand command_;                   // Last command for prediction
    
    Vec3 origin_;                           // World position
    Vec3 viewAngles_;                       // Only for demos, not from server
    Vec3 velocity_;                         // Movement velocity
    
    int weaponFrame_ = 0;
    int modelIndex_ = 0;
    int frame_ = 0;
    int skinNum_ = 0;
    int effects_ = 0;
    int flags_ = 0;                         // Dead, gib, etc.
    
    std::uint8_t alpha_ = 255;              // FTE transparency
    std::array<std::uint8_t, 3> colorMod_ = {255, 255, 255};  // FTE color mod
    
    std::uint8_t vwIndex_ = 0;              // Visible weapon index
    MoveType moveType_ = MoveType::Normal;
    float waterJumpTime_ = 0.0f;
    bool onGround_ = false;
    bool jumpHeld_ = false;
    int jumpMsec_ = 0;                      // Fix bunny-hop flickering
    
    // Antilag debug info
    Vec3 currentOrigin_;                    // Current location (no antilag applied)
    Vec3 rewindOrigin_;                     // Location antilag server has rewound to
    Vec3 clientOrigin_;                     // Location client rendered the player
    int antilagFlags_ = 0;                  // Bitmask of antilag debug flags
};

//=============================================================================
// PlayerInfo - Scoreboard and player metadata
//=============================================================================

/**
 * @brief Player information for scoreboard and rendering.
 * 
 * Contains all metadata about a player: name, team, colors, stats, etc.
 * 
 * Original: player_info_t
 */
class PlayerInfo {
public:
    static constexpr std::size_t MAX_NAME_LENGTH = client::MAX_SCOREBOARDNAME;
    
    PlayerInfo() noexcept = default;
    
    // Identity
    [[nodiscard]] int userId() const noexcept { return userId_; }
    [[nodiscard]] std::string_view name() const noexcept { return name_; }
    [[nodiscard]] std::string_view shortName() const noexcept { return shortName_; }
    [[nodiscard]] std::string_view team() const noexcept { return team_; }
    [[nodiscard]] std::string_view userInfo() const noexcept { return userInfo_; }
    
    void setUserId(int id) noexcept { userId_ = id; }
    void setName(std::string_view name) { name_ = name.substr(0, MAX_NAME_LENGTH); }
    void setShortName(std::string_view name) { shortName_ = name.substr(0, MAX_NAME_LENGTH); }
    void setTeam(std::string_view team) { team_ = team; }
    void setUserInfo(std::string_view info) { userInfo_ = info; }
    
    // Scoreboard stats
    [[nodiscard]] float enterTime() const noexcept { return enterTime_; }
    [[nodiscard]] int frags() const noexcept { return frags_; }
    [[nodiscard]] int ping() const noexcept { return ping_; }
    [[nodiscard]] std::uint8_t packetLoss() const noexcept { return packetLoss_; }
    [[nodiscard]] bool isSpectator() const noexcept { return spectator_; }
    
    void setEnterTime(float time) noexcept { enterTime_ = time; }
    void setFrags(int frags) noexcept { frags_ = frags; }
    void setPing(int ping) noexcept { ping_ = ping; }
    void setPacketLoss(std::uint8_t pl) noexcept { packetLoss_ = pl; }
    void setSpectator(bool spec) noexcept { spectator_ = spec; }
    
    // Colors
    [[nodiscard]] std::uint8_t topColor() const noexcept { return topColor_; }
    [[nodiscard]] std::uint8_t bottomColor() const noexcept { return bottomColor_; }
    [[nodiscard]] std::uint8_t realTopColor() const noexcept { return realTopColor_; }
    [[nodiscard]] std::uint8_t realBottomColor() const noexcept { return realBottomColor_; }
    
    void setTopColor(std::uint8_t color) noexcept { topColor_ = color; }
    void setBottomColor(std::uint8_t color) noexcept { bottomColor_ = color; }
    void setRealTopColor(std::uint8_t color) noexcept { realTopColor_ = color; }
    void setRealBottomColor(std::uint8_t color) noexcept { realBottomColor_ = color; }
    
    // Team info
    [[nodiscard]] bool isTeammate() const noexcept { return teammate_; }
    [[nodiscard]] int knownTeamColor() const noexcept { return knownTeamColor_; }
    void setTeammate(bool tm) noexcept { teammate_ = tm; }
    void setKnownTeamColor(int color) noexcept { knownTeamColor_ = color; }
    
    // FPS tracking
    [[nodiscard]] int fps() const noexcept { return fps_; }
    [[nodiscard]] int lastFps() const noexcept { return lastFps_; }
    void setFps(int fps) noexcept { fps_ = fps; }
    void setLastFps(int fps) noexcept { lastFps_ = fps; }
    
    // State flags
    [[nodiscard]] bool isDead() const noexcept { return dead_; }
    [[nodiscard]] bool needsSkinRefresh() const noexcept { return skinRefresh_; }
    [[nodiscard]] bool isIgnored() const noexcept { return ignored_; }
    [[nodiscard]] bool isVoipIgnored() const noexcept { return vignored_; }
    [[nodiscard]] bool isNear() const noexcept { return isNear_; }
    
    void setDead(bool dead) noexcept { dead_ = dead; }
    void setSkinRefresh(bool refresh) noexcept { skinRefresh_ = refresh; }
    void setIgnored(bool ignored) noexcept { ignored_ = ignored; }
    void setVoipIgnored(bool ignored) noexcept { vignored_ = ignored; }
    void setNear(bool near) noexcept { isNear_ = near; }
    
    // Gender
    [[nodiscard]] client::Gender gender() const noexcept { return gender_; }
    void setGender(client::Gender g) noexcept { gender_ = g; }
    
    // Stats array access
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
    
    // Health tracking for HUD
    [[nodiscard]] int maxHealth() const noexcept { return maxHealth_; }
    [[nodiscard]] int prevHealth() const noexcept { return prevHealth_; }
    [[nodiscard]] double maxHealthLastSet() const noexcept { return maxHealthLastSet_; }
    [[nodiscard]] double prevHealthLastSet() const noexcept { return prevHealthLastSet_; }
    
    void setMaxHealth(int health, double time) noexcept {
        maxHealth_ = health;
        maxHealthLastSet_ = time;
    }
    void setPrevHealth(int health, double time) noexcept {
        prevHealth_ = health;
        prevHealthLastSet_ = time;
    }
    
    // Authentication info
    [[nodiscard]] std::string_view loginName() const noexcept { return loginName_; }
    [[nodiscard]] std::string_view loginFlag() const noexcept { return loginFlag_; }
    [[nodiscard]] int loginFlagId() const noexcept { return loginFlagId_; }
    
    void setLoginName(std::string_view name) { loginName_ = name.substr(0, MAX_NAME_LENGTH); }
    void setLoginFlag(std::string_view flag) { loginFlag_ = flag; }
    void setLoginFlagId(int id) noexcept { loginFlagId_ = id; }
    
    // Chat flag
    [[nodiscard]] int chatFlag() const noexcept { return chatFlag_; }
    void setChatFlag(int flag) noexcept { chatFlag_ = flag; }
    
    // Clear all data
    void clear() {
        userId_ = 0;
        name_.clear();
        shortName_.clear();
        team_.clear();
        userInfo_.clear();
        enterTime_ = 0.0f;
        frags_ = 0;
        ping_ = 0;
        packetLoss_ = 0;
        spectator_ = false;
        topColor_ = 0;
        bottomColor_ = 0;
        realTopColor_ = 0;
        realBottomColor_ = 0;
        teammate_ = false;
        knownTeamColor_ = 0;
        fps_ = 0;
        lastFps_ = 0;
        dead_ = false;
        skinRefresh_ = false;
        ignored_ = false;
        vignored_ = false;
        isNear_ = false;
        gender_ = client::Gender::Unknown;
        stats_.fill(0);
        maxHealth_ = 0;
        prevHealth_ = 0;
        maxHealthLastSet_ = 0.0;
        prevHealthLastSet_ = 0.0;
        loginName_.clear();
        loginFlag_.clear();
        loginFlagId_ = 0;
        chatFlag_ = 0;
    }
    
private:
    int userId_ = 0;
    std::string name_;
    std::string shortName_;
    std::string team_;
    std::string userInfo_;
    
    float enterTime_ = 0.0f;
    int frags_ = 0;
    int ping_ = 0;
    std::uint8_t packetLoss_ = 0;
    bool spectator_ = false;
    
    std::uint8_t topColor_ = 0;
    std::uint8_t bottomColor_ = 0;
    std::uint8_t realTopColor_ = 0;
    std::uint8_t realBottomColor_ = 0;
    bool teammate_ = false;
    int knownTeamColor_ = 0;
    
    int fps_ = 0;
    int lastFps_ = 0;
    
    bool dead_ = false;
    bool skinRefresh_ = false;
    bool ignored_ = false;
    bool vignored_ = false;
    bool isNear_ = false;
    
    client::Gender gender_ = client::Gender::Unknown;
    
    std::array<int, client::MAX_CL_STATS> stats_ = {};
    
    int maxHealth_ = 0;
    int prevHealth_ = 0;
    double maxHealthLastSet_ = 0.0;
    double prevHealthLastSet_ = 0.0;
    
    std::string loginName_;
    std::string loginFlag_;
    int loginFlagId_ = 0;
    int chatFlag_ = 0;
};

} // namespace ezquake
