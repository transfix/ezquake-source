/**
 * @file corona_test.cpp
 * @brief Unit tests for corona and visual effect types
 */

#include <gtest/gtest.h>
#include "core/corona/corona_types.hpp"

using namespace ezquake::corona;

// ============================================================================
// Constants Tests
// ============================================================================

TEST(CoronaConstantsTest, MaxCoronas) {
    EXPECT_GE(constants::MAX_CORONAS, 256);
    EXPECT_LE(constants::MAX_CORONAS, 4096);
}

TEST(CoronaConstantsTest, DefaultRadius) {
    EXPECT_GT(constants::DEFAULT_RADIUS, 0.0f);
    EXPECT_LE(constants::DEFAULT_RADIUS, 100.0f);
}

TEST(CoronaConstantsTest, ExplosionFrames) {
    EXPECT_EQ(constants::EXPLOSION_FRAMES, 7);
    EXPECT_GT(constants::EXPLOSION_FRAME_TIME, 0.0f);
}

// ============================================================================
// CoronaType Tests
// ============================================================================

TEST(CoronaTypeTest, EnumValues) {
    EXPECT_EQ(static_cast<int>(CoronaType::Flash), 0);
    EXPECT_EQ(static_cast<int>(CoronaType::SmallFlash), 1);
    EXPECT_EQ(static_cast<int>(CoronaType::BlueFlash), 2);
    EXPECT_EQ(static_cast<int>(CoronaType::Free), 3);
}

TEST(CoronaTypeTest, Names) {
    EXPECT_EQ(coronaTypeName(CoronaType::Flash), "Flash");
    EXPECT_EQ(coronaTypeName(CoronaType::SmallFlash), "SmallFlash");
    EXPECT_EQ(coronaTypeName(CoronaType::BlueFlash), "BlueFlash");
    EXPECT_EQ(coronaTypeName(CoronaType::Lightning), "Lightning");
    EXPECT_EQ(coronaTypeName(CoronaType::Fire), "Fire");
    EXPECT_EQ(coronaTypeName(CoronaType::RocketLight), "RocketLight");
    EXPECT_EQ(coronaTypeName(CoronaType::GreenLight), "GreenLight");
    EXPECT_EQ(coronaTypeName(CoronaType::RedLight), "RedLight");
    EXPECT_EQ(coronaTypeName(CoronaType::Explode), "Explode");
    EXPECT_EQ(coronaTypeName(CoronaType::WizLight), "WizLight");
    EXPECT_EQ(coronaTypeName(CoronaType::KnightLight), "KnightLight");
    EXPECT_EQ(coronaTypeName(CoronaType::VoreLight), "VoreLight");
}

TEST(CoronaTypeTest, IsFlashType) {
    EXPECT_TRUE(isFlashType(CoronaType::Flash));
    EXPECT_TRUE(isFlashType(CoronaType::SmallFlash));
    EXPECT_TRUE(isFlashType(CoronaType::BlueFlash));
    EXPECT_TRUE(isFlashType(CoronaType::GunFlash));
    EXPECT_FALSE(isFlashType(CoronaType::Lightning));
    EXPECT_FALSE(isFlashType(CoronaType::Fire));
    EXPECT_FALSE(isFlashType(CoronaType::Explode));
}

TEST(CoronaTypeTest, IsLightningType) {
    EXPECT_TRUE(isLightningType(CoronaType::Lightning));
    EXPECT_TRUE(isLightningType(CoronaType::SmallLightning));
    EXPECT_FALSE(isLightningType(CoronaType::Flash));
    EXPECT_FALSE(isLightningType(CoronaType::Fire));
}

TEST(CoronaTypeTest, IsEnemyTracerType) {
    EXPECT_TRUE(isEnemyTracerType(CoronaType::WizLight));
    EXPECT_TRUE(isEnemyTracerType(CoronaType::KnightLight));
    EXPECT_TRUE(isEnemyTracerType(CoronaType::VoreLight));
    EXPECT_FALSE(isEnemyTracerType(CoronaType::Flash));
    EXPECT_FALSE(isEnemyTracerType(CoronaType::Lightning));
}

TEST(CoronaTypeTest, IsAnimatedType) {
    EXPECT_TRUE(isAnimatedType(CoronaType::Explode));
    EXPECT_FALSE(isAnimatedType(CoronaType::Flash));
    EXPECT_FALSE(isAnimatedType(CoronaType::Lightning));
    EXPECT_FALSE(isAnimatedType(CoronaType::Fire));
}

// ============================================================================
// CoronaTextureId Tests
// ============================================================================

TEST(CoronaTextureIdTest, EnumValues) {
    EXPECT_EQ(static_cast<int>(CoronaTextureId::Standard), 0);
    EXPECT_EQ(static_cast<int>(CoronaTextureId::GunFlash), 1);
    EXPECT_EQ(static_cast<int>(CoronaTextureId::ExplosionFlash1), 2);
    EXPECT_EQ(static_cast<int>(CoronaTextureId::Count), 9);
}

TEST(CoronaTextureIdTest, Names) {
    EXPECT_EQ(coronaTextureName(CoronaTextureId::Standard), "Standard");
    EXPECT_EQ(coronaTextureName(CoronaTextureId::GunFlash), "GunFlash");
    EXPECT_EQ(coronaTextureName(CoronaTextureId::ExplosionFlash1), "ExplosionFlash1");
    EXPECT_EQ(coronaTextureName(CoronaTextureId::ExplosionFlash7), "ExplosionFlash7");
}

TEST(CoronaTextureIdTest, IsExplosionTexture) {
    EXPECT_FALSE(isExplosionTexture(CoronaTextureId::Standard));
    EXPECT_FALSE(isExplosionTexture(CoronaTextureId::GunFlash));
    EXPECT_TRUE(isExplosionTexture(CoronaTextureId::ExplosionFlash1));
    EXPECT_TRUE(isExplosionTexture(CoronaTextureId::ExplosionFlash4));
    EXPECT_TRUE(isExplosionTexture(CoronaTextureId::ExplosionFlash7));
}

TEST(CoronaTextureIdTest, GetExplosionFrame) {
    EXPECT_FALSE(getExplosionFrame(CoronaTextureId::Standard).has_value());
    EXPECT_FALSE(getExplosionFrame(CoronaTextureId::GunFlash).has_value());
    EXPECT_EQ(getExplosionFrame(CoronaTextureId::ExplosionFlash1).value(), 0);
    EXPECT_EQ(getExplosionFrame(CoronaTextureId::ExplosionFlash4).value(), 3);
    EXPECT_EQ(getExplosionFrame(CoronaTextureId::ExplosionFlash7).value(), 6);
}

TEST(CoronaTextureIdTest, ExplosionFrameTexture) {
    EXPECT_EQ(explosionFrameTexture(0), CoronaTextureId::ExplosionFlash1);
    EXPECT_EQ(explosionFrameTexture(3), CoronaTextureId::ExplosionFlash4);
    EXPECT_EQ(explosionFrameTexture(6), CoronaTextureId::ExplosionFlash7);
    // Clamping
    EXPECT_EQ(explosionFrameTexture(-1), CoronaTextureId::ExplosionFlash1);
    EXPECT_EQ(explosionFrameTexture(100), CoronaTextureId::ExplosionFlash7);
}

// ============================================================================
// CoronaColor Tests
// ============================================================================

TEST(CoronaColorTest, DefaultConstruction) {
    CoronaColor color;
    EXPECT_EQ(color.r, 1.0f);
    EXPECT_EQ(color.g, 1.0f);
    EXPECT_EQ(color.b, 1.0f);
}

TEST(CoronaColorTest, ValueConstruction) {
    CoronaColor color(0.5f, 0.25f, 0.75f);
    EXPECT_EQ(color.r, 0.5f);
    EXPECT_EQ(color.g, 0.25f);
    EXPECT_EQ(color.b, 0.75f);
}

TEST(CoronaColorTest, FromBytes) {
    auto color = CoronaColor::fromBytes(255, 128, 0);
    EXPECT_FLOAT_EQ(color.r, 1.0f);
    EXPECT_NEAR(color.g, 0.5f, 0.01f);
    EXPECT_FLOAT_EQ(color.b, 0.0f);
}

TEST(CoronaColorTest, Scaled) {
    CoronaColor color(1.0f, 0.5f, 0.25f);
    auto scaled = color.scaled(0.5f);
    EXPECT_EQ(scaled.r, 0.5f);
    EXPECT_EQ(scaled.g, 0.25f);
    EXPECT_EQ(scaled.b, 0.125f);
}

TEST(CoronaColorTest, Blend) {
    CoronaColor c1(1.0f, 0.0f, 0.0f);
    CoronaColor c2(0.0f, 1.0f, 0.0f);
    auto blended = c1.blend(c2, 0.5f);
    EXPECT_EQ(blended.r, 0.5f);
    EXPECT_EQ(blended.g, 0.5f);
    EXPECT_EQ(blended.b, 0.0f);
}

TEST(CoronaColorTest, PresetColors) {
    EXPECT_EQ(CoronaColor::white(), CoronaColor(1.0f, 1.0f, 1.0f));
    EXPECT_NE(CoronaColor::yellow(), CoronaColor::white());
    EXPECT_NE(CoronaColor::red(), CoronaColor::blue());
}

TEST(CoronaColorTest, GetCoronaColor) {
    auto flash = getCoronaColor(CoronaType::Flash);
    EXPECT_GT(flash.r, 0.0f);  // Flash should have color
    
    auto green = getCoronaColor(CoronaType::GreenLight);
    EXPECT_GT(green.g, green.r);  // Green should have more green than red
    
    auto blue = getCoronaColor(CoronaType::BlueFlash);
    EXPECT_GT(blue.b, blue.r);  // Blue should have more blue than red
}

TEST(CoronaColorTest, GetCoronaRadius) {
    // Flash should be large
    EXPECT_GT(getCoronaRadius(CoronaType::Flash), getCoronaRadius(CoronaType::SmallFlash));
    
    // Explode should be the largest
    EXPECT_GE(getCoronaRadius(CoronaType::Explode), getCoronaRadius(CoronaType::Flash));
    
    // All radii should be positive
    EXPECT_GT(getCoronaRadius(CoronaType::Fire), 0.0f);
    EXPECT_GT(getCoronaRadius(CoronaType::RocketLight), 0.0f);
}

TEST(CoronaColorTest, GetCoronaLifetime) {
    // Flash types have short lifetimes
    EXPECT_GT(getCoronaLifetime(CoronaType::Flash), 0.0f);
    EXPECT_LT(getCoronaLifetime(CoronaType::SmallFlash), 1.0f);
    
    // Continuous types have 0 lifetime
    EXPECT_EQ(getCoronaLifetime(CoronaType::Fire), 0.0f);
    EXPECT_EQ(getCoronaLifetime(CoronaType::RocketLight), 0.0f);
}

// ============================================================================
// TextureHandle Tests
// ============================================================================

TEST(TextureHandleTest, DefaultConstruction) {
    TextureHandle handle;
    EXPECT_FALSE(handle.isValid());
    EXPECT_EQ(handle.id, 0);
}

TEST(TextureHandleTest, ValueConstruction) {
    TextureHandle handle(42);
    EXPECT_TRUE(handle.isValid());
    EXPECT_EQ(handle.id, 42);
}

TEST(TextureHandleTest, Equality) {
    TextureHandle h1(10);
    TextureHandle h2(10);
    TextureHandle h3(20);
    
    EXPECT_EQ(h1, h2);
    EXPECT_NE(h1, h3);
}

// ============================================================================
// CoronaTexture Tests
// ============================================================================

TEST(CoronaTextureTest, DefaultConstruction) {
    CoronaTexture tex;
    EXPECT_FALSE(tex.hasTexture());
    EXPECT_FALSE(tex.hasArrayTexture());
    EXPECT_FALSE(tex.isValid());
}

TEST(CoronaTextureTest, WithIndividualTexture) {
    CoronaTexture tex(TextureHandle(1));
    EXPECT_TRUE(tex.hasTexture());
    EXPECT_FALSE(tex.hasArrayTexture());
    EXPECT_TRUE(tex.isValid());
}

TEST(CoronaTextureTest, WithArrayTexture) {
    CoronaTexture tex(TextureHandle{}, TextureHandle(2), 5, 0.5f, 0.5f);
    EXPECT_FALSE(tex.hasTexture());
    EXPECT_TRUE(tex.hasArrayTexture());
    EXPECT_TRUE(tex.isValid());
    EXPECT_EQ(tex.arrayIndex, 5);
    EXPECT_EQ(tex.arrayScaleS, 0.5f);
}

TEST(CoronaTextureTest, GetEffectiveTexture) {
    // Prefers array texture when both present
    CoronaTexture tex(TextureHandle(1), TextureHandle(2));
    EXPECT_EQ(tex.getEffectiveTexture().id, 2);
    
    // Falls back to individual if no array
    CoronaTexture tex2(TextureHandle(3));
    EXPECT_EQ(tex2.getEffectiveTexture().id, 3);
}

TEST(CoronaTextureTest, Clear) {
    CoronaTexture tex(TextureHandle(1), TextureHandle(2), 5, 0.5f, 0.5f);
    tex.clear();
    EXPECT_FALSE(tex.isValid());
    EXPECT_EQ(tex.arrayIndex, 0);
    EXPECT_EQ(tex.arrayScaleS, 1.0f);
}

// ============================================================================
// CameraMode Tests
// ============================================================================

TEST(CameraModeTest, EnumValues) {
    EXPECT_EQ(static_cast<int>(CameraMode::Normal), 0);
    EXPECT_EQ(static_cast<int>(CameraMode::ChaseCam), 1);
    EXPECT_EQ(static_cast<int>(CameraMode::External), 2);
}

TEST(CameraModeTest, Names) {
    EXPECT_EQ(cameraModeName(CameraMode::Normal), "Normal");
    EXPECT_EQ(cameraModeName(CameraMode::ChaseCam), "ChaseCam");
    EXPECT_EQ(cameraModeName(CameraMode::External), "External");
}

TEST(CameraModeTest, IsThirdPerson) {
    EXPECT_FALSE(isThirdPerson(CameraMode::Normal));
    EXPECT_TRUE(isThirdPerson(CameraMode::ChaseCam));
    EXPECT_TRUE(isThirdPerson(CameraMode::External));
}

TEST(CameraModeTest, ShouldShowPlayerModel) {
    EXPECT_FALSE(shouldShowPlayerModel(CameraMode::Normal));
    EXPECT_TRUE(shouldShowPlayerModel(CameraMode::ChaseCam));
    EXPECT_TRUE(shouldShowPlayerModel(CameraMode::External));
}

// ============================================================================
// Position3D Tests
// ============================================================================

TEST(Position3DTest, DefaultConstruction) {
    Position3D pos;
    EXPECT_EQ(pos.x, 0.0f);
    EXPECT_EQ(pos.y, 0.0f);
    EXPECT_EQ(pos.z, 0.0f);
}

TEST(Position3DTest, ValueConstruction) {
    Position3D pos(1.0f, 2.0f, 3.0f);
    EXPECT_EQ(pos.x, 1.0f);
    EXPECT_EQ(pos.y, 2.0f);
    EXPECT_EQ(pos.z, 3.0f);
}

TEST(Position3DTest, DistanceSquared) {
    Position3D p1(0.0f, 0.0f, 0.0f);
    Position3D p2(3.0f, 4.0f, 0.0f);
    EXPECT_FLOAT_EQ(p1.distanceSquared(p2), 25.0f);  // 3^2 + 4^2 = 25
}

TEST(Position3DTest, Clear) {
    Position3D pos(1.0f, 2.0f, 3.0f);
    pos.clear();
    EXPECT_EQ(pos.x, 0.0f);
    EXPECT_EQ(pos.y, 0.0f);
    EXPECT_EQ(pos.z, 0.0f);
}

// ============================================================================
// Corona Tests
// ============================================================================

TEST(CoronaTest, DefaultConstruction) {
    Corona corona;
    EXPECT_FALSE(corona.active);
    EXPECT_EQ(corona.type, CoronaType::Free);
    EXPECT_EQ(corona.entityId, -1);
}

TEST(CoronaTest, TypeConstruction) {
    Position3D pos(100.0f, 200.0f, 50.0f);
    Corona corona(pos, CoronaType::Flash);
    
    EXPECT_TRUE(corona.active);
    EXPECT_EQ(corona.type, CoronaType::Flash);
    EXPECT_EQ(corona.origin.x, 100.0f);
    EXPECT_GT(corona.getRadius(), 0.0f);
}

TEST(CoronaTest, GetRadius) {
    Corona corona(Position3D{}, CoronaType::Flash);
    float defaultRadius = corona.getRadius();
    
    corona.radius = 100.0f;
    EXPECT_EQ(corona.getRadius(), 100.0f);  // Custom overrides
    
    corona.radius = 0.0f;
    EXPECT_EQ(corona.getRadius(), defaultRadius);  // Falls back to default
}

TEST(CoronaTest, GetLifetime) {
    Corona corona(Position3D{}, CoronaType::Flash);
    float defaultLifetime = corona.getLifetime();
    
    corona.lifetime = 2.0f;
    EXPECT_EQ(corona.getLifetime(), 2.0f);  // Custom overrides
    
    corona.lifetime = 0.0f;
    EXPECT_EQ(corona.getLifetime(), defaultLifetime);  // Falls back to default
}

TEST(CoronaTest, IsAlive) {
    Corona corona(Position3D{}, CoronaType::Flash);
    corona.spawnTime = 0.0f;
    
    EXPECT_TRUE(corona.isAlive(0.1f));  // Just spawned
    EXPECT_FALSE(corona.isAlive(10.0f));  // Long expired
    
    // Inactive corona is never alive
    corona.active = false;
    EXPECT_FALSE(corona.isAlive(0.0f));
}

TEST(CoronaTest, IsAliveContinuous) {
    Corona corona(Position3D{}, CoronaType::Fire);  // Continuous type
    corona.spawnTime = 0.0f;
    
    // Continuous coronas stay alive forever
    EXPECT_TRUE(corona.isAlive(0.0f));
    EXPECT_TRUE(corona.isAlive(1000.0f));
}

TEST(CoronaTest, CalculateAlpha) {
    Corona corona(Position3D{}, CoronaType::Flash);
    corona.spawnTime = 0.0f;
    corona.alpha = 1.0f;
    float lifetime = corona.getLifetime();
    
    EXPECT_FLOAT_EQ(corona.calculateAlpha(0.0f), 1.0f);  // Start
    EXPECT_LT(corona.calculateAlpha(lifetime * 0.5f), 1.0f);  // Middle
    EXPECT_FLOAT_EQ(corona.calculateAlpha(lifetime * 2.0f), 0.0f);  // Expired
}

TEST(CoronaTest, CalculateFrame) {
    Corona corona(Position3D{}, CoronaType::Explode);
    corona.spawnTime = 0.0f;
    
    EXPECT_EQ(corona.calculateFrame(0.0f), 0);
    EXPECT_GE(corona.calculateFrame(0.2f), 1);
    EXPECT_LE(corona.calculateFrame(10.0f), constants::EXPLOSION_FRAMES - 1);
}

TEST(CoronaTest, GetTextureId) {
    Corona flash(Position3D{}, CoronaType::Flash);
    EXPECT_EQ(flash.getTextureId(0.0f), CoronaTextureId::Standard);
    
    Corona gunflash(Position3D{}, CoronaType::GunFlash);
    EXPECT_EQ(gunflash.getTextureId(0.0f), CoronaTextureId::GunFlash);
    
    Corona explode(Position3D{}, CoronaType::Explode);
    EXPECT_TRUE(isExplosionTexture(explode.getTextureId(0.0f)));
}

TEST(CoronaTest, Deactivate) {
    Corona corona(Position3D{}, CoronaType::Flash);
    EXPECT_TRUE(corona.active);
    
    corona.deactivate();
    EXPECT_FALSE(corona.active);
    EXPECT_EQ(corona.alpha, 0.0f);
}

TEST(CoronaTest, Clear) {
    Corona corona(Position3D(1.0f, 2.0f, 3.0f), CoronaType::Flash);
    corona.entityId = 42;
    corona.clear();
    
    EXPECT_FALSE(corona.active);
    EXPECT_EQ(corona.type, CoronaType::Free);
    EXPECT_EQ(corona.entityId, -1);
    EXPECT_EQ(corona.origin.x, 0.0f);
}

// ============================================================================
// CoronaTextureArray Tests
// ============================================================================

TEST(CoronaTextureArrayTest, DefaultConstruction) {
    CoronaTextureArray arr;
    EXPECT_EQ(arr.size(), static_cast<std::size_t>(CoronaTextureId::Count));
    EXPECT_FALSE(arr.allLoaded());
    EXPECT_EQ(arr.countLoaded(), 0);
}

TEST(CoronaTextureArrayTest, GetSet) {
    CoronaTextureArray arr;
    arr.set(CoronaTextureId::Standard, CoronaTexture(TextureHandle(1)));
    
    EXPECT_TRUE(arr.get(CoronaTextureId::Standard).isValid());
    EXPECT_FALSE(arr.get(CoronaTextureId::GunFlash).isValid());
}

TEST(CoronaTextureArrayTest, IndexOperator) {
    CoronaTextureArray arr;
    arr[0] = CoronaTexture(TextureHandle(1));
    
    EXPECT_TRUE(arr[0].isValid());
    EXPECT_FALSE(arr[1].isValid());
}

TEST(CoronaTextureArrayTest, Clear) {
    CoronaTextureArray arr;
    arr.set(CoronaTextureId::Standard, CoronaTexture(TextureHandle(1)));
    arr.clear();
    
    EXPECT_FALSE(arr.get(CoronaTextureId::Standard).isValid());
    EXPECT_EQ(arr.countLoaded(), 0);
}

// ============================================================================
// CoronaStats Tests
// ============================================================================

TEST(CoronaStatsTest, DefaultState) {
    CoronaStats stats;
    EXPECT_EQ(stats.activeCount, 0);
    EXPECT_EQ(stats.peakCount, 0);
    EXPECT_EQ(stats.totalSpawned, 0);
}

TEST(CoronaStatsTest, RecordSpawn) {
    CoronaStats stats;
    stats.recordSpawn();
    stats.recordSpawn();
    
    EXPECT_EQ(stats.activeCount, 2);
    EXPECT_EQ(stats.peakCount, 2);
    EXPECT_EQ(stats.totalSpawned, 2);
}

TEST(CoronaStatsTest, RecordExpire) {
    CoronaStats stats;
    stats.recordSpawn();
    stats.recordSpawn();
    stats.recordExpire();
    
    EXPECT_EQ(stats.activeCount, 1);
    EXPECT_EQ(stats.peakCount, 2);  // Peak preserved
    EXPECT_EQ(stats.totalExpired, 1);
}

TEST(CoronaStatsTest, Reset) {
    CoronaStats stats;
    stats.recordSpawn();
    stats.recordCull();
    stats.reset();
    
    EXPECT_EQ(stats.activeCount, 0);
    EXPECT_EQ(stats.peakCount, 0);
    EXPECT_EQ(stats.culledCount, 0);
}

// ============================================================================
// CoronaConfig Tests
// ============================================================================

TEST(CoronaConfigTest, DefaultState) {
    CoronaConfig config;
    EXPECT_TRUE(config.enabled);
    EXPECT_TRUE(config.teleporterCoronas);
    EXPECT_EQ(config.brightness, 1.0f);
    EXPECT_EQ(config.sizeScale, 1.0f);
}

TEST(CoronaConfigTest, ApplyBrightness) {
    CoronaConfig config;
    config.brightness = 0.5f;
    
    CoronaColor color(1.0f, 0.8f, 0.6f);
    auto result = config.applyBrightness(color);
    
    EXPECT_EQ(result.r, 0.5f);
    EXPECT_EQ(result.g, 0.4f);
    EXPECT_EQ(result.b, 0.3f);
}

TEST(CoronaConfigTest, ApplyScale) {
    CoronaConfig config;
    config.sizeScale = 2.0f;
    
    EXPECT_EQ(config.applyScale(10.0f), 20.0f);
}

TEST(CoronaConfigTest, Reset) {
    CoronaConfig config;
    config.enabled = false;
    config.brightness = 0.5f;
    config.reset();
    
    EXPECT_TRUE(config.enabled);
    EXPECT_EQ(config.brightness, 1.0f);
}

// ============================================================================
// WeatherType Tests
// ============================================================================

TEST(WeatherTypeTest, EnumValues) {
    EXPECT_EQ(static_cast<int>(WeatherType::None), 0);
    EXPECT_EQ(static_cast<int>(WeatherType::Rain), 1);
    EXPECT_EQ(static_cast<int>(WeatherType::Snow), 2);
    EXPECT_EQ(static_cast<int>(WeatherType::Sleet), 3);
}

TEST(WeatherTypeTest, Names) {
    EXPECT_EQ(weatherTypeName(WeatherType::None), "None");
    EXPECT_EQ(weatherTypeName(WeatherType::Rain), "Rain");
    EXPECT_EQ(weatherTypeName(WeatherType::Snow), "Snow");
    EXPECT_EQ(weatherTypeName(WeatherType::Sleet), "Sleet");
}

// ============================================================================
// WeatherConfig Tests
// ============================================================================

TEST(WeatherConfigTest, DefaultState) {
    WeatherConfig config;
    EXPECT_EQ(config.type, WeatherType::None);
    EXPECT_FALSE(config.isActive());
}

TEST(WeatherConfigTest, IsActive) {
    WeatherConfig config;
    config.type = WeatherType::Rain;
    config.intensity = 0.5f;
    EXPECT_TRUE(config.isActive());
    
    config.intensity = 0.0f;
    EXPECT_FALSE(config.isActive());
    
    config.intensity = 0.5f;
    config.type = WeatherType::None;
    EXPECT_FALSE(config.isActive());
}

TEST(WeatherConfigTest, Reset) {
    WeatherConfig config;
    config.type = WeatherType::Snow;
    config.intensity = 1.0f;
    config.fastMode = true;
    config.reset();
    
    EXPECT_EQ(config.type, WeatherType::None);
    EXPECT_EQ(config.intensity, 0.5f);
    EXPECT_FALSE(config.fastMode);
}

// ============================================================================
// LightningConfig Tests
// ============================================================================

TEST(LightningConfigTest, DefaultState) {
    LightningConfig config;
    EXPECT_EQ(config.size, 4.0f);
    EXPECT_TRUE(config.sparks);
    EXPECT_EQ(config.sparkSize, 1.0f);
}

TEST(LightningConfigTest, Reset) {
    LightningConfig config;
    config.size = 10.0f;
    config.sparks = false;
    config.reset();
    
    EXPECT_EQ(config.size, 4.0f);
    EXPECT_TRUE(config.sparks);
}

// ============================================================================
// VisualEffectState Tests
// ============================================================================

TEST(VisualEffectStateTest, DefaultState) {
    VisualEffectState state;
    EXPECT_TRUE(state.coronaConfig.enabled);
    EXPECT_EQ(state.cameraMode, CameraMode::Normal);
    EXPECT_EQ(state.particleCount, 0);
    EXPECT_EQ(state.coronaCount, 0);
}

TEST(VisualEffectStateTest, UpdateParticleCount) {
    VisualEffectState state;
    state.updateParticleCount(100);
    EXPECT_EQ(state.particleCount, 100);
    EXPECT_EQ(state.particleCountHigh, 100);
    
    state.updateParticleCount(50);
    EXPECT_EQ(state.particleCount, 50);
    EXPECT_EQ(state.particleCountHigh, 100);  // Peak preserved
    
    state.updateParticleCount(150);
    EXPECT_EQ(state.particleCountHigh, 150);  // New peak
}

TEST(VisualEffectStateTest, UpdateCoronaCount) {
    VisualEffectState state;
    state.updateCoronaCount(50);
    EXPECT_EQ(state.coronaCount, 50);
    EXPECT_EQ(state.coronaCountHigh, 50);
    
    state.updateCoronaCount(25);
    EXPECT_EQ(state.coronaCountHigh, 50);  // Peak preserved
}

TEST(VisualEffectStateTest, ResetFrame) {
    VisualEffectState state;
    state.coronaStats.recordCull();
    state.resetFrame();
    
    EXPECT_EQ(state.coronaStats.culledCount, 0);
}

TEST(VisualEffectStateTest, FullReset) {
    VisualEffectState state;
    state.cameraMode = CameraMode::ChaseCam;
    state.updateParticleCount(100);
    state.weatherConfig.type = WeatherType::Rain;
    state.reset();
    
    EXPECT_EQ(state.cameraMode, CameraMode::Normal);
    EXPECT_EQ(state.particleCount, 0);
    EXPECT_EQ(state.particleCountHigh, 0);
    EXPECT_EQ(state.weatherConfig.type, WeatherType::None);
}
