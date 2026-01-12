/**
 * @file effects_test.cpp
 * @brief Unit tests for effects_types.hpp
 * 
 * Tests for particle system, dynamic lights, beams, explosions,
 * and other visual effect types.
 */

#include <gtest/gtest.h>
#include "../../core/effects/effects_types.hpp"

namespace ezquake::effects {
namespace {

//=============================================================================
// Effect Limits Tests
//=============================================================================

TEST(EffectLimitsTest, ParticleLimitsAreValid) {
    EXPECT_GT(effect_limits::ABSOLUTE_MIN_PARTICLES, 0);
    EXPECT_GT(effect_limits::ABSOLUTE_MAX_PARTICLES, effect_limits::ABSOLUTE_MIN_PARTICLES);
    EXPECT_GE(effect_limits::DEFAULT_PARTICLES, effect_limits::ABSOLUTE_MIN_PARTICLES);
    EXPECT_LE(effect_limits::DEFAULT_PARTICLES, effect_limits::ABSOLUTE_MAX_PARTICLES);
}

TEST(EffectLimitsTest, QMBParticleLimitsAreValid) {
    EXPECT_GT(effect_limits::QMB_MAX_PARTICLES, effect_limits::QMB_MIN_PARTICLES);
    EXPECT_GT(effect_limits::QMB_MAX_PARTICLES, effect_limits::ABSOLUTE_MAX_PARTICLES);
}

TEST(EffectLimitsTest, TempEntityLimitsAreValid) {
    EXPECT_GT(effect_limits::MAX_BEAMS, 0);
    EXPECT_GT(effect_limits::MAX_EXPLOSIONS, 0);
    EXPECT_GT(effect_limits::MAX_DLIGHTS, 0);
}

//=============================================================================
// Classic Particle Type Tests
//=============================================================================

TEST(ClassicParticleTypeTest, AllTypesHaveNames) {
    EXPECT_EQ(classicParticleTypeName(ClassicParticleType::Static), "Static");
    EXPECT_EQ(classicParticleTypeName(ClassicParticleType::Gravity), "Gravity");
    EXPECT_EQ(classicParticleTypeName(ClassicParticleType::SlowGravity), "SlowGravity");
    EXPECT_EQ(classicParticleTypeName(ClassicParticleType::Fire), "Fire");
    EXPECT_EQ(classicParticleTypeName(ClassicParticleType::Explode), "Explode");
    EXPECT_EQ(classicParticleTypeName(ClassicParticleType::Explode2), "Explode2");
    EXPECT_EQ(classicParticleTypeName(ClassicParticleType::Blob), "Blob");
    EXPECT_EQ(classicParticleTypeName(ClassicParticleType::Blob2), "Blob2");
    EXPECT_EQ(classicParticleTypeName(ClassicParticleType::Rail), "Rail");
}

TEST(ClassicParticleTypeTest, CountMatchesEnumSize) {
    // Count should be the last enum + 1
    EXPECT_EQ(static_cast<int>(ClassicParticleType::Count), 9);
}

//=============================================================================
// QMB Particle Type Tests
//=============================================================================

TEST(QMBParticleTypeTest, BloodParticleDetection) {
    EXPECT_TRUE(isBloodParticle(QMBParticleType::Blood1));
    EXPECT_TRUE(isBloodParticle(QMBParticleType::Blood2));
    EXPECT_TRUE(isBloodParticle(QMBParticleType::Blood3));
    EXPECT_TRUE(isBloodParticle(QMBParticleType::VXBlood));
    EXPECT_TRUE(isBloodParticle(QMBParticleType::BloodCloud));
    EXPECT_TRUE(isBloodParticle(QMBParticleType::TrailBleed));
    EXPECT_TRUE(isBloodParticle(QMBParticleType::BleedSpike));
    
    EXPECT_FALSE(isBloodParticle(QMBParticleType::Spark));
    EXPECT_FALSE(isBloodParticle(QMBParticleType::Smoke));
    EXPECT_FALSE(isBloodParticle(QMBParticleType::Fire));
}

TEST(QMBParticleTypeTest, TrailParticleDetection) {
    EXPECT_TRUE(isTrailParticle(QMBParticleType::TrailPart));
    EXPECT_TRUE(isTrailParticle(QMBParticleType::AlphaTrail));
    EXPECT_TRUE(isTrailParticle(QMBParticleType::RailTrail));
    EXPECT_TRUE(isTrailParticle(QMBParticleType::StreakTrail));
    EXPECT_TRUE(isTrailParticle(QMBParticleType::LavaTrail));
    EXPECT_TRUE(isTrailParticle(QMBParticleType::NailTrail));
    EXPECT_TRUE(isTrailParticle(QMBParticleType::InfernoTrail));
    
    EXPECT_FALSE(isTrailParticle(QMBParticleType::Spark));
    EXPECT_FALSE(isTrailParticle(QMBParticleType::Blood1));
}

TEST(QMBParticleTypeTest, CountIsValid) {
    // Should have many particle types
    EXPECT_GT(static_cast<int>(QMBParticleType::Count), 40);
}

//=============================================================================
// Particle Movement Tests
//=============================================================================

TEST(ParticleMovementTest, CollisionDetection) {
    EXPECT_TRUE(hasCollision(ParticleMovement::Bounce));
    EXPECT_TRUE(hasCollision(ParticleMovement::Die));
    EXPECT_TRUE(hasCollision(ParticleMovement::Rain));
    
    EXPECT_FALSE(hasCollision(ParticleMovement::Static));
    EXPECT_FALSE(hasCollision(ParticleMovement::Normal));
    EXPECT_FALSE(hasCollision(ParticleMovement::NoPhysics));
    EXPECT_FALSE(hasCollision(ParticleMovement::Float));
}

//=============================================================================
// Particle Draw Type Tests
//=============================================================================

TEST(ParticleDrawTypeTest, BillboardDetection) {
    EXPECT_TRUE(isBillboard(ParticleDrawType::Billboard));
    EXPECT_TRUE(isBillboard(ParticleDrawType::BillboardVel));
    
    EXPECT_FALSE(isBillboard(ParticleDrawType::Spark));
    EXPECT_FALSE(isBillboard(ParticleDrawType::SparkRay));
    EXPECT_FALSE(isBillboard(ParticleDrawType::Beam));
}

//=============================================================================
// Particle Blend Mode Tests
//=============================================================================

TEST(ParticleBlendModeTest, AdditiveDetection) {
    EXPECT_TRUE(isAdditive(ParticleBlendMode::SrcAlpha_One));
    EXPECT_TRUE(isAdditive(ParticleBlendMode::One_One));
    
    EXPECT_FALSE(isAdditive(ParticleBlendMode::SrcAlpha_OneMinusSrcAlpha));
    EXPECT_FALSE(isAdditive(ParticleBlendMode::Zero_OneMinusSrcColor));
}

//=============================================================================
// Trail Type Tests
//=============================================================================

TEST(TrailTypeTest, AllTypesHaveNames) {
    EXPECT_EQ(trailTypeName(TrailType::Rocket), "Rocket");
    EXPECT_EQ(trailTypeName(TrailType::Grenade), "Grenade");
    EXPECT_EQ(trailTypeName(TrailType::Blood), "Blood");
    EXPECT_EQ(trailTypeName(TrailType::Rail), "Rail");
    EXPECT_EQ(trailTypeName(TrailType::Voorhees), "Voorhees");
}

TEST(TrailTypeTest, BloodTrailDetection) {
    EXPECT_TRUE(isBloodTrail(TrailType::Blood));
    EXPECT_TRUE(isBloodTrail(TrailType::BigBlood));
    EXPECT_TRUE(isBloodTrail(TrailType::Bleeding));
    EXPECT_TRUE(isBloodTrail(TrailType::Bleeding2));
    
    EXPECT_FALSE(isBloodTrail(TrailType::Rocket));
    EXPECT_FALSE(isBloodTrail(TrailType::Grenade));
    EXPECT_FALSE(isBloodTrail(TrailType::Rail));
    EXPECT_FALSE(isBloodTrail(TrailType::Lava));
}

//=============================================================================
// Dynamic Light Type Tests
//=============================================================================

TEST(DynamicLightTypeTest, AllTypesHaveNames) {
    EXPECT_EQ(dynamicLightTypeName(DynamicLightType::Default), "Default");
    EXPECT_EQ(dynamicLightTypeName(DynamicLightType::MuzzleFlash), "MuzzleFlash");
    EXPECT_EQ(dynamicLightTypeName(DynamicLightType::Explosion), "Explosion");
    EXPECT_EQ(dynamicLightTypeName(DynamicLightType::Rocket), "Rocket");
    EXPECT_EQ(dynamicLightTypeName(DynamicLightType::Custom), "Custom");
}

TEST(DynamicLightTypeTest, ColorsAreNonZero) {
    auto defaultColor = dynamicLightColor(DynamicLightType::Default);
    EXPECT_EQ(defaultColor[0], 255);  // White
    EXPECT_EQ(defaultColor[1], 255);
    EXPECT_EQ(defaultColor[2], 255);
    
    auto redColor = dynamicLightColor(DynamicLightType::Red);
    EXPECT_EQ(redColor[0], 255);
    EXPECT_EQ(redColor[1], 0);
    EXPECT_EQ(redColor[2], 0);
    
    auto blueColor = dynamicLightColor(DynamicLightType::Blue);
    EXPECT_EQ(blueColor[0], 0);
    EXPECT_EQ(blueColor[1], 0);
    EXPECT_EQ(blueColor[2], 255);
    
    auto greenColor = dynamicLightColor(DynamicLightType::Green);
    EXPECT_EQ(greenColor[0], 0);
    EXPECT_EQ(greenColor[1], 255);
    EXPECT_EQ(greenColor[2], 0);
}

TEST(DynamicLightTypeTest, ExplosionColorIsWarm) {
    auto color = dynamicLightColor(DynamicLightType::Explosion);
    // Explosion should be warm (more red than blue)
    EXPECT_GT(color[0], color[2]);
}

//=============================================================================
// Color Ramp Tests
//=============================================================================

TEST(ColorRampTest, RampSizesAreCorrect) {
    EXPECT_EQ(color_ramp::RAMP1.size(), 8u);
    EXPECT_EQ(color_ramp::RAMP2.size(), 8u);
    EXPECT_EQ(color_ramp::RAMP3.size(), 6u);
}

TEST(ColorRampTest, RampValuesAreInPaletteRange) {
    for (auto val : color_ramp::RAMP1) {
        EXPECT_LT(val, 256);  // Valid palette index
    }
    for (auto val : color_ramp::RAMP2) {
        EXPECT_LT(val, 256);
    }
    for (auto val : color_ramp::RAMP3) {
        EXPECT_LT(val, 256);
    }
}

//=============================================================================
// Particle Color Tests
//=============================================================================

TEST(ParticleColorTest, DefaultConstruction) {
    ParticleColor color;
    EXPECT_EQ(color.r, 255);
    EXPECT_EQ(color.g, 255);
    EXPECT_EQ(color.b, 255);
    EXPECT_EQ(color.a, 255);
}

TEST(ParticleColorTest, ConstructWithValues) {
    ParticleColor color(128, 64, 32, 200);
    EXPECT_EQ(color.r, 128);
    EXPECT_EQ(color.g, 64);
    EXPECT_EQ(color.b, 32);
    EXPECT_EQ(color.a, 200);
}

TEST(ParticleColorTest, FromFloat) {
    auto color = ParticleColor::fromFloat(1.0f, 0.5f, 0.0f, 0.8f);
    EXPECT_EQ(color.r, 255);
    EXPECT_EQ(color.g, 127);
    EXPECT_EQ(color.b, 0);
    EXPECT_EQ(color.a, 204);
}

TEST(ParticleColorTest, ToFloat) {
    ParticleColor color(255, 128, 0, 255);
    auto floats = color.toFloat();
    EXPECT_NEAR(floats[0], 1.0f, 0.01f);
    EXPECT_NEAR(floats[1], 0.5f, 0.01f);
    EXPECT_NEAR(floats[2], 0.0f, 0.01f);
    EXPECT_NEAR(floats[3], 1.0f, 0.01f);
}

TEST(ParticleColorTest, Equality) {
    ParticleColor a(100, 150, 200, 250);
    ParticleColor b(100, 150, 200, 250);
    ParticleColor c(100, 150, 200, 100);
    
    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

//=============================================================================
// Classic Particle Tests
//=============================================================================

TEST(ClassicParticleTest, LifetimeCheck) {
    ClassicParticle particle;
    particle.die = 10.0f;
    
    EXPECT_TRUE(particle.isAlive(5.0f));
    EXPECT_TRUE(particle.isAlive(9.9f));
    EXPECT_FALSE(particle.isAlive(10.0f));
    EXPECT_FALSE(particle.isAlive(15.0f));
}

TEST(ClassicParticleTest, RemainingLife) {
    ClassicParticle particle;
    particle.die = 10.0f;
    
    EXPECT_NEAR(particle.remainingLife(5.0f), 5.0f, 0.001f);
    EXPECT_NEAR(particle.remainingLife(8.0f), 2.0f, 0.001f);
    EXPECT_NEAR(particle.remainingLife(10.0f), 0.0f, 0.001f);
}

//=============================================================================
// QMB Particle Tests
//=============================================================================

TEST(QMBParticleTest, LifetimeCheck) {
    QMBParticle particle;
    particle.startTime = 0.0f;
    particle.dieTime = 10.0f;
    
    EXPECT_TRUE(particle.isAlive(5.0f));
    EXPECT_FALSE(particle.isAlive(10.0f));
    EXPECT_FALSE(particle.isAlive(15.0f));
}

TEST(QMBParticleTest, Age) {
    QMBParticle particle;
    particle.startTime = 5.0f;
    particle.dieTime = 15.0f;
    
    EXPECT_NEAR(particle.age(5.0f), 0.0f, 0.001f);
    EXPECT_NEAR(particle.age(10.0f), 5.0f, 0.001f);
    EXPECT_NEAR(particle.age(15.0f), 10.0f, 0.001f);
}

TEST(QMBParticleTest, LifeProgress) {
    QMBParticle particle;
    particle.startTime = 0.0f;
    particle.dieTime = 10.0f;
    
    EXPECT_NEAR(particle.lifeProgress(0.0f), 0.0f, 0.001f);
    EXPECT_NEAR(particle.lifeProgress(5.0f), 0.5f, 0.001f);
    EXPECT_NEAR(particle.lifeProgress(10.0f), 1.0f, 0.001f);
}

TEST(QMBParticleTest, CurrentSize) {
    QMBParticle particle;
    particle.startTime = 0.0f;
    particle.size = 10.0f;
    particle.growth = 2.0f;
    
    EXPECT_NEAR(particle.currentSize(0.0f), 10.0f, 0.001f);
    EXPECT_NEAR(particle.currentSize(5.0f), 20.0f, 0.001f);
}

TEST(QMBParticleTest, CurrentAlpha) {
    QMBParticle particle;
    particle.startTime = 0.0f;
    particle.dieTime = 10.0f;
    particle.initialAlpha = 255;
    
    EXPECT_NEAR(particle.currentAlpha(0.0f), 255.0f, 0.01f);
    EXPECT_NEAR(particle.currentAlpha(5.0f), 127.5f, 0.1f);
    EXPECT_NEAR(particle.currentAlpha(10.0f), 0.0f, 0.01f);
}

//=============================================================================
// Particle Type Config Tests
//=============================================================================

TEST(ParticleTypeConfigTest, DefaultValues) {
    ParticleTypeConfig config;
    
    EXPECT_EQ(config.id, QMBParticleType::Spark);
    EXPECT_EQ(config.drawType, ParticleDrawType::Spark);
    EXPECT_EQ(config.blendMode, ParticleBlendMode::SrcAlpha_OneMinusSrcAlpha);
    EXPECT_EQ(config.texture, ParticleTexture::None);
    EXPECT_NEAR(config.startAlpha, 1.0f, 0.001f);
}

TEST(ParticleTypeConfigTest, HasGravity) {
    ParticleTypeConfig config;
    config.gravity = 0.0f;
    EXPECT_FALSE(config.hasGravity());
    
    config.gravity = 1.0f;
    EXPECT_TRUE(config.hasGravity());
    
    config.gravity = -0.5f;
    EXPECT_TRUE(config.hasGravity());
}

TEST(ParticleTypeConfigTest, IsAdditive) {
    ParticleTypeConfig config;
    
    config.blendMode = ParticleBlendMode::SrcAlpha_OneMinusSrcAlpha;
    EXPECT_FALSE(config.isAdditive());
    
    config.blendMode = ParticleBlendMode::SrcAlpha_One;
    EXPECT_TRUE(config.isAdditive());
    
    config.blendMode = ParticleBlendMode::One_One;
    EXPECT_TRUE(config.isAdditive());
}

//=============================================================================
// Dynamic Light Tests
//=============================================================================

TEST(DynamicLightTest, LifetimeCheck) {
    DynamicLight light;
    light.dieTime = 10.0f;
    
    EXPECT_TRUE(light.isAlive(5.0f));
    EXPECT_FALSE(light.isAlive(10.0f));
}

TEST(DynamicLightTest, CurrentRadius) {
    DynamicLight light;
    light.radius = 200.0f;
    light.decay = 20.0f;  // 20 units per second
    
    EXPECT_NEAR(light.currentRadius(5.0f, 0.0f), 100.0f, 0.001f);
    EXPECT_NEAR(light.currentRadius(10.0f, 0.0f), 0.0f, 0.001f);
    EXPECT_NEAR(light.currentRadius(15.0f, 0.0f), 0.0f, 0.001f);  // Clamped to 0
}

TEST(DynamicLightTest, GetColor) {
    DynamicLight light;
    
    light.type = DynamicLightType::Red;
    auto color = light.getColor();
    EXPECT_EQ(color[0], 255);
    EXPECT_EQ(color[1], 0);
    EXPECT_EQ(color[2], 0);
    
    light.type = DynamicLightType::Custom;
    light.customColor = ParticleColor(100, 150, 200);
    color = light.getColor();
    EXPECT_EQ(color[0], 100);
    EXPECT_EQ(color[1], 150);
    EXPECT_EQ(color[2], 200);
}

//=============================================================================
// Custom Light Tests
//=============================================================================

TEST(CustomLightTest, GetColorByType) {
    CustomLight light;
    
    light.type = DynamicLightType::Blue;
    auto color = light.getColor();
    EXPECT_EQ(color[0], 0);
    EXPECT_EQ(color[1], 0);
    EXPECT_EQ(color[2], 255);
}

TEST(CustomLightTest, GetCustomColor) {
    CustomLight light;
    light.type = DynamicLightType::Custom;
    light.color = ParticleColor(50, 100, 150);
    
    auto color = light.getColor();
    EXPECT_EQ(color[0], 50);
    EXPECT_EQ(color[1], 100);
    EXPECT_EQ(color[2], 150);
}

//=============================================================================
// Light Style Tests
//=============================================================================

TEST(LightStyleTest, DefaultBrightness) {
    LightStyle style;
    // Empty pattern = always full brightness
    EXPECT_NEAR(style.getBrightness(0.0f), 1.0f, 0.001f);
    EXPECT_NEAR(style.getBrightness(100.0f), 1.0f, 0.001f);
}

TEST(LightStyleTest, SetPattern) {
    LightStyle style;
    style.setPattern("mmm");
    
    EXPECT_EQ(style.length, 3);
    EXPECT_EQ(style.pattern[0], 'm');
    EXPECT_EQ(style.pattern[1], 'm');
    EXPECT_EQ(style.pattern[2], 'm');
}

TEST(LightStyleTest, BrightnessFromPattern) {
    LightStyle style;
    style.setPattern("az");  // 'a' = 0, 'z' = max
    
    // 'a' = 0 brightness
    float brightnessA = style.getBrightness(0.0f);
    EXPECT_NEAR(brightnessA, 0.0f, 0.001f);
    
    // 'z' = 25/12 = ~2.08 (double bright)
    float brightnessZ = style.getBrightness(0.1f);  // Next frame
    EXPECT_GT(brightnessZ, 1.5f);
}

TEST(LightStyleTest, NormalBrightness) {
    LightStyle style;
    style.setPattern("m");  // 'm' = normal brightness (1.0)
    
    float brightness = style.getBrightness(0.0f);
    EXPECT_NEAR(brightness, 1.0f, 0.001f);
}

//=============================================================================
// Beam Tests
//=============================================================================

TEST(BeamTest, LifetimeCheck) {
    Beam beam;
    beam.endTime = 10.0f;
    
    EXPECT_TRUE(beam.isAlive(5.0f));
    EXPECT_FALSE(beam.isAlive(10.0f));
}

TEST(BeamTest, Direction) {
    Beam beam;
    beam.start = math::Vec3(0, 0, 0);
    beam.end = math::Vec3(100, 0, 0);
    
    auto dir = beam.direction();
    EXPECT_NEAR(dir.x(), 100.0f, 0.001f);
    EXPECT_NEAR(dir.y(), 0.0f, 0.001f);
    EXPECT_NEAR(dir.z(), 0.0f, 0.001f);
}

TEST(BeamTest, Length) {
    Beam beam;
    beam.start = math::Vec3(0, 0, 0);
    beam.end = math::Vec3(3, 4, 0);
    
    EXPECT_NEAR(beam.length(), 5.0f, 0.001f);
}

//=============================================================================
// Explosion Tests
//=============================================================================

TEST(ExplosionTest, GetFrame) {
    Explosion explosion;
    explosion.startTime = 0.0f;
    
    // 10 fps, 6 frames
    EXPECT_EQ(explosion.getFrame(0.0f), 0);
    EXPECT_EQ(explosion.getFrame(0.1f), 1);
    EXPECT_EQ(explosion.getFrame(0.2f), 2);
    EXPECT_EQ(explosion.getFrame(0.5f), 5);  // Last frame
    EXPECT_EQ(explosion.getFrame(1.0f), 5);  // Clamped to last
}

TEST(ExplosionTest, IsComplete) {
    Explosion explosion;
    explosion.startTime = 0.0f;
    
    EXPECT_FALSE(explosion.isComplete(0.0f));
    EXPECT_FALSE(explosion.isComplete(0.4f));
    EXPECT_TRUE(explosion.isComplete(0.5f));
    EXPECT_TRUE(explosion.isComplete(1.0f));
}

//=============================================================================
// Effect Stats Tests
//=============================================================================

TEST(EffectStatsTest, ParticleUsage) {
    EffectStats stats;
    stats.activeParticles = 500;
    
    EXPECT_NEAR(stats.particleUsage(1000), 0.5f, 0.001f);
    EXPECT_NEAR(stats.particleUsage(2000), 0.25f, 0.001f);
}

TEST(EffectStatsTest, RecordSpawnAndDeath) {
    EffectStats stats;
    
    stats.recordParticleSpawn(100);
    EXPECT_EQ(stats.activeParticles, 100);
    EXPECT_EQ(stats.totalSpawned, 100);
    EXPECT_EQ(stats.peakParticles, 100);
    
    stats.recordParticleSpawn(50);
    EXPECT_EQ(stats.activeParticles, 150);
    EXPECT_EQ(stats.peakParticles, 150);
    
    stats.recordParticleDeath(30);
    EXPECT_EQ(stats.activeParticles, 120);
    EXPECT_EQ(stats.peakParticles, 150);  // Peak unchanged
}

TEST(EffectStatsTest, Reset) {
    EffectStats stats;
    stats.recordParticleSpawn(100);
    stats.reset();
    
    EXPECT_EQ(stats.activeParticles, 0);
    EXPECT_EQ(stats.peakParticles, 100);  // Peak preserved
    EXPECT_EQ(stats.totalSpawned, 100);   // Total preserved
}

//=============================================================================
// Particle Spawn Params Tests
//=============================================================================

TEST(ParticleSpawnParamsTest, FluentInterface) {
    auto params = ParticleSpawnParams()
        .withOrigin(math::Vec3(1, 2, 3))
        .withDirection(math::Vec3(0, 0, 1))
        .withColor(ParticleColor(255, 0, 0))
        .withSize(5.0f)
        .withLifetime(2.0f)
        .withCount(10)
        .withSpread(15.0f);
    
    EXPECT_NEAR(params.origin.x(), 1.0f, 0.001f);
    EXPECT_NEAR(params.origin.y(), 2.0f, 0.001f);
    EXPECT_NEAR(params.origin.z(), 3.0f, 0.001f);
    EXPECT_NEAR(params.direction.z(), 1.0f, 0.001f);
    EXPECT_EQ(params.color.r, 255);
    EXPECT_EQ(params.color.g, 0);
    EXPECT_NEAR(params.size, 5.0f, 0.001f);
    EXPECT_NEAR(params.lifetime, 2.0f, 0.001f);
    EXPECT_EQ(params.count, 10);
    EXPECT_NEAR(params.spread, 15.0f, 0.001f);
}

//=============================================================================
// Effect Config Tests
//=============================================================================

TEST(EffectConfigTest, DefaultValues) {
    EffectConfig config;
    
    EXPECT_EQ(config.maxParticles, effect_limits::DEFAULT_PARTICLES);
    EXPECT_TRUE(config.useQMBParticles);
    EXPECT_TRUE(config.drawParticles);
    EXPECT_TRUE(config.partBlood);
    EXPECT_TRUE(config.partExplosions);
}

TEST(EffectConfigTest, IsParticleEnabled) {
    EffectConfig config;
    
    // All enabled by default
    EXPECT_TRUE(config.isParticleEnabled(TrailType::Rocket));
    EXPECT_TRUE(config.isParticleEnabled(TrailType::Blood));
    
    // Disable blood trails
    config.partBlood = false;
    EXPECT_FALSE(config.isParticleEnabled(TrailType::Blood));
    EXPECT_FALSE(config.isParticleEnabled(TrailType::Bleeding));
    
    // Non-blood trails still work
    EXPECT_TRUE(config.isParticleEnabled(TrailType::Rocket));
    
    // Disable all trails
    config.partTrails = false;
    EXPECT_FALSE(config.isParticleEnabled(TrailType::Rocket));
}

} // anonymous namespace
} // namespace ezquake::effects
