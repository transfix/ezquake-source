/**
 * @file particle_types.hpp
 * @brief Particle system type definitions for ezQuake C++20 port
 * 
 * This file contains particle types and constants extracted from:
 * - r_particles_qmb.h - QMB particle system types
 * - r_part.c - Classic particle system
 * - particles_qmb.c - QMB particle implementation
 * 
 * Part of ezQuake C to C++20 Conversion - Iteration 24
 */

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <array>
#include "core/math/vec3.hpp"

namespace ezquake::render {

// =============================================================================
// Particle Limits
// =============================================================================

namespace particle_limits {
    constexpr int MIN_PARTICLES = 256;
    constexpr int MAX_PARTICLES = 32768;
    constexpr int DEFAULT_PARTICLES = 2048;
    constexpr int MAX_TEXTURE_COMPONENTS = 8;
    constexpr int TRAIL_MAX_LENGTH = 100;
    constexpr int TRAIL_NEAR_ALPHA = 75;
}

// =============================================================================
// Particle Type (from r_particles_qmb.h)
// =============================================================================

/**
 * @brief Types of QMB particles
 */
enum class ParticleType : uint8_t {
    // Basic effects
    Spark = 0,
    Smoke,
    Fire,
    Bubble,
    LavaSplash,
    GunBlast,
    Chunk,
    Shockwave,
    
    // Inferno effects
    InfernoFlame,
    InfernoTrail,
    SparkRay,
    StaticBubble,
    TrailPart,
    
    // Dark Places style
    DPSmoke,
    DPFire,
    TeleFlare,
    
    // Blood effects
    Blood1,
    Blood2,
    Blood3,
    
    // VULT particles
    Rain,
    AlphaTrail,
    RailTrail,
    Streak,
    StreakTrail,
    StreakWave,
    LightningBeam,
    VXBlood,
    LavaTrail,
    VXSmoke,
    VXSmokeRed,
    MuzzleFlash,
    Inferno,
    Shockwave2D,
    VXRocketSmoke,
    TrailBleed,
    BleedSpike,
    Flame,
    Bubble2,
    BloodCloud,
    ChunkDir,
    SmallSpark,
    
    // HyperNewbie particles
    SlimeGlow,
    SlimeBubble,
    BlackLavaSmoke,
    NailTrail,
    FlameTorch,
    
    Count
};

/**
 * @brief Get particle type name
 */
[[nodiscard]] inline std::string_view particleTypeName(ParticleType type) noexcept {
    switch (type) {
        case ParticleType::Spark: return "spark";
        case ParticleType::Smoke: return "smoke";
        case ParticleType::Fire: return "fire";
        case ParticleType::Bubble: return "bubble";
        case ParticleType::Blood1: return "blood1";
        case ParticleType::Blood2: return "blood2";
        case ParticleType::Blood3: return "blood3";
        case ParticleType::Rain: return "rain";
        case ParticleType::RailTrail: return "railtrail";
        case ParticleType::MuzzleFlash: return "muzzleflash";
        case ParticleType::Flame: return "flame";
        default: return "particle";
    }
}

/**
 * @brief Check if particle type is a blood effect
 */
[[nodiscard]] constexpr bool isBloodParticle(ParticleType type) noexcept {
    return type == ParticleType::Blood1 ||
           type == ParticleType::Blood2 ||
           type == ParticleType::Blood3 ||
           type == ParticleType::VXBlood ||
           type == ParticleType::BloodCloud;
}

/**
 * @brief Check if particle type is a trail effect
 */
[[nodiscard]] constexpr bool isTrailParticle(ParticleType type) noexcept {
    return type == ParticleType::InfernoTrail ||
           type == ParticleType::TrailPart ||
           type == ParticleType::AlphaTrail ||
           type == ParticleType::RailTrail ||
           type == ParticleType::StreakTrail ||
           type == ParticleType::LavaTrail ||
           type == ParticleType::TrailBleed ||
           type == ParticleType::NailTrail;
}

// =============================================================================
// Particle Movement (from r_particles_qmb.h)
// =============================================================================

/**
 * @brief Particle movement behaviors
 */
enum class ParticleMove : uint8_t {
    Static = 0,     // No movement
    Normal,         // Standard physics
    Bounce,         // Bounce off surfaces
    Die,            // Die on impact
    NoPhysics,      // No physics simulation
    Float,          // Float upward
    Rain,           // Rain physics
    Streak,         // Streak movement
    StreakWave,     // Wave-like streak
    Trail,          // Trail behavior
    
    Count
};

/**
 * @brief Get particle movement name
 */
[[nodiscard]] inline std::string_view particleMoveName(ParticleMove move) noexcept {
    switch (move) {
        case ParticleMove::Static: return "static";
        case ParticleMove::Normal: return "normal";
        case ParticleMove::Bounce: return "bounce";
        case ParticleMove::Die: return "die";
        case ParticleMove::NoPhysics: return "nophysics";
        case ParticleMove::Float: return "float";
        case ParticleMove::Rain: return "rain";
        case ParticleMove::Streak: return "streak";
        case ParticleMove::Trail: return "trail";
        default: return "unknown";
    }
}

/**
 * @brief Check if movement type uses physics
 */
[[nodiscard]] constexpr bool usesPhysics(ParticleMove move) noexcept {
    return move != ParticleMove::Static &&
           move != ParticleMove::NoPhysics;
}

// =============================================================================
// Particle Texture (from r_particles_qmb.h)
// =============================================================================

/**
 * @brief Particle texture identifiers
 */
enum class ParticleTexture : uint8_t {
    None = 0,
    Smoke,
    Bubble,
    Generic,
    DPSmoke,
    Lava,
    BlueFlare,
    Blood1,
    Blood2,
    Blood3,
    Shockwave,
    Lightning,
    Spark,
    
    Count
};

/**
 * @brief Get particle texture name
 */
[[nodiscard]] inline std::string_view particleTextureName(ParticleTexture tex) noexcept {
    switch (tex) {
        case ParticleTexture::None: return "none";
        case ParticleTexture::Smoke: return "smoke";
        case ParticleTexture::Bubble: return "bubble";
        case ParticleTexture::Generic: return "generic";
        case ParticleTexture::Blood1: return "blood1";
        case ParticleTexture::Lightning: return "lightning";
        case ParticleTexture::Spark: return "spark";
        default: return "unknown";
    }
}

// =============================================================================
// Particle Draw Type (from r_particles_qmb.h)
// =============================================================================

/**
 * @brief How particles are rendered
 */
enum class ParticleDraw : uint8_t {
    Spark = 0,          // Spark points
    SparkRay,           // Ray from origin
    Billboard,          // Camera-facing quad
    BillboardVel,       // Billboard aligned to velocity
    Beam,               // Beam between points
    Hide,               // Hidden (for logic only)
    Normal,             // Normal rendering
    DynamicTrail,       // Dynamic trail
    Torch,              // Torch flame
    
    Count
};

/**
 * @brief Get draw type name
 */
[[nodiscard]] inline std::string_view particleDrawName(ParticleDraw draw) noexcept {
    switch (draw) {
        case ParticleDraw::Spark: return "spark";
        case ParticleDraw::SparkRay: return "sparkray";
        case ParticleDraw::Billboard: return "billboard";
        case ParticleDraw::BillboardVel: return "billboard_vel";
        case ParticleDraw::Beam: return "beam";
        case ParticleDraw::Torch: return "torch";
        default: return "normal";
    }
}

/**
 * @brief Check if draw type uses quads
 */
[[nodiscard]] constexpr bool usesQuads(ParticleDraw draw) noexcept {
    return draw == ParticleDraw::Billboard ||
           draw == ParticleDraw::BillboardVel;
}

// =============================================================================
// Particle Blend (from r_particles_qmb.h)
// =============================================================================

/**
 * @brief Particle blending modes
 */
enum class ParticleBlend : uint8_t {
    SrcAlphaOneMinusSrcAlpha = 0,   // Standard alpha blending
    SrcAlphaOne,                     // Additive with alpha
    OneOne,                          // Pure additive
    ZeroOneMinusSrcColorConstant,    // Darkening (constant)
    ZeroOneMinusSrcColor,            // Darkening (per-channel)
    
    Count
};

/**
 * @brief Get blend mode name
 */
[[nodiscard]] inline std::string_view particleBlendName(ParticleBlend blend) noexcept {
    switch (blend) {
        case ParticleBlend::SrcAlphaOneMinusSrcAlpha: return "alpha";
        case ParticleBlend::SrcAlphaOne: return "additive_alpha";
        case ParticleBlend::OneOne: return "additive";
        case ParticleBlend::ZeroOneMinusSrcColorConstant: return "darken_const";
        case ParticleBlend::ZeroOneMinusSrcColor: return "darken";
        default: return "unknown";
    }
}

/**
 * @brief Check if blend mode is additive
 */
[[nodiscard]] constexpr bool isAdditiveBlend(ParticleBlend blend) noexcept {
    return blend == ParticleBlend::SrcAlphaOne ||
           blend == ParticleBlend::OneOne;
}

// =============================================================================
// Classic Particle Ramps
// =============================================================================

/**
 * @brief Color ramp for classic rocket trail particles
 */
inline constexpr std::array<uint8_t, 8> PARTICLE_RAMP1 = {
    0x6f, 0x6d, 0x6b, 0x69, 0x67, 0x65, 0x63, 0x61
};

/**
 * @brief Color ramp for classic grenade trail particles
 */
inline constexpr std::array<uint8_t, 8> PARTICLE_RAMP2 = {
    0x6f, 0x6e, 0x6d, 0x6c, 0x6b, 0x6a, 0x68, 0x66
};

/**
 * @brief Color ramp for classic blood particles
 */
inline constexpr std::array<uint8_t, 6> PARTICLE_RAMP3 = {
    0x6d, 0x6b, 6, 5, 4, 3
};

// =============================================================================
// Particle Color
// =============================================================================

/**
 * @brief RGBA particle color
 */
struct ParticleColor {
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    uint8_t a = 255;
    
    [[nodiscard]] constexpr bool isOpaque() const noexcept { return a == 255; }
    [[nodiscard]] constexpr bool isTransparent() const noexcept { return a == 0; }
    
    [[nodiscard]] static constexpr ParticleColor white() noexcept { return {255, 255, 255, 255}; }
    [[nodiscard]] static constexpr ParticleColor black() noexcept { return {0, 0, 0, 255}; }
    [[nodiscard]] static constexpr ParticleColor red() noexcept { return {255, 0, 0, 255}; }
    [[nodiscard]] static constexpr ParticleColor green() noexcept { return {0, 255, 0, 255}; }
    [[nodiscard]] static constexpr ParticleColor blue() noexcept { return {0, 0, 255, 255}; }
    [[nodiscard]] static constexpr ParticleColor yellow() noexcept { return {255, 255, 0, 255}; }
};

// =============================================================================
// Particle Instance
// =============================================================================

/**
 * @brief Single particle instance data
 */
struct Particle {
    // Position and movement
    math::Vec3 org{};         // Current position
    math::Vec3 endorg{};      // End position (for beams)
    math::Vec3 vel{};         // Velocity
    
    // Appearance
    ParticleColor color{};
    float size = 1.0f;
    float rotAngle = 0.0f;
    float rotSpeed = 0.0f;
    float growth = 0.0f;    // Size change rate
    uint8_t texIndex = 0;
    
    // Lifetime
    float start = 0.0f;     // Birth time
    float die = 0.0f;       // Death time
    uint8_t initialAlpha = 255;
    
    // Physics state
    uint8_t hit = 0;        // Hit surface
    uint8_t bounces = 0;    // Bounce count
    int cachedContents = 0;
    Vec3 cachedMovement{};
    float cachedDistance = 0.0f;
    
    // Entity tracking (for trails)
    int entityRef = -1;
    int entityTrailIndex = 0;
    int entityTrailNumber = 0;
    
    [[nodiscard]] bool isAlive(float time) const noexcept {
        return time < die;
    }
    
    [[nodiscard]] float lifeFraction(float time) const noexcept {
        float total = die - start;
        if (total <= 0.0f) return 1.0f;
        return (time - start) / total;
    }
    
    [[nodiscard]] float remainingLife(float time) const noexcept {
        return die - time;
    }
};

// =============================================================================
// Particle Type Definition
// =============================================================================

/**
 * @brief Defines properties for a type of particle
 */
struct ParticleTypeDef {
    ParticleType id = ParticleType::Spark;
    ParticleDraw drawType = ParticleDraw::Normal;
    ParticleBlend blendType = ParticleBlend::SrcAlphaOneMinusSrcAlpha;
    ParticleTexture texture = ParticleTexture::None;
    ParticleMove moveType = ParticleMove::Normal;
    
    float startAlpha = 1.0f;
    float gravity = 0.0f;
    float accel = 0.0f;
    float custom = 0.0f;
    
    int vertsPerPrimitive = 4;  // Usually quads
    int particleCount = 0;      // Active count
    
    [[nodiscard]] bool usesGravity() const noexcept {
        return gravity != 0.0f;
    }
    
    [[nodiscard]] bool isAdditive() const noexcept {
        return isAdditiveBlend(blendType);
    }
};

// =============================================================================
// Particle Texture Info
// =============================================================================

/**
 * @brief Texture coordinates for particle texture atlas
 */
struct ParticleTextureInfo {
    static constexpr int MAX_COMPONENTS = 8;
    
    uint32_t textureId = 0;
    uint32_t arrayTextureId = 0;
    int arrayIndex = 0;
    int componentCount = 0;
    
    // UV coordinates for each component [component][u1, v1, u2, v2]
    std::array<std::array<float, 4>, MAX_COMPONENTS> coords{};
    std::array<std::array<float, 4>, MAX_COMPONENTS> originalCoords{};
    
    [[nodiscard]] bool isValid() const noexcept {
        return textureId != 0 && componentCount > 0;
    }
    
    [[nodiscard]] const std::array<float, 4>& getCoords(std::size_t component) const noexcept {
        return coords[component % static_cast<std::size_t>(componentCount)];
    }
};

// =============================================================================
// Classic Particle
// =============================================================================

/**
 * @brief Simple particle for classic Quake particle system
 */
struct ClassicParticle {
    math::Vec3 org{};
    math::Vec3 vel{};
    float ramp = 0.0f;
    float die = 0.0f;
    int type = 0;       // Particle behavior type
    uint8_t color = 0;  // Palette index
    
    [[nodiscard]] bool isAlive(float time) const noexcept {
        return time < die;
    }
};

// =============================================================================
// Particle Effect Types
// =============================================================================

/**
 * @brief Trail effect types
 */
enum class TrailType : uint8_t {
    Rocket = 0,         // Rocket trail
    Grenade,            // Grenade trail
    Blood,              // Blood trail
    WizSpike,           // Wizard spike trail
    SlightBlood,        // Slight blood trail
    KnightSpike,        // Knight spike trail
    VoorTrail,          // Vore missile trail
    
    Count
};

/**
 * @brief Explosion types
 */
enum class ExplosionType : uint8_t {
    Normal = 0,         // Standard explosion
    Blood,              // Blood explosion
    TarExplosion,       // Tar baby explosion
    Detpack,            // Team Fortress detpack
    
    Count
};

// =============================================================================
// Helper Functions
// =============================================================================

/**
 * @brief Linear random in range [min, max]
 */
[[nodiscard]] inline float lhrandom(float min, float max) noexcept {
    return (static_cast<float>(std::rand() & 32767) / 32767.0f) * (max - min) + min;
}

/**
 * @brief Random in range [-1, 1]
 */
[[nodiscard]] inline float crand() noexcept {
    return (static_cast<float>(std::rand() & 32767) / 32767.0f) * 2.0f - 1.0f;
}

} // namespace ezquake::render
