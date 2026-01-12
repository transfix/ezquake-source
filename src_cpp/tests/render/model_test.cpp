/**
 * @file model_test.cpp
 * @brief Tests for model types
 * 
 * Part of ezQuake C to C++20 Conversion - Iteration 21
 */

#include <gtest/gtest.h>
#include "core/render/model.hpp"

using namespace ezquake::render;
using namespace ezquake::math;

// =============================================================================
// ModelType Tests
// =============================================================================

TEST(ModelTypeTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(ModelType::Brush), 0);
    EXPECT_EQ(static_cast<uint8_t>(ModelType::Sprite), 1);
    EXPECT_EQ(static_cast<uint8_t>(ModelType::Alias), 2);
    EXPECT_EQ(static_cast<uint8_t>(ModelType::Alias3), 3);
}

TEST(ModelTypeTest, Names) {
    EXPECT_EQ(modelTypeName(ModelType::Brush), "brush");
    EXPECT_EQ(modelTypeName(ModelType::Sprite), "sprite");
    EXPECT_EQ(modelTypeName(ModelType::Alias), "alias");
    EXPECT_EQ(modelTypeName(ModelType::Alias3), "alias3");
}

TEST(ModelTypeTest, IsBrushModel) {
    EXPECT_TRUE(isBrushModel(ModelType::Brush));
    EXPECT_FALSE(isBrushModel(ModelType::Sprite));
    EXPECT_FALSE(isBrushModel(ModelType::Alias));
}

TEST(ModelTypeTest, IsAliasModel) {
    EXPECT_TRUE(isAliasModel(ModelType::Alias));
    EXPECT_TRUE(isAliasModel(ModelType::Alias3));
    EXPECT_FALSE(isAliasModel(ModelType::Brush));
    EXPECT_FALSE(isAliasModel(ModelType::Sprite));
}

// =============================================================================
// ModelHint Tests
// =============================================================================

TEST(ModelHintTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(ModelHint::Normal), 0);
    EXPECT_EQ(static_cast<uint8_t>(ModelHint::Player), 1);
    EXPECT_EQ(static_cast<uint8_t>(ModelHint::Rocket), 28);
}

TEST(ModelHintTest, IsProjectile) {
    EXPECT_TRUE(isProjectile(ModelHint::Spike));
    EXPECT_TRUE(isProjectile(ModelHint::Grenade));
    EXPECT_TRUE(isProjectile(ModelHint::Rocket));
    EXPECT_TRUE(isProjectile(ModelHint::Laser));
    EXPECT_FALSE(isProjectile(ModelHint::Normal));
    EXPECT_FALSE(isProjectile(ModelHint::Player));
}

TEST(ModelHintTest, IsPowerup) {
    EXPECT_TRUE(isPowerup(ModelHint::Quad));
    EXPECT_TRUE(isPowerup(ModelHint::Pent));
    EXPECT_TRUE(isPowerup(ModelHint::Ring));
    EXPECT_TRUE(isPowerup(ModelHint::Suit));
    EXPECT_FALSE(isPowerup(ModelHint::Normal));
    EXPECT_FALSE(isPowerup(ModelHint::Rocket));
}

// =============================================================================
// SpriteType Tests
// =============================================================================

TEST(SpriteTypeTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(SpriteType::VpParallelUpright), 0);
    EXPECT_EQ(static_cast<uint8_t>(SpriteType::FacingUpright), 1);
    EXPECT_EQ(static_cast<uint8_t>(SpriteType::VpParallel), 2);
    EXPECT_EQ(static_cast<uint8_t>(SpriteType::Oriented), 3);
    EXPECT_EQ(static_cast<uint8_t>(SpriteType::VpParallelOriented), 4);
}

TEST(SpriteTypeTest, Names) {
    EXPECT_EQ(spriteTypeName(SpriteType::VpParallelUpright), "vp_parallel_upright");
    EXPECT_EQ(spriteTypeName(SpriteType::Oriented), "oriented");
}

TEST(SpriteTypeTest, IsUpright) {
    EXPECT_TRUE(isUpright(SpriteType::VpParallelUpright));
    EXPECT_TRUE(isUpright(SpriteType::FacingUpright));
    EXPECT_FALSE(isUpright(SpriteType::VpParallel));
    EXPECT_FALSE(isUpright(SpriteType::Oriented));
}

// =============================================================================
// EntityEffects Tests
// =============================================================================

TEST(EntityEffectsTest, Values) {
    EXPECT_EQ(EntityEffects::None, 0);
    EXPECT_EQ(EntityEffects::Rocket, 1);
    EXPECT_EQ(EntityEffects::Grenade, 2);
    EXPECT_EQ(EntityEffects::Gib, 4);
    EXPECT_EQ(EntityEffects::Rotate, 8);
}

TEST(EntityEffectsTest, HasTrail) {
    EXPECT_TRUE(EntityEffects::hasTrail(EntityEffects::Rocket));
    EXPECT_TRUE(EntityEffects::hasTrail(EntityEffects::Grenade));
    EXPECT_TRUE(EntityEffects::hasTrail(EntityEffects::Gib));
    EXPECT_TRUE(EntityEffects::hasTrail(EntityEffects::Tracer));
    EXPECT_FALSE(EntityEffects::hasTrail(EntityEffects::None));
    EXPECT_FALSE(EntityEffects::hasTrail(EntityEffects::Rotate));
}

TEST(EntityEffectsTest, ShouldRotate) {
    EXPECT_TRUE(EntityEffects::shouldRotate(EntityEffects::Rotate));
    EXPECT_TRUE(EntityEffects::shouldRotate(EntityEffects::Tracer2));
    EXPECT_FALSE(EntityEffects::shouldRotate(EntityEffects::None));
    EXPECT_FALSE(EntityEffects::shouldRotate(EntityEffects::Rocket));
}

// =============================================================================
// SurfaceFlags Tests
// =============================================================================

TEST(SurfaceFlagsTest, Values) {
    EXPECT_EQ(SurfaceFlags::PlaneBack, 2);
    EXPECT_EQ(SurfaceFlags::DrawSky, 4);
    EXPECT_EQ(SurfaceFlags::DrawTurb, 16);
    EXPECT_EQ(SurfaceFlags::DrawAlpha, 256);
}

TEST(SurfaceFlagsTest, IsSky) {
    EXPECT_TRUE(SurfaceFlags::isSky(SurfaceFlags::DrawSky));
    EXPECT_FALSE(SurfaceFlags::isSky(SurfaceFlags::DrawTurb));
}

TEST(SurfaceFlagsTest, IsWater) {
    EXPECT_TRUE(SurfaceFlags::isWater(SurfaceFlags::DrawTurb));
    EXPECT_FALSE(SurfaceFlags::isWater(SurfaceFlags::DrawSky));
}

TEST(SurfaceFlagsTest, NeedsSpecialRendering) {
    EXPECT_TRUE(SurfaceFlags::needsSpecialRendering(SurfaceFlags::DrawSky));
    EXPECT_TRUE(SurfaceFlags::needsSpecialRendering(SurfaceFlags::DrawTurb));
    EXPECT_TRUE(SurfaceFlags::needsSpecialRendering(SurfaceFlags::DrawAlpha));
    EXPECT_FALSE(SurfaceFlags::needsSpecialRendering(SurfaceFlags::PlaneBack));
}

// =============================================================================
// BSP Constants Tests
// =============================================================================

TEST(BspConstantsTest, Versions) {
    EXPECT_EQ(bsp_constants::Q1_VERSION, 29);
    EXPECT_EQ(bsp_constants::HL_VERSION, 30);
}

TEST(BspConstantsTest, Limits) {
    EXPECT_EQ(bsp_constants::MAX_HULLS, 4);
    EXPECT_EQ(bsp_constants::MAX_MODELS, 4096);
    EXPECT_EQ(bsp_constants::MAX_NODES, 32767);
    EXPECT_EQ(bsp_constants::MIP_LEVELS, 4);
    EXPECT_EQ(bsp_constants::MAX_LIGHTMAPS, 4);
}

// =============================================================================
// BspLump Tests
// =============================================================================

TEST(BspLumpTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(BspLump::Entities), 0);
    EXPECT_EQ(static_cast<uint8_t>(BspLump::Planes), 1);
    EXPECT_EQ(static_cast<uint8_t>(BspLump::Models), 14);
    EXPECT_EQ(static_cast<uint8_t>(BspLump::Count), 15);
}

TEST(BspLumpTest, Names) {
    EXPECT_EQ(bspLumpName(BspLump::Entities), "entities");
    EXPECT_EQ(bspLumpName(BspLump::Planes), "planes");
    EXPECT_EQ(bspLumpName(BspLump::Faces), "faces");
}

// =============================================================================
// PlaneType Tests
// =============================================================================

TEST(PlaneTypeTest, IsAxial) {
    EXPECT_TRUE(isAxialPlane(PlaneType::X));
    EXPECT_TRUE(isAxialPlane(PlaneType::Y));
    EXPECT_TRUE(isAxialPlane(PlaneType::Z));
    EXPECT_FALSE(isAxialPlane(PlaneType::AnyX));
    EXPECT_FALSE(isAxialPlane(PlaneType::AnyY));
    EXPECT_FALSE(isAxialPlane(PlaneType::AnyZ));
}

TEST(PlaneTypeTest, PlaneAxis) {
    EXPECT_EQ(planeAxis(PlaneType::X), 0);
    EXPECT_EQ(planeAxis(PlaneType::Y), 1);
    EXPECT_EQ(planeAxis(PlaneType::Z), 2);
    EXPECT_EQ(planeAxis(PlaneType::AnyX), 0);
    EXPECT_EQ(planeAxis(PlaneType::AnyY), 1);
    EXPECT_EQ(planeAxis(PlaneType::AnyZ), 2);
}

// =============================================================================
// AliasConstants Tests
// =============================================================================

TEST(AliasConstantsTest, Values) {
    EXPECT_EQ(alias_constants::VERSION, 6);
    EXPECT_EQ(alias_constants::MAX_VERTS, 2048);
    EXPECT_EQ(alias_constants::MAX_FRAMES, 256);
    EXPECT_EQ(alias_constants::MAX_TRIS, 2048);
    EXPECT_EQ(alias_constants::MAX_SKINS, 32);
}

TEST(AliasConstantsTest, Header) {
    // "IDPO" in little-endian
    EXPECT_EQ(alias_constants::HEADER, 0x4F504449u);
}

// =============================================================================
// SpriteConstants Tests
// =============================================================================

TEST(SpriteConstantsTest, Values) {
    EXPECT_EQ(sprite_constants::VERSION, 1);
    EXPECT_EQ(sprite_constants::MAX_FRAMES, 512);
}

TEST(SpriteConstantsTest, Header) {
    // "IDSP" in little-endian
    EXPECT_EQ(sprite_constants::HEADER, 0x50534449u);
}

// =============================================================================
// ModelBounds Tests
// =============================================================================

TEST(ModelBoundsTest, Default) {
    ModelBounds bounds;
    EXPECT_FLOAT_EQ(bounds.radius, 0.0f);
}

TEST(ModelBoundsTest, Center) {
    ModelBounds bounds;
    bounds.mins = Vec3(-10.0f, -20.0f, -5.0f);
    bounds.maxs = Vec3(10.0f, 20.0f, 5.0f);
    
    Vec3 center = bounds.center();
    EXPECT_FLOAT_EQ(center[0], 0.0f);
    EXPECT_FLOAT_EQ(center[1], 0.0f);
    EXPECT_FLOAT_EQ(center[2], 0.0f);
}

TEST(ModelBoundsTest, Size) {
    ModelBounds bounds;
    bounds.mins = Vec3(-10.0f, -20.0f, -5.0f);
    bounds.maxs = Vec3(10.0f, 20.0f, 5.0f);
    
    Vec3 size = bounds.size();
    EXPECT_FLOAT_EQ(size[0], 20.0f);
    EXPECT_FLOAT_EQ(size[1], 40.0f);
    EXPECT_FLOAT_EQ(size[2], 10.0f);
}

TEST(ModelBoundsTest, Contains) {
    ModelBounds bounds;
    bounds.mins = Vec3(-10.0f, -10.0f, -10.0f);
    bounds.maxs = Vec3(10.0f, 10.0f, 10.0f);
    
    EXPECT_TRUE(bounds.contains(Vec3(0.0f, 0.0f, 0.0f)));
    EXPECT_TRUE(bounds.contains(Vec3(5.0f, 5.0f, 5.0f)));
    EXPECT_FALSE(bounds.contains(Vec3(15.0f, 0.0f, 0.0f)));
}

TEST(ModelBoundsTest, Expand) {
    ModelBounds bounds;
    bounds.mins = Vec3(0.0f, 0.0f, 0.0f);
    bounds.maxs = Vec3(0.0f, 0.0f, 0.0f);
    
    bounds.expand(Vec3(10.0f, -5.0f, 20.0f));
    
    EXPECT_FLOAT_EQ(bounds.mins[1], -5.0f);
    EXPECT_FLOAT_EQ(bounds.maxs[0], 10.0f);
    EXPECT_FLOAT_EQ(bounds.maxs[2], 20.0f);
}

TEST(ModelBoundsTest, CalculateRadius) {
    ModelBounds bounds;
    bounds.mins = Vec3(-10.0f, -10.0f, -10.0f);
    bounds.maxs = Vec3(10.0f, 10.0f, 10.0f);
    
    bounds.calculateRadius();
    
    // radius = sqrt(10^2 + 10^2 + 10^2) = sqrt(300) ~= 17.32
    EXPECT_GT(bounds.radius, 17.0f);
    EXPECT_LT(bounds.radius, 18.0f);
}

// =============================================================================
// TextureVecs Tests
// =============================================================================

TEST(TextureVecsTest, Default) {
    TextureVecs vecs;
    EXPECT_FLOAT_EQ(vecs.s[0], 0.0f);
    EXPECT_FLOAT_EQ(vecs.t[0], 0.0f);
}

TEST(TextureVecsTest, CalcCoords) {
    TextureVecs vecs;
    vecs.s = {1.0f, 0.0f, 0.0f, 10.0f};
    vecs.t = {0.0f, 1.0f, 0.0f, 20.0f};
    
    Vec3 pos(5.0f, 8.0f, 0.0f);
    
    EXPECT_FLOAT_EQ(vecs.calcS(pos), 15.0f);  // 5 * 1 + 10
    EXPECT_FLOAT_EQ(vecs.calcT(pos), 28.0f);  // 8 * 1 + 20
}

// =============================================================================
// BspPlane Tests
// =============================================================================

TEST(BspPlaneTest, Default) {
    BspPlane plane;
    EXPECT_FLOAT_EQ(plane.dist, 0.0f);
    EXPECT_EQ(plane.type, PlaneType::X);
}

TEST(BspPlaneTest, DistanceToPoint) {
    BspPlane plane;
    plane.normal = Vec3(0.0f, 0.0f, 1.0f);
    plane.dist = 10.0f;
    
    EXPECT_FLOAT_EQ(plane.distanceToPoint(Vec3(0.0f, 0.0f, 15.0f)), 5.0f);
    EXPECT_FLOAT_EQ(plane.distanceToPoint(Vec3(0.0f, 0.0f, 5.0f)), -5.0f);
}

TEST(BspPlaneTest, IsInFront) {
    BspPlane plane;
    plane.normal = Vec3(0.0f, 0.0f, 1.0f);
    plane.dist = 10.0f;
    
    EXPECT_TRUE(plane.isInFront(Vec3(0.0f, 0.0f, 15.0f)));
    EXPECT_FALSE(plane.isInFront(Vec3(0.0f, 0.0f, 5.0f)));
}

TEST(BspPlaneTest, IsAxial) {
    BspPlane plane;
    plane.type = PlaneType::X;
    EXPECT_TRUE(plane.isAxial());
    
    plane.type = PlaneType::AnyX;
    EXPECT_FALSE(plane.isAxial());
}

// =============================================================================
// WorldspawnInfo Tests
// =============================================================================

TEST(WorldspawnInfoTest, Default) {
    WorldspawnInfo info;
    EXPECT_TRUE(info.skyboxName.empty());
    EXPECT_FLOAT_EQ(info.fogDensity, 0.0f);
    EXPECT_FLOAT_EQ(info.waterAlpha, 1.0f);
}

TEST(WorldspawnInfoTest, HasFog) {
    WorldspawnInfo info;
    EXPECT_FALSE(info.hasFog());
    
    info.fogDensity = 0.05f;
    EXPECT_TRUE(info.hasFog());
}

TEST(WorldspawnInfoTest, HasSkybox) {
    WorldspawnInfo info;
    EXPECT_FALSE(info.hasSkybox());
    
    info.skyboxName = "night";
    EXPECT_TRUE(info.hasSkybox());
}

TEST(WorldspawnInfoTest, Clear) {
    WorldspawnInfo info;
    info.skyboxName = "test";
    info.fogDensity = 0.1f;
    info.waterAlpha = 0.5f;
    
    info.clear();
    
    EXPECT_TRUE(info.skyboxName.empty());
    EXPECT_FLOAT_EQ(info.fogDensity, 0.0f);
    EXPECT_FLOAT_EQ(info.waterAlpha, 1.0f);
}

// =============================================================================
// SpriteFrame Tests
// =============================================================================

TEST(SpriteFrameTest, Default) {
    SpriteFrame frame;
    EXPECT_EQ(frame.width, 0);
    EXPECT_EQ(frame.height, 0);
    EXPECT_FALSE(frame.isValid());
}

TEST(SpriteFrameTest, IsValid) {
    SpriteFrame frame;
    frame.width = 64;
    frame.height = 64;
    EXPECT_TRUE(frame.isValid());
}

TEST(SpriteFrameTest, AspectRatio) {
    SpriteFrame frame;
    frame.width = 128;
    frame.height = 64;
    EXPECT_FLOAT_EQ(frame.aspectRatio(), 2.0f);
}

// =============================================================================
// AliasFrameDesc Tests
// =============================================================================

TEST(AliasFrameDescTest, Default) {
    AliasFrameDesc desc;
    EXPECT_EQ(desc.firstPose, 0);
    EXPECT_EQ(desc.numPoses, 1);
    EXPECT_FLOAT_EQ(desc.interval, 0.1f);
    EXPECT_FALSE(desc.isGroup());
}

TEST(AliasFrameDescTest, IsGroup) {
    AliasFrameDesc desc;
    desc.numPoses = 5;
    EXPECT_TRUE(desc.isGroup());
}

// =============================================================================
// Trivertex Tests
// =============================================================================

TEST(TrivertexTest, Default) {
    Trivertex vert;
    EXPECT_EQ(vert.v[0], 0);
    EXPECT_EQ(vert.v[1], 0);
    EXPECT_EQ(vert.v[2], 0);
    EXPECT_EQ(vert.lightNormalIndex, 0);
}

TEST(TrivertexTest, Decompress) {
    Trivertex vert;
    vert.v[0] = 10;
    vert.v[1] = 20;
    vert.v[2] = 30;
    
    Vec3 scale(0.5f, 0.5f, 0.5f);
    Vec3 origin(100.0f, 200.0f, 300.0f);
    
    Vec3 result = vert.decompress(scale, origin);
    
    EXPECT_FLOAT_EQ(result[0], 105.0f);   // 10 * 0.5 + 100
    EXPECT_FLOAT_EQ(result[1], 210.0f);   // 20 * 0.5 + 200
    EXPECT_FLOAT_EQ(result[2], 315.0f);   // 30 * 0.5 + 300
}

// =============================================================================
// CustomModelId Tests
// =============================================================================

TEST(CustomModelIdTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(CustomModelId::Explosion), 0);
    EXPECT_EQ(static_cast<uint8_t>(CustomModelId::Bolt), 1);
    EXPECT_EQ(static_cast<uint8_t>(CustomModelId::Count), 6);
}

TEST(CustomModelIdTest, Names) {
    EXPECT_EQ(customModelName(CustomModelId::Explosion), "explosion");
    EXPECT_EQ(customModelName(CustomModelId::Bolt), "bolt");
    EXPECT_EQ(customModelName(CustomModelId::Beam), "beam");
}
