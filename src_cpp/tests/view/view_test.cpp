/**
 * @file view_test.cpp
 * @brief Tests for view and camera subsystem
 */

#include <gtest/gtest.h>
#include "core/view/view.hpp"
#include <cmath>

using namespace ezquake;

//=============================================================================
// View Bob Constants Tests
//=============================================================================

TEST(ViewBobConstantsTest, DefaultValues) {
    EXPECT_FLOAT_EQ(view_bob::DEFAULT_BOB, 0.0f);
    EXPECT_FLOAT_EQ(view_bob::DEFAULT_BOBCYCLE, 0.0f);
    EXPECT_FLOAT_EQ(view_bob::DEFAULT_BOBUP, 0.0f);
    EXPECT_FLOAT_EQ(view_bob::MIN_BOB, -7.0f);
    EXPECT_FLOAT_EQ(view_bob::MAX_BOB, 4.0f);
}

//=============================================================================
// View Roll Constants Tests
//=============================================================================

TEST(ViewRollConstantsTest, DefaultValues) {
    EXPECT_FLOAT_EQ(view_roll::DEFAULT_SPEED, 200.0f);
    EXPECT_FLOAT_EQ(view_roll::DEFAULT_ANGLE, 0.0f);
    EXPECT_FLOAT_EQ(view_roll::DEFAULT_ALPHA, 20.0f);
    EXPECT_FLOAT_EQ(view_roll::MAX_ROLL, 45.0f);
}

//=============================================================================
// Color Shift Type Tests
//=============================================================================

TEST(ColorShiftTypeTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(ColorShiftType::Contents), 0);
    EXPECT_EQ(static_cast<uint8_t>(ColorShiftType::Damage), 1);
    EXPECT_EQ(static_cast<uint8_t>(ColorShiftType::Bonus), 2);
    EXPECT_EQ(static_cast<uint8_t>(ColorShiftType::Powerup), 3);
}

TEST(ColorShiftTypeTest, Names) {
    EXPECT_EQ(colorShiftTypeName(ColorShiftType::Contents), "contents");
    EXPECT_EQ(colorShiftTypeName(ColorShiftType::Damage), "damage");
    EXPECT_EQ(colorShiftTypeName(ColorShiftType::Bonus), "bonus");
    EXPECT_EQ(colorShiftTypeName(ColorShiftType::Powerup), "powerup");
}

//=============================================================================
// Color Shift Tests
//=============================================================================

class ViewColorShiftTest : public ::testing::Test {
protected:
    ViewColorShiftState shift_;
};

TEST_F(ViewColorShiftTest, DefaultState) {
    EXPECT_EQ(shift_.destColor[0], 0);
    EXPECT_EQ(shift_.destColor[1], 0);
    EXPECT_EQ(shift_.destColor[2], 0);
    EXPECT_EQ(shift_.percent, 0);
    EXPECT_FALSE(shift_.isActive());
}

TEST_F(ViewColorShiftTest, Set) {
    shift_.set(255, 128, 64, 100);
    
    EXPECT_EQ(shift_.destColor[0], 255);
    EXPECT_EQ(shift_.destColor[1], 128);
    EXPECT_EQ(shift_.destColor[2], 64);
    EXPECT_EQ(shift_.percent, 100);
    EXPECT_TRUE(shift_.isActive());
}

TEST_F(ViewColorShiftTest, PercentClamp) {
    shift_.set(255, 0, 0, 200);
    EXPECT_EQ(shift_.percent, 150);  // Clamped to max
    
    shift_.set(255, 0, 0, -50);
    EXPECT_EQ(shift_.percent, 0);  // Clamped to min
}

TEST_F(ViewColorShiftTest, Alpha) {
    shift_.set(255, 0, 0, 100);
    float alpha = shift_.alpha();
    EXPECT_NEAR(alpha, 100.0f / 255.0f, 0.001f);
}

TEST_F(ViewColorShiftTest, NormalizedColor) {
    shift_.set(255, 128, 0, 100);
    auto color = shift_.normalizedColor();
    
    EXPECT_FLOAT_EQ(color[0], 1.0f);
    EXPECT_NEAR(color[1], 128.0f / 255.0f, 0.001f);
    EXPECT_FLOAT_EQ(color[2], 0.0f);
}

TEST_F(ViewColorShiftTest, Decay) {
    shift_.set(255, 0, 0, 100);
    int before = shift_.percent;
    ASSERT_EQ(before, 100) << "percent should be 100 after set";
    shift_.decay(50.0f);
    int after = shift_.percent;
    EXPECT_EQ(after, 50) << "before=" << before << " after=" << after;
    
    shift_.decay(100.0f);
    EXPECT_EQ(shift_.percent, 0);
}

TEST_F(ViewColorShiftTest, Reset) {
    shift_.set(255, 128, 64, 100);
    shift_.reset();
    
    EXPECT_EQ(shift_.destColor[0], 0);
    EXPECT_EQ(shift_.percent, 0);
    EXPECT_FALSE(shift_.isActive());
}

//=============================================================================
// Contents Type Tests
//=============================================================================

TEST(ContentsTypeTest, Values) {
    EXPECT_EQ(static_cast<int8_t>(ContentsType::Empty), -1);
    EXPECT_EQ(static_cast<int8_t>(ContentsType::Solid), -2);
    EXPECT_EQ(static_cast<int8_t>(ContentsType::Water), -3);
    EXPECT_EQ(static_cast<int8_t>(ContentsType::Slime), -4);
    EXPECT_EQ(static_cast<int8_t>(ContentsType::Lava), -5);
    EXPECT_EQ(static_cast<int8_t>(ContentsType::Sky), -6);
}

TEST(ContentsTypeTest, GetColorShift) {
    auto empty = getContentsColorShift(ContentsType::Empty);
    EXPECT_EQ(empty.percent, color_shifts::EMPTY_PERCENT);
    
    auto lava = getContentsColorShift(ContentsType::Lava);
    EXPECT_EQ(lava.percent, color_shifts::LAVA_PERCENT);
    EXPECT_EQ(lava.destColor[0], color_shifts::LAVA_COLOR[0]);
    
    auto water = getContentsColorShift(ContentsType::Water);
    EXPECT_EQ(water.percent, color_shifts::WATER_PERCENT);
}

//=============================================================================
// View Blend Tests
//=============================================================================

class ViewBlendTest : public ::testing::Test {
protected:
    ViewBlend blend_;
};

TEST_F(ViewBlendTest, DefaultState) {
    EXPECT_FLOAT_EQ(blend_.r, 0.0f);
    EXPECT_FLOAT_EQ(blend_.g, 0.0f);
    EXPECT_FLOAT_EQ(blend_.b, 0.0f);
    EXPECT_FLOAT_EQ(blend_.a, 0.0f);
    EXPECT_FALSE(blend_.isActive());
}

TEST_F(ViewBlendTest, AddBlend) {
    blend_.addBlend(1.0f, 0.0f, 0.0f, 0.5f);
    
    EXPECT_TRUE(blend_.isActive());
    EXPECT_GT(blend_.r, 0.0f);
    EXPECT_FLOAT_EQ(blend_.g, 0.0f);
}

TEST_F(ViewBlendTest, MultipleBlends) {
    blend_.addBlend(1.0f, 0.0f, 0.0f, 0.5f);  // Red
    blend_.addBlend(0.0f, 0.0f, 1.0f, 0.5f);  // Blue
    
    EXPECT_TRUE(blend_.isActive());
    // Should have some of both colors
    EXPECT_GT(blend_.r, 0.0f);
    EXPECT_GT(blend_.b, 0.0f);
}

TEST_F(ViewBlendTest, ToArray) {
    blend_.addBlend(1.0f, 0.5f, 0.25f, 0.5f);
    auto arr = blend_.toArray();
    
    EXPECT_FLOAT_EQ(arr[0], blend_.r);
    EXPECT_FLOAT_EQ(arr[1], blend_.g);
    EXPECT_FLOAT_EQ(arr[2], blend_.b);
    EXPECT_LE(arr[3], 1.0f);  // Alpha is clamped
}

TEST_F(ViewBlendTest, Reset) {
    blend_.addBlend(1.0f, 0.0f, 0.0f, 0.5f);
    blend_.reset();
    
    EXPECT_FALSE(blend_.isActive());
    EXPECT_FLOAT_EQ(blend_.a, 0.0f);
}

//=============================================================================
// View Bob Tests
//=============================================================================

class ViewBobTest : public ::testing::Test {
protected:
    ViewBob bob_;
};

TEST_F(ViewBobTest, DefaultState) {
    EXPECT_DOUBLE_EQ(bob_.bobTime, 0.0);
    EXPECT_FLOAT_EQ(bob_.lastBob, 0.0f);
    EXPECT_FLOAT_EQ(bob_.bobAmount, view_bob::DEFAULT_BOB);
}

TEST_F(ViewBobTest, NoBobWhenCycleZero) {
    bob_.bobCycle = 0.0f;
    float result = bob_.calculate(true, 100.0f, 0.016);
    EXPECT_FLOAT_EQ(result, 0.0f);
}

TEST_F(ViewBobTest, KeepsLastBobInAir) {
    bob_.bobAmount = 0.02f;
    bob_.bobCycle = 0.6f;
    bob_.bobUp = 0.5f;
    
    bob_.calculate(true, 100.0f, 0.016);  // On ground
    float lastBob = bob_.lastBob;
    
    float inAir = bob_.calculate(false, 100.0f, 0.016);  // In air
    EXPECT_FLOAT_EQ(inAir, lastBob);
}

TEST_F(ViewBobTest, BobClamped) {
    bob_.bobAmount = 1.0f;  // Very high
    bob_.bobCycle = 0.6f;
    bob_.bobUp = 0.5f;
    
    float result = bob_.calculate(true, 1000.0f, 0.016);  // High velocity
    EXPECT_LE(result, view_bob::MAX_BOB);
    EXPECT_GE(result, view_bob::MIN_BOB);
}

TEST_F(ViewBobTest, Reset) {
    bob_.bobTime = 1.0;
    bob_.lastBob = 2.0f;
    bob_.reset();
    
    EXPECT_DOUBLE_EQ(bob_.bobTime, 0.0);
    EXPECT_FLOAT_EQ(bob_.lastBob, 0.0f);
}

//=============================================================================
// View Roll Tests
//=============================================================================

class ViewRollTest : public ::testing::Test {
protected:
    ViewRoll roll_;
};

TEST_F(ViewRollTest, DefaultState) {
    EXPECT_FLOAT_EQ(roll_.rollAngle, 0.0f);
    EXPECT_FLOAT_EQ(roll_.rollSpeed, view_roll::DEFAULT_SPEED);
}

TEST_F(ViewRollTest, CalculateTargetPositive) {
    roll_.maxRollAngle = 2.0f;
    float target = roll_.calculateTarget(100.0f);
    EXPECT_GT(target, 0.0f);
}

TEST_F(ViewRollTest, CalculateTargetNegative) {
    roll_.maxRollAngle = 2.0f;
    float target = roll_.calculateTarget(-100.0f);
    EXPECT_LT(target, 0.0f);
}

TEST_F(ViewRollTest, TargetClamped) {
    roll_.maxRollAngle = 100.0f;  // Very high
    roll_.rollSpeed = 1.0f;
    float target = roll_.calculateTarget(1000.0f);
    EXPECT_LE(std::abs(target), view_roll::MAX_ROLL);
}

TEST_F(ViewRollTest, UpdateSmooths) {
    roll_.rollAngle = 0.0f;
    roll_.maxRollAngle = 2.0f;
    roll_.rollAlpha = 20.0f;
    
    roll_.update(2.0f, 0.1f);  // Target is 2.0
    EXPECT_GT(roll_.rollAngle, 0.0f);  // Should have moved towards target
}

//=============================================================================
// Damage Kick Tests
//=============================================================================

class DamageKickTest : public ::testing::Test {
protected:
    DamageKick kick_;
};

TEST_F(DamageKickTest, DefaultState) {
    EXPECT_FLOAT_EQ(kick_.time, 0.0f);
    EXPECT_FLOAT_EQ(kick_.roll, 0.0f);
    EXPECT_FLOAT_EQ(kick_.pitch, 0.0f);
    EXPECT_FALSE(kick_.isActive());
}

TEST_F(DamageKickTest, ApplyDamage) {
    kick_.kickTime = 0.5f;
    kick_.kickRoll = 0.6f;
    kick_.kickPitch = 0.8f;
    
    kick_.applyDamage(50.0f, 0.5f, 0.7f);
    
    EXPECT_TRUE(kick_.isActive());
    EXPECT_FLOAT_EQ(kick_.time, 0.5f);
    EXPECT_FLOAT_EQ(kick_.roll, 50.0f * 0.5f * 0.6f);
    EXPECT_FLOAT_EQ(kick_.pitch, 50.0f * 0.7f * 0.8f);
}

TEST_F(DamageKickTest, CurrentValues) {
    kick_.kickTime = 1.0f;
    kick_.applyDamage(50.0f, 0.5f, 0.7f);
    
    // At full time, should get full kick
    float currentRoll = kick_.currentRoll();
    EXPECT_FLOAT_EQ(currentRoll, kick_.roll);
    
    // After half time decay
    kick_.update(0.5f);
    currentRoll = kick_.currentRoll();
    EXPECT_FLOAT_EQ(currentRoll, kick_.roll * 0.5f);
}

TEST_F(DamageKickTest, Update) {
    kick_.kickTime = 1.0f;
    kick_.applyDamage(50.0f, 0.5f, 0.7f);
    
    kick_.update(0.5f);
    EXPECT_FLOAT_EQ(kick_.time, 0.5f);
    
    kick_.update(1.0f);
    EXPECT_FLOAT_EQ(kick_.time, 0.0f);
    EXPECT_FALSE(kick_.isActive());
}

//=============================================================================
// Pitch Drift Tests
//=============================================================================

class PitchDriftTest : public ::testing::Test {
protected:
    PitchDrift drift_;
};

TEST_F(PitchDriftTest, DefaultState) {
    EXPECT_FALSE(drift_.enabled);
    EXPECT_FLOAT_EQ(drift_.velocity, 0.0f);
}

TEST_F(PitchDriftTest, Start) {
    drift_.start(1.0);
    EXPECT_TRUE(drift_.enabled);
    EXPECT_GT(drift_.velocity, 0.0f);
}

TEST_F(PitchDriftTest, Stop) {
    drift_.start(1.0);
    drift_.stop(2.0);
    
    EXPECT_FALSE(drift_.enabled);
    EXPECT_FLOAT_EQ(drift_.velocity, 0.0f);
    EXPECT_DOUBLE_EQ(drift_.lastStop, 2.0);
}

TEST_F(PitchDriftTest, NoUpdateInAir) {
    drift_.start(1.0);
    float adjust = drift_.update(10.0f, 200.0f, false, false, 0.016f);
    EXPECT_FLOAT_EQ(adjust, 0.0f);
}

TEST_F(PitchDriftTest, NoDriftInDemo) {
    drift_.start(1.0);
    float adjust = drift_.update(10.0f, 200.0f, true, true, 0.016f);
    EXPECT_FLOAT_EQ(adjust, 0.0f);
}

//=============================================================================
// Idle Sway Tests
//=============================================================================

class IdleSwayTest : public ::testing::Test {
protected:
    IdleSway sway_;
};

TEST_F(IdleSwayTest, DefaultDisabled) {
    EXPECT_FLOAT_EQ(sway_.scale, 0.0f);
    EXPECT_FALSE(sway_.isEnabled());
}

TEST_F(IdleSwayTest, CalculateWhenDisabled) {
    auto offsets = sway_.calculate(1.0);
    EXPECT_FLOAT_EQ(offsets[0], 0.0f);
    EXPECT_FLOAT_EQ(offsets[1], 0.0f);
    EXPECT_FLOAT_EQ(offsets[2], 0.0f);
}

TEST_F(IdleSwayTest, CalculateWhenEnabled) {
    sway_.scale = 1.0f;
    EXPECT_TRUE(sway_.isEnabled());
    
    auto offsets = sway_.calculate(1.0);
    // Verify we get valid output (may or may not be non-zero depending on time)
    EXPECT_TRUE(std::isfinite(offsets[0]));
    EXPECT_TRUE(std::isfinite(offsets[1]));
    EXPECT_TRUE(std::isfinite(offsets[2]));
}

//=============================================================================
// Punch Angle Tests
//=============================================================================

class PunchAngleTest : public ::testing::Test {
protected:
    PunchAngle punch_;
};

TEST_F(PunchAngleTest, DefaultState) {
    EXPECT_FLOAT_EQ(punch_.current, 0.0f);
    EXPECT_FLOAT_EQ(punch_.ideal, 0.0f);
}

TEST_F(PunchAngleTest, Apply) {
    punch_.apply(-2.0f);  // Small kick
    EXPECT_FLOAT_EQ(punch_.ideal, -2.0f);
}

TEST_F(PunchAngleTest, SmallKickDecay) {
    punch_.current = 0.0f;
    punch_.ideal = -2.0f;
    
    punch_.update(0.05f);  // 50ms
    // With small kick rate of 20/sec, should have moved 1 unit
    // But ideal is -2, so it's decaying from 0 towards -2
}

TEST_F(PunchAngleTest, Reset) {
    punch_.apply(-5.0f);
    punch_.current = -3.0f;
    punch_.reset();
    
    EXPECT_FLOAT_EQ(punch_.current, 0.0f);
    EXPECT_FLOAT_EQ(punch_.ideal, 0.0f);
}

//=============================================================================
// Flash State Tests
//=============================================================================

TEST(FlashStateTest, DefaultState) {
    FlashState flash;
    EXPECT_FALSE(flash.isFlashed());
}

TEST(FlashStateTest, BlockTime) {
    FlashState flash;
    flash.lastOwnFlashTime = 1.0;
    flash.lastOtherFlashTime = 0.5;
    EXPECT_FLOAT_EQ(flash.blockTime(), tf_flash::DURATION_OWN);
    
    flash.lastOtherFlashTime = 2.0;
    EXPECT_FLOAT_EQ(flash.blockTime(), tf_flash::DURATION_OTHER);
}

TEST(FlashStateTest, Reset) {
    FlashState flash;
    flash.flashed = true;
    flash.lastOwnFlashTime = 1.0;
    flash.reset();
    
    EXPECT_FALSE(flash.isFlashed());
    EXPECT_DOUBLE_EQ(flash.lastOwnFlashTime, 0.0);
}

//=============================================================================
// View Model State Tests
//=============================================================================

TEST(ViewModelStateTest, DefaultState) {
    ViewModelState vm;
    EXPECT_TRUE(vm.drawEnabled);
    EXPECT_FALSE(vm.drawWhenInvisible);
    EXPECT_FLOAT_EQ(vm.size, 1.0f);
}

TEST(ViewModelStateTest, ShouldDraw) {
    ViewModelState vm;
    
    EXPECT_TRUE(vm.shouldDraw(false, false, false, true, 100));
    EXPECT_FALSE(vm.shouldDraw(true, false, false, true, 100));  // Dead
    EXPECT_FALSE(vm.shouldDraw(false, true, false, true, 100));  // Gib
    EXPECT_FALSE(vm.shouldDraw(false, false, true, true, 100));  // Invisible
    EXPECT_FALSE(vm.shouldDraw(false, false, false, false, 100)); // Can't draw
    EXPECT_FALSE(vm.shouldDraw(false, false, false, true, 0));   // No health
}

TEST(ViewModelStateTest, InvisibleWithFlag) {
    ViewModelState vm;
    vm.drawWhenInvisible = true;
    
    EXPECT_TRUE(vm.shouldDraw(false, false, true, true, 100));  // Invisible but flag set
}

//=============================================================================
// View Effects State Tests
//=============================================================================

class ViewEffectsStateTest : public ::testing::Test {
protected:
    ViewState state_;
};

TEST_F(ViewEffectsStateTest, DefaultState) {
    for (size_t i = 0; i < NUM_COLOR_SHIFTS; ++i) {
        EXPECT_FALSE(state_.colorShifts[i].isActive());
    }
    EXPECT_FALSE(state_.blend.isActive());
    EXPECT_FALSE(state_.concussioned);
}

TEST_F(ViewEffectsStateTest, ShiftAccess) {
    state_.shift(ColorShiftType::Damage).set(255, 0, 0, 100);
    
    EXPECT_TRUE(state_.shift(ColorShiftType::Damage).isActive());
    EXPECT_FALSE(state_.shift(ColorShiftType::Contents).isActive());
}

TEST_F(ViewEffectsStateTest, UpdateEffects) {
    state_.shift(ColorShiftType::Damage).set(255, 0, 0, 100);
    state_.shift(ColorShiftType::Bonus).set(0, 255, 0, 50);
    
    state_.updateEffects(0.1f);  // 100ms
    
    EXPECT_LT(state_.shift(ColorShiftType::Damage).percent, 100);
    EXPECT_LT(state_.shift(ColorShiftType::Bonus).percent, 50);
}

TEST_F(ViewEffectsStateTest, Reset) {
    state_.shift(ColorShiftType::Damage).set(255, 0, 0, 100);
    state_.concussioned = true;
    
    state_.reset();
    
    EXPECT_FALSE(state_.shift(ColorShiftType::Damage).isActive());
    EXPECT_FALSE(state_.concussioned);
}

//=============================================================================
// Camera Mode Tests
//=============================================================================

TEST(SpectatorCameraModeTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(SpectatorCameraMode::None), 0);
    EXPECT_EQ(static_cast<uint8_t>(SpectatorCameraMode::Track), 1);
}

TEST(SpectatorCameraModeTest, Names) {
    EXPECT_EQ(spectatorCameraModeName(SpectatorCameraMode::None), "none");
    EXPECT_EQ(spectatorCameraModeName(SpectatorCameraMode::Track), "track");
}

TEST(ExternalCameraModeTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(ExternalCameraMode::Normal), 0);
    EXPECT_EQ(static_cast<uint8_t>(ExternalCameraMode::ChaseCam), 1);
    EXPECT_EQ(static_cast<uint8_t>(ExternalCameraMode::External), 2);
}

TEST(ExternalCameraModeTest, Names) {
    EXPECT_EQ(externalCameraModeName(ExternalCameraMode::Normal), "normal");
    EXPECT_EQ(externalCameraModeName(ExternalCameraMode::ChaseCam), "chasecam");
    EXPECT_EQ(externalCameraModeName(ExternalCameraMode::External), "external");
}

//=============================================================================
// Camera Constants Tests
//=============================================================================

TEST(CameraConstantsTest, Values) {
    EXPECT_FLOAT_EQ(camera::DEFAULT_CHASE_DIST, 100.0f);
    EXPECT_FLOAT_EQ(camera::DEFAULT_CHASE_HEIGHT, 32.0f);
    EXPECT_FLOAT_EQ(camera::FLYBY_MIN_DIST, 32.0f);
    EXPECT_FLOAT_EQ(camera::FLYBY_MAX_DIST, 800.0f);
    EXPECT_FLOAT_EQ(camera::LOCK_TIMEOUT, 0.3f);
}

//=============================================================================
// Spectator Tracking State Tests
//=============================================================================

class SpectatorTrackingStateTest : public ::testing::Test {
protected:
    SpectatorTrackingState state_;
};

TEST_F(SpectatorTrackingStateTest, DefaultState) {
    EXPECT_EQ(state_.autocam, SpectatorCameraMode::None);
    EXPECT_EQ(state_.trackNum, 0);
    EXPECT_FALSE(state_.locked);
    EXPECT_FALSE(state_.isTracking());
}

TEST_F(SpectatorTrackingStateTest, Lock) {
    state_.lock(5, 1.0);
    
    EXPECT_EQ(state_.trackNum, 5);
    EXPECT_FALSE(state_.locked);  // Not locked until position established
    EXPECT_DOUBLE_EQ(state_.lastLockTime, 1.0);
}

TEST_F(SpectatorTrackingStateTest, IsTracking) {
    state_.autocam = SpectatorCameraMode::Track;
    EXPECT_TRUE(state_.isTracking());
    
    state_.autocam = SpectatorCameraMode::None;
    EXPECT_FALSE(state_.isTracking());
}

TEST_F(SpectatorTrackingStateTest, IsLocked) {
    state_.autocam = SpectatorCameraMode::Track;
    state_.locked = true;
    EXPECT_TRUE(state_.isLocked());
    
    state_.locked = false;
    EXPECT_FALSE(state_.isLocked());
}

TEST_F(SpectatorTrackingStateTest, Unlock) {
    state_.autocam = SpectatorCameraMode::Track;
    state_.locked = true;
    
    state_.unlock();
    
    EXPECT_EQ(state_.autocam, SpectatorCameraMode::None);
    EXPECT_FALSE(state_.locked);
}

TEST_F(SpectatorTrackingStateTest, SetKiller) {
    state_.setKiller(3, 5, 5);  // Killer 3, victim 5, we are player 5
    EXPECT_EQ(state_.killerNum, 3);
    
    state_.killerNum = -1;
    state_.setKiller(3, 5, 2);  // We are player 2, not victim
    EXPECT_EQ(state_.killerNum, -1);  // Not set
}

TEST_F(SpectatorTrackingStateTest, Reset) {
    state_.autocam = SpectatorCameraMode::Track;
    state_.trackNum = 5;
    state_.locked = true;
    state_.killerNum = 3;
    
    state_.reset();
    
    EXPECT_EQ(state_.autocam, SpectatorCameraMode::None);
    EXPECT_EQ(state_.trackNum, 0);
    EXPECT_FALSE(state_.locked);
    EXPECT_EQ(state_.killerNum, -1);
}

//=============================================================================
// Chase Camera State Tests
//=============================================================================

TEST(ChaseCameraStateTest, DefaultState) {
    ChaseCameraState chase;
    EXPECT_TRUE(chase.enabled);
    EXPECT_FALSE(chase.thirdPerson);
    EXPECT_FALSE(chase.tppEnabled);
}

TEST(ChaseCameraStateTest, IsFirstPerson) {
    ChaseCameraState chase;
    EXPECT_TRUE(chase.isFirstPerson());
    
    chase.thirdPerson = true;
    EXPECT_FALSE(chase.isFirstPerson());
    
    chase.thirdPerson = false;
    chase.enabled = false;
    EXPECT_FALSE(chase.isFirstPerson());
}

TEST(ChaseCameraStateTest, ShouldShowPlayer) {
    ChaseCameraState chase;
    EXPECT_FALSE(chase.shouldShowPlayer());  // First person = don't show
    
    chase.thirdPerson = true;
    EXPECT_TRUE(chase.shouldShowPlayer());
}

//=============================================================================
// External Camera State Tests
//=============================================================================

class ExternalCameraStateTest : public ::testing::Test {
protected:
    ExternalCameraState state_;
};

TEST_F(ExternalCameraStateTest, DefaultState) {
    EXPECT_EQ(state_.mode, ExternalCameraMode::Normal);
    EXPECT_FALSE(state_.isExternal());
}

TEST_F(ExternalCameraStateTest, SetChaseCam) {
    state_.setChaseCam();
    EXPECT_EQ(state_.mode, ExternalCameraMode::ChaseCam);
    EXPECT_TRUE(state_.isExternal());
    EXPECT_TRUE(state_.isChaseCam());
}

TEST_F(ExternalCameraStateTest, SetExternal) {
    state_.setExternal({100.0f, 200.0f, 300.0f});
    EXPECT_EQ(state_.mode, ExternalCameraMode::External);
    EXPECT_TRUE(state_.isExternal());
    EXPECT_FALSE(state_.isChaseCam());
    EXPECT_FLOAT_EQ(state_.position[0], 100.0f);
}

TEST_F(ExternalCameraStateTest, Reset) {
    state_.setExternal({100.0f, 200.0f, 300.0f});
    state_.reset();
    
    EXPECT_EQ(state_.mode, ExternalCameraMode::Normal);
    EXPECT_FLOAT_EQ(state_.position[0], 0.0f);
}

//=============================================================================
// Flyby Position Tests
//=============================================================================

TEST(FlybyPositionTest, DefaultInvalid) {
    FlybyPosition pos;
    EXPECT_FALSE(pos.isValid());
}

TEST(FlybyPositionTest, ValidPosition) {
    FlybyPosition pos;
    pos.valid = true;
    pos.distance = 100.0f;
    
    EXPECT_TRUE(pos.isValid());
}

TEST(FlybyPositionTest, InvalidDistance) {
    FlybyPosition pos;
    pos.valid = true;
    pos.distance = 10.0f;  // Below minimum
    
    EXPECT_FALSE(pos.isValid());
}

TEST(FlybyPositionTest, IsBetterThan) {
    FlybyPosition pos1, pos2;
    pos1.valid = true;
    pos1.distance = 100.0f;
    
    pos2.valid = true;
    pos2.distance = 200.0f;
    
    EXPECT_TRUE(pos1.isBetterThan(pos2));
    EXPECT_FALSE(pos2.isBetterThan(pos1));
}

//=============================================================================
// Multiview State Tests
//=============================================================================

class MultiviewStateTest : public ::testing::Test {
protected:
    MultiviewState state_;
};

TEST_F(MultiviewStateTest, DefaultState) {
    EXPECT_FALSE(state_.enabled);
    EXPECT_EQ(state_.activeSlot, 0);
    EXPECT_EQ(state_.activeCount(), 0u);
}

TEST_F(MultiviewStateTest, SetSlotTrack) {
    state_.setSlotTrack(0, 5);
    state_.setSlotTrack(1, 7);
    
    EXPECT_TRUE(state_.slots[0].isTracking());
    EXPECT_TRUE(state_.slots[1].isTracking());
    EXPECT_EQ(state_.activeCount(), 2u);
}

TEST_F(MultiviewStateTest, ClearSlot) {
    state_.setSlotTrack(0, 5);
    state_.slots[0].clear();
    
    EXPECT_FALSE(state_.slots[0].isTracking());
}

TEST_F(MultiviewStateTest, Reset) {
    state_.setSlotTrack(0, 5);
    state_.setSlotTrack(1, 7);
    state_.enabled = true;
    
    state_.reset();
    
    EXPECT_FALSE(state_.enabled);
    EXPECT_EQ(state_.activeCount(), 0u);
}

//=============================================================================
// High Tracker State Tests
//=============================================================================

TEST(HighTrackerStateTest, DefaultState) {
    HighTrackerState tracker;
    EXPECT_FALSE(tracker.enabled);
    EXPECT_EQ(tracker.lastHighPlayer, -1);
    EXPECT_EQ(tracker.lastHighScore, -9999);
}

TEST(HighTrackerStateTest, IsHigherScore) {
    HighTrackerState tracker;
    EXPECT_TRUE(tracker.isHigherScore(0, 0));  // 0 > -9999
    
    tracker.update(0, 10);
    EXPECT_FALSE(tracker.isHigherScore(1, 5));  // 5 < 10
    EXPECT_TRUE(tracker.isHigherScore(2, 15));  // 15 > 10
}

TEST(HighTrackerStateTest, Update) {
    HighTrackerState tracker;
    tracker.update(5, 100);
    
    EXPECT_EQ(tracker.lastHighPlayer, 5);
    EXPECT_EQ(tracker.lastHighScore, 100);
}

//=============================================================================
// Camera Input State Tests
//=============================================================================

class CameraInputStateTest : public ::testing::Test {
protected:
    CameraInputState input_;
};

TEST_F(CameraInputStateTest, DefaultState) {
    EXPECT_FALSE(input_.attackDown);
    EXPECT_FALSE(input_.jumpDown);
    EXPECT_FALSE(input_.moveDownDown);
}

TEST_F(CameraInputStateTest, AttackPressed) {
    input_.update(true, false, false);
    EXPECT_TRUE(input_.attackPressed());
    
    input_.update(true, false, false);
    EXPECT_FALSE(input_.attackPressed());  // Still down, not new press
}

TEST_F(CameraInputStateTest, AttackReleased) {
    input_.update(true, false, false);
    input_.update(false, false, false);
    EXPECT_TRUE(input_.attackReleased());
}

TEST_F(CameraInputStateTest, JumpPressed) {
    input_.update(false, true, false);
    EXPECT_TRUE(input_.jumpPressed());
    
    input_.update(false, true, false);
    EXPECT_FALSE(input_.jumpPressed());  // Pogo-stick prevention
}

TEST_F(CameraInputStateTest, Reset) {
    input_.update(true, true, true);
    input_.reset();
    
    EXPECT_FALSE(input_.attackDown);
    EXPECT_FALSE(input_.prevAttack);
}

//=============================================================================
// Camera State Tests
//=============================================================================

class CameraStateTest : public ::testing::Test {
protected:
    CameraState state_;
};

TEST_F(CameraStateTest, DefaultState) {
    EXPECT_FALSE(state_.isSpectator());
    EXPECT_EQ(state_.mainTrackNum(), 0);
}

TEST_F(CameraStateTest, IsSpectator) {
    EXPECT_FALSE(state_.isSpectator());
    
    state_.spectator.autocam = SpectatorCameraMode::Track;
    EXPECT_TRUE(state_.isSpectator());
}

TEST_F(CameraStateTest, ShouldDrawPlayer) {
    // Not spectator - always draw
    EXPECT_TRUE(state_.shouldDrawPlayer(5, false));
    
    // Spectator tracking player 5 in first person
    state_.spectator.autocam = SpectatorCameraMode::Track;
    state_.spectator.trackNum = 5;
    state_.spectator.locked = true;
    state_.chase.enabled = true;
    state_.chase.thirdPerson = false;
    
    EXPECT_FALSE(state_.shouldDrawPlayer(5, true));  // Don't draw tracked player
    EXPECT_TRUE(state_.shouldDrawPlayer(3, true));   // Draw other players
}

TEST_F(CameraStateTest, Reset) {
    state_.spectator.autocam = SpectatorCameraMode::Track;
    state_.multiview.enabled = true;
    
    state_.reset();
    
    EXPECT_EQ(state_.spectator.autocam, SpectatorCameraMode::None);
    EXPECT_FALSE(state_.multiview.enabled);
}

//=============================================================================
// Vector To Angles Tests
//=============================================================================

TEST(VectorToAnglesTest, PointingUp) {
    auto angles = vectorToAngles(0.0f, 0.0f, 1.0f);
    EXPECT_FLOAT_EQ(angles[0], 90.0f);  // Pitch
    EXPECT_FLOAT_EQ(angles[1], 0.0f);   // Yaw
}

TEST(VectorToAnglesTest, PointingDown) {
    auto angles = vectorToAngles(0.0f, 0.0f, -1.0f);
    EXPECT_FLOAT_EQ(angles[0], 270.0f);  // Pitch
    EXPECT_FLOAT_EQ(angles[1], 0.0f);    // Yaw
}

TEST(VectorToAnglesTest, PointingForward) {
    auto angles = vectorToAngles(1.0f, 0.0f, 0.0f);
    EXPECT_FLOAT_EQ(angles[0], 0.0f);   // Pitch
    EXPECT_FLOAT_EQ(angles[1], 0.0f);   // Yaw
}

TEST(VectorToAnglesTest, PointingRight) {
    auto angles = vectorToAngles(0.0f, 1.0f, 0.0f);
    EXPECT_FLOAT_EQ(angles[0], 0.0f);    // Pitch
    EXPECT_FLOAT_EQ(angles[1], 90.0f);   // Yaw
}
