/*
 * ezQuake C++ Port - Crouch and Lerp Movement
 * 
 * Handles stair step-up smoothing and position interpolation.
 * 
 * Original: CL_CalcCrouch() and CL_LerpMove() in cl_pred.c
 * 
 * Copyright (C) 1996-1997 Id Software, Inc.
 * Copyright (C) 2024 ezQuake Team
 */

#pragma once

#include "core/math/vec3.hpp"
#include <algorithm>
#include <array>
#include <cmath>

namespace ezquake {

//=============================================================================
// CrouchCalculator - Smooth stair step-up
//=============================================================================

/**
 * @brief Calculates crouch offset for smooth stair climbing.
 * 
 * When the player walks up stairs, this smoothly interpolates the
 * view height to prevent jarring visual steps.
 * 
 * Original: CL_CalcCrouch() in cl_pred.c
 */
class CrouchCalculator {
public:
    constexpr CrouchCalculator() noexcept = default;
    
    /**
     * @brief Calculate crouch offset for current frame.
     * 
     * @param simOrg Current simulated origin
     * @param frameTime Frame time in seconds
     * @param onGround Is player on ground
     * @param noLerp True if we should reset (teleport, respawn)
     * @return Crouch offset (typically negative)
     */
    float calculate(const Vec3& simOrg, float frameTime, bool onGround, bool noLerp = false) noexcept {
        bool teleported = noLerp || (simOrg - oldOrigin_).lengthSquared() > 48.0f * 48.0f;
        oldOrigin_ = simOrg;
        
        if (teleported) {
            // Possibly teleported or respawned
            oldZ_ = simOrg.z();
            extraCrouch_ = 0.0f;
            crouchSpeed_ = 100.0f;
            crouch_ = 0.0f;
            return crouch_;
        }
        
        if (onGround && simOrg.z() - oldZ_ > 0.0f) {
            // Going up stairs
            if (simOrg.z() - oldZ_ > 20.0f) {
                // Steep stairs - increase speed
                if (crouchSpeed_ < 160.0f) {
                    extraCrouch_ = simOrg.z() - oldZ_ - frameTime * 200.0f - 15.0f;
                    extraCrouch_ = std::min(extraCrouch_, 5.0f);
                }
                crouchSpeed_ = 160.0f;
            }
            
            oldZ_ += frameTime * crouchSpeed_;
            if (oldZ_ > simOrg.z()) {
                oldZ_ = simOrg.z();
            }
            
            if (simOrg.z() - oldZ_ > 15.0f + extraCrouch_) {
                oldZ_ = simOrg.z() - 15.0f - extraCrouch_;
            }
            
            extraCrouch_ -= frameTime * 200.0f;
            extraCrouch_ = std::max(extraCrouch_, 0.0f);
            
            crouch_ = oldZ_ - simOrg.z();
        } else {
            // In air or moving down
            oldZ_ = simOrg.z();
            crouch_ += frameTime * 150.0f;
            if (crouch_ > 0.0f) {
                crouch_ = 0.0f;
            }
            crouchSpeed_ = 100.0f;
            extraCrouch_ = 0.0f;
        }
        
        return crouch_;
    }
    
    /**
     * @brief Get the current crouch offset.
     */
    [[nodiscard]] constexpr float crouch() const noexcept { return crouch_; }
    
    /**
     * @brief Reset all state.
     */
    constexpr void reset() noexcept {
        oldOrigin_ = Vec3{};
        oldZ_ = 0.0f;
        extraCrouch_ = 0.0f;
        crouchSpeed_ = 100.0f;
        crouch_ = 0.0f;
    }

private:
    Vec3 oldOrigin_{};
    float oldZ_ = 0.0f;
    float extraCrouch_ = 0.0f;
    float crouchSpeed_ = 100.0f;
    float crouch_ = 0.0f;
};

//=============================================================================
// LerpMoveCalculator - Interpolate between positions
//=============================================================================

/**
 * @brief Interpolates player position for smooth movement.
 * 
 * Stores multiple frames of position data and interpolates between
 * them for smooth rendering, even when network updates are choppy.
 * 
 * Original: CL_LerpMove() in cl_pred.c
 */
class LerpMoveCalculator {
public:
    static constexpr int NUM_FRAMES = 3;
    
    constexpr LerpMoveCalculator() noexcept {
        for (int i = 0; i < NUM_FRAMES; ++i) {
            lerpTimes_[i] = -1.0;
        }
    }
    
    /**
     * @brief Record a new position sample.
     * 
     * @param origin Current origin
     * @param angles Current angles
     * @param lerpTime Time for this sample (cmdtime)
     * @param sequence Network sequence number
     */
    void recordSample(const Vec3& origin, const Vec3& angles, double lerpTime, int sequence) noexcept {
        // Check for sequence reset
        if (sequence < lastSequence_) {
            // Reset
            lastSequence_ = -1;
            for (int i = 0; i < NUM_FRAMES; ++i) {
                lerpTimes_[i] = -1.0;
            }
            demoLatency_ = 0.01;
            noLerp_[0] = noLerp_[1] = false;
        }
        
        // Only update on new frames
        if (sequence == lastSequence_) {
            return;
        }
        lastSequence_ = sequence;
        
        // Shift data
        lerpTimes_[2] = lerpTimes_[1];
        lerpTimes_[1] = lerpTimes_[0];
        lerpTimes_[0] = lerpTime;
        
        lerpOrigin_[2] = lerpOrigin_[1];
        lerpOrigin_[1] = lerpOrigin_[0];
        lerpOrigin_[0] = origin;
        
        lerpAngles_[2] = lerpAngles_[1];
        lerpAngles_[1] = lerpAngles_[0];
        lerpAngles_[0] = angles;
        
        noLerp_[1] = noLerp_[0];
        noLerp_[0] = noLerpNext_;
        noLerpNext_ = false;
        
        // Check for teleport
        for (int i = 0; i < 3; ++i) {
            if (std::fabs(lerpOrigin_[0][i] - lerpOrigin_[1][i]) > 100.0f) {
                noLerp_[0] = true;
                break;
            }
        }
    }
    
    /**
     * @brief Calculate interpolated position.
     * 
     * @param currentTime Current render time
     * @param physFrame Is this a physics frame
     * @param physFrameTime Physics frame time
     * @param lerpAngles Also lerp angles (for spectating)
     * @return Interpolation result
     */
    struct LerpResult {
        Vec3 origin{};
        Vec3 angles{};
        bool valid = false;
    };
    
    LerpResult calculate(double currentTime, bool physFrame, double physFrameTime, bool lerpAngles = false) noexcept {
        LerpResult result;
        
        if (lerpTimes_[0] < 0.0) {
            return result;  // No data yet
        }
        
        double simTime = currentTime - demoLatency_;
        
        // Adjust latency
        if (simTime > lerpTimes_[0]) {
            // High clamp
            demoLatency_ = currentTime - lerpTimes_[0];
        } else if (lerpTimes_[2] >= 0.0 && simTime < lerpTimes_[2]) {
            // Low clamp
            demoLatency_ = currentTime - lerpTimes_[2];
        } else if (physFrame) {
            // Slowly drift down
            demoLatency_ -= std::min(physFrameTime * 0.005, 0.001);
        }
        
        simTime = currentTime - demoLatency_;
        
        // Decide where to lerp from
        int from, to;
        if (simTime > lerpTimes_[1]) {
            from = 1;
            to = 0;
        } else {
            from = 2;
            to = 1;
        }
        
        if (lerpTimes_[from] < 0.0 || noLerp_[to]) {
            return result;  // Can't lerp
        }
        
        double dt = lerpTimes_[to] - lerpTimes_[from];
        if (dt <= 0.0) {
            return result;
        }
        
        float frac = static_cast<float>((simTime - lerpTimes_[from]) / dt);
        frac = std::clamp(frac, 0.0f, 1.0f);
        
        // Interpolate origin
        result.origin = lerpOrigin_[from] + (lerpOrigin_[to] - lerpOrigin_[from]) * frac;
        
        // Interpolate angles if requested
        if (lerpAngles) {
            result.angles = angleInterpolate(lerpAngles_[from], frac, lerpAngles_[to]);
        } else {
            result.angles = lerpAngles_[0];
        }
        
        result.valid = true;
        return result;
    }
    
    /**
     * @brief Disable lerping for the next position update.
     */
    void disableLerpMove() noexcept {
        noLerp_[0] = noLerp_[1] = noLerpNext_ = true;
    }
    
    /**
     * @brief Reset all state.
     */
    void reset() noexcept {
        *this = LerpMoveCalculator{};
    }

private:
    int lastSequence_ = -1;
    double demoLatency_ = 0.01;
    
    std::array<double, NUM_FRAMES> lerpTimes_{-1.0, -1.0, -1.0};
    std::array<Vec3, NUM_FRAMES> lerpOrigin_{};
    std::array<Vec3, NUM_FRAMES> lerpAngles_{};
    std::array<bool, 2> noLerp_{false, false};
    bool noLerpNext_ = false;
    
    /**
     * @brief Interpolate between two angle vectors.
     */
    static Vec3 angleInterpolate(const Vec3& from, float frac, const Vec3& to) noexcept {
        Vec3 result;
        for (int i = 0; i < 3; ++i) {
            float delta = to[i] - from[i];
            // Handle wraparound
            while (delta > 180.0f) delta -= 360.0f;
            while (delta < -180.0f) delta += 360.0f;
            result[i] = from[i] + delta * frac;
        }
        return result;
    }
};

} // namespace ezquake
