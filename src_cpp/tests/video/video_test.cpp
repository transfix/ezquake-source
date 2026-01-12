/**
 * @file video_test.cpp
 * @brief Unit tests for video/display types
 */

#include <gtest/gtest.h>
#include "../../core/video/video_types.hpp"

using namespace ezquake::video;

// ============================================================================
// RenderFlag Tests
// ============================================================================

TEST(RenderFlagTest, BitwiseOr) {
    RenderFlag combined = RenderFlag::WeaponModel | RenderFlag::NoShadow;
    EXPECT_TRUE(hasFlag(combined, RenderFlag::WeaponModel));
    EXPECT_TRUE(hasFlag(combined, RenderFlag::NoShadow));
    EXPECT_FALSE(hasFlag(combined, RenderFlag::PlayerModel));
}

TEST(RenderFlagTest, BitwiseAnd) {
    RenderFlag flags = RenderFlag::WeaponModel | RenderFlag::NoShadow | RenderFlag::PlayerModel;
    RenderFlag masked = flags & RenderFlag::WeaponModel;
    EXPECT_TRUE(hasFlag(masked, RenderFlag::WeaponModel));
}

TEST(RenderFlagTest, HasFlag) {
    EXPECT_TRUE(hasFlag(RenderFlag::BackpackFlags, RenderFlag::RocketPack));
    EXPECT_TRUE(hasFlag(RenderFlag::BackpackFlags, RenderFlag::LGPack));
    EXPECT_FALSE(hasFlag(RenderFlag::BackpackFlags, RenderFlag::WeaponModel));
}

// ============================================================================
// VideoRect Tests
// ============================================================================

TEST(VideoRectTest, DefaultValues) {
    VideoRect rect;
    EXPECT_EQ(rect.x, 0);
    EXPECT_EQ(rect.y, 0);
    EXPECT_EQ(rect.width, 0);
    EXPECT_EQ(rect.height, 0);
    EXPECT_FALSE(rect.isValid());
}

TEST(VideoRectTest, IsValid) {
    VideoRect invalid{0, 0, 0, 0};
    EXPECT_FALSE(invalid.isValid());

    VideoRect valid{0, 0, 100, 100};
    EXPECT_TRUE(valid.isValid());

    VideoRect negativeWidth{0, 0, -10, 100};
    EXPECT_FALSE(negativeWidth.isValid());
}

TEST(VideoRectTest, Area) {
    VideoRect rect{0, 0, 100, 50};
    EXPECT_EQ(rect.area(), 5000);
}

TEST(VideoRectTest, RightBottom) {
    VideoRect rect{10, 20, 100, 50};
    EXPECT_EQ(rect.right(), 110);
    EXPECT_EQ(rect.bottom(), 70);
}

TEST(VideoRectTest, Aspect) {
    VideoRect rect{0, 0, 1920, 1080};
    EXPECT_NEAR(rect.aspect(), 16.0f / 9.0f, 0.01f);

    VideoRect rect43{0, 0, 640, 480};
    EXPECT_NEAR(rect43.aspect(), 4.0f / 3.0f, 0.01f);
}

TEST(VideoRectTest, AspectZeroHeight) {
    VideoRect rect{0, 0, 100, 0};
    EXPECT_FLOAT_EQ(rect.aspect(), 1.0f);
}

TEST(VideoRectTest, Contains) {
    VideoRect rect{10, 10, 100, 100};
    EXPECT_TRUE(rect.contains(50, 50));
    EXPECT_TRUE(rect.contains(10, 10));
    EXPECT_FALSE(rect.contains(110, 50));  // right edge is exclusive
    EXPECT_FALSE(rect.contains(5, 50));
}

TEST(VideoRectTest, Overlaps) {
    VideoRect rect1{0, 0, 100, 100};
    VideoRect rect2{50, 50, 100, 100};
    EXPECT_TRUE(rect1.overlaps(rect2));

    VideoRect rect3{200, 200, 50, 50};
    EXPECT_FALSE(rect1.overlaps(rect3));
}

TEST(VideoRectTest, FromSize) {
    VideoRect rect = VideoRect::fromSize(800, 600);
    EXPECT_EQ(rect.x, 0);
    EXPECT_EQ(rect.y, 0);
    EXPECT_EQ(rect.width, 800);
    EXPECT_EQ(rect.height, 600);
}

TEST(VideoRectTest, FromBounds) {
    VideoRect rect = VideoRect::fromBounds(10, 20, 800, 600);
    EXPECT_EQ(rect.x, 10);
    EXPECT_EQ(rect.y, 20);
    EXPECT_EQ(rect.width, 800);
    EXPECT_EQ(rect.height, 600);
}

TEST(VideoRectTest, Equality) {
    VideoRect rect1{10, 20, 100, 100};
    VideoRect rect2{10, 20, 100, 100};
    VideoRect rect3{10, 20, 100, 50};
    EXPECT_EQ(rect1, rect2);
    EXPECT_NE(rect1, rect3);
}

// ============================================================================
// VideoDefinition Tests
// ============================================================================

TEST(VideoDefinitionTest, DefaultValues) {
    VideoDefinition def;
    EXPECT_EQ(def.width, 640);
    EXPECT_EQ(def.height, 480);
    EXPECT_NEAR(def.aspect, 4.0f / 3.0f, 0.01f);
}

TEST(VideoDefinitionTest, PixelCount) {
    VideoDefinition def;
    def.width = 1920;
    def.height = 1080;
    EXPECT_EQ(def.pixelCount(), 1920 * 1080);
}

TEST(VideoDefinitionTest, ConsoleScale) {
    VideoDefinition def;
    def.width = 1920;
    def.conWidth = 960;
    EXPECT_FLOAT_EQ(def.consoleScale(), 2.0f);
}

TEST(VideoDefinitionTest, ConsoleScaleZero) {
    VideoDefinition def;
    def.conWidth = 0;
    EXPECT_FLOAT_EQ(def.consoleScale(), 1.0f);
}

TEST(VideoDefinitionTest, IsWidescreen) {
    VideoDefinition def;
    def.aspect = 4.0f / 3.0f;
    EXPECT_FALSE(def.isWidescreen());

    def.aspect = 16.0f / 9.0f;
    EXPECT_TRUE(def.isWidescreen());
}

TEST(VideoDefinitionTest, UpdateAspect) {
    VideoDefinition def;
    def.width = 1920;
    def.height = 1080;
    def.updateAspect();
    EXPECT_NEAR(def.aspect, 16.0f / 9.0f, 0.01f);
}

TEST(VideoDefinitionTest, Reset) {
    VideoDefinition def;
    def.width = 1920;
    def.height = 1080;
    def.reset();
    EXPECT_EQ(def.width, 640);
    EXPECT_EQ(def.height, 480);
}

// ============================================================================
// FogCalculation Tests
// ============================================================================

TEST(FogCalculationTest, Names) {
    EXPECT_EQ(fogCalculationName(FogCalculation::None), "none");
    EXPECT_EQ(fogCalculationName(FogCalculation::Linear), "linear");
    EXPECT_EQ(fogCalculationName(FogCalculation::Exponential), "exp");
    EXPECT_EQ(fogCalculationName(FogCalculation::ExponentialSquared), "exp2");
}

// ============================================================================
// FogSettings Tests
// ============================================================================

TEST(FogSettingsTest, ShouldApply) {
    FogSettings fog;
    EXPECT_FALSE(fog.shouldApply());

    fog.enabled = true;
    fog.render = true;
    fog.calculation = FogCalculation::Linear;
    EXPECT_TRUE(fog.shouldApply());
}

TEST(FogSettingsTest, CalculateLinearFog) {
    FogSettings fog;
    fog.linearStart = 0.0f;
    fog.linearEnd = 1000.0f;

    EXPECT_FLOAT_EQ(fog.calculateLinearFog(0.0f), 1.0f);
    EXPECT_FLOAT_EQ(fog.calculateLinearFog(500.0f), 0.5f);
    EXPECT_FLOAT_EQ(fog.calculateLinearFog(1000.0f), 0.0f);
}

TEST(FogSettingsTest, CalculateExpFog) {
    FogSettings fog;
    fog.density = 0.001f;
    float factor = fog.calculateExpFog(1000.0f);
    EXPECT_NEAR(factor, std::exp(-1.0f), 0.01f);
}

TEST(FogSettingsTest, Reset) {
    FogSettings fog;
    fog.enabled = true;
    fog.density = 0.5f;
    fog.reset();
    EXPECT_FALSE(fog.enabled);
    EXPECT_FLOAT_EQ(fog.density, 0.0f);
}

// ============================================================================
// RefreshDefinition Tests
// ============================================================================

TEST(RefreshDefinitionTest, UpdateFromViewRect) {
    RefreshDefinition refdef;
    refdef.viewRect = {10, 20, 800, 600};
    refdef.updateFromViewRect();

    EXPECT_EQ(refdef.rectRight, 810);
    EXPECT_EQ(refdef.rectBottom, 620);
    EXPECT_FLOAT_EQ(refdef.floatRectX, 10.0f);
    EXPECT_FLOAT_EQ(refdef.floatRectY, 20.0f);
}

TEST(RefreshDefinitionTest, DefaultFov) {
    RefreshDefinition refdef;
    EXPECT_FLOAT_EQ(refdef.fovX, 90.0f);
}

TEST(RefreshDefinitionTest, Reset) {
    RefreshDefinition refdef;
    refdef.viewRect = {10, 20, 800, 600};
    refdef.fovX = 110.0f;
    refdef.reset();
    EXPECT_EQ(refdef.viewRect.width, 0);
    EXPECT_FLOAT_EQ(refdef.fovX, 90.0f);
}

// ============================================================================
// ExtendedRefreshDefinition Tests
// ============================================================================

TEST(ExtendedRefreshDefinitionTest, UpdateTime) {
    ExtendedRefreshDefinition refdef2;
    refdef2.updateTime(3.14159265);
    EXPECT_NEAR(refdef2.cosTime, -1.0f, 0.01f);
    EXPECT_NEAR(refdef2.sinTime, 0.0f, 0.01f);
}

TEST(ExtendedRefreshDefinitionTest, Reset) {
    ExtendedRefreshDefinition refdef2;
    refdef2.time = 100.0;
    refdef2.allowCheats = true;
    refdef2.reset();
    EXPECT_DOUBLE_EQ(refdef2.time, 0.0);
    EXPECT_FALSE(refdef2.allowCheats);
}

// ============================================================================
// DisplayMode Tests
// ============================================================================

TEST(DisplayModeTest, Aspect) {
    DisplayMode mode{1920, 1080, 60, 32, false};
    EXPECT_NEAR(mode.aspect(), 16.0f / 9.0f, 0.01f);
}

TEST(DisplayModeTest, IsWidescreen) {
    DisplayMode mode43{640, 480, 60, 32, false};
    EXPECT_FALSE(mode43.isWidescreen());

    DisplayMode mode169{1920, 1080, 60, 32, false};
    EXPECT_TRUE(mode169.isWidescreen());
}

TEST(DisplayModeTest, PixelCount) {
    DisplayMode mode{1920, 1080, 60, 32, false};
    EXPECT_EQ(mode.pixelCount(), 1920 * 1080);
}

TEST(DisplayModeTest, ToString) {
    DisplayMode mode{1920, 1080, 144, 32, false};
    EXPECT_EQ(mode.toString(), "1920x1080@144Hz");
}

TEST(DisplayModeTest, Equality) {
    DisplayMode mode1{1920, 1080, 60, 32, false};
    DisplayMode mode2{1920, 1080, 60, 32, false};
    DisplayMode mode3{1920, 1080, 144, 32, false};
    EXPECT_EQ(mode1, mode2);
    EXPECT_NE(mode1, mode3);
}

TEST(DisplayModeTest, Comparison) {
    DisplayMode mode720{1280, 720, 60, 32, false};
    DisplayMode mode1080{1920, 1080, 60, 32, false};
    EXPECT_TRUE(mode720 < mode1080);
}

// ============================================================================
// GammaSettings Tests
// ============================================================================

TEST(GammaSettingsTest, ApplyGamma) {
    GammaSettings gamma;
    gamma.gamma = 2.0f;
    gamma.contrast = 1.0f;
    float result = gamma.applyGamma(0.25f);
    EXPECT_FLOAT_EQ(result, 0.5f);  // sqrt(0.25) = 0.5
}

TEST(GammaSettingsTest, GenerateRamp) {
    GammaSettings gamma;
    gamma.gamma = 1.0f;
    gamma.contrast = 1.0f;
    gamma.generateRamp();
    // Linear ramp: first should be 0, last should be 65535
    EXPECT_EQ(gamma.redRamp[0], 0);
    EXPECT_EQ(gamma.redRamp[255], 65535);
}

TEST(GammaSettingsTest, Reset) {
    GammaSettings gamma;
    gamma.gamma = 2.5f;
    gamma.reset();
    EXPECT_FLOAT_EQ(gamma.gamma, 1.0f);
}

// ============================================================================
// VSyncConfig Tests
// ============================================================================

TEST(VSyncConfigTest, IsActive) {
    VSyncConfig config;
    config.mode = VSyncMode::On;
    EXPECT_TRUE(config.isActive());

    config.mode = VSyncMode::Off;
    EXPECT_FALSE(config.isActive());

    config.mode = VSyncMode::Adaptive;
    EXPECT_TRUE(config.isActive());
}

TEST(VSyncModeTest, Values) {
    EXPECT_EQ(static_cast<int>(VSyncMode::Off), 0);
    EXPECT_EQ(static_cast<int>(VSyncMode::On), 1);
    EXPECT_EQ(static_cast<int>(VSyncMode::Adaptive), -1);
}

// ============================================================================
// WindowState Tests
// ============================================================================

TEST(WindowStateTest, IsActive) {
    WindowState state;
    EXPECT_TRUE(state.isActive());  // default: focused and not minimized

    state.minimized = true;
    EXPECT_FALSE(state.isActive());

    state.minimized = false;
    state.focused = false;
    EXPECT_FALSE(state.isActive());
}

TEST(WindowStateTest, Reset) {
    WindowState state;
    state.minimized = true;
    state.focused = false;
    state.reset();
    EXPECT_FALSE(state.minimized);
    EXPECT_TRUE(state.focused);
}

// ============================================================================
// CustomModelColor Tests
// ============================================================================

TEST(CustomModelColorTest, IsActive) {
    CustomModelColor color;
    EXPECT_FALSE(color.isActive());

    color.color[0] = 0.5f;
    EXPECT_TRUE(color.isActive());

    color = CustomModelColor{};
    color.fullbright = true;
    EXPECT_TRUE(color.isActive());
}

// ============================================================================
// VideoState Tests
// ============================================================================

TEST(VideoStateTest, SetResolution) {
    VideoState state;
    state.setResolution(1920, 1080);
    EXPECT_EQ(state.definition.width, 1920);
    EXPECT_EQ(state.definition.height, 1080);
    EXPECT_NEAR(state.definition.aspect, 16.0f / 9.0f, 0.01f);
    EXPECT_TRUE(state.definition.recalcRefdef);
}

TEST(VideoStateTest, InvalidateRefdef) {
    VideoState state;
    state.definition.recalcRefdef = false;
    state.invalidateRefdef();
    EXPECT_TRUE(state.definition.recalcRefdef);
}

TEST(VideoStateTest, Reset) {
    VideoState state;
    state.setResolution(1920, 1080);
    state.initialized = true;
    state.reset();
    EXPECT_EQ(state.definition.width, 640);
    EXPECT_FALSE(state.initialized);
}

// ============================================================================
// Factory Function Tests
// ============================================================================

TEST(FactoryTest, CreateCommonModes) {
    auto modes = createCommonModes();
    EXPECT_EQ(modes.size(), 5u);
    EXPECT_EQ(modes[0].width, 640);
    EXPECT_EQ(modes[4].width, 3840);  // 4K
}

TEST(FactoryTest, CalcFovAdjustment) {
    // At 4:3, no adjustment
    float fov43 = calcFovAdjustment(90.0f, 4.0f / 3.0f);
    EXPECT_FLOAT_EQ(fov43, 90.0f);

    // At 16:9, should be wider
    float fov169 = calcFovAdjustment(90.0f, 16.0f / 9.0f);
    EXPECT_GT(fov169, 90.0f);
    EXPECT_LT(fov169, 120.0f);
}

// ============================================================================
// Color Range Tests
// ============================================================================

TEST(ColorRangeTest, Values) {
    EXPECT_EQ(color_range::TOP, 16);
    EXPECT_EQ(color_range::BOTTOM, 96);
    EXPECT_EQ(color_range::RANGE_SIZE, 16);
}
