/**
 * @file corona_types.hpp
 * @brief Modern C++20 corona and visual effect types for ezQuake
 * 
 * This module provides type-safe replacements for corona-related C definitions
 * from vx_stuff.h including corona types, corona texture identifiers,
 * camera modes, and related visual effect structures.
 * 
 * Original C types converted:
 * - coronatype_t -> CoronaType (enum class)
 * - corona_texture_id -> CoronaTextureId (enum class)
 * - corona_texture_t -> CoronaTexture (struct)
 * - cameramode_t -> CameraMode (enum class)
 * 
 * Features:
 * - Type-safe enum classes with explicit underlying types
 * - Full constexpr support for compile-time evaluation
 * - String conversion utilities
 * - Color and radius properties for corona types
 * - Texture array support for corona rendering
 */

#pragma once

#include "core/types.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string_view>

namespace ezquake {
namespace corona {

// ============================================================================
// Constants
// ============================================================================

/**
 * @brief Corona rendering constants
 */
namespace constants {
    /// Maximum number of simultaneous coronas
    inline constexpr std::size_t MAX_CORONAS = 1024;
    
    /// Default corona radius in world units
    inline constexpr float DEFAULT_RADIUS = 32.0f;
    
    /// Default corona alpha (0-1)
    inline constexpr float DEFAULT_ALPHA = 1.0f;
    
    /// Corona fade speed (per second)
    inline constexpr float FADE_SPEED = 2.0f;
    
    /// Minimum visible alpha threshold
    inline constexpr float MIN_VISIBLE_ALPHA = 0.01f;
    
    /// Corona texture array layer count
    inline constexpr int TEXTURE_ARRAY_LAYERS = 16;
    
    /// Animation frame time for explosion coronas
    inline constexpr float EXPLOSION_FRAME_TIME = 0.05f;
    
    /// Total explosion corona frames
    inline constexpr int EXPLOSION_FRAMES = 7;
} // namespace constants

// ============================================================================
// Corona Type Enumeration
// ============================================================================

/**
 * @brief Types of corona visual effects
 * 
 * Converted from C enum coronatype_t in vx_stuff.h.
 * Each type defines a specific visual behavior and color.
 */
enum class CoronaType : std::uint8_t {
    /// Explosion flash - fades out quickly
    Flash = 0,
    /// Small muzzle flash for weapons
    SmallFlash,
    /// Blue blob explosion effect
    BlueFlash,
    /// Unused/free slot
    Free,
    /// Lightning beam corona
    Lightning,
    /// Small lightning corona
    SmallLightning,
    /// Torch/fire corona
    Fire,
    /// Rocket glow effect
    RocketLight,
    /// Green light (detpack)
    GreenLight,
    /// Red light (detpack about to blow)
    RedLight,
    /// Blue spark (gibbed building)
    BlueSpark,
    /// Gunshot flash effect
    GunFlash,
    /// Animated explosion corona
    Explode,
    /// White light (gunshot #2)
    WhiteLight,
    /// Wizard enemy tracer light
    WizLight,
    /// Knight enemy tracer light
    KnightLight,
    /// Vore enemy tracer light
    VoreLight,
    
    /// Sentinel for iteration
    Count
};

/**
 * @brief Get string name of a corona type
 */
[[nodiscard]] constexpr std::string_view coronaTypeName(CoronaType type) noexcept {
    switch (type) {
        case CoronaType::Flash:          return "Flash";
        case CoronaType::SmallFlash:     return "SmallFlash";
        case CoronaType::BlueFlash:      return "BlueFlash";
        case CoronaType::Free:           return "Free";
        case CoronaType::Lightning:      return "Lightning";
        case CoronaType::SmallLightning: return "SmallLightning";
        case CoronaType::Fire:           return "Fire";
        case CoronaType::RocketLight:    return "RocketLight";
        case CoronaType::GreenLight:     return "GreenLight";
        case CoronaType::RedLight:       return "RedLight";
        case CoronaType::BlueSpark:      return "BlueSpark";
        case CoronaType::GunFlash:       return "GunFlash";
        case CoronaType::Explode:        return "Explode";
        case CoronaType::WhiteLight:     return "WhiteLight";
        case CoronaType::WizLight:       return "WizLight";
        case CoronaType::KnightLight:    return "KnightLight";
        case CoronaType::VoreLight:      return "VoreLight";
        case CoronaType::Count:          return "Count";
        default:                         return "Unknown";
    }
}

/**
 * @brief Check if corona type is a flash (fades quickly)
 */
[[nodiscard]] constexpr bool isFlashType(CoronaType type) noexcept {
    switch (type) {
        case CoronaType::Flash:
        case CoronaType::SmallFlash:
        case CoronaType::BlueFlash:
        case CoronaType::GunFlash:
            return true;
        default:
            return false;
    }
}

/**
 * @brief Check if corona type is a lightning effect
 */
[[nodiscard]] constexpr bool isLightningType(CoronaType type) noexcept {
    return type == CoronaType::Lightning || type == CoronaType::SmallLightning;
}

/**
 * @brief Check if corona type is an enemy tracer
 */
[[nodiscard]] constexpr bool isEnemyTracerType(CoronaType type) noexcept {
    switch (type) {
        case CoronaType::WizLight:
        case CoronaType::KnightLight:
        case CoronaType::VoreLight:
            return true;
        default:
            return false;
    }
}

/**
 * @brief Check if corona type uses animation
 */
[[nodiscard]] constexpr bool isAnimatedType(CoronaType type) noexcept {
    return type == CoronaType::Explode;
}

// ============================================================================
// Corona Texture Identifiers
// ============================================================================

/**
 * @brief Corona texture identifiers for rendering
 * 
 * Converted from C enum corona_texture_id in vx_stuff.h.
 * Used to select which texture to use for corona rendering.
 */
enum class CoronaTextureId : std::uint8_t {
    /// Standard corona texture
    Standard = 0,
    /// Gunflash corona texture
    GunFlash,
    /// Explosion frame 1
    ExplosionFlash1,
    /// Explosion frame 2
    ExplosionFlash2,
    /// Explosion frame 3
    ExplosionFlash3,
    /// Explosion frame 4
    ExplosionFlash4,
    /// Explosion frame 5
    ExplosionFlash5,
    /// Explosion frame 6
    ExplosionFlash6,
    /// Explosion frame 7
    ExplosionFlash7,
    
    /// Total number of corona textures
    Count
};

/**
 * @brief Get string name of a corona texture
 */
[[nodiscard]] constexpr std::string_view coronaTextureName(CoronaTextureId id) noexcept {
    switch (id) {
        case CoronaTextureId::Standard:         return "Standard";
        case CoronaTextureId::GunFlash:         return "GunFlash";
        case CoronaTextureId::ExplosionFlash1:  return "ExplosionFlash1";
        case CoronaTextureId::ExplosionFlash2:  return "ExplosionFlash2";
        case CoronaTextureId::ExplosionFlash3:  return "ExplosionFlash3";
        case CoronaTextureId::ExplosionFlash4:  return "ExplosionFlash4";
        case CoronaTextureId::ExplosionFlash5:  return "ExplosionFlash5";
        case CoronaTextureId::ExplosionFlash6:  return "ExplosionFlash6";
        case CoronaTextureId::ExplosionFlash7:  return "ExplosionFlash7";
        case CoronaTextureId::Count:            return "Count";
        default:                                return "Unknown";
    }
}

/**
 * @brief Check if texture ID is an explosion frame
 */
[[nodiscard]] constexpr bool isExplosionTexture(CoronaTextureId id) noexcept {
    return id >= CoronaTextureId::ExplosionFlash1 && 
           id <= CoronaTextureId::ExplosionFlash7;
}

/**
 * @brief Get explosion frame index (0-6) for explosion textures
 * @return Frame index or std::nullopt if not an explosion texture
 */
[[nodiscard]] constexpr std::optional<int> getExplosionFrame(CoronaTextureId id) noexcept {
    if (!isExplosionTexture(id)) {
        return std::nullopt;
    }
    return static_cast<int>(id) - static_cast<int>(CoronaTextureId::ExplosionFlash1);
}

/**
 * @brief Get corona texture ID for an explosion frame
 * @param frame Frame index (0-6), clamped to valid range
 */
[[nodiscard]] constexpr CoronaTextureId explosionFrameTexture(int frame) noexcept {
    if (frame < 0) frame = 0;
    if (frame >= constants::EXPLOSION_FRAMES) frame = constants::EXPLOSION_FRAMES - 1;
    return static_cast<CoronaTextureId>(
        static_cast<int>(CoronaTextureId::ExplosionFlash1) + frame
    );
}

// ============================================================================
// Corona Color
// ============================================================================

/**
 * @brief RGB color for corona rendering
 */
struct CoronaColor {
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    
    constexpr CoronaColor() noexcept = default;
    constexpr CoronaColor(float r_, float g_, float b_) noexcept
        : r(r_), g(g_), b(b_) {}
    
    /// Create from byte values (0-255)
    [[nodiscard]] static constexpr CoronaColor fromBytes(
        std::uint8_t r, std::uint8_t g, std::uint8_t b
    ) noexcept {
        return {
            static_cast<float>(r) / 255.0f,
            static_cast<float>(g) / 255.0f,
            static_cast<float>(b) / 255.0f
        };
    }
    
    /// Scale color by factor
    [[nodiscard]] constexpr CoronaColor scaled(float factor) const noexcept {
        return { r * factor, g * factor, b * factor };
    }
    
    /// Blend with another color
    [[nodiscard]] constexpr CoronaColor blend(const CoronaColor& other, float t) const noexcept {
        return {
            r + (other.r - r) * t,
            g + (other.g - g) * t,
            b + (other.b - b) * t
        };
    }
    
    /// Equality comparison
    [[nodiscard]] constexpr bool operator==(const CoronaColor& other) const noexcept {
        return r == other.r && g == other.g && b == other.b;
    }
    
    [[nodiscard]] constexpr bool operator!=(const CoronaColor& other) const noexcept {
        return !(*this == other);
    }
    
    // Common corona colors
    [[nodiscard]] static constexpr CoronaColor white() noexcept { return {1.0f, 1.0f, 1.0f}; }
    [[nodiscard]] static constexpr CoronaColor yellow() noexcept { return {1.0f, 0.9f, 0.3f}; }
    [[nodiscard]] static constexpr CoronaColor orange() noexcept { return {1.0f, 0.5f, 0.2f}; }
    [[nodiscard]] static constexpr CoronaColor red() noexcept { return {1.0f, 0.2f, 0.1f}; }
    [[nodiscard]] static constexpr CoronaColor green() noexcept { return {0.2f, 1.0f, 0.2f}; }
    [[nodiscard]] static constexpr CoronaColor blue() noexcept { return {0.3f, 0.5f, 1.0f}; }
    [[nodiscard]] static constexpr CoronaColor purple() noexcept { return {0.7f, 0.3f, 1.0f}; }
    [[nodiscard]] static constexpr CoronaColor fire() noexcept { return {1.0f, 0.6f, 0.2f}; }
};

/**
 * @brief Get the default color for a corona type
 */
[[nodiscard]] constexpr CoronaColor getCoronaColor(CoronaType type) noexcept {
    switch (type) {
        case CoronaType::Flash:          return CoronaColor::yellow();
        case CoronaType::SmallFlash:     return CoronaColor::yellow();
        case CoronaType::BlueFlash:      return CoronaColor::blue();
        case CoronaType::Free:           return CoronaColor::white();
        case CoronaType::Lightning:      return CoronaColor::blue();
        case CoronaType::SmallLightning: return CoronaColor::blue();
        case CoronaType::Fire:           return CoronaColor::fire();
        case CoronaType::RocketLight:    return CoronaColor::orange();
        case CoronaType::GreenLight:     return CoronaColor::green();
        case CoronaType::RedLight:       return CoronaColor::red();
        case CoronaType::BlueSpark:      return CoronaColor::blue();
        case CoronaType::GunFlash:       return CoronaColor::yellow();
        case CoronaType::Explode:        return CoronaColor::yellow();
        case CoronaType::WhiteLight:     return CoronaColor::white();
        case CoronaType::WizLight:       return CoronaColor::green();
        case CoronaType::KnightLight:    return CoronaColor::orange();
        case CoronaType::VoreLight:      return CoronaColor::purple();
        default:                         return CoronaColor::white();
    }
}

/**
 * @brief Get the default radius for a corona type
 */
[[nodiscard]] constexpr float getCoronaRadius(CoronaType type) noexcept {
    switch (type) {
        case CoronaType::Flash:          return 64.0f;
        case CoronaType::SmallFlash:     return 24.0f;
        case CoronaType::BlueFlash:      return 48.0f;
        case CoronaType::Free:           return 32.0f;
        case CoronaType::Lightning:      return 40.0f;
        case CoronaType::SmallLightning: return 20.0f;
        case CoronaType::Fire:           return 32.0f;
        case CoronaType::RocketLight:    return 48.0f;
        case CoronaType::GreenLight:     return 56.0f;
        case CoronaType::RedLight:       return 64.0f;
        case CoronaType::BlueSpark:      return 16.0f;
        case CoronaType::GunFlash:       return 20.0f;
        case CoronaType::Explode:        return 80.0f;
        case CoronaType::WhiteLight:     return 24.0f;
        case CoronaType::WizLight:       return 24.0f;
        case CoronaType::KnightLight:    return 24.0f;
        case CoronaType::VoreLight:      return 32.0f;
        default:                         return constants::DEFAULT_RADIUS;
    }
}

/**
 * @brief Get the default lifetime for a corona type (in seconds)
 */
[[nodiscard]] constexpr float getCoronaLifetime(CoronaType type) noexcept {
    switch (type) {
        case CoronaType::Flash:          return 0.4f;
        case CoronaType::SmallFlash:     return 0.1f;
        case CoronaType::BlueFlash:      return 0.3f;
        case CoronaType::Free:           return 0.0f;
        case CoronaType::Lightning:      return 0.05f;  // Per-frame
        case CoronaType::SmallLightning: return 0.05f;  // Per-frame
        case CoronaType::Fire:           return 0.0f;   // Continuous
        case CoronaType::RocketLight:    return 0.0f;   // Continuous
        case CoronaType::GreenLight:     return 0.0f;   // Continuous
        case CoronaType::RedLight:       return 0.0f;   // Continuous
        case CoronaType::BlueSpark:      return 0.5f;
        case CoronaType::GunFlash:       return 0.15f;
        case CoronaType::Explode:        return constants::EXPLOSION_FRAME_TIME * 
                                               constants::EXPLOSION_FRAMES;
        case CoronaType::WhiteLight:     return 0.2f;
        case CoronaType::WizLight:       return 0.1f;
        case CoronaType::KnightLight:    return 0.1f;
        case CoronaType::VoreLight:      return 0.1f;
        default:                         return 0.5f;
    }
}

// ============================================================================
// Corona Texture Reference
// ============================================================================

/**
 * @brief Placeholder for texture reference handle
 * 
 * In the full implementation, this would wrap the actual OpenGL/Vulkan texture handle.
 */
struct TextureHandle {
    std::uint32_t id = 0;
    
    constexpr TextureHandle() noexcept = default;
    constexpr explicit TextureHandle(std::uint32_t id_) noexcept : id(id_) {}
    
    [[nodiscard]] constexpr bool isValid() const noexcept { return id != 0; }
    
    [[nodiscard]] constexpr bool operator==(const TextureHandle& other) const noexcept {
        return id == other.id;
    }
    
    [[nodiscard]] constexpr bool operator!=(const TextureHandle& other) const noexcept {
        return id != other.id;
    }
};

/**
 * @brief Corona texture with array texture support
 * 
 * Converted from C struct corona_texture_t in vx_stuff.h.
 * Supports both individual textures and texture array layers.
 */
struct CoronaTexture {
    /// Individual texture reference
    TextureHandle texnum{};
    
    /// Texture array reference (for atlas rendering)
    TextureHandle arrayTex{};
    
    /// Index within texture array
    int arrayIndex = 0;
    
    /// Texture coordinate scaling for S axis
    float arrayScaleS = 1.0f;
    
    /// Texture coordinate scaling for T axis
    float arrayScaleT = 1.0f;
    
    constexpr CoronaTexture() noexcept = default;
    
    constexpr CoronaTexture(
        TextureHandle tex,
        TextureHandle array = {},
        int index = 0,
        float scaleS = 1.0f,
        float scaleT = 1.0f
    ) noexcept
        : texnum(tex)
        , arrayTex(array)
        , arrayIndex(index)
        , arrayScaleS(scaleS)
        , arrayScaleT(scaleT)
    {}
    
    /// Check if individual texture is valid
    [[nodiscard]] constexpr bool hasTexture() const noexcept {
        return texnum.isValid();
    }
    
    /// Check if array texture is valid
    [[nodiscard]] constexpr bool hasArrayTexture() const noexcept {
        return arrayTex.isValid();
    }
    
    /// Check if any texture is usable
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return hasTexture() || hasArrayTexture();
    }
    
    /// Get effective texture handle (prefers array texture)
    [[nodiscard]] constexpr TextureHandle getEffectiveTexture() const noexcept {
        return hasArrayTexture() ? arrayTex : texnum;
    }
    
    /// Reset to default state
    constexpr void clear() noexcept {
        texnum = TextureHandle{};
        arrayTex = TextureHandle{};
        arrayIndex = 0;
        arrayScaleS = 1.0f;
        arrayScaleT = 1.0f;
    }
};

// ============================================================================
// Camera Mode
// ============================================================================

/**
 * @brief Camera viewing modes
 * 
 * Converted from C enum cameramode_t in vx_stuff.h.
 * Determines how the camera follows or views the player.
 */
enum class CameraMode : std::uint8_t {
    /// Normal first-person view
    Normal = 0,
    /// Chase camera (third-person behind player)
    ChaseCam,
    /// External view (third-person, various angles)
    External
};

/**
 * @brief Get string name of camera mode
 */
[[nodiscard]] constexpr std::string_view cameraModeName(CameraMode mode) noexcept {
    switch (mode) {
        case CameraMode::Normal:   return "Normal";
        case CameraMode::ChaseCam: return "ChaseCam";
        case CameraMode::External: return "External";
        default:                   return "Unknown";
    }
}

/**
 * @brief Check if camera mode is third-person
 */
[[nodiscard]] constexpr bool isThirdPerson(CameraMode mode) noexcept {
    return mode != CameraMode::Normal;
}

/**
 * @brief Check if camera shows player model
 */
[[nodiscard]] constexpr bool shouldShowPlayerModel(CameraMode mode) noexcept {
    return isThirdPerson(mode);
}

// ============================================================================
// Corona Instance
// ============================================================================

/**
 * @brief 3D position in world space
 */
struct Position3D {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    
    constexpr Position3D() noexcept = default;
    constexpr Position3D(float x_, float y_, float z_) noexcept
        : x(x_), y(y_), z(z_) {}
    
    /// Distance squared to another position
    [[nodiscard]] constexpr float distanceSquared(const Position3D& other) const noexcept {
        float dx = x - other.x;
        float dy = y - other.y;
        float dz = z - other.z;
        return dx*dx + dy*dy + dz*dz;
    }
    
    /// Clear to origin
    constexpr void clear() noexcept { x = y = z = 0.0f; }
    
    [[nodiscard]] constexpr bool operator==(const Position3D& other) const noexcept {
        return x == other.x && y == other.y && z == other.z;
    }
    
    [[nodiscard]] constexpr bool operator!=(const Position3D& other) const noexcept {
        return !(*this == other);
    }
};

/**
 * @brief Active corona instance in the world
 */
struct Corona {
    /// World position
    Position3D origin{};
    
    /// Corona visual type
    CoronaType type = CoronaType::Free;
    
    /// Custom color (overrides type default if set)
    CoronaColor color = CoronaColor::white();
    
    /// Custom radius (0 = use type default)
    float radius = 0.0f;
    
    /// Current alpha/opacity (0-1)
    float alpha = 1.0f;
    
    /// Spawn time (for lifetime calculation)
    float spawnTime = 0.0f;
    
    /// Custom lifetime (0 = use type default)
    float lifetime = 0.0f;
    
    /// Associated entity ID (-1 = none)
    int entityId = -1;
    
    /// Current animation frame (for animated types)
    int frame = 0;
    
    /// Is this corona slot in use?
    bool active = false;
    
    constexpr Corona() noexcept = default;
    
    /// Create corona with position and type
    constexpr Corona(const Position3D& pos, CoronaType t) noexcept
        : origin(pos)
        , type(t)
        , color(getCoronaColor(t))
        , radius(getCoronaRadius(t))
        , alpha(1.0f)
        , lifetime(getCoronaLifetime(t))
        , active(true)
    {}
    
    /// Get effective radius (custom or type default)
    [[nodiscard]] constexpr float getRadius() const noexcept {
        return radius > 0.0f ? radius : getCoronaRadius(type);
    }
    
    /// Get effective lifetime (custom or type default)
    [[nodiscard]] constexpr float getLifetime() const noexcept {
        return lifetime > 0.0f ? lifetime : getCoronaLifetime(type);
    }
    
    /// Check if corona should still be visible
    [[nodiscard]] bool isAlive(float currentTime) const noexcept {
        if (!active) return false;
        float lt = getLifetime();
        if (lt <= 0.0f) return true;  // Continuous corona
        return (currentTime - spawnTime) < lt;
    }
    
    /// Calculate current alpha based on lifetime
    [[nodiscard]] float calculateAlpha(float currentTime) const noexcept {
        float lt = getLifetime();
        if (lt <= 0.0f) return alpha;  // No fade for continuous
        
        float elapsed = currentTime - spawnTime;
        if (elapsed >= lt) return 0.0f;
        
        // Linear fade
        float progress = elapsed / lt;
        return alpha * (1.0f - progress);
    }
    
    /// Calculate current animation frame (for Explode type)
    [[nodiscard]] int calculateFrame(float currentTime) const noexcept {
        if (!isAnimatedType(type)) return 0;
        
        float elapsed = currentTime - spawnTime;
        int frameNum = static_cast<int>(elapsed / constants::EXPLOSION_FRAME_TIME);
        
        if (frameNum >= constants::EXPLOSION_FRAMES) {
            frameNum = constants::EXPLOSION_FRAMES - 1;
        }
        return frameNum;
    }
    
    /// Get texture ID for current state
    [[nodiscard]] CoronaTextureId getTextureId(float currentTime) const noexcept {
        if (isAnimatedType(type)) {
            return explosionFrameTexture(calculateFrame(currentTime));
        }
        if (type == CoronaType::GunFlash) {
            return CoronaTextureId::GunFlash;
        }
        return CoronaTextureId::Standard;
    }
    
    /// Deactivate this corona
    constexpr void deactivate() noexcept {
        active = false;
        alpha = 0.0f;
    }
    
    /// Reset to default state
    constexpr void clear() noexcept {
        origin = {};
        type = CoronaType::Free;
        color = CoronaColor::white();
        radius = 0.0f;
        alpha = 1.0f;
        spawnTime = 0.0f;
        lifetime = 0.0f;
        entityId = -1;
        frame = 0;
        active = false;
    }
};

// ============================================================================
// Corona Texture Array
// ============================================================================

/**
 * @brief Container for all corona textures
 */
class CoronaTextureArray {
public:
    constexpr CoronaTextureArray() noexcept = default;
    
    /// Get texture by ID
    [[nodiscard]] constexpr CoronaTexture& get(CoronaTextureId id) noexcept {
        auto idx = static_cast<std::size_t>(id);
        if (idx >= textures_.size()) {
            return textures_[0];  // Return standard as fallback
        }
        return textures_[idx];
    }
    
    [[nodiscard]] constexpr const CoronaTexture& get(CoronaTextureId id) const noexcept {
        auto idx = static_cast<std::size_t>(id);
        if (idx >= textures_.size()) {
            return textures_[0];  // Return standard as fallback
        }
        return textures_[idx];
    }
    
    /// Set texture for an ID
    constexpr void set(CoronaTextureId id, const CoronaTexture& tex) noexcept {
        auto idx = static_cast<std::size_t>(id);
        if (idx < textures_.size()) {
            textures_[idx] = tex;
        }
    }
    
    /// Access by index
    [[nodiscard]] constexpr CoronaTexture& operator[](std::size_t idx) noexcept {
        return textures_[idx];
    }
    
    [[nodiscard]] constexpr const CoronaTexture& operator[](std::size_t idx) const noexcept {
        return textures_[idx];
    }
    
    /// Number of textures
    [[nodiscard]] static constexpr std::size_t size() noexcept {
        return static_cast<std::size_t>(CoronaTextureId::Count);
    }
    
    /// Clear all textures
    constexpr void clear() noexcept {
        for (auto& tex : textures_) {
            tex.clear();
        }
    }
    
    /// Check if all textures are loaded
    [[nodiscard]] constexpr bool allLoaded() const noexcept {
        for (std::size_t i = 0; i < static_cast<std::size_t>(CoronaTextureId::Count); ++i) {
            if (!textures_[i].isValid()) {
                return false;
            }
        }
        return true;
    }
    
    /// Count loaded textures
    [[nodiscard]] constexpr std::size_t countLoaded() const noexcept {
        std::size_t count = 0;
        for (std::size_t i = 0; i < static_cast<std::size_t>(CoronaTextureId::Count); ++i) {
            if (textures_[i].isValid()) {
                ++count;
            }
        }
        return count;
    }

private:
    std::array<CoronaTexture, static_cast<std::size_t>(CoronaTextureId::Count)> textures_{};
};

// ============================================================================
// Corona Pool Statistics
// ============================================================================

/**
 * @brief Statistics for corona rendering
 */
struct CoronaStats {
    /// Current number of active coronas
    std::size_t activeCount = 0;
    
    /// Peak number of active coronas this frame
    std::size_t peakCount = 0;
    
    /// Total coronas spawned
    std::size_t totalSpawned = 0;
    
    /// Total coronas expired
    std::size_t totalExpired = 0;
    
    /// Coronas culled (not visible)
    std::size_t culledCount = 0;
    
    constexpr CoronaStats() noexcept = default;
    
    /// Record a spawn
    constexpr void recordSpawn() noexcept {
        ++activeCount;
        ++totalSpawned;
        if (activeCount > peakCount) {
            peakCount = activeCount;
        }
    }
    
    /// Record expiration
    constexpr void recordExpire() noexcept {
        if (activeCount > 0) --activeCount;
        ++totalExpired;
    }
    
    /// Record culling
    constexpr void recordCull() noexcept {
        ++culledCount;
    }
    
    /// Reset frame-based stats
    constexpr void resetFrame() noexcept {
        culledCount = 0;
    }
    
    /// Full reset
    constexpr void reset() noexcept {
        activeCount = 0;
        peakCount = 0;
        totalSpawned = 0;
        totalExpired = 0;
        culledCount = 0;
    }
};

// ============================================================================
// Effect Configuration
// ============================================================================

/**
 * @brief Configuration for corona visual effects
 */
struct CoronaConfig {
    /// Enable corona rendering
    bool enabled = true;
    
    /// Enable teleporter coronas
    bool teleporterCoronas = true;
    
    /// Global corona brightness multiplier
    float brightness = 1.0f;
    
    /// Global corona size multiplier
    float sizeScale = 1.0f;
    
    /// Enable corona occlusion testing
    bool occlusionTest = true;
    
    /// Maximum view distance for coronas
    float maxDistance = 2048.0f;
    
    constexpr CoronaConfig() noexcept = default;
    
    /// Apply brightness to a color
    [[nodiscard]] constexpr CoronaColor applyBrightness(const CoronaColor& color) const noexcept {
        return color.scaled(brightness);
    }
    
    /// Apply size scale to a radius
    [[nodiscard]] constexpr float applyScale(float radius) const noexcept {
        return radius * sizeScale;
    }
    
    /// Reset to defaults
    constexpr void reset() noexcept {
        enabled = true;
        teleporterCoronas = true;
        brightness = 1.0f;
        sizeScale = 1.0f;
        occlusionTest = true;
        maxDistance = 2048.0f;
    }
};

// ============================================================================
// Weather Effects
// ============================================================================

/**
 * @brief Weather effect types
 */
enum class WeatherType : std::uint8_t {
    /// No weather effect
    None = 0,
    /// Rain effect
    Rain,
    /// Snow effect
    Snow,
    /// Sleet (rain/snow mix)
    Sleet
};

/**
 * @brief Get string name of weather type
 */
[[nodiscard]] constexpr std::string_view weatherTypeName(WeatherType type) noexcept {
    switch (type) {
        case WeatherType::None:  return "None";
        case WeatherType::Rain:  return "Rain";
        case WeatherType::Snow:  return "Snow";
        case WeatherType::Sleet: return "Sleet";
        default:                 return "Unknown";
    }
}

/**
 * @brief Configuration for weather effects
 */
struct WeatherConfig {
    /// Current weather type
    WeatherType type = WeatherType::None;
    
    /// Weather intensity (0-1)
    float intensity = 0.5f;
    
    /// Use fast rendering mode
    bool fastMode = false;
    
    constexpr WeatherConfig() noexcept = default;
    
    /// Check if weather is active
    [[nodiscard]] constexpr bool isActive() const noexcept {
        return type != WeatherType::None && intensity > 0.0f;
    }
    
    /// Reset to defaults
    constexpr void reset() noexcept {
        type = WeatherType::None;
        intensity = 0.5f;
        fastMode = false;
    }
};

// ============================================================================
// Lightning Effect Parameters
// ============================================================================

/**
 * @brief Configuration for lightning beam rendering
 */
struct LightningConfig {
    /// Lightning beam color (RGB)
    CoronaColor color = CoronaColor::blue();
    
    /// Beam thickness
    float size = 4.0f;
    
    /// Enable spark particles on impact
    bool sparks = true;
    
    /// Spark size multiplier
    float sparkSize = 1.0f;
    
    constexpr LightningConfig() noexcept = default;
    
    /// Reset to defaults
    constexpr void reset() noexcept {
        color = CoronaColor::blue();
        size = 4.0f;
        sparks = true;
        sparkSize = 1.0f;
    }
};

// ============================================================================
// Visual Effect State
// ============================================================================

/**
 * @brief Combined state for all visual effects
 */
struct VisualEffectState {
    /// Corona configuration
    CoronaConfig coronaConfig{};
    
    /// Corona statistics
    CoronaStats coronaStats{};
    
    /// Corona textures
    CoronaTextureArray coronaTextures{};
    
    /// Current camera mode
    CameraMode cameraMode = CameraMode::Normal;
    
    /// Weather configuration
    WeatherConfig weatherConfig{};
    
    /// Lightning configuration
    LightningConfig lightningConfig{};
    
    /// Particle count this frame
    int particleCount = 0;
    
    /// Peak particle count
    int particleCountHigh = 0;
    
    /// Corona count this frame
    int coronaCount = 0;
    
    /// Peak corona count
    int coronaCountHigh = 0;
    
    constexpr VisualEffectState() noexcept = default;
    
    /// Update particle tracking
    constexpr void updateParticleCount(int count) noexcept {
        particleCount = count;
        if (count > particleCountHigh) {
            particleCountHigh = count;
        }
    }
    
    /// Update corona tracking
    constexpr void updateCoronaCount(int count) noexcept {
        coronaCount = count;
        if (count > coronaCountHigh) {
            coronaCountHigh = count;
        }
    }
    
    /// Reset frame-based state
    constexpr void resetFrame() noexcept {
        coronaStats.resetFrame();
    }
    
    /// Full reset
    constexpr void reset() noexcept {
        coronaConfig.reset();
        coronaStats.reset();
        coronaTextures.clear();
        cameraMode = CameraMode::Normal;
        weatherConfig.reset();
        lightningConfig.reset();
        particleCount = 0;
        particleCountHigh = 0;
        coronaCount = 0;
        coronaCountHigh = 0;
    }
};

} // namespace corona
} // namespace ezquake
