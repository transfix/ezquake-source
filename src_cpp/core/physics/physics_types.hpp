/**
 * @file physics_types.hpp
 * @brief Physics and collision type definitions for ezQuake C++ core
 * 
 * This file contains type-safe equivalents for physics structures
 * from sv_phys.c, sv_move.c, sv_world.c, sv_world.h.
 */

#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>

#include "core/math/vec3.hpp"

namespace ezquake::physics {

// Use Vec3 from ezquake::math namespace
using Vec3 = ezquake::math::Vec3;

// =============================================================================
// Trace Types (from sv_world.h)
// =============================================================================

/**
 * @brief Trace move type flags
 * 
 * Matches MOVE_* defines from sv_world.h.
 * Controls how traces interact with entities.
 */
enum class TraceType : uint32_t {
    Normal = 0,       // MOVE_NORMAL - standard collision
    NoMonsters = 1,   // MOVE_NOMONSTERS - ignore monsters (line of sight)
    Missile = 2,      // MOVE_MISSILE - missiles get extra size vs monsters
    Lagged = 64       // MOVE_LAGGED - use antilag positions
};

[[nodiscard]] constexpr std::string_view traceTypeName(TraceType type) noexcept {
    switch (type) {
        case TraceType::Normal: return "normal";
        case TraceType::NoMonsters: return "nomonsters";
        case TraceType::Missile: return "missile";
        case TraceType::Lagged: return "lagged";
        default: return "unknown";
    }
}

/**
 * @brief Combine trace type flags
 */
[[nodiscard]] constexpr uint32_t operator|(TraceType a, TraceType b) noexcept {
    return static_cast<uint32_t>(a) | static_cast<uint32_t>(b);
}

[[nodiscard]] constexpr uint32_t operator|(uint32_t a, TraceType b) noexcept {
    return a | static_cast<uint32_t>(b);
}

[[nodiscard]] constexpr bool hasFlag(uint32_t flags, TraceType type) noexcept {
    return (flags & static_cast<uint32_t>(type)) != 0;
}

// =============================================================================
// Area Types (from sv_world.h)
// =============================================================================

/**
 * @brief Area query type
 * 
 * Matches AREA_* defines from sv_world.h.
 */
enum class AreaType : uint8_t {
    Solid = 0,     // AREA_SOLID - solid entities
    Triggers = 1   // AREA_TRIGGERS - trigger entities
};

[[nodiscard]] constexpr std::string_view areaTypeName(AreaType type) noexcept {
    switch (type) {
        case AreaType::Solid: return "solid";
        case AreaType::Triggers: return "triggers";
        default: return "unknown";
    }
}

// =============================================================================
// Physics Constants
// =============================================================================

namespace physics_constants {
    // From sv_phys.c
    constexpr float STOP_EPSILON = 0.1f;
    constexpr int MAX_CLIP_PLANES = 5;
    
    // From sv_move.c
    constexpr float STEPSIZE = 18.0f;
    
    // From sv_world.h
    constexpr int AREA_DEPTH = 4;
    constexpr int AREA_NODES = 32;
    constexpr int MAX_ENT_LEAFS = 32;
    
    // Fly move return flags
    constexpr int BLOCKED_FLOOR = 1;
    constexpr int BLOCKED_STEP = 2;
    constexpr int BLOCKED_DEAD_STOP = 4;
    
    // Default physics values
    constexpr float DEFAULT_GRAVITY = 800.0f;
    constexpr float DEFAULT_MAXVELOCITY = 2000.0f;
    constexpr float DEFAULT_MAXSPEED = 320.0f;
    constexpr float DEFAULT_STOPSPEED = 100.0f;
    constexpr float DEFAULT_ACCELERATE = 10.0f;
    constexpr float DEFAULT_AIRACCELERATE = 10.0f;
    constexpr float DEFAULT_FRICTION = 4.0f;
    constexpr float DEFAULT_WATERFRICTION = 4.0f;
    constexpr float DEFAULT_WATERACCELERATE = 10.0f;
    constexpr float SPECTATOR_MAXSPEED = 500.0f;
}

// =============================================================================
// Blocked Flags
// =============================================================================

/**
 * @brief Movement blocked flags
 * 
 * Returned by SV_FlyMove to indicate what blocked movement.
 */
namespace BlockedFlags {
    constexpr int None = 0;
    constexpr int Floor = 1;      // Hit a floor (normal[2] > 0.7)
    constexpr int Step = 2;       // Hit a wall/step (normal[2] == 0)
    constexpr int DeadStop = 4;   // Completely stuck
    
    [[nodiscard]] constexpr bool hitFloor(int flags) noexcept {
        return (flags & Floor) != 0;
    }
    
    [[nodiscard]] constexpr bool hitStep(int flags) noexcept {
        return (flags & Step) != 0;
    }
    
    [[nodiscard]] constexpr bool isStuck(int flags) noexcept {
        return flags == 3 || flags == 7;  // trapped or dead stop
    }
}

// =============================================================================
// Trace Plane
// =============================================================================

/**
 * @brief Collision plane information
 */
struct TracePlane {
    Vec3 normal;
    float dist = 0.0f;
    
    [[nodiscard]] constexpr bool isFloor() const noexcept {
        return normal[2] > 0.7f;
    }
    
    [[nodiscard]] constexpr bool isWall() const noexcept {
        return normal[2] == 0.0f;
    }
    
    [[nodiscard]] constexpr bool isCeiling() const noexcept {
        return normal[2] < -0.7f;
    }
    
    [[nodiscard]] constexpr bool isSlope() const noexcept {
        float nz = normal[2];
        return nz > 0.0f && nz <= 0.7f;
    }
};

// =============================================================================
// Trace Result
// =============================================================================

/**
 * @brief Result of a trace/collision test
 * 
 * Simplified version of trace_t from world.h.
 */
struct TraceResult {
    bool allSolid = false;      // if true, plane is not valid
    bool startSolid = false;    // if true, started in solid
    bool inOpen = false;        // if true, ended in open space
    bool inWater = false;       // if true, ended in water
    float fraction = 1.0f;      // fraction of move completed (0.0 to 1.0)
    Vec3 endPos;                // final position
    TracePlane plane;           // surface normal at impact
    int entityNum = -1;         // entity hit (-1 = nothing/world)
    
    [[nodiscard]] constexpr bool didHit() const noexcept {
        return fraction < 1.0f || allSolid;
    }
    
    [[nodiscard]] constexpr bool startedInSolid() const noexcept {
        return startSolid;
    }
    
    [[nodiscard]] constexpr bool isTrapped() const noexcept {
        return allSolid;
    }
    
    [[nodiscard]] constexpr bool hitWorld() const noexcept {
        return didHit() && entityNum <= 0;
    }
    
    [[nodiscard]] constexpr bool hitEntity() const noexcept {
        return didHit() && entityNum > 0;
    }
};

// =============================================================================
// Bounding Box
// =============================================================================

/**
 * @brief Axis-aligned bounding box
 */
struct BoundingBox {
    Vec3 mins;
    Vec3 maxs;
    
    [[nodiscard]] Vec3 center() const noexcept {
        return (mins + maxs) * 0.5f;
    }
    
    [[nodiscard]] Vec3 size() const noexcept {
        return maxs - mins;
    }
    
    [[nodiscard]] Vec3 extents() const noexcept {
        return size() * 0.5f;
    }
    
    [[nodiscard]] bool contains(const Vec3& point) const noexcept {
        return point[0] >= mins[0] && point[0] <= maxs[0] &&
               point[1] >= mins[1] && point[1] <= maxs[1] &&
               point[2] >= mins[2] && point[2] <= maxs[2];
    }
    
    [[nodiscard]] bool intersects(const BoundingBox& other) const noexcept {
        return !(mins[0] > other.maxs[0] || maxs[0] < other.mins[0] ||
                 mins[1] > other.maxs[1] || maxs[1] < other.mins[1] ||
                 mins[2] > other.maxs[2] || maxs[2] < other.mins[2]);
    }
    
    [[nodiscard]] bool isEmpty() const noexcept {
        return mins[0] >= maxs[0] || mins[1] >= maxs[1] || mins[2] >= maxs[2];
    }
    
    void expand(float amount) noexcept {
        mins[0] -= amount;
        mins[1] -= amount;
        mins[2] -= amount;
        maxs[0] += amount;
        maxs[1] += amount;
        maxs[2] += amount;
    }
    
    void expandForItem() noexcept {
        // Items are expanded by 15 units horizontally for easier pickup
        mins[0] -= 15.0f;
        mins[1] -= 15.0f;
        maxs[0] += 15.0f;
        maxs[1] += 15.0f;
    }
    
    [[nodiscard]] static BoundingBox fromOriginAndSize(const Vec3& origin, const Vec3& mins_, const Vec3& maxs_) noexcept {
        return BoundingBox{origin + mins_, origin + maxs_};
    }
    
    [[nodiscard]] static BoundingBox point(const Vec3& p) noexcept {
        return BoundingBox{p, p};
    }
};

// =============================================================================
// Move Clip (from sv_world.c)
// =============================================================================

/**
 * @brief Movement clipping context
 * 
 * Matches moveclip_t from sv_world.c.
 * Used to track collision tests during movement.
 */
struct MoveClip {
    BoundingBox boxBounds;      // enclose the test object along entire move
    Vec3 mins;                  // size of the moving object
    Vec3 maxs;
    Vec3 mins2;                 // size when clipping against monsters
    Vec3 maxs2;
    Vec3 start;
    Vec3 end;
    TraceResult trace;
    uint32_t type = 0;
    int passEntityNum = -1;     // entity to skip
    
    void clear() noexcept {
        boxBounds = BoundingBox{};
        mins = Vec3{};
        maxs = Vec3{};
        mins2 = Vec3{};
        maxs2 = Vec3{};
        start = Vec3{};
        end = Vec3{};
        trace = TraceResult{};
        type = 0;
        passEntityNum = -1;
    }
};

// =============================================================================
// Area Node (from sv_world.h)
// =============================================================================

/**
 * @brief Spatial partition node
 * 
 * Matches areanode_t from sv_world.h.
 * Used for BSP-based spatial partitioning.
 */
struct AreaNode {
    int8_t axis = -1;           // -1 = leaf node, 0 = X, 1 = Y
    float dist = 0.0f;          // split plane distance
    int childIndices[2] = {-1, -1};  // indices into area node array
    // Note: actual entity links managed separately
    
    [[nodiscard]] constexpr bool isLeaf() const noexcept {
        return axis == -1;
    }
    
    [[nodiscard]] constexpr int getAxis() const noexcept {
        return axis;
    }
    
    /**
     * @brief Determine which child contains a point
     * @return 0 for negative side, 1 for positive side
     */
    [[nodiscard]] int classifyPoint(const Vec3& point) const noexcept {
        if (axis < 0) return -1;  // leaf
        return point[static_cast<size_t>(axis)] >= dist ? 0 : 1;
    }
};

// =============================================================================
// Velocity Clipping
// =============================================================================

/**
 * @brief Clip velocity against a plane
 * 
 * Implements ClipVelocity from sv_phys.c.
 * Slides the velocity along the impact plane.
 * 
 * @param in Input velocity
 * @param normal Surface normal
 * @param overbounce Bounce factor (1.0 = no bounce, >1 = some bounce)
 * @return Clipped velocity
 */
[[nodiscard]] inline Vec3 clipVelocity(const Vec3& in, const Vec3& normal, float overbounce = 1.0f) noexcept {
    float backoff = in.dot(normal) * overbounce;
    
    Vec3 out;
    for (size_t i = 0; i < 3; ++i) {
        float change = normal[i] * backoff;
        out[i] = in[i] - change;
        // Clamp to avoid tiny oscillations
        if (out[i] > -physics_constants::STOP_EPSILON && out[i] < physics_constants::STOP_EPSILON) {
            out[i] = 0.0f;
        }
    }
    return out;
}

// =============================================================================
// Gravity Helper
// =============================================================================

/**
 * @brief Apply gravity to velocity
 * 
 * @param velocity Current velocity (modified in place)
 * @param scale Gravity scale factor (typically entity gravity)
 * @param gravity World gravity value
 * @param frametime Time step
 */
inline void applyGravity(Vec3& velocity, float scale, float gravity, float frametime) noexcept {
    velocity[2] -= scale * gravity * frametime;
}

// =============================================================================
// Floor Check
// =============================================================================

/**
 * @brief Check if a normal represents a floor
 * 
 * In Quake, a floor has a Z normal > 0.7 (about 45 degrees).
 */
[[nodiscard]] constexpr bool isFloorNormal(float normalZ) noexcept {
    return normalZ > 0.7f;
}

/**
 * @brief Check if a normal represents a wall
 */
[[nodiscard]] constexpr bool isWallNormal(float normalZ) noexcept {
    return normalZ == 0.0f;
}

// =============================================================================
// Movement Parameters
// =============================================================================

/**
 * @brief Movement physics parameters
 * 
 * Configurable physics values from cvars.
 */
struct MoveVars {
    float gravity = physics_constants::DEFAULT_GRAVITY;
    float stopSpeed = physics_constants::DEFAULT_STOPSPEED;
    float maxSpeed = physics_constants::DEFAULT_MAXSPEED;
    float spectatorMaxSpeed = physics_constants::SPECTATOR_MAXSPEED;
    float accelerate = physics_constants::DEFAULT_ACCELERATE;
    float airAccelerate = physics_constants::DEFAULT_AIRACCELERATE;
    float waterAccelerate = physics_constants::DEFAULT_WATERACCELERATE;
    float friction = physics_constants::DEFAULT_FRICTION;
    float waterFriction = physics_constants::DEFAULT_WATERFRICTION;
    float maxVelocity = physics_constants::DEFAULT_MAXVELOCITY;
    
    // KTJump/bunny-hopping settings
    float ktJump = 1.0f;
    float bunnySpeedCap = 0.0f;
    bool slidefix = false;
    bool airstep = false;
    bool pground = false;
    bool rampjump = false;
    
    void setDefaults() noexcept {
        gravity = physics_constants::DEFAULT_GRAVITY;
        stopSpeed = physics_constants::DEFAULT_STOPSPEED;
        maxSpeed = physics_constants::DEFAULT_MAXSPEED;
        spectatorMaxSpeed = physics_constants::SPECTATOR_MAXSPEED;
        accelerate = physics_constants::DEFAULT_ACCELERATE;
        airAccelerate = physics_constants::DEFAULT_AIRACCELERATE;
        waterAccelerate = physics_constants::DEFAULT_WATERACCELERATE;
        friction = physics_constants::DEFAULT_FRICTION;
        waterFriction = physics_constants::DEFAULT_WATERFRICTION;
        maxVelocity = physics_constants::DEFAULT_MAXVELOCITY;
        ktJump = 1.0f;
        bunnySpeedCap = 0.0f;
        slidefix = false;
        airstep = false;
        pground = false;
        rampjump = false;
    }
};

// =============================================================================
// Velocity Clamping
// =============================================================================

/**
 * @brief Clamp velocity to maximum speed
 * 
 * Implements SV_CheckVelocity clamping from sv_phys.c.
 * Uses vector length clamping (Maddes fix) rather than per-component.
 * 
 * @param velocity Velocity to clamp (modified in place)
 * @param maxVelocity Maximum allowed speed
 * @return true if velocity was clamped
 */
inline bool clampVelocity(Vec3& velocity, float maxVelocity) noexcept {
    float speed = velocity.length();
    if (speed > maxVelocity) {
        velocity = velocity * (maxVelocity / speed);
        return true;
    }
    return false;
}

/**
 * @brief Check and fix NaN values in a vector
 * 
 * @param v Vector to check (modified in place)
 * @return true if any NaN was fixed
 */
inline bool fixNaN(Vec3& v) noexcept {
    bool hadNaN = false;
    for (size_t i = 0; i < 3; ++i) {
        if (std::isnan(v[i])) {
            v[i] = 0.0f;
            hadNaN = true;
        }
    }
    return hadNaN;}

// =============================================================================
// Content Types (from bspfile.h, used in physics)
// =============================================================================

/**
 * @brief BSP content types
 * 
 * Used for point contents checks during physics.
 */
enum class ContentType : int {
    Empty = -1,
    Solid = -2,
    Water = -3,
    Slime = -4,
    Lava = -5,
    Sky = -6,
    Origin = -7,       // removed at csg time
    Clip = -8,         // changed to contents_solid
    Current0 = -9,
    Current90 = -10,
    Current180 = -11,
    Current270 = -12,
    CurrentUp = -13,
    CurrentDown = -14
};

[[nodiscard]] constexpr std::string_view contentTypeName(ContentType type) noexcept {
    switch (type) {
        case ContentType::Empty: return "empty";
        case ContentType::Solid: return "solid";
        case ContentType::Water: return "water";
        case ContentType::Slime: return "slime";
        case ContentType::Lava: return "lava";
        case ContentType::Sky: return "sky";
        default: return "unknown";
    }
}

[[nodiscard]] constexpr bool isLiquid(ContentType type) noexcept {
    return type == ContentType::Water ||
           type == ContentType::Slime ||
           type == ContentType::Lava;
}

[[nodiscard]] constexpr bool isSolid(ContentType type) noexcept {
    return type == ContentType::Solid;
}

[[nodiscard]] constexpr bool isDamaging(ContentType type) noexcept {
    return type == ContentType::Slime || type == ContentType::Lava;
}

[[nodiscard]] constexpr bool isCurrent(ContentType type) noexcept {
    int val = static_cast<int>(type);
    return val <= -9 && val >= -14;
}

// =============================================================================
// Push Result
// =============================================================================

/**
 * @brief Result of pushing an entity
 */
struct PushResult {
    bool success = false;
    TraceResult trace;
    int blocked = 0;
    
    [[nodiscard]] constexpr bool wasBlocked() const noexcept {
        return blocked != 0;
    }
};

// =============================================================================
// Step Move Result  
// =============================================================================

/**
 * @brief Result of a stepped movement (for monsters)
 */
struct StepMoveResult {
    bool success = false;
    bool partialGround = false;
    Vec3 finalOrigin;
    
    [[nodiscard]] constexpr bool walkedOffEdge() const noexcept {
        return !success && !partialGround;
    }
};

// =============================================================================
// Fly Move Result
// =============================================================================

/**
 * @brief Result of SV_FlyMove
 */
struct FlyMoveResult {
    int blocked = 0;
    TraceResult stepTrace;  // trace of vertical wall hit (for player extrafriction)
    Vec3 finalVelocity;
    Vec3 finalOrigin;
    
    [[nodiscard]] bool hitFloor() const noexcept {
        return BlockedFlags::hitFloor(blocked);
    }
    
    [[nodiscard]] bool hitStep() const noexcept {
        return BlockedFlags::hitStep(blocked);
    }
    
    [[nodiscard]] bool wasTrapped() const noexcept {
        return BlockedFlags::isStuck(blocked);
    }
};

} // namespace ezquake::physics
