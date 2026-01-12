/**
 * @file charset_types.hpp
 * @brief Charset and font type definitions
 * 
 * This module provides types for character sets, font rendering,
 * and glyph management.
 * 
 * Replaces:
 * - r_texture.h: charset_t
 * - fonts.c: glyphinfo_t, gradient_def_t
 */

#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <string_view>
#include <array>
#include <optional>

namespace ezquake::charset {

// ============================================================================
// Constants
// ============================================================================

namespace charset_limits {
    constexpr int32_t MAX_CHARSETS = 256;
    constexpr int32_t MAX_USER_CHARSETS = 0xE0;
    constexpr int32_t GLYPHS_PER_CHARSET = 256;
    constexpr int32_t MAX_GLYPHS = 4096;
    constexpr int32_t FONT_TEXTURE_SIZE = 1024;
    constexpr int32_t TRACKER_IMAGES_CHARSET = 0xE0;
}

// ============================================================================
// Glyph Types
// ============================================================================

/**
 * @brief Texture coordinates for a glyph
 */
struct GlyphCoords {
    float s1 = 0.0f;    ///< Left texture coordinate
    float t1 = 0.0f;    ///< Top texture coordinate
    float s2 = 0.0f;    ///< Right texture coordinate
    float t2 = 0.0f;    ///< Bottom texture coordinate
    
    constexpr GlyphCoords() = default;
    
    constexpr GlyphCoords(float left, float top, float right, float bottom)
        : s1(left), t1(top), s2(right), t2(bottom) {}
    
    [[nodiscard]] constexpr float width() const noexcept { return s2 - s1; }
    [[nodiscard]] constexpr float height() const noexcept { return t2 - t1; }
    
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return s2 > s1 && t2 > t1;
    }
};

/**
 * @brief Information about a single glyph
 */
struct GlyphInfo {
    float width = 0.0f;         ///< Glyph width in pixels
    float height = 0.0f;        ///< Glyph height in pixels
    float advanceX = 0.0f;      ///< Horizontal advance
    float bearingX = 0.0f;      ///< Horizontal bearing
    float bearingY = 0.0f;      ///< Vertical bearing
    GlyphCoords texCoords;      ///< Texture coordinates
    uint32_t codepoint = 0;     ///< Unicode codepoint
    bool loaded = false;        ///< Whether glyph data is valid
    
    GlyphInfo() = default;
    
    [[nodiscard]] bool isValid() const noexcept {
        return loaded && width > 0;
    }
    
    [[nodiscard]] float pixelWidth() const noexcept {
        return advanceX > 0 ? advanceX : width;
    }
};

// ============================================================================
// Picture/Texture Reference
// ============================================================================

/**
 * @brief Texture reference for charset
 */
struct CharsetTextureRef {
    uint32_t id = 0;
    
    [[nodiscard]] bool isValid() const noexcept { return id != 0; }
    [[nodiscard]] explicit operator bool() const noexcept { return isValid(); }
    
    bool operator==(const CharsetTextureRef& other) const noexcept = default;
};

/**
 * @brief Picture data (glyph or image in charset)
 * 
 * Matches C: mpic_t structure for glyphs
 */
struct CharsetPic {
    float width = 0.0f;
    float height = 0.0f;
    float sl = 0.0f;    ///< Left texture coord
    float tl = 0.0f;    ///< Top texture coord
    float sh = 0.0f;    ///< Right texture coord (high)
    float th = 0.0f;    ///< Bottom texture coord (high)
    CharsetTextureRef texture;
    
    CharsetPic() = default;
    
    [[nodiscard]] bool isValid() const noexcept {
        return width > 0 && height > 0;
    }
    
    [[nodiscard]] GlyphCoords getCoords() const noexcept {
        return {sl, tl, sh, th};
    }
};

// ============================================================================
// Charset
// ============================================================================

/**
 * @brief Character set with glyphs
 * 
 * Replaces C: charset_t
 */
struct Charset {
    std::array<CharsetPic, charset_limits::GLYPHS_PER_CHARSET> glyphs{};
    CharsetTextureRef masterTexture;
    float customScaleX = 1.0f;
    float customScaleY = 1.0f;
    std::string name;
    bool loaded = false;
    
    Charset() = default;
    
    [[nodiscard]] const CharsetPic* getGlyph(uint8_t ch) const noexcept {
        return glyphs[ch].isValid() ? &glyphs[ch] : nullptr;
    }
    
    [[nodiscard]] CharsetPic* getGlyph(uint8_t ch) noexcept {
        return &glyphs[ch];
    }
    
    [[nodiscard]] bool hasGlyph(uint8_t ch) const noexcept {
        return glyphs[ch].isValid();
    }
    
    [[nodiscard]] float getCharWidth(uint8_t ch) const noexcept {
        return glyphs[ch].isValid() ? glyphs[ch].width * customScaleX : 0.0f;
    }
    
    [[nodiscard]] float getCharHeight(uint8_t ch) const noexcept {
        return glyphs[ch].isValid() ? glyphs[ch].height * customScaleY : 0.0f;
    }
    
    void setScale(float scaleX, float scaleY) noexcept {
        customScaleX = scaleX;
        customScaleY = scaleY;
    }
    
    void clear() {
        glyphs.fill(CharsetPic{});
        masterTexture = CharsetTextureRef{};
        customScaleX = 1.0f;
        customScaleY = 1.0f;
        loaded = false;
    }
};

// ============================================================================
// Font Gradient
// ============================================================================

/**
 * @brief Color for gradient
 */
struct FontColor {
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    uint8_t a = 255;
    
    constexpr FontColor() = default;
    
    constexpr FontColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}
    
    [[nodiscard]] constexpr bool isWhite() const noexcept {
        return r == 255 && g == 255 && b == 255;
    }
    
    /**
     * @brief Interpolate between two colors
     */
    [[nodiscard]] static FontColor lerp(const FontColor& a, const FontColor& b, float t) noexcept {
        t = std::clamp(t, 0.0f, 1.0f);
        return FontColor{
            static_cast<uint8_t>(static_cast<float>(a.r) * (1.0f - t) + static_cast<float>(b.r) * t),
            static_cast<uint8_t>(static_cast<float>(a.g) * (1.0f - t) + static_cast<float>(b.g) * t),
            static_cast<uint8_t>(static_cast<float>(a.b) * (1.0f - t) + static_cast<float>(b.b) * t),
            static_cast<uint8_t>(static_cast<float>(a.a) * (1.0f - t) + static_cast<float>(b.a) * t)
        };
    }
};

/**
 * @brief Font gradient definition
 * 
 * Replaces C: gradient_def_t
 */
struct FontGradient {
    FontColor topColor{255, 255, 255};
    FontColor bottomColor{107, 98, 86};
    float gradientStart = 0.2f;     ///< Y position where gradient starts (0-1)
    
    FontGradient() = default;
    
    FontGradient(const FontColor& top, const FontColor& bottom, float start)
        : topColor(top), bottomColor(bottom), gradientStart(std::clamp(start, 0.0f, 1.0f)) {}
    
    /**
     * @brief Get color at relative Y position
     */
    [[nodiscard]] FontColor colorAt(float relativeY) const noexcept {
        if (relativeY <= gradientStart) {
            return topColor;
        }
        float mix = (relativeY - gradientStart) / (1.0f - gradientStart);
        return FontColor::lerp(topColor, bottomColor, mix);
    }
};

// Standard gradients
namespace gradients {
    inline FontGradient Normal{{255, 255, 255}, {107, 98, 86}, 0.2f};
    inline FontGradient Alternate{{175, 120, 52}, {75, 52, 22}, 0.4f};
    inline FontGradient Numbers{{255, 255, 150}, {218, 132, 7}, 0.2f};
}

// ============================================================================
// Font Configuration
// ============================================================================

/**
 * @brief Font rendering configuration
 */
struct FontConfig {
    std::string facePath;           ///< Path to font file
    bool capitalize = false;        ///< Auto-capitalize
    float outlineWidth = 2.0f;      ///< Outline thickness
    float scale = 1.0f;             ///< Base scale
    bool proportional = false;      ///< Use proportional spacing
    
    FontGradient normalGradient;
    FontGradient alternateGradient;
    FontGradient numberGradient;
    
    FontConfig() 
        : normalGradient(gradients::Normal)
        , alternateGradient(gradients::Alternate)
        , numberGradient(gradients::Numbers) {}
};

// ============================================================================
// Charset Manager State
// ============================================================================

/**
 * @brief Charset manager state
 */
struct CharsetManager {
    std::array<Charset, charset_limits::MAX_CHARSETS> charsets{};
    std::array<Charset, charset_limits::MAX_CHARSETS> proportionalFonts{};
    float maxGlyphWidth = 0.0f;
    float maxNumGlyphWidth = 0.0f;
    FontConfig config;
    bool initialized = false;
    
    [[nodiscard]] Charset* getCharset(int32_t index) noexcept {
        if (index < 0 || index >= charset_limits::MAX_CHARSETS) return nullptr;
        return &charsets[static_cast<size_t>(index)];
    }
    
    [[nodiscard]] const Charset* getCharset(int32_t index) const noexcept {
        if (index < 0 || index >= charset_limits::MAX_CHARSETS) return nullptr;
        return &charsets[static_cast<size_t>(index)];
    }
    
    [[nodiscard]] Charset* getProportionalFont(int32_t index) noexcept {
        if (index < 0 || index >= charset_limits::MAX_CHARSETS) return nullptr;
        return &proportionalFonts[static_cast<size_t>(index)];
    }
    
    [[nodiscard]] int32_t loadedCharsetCount() const noexcept {
        int32_t count = 0;
        for (const auto& cs : charsets) {
            if (cs.loaded) ++count;
        }
        return count;
    }
    
    void clearAll() {
        for (auto& cs : charsets) cs.clear();
        for (auto& pf : proportionalFonts) pf.clear();
        maxGlyphWidth = 0.0f;
        maxNumGlyphWidth = 0.0f;
        initialized = false;
    }
};

// ============================================================================
// Character Classification
// ============================================================================

/**
 * @brief Character category for styling
 */
enum class CharCategory : uint8_t {
    Normal,         ///< Regular character
    Digit,          ///< 0-9
    Uppercase,      ///< A-Z
    Lowercase,      ///< a-z
    Punctuation,    ///< Punctuation marks
    Whitespace,     ///< Spaces, tabs
    Special         ///< Control chars, high-bit chars
};

[[nodiscard]] constexpr CharCategory categorizeChar(char ch) noexcept {
    if (ch >= '0' && ch <= '9') return CharCategory::Digit;
    if (ch >= 'A' && ch <= 'Z') return CharCategory::Uppercase;
    if (ch >= 'a' && ch <= 'z') return CharCategory::Lowercase;
    if (ch == ' ' || ch == '\t' || ch == '\n') return CharCategory::Whitespace;
    if (ch < 32 || static_cast<unsigned char>(ch) >= 128) return CharCategory::Special;
    return CharCategory::Punctuation;
}

[[nodiscard]] constexpr bool isDigit(char ch) noexcept {
    return ch >= '0' && ch <= '9';
}

[[nodiscard]] constexpr bool isLetter(char ch) noexcept {
    return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
}

// ============================================================================
// Text Measurement
// ============================================================================

/**
 * @brief Text measurement result
 */
struct TextMetrics {
    float width = 0.0f;         ///< Total width in pixels
    float height = 0.0f;        ///< Maximum height
    int32_t charCount = 0;      ///< Number of characters
    int32_t lineCount = 1;      ///< Number of lines
    
    [[nodiscard]] bool isEmpty() const noexcept {
        return charCount == 0;
    }
};

/**
 * @brief Measure text with given charset
 */
[[nodiscard]] inline TextMetrics measureText(
    const Charset& charset,
    std::string_view text,
    float scale = 1.0f
) noexcept {
    TextMetrics metrics;
    float lineWidth = 0.0f;
    float maxHeight = 0.0f;
    
    for (char c : text) {
        if (c == '\n') {
            metrics.width = std::max(metrics.width, lineWidth);
            lineWidth = 0.0f;
            ++metrics.lineCount;
            continue;
        }
        
        auto ch = static_cast<uint8_t>(c);
        if (auto* glyph = charset.getGlyph(ch)) {
            lineWidth += glyph->width * scale;
            maxHeight = std::max(maxHeight, glyph->height * scale);
        }
        ++metrics.charCount;
    }
    
    metrics.width = std::max(metrics.width, lineWidth);
    metrics.height = maxHeight * static_cast<float>(metrics.lineCount);
    
    return metrics;
}

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * @brief Get fixed-width calculation for text
 */
[[nodiscard]] inline float fixedWidth(
    int32_t maxLength,
    float scale,
    bool digitsOnly,
    bool proportional
) noexcept {
    // Simple fallback without FreeType
    return static_cast<float>(maxLength) * 8.0f * scale;
}

/**
 * @brief Get charset index from wide character
 */
[[nodiscard]] inline int32_t charsetFromWideChar(wchar_t ch) noexcept {
    return static_cast<int32_t>((static_cast<uint32_t>(ch) >> 8) & 0xFF);
}

/**
 * @brief Get glyph index within charset from wide character
 */
[[nodiscard]] inline uint8_t glyphIndexFromWideChar(wchar_t ch) noexcept {
    return static_cast<uint8_t>(ch & 0xFF);
}

/**
 * @brief Create default font configuration
 */
[[nodiscard]] inline FontConfig createDefaultFontConfig() {
    return FontConfig{};
}

} // namespace ezquake::charset
