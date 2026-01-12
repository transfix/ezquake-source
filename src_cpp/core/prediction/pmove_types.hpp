/*
 * ezQuake C++ Port - Player Movement Types
 * 
 * Modern C++20 replacements for pmove.h structures.
 * Core types for player movement physics and collision.
 * 
 * Copyright (C) 1996-1997 Id Software, Inc.
 * Copyright (C) 2024 ezQuake Team
 */

#pragma once

#include "core/types.hpp"
#include "core/math/vec3.hpp"
#include "core/client/client_types.hpp"
#include <array>
#include <cstdint>
#include <optional>
#include <span>

namespace ezquake {

//=============================================================================
// Constants
//=============================================================================

namespace pmove {

inline constexpr int MAX_PHYSENTS = 64;
inline constexpr int MAX_CLIP_PLANES = 5;
inline constexpr int MAX_TOUCH = 64;
inline constexpr float STEP_SIZE = 18.0f;
inline constexpr float MIN_STEP_NORMAL = 0.7f;  // ~45 degrees
inline constexpr float STOP_EPSILON = 0.1f;
inline constexpr float FLY_FRICTION = 4.0f;
inline constexpr float MAX_JUMPFIX_DOTPRODUCT = -0.1f;
inline constexpr float MAXGROUNDSPEED_DEFAULT = 180.0f;
inline constexpr float MAXGROUNDSPEED_MAXIMUM = 240.0f;

// Blocked flags for slide move
enum class Blocked : std::uint8_t {
    None  = 0,
    Floor = 1,
    Step  = 2,
    Other = 4,
    Any   = 7
};

[[nodiscard]] constexpr Blocked operator|(Blocked lhs, Blocked rhs) noexcept {
    return static_cast<Blocked>(static_cast<std::uint8_t>(lhs) | static_cast<std::uint8_t>(rhs));
}
[[nodiscard]] constexpr Blocked operator&(Blocked lhs, Blocked rhs) noexcept {
    return static_cast<Blocked>(static_cast<std::uint8_t>(lhs) & static_cast<std::uint8_t>(rhs));
}
[[nodiscard]] constexpr Blocked& operator|=(Blocked& lhs, Blocked rhs) noexcept {
    lhs = lhs | rhs;
    return lhs;
}
[[nodiscard]] constexpr bool hasBlocked(Blocked flags, Blocked check) noexcept {
    return (static_cast<std::uint8_t>(flags) & static_cast<std::uint8_t>(check)) != 0;
}

} // namespace pmove

//=============================================================================
// PMoveType - Player movement modes
//=============================================================================

/**
 * @brief Player movement types for physics simulation.
 * 
 * Original: pmtype_t
 */
enum class PMoveType : std::uint8_t {
    Normal = 0,         // Normal ground movement
    OldSpectator = 1,   // Fly, no clip to world (QW bug)
    Spectator = 2,      // Fly, no clip to world
    Dead = 3,           // No acceleration
    Fly = 4,            // Fly, bump into walls
    None = 5,           // Can't move
    Lock = 6            // Server controls origin and view angles
};

/**
 * @brief Check if a movement type allows input.
 */
[[nodiscard]] constexpr bool canMove(PMoveType type) noexcept {
    return type != PMoveType::None && type != PMoveType::Lock;
}

/**
 * @brief Check if a movement type ignores world collision.
 */
[[nodiscard]] constexpr bool isNoClip(PMoveType type) noexcept {
    return type == PMoveType::OldSpectator || type == PMoveType::Spectator;
}

//=============================================================================
// WaterLevel - How deep the player is in water
//=============================================================================

enum class WaterLevel : std::uint8_t {
    None = 0,       // Not in water
    Feet = 1,       // Feet in water
    Waist = 2,      // Waist deep
    Eyes = 3        // Fully submerged
};

//=============================================================================
// Contents - BSP leaf contents
//=============================================================================

enum class Contents : std::int32_t {
    Empty = -1,
    Solid = -2,
    Water = -3,
    Slime = -4,
    Lava = -5,
    Sky = -6
};

//=============================================================================
// PhysicsNormalFlags - Flags for physics normal adjustments
//=============================================================================

namespace physics {

enum class NormalFlags : std::uint8_t {
    None = 0,
    Set = 1,
    FlipX = 2,
    FlipY = 4,
    FlipZ = 8
};

[[nodiscard]] constexpr NormalFlags operator|(NormalFlags lhs, NormalFlags rhs) noexcept {
    return static_cast<NormalFlags>(static_cast<std::uint8_t>(lhs) | static_cast<std::uint8_t>(rhs));
}
[[nodiscard]] constexpr NormalFlags operator&(NormalFlags lhs, NormalFlags rhs) noexcept {
    return static_cast<NormalFlags>(static_cast<std::uint8_t>(lhs) & static_cast<std::uint8_t>(rhs));
}
[[nodiscard]] constexpr bool hasFlag(NormalFlags flags, NormalFlags check) noexcept {
    return (static_cast<std::uint8_t>(flags) & static_cast<std::uint8_t>(check)) != 0;
}

/**
 * @brief Physics normal for ramp jumping.
 */
struct PhysicsNormal {
    Vec3 normal{};
    NormalFlags flags = NormalFlags::None;
    
    [[nodiscard]] constexpr bool isSet() const noexcept {
        return hasFlag(flags, NormalFlags::Set);
    }
};

} // namespace physics

//=============================================================================
// Plane - Clipping plane
//=============================================================================

/**
 * @brief A clipping plane for collision detection.
 * 
 * Original: mplane_t (simplified)
 */
struct Plane {
    Vec3 normal{};
    float distance = 0.0f;
    
    constexpr Plane() noexcept = default;
    constexpr Plane(const Vec3& n, float d) noexcept : normal(n), distance(d) {}
    
    /**
     * @brief Calculate signed distance from point to plane.
     */
    [[nodiscard]] constexpr float distanceTo(const Vec3& point) const noexcept {
        return normal.dot(point) - distance;
    }
    
    /**
     * @brief Check if point is in front of plane.
     */
    [[nodiscard]] constexpr bool isInFront(const Vec3& point) const noexcept {
        return distanceTo(point) > 0.0f;
    }
};

//=============================================================================
// PhysEntity - Physical entity for collision
//=============================================================================

/**
 * @brief A physical entity that can be collided with.
 * 
 * Original: physent_t
 */
struct PhysEntity {
    Vec3 origin{};
    Vec3 mins{};        // Only for non-BSP models
    Vec3 maxs{};        // Only for non-BSP models
    int info = 0;       // Client/server identification
    bool isTransparent = false;
    
    // Note: model pointer would be added when integrating with collision system
    // void* model = nullptr;
    
    constexpr PhysEntity() noexcept = default;
    
    /**
     * @brief Check if this is a BSP model (has mins/maxs bounds).
     */
    [[nodiscard]] constexpr bool hasBounds() const noexcept {
        return mins != maxs;
    }
};

//=============================================================================
// TraceResult - Result of a collision trace
//=============================================================================

/**
 * @brief Result of a collision trace.
 * 
 * Original: trace_t
 */
struct TraceResult {
    bool allSolid = false;      // If true, plane is not valid
    bool startSolid = false;    // If true, initial point was in solid
    float fraction = 1.0f;      // Time completed, 1.0 = didn't hit anything
    Vec3 endPos{};              // Final position
    Plane plane{};              // Surface normal at impact
    int entityNum = -1;         // Entity hit
    physics::PhysicsNormal physicsNormal{};  // For ramp jumping
    
    constexpr TraceResult() noexcept = default;
    
    /**
     * @brief Check if we hit something.
     */
    [[nodiscard]] constexpr bool didHit() const noexcept {
        return fraction < 1.0f;
    }
    
    /**
     * @brief Check if we're stuck in solid.
     */
    [[nodiscard]] constexpr bool isStuck() const noexcept {
        return allSolid || startSolid;
    }
    
    /**
     * @brief Check if we hit a floor surface (walkable).
     */
    [[nodiscard]] constexpr bool hitFloor() const noexcept {
        return didHit() && plane.normal.z() >= pmove::MIN_STEP_NORMAL;
    }
    
    /**
     * @brief Check if we hit a wall (vertical surface).
     */
    [[nodiscard]] constexpr bool hitWall() const noexcept {
        return didHit() && plane.normal.z() == 0.0f;
    }
};

//=============================================================================
// MoveVars - Movement physics parameters
//=============================================================================

/**
 * @brief Movement physics parameters from server.
 * 
 * Original: movevars_t
 */
struct MoveVars {
    float gravity = 800.0f;
    float stopSpeed = 100.0f;
    float maxSpeed = 320.0f;
    float spectatorMaxSpeed = 500.0f;
    float accelerate = 10.0f;
    float airAccelerate = 0.7f;
    float waterAccelerate = 10.0f;
    float friction = 4.0f;
    float waterFriction = 1.0f;
    float entGravity = 1.0f;
    float bunnySpeedCap = 0.0f;
    float ktJump = 0.0f;
    
    // Movement behavior flags
    bool slideFix = false;      // NQ-style movement down ramps
    bool airStep = false;       // Allow stepping up in air
    bool pGround = false;       // NQ-style "onground" flag handling
    int rampJump = 0;           // Clip velocity during jump frame
    int safeStrafe = 0;         // sv_safestrafe value from server
    
    constexpr MoveVars() noexcept = default;
    
    /**
     * @brief Reset to default Quake values.
     */
    constexpr void reset() noexcept {
        *this = MoveVars{};
    }
    
    /**
     * @brief Get effective gravity for an entity.
     */
    [[nodiscard]] constexpr float effectiveGravity() const noexcept {
        return gravity * entGravity;
    }
};

//=============================================================================
// PlayerMoveState - Full player movement state
//=============================================================================

/**
 * @brief Complete player movement state for physics simulation.
 * 
 * This contains all state needed for a single player move,
 * including input, output, and world collision information.
 * 
 * Original: playermove_t
 */
class PlayerMoveState {
public:
    constexpr PlayerMoveState() noexcept = default;
    
    //=========================================================================
    // Player state
    //=========================================================================
    
    // Position
    [[nodiscard]] constexpr const Vec3& origin() const noexcept { return origin_; }
    constexpr void setOrigin(const Vec3& origin) noexcept { origin_ = origin; }
    
    // Angles
    [[nodiscard]] constexpr const Vec3& angles() const noexcept { return angles_; }
    constexpr void setAngles(const Vec3& angles) noexcept { angles_ = angles; }
    
    // Velocity
    [[nodiscard]] constexpr const Vec3& velocity() const noexcept { return velocity_; }
    constexpr void setVelocity(const Vec3& vel) noexcept { velocity_ = vel; }
    
    // Jump state
    [[nodiscard]] constexpr bool jumpHeld() const noexcept { return jumpHeld_; }
    [[nodiscard]] constexpr int jumpMsec() const noexcept { return jumpMsec_; }
    [[nodiscard]] constexpr float waterJumpTime() const noexcept { return waterJumpTime_; }
    
    constexpr void setJumpHeld(bool held) noexcept { jumpHeld_ = held; }
    constexpr void setJumpMsec(int msec) noexcept { jumpMsec_ = msec; }
    constexpr void setWaterJumpTime(float time) noexcept { waterJumpTime_ = time; }
    
    // Movement type
    [[nodiscard]] constexpr PMoveType moveType() const noexcept { return moveType_; }
    constexpr void setMoveType(PMoveType type) noexcept { moveType_ = type; }
    
    //=========================================================================
    // Results
    //=========================================================================
    
    // Ground state
    [[nodiscard]] constexpr bool onGround() const noexcept { return onGround_; }
    [[nodiscard]] constexpr int groundEntity() const noexcept { return groundEntity_; }
    
    constexpr void setOnGround(bool onGround) noexcept { onGround_ = onGround; }
    constexpr void setGroundEntity(int entity) noexcept { groundEntity_ = entity; }
    
    // Water state
    [[nodiscard]] constexpr WaterLevel waterLevel() const noexcept { return waterLevel_; }
    [[nodiscard]] constexpr Contents waterType() const noexcept { return waterType_; }
    
    constexpr void setWaterLevel(WaterLevel level) noexcept { waterLevel_ = level; }
    constexpr void setWaterType(Contents type) noexcept { waterType_ = type; }
    
    // Movement bounds
    [[nodiscard]] constexpr float maxGroundSpeed() const noexcept { return maxGroundSpeed_; }
    constexpr void setMaxGroundSpeed(float speed) noexcept { maxGroundSpeed_ = speed; }
    
    //=========================================================================
    // Input command
    //=========================================================================
    
    [[nodiscard]] constexpr const UserCommand& cmd() const noexcept { return cmd_; }
    constexpr void setCmd(const UserCommand& cmd) noexcept { cmd_ = cmd; }
    
    //=========================================================================
    // Touch list
    //=========================================================================
    
    [[nodiscard]] constexpr int numTouch() const noexcept { return numTouch_; }
    [[nodiscard]] std::span<const int> touchList() const noexcept {
        return {touchIndex_.data(), static_cast<size_t>(numTouch_)};
    }
    
    /**
     * @brief Add an entity to the touch list.
     * @return true if added, false if list is full or already present
     */
    bool addTouch(int entityNum) noexcept {
        if (numTouch_ >= pmove::MAX_TOUCH) {
            return false;
        }
        // Check for duplicates
        for (int i = 0; i < numTouch_; ++i) {
            if (touchIndex_[i] == entityNum) {
                return false;
            }
        }
        touchIndex_[numTouch_++] = entityNum;
        return true;
    }
    
    constexpr void clearTouchList() noexcept { numTouch_ = 0; }
    
    //=========================================================================
    // Physical entities
    //=========================================================================
    
    [[nodiscard]] constexpr int numPhysEnts() const noexcept { return numPhysEnts_; }
    
    [[nodiscard]] std::span<const PhysEntity> physEnts() const noexcept {
        return {physEnts_.data(), static_cast<size_t>(numPhysEnts_)};
    }
    
    [[nodiscard]] std::span<PhysEntity> physEnts() noexcept {
        return {physEnts_.data(), static_cast<size_t>(numPhysEnts_)};
    }
    
    bool addPhysEnt(const PhysEntity& ent) noexcept {
        if (numPhysEnts_ >= pmove::MAX_PHYSENTS) {
            return false;
        }
        physEnts_[numPhysEnts_++] = ent;
        return true;
    }
    
    constexpr void clearPhysEnts() noexcept { numPhysEnts_ = 0; }
    
    //=========================================================================
    // Helpers
    //=========================================================================
    
    /**
     * @brief Clear all result state for a new move.
     */
    constexpr void clearResults() noexcept {
        onGround_ = false;
        groundEntity_ = -1;
        waterLevel_ = WaterLevel::None;
        waterType_ = Contents::Empty;
        maxGroundSpeed_ = pmove::MAXGROUNDSPEED_DEFAULT;
        numTouch_ = 0;
    }
    
    /**
     * @brief Check if player is swimming.
     */
    [[nodiscard]] constexpr bool isSwimming() const noexcept {
        return waterLevel_ >= WaterLevel::Waist;
    }
    
    /**
     * @brief Check if player can jump.
     */
    [[nodiscard]] constexpr bool canJump() const noexcept {
        return moveType_ != PMoveType::Dead && 
               moveType_ != PMoveType::Fly &&
               (onGround_ || isSwimming());
    }

private:
    // Player state
    Vec3 origin_{};
    Vec3 angles_{};
    Vec3 velocity_{};
    bool jumpHeld_ = false;
    int jumpMsec_ = 0;
    float waterJumpTime_ = 0.0f;
    PMoveType moveType_ = PMoveType::Normal;
    
    // Results
    bool onGround_ = false;
    int groundEntity_ = -1;
    WaterLevel waterLevel_ = WaterLevel::None;
    Contents waterType_ = Contents::Empty;
    float maxGroundSpeed_ = pmove::MAXGROUNDSPEED_DEFAULT;
    
    // Input
    UserCommand cmd_{};
    
    // Touch list
    int numTouch_ = 0;
    std::array<int, pmove::MAX_TOUCH> touchIndex_{};
    
    // Physical entities
    int numPhysEnts_ = 0;
    std::array<PhysEntity, pmove::MAX_PHYSENTS> physEnts_{};
};

//=============================================================================
// Velocity clipping utility
//=============================================================================

/**
 * @brief Clip velocity by a plane normal.
 * 
 * Slides off of the impacting surface, reducing velocity component
 * perpendicular to the plane.
 * 
 * @param in Input velocity
 * @param normal Plane normal
 * @param overbounce Bounce factor (1.0 = no bounce, >1.0 = bounce back)
 * @return Clipped velocity
 */
[[nodiscard]] inline Vec3 clipVelocity(const Vec3& in, const Vec3& normal, float overbounce = 1.0f) noexcept {
    float backoff = in.dot(normal) * overbounce;
    
    Vec3 out = in - normal * backoff;
    
    // Snap tiny values to zero to prevent oscillation
    if (out.x() > -pmove::STOP_EPSILON && out.x() < pmove::STOP_EPSILON) out.x() = 0.0f;
    if (out.y() > -pmove::STOP_EPSILON && out.y() < pmove::STOP_EPSILON) out.y() = 0.0f;
    if (out.z() > -pmove::STOP_EPSILON && out.z() < pmove::STOP_EPSILON) out.z() = 0.0f;
    
    return out;
}

} // namespace ezquake
