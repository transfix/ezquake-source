/**
 * @file charset_test.cpp
 * @brief Unit tests for charset and font types
 */

#include <gtest/gtest.h>
#include "../../core/charset/charset_types.hpp"

using namespace ezquake::charset;

// ============================================================================
// GlyphCoords Tests
// ============================================================================

TEST(GlyphCoordsTest, DefaultConstruction) {
    GlyphCoords coords;
    EXPECT_FLOAT_EQ(coords.s1, 0.0f);
    EXPECT_FLOAT_EQ(coords.t1, 0.0f);
    EXPECT_FALSE(coords.isValid());
}

TEST(GlyphCoordsTest, ParameterizedConstruction) {
    GlyphCoords coords{0.0f, 0.0f, 0.125f, 0.125f};
    EXPECT_TRUE(coords.isValid());
    EXPECT_FLOAT_EQ(coords.width(), 0.125f);
    EXPECT_FLOAT_EQ(coords.height(), 0.125f);
}

// ============================================================================
// GlyphInfo Tests
// ============================================================================

TEST(GlyphInfoTest, DefaultConstruction) {
    GlyphInfo info;
    EXPECT_FALSE(info.isValid());
    EXPECT_FALSE(info.loaded);
}

TEST(GlyphInfoTest, PixelWidth) {
    GlyphInfo info;
    info.loaded = true;
    info.width = 10.0f;
    info.advanceX = 12.0f;
    
    EXPECT_FLOAT_EQ(info.pixelWidth(), 12.0f);
    
    info.advanceX = 0.0f;
    EXPECT_FLOAT_EQ(info.pixelWidth(), 10.0f);
}

// ============================================================================
// CharsetTextureRef Tests
// ============================================================================

TEST(CharsetTextureRefTest, Validity) {
    CharsetTextureRef ref;
    EXPECT_FALSE(ref.isValid());
    
    ref.id = 42;
    EXPECT_TRUE(ref.isValid());
    EXPECT_TRUE(static_cast<bool>(ref));
}

TEST(CharsetTextureRefTest, Equality) {
    CharsetTextureRef a{100};
    CharsetTextureRef b{100};
    CharsetTextureRef c{200};
    
    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

// ============================================================================
// CharsetPic Tests
// ============================================================================

TEST(CharsetPicTest, DefaultConstruction) {
    CharsetPic pic;
    EXPECT_FALSE(pic.isValid());
}

TEST(CharsetPicTest, Validity) {
    CharsetPic pic;
    pic.width = 8.0f;
    pic.height = 8.0f;
    
    EXPECT_TRUE(pic.isValid());
}

TEST(CharsetPicTest, GetCoords) {
    CharsetPic pic;
    pic.sl = 0.0f;
    pic.tl = 0.0f;
    pic.sh = 0.125f;
    pic.th = 0.125f;
    
    auto coords = pic.getCoords();
    EXPECT_FLOAT_EQ(coords.s1, 0.0f);
    EXPECT_FLOAT_EQ(coords.s2, 0.125f);
}

// ============================================================================
// Charset Tests
// ============================================================================

TEST(CharsetTest, DefaultConstruction) {
    Charset charset;
    EXPECT_FALSE(charset.loaded);
    EXPECT_FLOAT_EQ(charset.customScaleX, 1.0f);
}

TEST(CharsetTest, HasGlyph) {
    Charset charset;
    EXPECT_FALSE(charset.hasGlyph('A'));
    
    charset.glyphs[static_cast<uint8_t>('A')].width = 8.0f;
    charset.glyphs[static_cast<uint8_t>('A')].height = 8.0f;
    
    EXPECT_TRUE(charset.hasGlyph('A'));
}

TEST(CharsetTest, GetCharWidth) {
    Charset charset;
    charset.glyphs[static_cast<uint8_t>('X')].width = 10.0f;
    charset.glyphs[static_cast<uint8_t>('X')].height = 12.0f;
    
    EXPECT_FLOAT_EQ(charset.getCharWidth('X'), 10.0f);
    EXPECT_FLOAT_EQ(charset.getCharWidth('Y'), 0.0f);
}

TEST(CharsetTest, Scale) {
    Charset charset;
    charset.glyphs[static_cast<uint8_t>('A')].width = 8.0f;
    charset.glyphs[static_cast<uint8_t>('A')].height = 8.0f;
    
    charset.setScale(2.0f, 1.5f);
    
    EXPECT_FLOAT_EQ(charset.getCharWidth('A'), 16.0f);
    EXPECT_FLOAT_EQ(charset.getCharHeight('A'), 12.0f);
}

TEST(CharsetTest, Clear) {
    Charset charset;
    charset.loaded = true;
    charset.name = "test";
    charset.glyphs[0].width = 10.0f;
    charset.glyphs[0].height = 10.0f;
    
    charset.clear();
    
    EXPECT_FALSE(charset.loaded);
    EXPECT_FALSE(charset.hasGlyph(0));
}

// ============================================================================
// FontColor Tests
// ============================================================================

TEST(FontColorTest, DefaultConstruction) {
    FontColor color;
    EXPECT_TRUE(color.isWhite());
}

TEST(FontColorTest, CustomColor) {
    FontColor color{128, 64, 32, 255};
    EXPECT_EQ(color.r, 128u);
    EXPECT_EQ(color.g, 64u);
    EXPECT_EQ(color.b, 32u);
    EXPECT_FALSE(color.isWhite());
}

TEST(FontColorTest, Lerp) {
    FontColor a{0, 0, 0, 255};
    FontColor b{255, 255, 255, 255};
    
    auto mid = FontColor::lerp(a, b, 0.5f);
    EXPECT_NEAR(mid.r, 127, 1);
    EXPECT_NEAR(mid.g, 127, 1);
    EXPECT_NEAR(mid.b, 127, 1);
}

TEST(FontColorTest, LerpClamp) {
    FontColor a{100, 100, 100, 255};
    FontColor b{200, 200, 200, 255};
    
    auto under = FontColor::lerp(a, b, -0.5f);
    EXPECT_EQ(under.r, 100u);  // Clamped to 0
    
    auto over = FontColor::lerp(a, b, 1.5f);
    EXPECT_EQ(over.r, 200u);   // Clamped to 1
}

// ============================================================================
// FontGradient Tests
// ============================================================================

TEST(FontGradientTest, DefaultConstruction) {
    FontGradient gradient;
    EXPECT_FLOAT_EQ(gradient.gradientStart, 0.2f);
}

TEST(FontGradientTest, ColorAt) {
    FontGradient gradient{{255, 0, 0}, {0, 255, 0}, 0.5f};
    
    // Before gradient start - top color
    auto top = gradient.colorAt(0.25f);
    EXPECT_EQ(top.r, 255u);
    EXPECT_EQ(top.g, 0u);
    
    // At gradient start
    auto start = gradient.colorAt(0.5f);
    EXPECT_EQ(start.r, 255u);
    
    // At bottom - should be bottom color
    auto bottom = gradient.colorAt(1.0f);
    EXPECT_EQ(bottom.r, 0u);
    EXPECT_EQ(bottom.g, 255u);
}

TEST(FontGradientTest, StandardGradients) {
    EXPECT_TRUE(gradients::Normal.topColor.isWhite());
    EXPECT_FALSE(gradients::Alternate.topColor.isWhite());
}

// ============================================================================
// FontConfig Tests
// ============================================================================

TEST(FontConfigTest, DefaultConstruction) {
    FontConfig config;
    EXPECT_FALSE(config.capitalize);
    EXPECT_FLOAT_EQ(config.outlineWidth, 2.0f);
    EXPECT_FLOAT_EQ(config.scale, 1.0f);
}

// ============================================================================
// CharsetManager Tests
// ============================================================================

TEST(CharsetManagerTest, DefaultConstruction) {
    CharsetManager manager;
    EXPECT_FALSE(manager.initialized);
    EXPECT_EQ(manager.loadedCharsetCount(), 0);
}

TEST(CharsetManagerTest, GetCharset) {
    CharsetManager manager;
    
    auto* cs = manager.getCharset(0);
    ASSERT_NE(cs, nullptr);
    
    EXPECT_EQ(manager.getCharset(-1), nullptr);
    EXPECT_EQ(manager.getCharset(charset_limits::MAX_CHARSETS), nullptr);
}

TEST(CharsetManagerTest, LoadedCount) {
    CharsetManager manager;
    
    manager.charsets[0].loaded = true;
    manager.charsets[5].loaded = true;
    
    EXPECT_EQ(manager.loadedCharsetCount(), 2);
}

TEST(CharsetManagerTest, ClearAll) {
    CharsetManager manager;
    manager.charsets[0].loaded = true;
    manager.maxGlyphWidth = 10.0f;
    manager.initialized = true;
    
    manager.clearAll();
    
    EXPECT_FALSE(manager.charsets[0].loaded);
    EXPECT_FLOAT_EQ(manager.maxGlyphWidth, 0.0f);
    EXPECT_FALSE(manager.initialized);
}

// ============================================================================
// CharCategory Tests
// ============================================================================

TEST(CharCategoryTest, CategorizeChar) {
    EXPECT_EQ(categorizeChar('5'), CharCategory::Digit);
    EXPECT_EQ(categorizeChar('A'), CharCategory::Uppercase);
    EXPECT_EQ(categorizeChar('z'), CharCategory::Lowercase);
    EXPECT_EQ(categorizeChar(' '), CharCategory::Whitespace);
    EXPECT_EQ(categorizeChar('.'), CharCategory::Punctuation);
}

TEST(CharCategoryTest, IsDigit) {
    EXPECT_TRUE(isDigit('0'));
    EXPECT_TRUE(isDigit('9'));
    EXPECT_FALSE(isDigit('a'));
    EXPECT_FALSE(isDigit('A'));
}

TEST(CharCategoryTest, IsLetter) {
    EXPECT_TRUE(isLetter('a'));
    EXPECT_TRUE(isLetter('Z'));
    EXPECT_FALSE(isLetter('5'));
    EXPECT_FALSE(isLetter(' '));
}

// ============================================================================
// TextMetrics Tests
// ============================================================================

TEST(TextMetricsTest, DefaultConstruction) {
    TextMetrics metrics;
    EXPECT_TRUE(metrics.isEmpty());
    EXPECT_EQ(metrics.lineCount, 1);
}

TEST(TextMetricsTest, MeasureText) {
    Charset charset;
    // Set up some glyphs
    for (char c = 'A'; c <= 'Z'; ++c) {
        auto& glyph = charset.glyphs[static_cast<uint8_t>(c)];
        glyph.width = 8.0f;
        glyph.height = 12.0f;
    }
    
    auto metrics = measureText(charset, "HELLO");
    
    EXPECT_FLOAT_EQ(metrics.width, 40.0f);  // 5 * 8
    EXPECT_EQ(metrics.charCount, 5);
    EXPECT_EQ(metrics.lineCount, 1);
}

TEST(TextMetricsTest, MeasureMultiline) {
    Charset charset;
    for (char c = 'A'; c <= 'Z'; ++c) {
        charset.glyphs[static_cast<uint8_t>(c)].width = 8.0f;
        charset.glyphs[static_cast<uint8_t>(c)].height = 12.0f;
    }
    
    auto metrics = measureText(charset, "AB\nCDE");
    
    EXPECT_FLOAT_EQ(metrics.width, 24.0f);  // max of 2*8 and 3*8
    EXPECT_EQ(metrics.lineCount, 2);
}

// ============================================================================
// Helper Function Tests
// ============================================================================

TEST(CharsetHelpersTest, FixedWidth) {
    float width = fixedWidth(10, 1.0f, false, false);
    EXPECT_FLOAT_EQ(width, 80.0f);
    
    width = fixedWidth(5, 2.0f, false, false);
    EXPECT_FLOAT_EQ(width, 80.0f);
}

TEST(CharsetHelpersTest, CharsetFromWideChar) {
    EXPECT_EQ(charsetFromWideChar(L'A'), 0);
    EXPECT_EQ(charsetFromWideChar(static_cast<wchar_t>(0x0141)), 1); // Polish L with stroke
}

TEST(CharsetHelpersTest, GlyphIndexFromWideChar) {
    EXPECT_EQ(glyphIndexFromWideChar(L'A'), 'A');
    EXPECT_EQ(glyphIndexFromWideChar(static_cast<wchar_t>(0x0141)), 0x41);
}

TEST(CharsetHelpersTest, CreateDefaultFontConfig) {
    auto config = createDefaultFontConfig();
    EXPECT_FALSE(config.capitalize);
    EXPECT_FLOAT_EQ(config.scale, 1.0f);
}
