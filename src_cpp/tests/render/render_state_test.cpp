/**
 * @file render_state_test.cpp
 * @brief Tests for render state types
 * 
 * Part of ezQuake C to C++20 Conversion - Iteration 23
 */

#include <gtest/gtest.h>
#include "core/render/render_state_types.hpp"

using namespace ezquake::render;

// =============================================================================
// DepthFunc Tests
// =============================================================================

TEST(DepthFuncTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(DepthFunc::Less), 0);
    EXPECT_EQ(static_cast<uint8_t>(DepthFunc::Equal), 1);
    EXPECT_EQ(static_cast<uint8_t>(DepthFunc::LessOrEqual), 2);
    EXPECT_EQ(static_cast<uint8_t>(DepthFunc::Count), 3);
}

TEST(DepthFuncTest, Names) {
    EXPECT_EQ(depthFuncName(DepthFunc::Less), "less");
    EXPECT_EQ(depthFuncName(DepthFunc::Equal), "equal");
    EXPECT_EQ(depthFuncName(DepthFunc::LessOrEqual), "less_or_equal");
}

// =============================================================================
// CullFace Tests
// =============================================================================

TEST(CullFaceTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(CullFace::Front), 0);
    EXPECT_EQ(static_cast<uint8_t>(CullFace::Back), 1);
}

TEST(CullFaceTest, Names) {
    EXPECT_EQ(cullFaceName(CullFace::Front), "front");
    EXPECT_EQ(cullFaceName(CullFace::Back), "back");
}

// =============================================================================
// BlendFunc Tests
// =============================================================================

TEST(BlendFuncTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(BlendFunc::Overwrite), 0);
    EXPECT_EQ(static_cast<uint8_t>(BlendFunc::AdditiveBlending), 1);
    EXPECT_EQ(static_cast<uint8_t>(BlendFunc::PremultipliedAlpha), 2);
}

TEST(BlendFuncTest, Names) {
    EXPECT_EQ(blendFuncName(BlendFunc::Overwrite), "overwrite");
    EXPECT_EQ(blendFuncName(BlendFunc::AdditiveBlending), "additive");
    EXPECT_EQ(blendFuncName(BlendFunc::PremultipliedAlpha), "premultiplied_alpha");
}

TEST(BlendFuncTest, IsAdditive) {
    EXPECT_TRUE(isAdditiveBlend(BlendFunc::AdditiveBlending));
    EXPECT_TRUE(isAdditiveBlend(BlendFunc::SrcDstColorDestOne));
    EXPECT_FALSE(isAdditiveBlend(BlendFunc::Overwrite));
    EXPECT_FALSE(isAdditiveBlend(BlendFunc::PremultipliedAlpha));
}

// =============================================================================
// PolygonMode Tests
// =============================================================================

TEST(PolygonModeTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(PolygonMode::Fill), 0);
    EXPECT_EQ(static_cast<uint8_t>(PolygonMode::Line), 1);
}

// =============================================================================
// AlphaTestFunc Tests
// =============================================================================

TEST(AlphaTestFuncTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(AlphaTestFunc::Always), 0);
    EXPECT_EQ(static_cast<uint8_t>(AlphaTestFunc::Greater), 1);
}

// =============================================================================
// TexUnitMode Tests
// =============================================================================

TEST(TexUnitModeTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(TexUnitMode::Blend), 0);
    EXPECT_EQ(static_cast<uint8_t>(TexUnitMode::Replace), 1);
    EXPECT_EQ(static_cast<uint8_t>(TexUnitMode::Modulate), 2);
    EXPECT_EQ(static_cast<uint8_t>(TexUnitMode::Decal), 3);
    EXPECT_EQ(static_cast<uint8_t>(TexUnitMode::Add), 4);
}

TEST(TexUnitModeTest, Names) {
    EXPECT_EQ(texUnitModeName(TexUnitMode::Blend), "blend");
    EXPECT_EQ(texUnitModeName(TexUnitMode::Modulate), "modulate");
}

// =============================================================================
// BufferType Tests
// =============================================================================

TEST(BufferTypeTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(BufferType::Unknown), 0);
    EXPECT_EQ(static_cast<uint8_t>(BufferType::Vertex), 1);
    EXPECT_EQ(static_cast<uint8_t>(BufferType::Index), 2);
    EXPECT_EQ(static_cast<uint8_t>(BufferType::Storage), 3);
    EXPECT_EQ(static_cast<uint8_t>(BufferType::Uniform), 4);
    EXPECT_EQ(static_cast<uint8_t>(BufferType::Indirect), 5);
}

TEST(BufferTypeTest, Names) {
    EXPECT_EQ(bufferTypeName(BufferType::Vertex), "vertex");
    EXPECT_EQ(bufferTypeName(BufferType::Index), "index");
    EXPECT_EQ(bufferTypeName(BufferType::Storage), "storage");
}

// =============================================================================
// BufferUsage Tests
// =============================================================================

TEST(BufferUsageTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(BufferUsage::Unknown), 0);
    EXPECT_EQ(static_cast<uint8_t>(BufferUsage::OncePerFrame), 1);
    EXPECT_EQ(static_cast<uint8_t>(BufferUsage::ConstantData), 4);
}

TEST(BufferUsageTest, Names) {
    EXPECT_EQ(bufferUsageName(BufferUsage::OncePerFrame), "stream");
    EXPECT_EQ(bufferUsageName(BufferUsage::ConstantData), "static");
}

TEST(BufferUsageTest, ShouldDoubleBuffer) {
    EXPECT_TRUE(shouldDoubleBuffer(BufferUsage::OncePerFrame));
    EXPECT_TRUE(shouldDoubleBuffer(BufferUsage::ReusePerFrame));
    EXPECT_FALSE(shouldDoubleBuffer(BufferUsage::ReuseManyFrames));
    EXPECT_FALSE(shouldDoubleBuffer(BufferUsage::ConstantData));
}

// =============================================================================
// BufferId Tests
// =============================================================================

TEST(BufferIdTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(BufferId::None), 0);
    EXPECT_NE(static_cast<uint8_t>(BufferId::AliasModelVertex), 0);
    EXPECT_NE(static_cast<uint8_t>(BufferId::BrushModelVertex), 0);
}

// =============================================================================
// VaoId Tests
// =============================================================================

TEST(VaoIdTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(VaoId::None), 0);
    EXPECT_NE(static_cast<uint8_t>(VaoId::AliasModel), 0);
    EXPECT_NE(static_cast<uint8_t>(VaoId::BrushModel), 0);
}

TEST(VaoIdTest, Names) {
    EXPECT_EQ(vaoName(VaoId::AliasModel), "aliasmodel");
    EXPECT_EQ(vaoName(VaoId::BrushModel), "brushmodel");
    EXPECT_EQ(vaoName(VaoId::PostProcess), "postprocess");
}

// =============================================================================
// RenderStateId Tests
// =============================================================================

TEST(RenderStateIdTest, Values) {
    EXPECT_EQ(static_cast<uint16_t>(RenderStateId::Null), 0);
    EXPECT_NE(static_cast<uint16_t>(RenderStateId::Default3D), 0);
    EXPECT_NE(static_cast<uint16_t>(RenderStateId::Default2D), 0);
}

TEST(RenderStateIdTest, Count) {
    // Should have many render states
    EXPECT_GT(static_cast<uint16_t>(RenderStateId::Count), 50);
}

// =============================================================================
// Viewport Tests
// =============================================================================

TEST(ViewportTest, Default) {
    Viewport vp;
    EXPECT_FALSE(vp.isValid());
    EXPECT_EQ(vp.x, 0);
    EXPECT_EQ(vp.y, 0);
}

TEST(ViewportTest, IsValid) {
    Viewport vp{0, 0, 800, 600};
    EXPECT_TRUE(vp.isValid());
    
    vp.width = 0;
    EXPECT_FALSE(vp.isValid());
}

TEST(ViewportTest, AspectRatio) {
    Viewport vp{0, 0, 1920, 1080};
    EXPECT_NEAR(vp.aspectRatio(), 16.0f / 9.0f, 0.01f);
}

TEST(ViewportTest, Contains) {
    Viewport vp{100, 100, 200, 200};
    
    EXPECT_TRUE(vp.contains(150, 150));
    EXPECT_TRUE(vp.contains(100, 100));
    EXPECT_FALSE(vp.contains(50, 150));
    EXPECT_FALSE(vp.contains(350, 150));
}

TEST(ViewportTest, Equality) {
    Viewport a{0, 0, 800, 600};
    Viewport b{0, 0, 800, 600};
    Viewport c{0, 0, 1024, 768};
    
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
}

// =============================================================================
// DepthState Tests
// =============================================================================

TEST(DepthStateTest, Default) {
    DepthState ds;
    EXPECT_EQ(ds.func, DepthFunc::Less);
    EXPECT_TRUE(ds.testEnabled);
    EXPECT_TRUE(ds.maskEnabled);
    EXPECT_TRUE(ds.isDefault());
}

TEST(DepthStateTest, IsDefault) {
    DepthState ds;
    EXPECT_TRUE(ds.isDefault());
    
    ds.func = DepthFunc::Equal;
    EXPECT_FALSE(ds.isDefault());
}

// =============================================================================
// LineState Tests
// =============================================================================

TEST(LineStateTest, Default) {
    LineState ls;
    EXPECT_FLOAT_EQ(ls.width, 1.0f);
    EXPECT_FALSE(ls.smooth);
    EXPECT_TRUE(ls.isDefault());
}

TEST(LineStateTest, IsDefault) {
    LineState ls;
    EXPECT_TRUE(ls.isDefault());
    
    ls.width = 2.0f;
    EXPECT_FALSE(ls.isDefault());
}

// =============================================================================
// FogState Tests
// =============================================================================

TEST(FogStateTest, Default) {
    FogState fs;
    EXPECT_EQ(fs.mode, FogMode::Disabled);
    EXPECT_FALSE(fs.isEnabled());
}

TEST(FogStateTest, Enable) {
    FogState fs;
    fs.enable(0.5f, 0.5f, 0.5f, 0.02f);
    
    EXPECT_EQ(fs.mode, FogMode::Enabled);
    EXPECT_FLOAT_EQ(fs.color[0], 0.5f);
    EXPECT_FLOAT_EQ(fs.density, 0.02f);
    EXPECT_TRUE(fs.isEnabled());
}

TEST(FogStateTest, Disable) {
    FogState fs;
    fs.enable(0.5f, 0.5f, 0.5f, 0.02f);
    fs.disable();
    
    EXPECT_EQ(fs.mode, FogMode::Disabled);
    EXPECT_FALSE(fs.isEnabled());
}

// =============================================================================
// CullState Tests
// =============================================================================

TEST(CullStateTest, Default) {
    CullState cs;
    EXPECT_EQ(cs.mode, CullFace::Back);
    EXPECT_TRUE(cs.enabled);
}

TEST(CullStateTest, EnableDisable) {
    CullState cs;
    cs.disable();
    EXPECT_FALSE(cs.enabled);
    
    cs.enable(CullFace::Front);
    EXPECT_TRUE(cs.enabled);
    EXPECT_EQ(cs.mode, CullFace::Front);
}

// =============================================================================
// ColorMask Tests
// =============================================================================

TEST(ColorMaskTest, Default) {
    ColorMask cm;
    EXPECT_TRUE(cm.r);
    EXPECT_TRUE(cm.g);
    EXPECT_TRUE(cm.b);
    EXPECT_TRUE(cm.a);
    EXPECT_TRUE(cm.allEnabled());
    EXPECT_FALSE(cm.allDisabled());
}

TEST(ColorMaskTest, Presets) {
    auto all = ColorMask::all();
    EXPECT_TRUE(all.allEnabled());
    
    auto none = ColorMask::none();
    EXPECT_TRUE(none.allDisabled());
    
    auto rgbOnly = ColorMask::rgbOnly();
    EXPECT_TRUE(rgbOnly.r && rgbOnly.g && rgbOnly.b);
    EXPECT_FALSE(rgbOnly.a);
    
    auto alphaOnly = ColorMask::alphaOnly();
    EXPECT_FALSE(alphaOnly.r || alphaOnly.g || alphaOnly.b);
    EXPECT_TRUE(alphaOnly.a);
}

// =============================================================================
// ClearColor Tests
// =============================================================================

TEST(ClearColorTest, Default) {
    ClearColor cc;
    EXPECT_TRUE(cc.isBlack());
}

TEST(ClearColorTest, Presets) {
    auto black = ClearColor::black();
    EXPECT_TRUE(black.isBlack());
    
    auto white = ClearColor::white();
    EXPECT_FLOAT_EQ(white.r, 1.0f);
    EXPECT_FLOAT_EQ(white.g, 1.0f);
    EXPECT_FLOAT_EQ(white.b, 1.0f);
    
    auto gray = ClearColor::gray();
    EXPECT_FLOAT_EQ(gray.r, 0.5f);
}

// =============================================================================
// BufferRef Tests
// =============================================================================

TEST(BufferRefTest, Default) {
    BufferRef ref;
    EXPECT_FALSE(ref.isValid());
    EXPECT_TRUE(ref.isEmpty());
}

TEST(BufferRefTest, IsValid) {
    BufferRef ref;
    ref.id = BufferId::AliasModelVertex;
    ref.size = 1024;
    
    EXPECT_TRUE(ref.isValid());
    EXPECT_FALSE(ref.isEmpty());
}

// =============================================================================
// RenderLimits Tests
// =============================================================================

TEST(RenderLimitsTest, Values) {
    EXPECT_EQ(render_limits::MAX_TEXTURE_UNITS, 4);
    EXPECT_EQ(render_limits::MAX_ATTRIBUTES, 16);
    EXPECT_EQ(render_limits::MAX_LIGHTMAPS, 4);
    EXPECT_EQ(render_limits::MAX_DLIGHTS, 32);
    EXPECT_EQ(render_limits::MAX_BATCH_SIZE, 65536);
}

// =============================================================================
// DLightType Tests
// =============================================================================

TEST(DLightTypeTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(DLightType::Default), 0);
    EXPECT_EQ(static_cast<uint8_t>(DLightType::Red), 1);
    EXPECT_EQ(static_cast<uint8_t>(DLightType::Blue), 2);
}

TEST(DLightTypeTest, Colors) {
    auto red = dlightColor(DLightType::Red);
    EXPECT_EQ(red[0], 255);
    EXPECT_LT(red[1], 100);
    
    auto blue = dlightColor(DLightType::Blue);
    EXPECT_LT(blue[0], 100);
    EXPECT_EQ(blue[2], 255);
    
    auto def = dlightColor(DLightType::Default);
    EXPECT_EQ(def[0], 255);  // Warm white
}
