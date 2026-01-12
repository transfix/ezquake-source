/**
 * @file texture_types.hpp
 * @brief Texture type definitions for ezQuake C++20 port
 * 
 * This file contains texture-related types and constants extracted from:
 * - r_texture.h - Texture flags, loading functions
 * - r_texture_internal.h - Internal texture structures
 * - r_texture_cvars.c - Texture filtering modes
 * - wad.h - WAD file types
 * - quakedef.h - Texture type enums
 * - gl_model.h - texture_t structure
 * 
 * Part of ezQuake C to C++20 Conversion - Iteration 22
 */

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <array>
#include <optional>
#include "core/math/vec3.hpp"

namespace ezquake::render {

// =============================================================================
// Texture Type Enums (from quakedef.h)
// =============================================================================

/**
 * @brief Types of textures supported by the renderer
 */
enum class TextureType : uint8_t {
    Tex2D = 0,              // Standard 2D texture
    Tex2DArray = 1,         // 2D texture array
    CubeMap = 2,            // Cubemap texture
    Tex2DMultisampled = 3,  // Multisampled 2D texture
    
    Count = 4
};

/**
 * @brief Texture minification filter modes
 */
enum class MinFilter : uint8_t {
    Nearest = 0,                // GL_NEAREST
    Linear = 1,                 // GL_LINEAR
    NearestMipmapNearest = 2,   // GL_NEAREST_MIPMAP_NEAREST
    LinearMipmapNearest = 3,    // GL_LINEAR_MIPMAP_NEAREST
    NearestMipmapLinear = 4,    // GL_NEAREST_MIPMAP_LINEAR
    LinearMipmapLinear = 5,     // GL_LINEAR_MIPMAP_LINEAR (trilinear)
    
    Count = 6
};

/**
 * @brief Texture magnification filter modes
 */
enum class MagFilter : uint8_t {
    Nearest = 0,    // GL_NEAREST (pixelated)
    Linear = 1,     // GL_LINEAR (smooth)
    
    Count = 2
};

// =============================================================================
// Texture Flags (from r_texture.h)
// =============================================================================

/**
 * @brief Flags for texture loading and behavior
 */
namespace TextureFlags {
    constexpr uint32_t None = 0;
    
    // Loading behavior
    constexpr uint32_t Complain     = (1 << 0);   // Log error if texture missing
    constexpr uint32_t Mipmap       = (1 << 1);   // Generate mipmaps
    constexpr uint32_t Alpha        = (1 << 2);   // Has alpha channel
    constexpr uint32_t Luma         = (1 << 3);   // No gamma adjustment
    constexpr uint32_t Fullbright   = (1 << 4);   // Non-fullbright colors transparent
    constexpr uint32_t NoScale      = (1 << 5);   // Don't apply gl_max_size scaling
    constexpr uint32_t Brighten     = (1 << 6);   // Brighter 8->24 bit conversion
    constexpr uint32_t NoCompress   = (1 << 7);   // No texture compression
    constexpr uint32_t NoPCX        = (1 << 8);   // Don't load PCX images
    constexpr uint32_t NoTextureMode = (1 << 9);  // Ignore gl_texturemode changes
    constexpr uint32_t PremulAlpha  = (1 << 10);  // Pre-multiply alpha
    constexpr uint32_t ZeroAlpha    = (1 << 11);  // Set alpha to 0 after premul
    constexpr uint32_t MergedLuma   = (1 << 12);  // Alpha channel = luma %
    constexpr uint32_t Float        = (1 << 13);  // Floating point format
    constexpr uint32_t Viewmodel    = (1 << 14);  // Use viewmodel texture mode
    
    /** @brief Check if texture has mipmaps */
    [[nodiscard]] constexpr bool hasMipmaps(uint32_t flags) noexcept {
        return (flags & Mipmap) != 0;
    }
    
    /** @brief Check if texture has alpha */
    [[nodiscard]] constexpr bool hasAlpha(uint32_t flags) noexcept {
        return (flags & Alpha) != 0;
    }
    
    /** @brief Check if texture is floating point */
    [[nodiscard]] constexpr bool isFloat(uint32_t flags) noexcept {
        return (flags & Float) != 0;
    }
    
    /** @brief Check if texture should be scaled */
    [[nodiscard]] constexpr bool shouldScale(uint32_t flags) noexcept {
        return (flags & NoScale) == 0;
    }
    
    /** @brief Check if this is a luma/fullbright texture */
    [[nodiscard]] constexpr bool isLuma(uint32_t flags) noexcept {
        return (flags & (Luma | Fullbright)) == (Luma | Fullbright);
    }
}

// =============================================================================
// Texture Limits
// =============================================================================

namespace texture_limits {
    constexpr int MAX_TEXTURES = 8192;      // Maximum GL textures
    constexpr int MAX_CHARSETS = 256;       // Maximum character sets
    constexpr int MAX_USER_CHARSETS = 0xE0; // User-loadable charsets
    constexpr int MIP_LEVELS = 4;           // Mipmap levels per texture
    constexpr int MAX_TEXTURE_SIZE = 16384; // Maximum dimension
    constexpr int MIN_TEXTURE_SIZE = 1;     // Minimum dimension
}

// =============================================================================
// WAD Types (from wad.h)
// =============================================================================

/**
 * @brief WAD lump compression types
 */
enum class WadCompression : uint8_t {
    None = 0,
    LZSS = 1
};

/**
 * @brief WAD lump types
 */
enum class WadLumpType : uint8_t {
    None = 0,
    Label = 1,
    Lumpy = 64,     // 64 + grab command number
    Palette = 64,
    QTex = 65,
    QPic = 66,
    Sound = 67,
    Miptex = 68
};

/**
 * @brief Get WAD lump type name
 */
[[nodiscard]] inline std::string_view wadLumpTypeName(WadLumpType type) noexcept {
    switch (type) {
        case WadLumpType::None: return "none";
        case WadLumpType::Label: return "label";
        case WadLumpType::Palette: return "palette";
        case WadLumpType::QTex: return "qtex";
        case WadLumpType::QPic: return "qpic";
        case WadLumpType::Sound: return "sound";
        case WadLumpType::Miptex: return "miptex";
        default: return "unknown";
    }
}

// =============================================================================
// Texture Reference
// =============================================================================

/**
 * @brief Handle to a texture in the texture system
 * 
 * This is a lightweight handle that can be copied and passed by value.
 * A null/invalid reference has index 0.
 */
struct TextureRef {
    uint32_t index = 0;
    
    [[nodiscard]] constexpr bool isValid() const noexcept { return index != 0; }
    [[nodiscard]] constexpr bool isNull() const noexcept { return index == 0; }
    
    constexpr void invalidate() noexcept { index = 0; }
    
    [[nodiscard]] constexpr bool operator==(const TextureRef& other) const noexcept = default;
    [[nodiscard]] constexpr bool operator!=(const TextureRef& other) const noexcept = default;
    
    // Allow explicit conversion to uint32_t for OpenGL interop
    [[nodiscard]] explicit constexpr operator uint32_t() const noexcept { return index; }
};

constexpr TextureRef NULL_TEXTURE = { 0 };

// =============================================================================
// Filter Mode Info
// =============================================================================

/**
 * @brief Named filter mode for texture mode cvars
 */
struct FilterMode {
    std::string_view name;
    MinFilter minFilter;
    MagFilter magFilter;
    
    [[nodiscard]] bool usesMipmaps() const noexcept {
        return static_cast<uint8_t>(minFilter) >= static_cast<uint8_t>(MinFilter::NearestMipmapNearest);
    }
};

/**
 * @brief Standard OpenGL filter mode names
 */
inline constexpr std::array<FilterMode, 6> FILTER_MODES = {{
    { "GL_NEAREST", MinFilter::Nearest, MagFilter::Nearest },
    { "GL_LINEAR", MinFilter::Linear, MagFilter::Linear },
    { "GL_NEAREST_MIPMAP_NEAREST", MinFilter::NearestMipmapNearest, MagFilter::Nearest },
    { "GL_LINEAR_MIPMAP_NEAREST", MinFilter::LinearMipmapNearest, MagFilter::Linear },
    { "GL_NEAREST_MIPMAP_LINEAR", MinFilter::NearestMipmapLinear, MagFilter::Nearest },
    { "GL_LINEAR_MIPMAP_LINEAR", MinFilter::LinearMipmapLinear, MagFilter::Linear }
}};

/**
 * @brief Find filter mode by name
 */
[[nodiscard]] inline std::optional<FilterMode> findFilterMode(std::string_view name) noexcept {
    for (const auto& mode : FILTER_MODES) {
        if (mode.name == name) {
            return mode;
        }
    }
    return std::nullopt;
}

/**
 * @brief Get filter mode name
 */
[[nodiscard]] inline std::string_view minFilterName(MinFilter filter) noexcept {
    auto idx = static_cast<size_t>(filter);
    if (idx < FILTER_MODES.size()) {
        return FILTER_MODES[idx].name;
    }
    return "unknown";
}

// =============================================================================
// Turb Texture Types (from gl_model.h)
// =============================================================================

/**
 * @brief Types of turbulent (animated) textures
 */
enum class TurbType : uint8_t {
    None = 0,
    Water,      // Water surfaces
    Slime,      // Slime surfaces  
    Lava,       // Lava surfaces
    Teleport,   // Teleporter surfaces
    Sky,        // Sky texture (special handling)
    
    Count
};

/**
 * @brief Get turb type name
 */
[[nodiscard]] inline std::string_view turbTypeName(TurbType type) noexcept {
    switch (type) {
        case TurbType::None: return "none";
        case TurbType::Water: return "water";
        case TurbType::Slime: return "slime";
        case TurbType::Lava: return "lava";
        case TurbType::Teleport: return "teleport";
        case TurbType::Sky: return "sky";
        default: return "unknown";
    }
}

/**
 * @brief Check if turb type is liquid
 */
[[nodiscard]] constexpr bool isLiquid(TurbType type) noexcept {
    return type == TurbType::Water || type == TurbType::Slime || type == TurbType::Lava;
}

// =============================================================================
// Texture Info Structure
// =============================================================================

/**
 * @brief Information about a loaded texture
 * 
 * Corresponds to gltexture_t in the C code
 */
struct TextureInfo {
    // Identification
    std::string identifier;
    std::string pathname;
    uint32_t crc = 0;
    
    // Dimensions
    int imageWidth = 0;         // Original image dimensions
    int imageHeight = 0;
    int textureWidth = 0;       // Actual texture dimensions (may be power of 2)
    int textureHeight = 0;
    int depth = 1;              // For array textures
    int bpp = 4;                // Bytes per pixel
    int mipLevels = 1;
    
    // Type and flags
    TextureType type = TextureType::Tex2D;
    uint32_t flags = 0;
    MinFilter minFilter = MinFilter::LinearMipmapLinear;
    MagFilter magFilter = MagFilter::Linear;
    
    // State
    bool isArray = false;
    bool mipmapsGenerated = false;
    bool storageAllocated = false;
    
    // Reference
    TextureRef reference{};
    uint32_t glName = 0;        // OpenGL texture name/ID
    
    [[nodiscard]] bool isValid() const noexcept {
        return reference.isValid() && textureWidth > 0 && textureHeight > 0;
    }
    
    [[nodiscard]] bool hasMipmaps() const noexcept {
        return TextureFlags::hasMipmaps(flags);
    }
    
    [[nodiscard]] bool hasAlpha() const noexcept {
        return TextureFlags::hasAlpha(flags);
    }
    
    [[nodiscard]] float aspectRatio() const noexcept {
        if (textureHeight == 0) return 1.0f;
        return static_cast<float>(textureWidth) / static_cast<float>(textureHeight);
    }
    
    [[nodiscard]] int totalTexels() const noexcept {
        return textureWidth * textureHeight * depth;
    }
    
    [[nodiscard]] size_t estimatedMemory() const noexcept {
        size_t base = static_cast<size_t>(textureWidth) * static_cast<size_t>(textureHeight) * 
                      static_cast<size_t>(depth) * static_cast<size_t>(bpp);
        // Add ~33% for mipmaps if present
        if (mipmapsGenerated && mipLevels > 1) {
            base = (base * 4) / 3;
        }
        return base;
    }
};

// =============================================================================
// Brush Model Texture
// =============================================================================

/**
 * @brief Brush model texture definition
 * 
 * Corresponds to texture_t in gl_model.h
 */
struct BrushTexture {
    // Identity
    std::array<char, 16> name{};    // Texture name (null-terminated)
    int index = 0;                  // Index in textures array
    
    // Dimensions
    uint32_t width = 0;
    uint32_t height = 0;
    
    // Mipmap offsets (4 levels)
    std::array<uint32_t, 4> offsets{};
    
    // Rendering info
    TextureRef glTexture{};         // Main texture
    TextureRef fbTexture{};         // Fullbright mask
    TextureRef arrayTexture{};      // Array texture reference
    uint32_t arrayIndex = 0;        // Index in texture array
    float scaleS = 1.0f;            // Texture coordinate scale
    float scaleT = 1.0f;
    
    // Flat color for r_fastturb
    uint32_t flatColor3ub = 0;
    
    // Animation
    int animTotal = 0;              // Total tenths in sequence (0 = no animation)
    int animMin = 0;                // Time for this frame (min)
    int animMax = 0;                // Time for this frame (max)
    BrushTexture* animNext = nullptr;       // Next in animation sequence
    BrushTexture* alternateAnims = nullptr; // Alternate animations
    
    // Flags
    bool loaded = false;
    int isLumaTexture = 0;          // 0=normal, 1=luma
    bool isAlphaTested = false;
    bool isLitTurb = false;         // Turb with lightmap
    TurbType turbType = TurbType::None;
    
    // Texture array grouping
    int nextSameSize = -1;
    bool sizeStart = false;
    
    [[nodiscard]] std::string_view getName() const noexcept {
        return std::string_view(name.data());
    }
    
    [[nodiscard]] bool isAnimated() const noexcept {
        return animTotal > 0;
    }
    
    [[nodiscard]] bool isTurbulent() const noexcept {
        return turbType != TurbType::None;
    }
    
    [[nodiscard]] bool hasFullbright() const noexcept {
        return fbTexture.isValid();
    }
    
    [[nodiscard]] bool isSky() const noexcept {
        return turbType == TurbType::Sky || getName().starts_with("sky");
    }
    
    [[nodiscard]] bool isLiquid() const noexcept {
        return render::isLiquid(turbType);
    }
};

// =============================================================================
// QPic Structure (from wad.h)
// =============================================================================

/**
 * @brief Quick picture structure used in WAD files
 * 
 * This is a header for variable-sized picture data
 */
struct QPic {
    int32_t width = 0;
    int32_t height = 0;
    // Data follows (width * height bytes for 8-bit indexed)
    
    [[nodiscard]] bool isValid() const noexcept {
        return width > 0 && height > 0 && 
               width <= texture_limits::MAX_TEXTURE_SIZE && 
               height <= texture_limits::MAX_TEXTURE_SIZE;
    }
    
    [[nodiscard]] size_t dataSize() const noexcept {
        return static_cast<size_t>(width) * static_cast<size_t>(height);
    }
};

// =============================================================================
// Character Set
// =============================================================================

/**
 * @brief Character set for text rendering
 */
struct Charset {
    static constexpr int GLYPH_COUNT = 256;
    
    TextureRef masterTexture{};
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    
    // Glyph dimensions (all same size for console font)
    int glyphWidth = 8;
    int glyphHeight = 8;
    
    [[nodiscard]] bool isValid() const noexcept {
        return masterTexture.isValid();
    }
    
    /** @brief Calculate UV coordinates for a glyph */
    [[nodiscard]] std::array<float, 4> getGlyphUVs(uint8_t ch) const noexcept {
        // Standard 16x16 grid layout
        float col = static_cast<float>(ch % 16);
        float row = static_cast<float>(ch / 16);
        float u1 = col / 16.0f;
        float v1 = row / 16.0f;
        float u2 = (col + 1.0f) / 16.0f;
        float v2 = (row + 1.0f) / 16.0f;
        return { u1, v1, u2, v2 };
    }
};

// =============================================================================
// Texture Array Reference
// =============================================================================

/**
 * @brief Reference to a texture within a texture array
 */
struct TextureArrayRef {
    TextureRef arrayTexture{};
    int32_t index = 0;
    float scaleS = 1.0f;
    float scaleT = 1.0f;
    
    [[nodiscard]] bool isValid() const noexcept {
        return arrayTexture.isValid() && index >= 0;
    }
};

// =============================================================================
// Helper Functions
// =============================================================================

/**
 * @brief Check if a dimension is a power of two
 */
[[nodiscard]] constexpr bool isPowerOfTwo(int n) noexcept {
    return n > 0 && (n & (n - 1)) == 0;
}

/**
 * @brief Round up to next power of two
 */
[[nodiscard]] constexpr int nextPowerOfTwo(int n) noexcept {
    if (n <= 0) return 1;
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    return n + 1;
}

/**
 * @brief Calculate number of mipmap levels for given dimensions
 */
[[nodiscard]] constexpr int calculateMipLevels(int width, int height) noexcept {
    int levels = 1;
    int size = width > height ? width : height;
    while (size > 1) {
        size >>= 1;
        levels++;
    }
    return levels;
}

/**
 * @brief Clamp texture dimension to valid range
 */
[[nodiscard]] constexpr int clampTextureDimension(int size, int maxSize = texture_limits::MAX_TEXTURE_SIZE) noexcept {
    if (size < texture_limits::MIN_TEXTURE_SIZE) return texture_limits::MIN_TEXTURE_SIZE;
    if (size > maxSize) return maxSize;
    return size;
}

} // namespace ezquake::render
