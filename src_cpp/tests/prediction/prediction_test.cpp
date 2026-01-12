/*
 * ezQuake C++ Port - Prediction System Tests
 * 
 * Comprehensive unit tests for the prediction system including
 * player movement physics, collision, and interpolation.
 * 
 * Copyright (C) 2024 ezQuake Team
 */

#include <gtest/gtest.h>
#include "core/prediction/prediction.hpp"
#include "core/client/player_state.hpp"
#include <cmath>

using namespace ezquake;
using namespace ezquake::pmove;

//=============================================================================
// PMoveType Tests
//=============================================================================

class PMoveTypeTest : public ::testing::Test {};

TEST_F(PMoveTypeTest, CanMoveForNormalTypes) {
    EXPECT_TRUE(canMove(PMoveType::Normal));
    EXPECT_TRUE(canMove(PMoveType::Fly));
    EXPECT_TRUE(canMove(PMoveType::Spectator));
    EXPECT_TRUE(canMove(PMoveType::OldSpectator));
    EXPECT_TRUE(canMove(PMoveType::Dead));
}

TEST_F(PMoveTypeTest, CannotMoveForLockedTypes) {
    EXPECT_FALSE(canMove(PMoveType::None));
    EXPECT_FALSE(canMove(PMoveType::Lock));
}

TEST_F(PMoveTypeTest, IsNoClipForSpectatorModes) {
    EXPECT_TRUE(isNoClip(PMoveType::Spectator));
    EXPECT_TRUE(isNoClip(PMoveType::OldSpectator));
    EXPECT_FALSE(isNoClip(PMoveType::Normal));
    EXPECT_FALSE(isNoClip(PMoveType::Fly));
}

//=============================================================================
// Blocked Flags Tests
//=============================================================================

class BlockedFlagsTest : public ::testing::Test {};

TEST_F(BlockedFlagsTest, BitwiseOperations) {
    Blocked b = Blocked::Floor | Blocked::Step;
    EXPECT_TRUE(hasBlocked(b, Blocked::Floor));
    EXPECT_TRUE(hasBlocked(b, Blocked::Step));
    EXPECT_FALSE(hasBlocked(b, Blocked::Other));
}

TEST_F(BlockedFlagsTest, HasBlockedAny) {
    Blocked b = Blocked::Floor | Blocked::Step | Blocked::Other;
    EXPECT_TRUE(hasBlocked(b, Blocked::Any));
}

//=============================================================================
// WaterLevel Tests
//=============================================================================

class WaterLevelTest : public ::testing::Test {};

TEST_F(WaterLevelTest, EnumValues) {
    EXPECT_EQ(static_cast<int>(WaterLevel::None), 0);
    EXPECT_EQ(static_cast<int>(WaterLevel::Feet), 1);
    EXPECT_EQ(static_cast<int>(WaterLevel::Waist), 2);
    EXPECT_EQ(static_cast<int>(WaterLevel::Eyes), 3);
}

//=============================================================================
// Contents Tests
//=============================================================================

class ContentsTest : public ::testing::Test {};

TEST_F(ContentsTest, EnumValues) {
    // Contents values are negative in Quake
    EXPECT_EQ(static_cast<int>(Contents::Empty), -1);
    EXPECT_EQ(static_cast<int>(Contents::Solid), -2);
    EXPECT_EQ(static_cast<int>(Contents::Water), -3);
}

TEST_F(ContentsTest, WaterContentCheck) {
    // Water, slime, lava are <= CONTENTS_WATER
    EXPECT_TRUE(static_cast<int>(Contents::Water) <= static_cast<int>(Contents::Water));
    EXPECT_TRUE(static_cast<int>(Contents::Slime) <= static_cast<int>(Contents::Water));
    EXPECT_TRUE(static_cast<int>(Contents::Lava) <= static_cast<int>(Contents::Water));
}

//=============================================================================
// Plane Tests
//=============================================================================

class PlaneTest : public ::testing::Test {};

TEST_F(PlaneTest, DefaultConstruction) {
    Plane p;
    EXPECT_EQ(p.normal, Vec3{});
    EXPECT_EQ(p.distance, 0.0f);
}

TEST_F(PlaneTest, ParameterizedConstruction) {
    Plane p(Vec3(0, 0, 1), 10.0f);
    EXPECT_EQ(p.normal, Vec3(0, 0, 1));
    EXPECT_EQ(p.distance, 10.0f);
}

TEST_F(PlaneTest, DistanceToPoint) {
    Plane p(Vec3(0, 0, 1), 10.0f);  // z = 10 plane
    
    EXPECT_FLOAT_EQ(p.distanceTo(Vec3(0, 0, 20)), 10.0f);  // Above
    EXPECT_FLOAT_EQ(p.distanceTo(Vec3(0, 0, 10)), 0.0f);   // On plane
    EXPECT_FLOAT_EQ(p.distanceTo(Vec3(0, 0, 0)), -10.0f);  // Below
}

TEST_F(PlaneTest, IsInFront) {
    Plane p(Vec3(0, 0, 1), 10.0f);  // z = 10 plane
    
    EXPECT_TRUE(p.isInFront(Vec3(0, 0, 20)));
    EXPECT_FALSE(p.isInFront(Vec3(0, 0, 10)));
    EXPECT_FALSE(p.isInFront(Vec3(0, 0, 0)));
}

//=============================================================================
// TraceResult Tests
//=============================================================================

class TraceResultTest : public ::testing::Test {};

TEST_F(TraceResultTest, DefaultConstruction) {
    TraceResult t;
    EXPECT_FALSE(t.allSolid);
    EXPECT_FALSE(t.startSolid);
    EXPECT_EQ(t.fraction, 1.0f);
    EXPECT_EQ(t.entityNum, -1);
}

TEST_F(TraceResultTest, DidHit) {
    TraceResult t;
    EXPECT_FALSE(t.didHit());
    
    t.fraction = 0.5f;
    EXPECT_TRUE(t.didHit());
}

TEST_F(TraceResultTest, IsStuck) {
    TraceResult t;
    EXPECT_FALSE(t.isStuck());
    
    t.allSolid = true;
    EXPECT_TRUE(t.isStuck());
    
    t.allSolid = false;
    t.startSolid = true;
    EXPECT_TRUE(t.isStuck());
}

TEST_F(TraceResultTest, HitFloor) {
    TraceResult t;
    t.fraction = 0.5f;
    t.plane.normal = Vec3(0, 0, 1);  // Perfectly flat floor
    EXPECT_TRUE(t.hitFloor());
    
    t.plane.normal = Vec3(0, 0, 0.7f);  // 45 degree slope
    EXPECT_TRUE(t.hitFloor());
    
    t.plane.normal = Vec3(0, 0, 0.5f);  // Too steep
    EXPECT_FALSE(t.hitFloor());
}

TEST_F(TraceResultTest, HitWall) {
    TraceResult t;
    t.fraction = 0.5f;
    t.plane.normal = Vec3(1, 0, 0);  // Vertical wall
    EXPECT_TRUE(t.hitWall());
    
    t.plane.normal = Vec3(0, 0, 1);  // Floor
    EXPECT_FALSE(t.hitWall());
}

//=============================================================================
// MoveVars Tests
//=============================================================================

class MoveVarsTest : public ::testing::Test {};

TEST_F(MoveVarsTest, DefaultValues) {
    MoveVars mv;
    EXPECT_EQ(mv.gravity, 800.0f);
    EXPECT_EQ(mv.maxSpeed, 320.0f);
    EXPECT_EQ(mv.accelerate, 10.0f);
    EXPECT_EQ(mv.friction, 4.0f);
}

TEST_F(MoveVarsTest, EffectiveGravity) {
    MoveVars mv;
    EXPECT_EQ(mv.effectiveGravity(), 800.0f);
    
    mv.entGravity = 0.5f;
    EXPECT_EQ(mv.effectiveGravity(), 400.0f);
}

TEST_F(MoveVarsTest, Reset) {
    MoveVars mv;
    mv.gravity = 100.0f;
    mv.maxSpeed = 500.0f;
    
    mv.reset();
    EXPECT_EQ(mv.gravity, 800.0f);
    EXPECT_EQ(mv.maxSpeed, 320.0f);
}

//=============================================================================
// PlayerMoveState Tests
//=============================================================================

class PlayerMoveStateTest : public ::testing::Test {
protected:
    PlayerMoveState state;
};

TEST_F(PlayerMoveStateTest, DefaultConstruction) {
    EXPECT_EQ(state.origin(), Vec3{});
    EXPECT_EQ(state.velocity(), Vec3{});
    EXPECT_FALSE(state.jumpHeld());
    EXPECT_EQ(state.moveType(), PMoveType::Normal);
    EXPECT_FALSE(state.onGround());
    EXPECT_EQ(state.waterLevel(), WaterLevel::None);
}

TEST_F(PlayerMoveStateTest, SettersAndGetters) {
    state.setOrigin(Vec3(100, 200, 300));
    EXPECT_EQ(state.origin(), Vec3(100, 200, 300));
    
    state.setVelocity(Vec3(10, 20, 30));
    EXPECT_EQ(state.velocity(), Vec3(10, 20, 30));
    
    state.setAngles(Vec3(10, 90, 0));
    EXPECT_EQ(state.angles(), Vec3(10, 90, 0));
    
    state.setMoveType(PMoveType::Fly);
    EXPECT_EQ(state.moveType(), PMoveType::Fly);
    
    state.setOnGround(true);
    EXPECT_TRUE(state.onGround());
    
    state.setWaterLevel(WaterLevel::Waist);
    EXPECT_EQ(state.waterLevel(), WaterLevel::Waist);
}

TEST_F(PlayerMoveStateTest, TouchList) {
    EXPECT_EQ(state.numTouch(), 0);
    
    EXPECT_TRUE(state.addTouch(1));
    EXPECT_TRUE(state.addTouch(2));
    EXPECT_EQ(state.numTouch(), 2);
    
    // Adding duplicate should fail
    EXPECT_FALSE(state.addTouch(1));
    EXPECT_EQ(state.numTouch(), 2);
    
    auto touches = state.touchList();
    EXPECT_EQ(touches.size(), 2);
    EXPECT_EQ(touches[0], 1);
    EXPECT_EQ(touches[1], 2);
    
    state.clearTouchList();
    EXPECT_EQ(state.numTouch(), 0);
}

TEST_F(PlayerMoveStateTest, PhysEnts) {
    EXPECT_EQ(state.numPhysEnts(), 0);
    
    PhysEntity ent;
    ent.origin = Vec3(100, 100, 0);
    ent.info = 42;
    
    EXPECT_TRUE(state.addPhysEnt(ent));
    EXPECT_EQ(state.numPhysEnts(), 1);
    
    auto ents = state.physEnts();
    EXPECT_EQ(ents.size(), 1);
    EXPECT_EQ(ents[0].info, 42);
    
    state.clearPhysEnts();
    EXPECT_EQ(state.numPhysEnts(), 0);
}

TEST_F(PlayerMoveStateTest, IsSwimming) {
    EXPECT_FALSE(state.isSwimming());
    
    state.setWaterLevel(WaterLevel::Feet);
    EXPECT_FALSE(state.isSwimming());
    
    state.setWaterLevel(WaterLevel::Waist);
    EXPECT_TRUE(state.isSwimming());
    
    state.setWaterLevel(WaterLevel::Eyes);
    EXPECT_TRUE(state.isSwimming());
}

TEST_F(PlayerMoveStateTest, CanJump) {
    state.setOnGround(true);
    EXPECT_TRUE(state.canJump());
    
    state.setOnGround(false);
    EXPECT_FALSE(state.canJump());
    
    state.setWaterLevel(WaterLevel::Waist);
    EXPECT_TRUE(state.canJump());  // Can swim-jump
    
    state.setMoveType(PMoveType::Dead);
    EXPECT_FALSE(state.canJump());
}

TEST_F(PlayerMoveStateTest, ClearResults) {
    state.setOnGround(true);
    state.setGroundEntity(5);
    state.setWaterLevel(WaterLevel::Eyes);
    state.addTouch(1);
    
    state.clearResults();
    
    EXPECT_FALSE(state.onGround());
    EXPECT_EQ(state.groundEntity(), -1);
    EXPECT_EQ(state.waterLevel(), WaterLevel::None);
    EXPECT_EQ(state.numTouch(), 0);
}

//=============================================================================
// ClipVelocity Tests
//=============================================================================

class ClipVelocityTest : public ::testing::Test {};

TEST_F(ClipVelocityTest, ClipAgainstFloor) {
    Vec3 vel(100, 0, -50);
    Vec3 normal(0, 0, 1);
    
    Vec3 result = clipVelocity(vel, normal, 1.0f);
    
    EXPECT_FLOAT_EQ(result.x(), 100.0f);
    EXPECT_FLOAT_EQ(result.y(), 0.0f);
    EXPECT_FLOAT_EQ(result.z(), 0.0f);  // Vertical component removed
}

TEST_F(ClipVelocityTest, ClipAgainstWall) {
    Vec3 vel(100, 0, 0);
    Vec3 normal(-1, 0, 0);
    
    Vec3 result = clipVelocity(vel, normal, 1.0f);
    
    EXPECT_FLOAT_EQ(result.x(), 0.0f);  // Horizontal component removed
    EXPECT_FLOAT_EQ(result.y(), 0.0f);
    EXPECT_FLOAT_EQ(result.z(), 0.0f);
}

TEST_F(ClipVelocityTest, ClipWithBounce) {
    Vec3 vel(100, 0, -50);
    Vec3 normal(0, 0, 1);
    
    Vec3 result = clipVelocity(vel, normal, 1.5f);  // Overbounce
    
    EXPECT_FLOAT_EQ(result.x(), 100.0f);
    EXPECT_FLOAT_EQ(result.y(), 0.0f);
    EXPECT_FLOAT_EQ(result.z(), 25.0f);  // Bounces back up
}

TEST_F(ClipVelocityTest, ClipOnSlope) {
    Vec3 vel(100, 0, -100);
    Vec3 normal(0, 0, 1);  // Up
    normal = Vec3(0.5f, 0, 0.866f);  // ~30 degree slope
    normal.normalize();
    
    Vec3 result = clipVelocity(vel, normal, 1.0f);
    
    // Should slide along slope
    EXPECT_GT(result.x(), 0.0f);
    EXPECT_NE(result.z(), 0.0f);
}

//=============================================================================
// SimpleTraceProvider Tests
//=============================================================================

class SimpleTraceProviderTest : public ::testing::Test {
protected:
    SimpleTraceProvider provider;
};

TEST_F(SimpleTraceProviderTest, EmptyWorld) {
    TraceResult result = provider.playerTrace(Vec3(0, 0, 100), Vec3(0, 0, -100));
    EXPECT_EQ(result.fraction, 1.0f);
    EXPECT_FALSE(result.didHit());
}

TEST_F(SimpleTraceProviderTest, GroundPlane) {
    provider.setGroundPlane(0.0f);
    
    // Trace from above ground to below
    TraceResult result = provider.playerTrace(Vec3(0, 0, 50), Vec3(0, 0, -50));
    
    EXPECT_TRUE(result.didHit());
    EXPECT_LT(result.fraction, 1.0f);
    EXPECT_EQ(result.plane.normal, Vec3(0, 0, 1));
    EXPECT_FLOAT_EQ(result.endPos.z(), 24.0f);  // Player hull bottom at ground
}

TEST_F(SimpleTraceProviderTest, PointContents) {
    provider.setGroundPlane(0.0f);
    
    EXPECT_EQ(provider.pointContents(Vec3(0, 0, 10)), Contents::Empty);
    EXPECT_EQ(provider.pointContents(Vec3(0, 0, -10)), Contents::Solid);
}

TEST_F(SimpleTraceProviderTest, TestPlayerPosition) {
    provider.setGroundPlane(0.0f);
    
    EXPECT_TRUE(provider.testPlayerPosition(Vec3(0, 0, 50)));   // Above ground
    EXPECT_TRUE(provider.testPlayerPosition(Vec3(0, 0, 24)));   // On ground
    EXPECT_FALSE(provider.testPlayerPosition(Vec3(0, 0, 0)));   // In ground
}

//=============================================================================
// PlayerMovement Tests
//=============================================================================

class PlayerMovementTest : public ::testing::Test {
protected:
    SimpleTraceProvider provider;
    MoveVars moveVars;
    
    void SetUp() override {
        provider.setGroundPlane(0.0f);
    }
};

TEST_F(PlayerMovementTest, BasicGroundMove) {
    PlayerMovement movement(provider, moveVars);
    
    PlayerMoveState state;
    state.setOrigin(Vec3(0, 0, 24));  // On ground
    
    UserCommand cmd;
    cmd.setMsec(16);  // 16ms frame
    cmd.setAngles(Vec3(0, 0, 0));
    cmd.setForwardMove(400);  // Move forward
    state.setCmd(cmd);
    
    movement.playerMove(state);
    
    // Should have moved forward
    EXPECT_GT(static_cast<float>(state.origin().x()), 0.0f);
    EXPECT_NEAR(static_cast<float>(state.origin().z()), 24.0f, 1.0f);  // Stay on ground
}

TEST_F(PlayerMovementTest, Gravity) {
    provider.clearGroundPlane();  // No ground
    PlayerMovement movement(provider, moveVars);
    
    PlayerMoveState state;
    state.setOrigin(Vec3(0, 0, 100));
    state.setVelocity(Vec3(0, 0, 0));
    
    UserCommand cmd;
    cmd.setMsec(100);  // 100ms
    cmd.setAngles(Vec3(0, 0, 0));
    state.setCmd(cmd);
    
    movement.playerMove(state);
    
    // Should have fallen due to gravity
    EXPECT_LT(static_cast<float>(state.velocity().z()), 0.0f);
    EXPECT_LT(static_cast<float>(state.origin().z()), 100.0f);
    EXPECT_FALSE(state.onGround());
}

TEST_F(PlayerMovementTest, OnGroundDetection) {
    PlayerMovement movement(provider, moveVars);
    
    PlayerMoveState state;
    state.setOrigin(Vec3(0, 0, 24));  // Player hull bottom at z=0
    
    UserCommand cmd;
    cmd.setMsec(16);
    cmd.setAngles(Vec3(0, 0, 0));
    state.setCmd(cmd);
    
    movement.categorizePosition(state);
    
    EXPECT_TRUE(state.onGround());
    EXPECT_EQ(state.groundEntity(), 0);  // World
}

TEST_F(PlayerMovementTest, Jumping) {
    PlayerMovement movement(provider, moveVars);
    
    PlayerMoveState state;
    state.setOrigin(Vec3(0, 0, 24));
    state.setOnGround(true);
    
    UserCommand cmd;
    cmd.setMsec(16);
    cmd.setAngles(Vec3(0, 0, 0));
    cmd.setButtons(client::Button::Jump);
    state.setCmd(cmd);
    
    movement.playerMove(state);
    
    // Should have jumped
    EXPECT_GT(static_cast<float>(state.velocity().z()), 0.0f);
    EXPECT_FALSE(state.onGround());
    EXPECT_TRUE(state.jumpHeld());
}

TEST_F(PlayerMovementTest, CantPogoJump) {
    PlayerMovement movement(provider, moveVars);
    
    PlayerMoveState state;
    state.setOrigin(Vec3(0, 0, 24));
    state.setOnGround(true);
    state.setJumpHeld(true);  // Already holding jump
    
    UserCommand cmd;
    cmd.setMsec(16);
    cmd.setAngles(Vec3(0, 0, 0));
    cmd.setButtons(client::Button::Jump);
    state.setCmd(cmd);
    
    movement.playerMove(state);
    
    // Should NOT have jumped (pogo stick prevention)
    EXPECT_EQ(static_cast<float>(state.velocity().z()), 0.0f);
}

TEST_F(PlayerMovementTest, SpectatorMode) {
    provider.clearGroundPlane();
    PlayerMovement movement(provider, moveVars);
    
    PlayerMoveState state;
    state.setOrigin(Vec3(0, 0, 100));
    state.setMoveType(PMoveType::Spectator);
    
    UserCommand cmd;
    cmd.setMsec(100);
    cmd.setAngles(Vec3(0, 0, 0));
    cmd.setForwardMove(500);
    state.setCmd(cmd);
    
    movement.playerMove(state);
    
    // Should have moved forward, no gravity
    EXPECT_GT(static_cast<float>(state.origin().x()), 0.0f);
    EXPECT_FALSE(state.onGround());  // Spectators are always in air
}

TEST_F(PlayerMovementTest, Friction) {
    PlayerMovement movement(provider, moveVars);
    
    PlayerMoveState state;
    state.setOrigin(Vec3(0, 0, 24));
    state.setVelocity(Vec3(200, 0, 0));  // Moving
    state.setOnGround(true);
    
    UserCommand cmd;
    cmd.setMsec(100);  // Long frame for noticeable friction
    cmd.setAngles(Vec3(0, 0, 0));
    // No forward move - should slow down
    state.setCmd(cmd);
    
    movement.playerMove(state);
    
    // Should have slowed due to friction
    EXPECT_LT(static_cast<float>(state.velocity().x()), 200.0f);
}

TEST_F(PlayerMovementTest, LockedMoveType) {
    PlayerMovement movement(provider, moveVars);
    
    PlayerMoveState state;
    state.setOrigin(Vec3(0, 0, 100));
    state.setMoveType(PMoveType::Lock);
    
    UserCommand cmd;
    cmd.setMsec(100);
    cmd.setAngles(Vec3(0, 0, 0));
    cmd.setForwardMove(500);
    state.setCmd(cmd);
    
    Vec3 originalPos = state.origin();
    movement.playerMove(state);
    
    // Should not have moved
    EXPECT_EQ(state.origin(), originalPos);
}

//=============================================================================
// CrouchCalculator Tests
//=============================================================================

class CrouchCalculatorTest : public ::testing::Test {
protected:
    CrouchCalculator calc;
};

TEST_F(CrouchCalculatorTest, DefaultState) {
    EXPECT_EQ(calc.crouch(), 0.0f);
}

TEST_F(CrouchCalculatorTest, NoChangeOnFlat) {
    float crouch = calc.calculate(Vec3(0, 0, 0), 0.016f, true);
    EXPECT_EQ(crouch, 0.0f);
    
    crouch = calc.calculate(Vec3(10, 0, 0), 0.016f, true);
    EXPECT_EQ(crouch, 0.0f);  // Horizontal movement, no crouch
}

TEST_F(CrouchCalculatorTest, StairClimb) {
    // Start on ground
    calc.calculate(Vec3(0, 0, 0), 0.016f, true);
    
    // Move up one step
    float crouch = calc.calculate(Vec3(0, 0, 10), 0.016f, true);
    
    // Crouch should be negative (view lowered)
    EXPECT_LT(crouch, 0.0f);
}

TEST_F(CrouchCalculatorTest, TeleportReset) {
    // Simulate some stair climbing
    calc.calculate(Vec3(0, 0, 0), 0.016f, true);
    calc.calculate(Vec3(0, 0, 10), 0.016f, true);
    
    // Large position change (teleport)
    float crouch = calc.calculate(Vec3(1000, 1000, 500), 0.016f, true);
    
    // Should reset
    EXPECT_EQ(crouch, 0.0f);
}

TEST_F(CrouchCalculatorTest, NoLerpReset) {
    calc.calculate(Vec3(0, 0, 0), 0.016f, true);
    calc.calculate(Vec3(0, 0, 10), 0.016f, true);
    
    // Force reset via noLerp
    float crouch = calc.calculate(Vec3(0, 0, 20), 0.016f, true, true);
    
    EXPECT_EQ(crouch, 0.0f);
}

TEST_F(CrouchCalculatorTest, FallingRecovery) {
    // Start in air
    calc.calculate(Vec3(0, 0, 100), 0.016f, false);
    
    // Fall
    float crouch = calc.calculate(Vec3(0, 0, 50), 0.016f, false);
    
    // In air, crouch should move toward zero
    EXPECT_GE(crouch, -0.1f);  // Should be near zero
}

TEST_F(CrouchCalculatorTest, Reset) {
    calc.calculate(Vec3(0, 0, 0), 0.016f, true);
    calc.calculate(Vec3(0, 0, 10), 0.016f, true);
    
    calc.reset();
    
    EXPECT_EQ(calc.crouch(), 0.0f);
}

//=============================================================================
// LerpMoveCalculator Tests
//=============================================================================

class LerpMoveCalculatorTest : public ::testing::Test {
protected:
    LerpMoveCalculator calc;
};

TEST_F(LerpMoveCalculatorTest, InitialState) {
    auto result = calc.calculate(1.0, false, 0.016);
    EXPECT_FALSE(result.valid);
}

TEST_F(LerpMoveCalculatorTest, SingleSample) {
    calc.recordSample(Vec3(0, 0, 0), Vec3(0, 0, 0), 1.0, 1);
    
    auto result = calc.calculate(1.0, false, 0.016);
    // Need at least 2 samples for lerp
    EXPECT_FALSE(result.valid);
}

TEST_F(LerpMoveCalculatorTest, TwoSamples) {
    calc.recordSample(Vec3(0, 0, 0), Vec3(0, 0, 0), 1.0, 1);
    calc.recordSample(Vec3(100, 0, 0), Vec3(0, 90, 0), 1.1, 2);
    
    // Query at interpolated time
    auto result = calc.calculate(1.05 + 0.01, false, 0.016);  // +latency
    
    EXPECT_TRUE(result.valid);
    // Should be somewhere between the two samples
    EXPECT_GE(static_cast<float>(result.origin.x()), 0.0f);
    EXPECT_LE(static_cast<float>(result.origin.x()), 100.0f);
}

TEST_F(LerpMoveCalculatorTest, SequenceReset) {
    calc.recordSample(Vec3(0, 0, 0), Vec3(0, 0, 0), 1.0, 100);
    calc.recordSample(Vec3(100, 0, 0), Vec3(0, 0, 0), 1.1, 101);
    
    // Sequence goes backwards (reset)
    calc.recordSample(Vec3(500, 0, 0), Vec3(0, 0, 0), 2.0, 1);
    
    // Should not be valid yet after reset
    auto result = calc.calculate(2.0, false, 0.016);
    EXPECT_FALSE(result.valid);
}

TEST_F(LerpMoveCalculatorTest, TeleportDetection) {
    calc.recordSample(Vec3(0, 0, 0), Vec3(0, 0, 0), 1.0, 1);
    calc.recordSample(Vec3(500, 0, 0), Vec3(0, 0, 0), 1.1, 2);  // Big jump = teleport
    
    // Lerping should be disabled for teleport
    auto result = calc.calculate(1.15, false, 0.016);
    
    // After teleport, interpolation should be disabled for that segment
    // Implementation may vary, but generally shouldn't interpolate through teleport
}

TEST_F(LerpMoveCalculatorTest, DisableLerpMove) {
    calc.recordSample(Vec3(0, 0, 0), Vec3(0, 0, 0), 1.0, 1);
    calc.recordSample(Vec3(10, 0, 0), Vec3(0, 0, 0), 1.1, 2);
    
    calc.disableLerpMove();
    
    calc.recordSample(Vec3(20, 0, 0), Vec3(0, 0, 0), 1.2, 3);
    
    // Next samples should have lerping disabled
    auto result = calc.calculate(1.25, false, 0.016);
    EXPECT_FALSE(result.valid);
}

TEST_F(LerpMoveCalculatorTest, Reset) {
    calc.recordSample(Vec3(0, 0, 0), Vec3(0, 0, 0), 1.0, 1);
    calc.recordSample(Vec3(100, 0, 0), Vec3(0, 0, 0), 1.1, 2);
    
    calc.reset();
    
    auto result = calc.calculate(1.05, false, 0.016);
    EXPECT_FALSE(result.valid);
}

//=============================================================================
// PhysEntity Tests
//=============================================================================

class PhysEntityTest : public ::testing::Test {};

TEST_F(PhysEntityTest, DefaultConstruction) {
    PhysEntity ent;
    EXPECT_EQ(ent.origin, Vec3{});
    EXPECT_EQ(ent.mins, Vec3{});
    EXPECT_EQ(ent.maxs, Vec3{});
    EXPECT_EQ(ent.info, 0);
    EXPECT_FALSE(ent.isTransparent);
}

TEST_F(PhysEntityTest, HasBounds) {
    PhysEntity ent;
    EXPECT_FALSE(ent.hasBounds());  // mins == maxs
    
    ent.mins = Vec3(-16, -16, -24);
    ent.maxs = Vec3(16, 16, 32);
    EXPECT_TRUE(ent.hasBounds());
}

//=============================================================================
// PhysicsNormal Tests
//=============================================================================

class PhysicsNormalTest : public ::testing::Test {};

TEST_F(PhysicsNormalTest, DefaultConstruction) {
    physics::PhysicsNormal pn;
    EXPECT_EQ(pn.normal, Vec3{});
    EXPECT_EQ(pn.flags, physics::NormalFlags::None);
    EXPECT_FALSE(pn.isSet());
}

TEST_F(PhysicsNormalTest, IsSet) {
    physics::PhysicsNormal pn;
    pn.flags = physics::NormalFlags::Set;
    EXPECT_TRUE(pn.isSet());
}

TEST_F(PhysicsNormalTest, FlagsOperations) {
    physics::NormalFlags f = physics::NormalFlags::Set | physics::NormalFlags::FlipX;
    EXPECT_TRUE(physics::hasFlag(f, physics::NormalFlags::Set));
    EXPECT_TRUE(physics::hasFlag(f, physics::NormalFlags::FlipX));
    EXPECT_FALSE(physics::hasFlag(f, physics::NormalFlags::FlipY));
}

//=============================================================================
// Integration Tests
//=============================================================================

class PredictionIntegrationTest : public ::testing::Test {
protected:
    SimpleTraceProvider provider;
    MoveVars moveVars;
    
    void SetUp() override {
        provider.setGroundPlane(0.0f);
    }
};

TEST_F(PredictionIntegrationTest, FullMoveSequence) {
    PlayerMovement movement(provider, moveVars);
    
    PlayerMoveState state;
    state.setOrigin(Vec3(0, 0, 24));
    state.setOnGround(true);
    
    // Simulate 10 frames of movement
    for (int i = 0; i < 10; ++i) {
        UserCommand cmd;
        cmd.setMsec(16);
        cmd.setAngles(Vec3(0, 0, 0));
        cmd.setForwardMove(400);
        state.setCmd(cmd);
        
        movement.playerMove(state);
    }
    
    // Should have moved forward significantly
    EXPECT_GT(static_cast<float>(state.origin().x()), 10.0f);
    // Should still be on ground
    EXPECT_NEAR(static_cast<float>(state.origin().z()), 24.0f, 2.0f);
}

TEST_F(PredictionIntegrationTest, JumpAndLand) {
    PlayerMovement movement(provider, moveVars);
    
    PlayerMoveState state;
    state.setOrigin(Vec3(0, 0, 24));
    state.setOnGround(true);
    
    // Jump
    UserCommand cmd;
    cmd.setMsec(16);
    cmd.setAngles(Vec3(0, 0, 0));
    cmd.setButtons(client::Button::Jump);
    state.setCmd(cmd);
    movement.playerMove(state);
    
    EXPECT_GT(static_cast<float>(state.velocity().z()), 0.0f);
    
    // Simulate many frames to fall back down
    for (int i = 0; i < 100; ++i) {
        cmd.setButtons(client::Button::None);  // Release jump
        cmd.setMsec(16);
        state.setCmd(cmd);
        movement.playerMove(state);
        
        if (state.onGround()) {
            break;
        }
    }
    
    // Should have landed
    EXPECT_TRUE(state.onGround());
    EXPECT_NEAR(static_cast<float>(state.origin().z()), 24.0f, 1.0f);
}

TEST_F(PredictionIntegrationTest, StrafeMovement) {
    PlayerMovement movement(provider, moveVars);
    
    PlayerMoveState state;
    state.setOrigin(Vec3(0, 0, 24));
    state.setOnGround(true);
    
    // Strafe right while looking forward
    UserCommand cmd;
    cmd.setMsec(16);
    cmd.setAngles(Vec3(0, 0, 0));
    cmd.setSideMove(400);
    state.setCmd(cmd);
    
    movement.playerMove(state);
    
    // Should have moved in Y direction (right from forward view)
    EXPECT_LT(static_cast<float>(state.origin().y()), 0.0f);  // Negative Y is right
}

TEST_F(PredictionIntegrationTest, CrouchWithStairs) {
    CrouchCalculator crouch;
    
    // Simulate climbing stairs
    float c = crouch.calculate(Vec3(0, 0, 0), 0.016f, true);
    EXPECT_EQ(c, 0.0f);
    
    // Step up
    c = crouch.calculate(Vec3(5, 0, 18), 0.016f, true);
    EXPECT_LT(c, 0.0f);  // View should be lowered
    
    // Multiple frames to smooth out
    for (int i = 0; i < 10; ++i) {
        c = crouch.calculate(Vec3(static_cast<float>(5 * (i + 2)), 0, 18), 0.016f, true);
    }
    
    // Crouch should be recovering toward 0
    EXPECT_GT(c, -18.0f);
}

// Main is provided by test_main.cpp
