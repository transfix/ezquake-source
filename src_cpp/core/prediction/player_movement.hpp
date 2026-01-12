/*
 * ezQuake C++ Port - Player Movement Physics
 * 
 * Modern C++20 implementation of the Quake player movement system.
 * Handles physics simulation including friction, acceleration, jumping,
 * water movement, and collision response.
 * 
 * Original: pmove.c
 * 
 * Copyright (C) 1996-1997 Id Software, Inc.
 * Copyright (C) 2024 ezQuake Team
 */

#pragma once

#include "core/prediction/pmove_types.hpp"
#include <cmath>
#include <algorithm>
#include <functional>

namespace ezquake {

//=============================================================================
// TraceProvider - Interface for collision traces
//=============================================================================

/**
 * @brief Interface for providing collision traces.
 * 
 * This allows the player movement to be decoupled from the actual
 * collision detection system, enabling testing with mock traces.
 */
class TraceProvider {
public:
    virtual ~TraceProvider() = default;
    
    /**
     * @brief Trace a player-sized hull from start to end.
     */
    [[nodiscard]] virtual TraceResult playerTrace(const Vec3& start, const Vec3& end) const = 0;
    
    /**
     * @brief Trace a line from start to end (point trace).
     */
    [[nodiscard]] virtual TraceResult lineTrace(const Vec3& start, const Vec3& end) const = 0;
    
    /**
     * @brief Get the contents at a point.
     */
    [[nodiscard]] virtual Contents pointContents(const Vec3& point) const = 0;
    
    /**
     * @brief Test if a player-sized hull can fit at a point.
     */
    [[nodiscard]] virtual bool testPlayerPosition(const Vec3& point) const = 0;
};

//=============================================================================
// SimpleTraceProvider - Basic trace provider for testing
//=============================================================================

/**
 * @brief Simple trace provider that assumes empty world.
 * 
 * Used for testing movement physics in isolation.
 */
class SimpleTraceProvider : public TraceProvider {
public:
    // Set the ground plane for simple floor collision
    void setGroundPlane(float height = 0.0f) noexcept {
        hasGround_ = true;
        groundHeight_ = height;
    }
    
    void clearGroundPlane() noexcept {
        hasGround_ = false;
    }
    
    [[nodiscard]] TraceResult playerTrace(const Vec3& start, const Vec3& end) const override {
        TraceResult result;
        result.endPos = end;
        
        if (hasGround_) {
            // Player hull bottom is at origin - 24
            float startZ = start.z() - 24.0f;
            float endZ = end.z() - 24.0f;
            
            if (endZ < groundHeight_ && startZ >= groundHeight_) {
                // We would pass through the ground
                float fraction = (startZ - groundHeight_) / (startZ - endZ);
                fraction = std::clamp(fraction, 0.0f, 1.0f);
                
                result.fraction = fraction;
                result.endPos = start + (end - start) * fraction;
                result.endPos.z() = groundHeight_ + 24.0f;  // Sit on ground
                result.plane.normal = Vec3(0, 0, 1);
                result.plane.distance = groundHeight_;
                result.entityNum = 0;  // World
            }
        }
        
        return result;
    }
    
    [[nodiscard]] TraceResult lineTrace(const Vec3& start, const Vec3& end) const override {
        return playerTrace(start, end);
    }
    
    [[nodiscard]] Contents pointContents(const Vec3& point) const override {
        if (hasGround_ && point.z() < groundHeight_) {
            return Contents::Solid;
        }
        return Contents::Empty;
    }
    
    [[nodiscard]] bool testPlayerPosition(const Vec3& point) const override {
        if (hasGround_) {
            return (point.z() - 24.0f) >= groundHeight_;
        }
        return true;
    }

private:
    bool hasGround_ = false;
    float groundHeight_ = 0.0f;
};

//=============================================================================
// PlayerMovement - Main movement physics class
//=============================================================================

/**
 * @brief Handles player movement physics simulation.
 * 
 * This class encapsulates all the Quake player movement physics including:
 * - Ground movement with friction
 * - Air movement with air acceleration
 * - Water movement
 * - Spectator/fly modes
 * - Jumping and water jumping
 * - Collision response (slide/step)
 * 
 * Original: pmove.c functions
 */
class PlayerMovement {
public:
    /**
     * @brief Construct with trace provider and movement variables.
     */
    explicit PlayerMovement(const TraceProvider& traceProvider, const MoveVars& moveVars) noexcept
        : traceProvider_(&traceProvider), moveVars_(&moveVars) {}
    
    /**
     * @brief Execute a player move.
     * 
     * @param state The movement state to update
     * @return Blocked flags indicating what was hit
     */
    pmove::Blocked playerMove(PlayerMoveState& state) noexcept {
        pmove::Blocked blocked = pmove::Blocked::None;
        
        // Calculate frame time
        frameTime_ = state.cmd().msec() * 0.001f;
        
        // Clear touch list
        state.clearTouchList();
        
        // Handle special move types
        if (state.moveType() == PMoveType::None || state.moveType() == PMoveType::Lock) {
            categorizePosition(state);
            return blocked;
        }
        
        // Take angles directly from command
        state.setAngles(state.cmd().angles());
        calculateForwardRight(state.angles());
        
        // Spectator movement
        if (state.moveType() == PMoveType::Spectator || 
            state.moveType() == PMoveType::OldSpectator) {
            spectatorMove(state);
            state.setOnGround(false);
            return blocked;
        }
        
        // Nudge position if stuck
        nudgePosition(state);
        
        // Categorize position (ground/water)
        categorizePosition(state);
        
        // Check for water jump
        if (state.waterLevel() == WaterLevel::Waist && state.moveType() != PMoveType::Fly) {
            checkWaterJump(state);
        }
        
        // Handle water jump time
        if (state.velocity().z() < 0.0f || state.moveType() == PMoveType::Dead) {
            state.setWaterJumpTime(0.0f);
        }
        if (state.waterJumpTime() > 0.0f) {
            float newTime = state.waterJumpTime() - frameTime_;
            state.setWaterJumpTime(std::max(0.0f, newTime));
        }
        
        // Handle jump msec timing
        if (state.jumpMsec() > 0) {
            int newJumpMsec = state.jumpMsec() + state.cmd().msec();
            state.setJumpMsec(newJumpMsec > 50 ? 0 : newJumpMsec);
        }
        
        // Check for jump
        checkJump(state);
        
        // Apply friction
        friction(state);
        
        // Movement based on water level
        if (state.waterLevel() >= WaterLevel::Waist) {
            blocked = waterMove(state);
        } else if (state.moveType() == PMoveType::Fly) {
            blocked = flyMove(state);
        } else {
            blocked = airMove(state);
        }
        
        // Final position categorization
        categorizePosition(state);
        
        // Final velocity clip when landing
        if (!moveVars_->pGround) {
            if (state.onGround() && state.velocity().z() < -300.0f) {
                if (state.velocity().dot(groundNormal_) < pmove::MAX_JUMPFIX_DOTPRODUCT) {
                    state.setVelocity(clipVelocity(state.velocity(), groundNormal_, 1.0f));
                }
            }
        }
        
        return blocked;
    }
    
    /**
     * @brief Categorize the player's current position.
     * 
     * Determines if player is on ground, in water, etc.
     */
    void categorizePosition(PlayerMoveState& state) noexcept {
        state.setMaxGroundSpeed(pmove::MAXGROUNDSPEED_DEFAULT);
        
        // Check for ground
        Vec3 point = state.origin();
        point.z() -= 1.0f;
        
        TraceResult trace;
        
        if (moveVars_->rampJump) {
            trace = traceProvider_->playerTrace(state.origin(), point);
            
            // Handle ramp jumping - increase max ground speed for steeper ramps
            if (trace.physicsNormal.isSet() && !isFarFromGround(trace) && 
                trace.entityNum == 0 && trace.plane.normal.z() > pmove::MIN_STEP_NORMAL &&
                trace.plane.normal.z() < 1.0f &&
                state.velocity().dot(trace.plane.normal) < pmove::MAX_JUMPFIX_DOTPRODUCT) {
                
                float range = 1.0f - std::asin(trace.plane.normal.z()) * 2.0f / 3.14159265f;
                range = std::min(range, 0.5f) * 2.0f;
                
                float newMaxSpeed = pmove::MAXGROUNDSPEED_DEFAULT + 
                    (pmove::MAXGROUNDSPEED_MAXIMUM - pmove::MAXGROUNDSPEED_DEFAULT) * range;
                state.setMaxGroundSpeed(newMaxSpeed);
            }
        }
        
        // Check if moving too fast to be on ground
        if (state.velocity().z() > state.maxGroundSpeed()) {
            state.setOnGround(false);
        } else if (!moveVars_->pGround || state.onGround()) {
            if (!moveVars_->rampJump) {
                trace = traceProvider_->playerTrace(state.origin(), point);
            }
            
            if (isFarFromGround(trace)) {
                state.setOnGround(false);
            } else {
                state.setOnGround(true);
                state.setGroundEntity(trace.entityNum);
                state.setWaterJumpTime(0.0f);
                groundNormal_ = trace.plane.normal;
                
                // Touch the ground entity
                if (trace.entityNum > 0) {
                    state.addTouch(trace.entityNum);
                }
            }
        }
        
        // Check water level
        checkWaterLevel(state);
        
        // Snap to ground if appropriate
        if (!moveVars_->pGround) {
            if (state.onGround() && state.moveType() != PMoveType::Fly &&
                state.waterLevel() < WaterLevel::Waist) {
                if (!trace.startSolid && !trace.allSolid) {
                    state.setOrigin(trace.endPos);
                }
            }
        }
    }
    
    /**
     * @brief Get the current frame time.
     */
    [[nodiscard]] float frameTime() const noexcept { return frameTime_; }
    
    /**
     * @brief Get the ground normal from the last categorization.
     */
    [[nodiscard]] const Vec3& groundNormal() const noexcept { return groundNormal_; }

private:
    const TraceProvider* traceProvider_;
    const MoveVars* moveVars_;
    
    float frameTime_ = 0.0f;
    Vec3 forward_{};
    Vec3 right_{};
    Vec3 groundNormal_{0, 0, 1};
    
    //=========================================================================
    // Movement helpers
    //=========================================================================
    
    void calculateForwardRight(const Vec3& angles) noexcept {
        // Convert angles to direction vectors
        float pitch = angles.x() * (3.14159265f / 180.0f);
        float yaw = angles.y() * (3.14159265f / 180.0f);
        
        float cp = std::cos(pitch);
        float sp = std::sin(pitch);
        float cy = std::cos(yaw);
        float sy = std::sin(yaw);
        
        forward_ = Vec3(cp * cy, cp * sy, -sp);
        right_ = Vec3(sy, -cy, 0);
    }
    
    [[nodiscard]] static bool isFarFromGround(const TraceResult& trace) noexcept {
        return trace.fraction == 1.0f || trace.plane.normal.z() < pmove::MIN_STEP_NORMAL;
    }
    
    void checkWaterLevel(PlayerMoveState& state) noexcept {
        state.setWaterLevel(WaterLevel::None);
        state.setWaterType(Contents::Empty);
        
        // Check feet
        Vec3 point = state.origin();
        point.z() += -24.0f + 1.0f;  // Player mins z + 1
        
        Contents cont = traceProvider_->pointContents(point);
        if (cont <= Contents::Water) {
            state.setWaterType(cont);
            state.setWaterLevel(WaterLevel::Feet);
            
            // Check waist
            point.z() = state.origin().z() + (-24.0f + 32.0f) * 0.5f;
            cont = traceProvider_->pointContents(point);
            if (cont <= Contents::Water) {
                state.setWaterLevel(WaterLevel::Waist);
                
                // Check eyes
                point.z() = state.origin().z() + 22.0f;
                cont = traceProvider_->pointContents(point);
                if (cont <= Contents::Water) {
                    state.setWaterLevel(WaterLevel::Eyes);
                }
            }
        }
    }
    
    void nudgePosition(PlayerMoveState& state) noexcept {
        static constexpr int signs[3] = {0, -1, 1};
        
        Vec3 base = state.origin();
        
        // Quantize origin to 1/8 unit
        Vec3 origin = state.origin();
        for (int i = 0; i < 3; ++i) {
            origin[i] = std::floor(origin[i] * 8.0f) * 0.125f;
        }
        state.setOrigin(origin);
        
        // Try nudging in all directions
        for (int z = 0; z <= 2; ++z) {
            for (int y = 0; y <= 2; ++y) {
                for (int x = 0; x <= 2; ++x) {
                    Vec3 test = base + Vec3(
                        signs[x] * 0.125f,
                        signs[y] * 0.125f,
                        signs[z] * 0.125f
                    );
                    state.setOrigin(test);
                    if (traceProvider_->testPlayerPosition(test)) {
                        return;
                    }
                }
            }
        }
        
        // Try moving up (maps may spawn player in ground)
        for (int z = 1; z <= 18; ++z) {
            Vec3 test = base;
            test.z() += static_cast<float>(z);
            state.setOrigin(test);
            if (traceProvider_->testPlayerPosition(test)) {
                return;
            }
        }
        
        // Give up, use original
        state.setOrigin(base);
    }
    
    void checkJump(PlayerMoveState& state) noexcept {
        if (state.moveType() == PMoveType::Fly) {
            return;
        }
        
        if (state.moveType() == PMoveType::Dead) {
            state.setJumpHeld(true);
            return;
        }
        
        if (!state.cmd().isJumping()) {
            state.setJumpHeld(false);
            return;
        }
        
        if (state.waterJumpTime() > 0.0f) {
            return;
        }
        
        // Swimming jump
        if (state.waterLevel() >= WaterLevel::Waist) {
            state.setOnGround(false);
            
            float jumpVel = 100.0f;
            if (state.waterType() == Contents::Slime) {
                jumpVel = 80.0f;
            } else if (state.waterType() != Contents::Water) {
                jumpVel = 50.0f;
            }
            
            Vec3 vel = state.velocity();
            vel.z() = jumpVel;
            state.setVelocity(vel);
            return;
        }
        
        if (!state.onGround()) {
            return;
        }
        
        if (state.jumpHeld() && state.jumpMsec() == 0) {
            return;
        }
        
        // Check for jump bug fix
        if (!moveVars_->pGround) {
            Vec3 vel = state.velocity();
            if ((moveVars_->rampJump || vel.z() < 0.0f) &&
                vel.dot(groundNormal_) < pmove::MAX_JUMPFIX_DOTPRODUCT) {
                vel = clipVelocity(vel, groundNormal_, 1.0f);
                state.setVelocity(vel);
            }
        }
        
        state.setOnGround(false);
        
        // Apply jump velocity
        Vec3 vel = state.velocity();
        
        // Adjust for ramp jumping max speed
        if (state.maxGroundSpeed() > pmove::MAXGROUNDSPEED_DEFAULT && 
            vel.z() > pmove::MAXGROUNDSPEED_DEFAULT) {
            vel.z() = pmove::MAXGROUNDSPEED_DEFAULT;
        }
        
        vel.z() += 270.0f;
        
        // ktjump modifier
        if (moveVars_->ktJump > 0.0f) {
            float kt = std::min(moveVars_->ktJump, 1.0f);
            if (vel.z() < 270.0f) {
                vel.z() = vel.z() * (1.0f - kt) + 270.0f * kt;
            }
        }
        
        state.setVelocity(vel);
        state.setJumpHeld(true);
        state.setJumpMsec(state.cmd().msec());
    }
    
    void checkWaterJump(PlayerMoveState& state) noexcept {
        if (state.waterJumpTime() > 0.0f) {
            return;
        }
        
        // Don't hop out if we just jumped in
        if (state.velocity().z() < -180.0f) {
            return;
        }
        
        // See if near an edge
        Vec3 flatForward = Vec3(forward_.x(), forward_.y(), 0);
        flatForward.normalize();
        
        Vec3 spot = state.origin() + flatForward * 24.0f;
        spot.z() += 8.0f;
        
        Contents cont = traceProvider_->pointContents(spot);
        if (cont != Contents::Solid) {
            return;
        }
        
        spot.z() += 24.0f;
        cont = traceProvider_->pointContents(spot);
        if (cont != Contents::Empty) {
            return;
        }
        
        // Jump out of water
        Vec3 vel = flatForward * 50.0f;
        vel.z() = 310.0f;
        state.setVelocity(vel);
        state.setWaterJumpTime(2.0f);
        state.setJumpHeld(true);
    }
    
    void friction(PlayerMoveState& state) noexcept {
        if (state.waterJumpTime() > 0.0f) {
            return;
        }
        
        Vec3 vel = state.velocity();
        float speed = vel.length();
        
        if (speed < 1.0f) {
            vel.x() = 0.0f;
            vel.y() = 0.0f;
            if (state.moveType() == PMoveType::Fly) {
                vel.z() = 0.0f;
            }
            state.setVelocity(vel);
            return;
        }
        
        float drop = 0.0f;
        
        if (state.waterLevel() >= WaterLevel::Waist) {
            // Water friction
            drop = speed * moveVars_->waterFriction * 
                   static_cast<float>(static_cast<int>(state.waterLevel())) * frameTime_;
        } else if (state.moveType() == PMoveType::Fly) {
            // Fly mode friction
            drop = speed * pmove::FLY_FRICTION * frameTime_;
        } else if (state.onGround()) {
            // Ground friction
            float friction = moveVars_->friction;
            
            // Extra friction at dropoffs
            Vec3 start = state.origin();
            start.x() += vel.x() / speed * 16.0f;
            start.y() += vel.y() / speed * 16.0f;
            start.z() += -24.0f;  // Player mins z
            
            Vec3 stop = start;
            stop.z() -= 34.0f;
            
            TraceResult trace = traceProvider_->playerTrace(start, stop);
            if (trace.fraction == 1.0f) {
                friction *= 2.0f;
            }
            
            float control = std::max(speed, moveVars_->stopSpeed);
            drop = control * friction * frameTime_;
        } else {
            // In air, no friction
            return;
        }
        
        // Scale velocity
        float newSpeed = std::max(0.0f, speed - drop) / speed;
        state.setVelocity(vel * newSpeed);
    }
    
    void accelerate(PlayerMoveState& state, const Vec3& wishDir, float wishSpeed, float accel) noexcept {
        if (state.moveType() == PMoveType::Dead || state.waterJumpTime() > 0.0f) {
            return;
        }
        
        float currentSpeed = state.velocity().dot(wishDir);
        float addSpeed = wishSpeed - currentSpeed;
        
        if (addSpeed <= 0.0f) {
            return;
        }
        
        float accelSpeed = std::min(accel * frameTime_ * wishSpeed, addSpeed);
        state.setVelocity(state.velocity() + wishDir * accelSpeed);
    }
    
    void airAccelerate(PlayerMoveState& state, const Vec3& wishDir, float wishSpeed, float accel) noexcept {
        if (state.moveType() == PMoveType::Dead || state.waterJumpTime() > 0.0f) {
            return;
        }
        
        float originalSpeed = 0.0f;
        if (moveVars_->bunnySpeedCap > 0.0f) {
            originalSpeed = std::sqrt(state.velocity().x() * state.velocity().x() + 
                                      state.velocity().y() * state.velocity().y());
        }
        
        float wishSpd = std::min(wishSpeed, 30.0f);
        float currentSpeed = state.velocity().dot(wishDir);
        float addSpeed = wishSpd - currentSpeed;
        
        if (addSpeed <= 0.0f) {
            return;
        }
        
        float accelSpeed = std::min(accel * wishSpeed * frameTime_, addSpeed);
        Vec3 newVel = state.velocity() + wishDir * accelSpeed;
        
        // Apply bunny speed cap
        if (moveVars_->bunnySpeedCap > 0.0f) {
            float newSpeed = std::sqrt(newVel.x() * newVel.x() + newVel.y() * newVel.y());
            if (newSpeed > originalSpeed) {
                float speedCap = moveVars_->maxSpeed * moveVars_->bunnySpeedCap;
                if (newSpeed > speedCap) {
                    float useSpeed = std::max(originalSpeed, speedCap);
                    newVel.x() *= useSpeed / newSpeed;
                    newVel.y() *= useSpeed / newSpeed;
                }
            }
        }
        
        state.setVelocity(newVel);
    }
    
    //=========================================================================
    // Movement modes
    //=========================================================================
    
    pmove::Blocked slideMove(PlayerMoveState& state) noexcept {
        pmove::Blocked blocked = pmove::Blocked::None;
        
        std::array<Vec3, pmove::MAX_CLIP_PLANES> planes;
        int numPlanes = 0;
        
        Vec3 primalVel = state.velocity();
        Vec3 originalVel = state.velocity();
        
        float timeLeft = frameTime_;
        constexpr int numBumps = 4;
        
        for (int bump = 0; bump < numBumps; ++bump) {
            Vec3 end = state.origin() + state.velocity() * timeLeft;
            TraceResult trace = traceProvider_->playerTrace(state.origin(), end);
            
            if (trace.startSolid || trace.allSolid) {
                // Trapped in solid
                state.setVelocity(Vec3{});
                return pmove::Blocked::Floor | pmove::Blocked::Step;
            }
            
            if (trace.fraction > 0.0f) {
                // Actually covered some distance
                state.setOrigin(trace.endPos);
                numPlanes = 0;
            }
            
            if (trace.fraction == 1.0f) {
                break;  // Moved the entire distance
            }
            
            // Save entity for contact
            state.addTouch(trace.entityNum);
            
            // Categorize what we hit
            if (trace.plane.normal.z() >= pmove::MIN_STEP_NORMAL) {
                blocked |= pmove::Blocked::Floor;
            } else if (trace.plane.normal.z() == 0.0f) {
                blocked |= pmove::Blocked::Step;
            } else {
                blocked |= pmove::Blocked::Other;
            }
            
            timeLeft -= timeLeft * trace.fraction;
            
            if (numPlanes >= pmove::MAX_CLIP_PLANES) {
                state.setVelocity(Vec3{});
                break;
            }
            
            planes[numPlanes++] = trace.plane.normal;
            
            // Clip velocity to all planes
            int i;
            for (i = 0; i < numPlanes; ++i) {
                Vec3 clipped = clipVelocity(originalVel, planes[i], 1.0f);
                
                int j;
                for (j = 0; j < numPlanes; ++j) {
                    if (j != i && clipped.dot(planes[j]) < 0.0f) {
                        break;
                    }
                }
                
                if (j == numPlanes) {
                    state.setVelocity(clipped);
                    break;
                }
            }
            
            if (i != numPlanes) {
                // Successfully clipped
            } else {
                // Go along the crease
                if (numPlanes != 2) {
                    state.setVelocity(Vec3{});
                    break;
                }
                
                Vec3 dir = planes[0].cross(planes[1]);
                float d = dir.dot(state.velocity());
                state.setVelocity(dir * d);
            }
            
            // Stop if velocity is against original
            if (state.velocity().dot(primalVel) <= 0.0f) {
                state.setVelocity(Vec3{});
                break;
            }
        }
        
        // Preserve velocity during water jump
        if (state.waterJumpTime() > 0.0f) {
            state.setVelocity(primalVel);
        }
        
        return blocked;
    }
    
    pmove::Blocked stepSlideMove(PlayerMoveState& state, bool inAir) noexcept {
        Vec3 original = state.origin();
        Vec3 originalVel = state.velocity();
        
        pmove::Blocked blocked = slideMove(state);
        
        if (blocked == pmove::Blocked::None) {
            return blocked;
        }
        
        // Check if we can step
        if (inAir) {
            if (!pmove::hasBlocked(blocked, pmove::Blocked::Step)) {
                return blocked;
            }
            
            // Check for solid ground below
            Vec3 org = (originalVel.z() < 0.0f) ? state.origin() : original;
            Vec3 dest = org;
            dest.z() -= pmove::STEP_SIZE;
            
            TraceResult trace = traceProvider_->playerTrace(org, dest);
            if (trace.fraction == 1.0f || trace.plane.normal.z() < pmove::MIN_STEP_NORMAL) {
                return blocked;
            }
        }
        
        float stepSize = inAir ? pmove::STEP_SIZE - (original.z() - state.origin().z()) : pmove::STEP_SIZE;
        
        Vec3 down = state.origin();
        Vec3 downVel = state.velocity();
        
        // Reset and try stepping up
        state.setOrigin(original);
        state.setVelocity(originalVel);
        
        // Move up
        Vec3 dest = state.origin();
        dest.z() += stepSize;
        TraceResult trace = traceProvider_->playerTrace(state.origin(), dest);
        if (!trace.startSolid && !trace.allSolid) {
            state.setOrigin(trace.endPos);
        }
        
        if (inAir && originalVel.z() < 0.0f) {
            Vec3 vel = state.velocity();
            vel.z() = 0.0f;
            state.setVelocity(vel);
        }
        
        // Slide move from stepped position
        slideMove(state);
        
        // Step back down
        dest = state.origin();
        dest.z() -= stepSize;
        trace = traceProvider_->playerTrace(state.origin(), dest);
        if (trace.fraction != 1.0f && trace.plane.normal.z() < pmove::MIN_STEP_NORMAL) {
            // Use original down position
            state.setOrigin(down);
            state.setVelocity(downVel);
            return blocked;
        }
        
        if (!trace.startSolid && !trace.allSolid) {
            state.setOrigin(trace.endPos);
        }
        
        if (state.origin().z() < original.z()) {
            // Use original down position
            state.setOrigin(down);
            state.setVelocity(downVel);
            return blocked;
        }
        
        Vec3 up = state.origin();
        
        // Compare distances
        float downDist = (down.x() - original.x()) * (down.x() - original.x()) +
                         (down.y() - original.y()) * (down.y() - original.y());
        float upDist = (up.x() - original.x()) * (up.x() - original.x()) +
                       (up.y() - original.y()) * (up.y() - original.y());
        
        if (downDist >= upDist) {
            state.setOrigin(down);
            state.setVelocity(downVel);
            return blocked;
        }
        
        // Use stepped move, preserve z velocity from slide
        Vec3 vel = state.velocity();
        vel.z() = downVel.z();
        state.setVelocity(vel);
        
        // Air step velocity reduction
        if (!state.onGround() && state.waterLevel() < WaterLevel::Waist &&
            pmove::hasBlocked(blocked, pmove::Blocked::Step)) {
            float scale = 1.0f - 0.01f * (state.origin().z() - original.z());
            vel = state.velocity();
            vel.x() *= scale;
            vel.y() *= scale;
            state.setVelocity(vel);
        }
        
        return blocked;
    }
    
    pmove::Blocked waterMove(PlayerMoveState& state) noexcept {
        Vec3 wishVel;
        for (int i = 0; i < 3; ++i) {
            wishVel[i] = forward_[i] * state.cmd().forwardMove() +
                        right_[i] * state.cmd().sideMove();
        }
        
        if (state.moveType() != PMoveType::Fly && 
            state.cmd().forwardMove() == 0 && 
            state.cmd().sideMove() == 0 &&
            state.cmd().upMove() == 0) {
            wishVel.z() -= 60.0f;  // Drift down
        } else {
            wishVel.z() += state.cmd().upMove();
        }
        
        Vec3 wishDir = wishVel;
        float wishSpeed = wishDir.normalize();
        
        if (wishSpeed > moveVars_->maxSpeed) {
            wishVel = wishVel * (moveVars_->maxSpeed / wishSpeed);
            wishSpeed = moveVars_->maxSpeed;
        }
        wishSpeed *= 0.7f;
        
        accelerate(state, wishDir, wishSpeed, moveVars_->waterAccelerate);
        
        return stepSlideMove(state, false);
    }
    
    pmove::Blocked flyMove(PlayerMoveState& state) noexcept {
        Vec3 wishVel;
        for (int i = 0; i < 3; ++i) {
            wishVel[i] = forward_[i] * state.cmd().forwardMove() +
                        right_[i] * state.cmd().sideMove();
        }
        wishVel.z() += state.cmd().upMove();
        
        Vec3 wishDir = wishVel;
        float wishSpeed = wishDir.normalize();
        
        if (wishSpeed > moveVars_->maxSpeed) {
            wishVel = wishVel * (moveVars_->maxSpeed / wishSpeed);
            wishSpeed = moveVars_->maxSpeed;
        }
        
        accelerate(state, wishDir, wishSpeed, moveVars_->accelerate);
        
        return stepSlideMove(state, false);
    }
    
    pmove::Blocked airMove(PlayerMoveState& state) noexcept {
        float fmove = static_cast<float>(state.cmd().forwardMove());
        float smove = static_cast<float>(state.cmd().sideMove());
        
        Vec3 fwd = Vec3(forward_.x(), forward_.y(), 0);
        Vec3 rt = Vec3(right_.x(), right_.y(), 0);
        fwd.normalize();
        rt.normalize();
        
        Vec3 wishVel;
        for (int i = 0; i < 2; ++i) {
            wishVel[i] = fwd[i] * fmove + rt[i] * smove;
        }
        wishVel.z() = 0.0f;
        
        Vec3 wishDir = wishVel;
        float wishSpeed = wishDir.normalize();
        
        // Clamp to max speed
        if (wishSpeed > moveVars_->maxSpeed) {
            wishVel = wishVel * (moveVars_->maxSpeed / wishSpeed);
            wishSpeed = moveVars_->maxSpeed;
        }
        
        if (state.onGround()) {
            if (moveVars_->slideFix) {
                Vec3 vel = state.velocity();
                vel.z() = std::min(vel.z(), 0.0f);
                state.setVelocity(vel);
                accelerate(state, wishDir, wishSpeed, moveVars_->accelerate);
                vel = state.velocity();
                vel.z() -= moveVars_->effectiveGravity() * frameTime_;
                state.setVelocity(vel);
            } else {
                Vec3 vel = state.velocity();
                vel.z() = 0.0f;
                state.setVelocity(vel);
                accelerate(state, wishDir, wishSpeed, moveVars_->accelerate);
            }
            
            if (state.velocity().x() == 0.0f && state.velocity().y() == 0.0f) {
                state.setVelocity(Vec3{});
                return pmove::Blocked::None;
            }
            
            return stepSlideMove(state, false);
        }
        
        // In air
        airAccelerate(state, wishDir, wishSpeed, moveVars_->accelerate);
        
        // Add gravity
        Vec3 vel = state.velocity();
        vel.z() -= moveVars_->effectiveGravity() * frameTime_;
        state.setVelocity(vel);
        
        pmove::Blocked blocked;
        if (moveVars_->airStep) {
            blocked = stepSlideMove(state, true);
        } else {
            blocked = slideMove(state);
        }
        
        if (moveVars_->pGround) {
            if (pmove::hasBlocked(blocked, pmove::Blocked::Floor)) {
                state.setOnGround(true);
            }
        }
        
        return blocked;
    }
    
    void spectatorMove(PlayerMoveState& state) noexcept {
        Vec3 vel = state.velocity();
        float speed = vel.length();
        
        // Friction
        if (speed < 1.0f) {
            state.setVelocity(Vec3{});
        } else {
            float friction = moveVars_->friction * 1.5f;  // Extra friction
            float control = std::max(speed, moveVars_->stopSpeed);
            float drop = control * friction * frameTime_;
            float newSpeed = std::max(0.0f, speed - drop) / speed;
            state.setVelocity(vel * newSpeed);
        }
        
        // Accelerate
        float fmove = static_cast<float>(state.cmd().forwardMove());
        float smove = static_cast<float>(state.cmd().sideMove());
        
        Vec3 fwd = forward_;
        Vec3 rt = right_;
        fwd.normalize();
        rt.normalize();
        
        Vec3 wishVel;
        for (int i = 0; i < 3; ++i) {
            wishVel[i] = fwd[i] * fmove + rt[i] * smove;
        }
        wishVel.z() += state.cmd().upMove();
        
        Vec3 wishDir = wishVel;
        float wishSpeed = wishDir.normalize();
        
        // Clamp to spectator max speed
        if (wishSpeed > moveVars_->spectatorMaxSpeed) {
            wishVel = wishVel * (moveVars_->spectatorMaxSpeed / wishSpeed);
            wishSpeed = moveVars_->spectatorMaxSpeed;
        }
        
        float currentSpeed = state.velocity().dot(wishDir);
        float addSpeed = wishSpeed - currentSpeed;
        
        // Buggy QW spectator mode, kept for compatibility
        if (state.moveType() == PMoveType::OldSpectator) {
            if (addSpeed <= 0.0f) {
                return;
            }
        }
        
        if (addSpeed > 0.0f) {
            float accelSpeed = std::min(moveVars_->accelerate * frameTime_ * wishSpeed, addSpeed);
            state.setVelocity(state.velocity() + wishDir * accelSpeed);
        }
        
        // Move
        state.setOrigin(state.origin() + state.velocity() * frameTime_);
    }
};

// ClientPredictor requires the full PlayerState definition from client_types.hpp
// This class is not compiled unless EZQUAKE_HAS_PLAYER_STATE is defined and
// client_types.hpp is fully included, not just forward-declared.
#ifdef EZQUAKE_HAS_PLAYER_STATE

//=============================================================================
// ClientPredictor - Client-side movement prediction
//=============================================================================

/**
 * @brief Predicts player movement from past command.
 * 
 * Used for client-side prediction to reduce perceived latency.
 * 
 * Original: CL_PredictUsercmd() in cl_pred.c
 */
class ClientPredictor {
public:
    /**
     * @brief Construct with trace provider and movement variables.
     */
    explicit ClientPredictor(const TraceProvider& traceProvider, const MoveVars& moveVars) noexcept
        : movement_(traceProvider, moveVars), moveVars_(&moveVars) {}
    
    /**
     * @brief Predict a single user command.
     * 
     * Copies state from 'from' to 'to', applying the command.
     * Splits long moves for accuracy.
     * 
     * @param from Source player state
     * @param to Destination player state
     * @param cmd User command to apply
     */
    void predictUserCmd(const PlayerState& from, PlayerState& to, const UserCommand& cmd) noexcept {
        // Split up very long moves
        if (cmd.msec() > 50) {
            PlayerState temp;
            UserCommand split = cmd;
            split.setMsec(cmd.msec() / 2);
            
            predictUserCmd(from, temp, split);
            predictUserCmd(temp, to, split);
            return;
        }
        
        // Setup movement state from player state
        PlayerMoveState pmove;
        pmove.setOrigin(from.origin());
        pmove.setAngles(cmd.angles());
        pmove.setVelocity(from.velocity());
        pmove.setJumpMsec(from.jumpMsec());
        pmove.setJumpHeld(from.jumpHeld());
        pmove.setWaterJumpTime(from.waterJumpTime());
        pmove.setMoveType(static_cast<PMoveType>(from.pmType()));
        pmove.setOnGround(from.onGround());
        pmove.setCmd(cmd);
        
        // Execute the move
        movement_.playerMove(pmove);
        
        // Copy results to output state
        to.setOrigin(pmove.origin());
        to.setViewAngles(pmove.angles());
        to.setVelocity(pmove.velocity());
        to.setWaterJumpTime(pmove.waterJumpTime());
        to.setPmType(static_cast<int>(pmove.moveType()));
        to.setJumpHeld(pmove.jumpHeld());
        to.setJumpMsec(pmove.jumpMsec());
        to.setOnGround(pmove.onGround());
        
        // Preserve weapon frame from source
        to.setWeaponFrame(from.weaponFrame());
    }
    
    /**
     * @brief Categorize player position for ground detection.
     * 
     * Used when cl_nopred is 1 to determine ground state.
     */
    void categorizePosition(PlayerMoveState& state) noexcept {
        movement_.categorizePosition(state);
    }

private:
    PlayerMovement movement_;
    const MoveVars* moveVars_;
};

#endif // EZQUAKE_HAS_PLAYER_STATE

} // namespace ezquake
