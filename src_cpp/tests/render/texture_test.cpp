/**
 * @file texture_test.cpp
 * @brief Tests for texture types
 * 
 * Part of ezQuake C to C++20 Conversion - Iteration 22
 */

#include <gtest/gtest.h>
#include "core/render/texture_types.hpp"

using namespace ezquake::render;

// =============================================================================
// TextureType Tests
// =============================================================================

TEST(TextureTypeTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(TextureType::Tex2D), 0);
    EXPECT_EQ(static_cast<uint8_t>(TextureType::Tex2DArray), 1);
    EXPECT_EQ(static_cast<uint8_t>(TextureType::CubeMap), 2);
    EXPECT_EQ(static_cast<uint8_t>(TextureType::Tex2DMultisampled), 3);
    EXPECT_EQ(static_cast<uint8_t>(TextureType::Count), 4);
}

// =============================================================================
// MinFilter Tests
// =============================================================================

TEST(MinFilterTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(MinFilter::Nearest), 0);
    EXPECT_EQ(static_cast<uint8_t>(MinFilter::Linear), 1);
    EXPECT_EQ(static_cast<uint8_t>(MinFilter::NearestMipmapNearest), 2);
    EXPECT_EQ(static_cast<uint8_t>(MinFilter::LinearMipmapNearest), 3);
    EXPECT_EQ(static_cast<uint8_t>(MinFilter::NearestMipmapLinear), 4);
    EXPECT_EQ(static_cast<uint8_t>(MinFilter::LinearMipmapLinear), 5);
}

TEST(MinFilterTest, Names) {
    EXPECT_EQ(minFilterName(MinFilter::Nearest), "GL_NEAREST");
    EXPECT_EQ(minFilterName(MinFilter::Linear), "GL_LINEAR");
    EXPECT_EQ(minFilterName(MinFilter::LinearMipmapLinear), "GL_LINEAR_MIPMAP_LINEAR");
}

// =============================================================================
// MagFilter Tests
// =============================================================================

TEST(MagFilterTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(MagFilter::Nearest), 0);
    EXPECT_EQ(static_cast<uint8_t>(MagFilter::Linear), 1);
    EXPECT_EQ(static_cast<uint8_t>(MagFilter::Count), 2);
}

// =============================================================================
// TextureFlags Tests
// =============================================================================

TEST(TextureFlagsTest, Values) {
    EXPECT_EQ(TextureFlags::None, 0);
    EXPECT_EQ(TextureFlags::Complain, 1 << 0);
    EXPECT_EQ(TextureFlags::Mipmap, 1 << 1);
    EXPECT_EQ(TextureFlags::Alpha, 1 << 2);
    EXPECT_EQ(TextureFlags::Luma, 1 << 3);
    EXPECT_EQ(TextureFlags::Fullbright, 1 << 4);
    EXPECT_EQ(TextureFlags::Float, 1 << 13);
    EXPECT_EQ(TextureFlags::Viewmodel, 1 << 14);
}

TEST(TextureFlagsTest, HasMipmaps) {
    EXPECT_TRUE(TextureFlags::hasMipmaps(TextureFlags::Mipmap));
    EXPECT_TRUE(TextureFlags::hasMipmaps(TextureFlags::Mipmap | TextureFlags::Alpha));
    EXPECT_FALSE(TextureFlags::hasMipmaps(TextureFlags::Alpha));
    EXPECT_FALSE(TextureFlags::hasMipmaps(TextureFlags::None));
}

TEST(TextureFlagsTest, HasAlpha) {
    EXPECT_TRUE(TextureFlags::hasAlpha(TextureFlags::Alpha));
    EXPECT_TRUE(TextureFlags::hasAlpha(TextureFlags::Alpha | TextureFlags::Mipmap));
    EXPECT_FALSE(TextureFlags::hasAlpha(TextureFlags::Mipmap));
}

TEST(TextureFlagsTest, IsFloat) {
    EXPECT_TRUE(TextureFlags::isFloat(TextureFlags::Float));
    EXPECT_FALSE(TextureFlags::isFloat(TextureFlags::Alpha));
}

TEST(TextureFlagsTest, ShouldScale) {
    EXPECT_TRUE(TextureFlags::shouldScale(TextureFlags::None));
    EXPECT_TRUE(TextureFlags::shouldScale(TextureFlags::Mipmap));
    EXPECT_FALSE(TextureFlags::shouldScale(TextureFlags::NoScale));
}

TEST(TextureFlagsTest, IsLuma) {
    EXPECT_TRUE(TextureFlags::isLuma(TextureFlags::Luma | TextureFlags::Fullbright));
    EXPECT_FALSE(TextureFlags::isLuma(TextureFlags::Luma));
    EXPECT_FALSE(TextureFlags::isLuma(TextureFlags::Fullbright));
}

// =============================================================================
// Texture Limits Tests
// =============================================================================

TEST(TextureLimitsTest, Values) {
    EXPECT_EQ(texture_limits::MAX_TEXTURES, 8192);
    EXPECT_EQ(texture_limits::MAX_CHARSETS, 256);
    EXPECT_EQ(texture_limits::MIP_LEVELS, 4);
    EXPECT_EQ(texture_limits::MAX_TEXTURE_SIZE, 16384);
    EXPECT_EQ(texture_limits::MIN_TEXTURE_SIZE, 1);
}

// =============================================================================
// WadLumpType Tests
// =============================================================================

TEST(WadLumpTypeTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(WadLumpType::None), 0);
    EXPECT_EQ(static_cast<uint8_t>(WadLumpType::Label), 1);
    EXPECT_EQ(static_cast<uint8_t>(WadLumpType::Lumpy), 64);
    EXPECT_EQ(static_cast<uint8_t>(WadLumpType::Miptex), 68);
}

TEST(WadLumpTypeTest, Names) {
    EXPECT_EQ(wadLumpTypeName(WadLumpType::None), "none");
    EXPECT_EQ(wadLumpTypeName(WadLumpType::Miptex), "miptex");
    EXPECT_EQ(wadLumpTypeName(WadLumpType::QPic), "qpic");
}

// =============================================================================
// TextureRef Tests
// =============================================================================

TEST(TextureRefTest, Default) {
    TextureRef ref;
    EXPECT_EQ(ref.index, 0);
    EXPECT_FALSE(ref.isValid());
    EXPECT_TRUE(ref.isNull());
}

TEST(TextureRefTest, IsValid) {
    TextureRef ref{42};
    EXPECT_TRUE(ref.isValid());
    EXPECT_FALSE(ref.isNull());
}

TEST(TextureRefTest, Invalidate) {
    TextureRef ref{42};
    EXPECT_TRUE(ref.isValid());
    ref.invalidate();
    EXPECT_FALSE(ref.isValid());
    EXPECT_EQ(ref.index, 0);
}

TEST(TextureRefTest, Equality) {
    TextureRef a{42};
    TextureRef b{42};
    TextureRef c{100};
    
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
    EXPECT_TRUE(a != c);
}

TEST(TextureRefTest, NullTexture) {
    EXPECT_TRUE(NULL_TEXTURE.isNull());
    EXPECT_FALSE(NULL_TEXTURE.isValid());
}

// =============================================================================
// FilterMode Tests
// =============================================================================

TEST(FilterModeTest, FindByName) {
    auto result = findFilterMode("GL_LINEAR");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->minFilter, MinFilter::Linear);
    EXPECT_EQ(result->magFilter, MagFilter::Linear);
}

TEST(FilterModeTest, FindTrilinear) {
    auto result = findFilterMode("GL_LINEAR_MIPMAP_LINEAR");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->minFilter, MinFilter::LinearMipmapLinear);
    EXPECT_TRUE(result->usesMipmaps());
}

TEST(FilterModeTest, NotFound) {
    auto result = findFilterMode("INVALID");
    EXPECT_FALSE(result.has_value());
}

TEST(FilterModeTest, UsesMipmaps) {
    // Non-mipmap modes
    EXPECT_FALSE(FILTER_MODES[0].usesMipmaps());  // GL_NEAREST
    EXPECT_FALSE(FILTER_MODES[1].usesMipmaps());  // GL_LINEAR
    
    // Mipmap modes
    EXPECT_TRUE(FILTER_MODES[2].usesMipmaps());   // GL_NEAREST_MIPMAP_NEAREST
    EXPECT_TRUE(FILTER_MODES[5].usesMipmaps());   // GL_LINEAR_MIPMAP_LINEAR
}

// =============================================================================
// TurbType Tests
// =============================================================================

TEST(TurbTypeTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(TurbType::None), 0);
    EXPECT_EQ(static_cast<uint8_t>(TurbType::Water), 1);
    EXPECT_EQ(static_cast<uint8_t>(TurbType::Slime), 2);
    EXPECT_EQ(static_cast<uint8_t>(TurbType::Lava), 3);
}

TEST(TurbTypeTest, Names) {
    EXPECT_EQ(turbTypeName(TurbType::None), "none");
    EXPECT_EQ(turbTypeName(TurbType::Water), "water");
    EXPECT_EQ(turbTypeName(TurbType::Lava), "lava");
}

TEST(TurbTypeTest, IsLiquid) {
    EXPECT_TRUE(isLiquid(TurbType::Water));
    EXPECT_TRUE(isLiquid(TurbType::Slime));
    EXPECT_TRUE(isLiquid(TurbType::Lava));
    EXPECT_FALSE(isLiquid(TurbType::None));
    EXPECT_FALSE(isLiquid(TurbType::Teleport));
    EXPECT_FALSE(isLiquid(TurbType::Sky));
}

// =============================================================================
// TextureInfo Tests
// =============================================================================

TEST(TextureInfoTest, Default) {
    TextureInfo info;
    EXPECT_FALSE(info.isValid());
    EXPECT_EQ(info.imageWidth, 0);
    EXPECT_EQ(info.textureWidth, 0);
    EXPECT_EQ(info.type, TextureType::Tex2D);
}

TEST(TextureInfoTest, IsValid) {
    TextureInfo info;
    info.reference = TextureRef{1};
    info.textureWidth = 256;
    info.textureHeight = 256;
    EXPECT_TRUE(info.isValid());
}

TEST(TextureInfoTest, AspectRatio) {
    TextureInfo info;
    info.textureWidth = 512;
    info.textureHeight = 256;
    EXPECT_FLOAT_EQ(info.aspectRatio(), 2.0f);
}

TEST(TextureInfoTest, TotalTexels) {
    TextureInfo info;
    info.textureWidth = 64;
    info.textureHeight = 64;
    info.depth = 2;
    EXPECT_EQ(info.totalTexels(), 8192);  // 64 * 64 * 2
}

TEST(TextureInfoTest, EstimatedMemory) {
    TextureInfo info;
    info.textureWidth = 256;
    info.textureHeight = 256;
    info.depth = 1;
    info.bpp = 4;
    info.mipmapsGenerated = false;
    
    EXPECT_EQ(info.estimatedMemory(), 262144u);  // 256 * 256 * 4
    
    info.mipmapsGenerated = true;
    info.mipLevels = 8;
    EXPECT_GT(info.estimatedMemory(), 262144u);  // ~33% more
}

TEST(TextureInfoTest, Flags) {
    TextureInfo info;
    info.flags = TextureFlags::Mipmap | TextureFlags::Alpha;
    
    EXPECT_TRUE(info.hasMipmaps());
    EXPECT_TRUE(info.hasAlpha());
}

// =============================================================================
// BrushTexture Tests
// =============================================================================

TEST(BrushTextureTest, Default) {
    BrushTexture tex;
    EXPECT_FALSE(tex.glTexture.isValid());
    EXPECT_FALSE(tex.isAnimated());
    EXPECT_FALSE(tex.isTurbulent());
}

TEST(BrushTextureTest, GetName) {
    BrushTexture tex;
    std::copy_n("e1u1", 5, tex.name.begin());
    EXPECT_EQ(tex.getName(), "e1u1");
}

TEST(BrushTextureTest, IsAnimated) {
    BrushTexture tex;
    EXPECT_FALSE(tex.isAnimated());
    
    tex.animTotal = 4;
    EXPECT_TRUE(tex.isAnimated());
}

TEST(BrushTextureTest, IsTurbulent) {
    BrushTexture tex;
    EXPECT_FALSE(tex.isTurbulent());
    
    tex.turbType = TurbType::Water;
    EXPECT_TRUE(tex.isTurbulent());
}

TEST(BrushTextureTest, IsSky) {
    BrushTexture tex;
    EXPECT_FALSE(tex.isSky());
    
    // By turb type
    tex.turbType = TurbType::Sky;
    EXPECT_TRUE(tex.isSky());
    
    // By name
    tex.turbType = TurbType::None;
    std::copy_n("sky1", 5, tex.name.begin());
    EXPECT_TRUE(tex.isSky());
}

TEST(BrushTextureTest, IsLiquid) {
    BrushTexture tex;
    EXPECT_FALSE(tex.isLiquid());
    
    tex.turbType = TurbType::Water;
    EXPECT_TRUE(tex.isLiquid());
    
    tex.turbType = TurbType::Teleport;
    EXPECT_FALSE(tex.isLiquid());
}

TEST(BrushTextureTest, HasFullbright) {
    BrushTexture tex;
    EXPECT_FALSE(tex.hasFullbright());
    
    tex.fbTexture = TextureRef{5};
    EXPECT_TRUE(tex.hasFullbright());
}

// =============================================================================
// QPic Tests
// =============================================================================

TEST(QPicTest, Default) {
    QPic pic;
    EXPECT_FALSE(pic.isValid());
    EXPECT_EQ(pic.dataSize(), 0u);
}

TEST(QPicTest, IsValid) {
    QPic pic;
    pic.width = 64;
    pic.height = 64;
    EXPECT_TRUE(pic.isValid());
    
    pic.width = 0;
    EXPECT_FALSE(pic.isValid());
    
    pic.width = texture_limits::MAX_TEXTURE_SIZE + 1;
    pic.height = 64;
    EXPECT_FALSE(pic.isValid());
}

TEST(QPicTest, DataSize) {
    QPic pic;
    pic.width = 128;
    pic.height = 64;
    EXPECT_EQ(pic.dataSize(), 8192u);  // 128 * 64
}

// =============================================================================
// Charset Tests
// =============================================================================

TEST(CharsetTest, Default) {
    Charset cs;
    EXPECT_FALSE(cs.isValid());
    EXPECT_EQ(cs.glyphWidth, 8);
    EXPECT_EQ(cs.glyphHeight, 8);
}

TEST(CharsetTest, GlyphUVs) {
    Charset cs;
    
    // Character 0 (top-left of texture)
    auto uvs0 = cs.getGlyphUVs(0);
    EXPECT_FLOAT_EQ(uvs0[0], 0.0f);   // u1
    EXPECT_FLOAT_EQ(uvs0[1], 0.0f);   // v1
    EXPECT_FLOAT_EQ(uvs0[2], 1.0f / 16.0f);  // u2
    EXPECT_FLOAT_EQ(uvs0[3], 1.0f / 16.0f);  // v2
    
    // Character 'A' (65 = row 4, col 1)
    auto uvsA = cs.getGlyphUVs(65);
    EXPECT_FLOAT_EQ(uvsA[0], 1.0f / 16.0f);   // col 1
    EXPECT_FLOAT_EQ(uvsA[1], 4.0f / 16.0f);   // row 4
}

// =============================================================================
// TextureArrayRef Tests
// =============================================================================

TEST(TextureArrayRefTest, Default) {
    TextureArrayRef ref;
    EXPECT_FALSE(ref.isValid());
    EXPECT_EQ(ref.index, 0);
}

TEST(TextureArrayRefTest, IsValid) {
    TextureArrayRef ref;
    ref.arrayTexture = TextureRef{1};
    ref.index = 5;
    EXPECT_TRUE(ref.isValid());
    
    ref.index = -1;
    EXPECT_FALSE(ref.isValid());
}

// =============================================================================
// Helper Function Tests
// =============================================================================

TEST(IsPowerOfTwoTest, Powers) {
    EXPECT_TRUE(isPowerOfTwo(1));
    EXPECT_TRUE(isPowerOfTwo(2));
    EXPECT_TRUE(isPowerOfTwo(4));
    EXPECT_TRUE(isPowerOfTwo(256));
    EXPECT_TRUE(isPowerOfTwo(1024));
    EXPECT_TRUE(isPowerOfTwo(4096));
}

TEST(IsPowerOfTwoTest, NonPowers) {
    EXPECT_FALSE(isPowerOfTwo(0));
    EXPECT_FALSE(isPowerOfTwo(3));
    EXPECT_FALSE(isPowerOfTwo(5));
    EXPECT_FALSE(isPowerOfTwo(100));
    EXPECT_FALSE(isPowerOfTwo(-1));
}

TEST(NextPowerOfTwoTest, Values) {
    EXPECT_EQ(nextPowerOfTwo(0), 1);
    EXPECT_EQ(nextPowerOfTwo(1), 1);
    EXPECT_EQ(nextPowerOfTwo(2), 2);
    EXPECT_EQ(nextPowerOfTwo(3), 4);
    EXPECT_EQ(nextPowerOfTwo(5), 8);
    EXPECT_EQ(nextPowerOfTwo(100), 128);
    EXPECT_EQ(nextPowerOfTwo(256), 256);
    EXPECT_EQ(nextPowerOfTwo(257), 512);
}

TEST(CalculateMipLevelsTest, Values) {
    EXPECT_EQ(calculateMipLevels(1, 1), 1);
    EXPECT_EQ(calculateMipLevels(2, 2), 2);
    EXPECT_EQ(calculateMipLevels(4, 4), 3);
    EXPECT_EQ(calculateMipLevels(256, 256), 9);
    EXPECT_EQ(calculateMipLevels(512, 256), 10);  // Takes max dimension
}

TEST(ClampTextureDimensionTest, Values) {
    EXPECT_EQ(clampTextureDimension(0), texture_limits::MIN_TEXTURE_SIZE);
    EXPECT_EQ(clampTextureDimension(-5), texture_limits::MIN_TEXTURE_SIZE);
    EXPECT_EQ(clampTextureDimension(100), 100);
    EXPECT_EQ(clampTextureDimension(20000), texture_limits::MAX_TEXTURE_SIZE);
    EXPECT_EQ(clampTextureDimension(100, 50), 50);  // Custom max
}
