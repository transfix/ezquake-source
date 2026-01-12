/**
 * @file lightmap_test.cpp
 * @brief Tests for lightmap and dynamic lighting types
 */

#include <gtest/gtest.h>
#include "core/render/lightmap_types.hpp"

using namespace ezquake::render;

// =============================================================================
// Lightmap Limits Tests
// =============================================================================

TEST(LightmapLimitsTest, Constants) {
    EXPECT_EQ(lightmap_limits::MAX_LIGHTMAPS, 4);
    EXPECT_EQ(lightmap_limits::LIGHTMAP_WIDTH, 128);
    EXPECT_EQ(lightmap_limits::LIGHTMAP_HEIGHT, 128);
    EXPECT_EQ(lightmap_limits::MAX_LIGHTMAP_SIZE, 64 * 64);
    EXPECT_EQ(lightmap_limits::MAX_LIGHTSTYLES, 64);
    EXPECT_EQ(lightmap_limits::MAX_STYLESTRING, 64);
    EXPECT_EQ(lightmap_limits::MAX_DLIGHTS, 32);
}

TEST(LightmapLimitsTest, DerivedConstants) {
    EXPECT_EQ(lightmap_limits::DLIGHT_ACTIVE_BITS, 32);
    EXPECT_EQ(lightmap_limits::DLIGHT_ACTIVE_WORDS, 1);
    EXPECT_EQ(lightmap_limits::BYTES_PER_PIXEL, 4);
    EXPECT_EQ(lightmap_limits::LIGHTMAP_BYTES, 4 * 128 * 128);
}

// =============================================================================
// DLightColorType Tests
// =============================================================================

TEST(DLightColorTypeTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(DLightColorType::Default), 0);
    EXPECT_EQ(static_cast<uint8_t>(DLightColorType::MuzzleFlash), 1);
    EXPECT_EQ(static_cast<uint8_t>(DLightColorType::Explosion), 2);
    EXPECT_EQ(static_cast<uint8_t>(DLightColorType::Rocket), 3);
    EXPECT_EQ(static_cast<uint8_t>(DLightColorType::Red), 4);
    EXPECT_EQ(static_cast<uint8_t>(DLightColorType::Blue), 5);
    EXPECT_EQ(static_cast<uint8_t>(DLightColorType::Custom), 11);
}

TEST(DLightColorTypeTest, Names) {
    EXPECT_STREQ(getDLightColorTypeName(DLightColorType::Default), "Default");
    EXPECT_STREQ(getDLightColorTypeName(DLightColorType::MuzzleFlash), "MuzzleFlash");
    EXPECT_STREQ(getDLightColorTypeName(DLightColorType::Explosion), "Explosion");
    EXPECT_STREQ(getDLightColorTypeName(DLightColorType::Custom), "Custom");
}

// =============================================================================
// LightColor Tests
// =============================================================================

TEST(LightColorTest, DefaultConstruction) {
    LightColor color;
    EXPECT_EQ(color.r, 255);
    EXPECT_EQ(color.g, 255);
    EXPECT_EQ(color.b, 255);
}

TEST(LightColorTest, ValueConstruction) {
    LightColor color(128, 64, 32);
    EXPECT_EQ(color.r, 128);
    EXPECT_EQ(color.g, 64);
    EXPECT_EQ(color.b, 32);
}

TEST(LightColorTest, FromFloat) {
    auto color = LightColor::fromFloat(1.0f, 0.5f, 0.0f);
    EXPECT_EQ(color.r, 255);
    EXPECT_EQ(color.g, 127);
    EXPECT_EQ(color.b, 0);
}

TEST(LightColorTest, ToFloat) {
    LightColor color(255, 128, 0);
    auto floats = color.toFloat();
    EXPECT_NEAR(floats[0], 1.0f, 0.01f);
    EXPECT_NEAR(floats[1], 0.5f, 0.01f);
    EXPECT_NEAR(floats[2], 0.0f, 0.01f);
}

TEST(LightColorTest, Intensity) {
    auto white = LightColor::white();
    EXPECT_NEAR(white.intensity(), 1.0f, 0.01f);
    
    auto black = LightColor(0, 0, 0);
    EXPECT_NEAR(black.intensity(), 0.0f, 0.01f);
}

TEST(LightColorTest, Presets) {
    EXPECT_EQ(LightColor::white().r, 255);
    EXPECT_EQ(LightColor::red().r, 255);
    EXPECT_EQ(LightColor::red().g, 0);
    EXPECT_EQ(LightColor::green().g, 255);
    EXPECT_EQ(LightColor::blue().b, 255);
}

// =============================================================================
// DynamicLight Tests
// =============================================================================

TEST(RenderDynamicLightTest, DefaultConstruction) {
    DynamicLight light;
    EXPECT_EQ(light.key, 0);
    EXPECT_EQ(light.radius, 0.0f);
    EXPECT_EQ(light.die, 0.0f);
    EXPECT_EQ(light.type, DLightColorType::Default);
}

TEST(RenderDynamicLightTest, IsActive) {
    DynamicLight light;
    light.die = 10.0f;
    
    EXPECT_TRUE(light.isActive(5.0f));
    EXPECT_FALSE(light.isActive(15.0f));
}

TEST(RenderDynamicLightTest, CurrentRadius) {
    DynamicLight light;
    light.radius = 100.0f;
    light.decay = 10.0f;
    
    EXPECT_EQ(light.currentRadius(0.0f, 0.0f), 100.0f);
    EXPECT_EQ(light.currentRadius(5.0f, 0.0f), 50.0f);
    EXPECT_EQ(light.currentRadius(10.0f, 0.0f), 0.0f);
}

TEST(RenderDynamicLightTest, Contributes) {
    DynamicLight light;
    light.radius = 100.0f;
    light.minLight = 10.0f;
    
    EXPECT_TRUE(light.contributes(50.0f));
    EXPECT_FALSE(light.contributes(95.0f)); // Too close to edge
    EXPECT_FALSE(light.contributes(150.0f)); // Outside radius
}

TEST(RenderDynamicLightTest, Reset) {
    DynamicLight light;
    light.key = 42;
    light.radius = 100.0f;
    light.type = DLightColorType::Explosion;
    
    light.reset();
    EXPECT_EQ(light.key, 0);
    EXPECT_EQ(light.radius, 0.0f);
    EXPECT_EQ(light.type, DLightColorType::Default);
}

// =============================================================================
// CustomLight Tests
// =============================================================================

TEST(RenderCustomLightTest, IsCustom) {
    CustomLight light;
    EXPECT_FALSE(light.isCustom());
    
    light.type = DLightColorType::Custom;
    EXPECT_TRUE(light.isCustom());
}

// =============================================================================
// LightStyle Tests
// =============================================================================

TEST(LightStyleTest, DefaultBrightness) {
    LightStyle style;
    // Empty style returns full bright
    EXPECT_EQ(style.getBrightness(0), 256);
}

TEST(LightStyleTest, PatternBrightness) {
    LightStyle style;
    style.setPattern("am");
    
    EXPECT_EQ(style.length, 2);
    EXPECT_EQ(style.getBrightness(0), 0);    // 'a' = 0
    EXPECT_EQ(style.getBrightness(1), 264);  // 'm' = 12 * 22 = 264
}

TEST(LightStyleTest, IsConstant) {
    LightStyle style;
    EXPECT_TRUE(style.isConstant()); // Empty
    
    style.setPattern("m");
    EXPECT_TRUE(style.isConstant()); // Single char
    
    style.setPattern("mz");
    EXPECT_FALSE(style.isConstant()); // Multiple chars
}

TEST(LightStyleTest, NormalizedBrightness) {
    LightStyle style;
    style.setPattern("m"); // Normal brightness
    
    float brightness = style.getNormalizedBrightness(0);
    EXPECT_GT(brightness, 0.9f);
}

// =============================================================================
// LightmapRect Tests
// =============================================================================

TEST(LightmapRectTest, DefaultIsEmpty) {
    LightmapRect rect;
    EXPECT_TRUE(rect.isEmpty());
}

TEST(LightmapRectTest, Dimensions) {
    LightmapRect rect{10, 20, 30, 40};
    EXPECT_EQ(rect.left, 10);
    EXPECT_EQ(rect.top, 20);
    EXPECT_EQ(rect.width, 30);
    EXPECT_EQ(rect.height, 40);
    EXPECT_EQ(rect.right(), 40);
    EXPECT_EQ(rect.bottom(), 60);
}

TEST(LightmapRectTest, Expand) {
    LightmapRect rect{10, 10, 20, 20};
    LightmapRect other{5, 5, 40, 40};
    
    rect.expand(other);
    EXPECT_EQ(rect.left, 5);
    EXPECT_EQ(rect.top, 5);
    EXPECT_EQ(rect.right(), 45);
    EXPECT_EQ(rect.bottom(), 45);
}

TEST(LightmapRectTest, Reset) {
    LightmapRect rect{10, 10, 20, 20};
    rect.reset();
    EXPECT_TRUE(rect.isEmpty());
}

// =============================================================================
// LightmapData Tests
// =============================================================================

TEST(LightmapDataTest, Constants) {
    EXPECT_EQ(LightmapData::WIDTH, 128);
    EXPECT_EQ(LightmapData::HEIGHT, 128);
    EXPECT_EQ(LightmapData::PIXEL_COUNT, 128 * 128);
    EXPECT_EQ(LightmapData::BYTES, 4 * 128 * 128);
}

TEST(LightmapDataTest, ResetAllocation) {
    LightmapData data;
    data.allocated[0] = 50;
    data.allocated[64] = 100;
    
    data.resetAllocation();
    EXPECT_EQ(data.allocated[0], 0);
    EXPECT_EQ(data.allocated[64], 0);
}

TEST(LightmapDataTest, MarkModified) {
    LightmapData data;
    EXPECT_FALSE(data.modified);
    
    data.markFullyModified();
    EXPECT_TRUE(data.modified);
    EXPECT_EQ(data.changeArea.width, 128);
    EXPECT_EQ(data.changeArea.height, 128);
}

TEST(LightmapDataTest, ClearModified) {
    LightmapData data;
    data.markFullyModified();
    data.clearModified();
    
    EXPECT_FALSE(data.modified);
    EXPECT_TRUE(data.changeArea.isEmpty());
}

// =============================================================================
// RGB9E5 Tests
// =============================================================================

TEST(RGB9E5Test, TableValues) {
    // First value (smallest)
    EXPECT_NEAR(RGB9E5_TABLE[0], 1.0f / (1 << 24), 1e-12f);
    // Middle value (1.0)
    EXPECT_NEAR(RGB9E5_TABLE[24], 1.0f, 1e-6f);
    // Last value (largest)
    EXPECT_NEAR(RGB9E5_TABLE[31], 128.0f, 1e-3f);
}

TEST(RGB9E5Test, DecodeZero) {
    auto rgb = decodeRGB9E5(0);
    EXPECT_EQ(rgb[0], 0.0f);
    EXPECT_EQ(rgb[1], 0.0f);
    EXPECT_EQ(rgb[2], 0.0f);
}

// =============================================================================
// SurfaceLightInfo Tests
// =============================================================================

TEST(SurfaceLightInfoTest, DefaultStyles) {
    SurfaceLightInfo info;
    EXPECT_EQ(info.styles[0], 255);
    EXPECT_EQ(info.activeStyles(), 0);
}

TEST(SurfaceLightInfoTest, ActiveStyles) {
    SurfaceLightInfo info;
    info.styles[0] = 0;
    info.styles[1] = 1;
    info.styles[2] = 255;
    
    EXPECT_EQ(info.activeStyles(), 2);
}

// =============================================================================
// VertexLight Tests
// =============================================================================

TEST(VertexLightTest, FinalColor) {
    VertexLight light;
    light.color = LightColor(255, 128, 0);
    light.intensity = 0.5f;
    
    auto final = light.getFinalColor();
    EXPECT_NEAR(final[0], 0.5f, 0.01f);
    EXPECT_NEAR(final[1], 0.25f, 0.01f);
    EXPECT_NEAR(final[2], 0.0f, 0.01f);
}

// =============================================================================
// DynamicLightMode Tests
// =============================================================================

TEST(DynamicLightModeTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(DynamicLightMode::Off), 0);
    EXPECT_EQ(static_cast<uint8_t>(DynamicLightMode::Software), 1);
    EXPECT_EQ(static_cast<uint8_t>(DynamicLightMode::Hardware), 2);
}

TEST(DynamicLightModeTest, IsEnabled) {
    EXPECT_FALSE(isDynamicLightingEnabled(DynamicLightMode::Off));
    EXPECT_TRUE(isDynamicLightingEnabled(DynamicLightMode::Software));
    EXPECT_TRUE(isDynamicLightingEnabled(DynamicLightMode::Hardware));
}

TEST(DynamicLightModeTest, ModeChecks) {
    EXPECT_TRUE(isSoftwareLighting(DynamicLightMode::Software));
    EXPECT_FALSE(isSoftwareLighting(DynamicLightMode::Hardware));
    
    EXPECT_TRUE(isHardwareLighting(DynamicLightMode::Hardware));
    EXPECT_FALSE(isHardwareLighting(DynamicLightMode::Software));
}

// =============================================================================
// LightEntityParams Tests
// =============================================================================

TEST(LightEntityParamsTest, Defaults) {
    LightEntityParams params;
    EXPECT_EQ(params.light, 300.0f);
    EXPECT_EQ(params.style, 0);
    EXPECT_FALSE(params.isSpotlight());
}

TEST(LightEntityParamsTest, Spotlight) {
    LightEntityParams params;
    params.angle = 45.0f;
    EXPECT_TRUE(params.isSpotlight());
}

// =============================================================================
// DynamicLightPool Tests
// =============================================================================

TEST(DynamicLightPoolTest, Constants) {
    EXPECT_EQ(DynamicLightPool::MAX_LIGHTS, 32);
    EXPECT_EQ(DynamicLightPool::ACTIVE_WORDS, 1);
}

TEST(DynamicLightPoolTest, ActiveBits) {
    DynamicLightPool pool;
    
    EXPECT_FALSE(pool.isActive(0));
    pool.setActive(0, true);
    EXPECT_TRUE(pool.isActive(0));
    
    pool.setActive(31, true);
    EXPECT_TRUE(pool.isActive(31));
    
    pool.setActive(0, false);
    EXPECT_FALSE(pool.isActive(0));
}

TEST(DynamicLightPoolTest, FindFreeSlot) {
    DynamicLightPool pool;
    
    EXPECT_EQ(pool.findFreeSlot(), 0);
    
    pool.setActive(0, true);
    EXPECT_EQ(pool.findFreeSlot(), 1);
}

TEST(DynamicLightPoolTest, FindByKey) {
    DynamicLightPool pool;
    
    pool.lights[5].key = 42;
    pool.setActive(5, true);
    
    EXPECT_EQ(pool.findByKey(42), 5);
    EXPECT_EQ(pool.findByKey(99), -1);
    EXPECT_EQ(pool.findByKey(0), -1);
}

TEST(DynamicLightPoolTest, Clear) {
    DynamicLightPool pool;
    pool.setActive(0, true);
    pool.setActive(15, true);
    pool.lights[0].key = 1;
    
    pool.clear();
    EXPECT_FALSE(pool.isActive(0));
    EXPECT_FALSE(pool.isActive(15));
    EXPECT_EQ(pool.lights[0].key, 0);
}

TEST(DynamicLightPoolTest, CountActive) {
    DynamicLightPool pool;
    EXPECT_EQ(pool.countActive(), 0);
    
    pool.setActive(0, true);
    pool.setActive(10, true);
    pool.setActive(20, true);
    EXPECT_EQ(pool.countActive(), 3);
}

// =============================================================================
// LightStylePool Tests
// =============================================================================

TEST(LightStylePoolTest, Constants) {
    EXPECT_EQ(LightStylePool::MAX_STYLES, 64);
}

TEST(LightStylePoolTest, GetValue) {
    LightStylePool pool;
    pool.setValue(0, 512);
    
    EXPECT_EQ(pool.getValue(0), 512);
}

TEST(LightStylePoolTest, Clear) {
    LightStylePool pool;
    pool.styles[0].setPattern("az");
    pool.setValue(5, 100);
    
    pool.clear();
    EXPECT_EQ(pool.styles[0].length, 0);
    EXPECT_EQ(pool.getValue(5), 256); // Default full bright
}
