/**
 * @file effects_types.hpp
 * @brief Visual effects types for particles, explosions, dynamic lights, and beams
 * 
 * Modern C++20 implementation of particle system types, dynamic lighting,
 * explosions, beams, and temp entity effects from r_part.c, r_particles_qmb.h,
 * cl_tent.c, and client.h.
 * 
 * @note Iteration 35: Effect system types
 */

#pragma once

#include <array>
#include <cstdint>
#include <string_view>
#include <optional>
#include <chrono>
#include <algorithm>

#include "../math/vec3.hpp"

namespace ezquake::effects {

//=============================================================================
// Effect System Constants
//=============================================================================

namespace effect_limits {
    // Classic particle limits
    inline constexpr int ABSOLUTE_MIN_PARTICLES = 512;
    inline constexpr int ABSOLUTE_MAX_PARTICLES = 8192;
    inline constexpr int DEFAULT_PARTICLES = 2048;
    
    // QMB particle limits (extended)
    inline constexpr int QMB_MIN_PARTICLES = 256;
    inline constexpr int QMB_MAX_PARTICLES = 32768;
    
    // Temporary entities
    inline constexpr int MAX_BEAMS = 32;
    inline constexpr int MAX_EXPLOSIONS = 32;
    inline constexpr int MAX_DLIGHTS = 32;
    
    // Light styles
    inline constexpr int MAX_LIGHTSTYLES = 64;
    inline constexpr int MAX_STYLESTRING = 64;
    
    // Color ramps
    inline constexpr int COLOR_RAMP_SIZE = 8;
    
    // Particle texture components
    inline constexpr int MAX_PTEX_COMPONENTS = 8;
}

//=============================================================================
// Classic Particle Types (Quake original)
//=============================================================================

/**
 * @brief Classic particle behavior type (original Quake)
 * 
 * Converted from: particles_classic.h ptype_t
 */
enum class ClassicParticleType : uint8_t {
    Static = 0,     ///< No movement, fades out
    Gravity,        ///< Falls with gravity
    SlowGravity,    ///< Falls slowly (blood drops)
    Fire,           ///< Fire effect (moves up)
    Explode,        ///< Explosion particles (ramp1 colors)
    Explode2,       ///< Explosion particles (ramp2 colors)
    Blob,           ///< Blob particles (tar baby)
    Blob2,          ///< Secondary blob
    Rail,           ///< Rail trail particles
    
    Count
};

/**
 * @brief Get human-readable name for classic particle type
 */
[[nodiscard]] constexpr std::string_view classicParticleTypeName(ClassicParticleType type) noexcept {
    switch (type) {
        case ClassicParticleType::Static:      return "Static";
        case ClassicParticleType::Gravity:     return "Gravity";
        case ClassicParticleType::SlowGravity: return "SlowGravity";
        case ClassicParticleType::Fire:        return "Fire";
        case ClassicParticleType::Explode:     return "Explode";
        case ClassicParticleType::Explode2:    return "Explode2";
        case ClassicParticleType::Blob:        return "Blob";
        case ClassicParticleType::Blob2:       return "Blob2";
        case ClassicParticleType::Rail:        return "Rail";
        default:                               return "Unknown";
    }
}

//=============================================================================
// QMB Extended Particle Types
//=============================================================================

/**
 * @brief QMB particle visual type (enhanced particles)
 * 
 * Converted from: r_particles_qmb.h part_type_t
 */
enum class QMBParticleType : uint8_t {
    // Core types
    Spark = 0,
    Smoke,
    Fire,
    Bubble,
    LavaSplash,
    GunBlast,
    Chunk,
    ShockWave,
    InfernoFlame,
    InfernoTrail,
    SparkRay,
    StaticBubble,
    TrailPart,
    DPSmoke,
    DPFire,
    TeleFlare,
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
    ShockWave2D,
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

[[nodiscard]] constexpr bool isBloodParticle(QMBParticleType type) noexcept {
    return type == QMBParticleType::Blood1 ||
           type == QMBParticleType::Blood2 ||
           type == QMBParticleType::Blood3 ||
           type == QMBParticleType::VXBlood ||
           type == QMBParticleType::BloodCloud ||
           type == QMBParticleType::TrailBleed ||
           type == QMBParticleType::BleedSpike;
}

[[nodiscard]] constexpr bool isTrailParticle(QMBParticleType type) noexcept {
    return type == QMBParticleType::TrailPart ||
           type == QMBParticleType::AlphaTrail ||
           type == QMBParticleType::RailTrail ||
           type == QMBParticleType::StreakTrail ||
           type == QMBParticleType::LavaTrail ||
           type == QMBParticleType::NailTrail ||
           type == QMBParticleType::InfernoTrail;
}

//=============================================================================
// Particle Movement Types
//=============================================================================

/**
 * @brief Particle physics behavior
 * 
 * Converted from: r_particles_qmb.h part_move_t
 */
enum class ParticleMovement : uint8_t {
    Static = 0,     ///< No movement
    Normal,         ///< Standard movement with gravity
    Bounce,         ///< Bounce off surfaces
    Die,            ///< Die on impact
    NoPhysics,      ///< No physics (purely visual)
    Float,          ///< Float (bubbles)
    
    // VULT extensions
    Rain,           ///< Rain drop movement
    Streak,         ///< Streak effect
    StreakWave,     ///< Wave streak
    Trail,          ///< Trail movement
    
    Count
};

[[nodiscard]] constexpr bool hasCollision(ParticleMovement move) noexcept {
    return move == ParticleMovement::Bounce ||
           move == ParticleMovement::Die ||
           move == ParticleMovement::Rain;
}

//=============================================================================
// Particle Texture Types
//=============================================================================

/**
 * @brief Particle texture identifiers
 * 
 * Converted from: r_particles_qmb.h part_tex_t
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
    
    // VULT textures
    ShockWave,
    Lightning,
    Spark,
    
    Count
};

//=============================================================================
// Particle Draw Types
//=============================================================================

/**
 * @brief How to draw the particle
 * 
 * Converted from: r_particles_qmb.h part_draw_t
 */
enum class ParticleDrawType : uint8_t {
    Spark = 0,          ///< Point/spark
    SparkRay,           ///< Ray from spark
    Billboard,          ///< Face camera
    BillboardVel,       ///< Billboard aligned to velocity
    
    // VULT types
    Beam,               ///< Beam/line
    Hide,               ///< Hidden (for calculations)
    Normal,             ///< Normal rendering
    DynamicTrail,       ///< Dynamic trail
    Torch,              ///< Torch flame
    
    Count
};

[[nodiscard]] constexpr bool isBillboard(ParticleDrawType draw) noexcept {
    return draw == ParticleDrawType::Billboard ||
           draw == ParticleDrawType::BillboardVel;
}

//=============================================================================
// Particle Blend Types
//=============================================================================

/**
 * @brief Particle blending mode
 * 
 * Converted from: r_particles_qmb.h part_blend_id
 */
enum class ParticleBlendMode : uint8_t {
    SrcAlpha_OneMinusSrcAlpha = 0,  ///< Standard transparency
    SrcAlpha_One,                    ///< Additive with alpha
    One_One,                         ///< Pure additive
    Zero_OneMinusSrcColorConstant,   ///< Reduce by constant
    Zero_OneMinusSrcColor,           ///< Reduce by varying color
    
    Count
};

[[nodiscard]] constexpr bool isAdditive(ParticleBlendMode blend) noexcept {
    return blend == ParticleBlendMode::SrcAlpha_One ||
           blend == ParticleBlendMode::One_One;
}

//=============================================================================
// Trail Types
//=============================================================================

/**
 * @brief Entity trail type
 * 
 * Converted from: quakedef.h trail_type_t
 */
enum class TrailType : uint8_t {
    Rocket = 0,     ///< Rocket trail (fire + smoke)
    Grenade,        ///< Grenade trail
    AltRocket,      ///< Alternative rocket trail
    Blood,          ///< Blood trail (gore)
    BigBlood,       ///< Large blood splatter
    Tracer1,        ///< Green tracer (wizard)
    Tracer2,        ///< Orange tracer (hellknight)
    Voorhees,       ///< Vore trail
    
    // VULT extensions
    Rail,           ///< Rail gun trail
    Rail2,          ///< Alternative rail trail
    Lava,           ///< Lava trail
    AMFRocket,      ///< AMF rocket trail
    Bleeding,       ///< Bleeding trail
    Bleeding2,      ///< Alternative bleeding trail
    
    Count
};

[[nodiscard]] constexpr std::string_view trailTypeName(TrailType type) noexcept {
    switch (type) {
        case TrailType::Rocket:     return "Rocket";
        case TrailType::Grenade:    return "Grenade";
        case TrailType::AltRocket:  return "AltRocket";
        case TrailType::Blood:      return "Blood";
        case TrailType::BigBlood:   return "BigBlood";
        case TrailType::Tracer1:    return "Tracer1";
        case TrailType::Tracer2:    return "Tracer2";
        case TrailType::Voorhees:   return "Voorhees";
        case TrailType::Rail:       return "Rail";
        case TrailType::Rail2:      return "Rail2";
        case TrailType::Lava:       return "Lava";
        case TrailType::AMFRocket:  return "AMFRocket";
        case TrailType::Bleeding:   return "Bleeding";
        case TrailType::Bleeding2:  return "Bleeding2";
        default:                    return "Unknown";
    }
}

[[nodiscard]] constexpr bool isBloodTrail(TrailType type) noexcept {
    return type == TrailType::Blood ||
           type == TrailType::BigBlood ||
           type == TrailType::Bleeding ||
           type == TrailType::Bleeding2;
}

//=============================================================================
// Dynamic Light Types
//=============================================================================

/**
 * @brief Dynamic light color type
 * 
 * Converted from: client.h dlighttype_t
 */
enum class DynamicLightType : uint8_t {
    Default = 0,    ///< Standard light
    MuzzleFlash,    ///< Muzzle flash (bright, fast decay)
    Explosion,      ///< Explosion light
    Rocket,         ///< Rocket glow
    Red,
    Blue,
    RedBlue,
    Green,
    RedGreen,
    BlueGreen,
    White,
    Custom,         ///< Custom RGB color
    
    Count
};

[[nodiscard]] constexpr std::string_view dynamicLightTypeName(DynamicLightType type) noexcept {
    switch (type) {
        case DynamicLightType::Default:     return "Default";
        case DynamicLightType::MuzzleFlash: return "MuzzleFlash";
        case DynamicLightType::Explosion:   return "Explosion";
        case DynamicLightType::Rocket:      return "Rocket";
        case DynamicLightType::Red:         return "Red";
        case DynamicLightType::Blue:        return "Blue";
        case DynamicLightType::RedBlue:     return "RedBlue";
        case DynamicLightType::Green:       return "Green";
        case DynamicLightType::RedGreen:    return "RedGreen";
        case DynamicLightType::BlueGreen:   return "BlueGreen";
        case DynamicLightType::White:       return "White";
        case DynamicLightType::Custom:      return "Custom";
        default:                            return "Unknown";
    }
}

/**
 * @brief Get base color for dynamic light type
 * @return RGB color (0-255 range per component)
 */
[[nodiscard]] constexpr std::array<uint8_t, 3> dynamicLightColor(DynamicLightType type) noexcept {
    switch (type) {
        case DynamicLightType::Default:
        case DynamicLightType::White:       return {255, 255, 255};
        case DynamicLightType::MuzzleFlash: return {255, 180, 80};
        case DynamicLightType::Explosion:   return {255, 150, 50};
        case DynamicLightType::Rocket:      return {255, 120, 50};
        case DynamicLightType::Red:         return {255, 0, 0};
        case DynamicLightType::Blue:        return {0, 0, 255};
        case DynamicLightType::RedBlue:     return {255, 0, 255};
        case DynamicLightType::Green:       return {0, 255, 0};
        case DynamicLightType::RedGreen:    return {255, 255, 0};
        case DynamicLightType::BlueGreen:   return {0, 255, 255};
        default:                            return {255, 255, 255};
    }
}

//=============================================================================
// Color Ramps (for palette-based particle effects)
//=============================================================================

namespace color_ramp {
    // Fire/explosion color ramps (palette indices)
    inline constexpr std::array<uint8_t, 8> RAMP1 = {0x6f, 0x6d, 0x6b, 0x69, 0x67, 0x65, 0x63, 0x61};
    inline constexpr std::array<uint8_t, 8> RAMP2 = {0x6f, 0x6e, 0x6d, 0x6c, 0x6b, 0x6a, 0x68, 0x66};
    inline constexpr std::array<uint8_t, 6> RAMP3 = {0x6d, 0x6b, 6, 5, 4, 3};
}

//=============================================================================
// Effect Structures
//=============================================================================

/**
 * @brief RGBA color value for particles (byte precision)
 */
struct ParticleColor {
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    uint8_t a = 255;
    
    constexpr ParticleColor() = default;
    constexpr ParticleColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255) 
        : r(red), g(green), b(blue), a(alpha) {}
    
    /**
     * @brief Create from Quake palette index
     */
    [[nodiscard]] static ParticleColor fromPaletteIndex(uint8_t index);
    
    /**
     * @brief Create from float RGB (0-1 range)
     */
    [[nodiscard]] static constexpr ParticleColor fromFloat(float r, float g, float b, float a = 1.0f) {
        return ParticleColor(
            static_cast<uint8_t>(r * 255.0f),
            static_cast<uint8_t>(g * 255.0f),
            static_cast<uint8_t>(b * 255.0f),
            static_cast<uint8_t>(a * 255.0f)
        );
    }
    
    [[nodiscard]] constexpr std::array<float, 4> toFloat() const noexcept {
        return {r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f};
    }
    
    [[nodiscard]] constexpr bool operator==(const ParticleColor& other) const noexcept = default;
};

/**
 * @brief Classic particle (original Quake format)
 * 
 * Converted from: particles_classic.h qparticle_t
 */
struct ClassicParticle {
    math::Vec3 origin{};
    math::Vec3 velocity{};
    float color = 0.0f;             ///< Palette color (can interpolate)
    float ramp = 0.0f;              ///< Color ramp position
    float die = 0.0f;               ///< Time to die
    ClassicParticleType type = ClassicParticleType::Static;
    
    [[nodiscard]] constexpr bool isAlive(float currentTime) const noexcept {
        return currentTime < die;
    }
    
    [[nodiscard]] constexpr float remainingLife(float currentTime) const noexcept {
        return die - currentTime;
    }
};

/**
 * @brief QMB extended particle
 * 
 * Converted from: r_particles_qmb.h particle_t
 */
struct QMBParticle {
    math::Vec3 origin{};
    math::Vec3 endOrigin{};         ///< End position (for beams/trails)
    math::Vec3 velocity{};
    ParticleColor color{};
    
    float size = 1.0f;
    float growth = 0.0f;            ///< Size change per second
    float rotationAngle = 0.0f;
    float rotationSpeed = 0.0f;
    float startTime = 0.0f;
    float dieTime = 0.0f;
    
    uint8_t textureIndex = 0;
    uint8_t bounces = 0;
    uint8_t initialAlpha = 255;
    bool hit = false;               ///< Has hit surface
    
    // Caching for physics
    int cachedContents = 0;
    math::Vec3 cachedMovement{};
    float cachedDistance = 0.0f;
    
    // Entity reference for trails
    int entityRef = 0;
    int entityTrailIndex = 0;
    int entityTrailNumber = 0;
    
    [[nodiscard]] constexpr bool isAlive(float currentTime) const noexcept {
        return currentTime < dieTime;
    }
    
    [[nodiscard]] constexpr float age(float currentTime) const noexcept {
        return currentTime - startTime;
    }
    
    [[nodiscard]] constexpr float lifeProgress(float currentTime) const noexcept {
        float totalLife = dieTime - startTime;
        if (totalLife <= 0.0f) return 1.0f;
        return (currentTime - startTime) / totalLife;
    }
    
    [[nodiscard]] constexpr float currentSize(float currentTime) const noexcept {
        return size + growth * age(currentTime);
    }
    
    [[nodiscard]] constexpr float currentAlpha(float currentTime) const noexcept {
        // Alpha fades from initialAlpha to 0 over lifetime
        float progress = lifeProgress(currentTime);
        return static_cast<float>(initialAlpha) * (1.0f - progress);
    }
};

/**
 * @brief Particle type definition (rendering properties)
 * 
 * Converted from: r_particles_qmb.h particle_type_t
 */
struct ParticleTypeConfig {
    QMBParticleType id = QMBParticleType::Spark;
    ParticleDrawType drawType = ParticleDrawType::Spark;
    ParticleBlendMode blendMode = ParticleBlendMode::SrcAlpha_OneMinusSrcAlpha;
    ParticleTexture texture = ParticleTexture::None;
    ParticleMovement movement = ParticleMovement::Normal;
    
    float startAlpha = 1.0f;
    float gravity = 0.0f;           ///< Gravity multiplier
    float acceleration = 0.0f;      ///< Additional acceleration
    float custom = 0.0f;            ///< Type-specific parameter
    
    int vertsPerPrimitive = 1;
    int particleCount = 0;
    
    [[nodiscard]] constexpr bool hasGravity() const noexcept {
        return gravity != 0.0f;
    }
    
    [[nodiscard]] constexpr bool isAdditive() const noexcept {
        return ezquake::effects::isAdditive(blendMode);
    }
};

/**
 * @brief Particle texture coordinates
 * 
 * Converted from: r_particles_qmb.h particle_texture_t
 */
struct ParticleTextureCoords {
    int components = 1;             ///< Number of sub-textures
    std::array<std::array<float, 4>, effect_limits::MAX_PTEX_COMPONENTS> coords{};
    std::array<std::array<float, 4>, effect_limits::MAX_PTEX_COMPONENTS> originalCoords{};
    
    [[nodiscard]] constexpr const std::array<float, 4>& getCoords(int index) const noexcept {
        return coords[static_cast<size_t>(index % components)];
    }
    
    /**
     * @brief Get random texture component index
     */
    [[nodiscard]] int randomComponent() const noexcept;
};

/**
 * @brief Dynamic light source
 * 
 * Converted from: client.h dlight_t
 */
struct DynamicLight {
    int key = 0;                    ///< Entity key (for reuse)
    math::Vec3 origin{};
    float radius = 0.0f;
    float dieTime = 0.0f;           ///< Stop lighting after this time
    float decay = 0.0f;             ///< Radius reduction per second
    float minLight = 0.0f;          ///< Don't contribute below this
    DynamicLightType type = DynamicLightType::Default;
    ParticleColor customColor{255, 255, 255};
    bool noBubble = false;          ///< Disable flashblend bubble
    
    [[nodiscard]] constexpr bool isAlive(float currentTime) const noexcept {
        return currentTime < dieTime;
    }
    
    [[nodiscard]] constexpr float currentRadius(float currentTime, float startTime) const noexcept {
        float elapsed = currentTime - startTime;
        return std::max(0.0f, radius - decay * elapsed);
    }
    
    /**
     * @brief Get the actual light color
     */
    [[nodiscard]] constexpr std::array<uint8_t, 3> getColor() const noexcept {
        if (type == DynamicLightType::Custom) {
            return {customColor.r, customColor.g, customColor.b};
        }
        return dynamicLightColor(type);
    }
};

/**
 * @brief Custom light settings (for entities)
 * 
 * Converted from: client.h customlight_t
 */
struct CustomLight {
    DynamicLightType type = DynamicLightType::Default;
    ParticleColor color{255, 255, 255};
    
    [[nodiscard]] constexpr std::array<uint8_t, 3> getColor() const noexcept {
        if (type == DynamicLightType::Custom) {
            return {color.r, color.g, color.b};
        }
        return dynamicLightColor(type);
    }
};

/**
 * @brief Light style (flickering lights, etc.)
 * 
 * Converted from: client.h lightstyle_t
 */
struct LightStyle {
    int length = 0;
    std::array<char, effect_limits::MAX_STYLESTRING> pattern{};
    
    /**
     * @brief Get brightness at given time
     * @param time Current time in seconds
     * @return Brightness 0.0 to 1.0
     */
    [[nodiscard]] float getBrightness(float time) const noexcept {
        if (length == 0) return 1.0f;
        
        // Pattern characters: 'a' = no light, 'z' = double bright
        // 'm' = normal brightness
        int frame = static_cast<int>(time * 10.0f) % length;
        char c = pattern[static_cast<size_t>(frame)];
        
        // 'a' = 0, 'z' = 25, 'm' = 12
        return static_cast<float>(c - 'a') / 12.0f;
    }
    
    /**
     * @brief Set pattern from string
     */
    void setPattern(std::string_view pat) noexcept {
        length = static_cast<int>(std::min(pat.size(), pattern.size() - 1));
        for (int i = 0; i < length; ++i) {
            pattern[static_cast<size_t>(i)] = pat[static_cast<size_t>(i)];
        }
        pattern[static_cast<size_t>(length)] = '\0';
    }
};

/**
 * @brief Beam effect (lightning bolts, etc.)
 * 
 * Converted from: cl_tent.c beam_t
 */
struct Beam {
    int entityNum = 0;              ///< Entity that owns this beam
    math::Vec3 start{};
    math::Vec3 end{};
    float endTime = 0.0f;
    // Model handled by model system
    
    [[nodiscard]] constexpr bool isAlive(float currentTime) const noexcept {
        return currentTime < endTime;
    }
    
    [[nodiscard]] constexpr math::Vec3 direction() const noexcept {
        return end - start;
    }
    
    [[nodiscard]] float length() const noexcept {
        return direction().length();
    }
};

/**
 * @brief Explosion sprite effect
 * 
 * Converted from: cl_tent.c explosion_t
 */
struct Explosion {
    math::Vec3 origin{};
    float startTime = 0.0f;
    // Model handled by model system
    
    /**
     * @brief Get animation frame at current time
     * @param currentTime Current time
     * @param frameRate Frames per second
     * @param totalFrames Total animation frames
     * @return Current frame (0-based)
     */
    [[nodiscard]] constexpr int getFrame(float currentTime, float frameRate = 10.0f, int totalFrames = 6) const noexcept {
        float elapsed = currentTime - startTime;
        int frame = static_cast<int>(elapsed * frameRate);
        return std::min(frame, totalFrames - 1);
    }
    
    [[nodiscard]] constexpr bool isComplete(float currentTime, float frameRate = 10.0f, int totalFrames = 6) const noexcept {
        return getFrame(currentTime, frameRate, totalFrames) >= totalFrames - 1;
    }
};

//=============================================================================
// Effect System State
//=============================================================================

/**
 * @brief Overall effect system statistics
 */
struct EffectStats {
    int activeParticles = 0;
    int peakParticles = 0;
    int totalSpawned = 0;
    
    int activeLights = 0;
    int activeBeams = 0;
    int activeExplosions = 0;
    
    [[nodiscard]] constexpr float particleUsage(int maxParticles) const noexcept {
        if (maxParticles <= 0) return 0.0f;
        return static_cast<float>(activeParticles) / static_cast<float>(maxParticles);
    }
    
    void recordParticleSpawn(int count = 1) noexcept {
        activeParticles += count;
        totalSpawned += count;
        if (activeParticles > peakParticles) {
            peakParticles = activeParticles;
        }
    }
    
    void recordParticleDeath(int count = 1) noexcept {
        activeParticles = std::max(0, activeParticles - count);
    }
    
    void reset() noexcept {
        activeParticles = 0;
        // Keep peak and total for statistics
    }
};

/**
 * @brief Particle spawn parameters
 */
struct ParticleSpawnParams {
    math::Vec3 origin{};
    math::Vec3 direction{};
    ParticleColor color{};
    float size = 1.0f;
    float lifetime = 1.0f;
    int count = 1;
    float spread = 0.0f;            ///< Random spread angle
    float velocityScale = 1.0f;
    
    [[nodiscard]] constexpr ParticleSpawnParams& withOrigin(const math::Vec3& org) noexcept {
        origin = org;
        return *this;
    }
    
    [[nodiscard]] constexpr ParticleSpawnParams& withDirection(const math::Vec3& dir) noexcept {
        direction = dir;
        return *this;
    }
    
    [[nodiscard]] constexpr ParticleSpawnParams& withColor(const ParticleColor& col) noexcept {
        color = col;
        return *this;
    }
    
    [[nodiscard]] constexpr ParticleSpawnParams& withSize(float s) noexcept {
        size = s;
        return *this;
    }
    
    [[nodiscard]] constexpr ParticleSpawnParams& withLifetime(float t) noexcept {
        lifetime = t;
        return *this;
    }
    
    [[nodiscard]] constexpr ParticleSpawnParams& withCount(int c) noexcept {
        count = c;
        return *this;
    }
    
    [[nodiscard]] constexpr ParticleSpawnParams& withSpread(float s) noexcept {
        spread = s;
        return *this;
    }
};

/**
 * @brief Effect configuration options
 */
struct EffectConfig {
    int maxParticles = effect_limits::DEFAULT_PARTICLES;
    bool useQMBParticles = true;
    bool drawParticles = true;
    
    // Blood colors (Quake palette indices)
    int lgBloodColor = 225;         ///< Lightning gun blood
    int rlBloodColorSmall = 225;    ///< Rocket launcher blood (small)
    int rlBloodColorBig = 73;       ///< Rocket launcher blood (large)
    int sgBloodColor = 73;          ///< Shotgun blood
    
    // Particle effect toggles
    bool partBlood = true;
    bool partGunshots = true;
    bool partBloodTrails = true;
    bool partTrails = true;
    bool partSpikes = true;
    bool partExplosions = true;
    bool partBlobs = true;
    bool partLavaSplash = true;
    bool partTeleSplash = true;
    
    [[nodiscard]] constexpr bool isParticleEnabled(TrailType trail) const noexcept {
        if (isBloodTrail(trail)) return partBlood && partBloodTrails;
        return partTrails;
    }
};

} // namespace ezquake::effects
