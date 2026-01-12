/**
 * @file particle_test.cpp
 * @brief Tests for particle types
 * 
 * Part of ezQuake C to C++20 Conversion - Iteration 24
 */

#include <gtest/gtest.h>
#include "core/render/particle_types.hpp"

using namespace ezquake::render;

// =============================================================================
// ParticleType Tests
// =============================================================================

TEST(ParticleTypeTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(ParticleType::Spark), 0);
    EXPECT_EQ(static_cast<uint8_t>(ParticleType::Smoke), 1);
    EXPECT_EQ(static_cast<uint8_t>(ParticleType::Fire), 2);
    EXPECT_GT(static_cast<uint8_t>(ParticleType::Count), 40);
}

TEST(ParticleTypeTest, Names) {
    EXPECT_EQ(particleTypeName(ParticleType::Spark), "spark");
    EXPECT_EQ(particleTypeName(ParticleType::Smoke), "smoke");
    EXPECT_EQ(particleTypeName(ParticleType::Fire), "fire");
    EXPECT_EQ(particleTypeName(ParticleType::Blood1), "blood1");
}

TEST(ParticleTypeTest, IsBlood) {
    EXPECT_TRUE(isBloodParticle(ParticleType::Blood1));
    EXPECT_TRUE(isBloodParticle(ParticleType::Blood2));
    EXPECT_TRUE(isBloodParticle(ParticleType::Blood3));
    EXPECT_TRUE(isBloodParticle(ParticleType::VXBlood));
    EXPECT_TRUE(isBloodParticle(ParticleType::BloodCloud));
    EXPECT_FALSE(isBloodParticle(ParticleType::Spark));
    EXPECT_FALSE(isBloodParticle(ParticleType::Smoke));
}

TEST(ParticleTypeTest, IsTrail) {
    EXPECT_TRUE(isTrailParticle(ParticleType::InfernoTrail));
    EXPECT_TRUE(isTrailParticle(ParticleType::RailTrail));
    EXPECT_TRUE(isTrailParticle(ParticleType::NailTrail));
    EXPECT_FALSE(isTrailParticle(ParticleType::Spark));
    EXPECT_FALSE(isTrailParticle(ParticleType::Bubble));
}

// =============================================================================
// ParticleMove Tests
// =============================================================================

TEST(ParticleMoveTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(ParticleMove::Static), 0);
    EXPECT_EQ(static_cast<uint8_t>(ParticleMove::Normal), 1);
    EXPECT_EQ(static_cast<uint8_t>(ParticleMove::Bounce), 2);
}

TEST(ParticleMoveTest, Names) {
    EXPECT_EQ(particleMoveName(ParticleMove::Static), "static");
    EXPECT_EQ(particleMoveName(ParticleMove::Normal), "normal");
    EXPECT_EQ(particleMoveName(ParticleMove::Bounce), "bounce");
}

TEST(ParticleMoveTest, UsesPhysics) {
    EXPECT_FALSE(usesPhysics(ParticleMove::Static));
    EXPECT_FALSE(usesPhysics(ParticleMove::NoPhysics));
    EXPECT_TRUE(usesPhysics(ParticleMove::Normal));
    EXPECT_TRUE(usesPhysics(ParticleMove::Bounce));
    EXPECT_TRUE(usesPhysics(ParticleMove::Float));
}

// =============================================================================
// ParticleTexture Tests
// =============================================================================

TEST(ParticleTextureTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(ParticleTexture::None), 0);
    EXPECT_EQ(static_cast<uint8_t>(ParticleTexture::Smoke), 1);
    EXPECT_EQ(static_cast<uint8_t>(ParticleTexture::Bubble), 2);
}

TEST(ParticleTextureTest, Names) {
    EXPECT_EQ(particleTextureName(ParticleTexture::None), "none");
    EXPECT_EQ(particleTextureName(ParticleTexture::Smoke), "smoke");
    EXPECT_EQ(particleTextureName(ParticleTexture::Spark), "spark");
}

// =============================================================================
// ParticleDraw Tests
// =============================================================================

TEST(ParticleDrawTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(ParticleDraw::Spark), 0);
    EXPECT_EQ(static_cast<uint8_t>(ParticleDraw::Billboard), 2);
}

TEST(ParticleDrawTest, Names) {
    EXPECT_EQ(particleDrawName(ParticleDraw::Spark), "spark");
    EXPECT_EQ(particleDrawName(ParticleDraw::Billboard), "billboard");
    EXPECT_EQ(particleDrawName(ParticleDraw::Torch), "torch");
}

TEST(ParticleDrawTest, UsesQuads) {
    EXPECT_TRUE(usesQuads(ParticleDraw::Billboard));
    EXPECT_TRUE(usesQuads(ParticleDraw::BillboardVel));
    EXPECT_FALSE(usesQuads(ParticleDraw::Spark));
    EXPECT_FALSE(usesQuads(ParticleDraw::Beam));
}

// =============================================================================
// ParticleBlend Tests
// =============================================================================

TEST(ParticleBlendTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(ParticleBlend::SrcAlphaOneMinusSrcAlpha), 0);
    EXPECT_EQ(static_cast<uint8_t>(ParticleBlend::SrcAlphaOne), 1);
    EXPECT_EQ(static_cast<uint8_t>(ParticleBlend::OneOne), 2);
}

TEST(ParticleBlendTest, Names) {
    EXPECT_EQ(particleBlendName(ParticleBlend::SrcAlphaOneMinusSrcAlpha), "alpha");
    EXPECT_EQ(particleBlendName(ParticleBlend::OneOne), "additive");
}

TEST(ParticleBlendTest, IsAdditive) {
    EXPECT_TRUE(isAdditiveBlend(ParticleBlend::SrcAlphaOne));
    EXPECT_TRUE(isAdditiveBlend(ParticleBlend::OneOne));
    EXPECT_FALSE(isAdditiveBlend(ParticleBlend::SrcAlphaOneMinusSrcAlpha));
    EXPECT_FALSE(isAdditiveBlend(ParticleBlend::ZeroOneMinusSrcColor));
}

// =============================================================================
// Particle Limits Tests
// =============================================================================

TEST(ParticleLimitsTest, Values) {
    EXPECT_EQ(particle_limits::MIN_PARTICLES, 256);
    EXPECT_EQ(particle_limits::MAX_PARTICLES, 32768);
    EXPECT_EQ(particle_limits::DEFAULT_PARTICLES, 2048);
    EXPECT_EQ(particle_limits::MAX_TEXTURE_COMPONENTS, 8);
}

// =============================================================================
// Particle Ramps Tests
// =============================================================================

TEST(ParticleRampsTest, Ramp1) {
    EXPECT_EQ(PARTICLE_RAMP1.size(), 8u);
    EXPECT_EQ(PARTICLE_RAMP1[0], 0x6f);
    EXPECT_EQ(PARTICLE_RAMP1[7], 0x61);
}

TEST(ParticleRampsTest, Ramp2) {
    EXPECT_EQ(PARTICLE_RAMP2.size(), 8u);
    EXPECT_EQ(PARTICLE_RAMP2[0], 0x6f);
}

TEST(ParticleRampsTest, Ramp3) {
    EXPECT_EQ(PARTICLE_RAMP3.size(), 6u);
    EXPECT_EQ(PARTICLE_RAMP3[0], 0x6d);
}

// =============================================================================
// ParticleColor Tests
// =============================================================================

TEST(ParticleColorTest, Default) {
    ParticleColor c;
    EXPECT_EQ(c.r, 255);
    EXPECT_EQ(c.g, 255);
    EXPECT_EQ(c.b, 255);
    EXPECT_EQ(c.a, 255);
    EXPECT_TRUE(c.isOpaque());
    EXPECT_FALSE(c.isTransparent());
}

TEST(ParticleColorTest, Presets) {
    auto white = ParticleColor::white();
    EXPECT_EQ(white.r, 255);
    EXPECT_EQ(white.g, 255);
    EXPECT_EQ(white.b, 255);
    
    auto red = ParticleColor::red();
    EXPECT_EQ(red.r, 255);
    EXPECT_EQ(red.g, 0);
    EXPECT_EQ(red.b, 0);
    
    auto blue = ParticleColor::blue();
    EXPECT_EQ(blue.r, 0);
    EXPECT_EQ(blue.b, 255);
}

// =============================================================================
// Particle Tests
// =============================================================================

TEST(ParticleTest, Default) {
    Particle p;
    EXPECT_FLOAT_EQ(p.size, 1.0f);
    EXPECT_EQ(p.initialAlpha, 255);
    EXPECT_EQ(p.bounces, 0);
}

TEST(ParticleTest, IsAlive) {
    Particle p;
    p.start = 0.0f;
    p.die = 10.0f;
    
    EXPECT_TRUE(p.isAlive(5.0f));
    EXPECT_TRUE(p.isAlive(0.0f));
    EXPECT_FALSE(p.isAlive(10.0f));
    EXPECT_FALSE(p.isAlive(15.0f));
}

TEST(ParticleTest, LifeFraction) {
    Particle p;
    p.start = 0.0f;
    p.die = 10.0f;
    
    EXPECT_FLOAT_EQ(p.lifeFraction(0.0f), 0.0f);
    EXPECT_FLOAT_EQ(p.lifeFraction(5.0f), 0.5f);
    EXPECT_FLOAT_EQ(p.lifeFraction(10.0f), 1.0f);
}

TEST(ParticleTest, RemainingLife) {
    Particle p;
    p.start = 0.0f;
    p.die = 10.0f;
    
    EXPECT_FLOAT_EQ(p.remainingLife(0.0f), 10.0f);
    EXPECT_FLOAT_EQ(p.remainingLife(7.0f), 3.0f);
}

// =============================================================================
// ParticleTypeDef Tests
// =============================================================================

TEST(ParticleTypeDefTest, Default) {
    ParticleTypeDef def;
    EXPECT_EQ(def.id, ParticleType::Spark);
    EXPECT_EQ(def.drawType, ParticleDraw::Normal);
    EXPECT_EQ(def.vertsPerPrimitive, 4);
}

TEST(ParticleTypeDefTest, UsesGravity) {
    ParticleTypeDef def;
    EXPECT_FALSE(def.usesGravity());
    
    def.gravity = -800.0f;
    EXPECT_TRUE(def.usesGravity());
}

TEST(ParticleTypeDefTest, IsAdditive) {
    ParticleTypeDef def;
    EXPECT_FALSE(def.isAdditive());
    
    def.blendType = ParticleBlend::OneOne;
    EXPECT_TRUE(def.isAdditive());
}

// =============================================================================
// ParticleTextureInfo Tests
// =============================================================================

TEST(ParticleTextureInfoTest, Default) {
    ParticleTextureInfo info;
    EXPECT_FALSE(info.isValid());
    EXPECT_EQ(info.textureId, 0u);
    EXPECT_EQ(info.componentCount, 0);
}

TEST(ParticleTextureInfoTest, IsValid) {
    ParticleTextureInfo info;
    info.textureId = 1;
    info.componentCount = 4;
    EXPECT_TRUE(info.isValid());
}

TEST(ParticleTextureInfoTest, GetCoords) {
    ParticleTextureInfo info;
    info.componentCount = 2;
    info.coords[0] = {0.0f, 0.0f, 0.5f, 0.5f};
    info.coords[1] = {0.5f, 0.0f, 1.0f, 0.5f};
    
    auto c0 = info.getCoords(0);
    EXPECT_FLOAT_EQ(c0[0], 0.0f);
    EXPECT_FLOAT_EQ(c0[2], 0.5f);
    
    auto c1 = info.getCoords(1);
    EXPECT_FLOAT_EQ(c1[0], 0.5f);
    
    // Wraps around
    auto c2 = info.getCoords(2);
    EXPECT_FLOAT_EQ(c2[0], 0.0f);
}

// =============================================================================
// ClassicParticle Tests
// =============================================================================

TEST(ClassicParticleTest, Default) {
    ClassicParticle p;
    EXPECT_FLOAT_EQ(p.ramp, 0.0f);
    EXPECT_EQ(p.color, 0);
}

TEST(ClassicParticleTest, IsAlive) {
    ClassicParticle p;
    p.die = 5.0f;
    
    EXPECT_TRUE(p.isAlive(2.0f));
    EXPECT_FALSE(p.isAlive(5.0f));
    EXPECT_FALSE(p.isAlive(10.0f));
}

// =============================================================================
// Trail and Explosion Types Tests
// =============================================================================

TEST(ParticleTrailTypeTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(TrailType::Rocket), 0);
    EXPECT_EQ(static_cast<uint8_t>(TrailType::Grenade), 1);
    EXPECT_EQ(static_cast<uint8_t>(TrailType::Blood), 2);
}

TEST(ParticleExplosionTypeTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(ExplosionType::Normal), 0);
    EXPECT_EQ(static_cast<uint8_t>(ExplosionType::Blood), 1);
    EXPECT_EQ(static_cast<uint8_t>(ExplosionType::Detpack), 3);
}

// =============================================================================
// Random Helper Tests
// =============================================================================

TEST(LHRandomTest, InRange) {
    // Test that lhrandom produces values in expected range
    for (int i = 0; i < 100; i++) {
        float val = lhrandom(0.0f, 1.0f);
        EXPECT_GE(val, 0.0f);
        EXPECT_LE(val, 1.0f);
    }
    
    for (int i = 0; i < 100; i++) {
        float val = lhrandom(-10.0f, 10.0f);
        EXPECT_GE(val, -10.0f);
        EXPECT_LE(val, 10.0f);
    }
}

TEST(CRandTest, InRange) {
    // Test that crand produces values in [-1, 1]
    for (int i = 0; i < 100; i++) {
        float val = crand();
        EXPECT_GE(val, -1.0f);
        EXPECT_LE(val, 1.0f);
    }
}
