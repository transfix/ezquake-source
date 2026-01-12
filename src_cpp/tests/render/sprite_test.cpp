/**
 * @file sprite_test.cpp
 * @brief Tests for sprite, billboard, and sky rendering types
 */

#include <gtest/gtest.h>
#include "core/render/sprite_types.hpp"

using namespace ezquake::render;

// =============================================================================
// Sprite Limits Tests
// =============================================================================

TEST(SpriteLimitsTest, Constants) {
    EXPECT_EQ(sprite_limits::MAX_SPRITES_PER_BATCH, 16384);
    EXPECT_EQ(sprite_limits::MAX_VERTS_PER_BATCH, 65536);
    EXPECT_EQ(sprite_limits::INDEXES_MAX_QUADS, 512);
    EXPECT_EQ(sprite_limits::MAX_SKYBOX_TEXTURES, 6);
}

TEST(SpriteLimitsTest, MagicNumbers) {
    EXPECT_EQ(sprite_limits::ID_SPRITE_HEADER, 0x50534449);
    EXPECT_EQ(sprite_limits::ID_POLY_HEADER, 0x4F504449);
}

TEST(SpriteLimitsTest, AliasConstants) {
    EXPECT_EQ(sprite_limits::SPRITE_VERSION, 1);
    EXPECT_EQ(sprite_limits::ALIAS_VERSION, 6);
    EXPECT_EQ(sprite_limits::ALIAS_ONSEAM, 0x0020);
}

// =============================================================================
// Sync Type Tests
// =============================================================================

TEST(SyncTypeTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(SyncType::Sync), 0);
    EXPECT_EQ(static_cast<uint8_t>(SyncType::Random), 1);
}

// =============================================================================
// Sprite Orientation Tests
// =============================================================================

TEST(SpriteOrientationTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(SpriteOrientation::VpParallelUpright), 0);
    EXPECT_EQ(static_cast<uint8_t>(SpriteOrientation::FacingUpright), 1);
    EXPECT_EQ(static_cast<uint8_t>(SpriteOrientation::VpParallel), 2);
    EXPECT_EQ(static_cast<uint8_t>(SpriteOrientation::Oriented), 3);
    EXPECT_EQ(static_cast<uint8_t>(SpriteOrientation::VpParallelOriented), 4);
}

TEST(SpriteOrientationTest, Names) {
    EXPECT_STREQ(getSpriteOrientationName(SpriteOrientation::VpParallel), "VpParallel");
    EXPECT_STREQ(getSpriteOrientationName(SpriteOrientation::Oriented), "Oriented");
}

// =============================================================================
// Sprite Frame Type Tests
// =============================================================================

TEST(SpriteFrameTypeTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(SpriteFrameType::Single), 0);
    EXPECT_EQ(static_cast<uint8_t>(SpriteFrameType::Group), 1);
}

// =============================================================================
// Primitive Type Tests
// =============================================================================

TEST(PrimitiveTypeTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(PrimitiveType::TriangleStrip), 0);
    EXPECT_EQ(static_cast<uint8_t>(PrimitiveType::TriangleFan), 1);
    EXPECT_EQ(static_cast<uint8_t>(PrimitiveType::Triangles), 2);
}

TEST(PrimitiveTypeTest, Names) {
    EXPECT_STREQ(getPrimitiveTypeName(PrimitiveType::Triangles), "Triangles");
    EXPECT_STREQ(getPrimitiveTypeName(PrimitiveType::TriangleStrip), "TriangleStrip");
}

// =============================================================================
// Sprite 3D Batch ID Tests
// =============================================================================

TEST(Sprite3DBatchIdTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(Sprite3DBatchId::Entities), 0);
    EXPECT_EQ(static_cast<uint8_t>(Sprite3DBatchId::ParticlesClassic), 1);
    EXPECT_EQ(static_cast<uint8_t>(Sprite3DBatchId::FlashblendLights), 48);
}

TEST(Sprite3DBatchIdTest, IsParticleBatch) {
    EXPECT_FALSE(isParticleBatch(Sprite3DBatchId::Entities));
    EXPECT_TRUE(isParticleBatch(Sprite3DBatchId::ParticlesClassic));
    EXPECT_TRUE(isParticleBatch(Sprite3DBatchId::ParticleSpark));
    EXPECT_FALSE(isParticleBatch(Sprite3DBatchId::FlashblendLights));
}

TEST(Sprite3DBatchIdTest, IsQMBParticleBatch) {
    EXPECT_FALSE(isQMBParticleBatch(Sprite3DBatchId::ParticlesClassic));
    EXPECT_TRUE(isQMBParticleBatch(Sprite3DBatchId::ParticleSpark));
    EXPECT_TRUE(isQMBParticleBatch(Sprite3DBatchId::ParticleBlood1));
}

TEST(Sprite3DBatchIdTest, IsCoronaBatch) {
    EXPECT_FALSE(isCoronaBatch(Sprite3DBatchId::FlashblendLights));
    EXPECT_TRUE(isCoronaBatch(Sprite3DBatchId::CoronaStandard));
    EXPECT_TRUE(isCoronaBatch(Sprite3DBatchId::CoronaExplosionFlash7));
}

// =============================================================================
// Sprite Vertex Pool Tests
// =============================================================================

TEST(SpriteVertPoolIdTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(SpriteVertPoolId::GameEntities), 0);
    EXPECT_EQ(static_cast<uint8_t>(SpriteVertPoolId::Count), 5);
}

// =============================================================================
// Sprite 3D Vertex Tests
// =============================================================================

TEST(Sprite3DVertexTest, Size) {
    EXPECT_EQ(sizeof(Sprite3DVertex), 32);
}

TEST(Sprite3DVertexTest, SetPosition) {
    Sprite3DVertex vert;
    vert.setPosition(1.0f, 2.0f, 3.0f);
    
    EXPECT_FLOAT_EQ(vert.x(), 1.0f);
    EXPECT_FLOAT_EQ(vert.y(), 2.0f);
    EXPECT_FLOAT_EQ(vert.z(), 3.0f);
}

TEST(Sprite3DVertexTest, SetTexCoord) {
    Sprite3DVertex vert;
    vert.setTexCoord(0.5f, 0.75f, 2.0f);
    
    EXPECT_FLOAT_EQ(vert.u(), 0.5f);
    EXPECT_FLOAT_EQ(vert.v(), 0.75f);
    EXPECT_FLOAT_EQ(vert.layer(), 2.0f);
}

TEST(Sprite3DVertexTest, SetColor) {
    Sprite3DVertex vert;
    vert.setColor(128, 64, 32, 255);
    
    EXPECT_EQ(vert.color[0], 128);
    EXPECT_EQ(vert.color[1], 64);
    EXPECT_EQ(vert.color[2], 32);
    EXPECT_EQ(vert.color[3], 255);
}

// =============================================================================
// Sprite Header Tests
// =============================================================================

TEST(SpriteHeaderTest, IsValid) {
    SpriteHeader header;
    EXPECT_FALSE(header.isValid());
    
    header.ident = sprite_limits::ID_SPRITE_HEADER;
    header.version = sprite_limits::SPRITE_VERSION;
    EXPECT_TRUE(header.isValid());
}

TEST(SpriteHeaderTest, TotalPixels) {
    SpriteHeader header;
    header.width = 64;
    header.height = 64;
    
    EXPECT_EQ(header.totalPixels(), 4096);
}

TEST(SpriteHeaderTest, Orientation) {
    SpriteHeader header;
    header.orientationType = 2;
    
    EXPECT_EQ(header.orientation(), SpriteOrientation::VpParallel);
}

// =============================================================================
// Sprite Frame Header Tests
// =============================================================================

TEST(SpriteFrameHeaderTest, DataSize) {
    SpriteFrameHeader frame;
    frame.width = 32;
    frame.height = 32;
    
    EXPECT_EQ(frame.dataSize(), 1024);
}

TEST(SpriteFrameHeaderTest, Origin) {
    SpriteFrameHeader frame;
    frame.origin = {-16, -16};
    
    EXPECT_EQ(frame.originX(), -16);
    EXPECT_EQ(frame.originY(), -16);
}

// =============================================================================
// Alias Model Header Tests
// =============================================================================

TEST(AliasModelHeaderTest, IsValid) {
    AliasModelHeader header;
    EXPECT_FALSE(header.isValid());
    
    header.ident = sprite_limits::ID_POLY_HEADER;
    header.version = sprite_limits::ALIAS_VERSION;
    EXPECT_TRUE(header.isValid());
}

TEST(AliasModelHeaderTest, SkinPixels) {
    AliasModelHeader header;
    header.skinWidth = 320;
    header.skinHeight = 200;
    header.numSkins = 4;
    
    EXPECT_EQ(header.skinPixels(), 64000);
    EXPECT_EQ(header.totalSkinBytes(), 256000);
}

// =============================================================================
// Tri Vertex Tests
// =============================================================================

TEST(TriVertexTest, Size) {
    EXPECT_EQ(sizeof(TriVertex), 4);
}

TEST(TriVertexTest, Accessors) {
    TriVertex vert;
    vert.position = {10, 20, 30};
    vert.lightNormalIndex = 42;
    
    EXPECT_EQ(vert.x(), 10);
    EXPECT_EQ(vert.y(), 20);
    EXPECT_EQ(vert.z(), 30);
    EXPECT_EQ(vert.lightNormalIndex, 42);
}

// =============================================================================
// Alias Tex Coord Tests
// =============================================================================

TEST(AliasTexCoordTest, IsOnSeam) {
    AliasTexCoord coord;
    EXPECT_FALSE(coord.isOnSeam());
    
    coord.onSeam = 1;
    EXPECT_TRUE(coord.isOnSeam());
}

// =============================================================================
// Alias Triangle Tests
// =============================================================================

TEST(AliasTriangleTest, IsFrontFacing) {
    AliasTriangle tri;
    EXPECT_FALSE(tri.isFrontFacing());
    
    tri.facesFront = sprite_limits::DT_FACES_FRONT;
    EXPECT_TRUE(tri.isFrontFacing());
}

TEST(AliasTriangleTest, VertexIndices) {
    AliasTriangle tri;
    tri.vertIndex = {10, 20, 30};
    
    EXPECT_EQ(tri.vertex0(), 10);
    EXPECT_EQ(tri.vertex1(), 20);
    EXPECT_EQ(tri.vertex2(), 30);
}

// =============================================================================
// Alias Frame Header Tests
// =============================================================================

TEST(AliasFrameHeaderTest, FrameName) {
    AliasFrameHeader frame;
    std::copy_n("walk01", 6, frame.name.begin());
    frame.name[6] = '\0';
    
    EXPECT_EQ(frame.frameName(), "walk01");
}

// =============================================================================
// Skybox Face Tests
// =============================================================================

TEST(SkyboxFaceTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(SkyboxFace::Right), 0);
    EXPECT_EQ(static_cast<uint8_t>(SkyboxFace::Back), 5);
}

TEST(SkyboxFaceTest, Suffixes) {
    EXPECT_STREQ(getSkyboxFaceSuffix(SkyboxFace::Right), "rt");
    EXPECT_STREQ(getSkyboxFaceSuffix(SkyboxFace::Left), "lf");
    EXPECT_STREQ(getSkyboxFaceSuffix(SkyboxFace::Up), "up");
    EXPECT_STREQ(getSkyboxFaceSuffix(SkyboxFace::Front), "ft");
}

// =============================================================================
// Sky State Tests
// =============================================================================

TEST(SkyStateTest, HasSkybox) {
    SkyState sky;
    EXPECT_FALSE(sky.hasSkybox());
    
    sky.skyboxLoaded = true;
    sky.skyboxTextures[0] = 1;
    EXPECT_TRUE(sky.hasSkybox());
}

TEST(SkyStateTest, HasScrollingSky) {
    SkyState sky;
    EXPECT_FALSE(sky.hasScrollingSky());
    
    sky.solidSkyTexture = 1;
    EXPECT_TRUE(sky.hasScrollingSky());
}

// =============================================================================
// Sky Wind Tests
// =============================================================================

TEST(SkyWindTest, IsActive) {
    SkyWind wind;
    EXPECT_FALSE(wind.isActive());
    
    wind.active = true;
    EXPECT_TRUE(wind.isActive());
}

// =============================================================================
// Sprite Batch Info Tests
// =============================================================================

TEST(SpriteBatchInfoTest, IsEmpty) {
    SpriteBatchInfo batch;
    EXPECT_TRUE(batch.isEmpty());
    
    batch.count = 10;
    EXPECT_FALSE(batch.isEmpty());
}

TEST(SpriteBatchInfoTest, VertexCount) {
    SpriteBatchInfo batch;
    batch.count = 100;
    
    EXPECT_EQ(batch.vertexCount(), 400);
}

// =============================================================================
// Sprite Render Stats Tests
// =============================================================================

TEST(SpriteRenderStatsTest, Reset) {
    SpriteRenderStats stats;
    stats.totalBatches = 10;
    stats.totalSprites = 100;
    
    stats.reset();
    
    EXPECT_EQ(stats.totalBatches, 0);
    EXPECT_EQ(stats.totalSprites, 0);
}

TEST(SpriteRenderStatsTest, AddBatch) {
    SpriteRenderStats stats;
    stats.addBatch(50, 200);
    stats.addBatch(30, 120);
    
    EXPECT_EQ(stats.totalBatches, 2);
    EXPECT_EQ(stats.totalSprites, 80);
    EXPECT_EQ(stats.totalVertices, 320);
    EXPECT_EQ(stats.drawCalls, 2);
}
