/**
 * @file draw_test.cpp
 * @brief Unit tests for 2D drawing types
 */

#include <gtest/gtest.h>
#include "../../core/draw/draw_types.hpp"

using namespace ezquake::draw;

// ============================================================================
// Color Tests
// ============================================================================

TEST(ColorTest, DefaultConstruction) {
    Color c;
    EXPECT_EQ(c.r(), 255);
    EXPECT_EQ(c.g(), 255);
    EXPECT_EQ(c.b(), 255);
    EXPECT_EQ(c.a(), 255);
}

TEST(ColorTest, ComponentConstruction) {
    Color c(128, 64, 32, 200);
    EXPECT_EQ(c.r(), 128);
    EXPECT_EQ(c.g(), 64);
    EXPECT_EQ(c.b(), 32);
    EXPECT_EQ(c.a(), 200);
}

TEST(ColorTest, PackedConstruction) {
    // 0xAABBGGRR format
    Color c(static_cast<int32_t>(0xFF804020));  // R=32, G=64, B=128, A=255
    EXPECT_EQ(c.r(), 0x20);
    EXPECT_EQ(c.g(), 0x40);
    EXPECT_EQ(c.b(), 0x80);
    EXPECT_EQ(c.a(), 0xFF);
}

TEST(ColorTest, ToFloatRGBA) {
    Color c(255, 127, 0, 255);
    auto rgba = c.toFloatRGBA();
    EXPECT_FLOAT_EQ(rgba[0], 1.0f);
    EXPECT_NEAR(rgba[1], 0.498f, 0.01f);
    EXPECT_FLOAT_EQ(rgba[2], 0.0f);
    EXPECT_FLOAT_EQ(rgba[3], 1.0f);
}

TEST(ColorTest, ToByteRGBA) {
    Color c(100, 150, 200, 250);
    auto rgba = c.toByteRGBA();
    EXPECT_EQ(rgba[0], 100);
    EXPECT_EQ(rgba[1], 150);
    EXPECT_EQ(rgba[2], 200);
    EXPECT_EQ(rgba[3], 250);
}

TEST(ColorTest, PremultiplyAlpha) {
    Color c(200, 100, 50, 127);  // ~50% alpha
    Color premult = c.premultiplyAlpha();
    EXPECT_NEAR(premult.r(), 99, 1);  // 200 * 0.498
    EXPECT_NEAR(premult.g(), 49, 1);  // 100 * 0.498
    EXPECT_NEAR(premult.b(), 24, 1);  // 50 * 0.498
    EXPECT_EQ(premult.a(), 127);
}

TEST(ColorTest, CommonColors) {
    EXPECT_EQ(Color::white().r(), 255);
    EXPECT_EQ(Color::black().r(), 0);
    EXPECT_EQ(Color::red().r(), 255);
    EXPECT_EQ(Color::red().g(), 0);
    EXPECT_EQ(Color::green().g(), 255);
    EXPECT_EQ(Color::blue().b(), 255);
    EXPECT_EQ(Color::transparent().a(), 0);
}

TEST(ColorTest, Equality) {
    Color c1(100, 100, 100, 255);
    Color c2(100, 100, 100, 255);
    Color c3(100, 100, 100, 128);
    
    EXPECT_EQ(c1, c2);
    EXPECT_NE(c1, c3);
}

// ============================================================================
// TextAlignment Tests
// ============================================================================

TEST(TextAlignmentTest, Names) {
    EXPECT_EQ(textAlignmentName(TextAlignment::Left), "left");
    EXPECT_EQ(textAlignmentName(TextAlignment::Center), "center");
    EXPECT_EQ(textAlignmentName(TextAlignment::Right), "right");
}

// ============================================================================
// ColorInfo Tests
// ============================================================================

TEST(ColorInfoTest, DefaultConstruction) {
    ColorInfo info;
    EXPECT_EQ(info.color, Color::white());
    EXPECT_EQ(info.index, 0);
}

TEST(ColorInfoTest, ValueConstruction) {
    ColorInfo info(Color::red(), 5);
    EXPECT_EQ(info.color, Color::red());
    EXPECT_EQ(info.index, 5);
}

TEST(ColorInfoTest, Comparison) {
    ColorInfo a(Color::red(), 5);
    ColorInfo b(Color::blue(), 10);
    EXPECT_TRUE(a < b);
    EXPECT_FALSE(b < a);
}

// ============================================================================
// DrawTextureRef Tests
// ============================================================================

TEST(DrawTextureRefTest, DefaultConstruction) {
    TextureRef ref;
    EXPECT_FALSE(ref.isValid());
    EXPECT_EQ(ref.index, -1);
}

TEST(DrawTextureRefTest, ValueConstruction) {
    TextureRef ref(42);
    EXPECT_TRUE(ref.isValid());
    EXPECT_EQ(ref.index, 42);
}

TEST(DrawTextureRefTest, Equality) {
    TextureRef a(10);
    TextureRef b(10);
    TextureRef c(20);
    
    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

// ============================================================================
// Picture Tests
// ============================================================================

TEST(PictureTest, DefaultConstruction) {
    Picture pic;
    EXPECT_FALSE(pic.isValid());
    EXPECT_EQ(pic.width, 0);
    EXPECT_EQ(pic.height, 0);
}

TEST(PictureTest, IsValid) {
    Picture pic;
    pic.width = 64;
    pic.height = 64;
    EXPECT_FALSE(pic.isValid());  // No texture
    
    pic.texture = TextureRef(1);
    EXPECT_TRUE(pic.isValid());
}

TEST(PictureTest, AspectRatio) {
    Picture pic;
    pic.width = 320;
    pic.height = 200;
    pic.texture = TextureRef(1);
    EXPECT_FLOAT_EQ(pic.aspectRatio(), 1.6f);
}

TEST(PictureTest, UVWidthHeight) {
    Picture pic;
    pic.sl = 0.0f;
    pic.tl = 0.25f;
    pic.sh = 0.5f;
    pic.th = 0.75f;
    
    EXPECT_FLOAT_EQ(pic.uvWidth(), 0.5f);
    EXPECT_FLOAT_EQ(pic.uvHeight(), 0.5f);
}

TEST(PictureTest, GetSubRegionUV) {
    Picture pic;
    pic.width = 256;
    pic.height = 256;
    pic.sl = 0.0f;
    pic.tl = 0.0f;
    pic.sh = 1.0f;
    pic.th = 1.0f;
    
    auto uv = pic.getSubRegionUV(64, 64, 128, 128);
    EXPECT_FLOAT_EQ(uv[0], 0.25f);   // sl
    EXPECT_FLOAT_EQ(uv[1], 0.25f);   // tl
    EXPECT_FLOAT_EQ(uv[2], 0.75f);   // sh
    EXPECT_FLOAT_EQ(uv[3], 0.75f);   // th
}

// ============================================================================
// CachedPicId Tests
// ============================================================================

TEST(CachedPicIdTest, PathLookup) {
    EXPECT_EQ(cachedPicPath(CachedPicId::Pause), "gfx/pause.lmp");
    EXPECT_EQ(cachedPicPath(CachedPicId::Loading), "gfx/loading.lmp");
    EXPECT_EQ(cachedPicPath(CachedPicId::TitleMain), "gfx/ttl_main.lmp");
    EXPECT_EQ(cachedPicPath(CachedPicId::MainMenu), "gfx/mainmenu.lmp");
}

TEST(CachedPicIdTest, BoxPics) {
    EXPECT_EQ(cachedPicPath(CachedPicId::BoxTopLeft), "gfx/box_tl.lmp");
    EXPECT_EQ(cachedPicPath(CachedPicId::BoxMiddleMiddle), "gfx/box_mm.lmp");
    EXPECT_EQ(cachedPicPath(CachedPicId::BoxBottomRight), "gfx/box_br.lmp");
}

TEST(CachedPicIdTest, MenuDots) {
    EXPECT_EQ(cachedPicPath(CachedPicId::MenuDot1), "gfx/menudot1.lmp");
    EXPECT_EQ(cachedPicPath(CachedPicId::MenuDot6), "gfx/menudot6.lmp");
}

// ============================================================================
// WadPicId Tests
// ============================================================================

TEST(WadPicIdTest, NameLookup) {
    EXPECT_EQ(wadPicName(WadPicId::Ram), "ram");
    EXPECT_EQ(wadPicName(WadPicId::Net), "net");
    EXPECT_EQ(wadPicName(WadPicId::Turtle), "turtle");
    EXPECT_EQ(wadPicName(WadPicId::Disc), "disc");
}

TEST(WadPicIdTest, NumberNames) {
    EXPECT_EQ(wadPicName(WadPicId::Num0), "num_0");
    EXPECT_EQ(wadPicName(WadPicId::Num9), "num_9");
    EXPECT_EQ(wadPicName(WadPicId::ANum0), "anum_0");
    EXPECT_EQ(wadPicName(WadPicId::NumMinus), "num_minus");
}

TEST(WadPicIdTest, StatusBarNames) {
    EXPECT_EQ(wadPicName(WadPicId::SbShells), "sb_shells");
    EXPECT_EQ(wadPicName(WadPicId::SbNails), "sb_nails");
    EXPECT_EQ(wadPicName(WadPicId::SbArmor1), "sb_armor1");
    EXPECT_EQ(wadPicName(WadPicId::SbQuad), "sb_quad");
    EXPECT_EQ(wadPicName(WadPicId::SbSbar), "sbar");
}

// ============================================================================
// WadPicture Tests
// ============================================================================

TEST(WadPictureTest, IsLoaded) {
    WadPicture wad;
    EXPECT_FALSE(wad.isLoaded());
    
    Picture pic;
    pic.width = 64;
    pic.height = 64;
    pic.texture = TextureRef(1);
    
    wad.picture = &pic;
    EXPECT_TRUE(wad.isLoaded());
}

// ============================================================================
// ScissorRect Tests
// ============================================================================

TEST(ScissorRectTest, Contains) {
    ScissorRect rect;
    rect.x = 100;
    rect.y = 100;
    rect.width = 200;
    rect.height = 150;
    rect.enabled = true;
    
    EXPECT_TRUE(rect.contains(150, 150));
    EXPECT_TRUE(rect.contains(100, 100));
    EXPECT_FALSE(rect.contains(50, 150));
    EXPECT_FALSE(rect.contains(350, 150));
}

TEST(ScissorRectTest, DisabledAlwaysContains) {
    ScissorRect rect;
    rect.enabled = false;
    
    EXPECT_TRUE(rect.contains(-1000, 5000));
}

TEST(ScissorRectTest, Reset) {
    ScissorRect rect;
    rect.x = 100;
    rect.enabled = true;
    rect.reset();
    
    EXPECT_EQ(rect.x, 0.0f);
    EXPECT_FALSE(rect.enabled);
}

// ============================================================================
// DrawState Tests
// ============================================================================

TEST(DrawStateTest, EffectiveAlpha) {
    DrawState state;
    state.overallAlpha = 0.5f;
    EXPECT_FLOAT_EQ(state.effectiveAlpha(0.8f), 0.4f);
}

TEST(DrawStateTest, Reset) {
    DrawState state;
    state.overallAlpha = 0.5f;
    state.scissor.enabled = true;
    state.reset();
    
    EXPECT_FLOAT_EQ(state.overallAlpha, 1.0f);
    EXPECT_FALSE(state.scissor.enabled);
}

// ============================================================================
// PictureCache Tests
// ============================================================================

TEST(PictureCacheTest, AddAndFind) {
    PictureCache cache;
    EXPECT_TRUE(cache.empty());
    
    auto* pic = cache.add("test/path.png");
    ASSERT_NE(pic, nullptr);
    EXPECT_EQ(cache.count(), 1u);
    
    // Not found yet (not marked as loaded)
    EXPECT_EQ(cache.find("test/path.png"), nullptr);
    
    pic->width = 64;
    pic->height = 64;
    pic->texture = TextureRef(1);
    cache.markLoaded(pic);
    
    // Now should be findable
    EXPECT_EQ(cache.find("test/path.png"), pic);
}

TEST(PictureCacheTest, Clear) {
    PictureCache cache;
    cache.add("a.png");
    cache.add("b.png");
    EXPECT_EQ(cache.count(), 2u);
    
    cache.clear();
    EXPECT_TRUE(cache.empty());
}

// ============================================================================
// BigfontCharCoords Tests
// ============================================================================

TEST(BigfontCharCoordsTest, GetCoords) {
    // Characters laid out in 16-column grid
    auto coords = getBigfontSourceCoords('A', 16, 24);  // ASCII 65
    EXPECT_EQ(coords.sourceX, (65 % 16) * 16);  // Column * width
    EXPECT_EQ(coords.sourceY, (65 / 16) * 24);  // Row * height
}

TEST(BigfontCharCoordsTest, FirstRow) {
    auto coords = getBigfontSourceCoords('\x0F', 16, 24);  // ASCII 15
    EXPECT_EQ(coords.sourceX, 15 * 16);
    EXPECT_EQ(coords.sourceY, 0);  // First row
}

// ============================================================================
// DrawRect Tests
// ============================================================================

TEST(DrawRectTest, BasicProperties) {
    DrawRect rect(100, 50, 200, 150);
    EXPECT_FLOAT_EQ(rect.right(), 300);
    EXPECT_FLOAT_EQ(rect.bottom(), 200);
}

TEST(DrawRectTest, Contains) {
    DrawRect rect(0, 0, 100, 100);
    EXPECT_TRUE(rect.contains(50, 50));
    EXPECT_TRUE(rect.contains(0, 0));
    EXPECT_FALSE(rect.contains(100, 50));  // Right edge is exclusive
    EXPECT_FALSE(rect.contains(-1, 50));
}

TEST(DrawRectTest, Overlaps) {
    DrawRect a(0, 0, 100, 100);
    DrawRect b(50, 50, 100, 100);
    DrawRect c(200, 200, 50, 50);
    
    EXPECT_TRUE(a.overlaps(b));
    EXPECT_TRUE(b.overlaps(a));
    EXPECT_FALSE(a.overlaps(c));
}

TEST(DrawRectTest, FitPictureWider) {
    DrawRect rect(0, 0, 100, 100);
    Picture pic;
    pic.width = 200;
    pic.height = 100;
    pic.texture = TextureRef(1);
    
    auto fitted = rect.fitPicture(pic);
    EXPECT_FLOAT_EQ(fitted.width, 100);  // Fits to rect width
    EXPECT_FLOAT_EQ(fitted.height, 50);  // Maintains 2:1 aspect
    EXPECT_FLOAT_EQ(fitted.y, 25);       // Centered vertically
}

TEST(DrawRectTest, FitPictureTaller) {
    DrawRect rect(0, 0, 100, 100);
    Picture pic;
    pic.width = 100;
    pic.height = 200;
    pic.texture = TextureRef(1);
    
    auto fitted = rect.fitPicture(pic);
    EXPECT_FLOAT_EQ(fitted.height, 100);  // Fits to rect height
    EXPECT_FLOAT_EQ(fitted.width, 50);    // Maintains 1:2 aspect
    EXPECT_FLOAT_EQ(fitted.x, 25);        // Centered horizontally
}

// ============================================================================
// Factory Function Tests
// ============================================================================

TEST(FactoryTest, CreateDefaultDrawState) {
    auto state = createDefaultDrawState();
    EXPECT_TRUE(state.initialized);
    EXPECT_FLOAT_EQ(state.overallAlpha, 1.0f);
}

TEST(FactoryTest, ColorFromRGBA) {
    uint8_t rgba[] = { 128, 64, 32, 255 };
    auto color = colorFromRGBA(rgba);
    EXPECT_EQ(color.r(), 128);
    EXPECT_EQ(color.g(), 64);
    EXPECT_EQ(color.b(), 32);
    EXPECT_EQ(color.a(), 255);
}

TEST(FactoryTest, ColorFromFloatRGBA) {
    float rgba[] = { 1.0f, 0.5f, 0.0f, 1.0f };
    auto color = colorFromFloatRGBA(rgba);
    EXPECT_EQ(color.r(), 255);
    EXPECT_NEAR(color.g(), 127, 1);
    EXPECT_EQ(color.b(), 0);
    EXPECT_EQ(color.a(), 255);
}

TEST(FactoryTest, ColorFromFloatRGBAClamped) {
    float rgba[] = { 2.0f, -0.5f, 0.5f, 1.5f };
    auto color = colorFromFloatRGBA(rgba);
    EXPECT_EQ(color.r(), 255);  // Clamped to 1.0
    EXPECT_EQ(color.g(), 0);    // Clamped to 0.0
    EXPECT_NEAR(color.b(), 127, 1);
    EXPECT_EQ(color.a(), 255);  // Clamped to 1.0
}
