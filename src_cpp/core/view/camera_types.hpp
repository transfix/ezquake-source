/**
 * @file camera_types.hpp
 * @brief Camera control and spectator tracking type definitions
 * 
 * Converted from cl_cam.c and vx_camera.c
 * Core types for camera modes, spectator tracking, and third-person views.
 */

#pragma once

#include <cstdint>
#include <array>
#include <optional>
#include <string_view>
#include <algorithm>
#include <cmath>

namespace ezquake {

//=============================================================================
// Camera Mode Types (from cl_cam.c and vx_camera.c)
//=============================================================================

/**
 * @brief Spectator camera mode
 * 
 * Original: CAM_NONE, CAM_TRACK
 */
enum class SpectatorCameraMode : uint8_t {
    None = 0,   ///< Free-flying spectator (CAM_NONE)
    Track = 1   ///< Tracking a player (CAM_TRACK)
};

/**
 * @brief Get name of spectator camera mode
 */
constexpr std::string_view spectatorCameraModeName(SpectatorCameraMode mode) noexcept {
    switch (mode) {
        case SpectatorCameraMode::None:  return "none";
        case SpectatorCameraMode::Track: return "track";
        default:                         return "unknown";
    }
}

/**
 * @brief External camera mode types
 * 
 * Original: cameramode_t in vx_camera.c
 */
enum class ExternalCameraMode : uint8_t {
    Normal = 0,     ///< First-person view (C_NORMAL)
    ChaseCam = 1,   ///< Third-person chase camera (C_CHASECAM)
    External = 2    ///< External fixed camera (C_EXTERNAL)
};

/**
 * @brief Get name of external camera mode
 */
constexpr std::string_view externalCameraModeName(ExternalCameraMode mode) noexcept {
    switch (mode) {
        case ExternalCameraMode::Normal:   return "normal";
        case ExternalCameraMode::ChaseCam: return "chasecam";
        case ExternalCameraMode::External: return "external";
        default:                           return "unknown";
    }
}

//=============================================================================
// Camera Constants
//=============================================================================

namespace camera {
    constexpr float DEFAULT_CHASE_DIST = 100.0f;   ///< amf_camera_chase_dist default
    constexpr float DEFAULT_CHASE_HEIGHT = 32.0f;  ///< amf_camera_chase_height default
    constexpr float FLYBY_MIN_DIST = 32.0f;        ///< Minimum flyby distance
    constexpr float FLYBY_MAX_DIST = 800.0f;       ///< Maximum flyby distance
    constexpr float LOCK_TIMEOUT = 0.3f;           ///< Time before lock expires
    constexpr float LOCK_COOLDOWN = 0.1f;          ///< Minimum time between locks
    constexpr float VISIBILITY_UPDATE = 0.1f;      ///< Visibility check interval
}

//=============================================================================
// Spectator Tracking State
//=============================================================================

/**
 * @brief Player tracking state for spectator mode
 * 
 * Original: Various cl.* fields and static variables in cl_cam.c
 */
struct SpectatorTrackingState {
    SpectatorCameraMode autocam{SpectatorCameraMode::None};  ///< Current camera mode
    int trackNum{0};            ///< Player number being tracked (spec_track)
    int idealTrack{0};          ///< Ideal player to track (ideal_track)
    bool locked{false};         ///< Whether locked onto target (spec_locked)
    double lastLockTime{0.0};   ///< Last time lock was acquired
    double lastViewTime{0.0};   ///< Last time visibility was updated
    int killerNum{-1};          ///< Player who killed tracked player
    
    // View angles when tracking
    std::array<float, 3> viewAngles{0.0f, 0.0f, 0.0f};
    
    /**
     * @brief Check if currently tracking a player
     */
    [[nodiscard]] bool isTracking() const noexcept {
        return autocam == SpectatorCameraMode::Track;
    }
    
    /**
     * @brief Check if locked onto a player
     */
    [[nodiscard]] bool isLocked() const noexcept {
        return locked && isTracking();
    }
    
    /**
     * @brief Lock onto a player
     */
    void lock(int playerNum, double currentTime) noexcept {
        trackNum = playerNum;
        locked = false;  // Will be set true when position established
        lastLockTime = currentTime;
    }
    
    /**
     * @brief Unlock from current player
     */
    void unlock() noexcept {
        autocam = SpectatorCameraMode::None;
        locked = false;
    }
    
    /**
     * @brief Set the killer for trackkiller functionality
     */
    void setKiller(int killer, int victim, int viewPlayer) noexcept {
        if (victim != viewPlayer) return;
        if (killer < 0 || killer >= 32) return;  // MAX_CLIENTS
        killerNum = killer;
    }
    
    /**
     * @brief Reset tracking state
     */
    void reset() noexcept {
        autocam = SpectatorCameraMode::None;
        trackNum = 0;
        idealTrack = 0;
        locked = false;
        lastLockTime = 0.0;
        lastViewTime = 0.0;
        killerNum = -1;
        viewAngles = {0.0f, 0.0f, 0.0f};
    }
};

//=============================================================================
// Chase Camera State
//=============================================================================

/**
 * @brief Chase camera configuration and state
 */
struct ChaseCameraState {
    bool enabled{true};                 ///< cl_chasecam - through-the-eyes view
    bool thirdPerson{false};            ///< cam_thirdperson
    bool tppEnabled{false};             ///< cl_camera_tpp
    float distance{camera::DEFAULT_CHASE_DIST};    ///< Chase distance
    float height{camera::DEFAULT_CHASE_HEIGHT};    ///< Chase height offset
    
    /**
     * @brief Check if in first-person chase mode
     */
    [[nodiscard]] bool isFirstPerson() const noexcept {
        return enabled && !thirdPerson && !tppEnabled;
    }
    
    /**
     * @brief Check if should show tracked player model
     */
    [[nodiscard]] bool shouldShowPlayer() const noexcept {
        return !isFirstPerson();
    }
};

//=============================================================================
// External Camera State
//=============================================================================

/**
 * @brief External/cinematic camera state
 * 
 * Original: Various static variables in vx_camera.c
 */
struct ExternalCameraState {
    ExternalCameraMode mode{ExternalCameraMode::Normal};
    
    // Camera position and angles
    std::array<float, 3> position{0.0f, 0.0f, 0.0f};
    std::array<float, 3> angles{0.0f, 0.0f, 0.0f};
    std::array<float, 3> targetPosition{0.0f, 0.0f, 0.0f};
    std::array<float, 3> targetAngles{0.0f, 0.0f, 0.0f};
    float speed{100.0f};
    
    /**
     * @brief Check if using external camera
     */
    [[nodiscard]] bool isExternal() const noexcept {
        return mode != ExternalCameraMode::Normal;
    }
    
    /**
     * @brief Check if in chase cam mode
     */
    [[nodiscard]] bool isChaseCam() const noexcept {
        return mode == ExternalCameraMode::ChaseCam;
    }
    
    /**
     * @brief Set camera to normal first-person
     */
    void setNormal() noexcept {
        mode = ExternalCameraMode::Normal;
    }
    
    /**
     * @brief Set camera to chase cam mode
     */
    void setChaseCam() noexcept {
        mode = ExternalCameraMode::ChaseCam;
    }
    
    /**
     * @brief Set camera to external mode at position
     */
    void setExternal(const std::array<float, 3>& pos) noexcept {
        mode = ExternalCameraMode::External;
        position = pos;
    }
    
    /**
     * @brief Reset to default state
     */
    void reset() noexcept {
        mode = ExternalCameraMode::Normal;
        position = {0.0f, 0.0f, 0.0f};
        angles = {0.0f, 0.0f, 0.0f};
        targetPosition = {0.0f, 0.0f, 0.0f};
        targetAngles = {0.0f, 0.0f, 0.0f};
    }
};

//=============================================================================
// Flyby Camera Position Finder
//=============================================================================

/**
 * @brief Desired camera position for flyby spectating
 */
struct FlybyPosition {
    std::array<float, 3> position{0.0f, 0.0f, 0.0f};
    float distance{camera::FLYBY_MAX_DIST};
    bool valid{false};
    
    /**
     * @brief Check if this is a valid position
     */
    [[nodiscard]] bool isValid() const noexcept {
        return valid && distance >= camera::FLYBY_MIN_DIST && distance <= camera::FLYBY_MAX_DIST;
    }
    
    /**
     * @brief Check if this position is better than another
     */
    [[nodiscard]] bool isBetterThan(const FlybyPosition& other) const noexcept {
        if (!valid) return false;
        if (!other.valid) return true;
        return distance < other.distance;
    }
};

//=============================================================================
// Multiview Camera State
//=============================================================================

/**
 * @brief Multiview camera tracking slot
 */
struct MultiviewSlot {
    int trackNum{-1};           ///< Player being tracked (-1 = none)
    bool active{false};         ///< Whether slot is active
    
    /**
     * @brief Check if tracking a valid player
     */
    [[nodiscard]] bool isTracking() const noexcept {
        return active && trackNum >= 0;
    }
    
    /**
     * @brief Set tracked player
     */
    void setTrack(int playerNum) noexcept {
        trackNum = playerNum;
        active = (playerNum >= 0);
    }
    
    /**
     * @brief Clear tracking
     */
    void clear() noexcept {
        trackNum = -1;
        active = false;
    }
};

constexpr size_t MAX_MULTIVIEW_SLOTS = 4;

/**
 * @brief Multiview state for watching multiple players
 */
struct MultiviewState {
    std::array<MultiviewSlot, MAX_MULTIVIEW_SLOTS> slots;
    bool enabled{false};
    int activeSlot{0};
    
    /**
     * @brief Get number of active views
     */
    [[nodiscard]] size_t activeCount() const noexcept {
        size_t count = 0;
        for (const auto& slot : slots) {
            if (slot.isTracking()) ++count;
        }
        return count;
    }
    
    /**
     * @brief Set track for a specific slot
     */
    void setSlotTrack(int slot, int playerNum) noexcept {
        if (slot >= 0 && static_cast<size_t>(slot) < MAX_MULTIVIEW_SLOTS) {
            slots[static_cast<size_t>(slot)].setTrack(playerNum);
        }
    }
    
    /**
     * @brief Reset all multiview state
     */
    void reset() noexcept {
        for (auto& slot : slots) {
            slot.clear();
        }
        enabled = false;
        activeSlot = 0;
    }
};

//=============================================================================
// High Tracker State
//=============================================================================

/**
 * @brief cl_hightrack - automatically track high fragger
 */
struct HighTrackerState {
    bool enabled{false};        ///< cl_hightrack
    int lastHighPlayer{-1};     ///< Last known high fragger
    int lastHighScore{-9999};   ///< Last known high score
    
    /**
     * @brief Check if player is higher scorer
     */
    [[nodiscard]] bool isHigherScore(int playerNum, int score) const noexcept {
        return score > lastHighScore;
    }
    
    /**
     * @brief Update high tracker
     */
    void update(int playerNum, int score) noexcept {
        if (score > lastHighScore) {
            lastHighPlayer = playerNum;
            lastHighScore = score;
        }
    }
    
    /**
     * @brief Reset tracker
     */
    void reset() noexcept {
        lastHighPlayer = -1;
        lastHighScore = -9999;
    }
};

//=============================================================================
// Camera Input State
//=============================================================================

/**
 * @brief Button state for camera controls
 */
struct CameraInputState {
    bool attackDown{false};     ///< Attack button state
    bool jumpDown{false};       ///< Jump button state  
    bool moveDownDown{false};   ///< +movedown state
    
    // Previous states for edge detection
    bool prevAttack{false};
    bool prevJump{false};
    bool prevMoveDown{false};
    
    /**
     * @brief Check if attack was just pressed
     */
    [[nodiscard]] bool attackPressed() const noexcept {
        return attackDown && !prevAttack;
    }
    
    /**
     * @brief Check if attack was just released
     */
    [[nodiscard]] bool attackReleased() const noexcept {
        return !attackDown && prevAttack;
    }
    
    /**
     * @brief Check if jump was just pressed (for pogo-stick prevention)
     */
    [[nodiscard]] bool jumpPressed() const noexcept {
        return jumpDown && !prevJump;
    }
    
    /**
     * @brief Check if movedown was just pressed
     */
    [[nodiscard]] bool moveDownPressed() const noexcept {
        return moveDownDown && !prevMoveDown;
    }
    
    /**
     * @brief Update input state from buttons
     */
    void update(bool attack, bool jump, bool moveDown) noexcept {
        prevAttack = attackDown;
        prevJump = jumpDown;
        prevMoveDown = moveDownDown;
        
        attackDown = attack;
        jumpDown = jump;
        moveDownDown = moveDown;
    }
    
    /**
     * @brief Reset input state
     */
    void reset() noexcept {
        attackDown = jumpDown = moveDownDown = false;
        prevAttack = prevJump = prevMoveDown = false;
    }
};

//=============================================================================
// Complete Camera State
//=============================================================================

/**
 * @brief Complete camera system state
 */
struct CameraState {
    // Spectator tracking
    SpectatorTrackingState spectator;
    
    // Camera modes
    ChaseCameraState chase;
    ExternalCameraState external;
    
    // Multiview
    MultiviewState multiview;
    
    // High fragger tracking
    HighTrackerState highTracker;
    
    // Input
    CameraInputState input;
    
    // Desired position for flyby
    std::array<float, 3> desiredPosition{0.0f, 0.0f, 0.0f};
    
    /**
     * @brief Check if in spectator mode
     */
    [[nodiscard]] bool isSpectator() const noexcept {
        return spectator.autocam != SpectatorCameraMode::None || 
               spectator.locked;
    }
    
    /**
     * @brief Check if should draw viewmodel
     */
    [[nodiscard]] bool shouldDrawViewModel(bool isSpectatorPlayer, bool chasecamEnabled) const noexcept {
        if (!isSpectatorPlayer) return true;
        return spectator.isLocked() && chasecamEnabled;
    }
    
    /**
     * @brief Check if should draw a specific player
     */
    [[nodiscard]] bool shouldDrawPlayer(int playerNum, bool isSpectatorPlayer) const noexcept {
        if (!isSpectatorPlayer) return true;
        if (!spectator.isLocked()) return true;
        if (spectator.trackNum != playerNum) return true;
        return !chase.isFirstPerson();
    }
    
    /**
     * @brief Get the main tracking number
     */
    [[nodiscard]] int mainTrackNum() const noexcept {
        return spectator.idealTrack;
    }
    
    /**
     * @brief Reset all camera state
     */
    void reset() noexcept {
        spectator.reset();
        chase = ChaseCameraState{};
        external.reset();
        multiview.reset();
        highTracker.reset();
        input.reset();
        desiredPosition = {0.0f, 0.0f, 0.0f};
    }
};

//=============================================================================
// Utility Functions
//=============================================================================

/**
 * @brief Convert vector to angles
 * 
 * Original: vectoangles() in cl_cam.c
 */
inline std::array<float, 3> vectorToAngles(float x, float y, float z) noexcept {
    float yaw, pitch;
    
    if (y == 0.0f && x == 0.0f) {
        yaw = 0.0f;
        pitch = (z > 0.0f) ? 90.0f : 270.0f;
    } else {
        yaw = std::atan2(y, x) * 180.0f / static_cast<float>(M_PI);
        if (yaw < 0.0f) yaw += 360.0f;
        
        float forward = std::sqrt(x * x + y * y);
        pitch = std::atan2(z, forward) * 180.0f / static_cast<float>(M_PI);
        if (pitch < 0.0f) pitch += 360.0f;
    }
    
    return {pitch, yaw, 0.0f};
}

} // namespace ezquake
