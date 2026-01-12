/**
 * @file lightmap_types.hpp
 * @brief Lightmap and dynamic lighting type definitions
 *
 * Converted from:
 * - r_lightmaps.h, r_lightmaps_internal.h - Lightmap structures
 * - r_lighting.h - Lighting constants and tables
 * - client.h - dlight_t, lightstyle_t, dlighttype_t
 * - bspfile.h - MAXLIGHTMAPS
 * - vx_vertexlights.h - Vertex lighting types
 */

#pragma once

#include <array>
#include <cstdint>
#include <cstddef>
#include <cmath>
#include "core/math/vec3.hpp"

namespace ezquake::render {

// =============================================================================
// Lightmap Constants
// =============================================================================

namespace lightmap_limits {
    /// Maximum lightmaps per surface
    inline constexpr std::size_t MAX_LIGHTMAPS = 4;
    
    /// Lightmap texture dimensions
    inline constexpr std::size_t LIGHTMAP_WIDTH = 128;
    inline constexpr std::size_t LIGHTMAP_HEIGHT = 128;
    
    /// Maximum lightmap surface size
    inline constexpr std::size_t MAX_LIGHTMAP_SIZE = 64 * 64;
    
    /// Lightmap array growth factor
    inline constexpr std::size_t ARRAY_GROWTH = 4;
    
    /// Maximum light styles
    inline constexpr std::size_t MAX_LIGHTSTYLES = 64;
    
    /// Maximum style string length
    inline constexpr std::size_t MAX_STYLESTRING = 64;
    
    /// Maximum dynamic lights
    inline constexpr std::size_t MAX_DLIGHTS = 32;
    
    /// Bits per dlight active word
    inline constexpr std::size_t DLIGHT_ACTIVE_BITS = 32;
    
    /// Number of dlight active words
    inline constexpr std::size_t DLIGHT_ACTIVE_WORDS = MAX_DLIGHTS / DLIGHT_ACTIVE_BITS;
    
    /// Bytes per lightmap pixel (RGBA)
    inline constexpr std::size_t BYTES_PER_PIXEL = 4;
    
    /// Total bytes per lightmap
    inline constexpr std::size_t LIGHTMAP_BYTES = 
        BYTES_PER_PIXEL * LIGHTMAP_WIDTH * LIGHTMAP_HEIGHT;
}

// =============================================================================
// Dynamic Light Type
// =============================================================================

/**
 * @brief Dynamic light color/type classification
 *
 * From client.h: dlighttype_t
 * Used to classify dynamic lights by their color/source
 */
enum class DLightColorType : uint8_t {
    Default = 0,        // Default yellow/white
    MuzzleFlash,        // Weapon muzzle flash
    Explosion,          // Explosion flash
    Rocket,             // Rocket glow
    Red,                // Pure red
    Blue,               // Pure blue
    RedBlue,            // Alternating red/blue
    Green,              // Pure green
    RedGreen,           // Alternating red/green
    BlueGreen,          // Alternating blue/green
    White,              // Pure white
    Custom,             // Custom color (use color field)
    
    Count               // Number of types
};

/**
 * @brief Get display name for dynamic light type
 */
[[nodiscard]] constexpr const char* getDLightColorTypeName(DLightColorType type) noexcept {
    switch (type) {
        case DLightColorType::Default:     return "Default";
        case DLightColorType::MuzzleFlash: return "MuzzleFlash";
        case DLightColorType::Explosion:   return "Explosion";
        case DLightColorType::Rocket:      return "Rocket";
        case DLightColorType::Red:         return "Red";
        case DLightColorType::Blue:        return "Blue";
        case DLightColorType::RedBlue:     return "RedBlue";
        case DLightColorType::Green:       return "Green";
        case DLightColorType::RedGreen:    return "RedGreen";
        case DLightColorType::BlueGreen:   return "BlueGreen";
        case DLightColorType::White:       return "White";
        case DLightColorType::Custom:      return "Custom";
        default:                           return "Unknown";
    }
}

// =============================================================================
// Light Color
// =============================================================================

/**
 * @brief RGB color for lighting
 */
struct LightColor {
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    
    constexpr LightColor() noexcept = default;
    constexpr LightColor(uint8_t r_, uint8_t g_, uint8_t b_) noexcept
        : r(r_), g(g_), b(b_) {}
    
    /// Create from float components (0-1 range)
    [[nodiscard]] static constexpr LightColor fromFloat(float r_, float g_, float b_) noexcept {
        return LightColor{
            static_cast<uint8_t>(r_ * 255.0f),
            static_cast<uint8_t>(g_ * 255.0f),
            static_cast<uint8_t>(b_ * 255.0f)
        };
    }
    
    /// Get as float array (0-1 range)
    [[nodiscard]] constexpr std::array<float, 3> toFloat() const noexcept {
        return {{ r / 255.0f, g / 255.0f, b / 255.0f }};
    }
    
    /// Get intensity (grayscale approximation)
    [[nodiscard]] constexpr float intensity() const noexcept {
        return (0.299f * r + 0.587f * g + 0.114f * b) / 255.0f;
    }
    
    /// Standard colors
    [[nodiscard]] static constexpr LightColor white() noexcept { return {255, 255, 255}; }
    [[nodiscard]] static constexpr LightColor red() noexcept { return {255, 0, 0}; }
    [[nodiscard]] static constexpr LightColor green() noexcept { return {0, 255, 0}; }
    [[nodiscard]] static constexpr LightColor blue() noexcept { return {0, 0, 255}; }
    [[nodiscard]] static constexpr LightColor yellow() noexcept { return {255, 255, 0}; }
    [[nodiscard]] static constexpr LightColor orange() noexcept { return {255, 128, 0}; }
    
    constexpr bool operator==(const LightColor& other) const noexcept = default;
};

// =============================================================================
// Dynamic Light
// =============================================================================

/**
 * @brief Dynamic light source (point light)
 *
 * From client.h: dlight_t
 */
struct DynamicLight {
    using Vec3 = math::Vec3;
    
    int key = 0;                        // Entity key for light reuse
    Vec3 origin{};                      // World position
    float radius = 0.0f;                // Light radius
    float die = 0.0f;                   // Time to stop lighting
    float decay = 0.0f;                 // Decay rate per second
    float minLight = 0.0f;              // Minimum contribution threshold
    int bubble = 0;                     // Non-zero = no flashblend bubble
    DLightColorType type = DLightColorType::Default;
    LightColor color{};                 // Custom color (when type == Custom)
    
    /// Check if light is still active
    [[nodiscard]] constexpr bool isActive(float time) const noexcept {
        return time < die;
    }
    
    /// Calculate current radius accounting for decay
    [[nodiscard]] constexpr float currentRadius(float time, float startTime) const noexcept {
        float elapsed = time - startTime;
        return radius - (decay * elapsed);
    }
    
    /// Check if light contributes at given distance
    [[nodiscard]] constexpr bool contributes(float distance) const noexcept {
        return distance < radius && (radius - distance) > minLight;
    }
    
    /// Reset light state
    constexpr void reset() noexcept {
        key = 0;
        origin = Vec3{};
        radius = 0.0f;
        die = 0.0f;
        decay = 0.0f;
        minLight = 0.0f;
        bubble = 0;
        type = DLightColorType::Default;
        color = LightColor{};
    }
};

/**
 * @brief Custom light settings (for entity effects)
 */
struct CustomLight {
    DLightColorType type = DLightColorType::Default;
    LightColor color{};
    uint8_t alpha = 255;
    
    [[nodiscard]] constexpr bool isCustom() const noexcept {
        return type == DLightColorType::Custom;
    }
};

// =============================================================================
// Light Styles
// =============================================================================

/**
 * @brief Animated light style definition
 *
 * From client.h: lightstyle_t
 * Light styles are animated sequences where each character represents
 * a brightness level ('a' = dark, 'm' = normal, 'z' = bright)
 */
struct LightStyle {
    static constexpr std::size_t MAX_LENGTH = lightmap_limits::MAX_STYLESTRING;
    
    int length = 0;
    std::array<char, MAX_LENGTH> map{};
    
    /// Get brightness at given frame (0-255 range)
    [[nodiscard]] constexpr int getBrightness(int frame) const noexcept {
        if (length == 0) return 256; // Full bright
        char c = map[static_cast<std::size_t>(frame % length)];
        return (c - 'a') * 22; // 'a' = 0, 'z' = 550, but scaled
    }
    
    /// Get normalized brightness (0.0 - 1.0+)
    [[nodiscard]] constexpr float getNormalizedBrightness(int frame) const noexcept {
        return static_cast<float>(getBrightness(frame)) / 256.0f;
    }
    
    /// Check if style is constant (single value)
    [[nodiscard]] constexpr bool isConstant() const noexcept {
        return length <= 1;
    }
    
    /// Set from string
    void setPattern(const char* pattern) noexcept {
        length = 0;
        while (pattern && pattern[length] != '\0' && 
               static_cast<std::size_t>(length) < MAX_LENGTH - 1) {
            map[static_cast<std::size_t>(length)] = pattern[length];
            ++length;
        }
        if (static_cast<std::size_t>(length) < MAX_LENGTH) {
            map[static_cast<std::size_t>(length)] = '\0';
        }
    }
};

// =============================================================================
// Lightmap Rectangle
// =============================================================================

/**
 * @brief Rectangle for tracking dirty lightmap regions
 */
struct LightmapRect {
    uint32_t left = 0;
    uint32_t top = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    
    [[nodiscard]] constexpr bool isEmpty() const noexcept {
        return width == 0 || height == 0;
    }
    
    [[nodiscard]] constexpr uint32_t right() const noexcept {
        return left + width;
    }
    
    [[nodiscard]] constexpr uint32_t bottom() const noexcept {
        return top + height;
    }
    
    /// Expand rect to include another rect
    constexpr void expand(const LightmapRect& other) noexcept {
        if (other.isEmpty()) return;
        if (isEmpty()) {
            *this = other;
            return;
        }
        uint32_t newLeft = left < other.left ? left : other.left;
        uint32_t newTop = top < other.top ? top : other.top;
        uint32_t newRight = right() > other.right() ? right() : other.right();
        uint32_t newBottom = bottom() > other.bottom() ? bottom() : other.bottom();
        left = newLeft;
        top = newTop;
        width = newRight - newLeft;
        height = newBottom - newTop;
    }
    
    /// Reset to empty
    constexpr void reset() noexcept {
        left = top = width = height = 0;
    }
};

// =============================================================================
// Lightmap Data
// =============================================================================

/**
 * @brief Per-lightmap texture data
 *
 * From r_lightmaps_internal.h: lightmap_data_t
 */
struct LightmapData {
    static constexpr std::size_t WIDTH = lightmap_limits::LIGHTMAP_WIDTH;
    static constexpr std::size_t HEIGHT = lightmap_limits::LIGHTMAP_HEIGHT;
    static constexpr std::size_t PIXEL_COUNT = WIDTH * HEIGHT;
    static constexpr std::size_t BYTES = lightmap_limits::LIGHTMAP_BYTES;
    
    // Raw pixel data (RGBA)
    std::array<uint8_t, BYTES> rawData{};
    
    // Computed lighting data (for hardware lighting)
    std::array<int32_t, BYTES> computeData{};
    
    // Source lighting data
    std::array<uint32_t, BYTES> sourceData{};
    
    // Allocation tracking (columns)
    std::array<int32_t, WIDTH> allocated{};
    
    // GPU texture reference
    uint32_t textureRef = 0;
    
    // Dirty region tracking
    LightmapRect changeArea{};
    bool modified = false;
    
    /// Reset allocation state
    void resetAllocation() noexcept {
        allocated.fill(0);
    }
    
    /// Mark entire lightmap as modified
    void markFullyModified() noexcept {
        modified = true;
        changeArea.left = 0;
        changeArea.top = 0;
        changeArea.width = static_cast<uint32_t>(WIDTH);
        changeArea.height = static_cast<uint32_t>(HEIGHT);
    }
    
    /// Clear modification state
    void clearModified() noexcept {
        modified = false;
        changeArea.reset();
    }
};

// =============================================================================
// RGB9E5 Shared Exponent Format
// =============================================================================

/**
 * @brief RGB9E5 shared exponent format decoder table
 *
 * From r_lighting.h: rgb9e5tab
 * Used for HDR lightmap data
 */
inline constexpr std::array<float, 32> RGB9E5_TABLE = {{
    // Multipliers for 9-bit mantissa based on biased exponent
    // pow(2, biasedexponent - bias - bits) where bias=15, bits=9
    1.0f/(1<<24), 1.0f/(1<<23), 1.0f/(1<<22), 1.0f/(1<<21),
    1.0f/(1<<20), 1.0f/(1<<19), 1.0f/(1<<18), 1.0f/(1<<17),
    1.0f/(1<<16), 1.0f/(1<<15), 1.0f/(1<<14), 1.0f/(1<<13),
    1.0f/(1<<12), 1.0f/(1<<11), 1.0f/(1<<10), 1.0f/(1<<9),
    1.0f/(1<<8),  1.0f/(1<<7),  1.0f/(1<<6),  1.0f/(1<<5),
    1.0f/(1<<4),  1.0f/(1<<3),  1.0f/(1<<2),  1.0f/(1<<1),
    1.0f,         1.0f*(1<<1),  1.0f*(1<<2),  1.0f*(1<<3),
    1.0f*(1<<4),  1.0f*(1<<5),  1.0f*(1<<6),  1.0f*(1<<7),
}};

/**
 * @brief Decode RGB9E5 packed value to RGB floats
 */
[[nodiscard]] inline std::array<float, 3> decodeRGB9E5(uint32_t packed) noexcept {
    // Extract shared exponent (5 bits)
    uint32_t exp = (packed >> 27) & 0x1F;
    float multiplier = RGB9E5_TABLE[exp];
    
    // Extract mantissas (9 bits each)
    float r = static_cast<float>((packed >> 0) & 0x1FF) * multiplier;
    float g = static_cast<float>((packed >> 9) & 0x1FF) * multiplier;
    float b = static_cast<float>((packed >> 18) & 0x1FF) * multiplier;
    
    return {{ r, g, b }};
}

// =============================================================================
// Surface Lighting Info
// =============================================================================

/**
 * @brief Surface lighting information
 *
 * Based on msurface_s lighting fields
 */
struct SurfaceLightInfo {
    static constexpr std::size_t MAX_STYLES = lightmap_limits::MAX_LIGHTMAPS;
    
    // Light style indices (255 = unused)
    std::array<uint8_t, MAX_STYLES> styles{{ 255, 255, 255, 255 }};
    
    // Cached light values for dirty detection
    std::array<int32_t, MAX_STYLES> cachedLight{{ 0, 0, 0, 0 }};
    
    // Lightmap texture coordinates
    int lightmapTextureNum = -1;
    int lightS = 0; // S offset into lightmap
    int lightT = 0; // T offset into lightmap
    
    /// Count active light styles
    [[nodiscard]] constexpr std::size_t activeStyles() const noexcept {
        std::size_t count = 0;
        for (std::size_t i = 0; i < MAX_STYLES; ++i) {
            if (styles[i] != 255) ++count;
            else break;
        }
        return count;
    }
    
    /// Check if surface needs lightmap update
    [[nodiscard]] bool needsUpdate(const uint32_t* styleValues) const noexcept {
        for (std::size_t i = 0; i < MAX_STYLES && styles[i] != 255; ++i) {
            if (cachedLight[i] != static_cast<int32_t>(styleValues[styles[i]])) {
                return true;
            }
        }
        return false;
    }
};

// =============================================================================
// Vertex Lighting
// =============================================================================

/**
 * @brief Vertex lighting mode
 */
enum class VertexLightMode : uint8_t {
    None = 0,           // No vertex lighting
    Static,             // Static vertex colors
    Dynamic,            // Dynamic vertex lighting
    Combined            // Static + dynamic
};

/**
 * @brief Vertex light data
 */
struct VertexLight {
    LightColor color{};
    float intensity = 1.0f;
    
    /// Apply intensity to get final color
    [[nodiscard]] constexpr std::array<float, 3> getFinalColor() const noexcept {
        return {{
            (color.r / 255.0f) * intensity,
            (color.g / 255.0f) * intensity,
            (color.b / 255.0f) * intensity
        }};
    }
};

// =============================================================================
// Lighting Mode
// =============================================================================

/**
 * @brief Dynamic lighting computation mode
 */
enum class DynamicLightMode : uint8_t {
    Off = 0,            // r_dynamic 0 - no dynamic lights
    Software = 1,       // r_dynamic 1 - software lightmap updates
    Hardware = 2        // r_dynamic 2 - hardware (shader) lighting
};

/**
 * @brief Get if dynamic lighting is enabled
 */
[[nodiscard]] constexpr bool isDynamicLightingEnabled(DynamicLightMode mode) noexcept {
    return mode != DynamicLightMode::Off;
}

/**
 * @brief Check if using software lightmap updates
 */
[[nodiscard]] constexpr bool isSoftwareLighting(DynamicLightMode mode) noexcept {
    return mode == DynamicLightMode::Software;
}

/**
 * @brief Check if using hardware (shader) lighting
 */
[[nodiscard]] constexpr bool isHardwareLighting(DynamicLightMode mode) noexcept {
    return mode == DynamicLightMode::Hardware;
}

// =============================================================================
// Light Entity Data
// =============================================================================

/**
 * @brief Light entity spawn parameters
 *
 * For map entity lights (light, light_fluoro, etc.)
 */
struct LightEntityParams {
    math::Vec3 origin{};
    float light = 300.0f;   // Light intensity
    int style = 0;          // Light style index
    float angle = 0.0f;     // Spotlight angle (0 = point light)
    LightColor color{};     // Light color
    
    /// Check if this is a spotlight
    [[nodiscard]] constexpr bool isSpotlight() const noexcept {
        return angle > 0.0f;
    }
};

// =============================================================================
// Dynamic Light Array
// =============================================================================

/**
 * @brief Container for dynamic light pool
 */
struct DynamicLightPool {
    static constexpr std::size_t MAX_LIGHTS = lightmap_limits::MAX_DLIGHTS;
    static constexpr std::size_t ACTIVE_WORDS = lightmap_limits::DLIGHT_ACTIVE_WORDS;
    
    std::array<DynamicLight, MAX_LIGHTS> lights{};
    std::array<uint32_t, ACTIVE_WORDS> activeBits{};
    
    /// Check if a light slot is active
    [[nodiscard]] constexpr bool isActive(std::size_t index) const noexcept {
        if (index >= MAX_LIGHTS) return false;
        std::size_t word = index / 32;
        std::size_t bit = index % 32;
        return (activeBits[word] & (1u << bit)) != 0;
    }
    
    /// Set light slot active state
    constexpr void setActive(std::size_t index, bool active) noexcept {
        if (index >= MAX_LIGHTS) return;
        std::size_t word = index / 32;
        std::size_t bit = index % 32;
        if (active) {
            activeBits[word] |= (1u << bit);
        } else {
            activeBits[word] &= ~(1u << bit);
        }
    }
    
    /// Find free light slot
    [[nodiscard]] constexpr int findFreeSlot() const noexcept {
        for (std::size_t i = 0; i < MAX_LIGHTS; ++i) {
            if (!isActive(i)) return static_cast<int>(i);
        }
        return -1;
    }
    
    /// Find light by key (for entity light reuse)
    [[nodiscard]] constexpr int findByKey(int key) const noexcept {
        if (key == 0) return -1;
        for (std::size_t i = 0; i < MAX_LIGHTS; ++i) {
            if (isActive(i) && lights[i].key == key) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }
    
    /// Clear all lights
    constexpr void clear() noexcept {
        activeBits.fill(0);
        for (auto& light : lights) {
            light.reset();
        }
    }
    
    /// Count active lights
    [[nodiscard]] constexpr std::size_t countActive() const noexcept {
        std::size_t count = 0;
        for (std::size_t i = 0; i < MAX_LIGHTS; ++i) {
            if (isActive(i)) ++count;
        }
        return count;
    }
};

// =============================================================================
// Light Style Array
// =============================================================================

/**
 * @brief Container for light style pool
 */
struct LightStylePool {
    static constexpr std::size_t MAX_STYLES = lightmap_limits::MAX_LIGHTSTYLES;
    
    std::array<LightStyle, MAX_STYLES> styles{};
    std::array<uint32_t, 256> values{}; // 8.8 fixed point brightness values
    
    /// Get style by index
    [[nodiscard]] constexpr const LightStyle& getStyle(std::size_t index) const noexcept {
        return styles[index < MAX_STYLES ? index : 0];
    }
    
    /// Get current brightness value (8.8 fixed point)
    [[nodiscard]] constexpr uint32_t getValue(std::size_t index) const noexcept {
        return values[index < 256 ? index : 0];
    }
    
    /// Update brightness value
    constexpr void setValue(std::size_t index, uint32_t value) noexcept {
        if (index < 256) {
            values[index] = value;
        }
    }
    
    /// Clear all styles
    void clear() noexcept {
        for (auto& style : styles) {
            style.length = 0;
        }
        values.fill(256); // Default full bright
    }
};

} // namespace ezquake::render
