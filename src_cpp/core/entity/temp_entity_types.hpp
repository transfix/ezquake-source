/**
 * @file temp_entity_types.hpp
 * @brief Temporary entity type definitions for effects
 * 
 * Converted from cl_tent.c and protocol.h
 * Core types for explosions, beams, particles, and other temporary effects.
 */

#pragma once

#include <cstdint>
#include <array>
#include <string_view>
#include <algorithm>
#include <cmath>
#include <optional>
#include "core/math/vec3.hpp"

namespace ezquake {

//=============================================================================
// Temp Entity Type Enum (from protocol.h)
//=============================================================================

/**
 * @brief Temporary entity types from network protocol
 * 
 * Original: TE_* defines in protocol.h
 */
enum class TempEntityType : uint8_t {
    Spike = 0,           ///< TE_SPIKE - Nail hitting wall
    SuperSpike = 1,      ///< TE_SUPERSPIKE - Super nail hitting wall
    Gunshot = 2,         ///< TE_GUNSHOT - Bullet hitting wall
    Explosion = 3,       ///< TE_EXPLOSION - Rocket explosion
    TarExplosion = 4,    ///< TE_TAREXPLOSION - Tarbaby explosion
    Lightning1 = 5,      ///< TE_LIGHTNING1 - Shambler lightning
    Lightning2 = 6,      ///< TE_LIGHTNING2 - Wizard lightning
    WizSpike = 7,        ///< TE_WIZSPIKE - Wizard spike
    KnightSpike = 8,     ///< TE_KNIGHTSPIKE - Hell knight spike
    Lightning3 = 9,      ///< TE_LIGHTNING3 - Generic lightning
    LavaSplash = 10,     ///< TE_LAVASPLASH - Lava splash
    Teleport = 11,       ///< TE_TELEPORT - Teleport effect
    Blood = 12,          ///< TE_BLOOD - Blood splatter
    LightningBlood = 13  ///< TE_LIGHTNINGBLOOD - Lightning hitting body
};

/**
 * @brief NetQuake-specific temp entity types
 * 
 * Original: NQ_TE_* defines in protocol.h
 */
enum class NqTempEntityType : uint8_t {
    Explosion2 = 12,  ///< NQ_TE_EXPLOSION2 - Colored explosion
    Beam = 13         ///< NQ_TE_BEAM - Grappling hook beam
};

/**
 * @brief Get name of temp entity type for debugging
 */
constexpr std::string_view tempEntityTypeName(TempEntityType type) noexcept {
    switch (type) {
        case TempEntityType::Spike:          return "spike";
        case TempEntityType::SuperSpike:     return "super_spike";
        case TempEntityType::Gunshot:        return "gunshot";
        case TempEntityType::Explosion:      return "explosion";
        case TempEntityType::TarExplosion:   return "tar_explosion";
        case TempEntityType::Lightning1:     return "lightning1";
        case TempEntityType::Lightning2:     return "lightning2";
        case TempEntityType::WizSpike:       return "wiz_spike";
        case TempEntityType::KnightSpike:    return "knight_spike";
        case TempEntityType::Lightning3:     return "lightning3";
        case TempEntityType::LavaSplash:     return "lava_splash";
        case TempEntityType::Teleport:       return "teleport";
        case TempEntityType::Blood:          return "blood";
        case TempEntityType::LightningBlood: return "lightning_blood";
        default:                             return "unknown";
    }
}

/**
 * @brief Check if temp entity type is a lightning bolt
 */
constexpr bool isLightningType(TempEntityType type) noexcept {
    return type == TempEntityType::Lightning1 ||
           type == TempEntityType::Lightning2 ||
           type == TempEntityType::Lightning3;
}

/**
 * @brief Check if temp entity type needs position parsing only
 */
constexpr bool isPositionOnlyType(TempEntityType type) noexcept {
    switch (type) {
        case TempEntityType::WizSpike:
        case TempEntityType::KnightSpike:
        case TempEntityType::Spike:
        case TempEntityType::SuperSpike:
        case TempEntityType::Explosion:
        case TempEntityType::TarExplosion:
        case TempEntityType::LavaSplash:
        case TempEntityType::Teleport:
            return true;
        default:
            return false;
    }
}

//=============================================================================
// Trail Types (from quakedef.h)
//=============================================================================

/**
 * @brief Particle trail types
 * 
 * Original: trail_type_t enum in quakedef.h
 */
enum class TrailType : uint8_t {
    Rocket = 0,       ///< ROCKET_TRAIL
    Grenade = 1,      ///< GRENADE_TRAIL
    AltRocket = 2,    ///< ALT_ROCKET_TRAIL
    Blood = 3,        ///< BLOOD_TRAIL
    BigBlood = 4,     ///< BIG_BLOOD_TRAIL
    Tracer1 = 5,      ///< TRACER1_TRAIL
    Tracer2 = 6,      ///< TRACER2_TRAIL
    Voor = 7,         ///< VOOR_TRAIL (vore)
    Rail = 8,         ///< RAIL_TRAIL
    Rail2 = 9,        ///< RAIL_TRAIL2
    Lava = 10,        ///< LAVA_TRAIL
    AmfRocket = 11,   ///< AMF_ROCKET_TRAIL
    Bleeding = 12,    ///< BLEEDING_TRAIL
    Bleeding2 = 13    ///< BLEEDING_TRAIL2
};

/**
 * @brief Get name of trail type for debugging
 */
constexpr std::string_view trailTypeName(TrailType type) noexcept {
    switch (type) {
        case TrailType::Rocket:    return "rocket";
        case TrailType::Grenade:   return "grenade";
        case TrailType::AltRocket: return "alt_rocket";
        case TrailType::Blood:     return "blood";
        case TrailType::BigBlood:  return "big_blood";
        case TrailType::Tracer1:   return "tracer1";
        case TrailType::Tracer2:   return "tracer2";
        case TrailType::Voor:      return "voor";
        case TrailType::Rail:      return "rail";
        case TrailType::Rail2:     return "rail2";
        case TrailType::Lava:      return "lava";
        case TrailType::AmfRocket: return "amf_rocket";
        case TrailType::Bleeding:  return "bleeding";
        case TrailType::Bleeding2: return "bleeding2";
        default:                   return "unknown";
    }
}

//=============================================================================
// Explosion Types (from cl_tent.c r_explosiontype cvar)
//=============================================================================

/**
 * @brief Explosion rendering types
 * 
 * Original: r_explosiontype cvar values
 */
enum class ExplosionType : uint8_t {
    Normal = 0,        ///< Standard sprite + particles
    Sprite = 1,        ///< Same as normal
    Teleport = 2,      ///< Teleport splash effect
    LightningBlood = 3,///< Lightning blood particles
    BigBlood = 4,      ///< Large blood splatter
    DoubleGunshot = 5, ///< Double gunshot particles
    Blob = 6,          ///< Blob explosion
    Detpack = 7,       ///< Team Fortress detpack
    FuelRod = 8,       ///< Fuel rod explosion
    // 9 unused
    None = 10          ///< No explosion effect
};

/**
 * @brief Get name of explosion type for debugging
 */
constexpr std::string_view explosionTypeName(ExplosionType type) noexcept {
    switch (type) {
        case ExplosionType::Normal:        return "normal";
        case ExplosionType::Sprite:        return "sprite";
        case ExplosionType::Teleport:      return "teleport";
        case ExplosionType::LightningBlood:return "lightning_blood";
        case ExplosionType::BigBlood:      return "big_blood";
        case ExplosionType::DoubleGunshot: return "double_gunshot";
        case ExplosionType::Blob:          return "blob";
        case ExplosionType::Detpack:       return "detpack";
        case ExplosionType::FuelRod:       return "fuel_rod";
        case ExplosionType::None:          return "none";
        default:                           return "unknown";
    }
}

//=============================================================================
// Constants
//=============================================================================

namespace temp_entity {
    constexpr size_t MAX_TEMP_ENTITIES = 32;  ///< Max saved temp entities
    constexpr size_t MAX_BEAMS = 32;          ///< Max simultaneous beams
    constexpr size_t MAX_EXPLOSIONS = 32;     ///< Max simultaneous explosions
    constexpr size_t MAX_LIGHTNING_BEAMS = 10;///< Max lightning beam segments
    
    // Beam timing
    constexpr float BEAM_DURATION = 0.2f;     ///< How long beams last
    constexpr float BEAM_SEGMENT_LENGTH = 30.0f; ///< Length of each beam segment
    
    // Lightning bolt model indices
    constexpr int BOLT_MODEL_1 = 1;  ///< Shambler bolt
    constexpr int BOLT_MODEL_2 = 2;  ///< Wizard bolt  
    constexpr int BOLT_MODEL_3 = 3;  ///< Generic bolt
    constexpr int BEAM_MODEL = 4;    ///< Grappling hook beam
    
    // Blood colors (palette indices)
    constexpr int DEFAULT_LG_BLOOD_COLOR = 225;
    constexpr int DEFAULT_RL_BLOOD_SMALL = 225;
    constexpr int DEFAULT_RL_BLOOD_BIG = 73;
    constexpr int DEFAULT_SG_BLOOD_COLOR = 73;
    
    // Particle counts
    constexpr int SPIKE_PARTICLES = 10;
    constexpr int SUPERSPIKE_PARTICLES = 20;
    constexpr int WIZSPIKE_PARTICLES = 30;
    constexpr int LG_BLOOD_PARTICLES = 50;
}

//=============================================================================
// Saved Temp Entity (for demo replay)
//=============================================================================

/**
 * @brief Saved temp entity record
 * 
 * Original: temp_entity_t struct in protocol.h
 */
struct SavedTempEntity {
    Vec3 position{0.0f, 0.0f, 0.0f};  ///< Position
    float time{0.0f};                  ///< Timestamp
    TempEntityType type{TempEntityType::Spike}; ///< Entity type
    
    /**
     * @brief Check if entity is expired
     */
    [[nodiscard]] bool isExpired(float currentTime, float maxAge = 2.0f) const noexcept {
        return (currentTime - time) > maxAge;
    }
};

/**
 * @brief Ring buffer of saved temp entities
 * 
 * Original: temp_entity_list_t struct in protocol.h
 */
class TempEntityHistory {
public:
    static constexpr size_t MAX_ENTITIES = temp_entity::MAX_TEMP_ENTITIES;
    
    /**
     * @brief Add a temp entity to history
     */
    void add(const Vec3& pos, float time, TempEntityType type) noexcept {
        entities_[count_] = SavedTempEntity{pos, time, type};
        count_ = (count_ + 1) % MAX_ENTITIES;
        if (total_ < MAX_ENTITIES) {
            ++total_;
        }
    }
    
    /**
     * @brief Get entity at index
     */
    [[nodiscard]] const SavedTempEntity& at(size_t index) const noexcept {
        return entities_[index % MAX_ENTITIES];
    }
    
    /**
     * @brief Get current write position
     */
    [[nodiscard]] size_t writeIndex() const noexcept { return count_; }
    
    /**
     * @brief Get total entities stored (up to MAX)
     */
    [[nodiscard]] size_t size() const noexcept { return total_; }
    
    /**
     * @brief Check if empty
     */
    [[nodiscard]] bool empty() const noexcept { return total_ == 0; }
    
    /**
     * @brief Clear all history
     */
    void clear() noexcept {
        count_ = 0;
        total_ = 0;
    }
    
private:
    std::array<SavedTempEntity, MAX_ENTITIES> entities_;
    size_t count_{0};
    size_t total_{0};
};

//=============================================================================
// Explosion State
//=============================================================================

/**
 * @brief Explosion sprite state
 * 
 * Original: explosion_t struct in cl_tent.c
 */
struct ExplosionState {
    Vec3 origin{0.0f, 0.0f, 0.0f};  ///< Position
    float startTime{0.0f};          ///< When explosion started
    bool active{false};             ///< Is this slot in use
    
    // Model reference would be stored externally
    // model_t* model is handled by the renderer
    
    /**
     * @brief Get current animation frame
     * @param currentTime Current game time
     * @param framesPerSecond Animation speed (default 10 fps)
     * @return Current frame number
     */
    [[nodiscard]] int currentFrame(float currentTime, float framesPerSecond = 10.0f) const noexcept {
        if (!active) return 0;
        return static_cast<int>((currentTime - startTime) * framesPerSecond);
    }
    
    /**
     * @brief Check if explosion animation is complete
     * @param currentTime Current game time
     * @param totalFrames Total frames in animation
     */
    [[nodiscard]] bool isComplete(float currentTime, int totalFrames = 6) const noexcept {
        return currentFrame(currentTime) >= totalFrames;
    }
    
    /**
     * @brief Start a new explosion
     */
    void start(const Vec3& pos, float time) noexcept {
        origin = pos;
        startTime = time;
        active = true;
    }
    
    /**
     * @brief Clear this explosion slot
     */
    void clear() noexcept {
        origin = Vec3{0.0f, 0.0f, 0.0f};
        startTime = 0.0f;
        active = false;
    }
};

/**
 * @brief Pool of explosion slots with linked list allocation
 * 
 * Original: cl_explosions array with linked list in cl_tent.c
 */
class ExplosionPool {
public:
    static constexpr size_t MAX_EXPLOSIONS = temp_entity::MAX_EXPLOSIONS;
    
    ExplosionPool() noexcept {
        clear();
    }
    
    /**
     * @brief Allocate an explosion slot
     * @return Pointer to slot, or nullptr if pool is full
     */
    ExplosionState* allocate() noexcept {
        // Find first inactive slot
        for (auto& exp : explosions_) {
            if (!exp.active) {
                return &exp;
            }
        }
        
        // Pool full - find oldest explosion
        ExplosionState* oldest = nullptr;
        float oldestTime = std::numeric_limits<float>::max();
        for (auto& exp : explosions_) {
            if (exp.startTime < oldestTime) {
                oldestTime = exp.startTime;
                oldest = &exp;
            }
        }
        
        return oldest;
    }
    
    /**
     * @brief Update all explosions, freeing completed ones
     */
    void update(float currentTime, int totalFrames = 6) noexcept {
        for (auto& exp : explosions_) {
            if (exp.active && exp.isComplete(currentTime, totalFrames)) {
                exp.clear();
            }
        }
    }
    
    /**
     * @brief Get number of active explosions
     */
    [[nodiscard]] size_t activeCount() const noexcept {
        size_t count = 0;
        for (const auto& exp : explosions_) {
            if (exp.active) ++count;
        }
        return count;
    }
    
    /**
     * @brief Iterate over active explosions
     */
    template<typename Func>
    void forEachActive(Func&& func) const {
        for (const auto& exp : explosions_) {
            if (exp.active) {
                func(exp);
            }
        }
    }
    
    /**
     * @brief Clear all explosions
     */
    void clear() noexcept {
        for (auto& exp : explosions_) {
            exp.clear();
        }
    }
    
private:
    std::array<ExplosionState, MAX_EXPLOSIONS> explosions_;
};

//=============================================================================
// Beam State
//=============================================================================

/**
 * @brief Beam model type
 */
enum class BeamModelType : uint8_t {
    Bolt1 = 1,   ///< Shambler lightning (bolt.mdl)
    Bolt2 = 2,   ///< Wizard lightning (bolt2.mdl)
    Bolt3 = 3,   ///< Generic lightning (bolt3.mdl)
    Beam = 4     ///< Grappling hook beam (beam.mdl)
};

/**
 * @brief Lightning/beam state
 * 
 * Original: beam_t struct in cl_tent.c
 */
struct BeamState {
    int entity{0};                      ///< Entity that owns this beam
    BeamModelType modelType{BeamModelType::Bolt1}; ///< Model to render
    float endTime{0.0f};                ///< When beam expires
    Vec3 start{0.0f, 0.0f, 0.0f};       ///< Start position
    Vec3 end{0.0f, 0.0f, 0.0f};         ///< End position
    
    /**
     * @brief Check if beam is active
     */
    [[nodiscard]] bool isActive(float currentTime) const noexcept {
        return endTime > currentTime;
    }
    
    /**
     * @brief Calculate beam length
     */
    [[nodiscard]] float length() const noexcept {
        return (end - start).length();
    }
    
    /**
     * @brief Calculate beam direction (normalized)
     */
    [[nodiscard]] Vec3 direction() const noexcept {
        return (end - start).normalized();
    }
    
    /**
     * @brief Calculate pitch and yaw angles for beam
     * @return [pitch, yaw] in degrees
     */
    [[nodiscard]] std::array<float, 2> angles() const noexcept {
        Vec3 dir = end - start;
        
        if (dir.y() == 0.0f && dir.x() == 0.0f) {
            return {dir.z() > 0.0f ? 90.0f : 270.0f, 0.0f};
        }
        
        float yaw = std::atan2(dir.y(), dir.x()) * 180.0f / 3.14159265f;
        if (yaw < 0.0f) yaw += 360.0f;
        
        float forward = std::sqrt(dir.x() * dir.x() + dir.y() * dir.y());
        float pitch = std::atan2(dir.z(), forward) * 180.0f / 3.14159265f;
        if (pitch < 0.0f) pitch += 360.0f;
        
        return {pitch, yaw};
    }
    
    /**
     * @brief Update beam with new data
     */
    void update(int ent, BeamModelType model, const Vec3& startPos, 
                const Vec3& endPos, float currentTime) noexcept {
        entity = ent;
        modelType = model;
        start = startPos;
        end = endPos;
        endTime = currentTime + temp_entity::BEAM_DURATION;
    }
    
    /**
     * @brief Clear beam slot
     */
    void clear() noexcept {
        entity = 0;
        modelType = BeamModelType::Bolt1;
        endTime = 0.0f;
        start = Vec3{0.0f, 0.0f, 0.0f};
        end = Vec3{0.0f, 0.0f, 0.0f};
    }
};

/**
 * @brief Pool of beam slots
 * 
 * Original: cl_beams array in cl_tent.c
 */
class BeamPool {
public:
    static constexpr size_t MAX_BEAMS = temp_entity::MAX_BEAMS;
    
    BeamPool() noexcept {
        clear();
    }
    
    /**
     * @brief Find or allocate beam for entity
     * @param entity Entity number
     * @param currentTime Current time for expiration check
     * @return Pointer to beam slot, or nullptr if full
     */
    BeamState* findOrAllocate(int entity, float currentTime) noexcept {
        // First, look for existing beam for this entity
        for (auto& beam : beams_) {
            if (beam.entity == entity) {
                return &beam;
            }
        }
        
        // Find a free slot
        for (auto& beam : beams_) {
            if (!beam.isActive(currentTime)) {
                return &beam;
            }
        }
        
        // No free slots
        return nullptr;
    }
    
    /**
     * @brief Update beam for entity
     * @return true if beam was added/updated, false if pool full
     */
    bool updateBeam(int entity, BeamModelType model, const Vec3& start,
                    const Vec3& end, float currentTime) noexcept {
        BeamState* beam = findOrAllocate(entity, currentTime);
        if (!beam) return false;
        
        beam->update(entity, model, start, end, currentTime);
        return true;
    }
    
    /**
     * @brief Get number of active beams
     */
    [[nodiscard]] size_t activeCount(float currentTime) const noexcept {
        size_t count = 0;
        for (const auto& beam : beams_) {
            if (beam.isActive(currentTime)) ++count;
        }
        return count;
    }
    
    /**
     * @brief Iterate over active beams
     */
    template<typename Func>
    void forEachActive(float currentTime, Func&& func) const {
        for (const auto& beam : beams_) {
            if (beam.isActive(currentTime)) {
                func(beam);
            }
        }
    }
    
    /**
     * @brief Get player beam end position (for cl_fakeshaft)
     */
    [[nodiscard]] std::optional<Vec3> getPlayerBeamEnd(int playerNum, float currentTime) const noexcept {
        int playerEntity = playerNum + 1;
        for (const auto& beam : beams_) {
            if (beam.entity == playerEntity && beam.isActive(currentTime)) {
                return beam.end;
            }
        }
        return std::nullopt;
    }
    
    /**
     * @brief Clear all beams
     */
    void clear() noexcept {
        for (auto& beam : beams_) {
            beam.clear();
        }
    }
    
private:
    std::array<BeamState, MAX_BEAMS> beams_;
};

//=============================================================================
// Rail Trail Colors (for instagib)
//=============================================================================

/**
 * @brief Rail trail color indices
 * 
 * Original: colors array in CL_ParseBeam for rail trails
 */
enum class RailColor : uint8_t {
    White = 0,       ///< Color index 6
    Blue = 1,        ///< Color index 208
    Green = 2,       ///< Color index 180
    LightBlue = 3,   ///< Color index 35
    Red = 4,         ///< Color index 224
    Magenta = 5,     ///< Color index 133
    Yellow = 6,      ///< Color index 192
    White2 = 7       ///< Color index 6 (duplicate)
};

/**
 * @brief Get palette index for rail color
 */
constexpr int railColorToPalette(RailColor color) noexcept {
    constexpr std::array<int, 8> colors = {
        6,    // White
        208,  // Blue
        180,  // Green
        35,   // Light blue
        224,  // Red
        133,  // Magenta
        192,  // Yellow
        6     // White
    };
    return colors[static_cast<size_t>(color) % colors.size()];
}

/**
 * @brief Decode rail trail entity number
 * 
 * Entity numbers -512 to -1 encode player number and color
 * Format: 1111111nnnnnnccc (n = player 0-63, c = color 0-7)
 */
struct RailTrailInfo {
    int playerNum{0};    ///< Player number (0-63)
    RailColor color{RailColor::White}; ///< Trail color
    
    /**
     * @brief Decode from entity number
     * @param entity Entity number (-512 to -1)
     * @return Decoded info, or nullopt if not a rail trail
     */
    static std::optional<RailTrailInfo> decode(int entity) noexcept {
        if (entity < -512 || entity > -1) {
            return std::nullopt;
        }
        
        RailTrailInfo info;
        info.color = static_cast<RailColor>(entity & 7);
        info.playerNum = (entity >> 3) & 63;
        return info;
    }
    
    /**
     * @brief Get palette index for this trail
     */
    [[nodiscard]] int paletteIndex() const noexcept {
        return railColorToPalette(color);
    }
};

//=============================================================================
// Instagib Trail Types
//=============================================================================

/**
 * @brief Instagib trail rendering style
 * 
 * Original: r_instagibtrail cvar values
 */
enum class InstagibTrailType : uint8_t {
    None = 0,
    Grenade = 1,
    Rocket = 2,
    AltRocket = 3,
    Blood = 4,
    BigBlood = 5,
    Tracer1 = 6,
    Tracer2 = 7,
    Voor = 8,
    ClassicRail = 9,
    Rail = 10,
    QmbRail = 11,
    Lava = 12,
    AmfRocket = 13
};

/**
 * @brief Convert instagib trail type to particle trail type
 */
constexpr std::optional<TrailType> instagibToTrailType(InstagibTrailType type) noexcept {
    switch (type) {
        case InstagibTrailType::Grenade:   return TrailType::Grenade;
        case InstagibTrailType::Rocket:    return TrailType::Rocket;
        case InstagibTrailType::AltRocket: return TrailType::AltRocket;
        case InstagibTrailType::Blood:     return TrailType::Blood;
        case InstagibTrailType::BigBlood:  return TrailType::BigBlood;
        case InstagibTrailType::Tracer1:   return TrailType::Tracer1;
        case InstagibTrailType::Tracer2:   return TrailType::Tracer2;
        case InstagibTrailType::Voor:      return TrailType::Voor;
        case InstagibTrailType::Rail:      return TrailType::Rail;
        case InstagibTrailType::Lava:      return TrailType::Lava;
        case InstagibTrailType::AmfRocket: return TrailType::AmfRocket;
        default:                           return std::nullopt;
    }
}

//=============================================================================
// Spark Effect
//=============================================================================

/**
 * @brief Spark generation parameters
 * 
 * Original: SparkGen function in cl_tent.c
 */
struct SparkParams {
    Vec3 position{0.0f, 0.0f, 0.0f};  ///< Origin
    std::array<uint8_t, 3> color{255, 255, 255}; ///< RGB color
    int count{10};                    ///< Number of sparks
    float size{50.0f};                ///< Spark size
    float duration{1.0f};             ///< How long sparks last
    
    /**
     * @brief Create spark params for wizard spike
     */
    static SparkParams wizSpike(const Vec3& pos) noexcept {
        return {pos, {0, 124, 0}, 20, 90.0f, 1.0f};
    }
    
    /**
     * @brief Create spark params for knight spike
     */
    static SparkParams knightSpike(const Vec3& pos) noexcept {
        return {pos, {255, 77, 0}, 20, 150.0f, 1.0f};
    }
    
    /**
     * @brief Create spark params for lightning
     */
    static SparkParams lightning(const Vec3& pos, float sparksValue, float sparksSizeValue,
                                  const std::array<uint8_t, 3>& lightningColor) noexcept {
        return {pos, lightningColor, static_cast<int>(3 * sparksValue), sparksSizeValue, 0.25f};
    }
};

//=============================================================================
// Temp Entity Manager State
//=============================================================================

/**
 * @brief Combined state for all temporary entities
 * 
 * Original: Global state in cl_tent.c
 */
struct TempEntityState {
    BeamPool beams;               ///< Lightning beams
    ExplosionPool explosions;     ///< Explosion sprites
    TempEntityHistory history;    ///< Saved temp entities for demo
    
    // Player beam tracking for cl_fakeshaft
    Vec3 playerBeamEnd{0.0f, 0.0f, 0.0f};
    bool playerBeamUpdated{false};
    
    /**
     * @brief Clear all temp entity state
     */
    void clear() noexcept {
        beams.clear();
        explosions.clear();
        history.clear();
        playerBeamEnd = Vec3{0.0f, 0.0f, 0.0f};
        playerBeamUpdated = false;
    }
    
    /**
     * @brief Update all temp entities
     */
    void update(float currentTime) noexcept {
        explosions.update(currentTime);
        // Beams are cleaned up lazily in forEachActive
    }
};

} // namespace ezquake
