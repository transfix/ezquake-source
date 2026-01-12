/**
 * @file physics_test.cpp
 * @brief Tests for physics type definitions
 */

#include <gtest/gtest.h>
#include "core/physics/physics_types.hpp"

using namespace ezquake::physics;

// =============================================================================
// TraceType Tests
// =============================================================================

TEST(TraceTypeTest, Values) {
    EXPECT_EQ(static_cast<uint32_t>(TraceType::Normal), 0u);
    EXPECT_EQ(static_cast<uint32_t>(TraceType::NoMonsters), 1u);
    EXPECT_EQ(static_cast<uint32_t>(TraceType::Missile), 2u);
    EXPECT_EQ(static_cast<uint32_t>(TraceType::Lagged), 64u);
}

TEST(TraceTypeTest, Names) {
    EXPECT_EQ(traceTypeName(TraceType::Normal), "normal");
    EXPECT_EQ(traceTypeName(TraceType::NoMonsters), "nomonsters");
    EXPECT_EQ(traceTypeName(TraceType::Missile), "missile");
    EXPECT_EQ(traceTypeName(TraceType::Lagged), "lagged");
}

TEST(TraceTypeTest, CombineFlags) {
    uint32_t flags = TraceType::Missile | TraceType::Lagged;
    EXPECT_EQ(flags, 66u);  // 2 | 64
    EXPECT_TRUE(hasFlag(flags, TraceType::Missile));
    EXPECT_TRUE(hasFlag(flags, TraceType::Lagged));
    EXPECT_FALSE(hasFlag(flags, TraceType::NoMonsters));
}

// =============================================================================
// AreaType Tests
// =============================================================================

TEST(AreaTypeTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(AreaType::Solid), 0);
    EXPECT_EQ(static_cast<uint8_t>(AreaType::Triggers), 1);
}

TEST(AreaTypeTest, Names) {
    EXPECT_EQ(areaTypeName(AreaType::Solid), "solid");
    EXPECT_EQ(areaTypeName(AreaType::Triggers), "triggers");
}

// =============================================================================
// Physics Constants Tests
// =============================================================================

TEST(PhysicsConstantsTest, Values) {
    EXPECT_FLOAT_EQ(physics_constants::STOP_EPSILON, 0.1f);
    EXPECT_EQ(physics_constants::MAX_CLIP_PLANES, 5);
    EXPECT_FLOAT_EQ(physics_constants::STEPSIZE, 18.0f);
    EXPECT_EQ(physics_constants::AREA_DEPTH, 4);
    EXPECT_EQ(physics_constants::AREA_NODES, 32);
}

TEST(PhysicsConstantsTest, DefaultValues) {
    EXPECT_FLOAT_EQ(physics_constants::DEFAULT_GRAVITY, 800.0f);
    EXPECT_FLOAT_EQ(physics_constants::DEFAULT_MAXSPEED, 320.0f);
    EXPECT_FLOAT_EQ(physics_constants::DEFAULT_FRICTION, 4.0f);
}

TEST(PhysicsConstantsTest, BlockedFlags) {
    EXPECT_EQ(physics_constants::BLOCKED_FLOOR, 1);
    EXPECT_EQ(physics_constants::BLOCKED_STEP, 2);
    EXPECT_EQ(physics_constants::BLOCKED_DEAD_STOP, 4);
}

// =============================================================================
// PhysicsBlockedFlags Tests
// =============================================================================

TEST(PhysicsBlockedFlagsTest, HitFloor) {
    EXPECT_TRUE(BlockedFlags::hitFloor(BlockedFlags::Floor));
    EXPECT_TRUE(BlockedFlags::hitFloor(BlockedFlags::Floor | BlockedFlags::Step));
    EXPECT_FALSE(BlockedFlags::hitFloor(BlockedFlags::Step));
    EXPECT_FALSE(BlockedFlags::hitFloor(BlockedFlags::None));
}

TEST(PhysicsBlockedFlagsTest, HitStep) {
    EXPECT_TRUE(BlockedFlags::hitStep(BlockedFlags::Step));
    EXPECT_TRUE(BlockedFlags::hitStep(BlockedFlags::Floor | BlockedFlags::Step));
    EXPECT_FALSE(BlockedFlags::hitStep(BlockedFlags::Floor));
}

TEST(PhysicsBlockedFlagsTest, IsStuck) {
    EXPECT_TRUE(BlockedFlags::isStuck(3));  // floor + step = trapped
    EXPECT_TRUE(BlockedFlags::isStuck(7));  // all flags
    EXPECT_FALSE(BlockedFlags::isStuck(1));
    EXPECT_FALSE(BlockedFlags::isStuck(2));
}

// =============================================================================
// TracePlane Tests
// =============================================================================

TEST(TracePlaneTest, IsFloor) {
    TracePlane floor;
    floor.normal = Vec3(0.0f, 0.0f, 1.0f);
    EXPECT_TRUE(floor.isFloor());
    EXPECT_FALSE(floor.isWall());
    EXPECT_FALSE(floor.isCeiling());
    
    // Sloped floor (still counts as floor if > 0.7)
    floor.normal = Vec3(0.0f, 0.3f, 0.95f);
    EXPECT_TRUE(floor.isFloor());
}

TEST(TracePlaneTest, IsWall) {
    TracePlane wall;
    wall.normal = Vec3(1.0f, 0.0f, 0.0f);
    EXPECT_TRUE(wall.isWall());
    EXPECT_FALSE(wall.isFloor());
}

TEST(TracePlaneTest, IsCeiling) {
    TracePlane ceiling;
    ceiling.normal = Vec3(0.0f, 0.0f, -1.0f);
    EXPECT_TRUE(ceiling.isCeiling());
    EXPECT_FALSE(ceiling.isFloor());
}

TEST(TracePlaneTest, IsSlope) {
    TracePlane slope;
    slope.normal = Vec3(0.6f, 0.0f, 0.5f);
    EXPECT_TRUE(slope.isSlope());
    EXPECT_FALSE(slope.isFloor());
    EXPECT_FALSE(slope.isWall());
}

// =============================================================================
// PhysicsTraceResult Tests
// =============================================================================

TEST(PhysicsTraceResultTest, Default) {
    TraceResult trace;
    EXPECT_FALSE(trace.allSolid);
    EXPECT_FALSE(trace.startSolid);
    EXPECT_FLOAT_EQ(trace.fraction, 1.0f);
    EXPECT_EQ(trace.entityNum, -1);
    EXPECT_FALSE(trace.didHit());
}

TEST(PhysicsTraceResultTest, DidHit) {
    TraceResult trace;
    trace.fraction = 0.5f;
    EXPECT_TRUE(trace.didHit());
    
    trace.fraction = 1.0f;
    trace.allSolid = true;
    EXPECT_TRUE(trace.didHit());
}

TEST(PhysicsTraceResultTest, HitWorld) {
    TraceResult trace;
    trace.fraction = 0.5f;
    trace.entityNum = 0;  // world
    EXPECT_TRUE(trace.hitWorld());
    EXPECT_FALSE(trace.hitEntity());
}

TEST(PhysicsTraceResultTest, HitEntity) {
    TraceResult trace;
    trace.fraction = 0.5f;
    trace.entityNum = 5;
    EXPECT_TRUE(trace.hitEntity());
    EXPECT_FALSE(trace.hitWorld());
}

// =============================================================================
// BoundingBox Tests
// =============================================================================

TEST(BoundingBoxTest, Default) {
    BoundingBox box;
    EXPECT_TRUE(box.isEmpty());
}

TEST(BoundingBoxTest, CenterAndSize) {
    BoundingBox box;
    box.mins = Vec3(-10.0f, -20.0f, -5.0f);
    box.maxs = Vec3(10.0f, 20.0f, 5.0f);
    
    Vec3 center = box.center();
    EXPECT_FLOAT_EQ(center[0], 0.0f);
    EXPECT_FLOAT_EQ(center[1], 0.0f);
    EXPECT_FLOAT_EQ(center[2], 0.0f);
    
    Vec3 size = box.size();
    EXPECT_FLOAT_EQ(size[0], 20.0f);
    EXPECT_FLOAT_EQ(size[1], 40.0f);
    EXPECT_FLOAT_EQ(size[2], 10.0f);
}

TEST(BoundingBoxTest, Contains) {
    BoundingBox box;
    box.mins = Vec3(-10.0f, -10.0f, -10.0f);
    box.maxs = Vec3(10.0f, 10.0f, 10.0f);
    
    EXPECT_TRUE(box.contains(Vec3(0.0f, 0.0f, 0.0f)));
    EXPECT_TRUE(box.contains(Vec3(5.0f, 5.0f, 5.0f)));
    EXPECT_TRUE(box.contains(Vec3(-10.0f, -10.0f, -10.0f)));
    EXPECT_FALSE(box.contains(Vec3(15.0f, 0.0f, 0.0f)));
}

TEST(BoundingBoxTest, Intersects) {
    BoundingBox box1;
    box1.mins = Vec3(0.0f, 0.0f, 0.0f);
    box1.maxs = Vec3(10.0f, 10.0f, 10.0f);
    
    BoundingBox box2;
    box2.mins = Vec3(5.0f, 5.0f, 5.0f);
    box2.maxs = Vec3(15.0f, 15.0f, 15.0f);
    
    EXPECT_TRUE(box1.intersects(box2));
    
    BoundingBox box3;
    box3.mins = Vec3(20.0f, 20.0f, 20.0f);
    box3.maxs = Vec3(30.0f, 30.0f, 30.0f);
    
    EXPECT_FALSE(box1.intersects(box3));
}

TEST(BoundingBoxTest, Expand) {
    BoundingBox box;
    box.mins = Vec3(-5.0f, -5.0f, -5.0f);
    box.maxs = Vec3(5.0f, 5.0f, 5.0f);
    
    box.expand(2.0f);
    
    EXPECT_FLOAT_EQ(box.mins[0], -7.0f);
    EXPECT_FLOAT_EQ(box.maxs[0], 7.0f);
}

TEST(BoundingBoxTest, ExpandForItem) {
    BoundingBox box;
    box.mins = Vec3(-5.0f, -5.0f, -5.0f);
    box.maxs = Vec3(5.0f, 5.0f, 5.0f);
    
    box.expandForItem();
    
    EXPECT_FLOAT_EQ(box.mins[0], -20.0f);  // -5 - 15
    EXPECT_FLOAT_EQ(box.mins[1], -20.0f);
    EXPECT_FLOAT_EQ(box.mins[2], -5.0f);   // Z unchanged
    EXPECT_FLOAT_EQ(box.maxs[0], 20.0f);
    EXPECT_FLOAT_EQ(box.maxs[1], 20.0f);
    EXPECT_FLOAT_EQ(box.maxs[2], 5.0f);    // Z unchanged
}

TEST(BoundingBoxTest, FromOriginAndSize) {
    Vec3 origin(100.0f, 200.0f, 300.0f);
    Vec3 mins(-16.0f, -16.0f, -24.0f);
    Vec3 maxs(16.0f, 16.0f, 32.0f);
    
    BoundingBox box = BoundingBox::fromOriginAndSize(origin, mins, maxs);
    
    EXPECT_FLOAT_EQ(box.mins[0], 84.0f);
    EXPECT_FLOAT_EQ(box.maxs[0], 116.0f);
}

// =============================================================================
// MoveClip Tests
// =============================================================================

TEST(MoveClipTest, Default) {
    MoveClip clip;
    EXPECT_EQ(clip.type, 0u);
    EXPECT_EQ(clip.passEntityNum, -1);
}

TEST(MoveClipTest, Clear) {
    MoveClip clip;
    clip.type = 3;
    clip.passEntityNum = 5;
    clip.start = Vec3(100.0f, 200.0f, 300.0f);
    
    clip.clear();
    
    EXPECT_EQ(clip.type, 0u);
    EXPECT_EQ(clip.passEntityNum, -1);
    EXPECT_FLOAT_EQ(clip.start[0], 0.0f);
}

// =============================================================================
// AreaNode Tests
// =============================================================================

TEST(AreaNodeTest, Default) {
    AreaNode node;
    EXPECT_TRUE(node.isLeaf());
    EXPECT_EQ(node.getAxis(), -1);
}

TEST(AreaNodeTest, ClassifyPoint) {
    AreaNode node;
    node.axis = 0;  // X axis
    node.dist = 100.0f;
    
    // Point on positive side (X >= dist)
    EXPECT_EQ(node.classifyPoint(Vec3(150.0f, 0.0f, 0.0f)), 0);
    
    // Point on negative side (X < dist)
    EXPECT_EQ(node.classifyPoint(Vec3(50.0f, 0.0f, 0.0f)), 1);
}

// =============================================================================
// PhysicsClipVelocity Tests
// =============================================================================

TEST(PhysicsClipVelocityTest, HitFloor) {
    Vec3 velocity(100.0f, 0.0f, -300.0f);
    Vec3 normal(0.0f, 0.0f, 1.0f);
    
    Vec3 clipped = clipVelocity(velocity, normal, 1.0f);
    
    // Horizontal preserved, vertical zeroed
    EXPECT_FLOAT_EQ(clipped[0], 100.0f);
    EXPECT_FLOAT_EQ(clipped[1], 0.0f);
    EXPECT_FLOAT_EQ(clipped[2], 0.0f);
}

TEST(PhysicsClipVelocityTest, HitWall) {
    Vec3 velocity(100.0f, 0.0f, -50.0f);
    Vec3 normal(-1.0f, 0.0f, 0.0f);
    
    Vec3 clipped = clipVelocity(velocity, normal, 1.0f);
    
    // X zeroed, others preserved
    EXPECT_FLOAT_EQ(clipped[0], 0.0f);
    EXPECT_FLOAT_EQ(clipped[1], 0.0f);
    EXPECT_FLOAT_EQ(clipped[2], -50.0f);
}

TEST(PhysicsClipVelocityTest, Overbounce) {
    Vec3 velocity(0.0f, 0.0f, -100.0f);
    Vec3 normal(0.0f, 0.0f, 1.0f);
    
    Vec3 clipped = clipVelocity(velocity, normal, 1.5f);
    
    // With overbounce > 1, velocity bounces back
    EXPECT_FLOAT_EQ(clipped[2], 50.0f);  // -100 - (-100 * 1.5) = 50
}

// =============================================================================
// ApplyGravity Tests
// =============================================================================

TEST(ApplyGravityTest, Standard) {
    Vec3 velocity(100.0f, 0.0f, 0.0f);
    applyGravity(velocity, 1.0f, 800.0f, 0.1f);
    
    // Z should decrease by gravity * frametime
    EXPECT_FLOAT_EQ(velocity[0], 100.0f);  // unchanged
    EXPECT_FLOAT_EQ(velocity[2], -80.0f);  // -800 * 0.1
}

TEST(ApplyGravityTest, Scaled) {
    Vec3 velocity(0.0f, 0.0f, 0.0f);
    applyGravity(velocity, 0.5f, 800.0f, 0.1f);  // Half gravity
    
    EXPECT_FLOAT_EQ(velocity[2], -40.0f);
}

// =============================================================================
// Floor Check Tests
// =============================================================================

TEST(FloorCheckTest, IsFloorNormal) {
    EXPECT_TRUE(isFloorNormal(1.0f));
    EXPECT_TRUE(isFloorNormal(0.71f));
    EXPECT_FALSE(isFloorNormal(0.7f));
    EXPECT_FALSE(isFloorNormal(0.5f));
    EXPECT_FALSE(isFloorNormal(0.0f));
}

TEST(FloorCheckTest, IsWallNormal) {
    EXPECT_TRUE(isWallNormal(0.0f));
    EXPECT_FALSE(isWallNormal(0.1f));
    EXPECT_FALSE(isWallNormal(-0.1f));
}

// =============================================================================
// PhysicsMoveVars Tests
// =============================================================================

TEST(PhysicsMoveVarsTest, Defaults) {
    MoveVars vars;
    EXPECT_FLOAT_EQ(vars.gravity, 800.0f);
    EXPECT_FLOAT_EQ(vars.maxSpeed, 320.0f);
    EXPECT_FLOAT_EQ(vars.friction, 4.0f);
}

TEST(PhysicsMoveVarsTest, SetDefaults) {
    MoveVars vars;
    vars.gravity = 1000.0f;
    vars.maxSpeed = 500.0f;
    
    vars.setDefaults();
    
    EXPECT_FLOAT_EQ(vars.gravity, 800.0f);
    EXPECT_FLOAT_EQ(vars.maxSpeed, 320.0f);
}

// =============================================================================
// ClampVelocity Tests
// =============================================================================

TEST(ClampVelocityTest, BelowMax) {
    Vec3 velocity(100.0f, 0.0f, 0.0f);
    bool clamped = clampVelocity(velocity, 2000.0f);
    
    EXPECT_FALSE(clamped);
    EXPECT_FLOAT_EQ(velocity[0], 100.0f);
}

TEST(ClampVelocityTest, AboveMax) {
    Vec3 velocity(3000.0f, 0.0f, 0.0f);
    bool clamped = clampVelocity(velocity, 2000.0f);
    
    EXPECT_TRUE(clamped);
    EXPECT_FLOAT_EQ(velocity[0], 2000.0f);
}

TEST(ClampVelocityTest, DiagonalAboveMax) {
    Vec3 velocity(1500.0f, 1500.0f, 0.0f);  // ~2121 length
    bool clamped = clampVelocity(velocity, 2000.0f);
    
    EXPECT_TRUE(clamped);
    // Both components should be scaled proportionally
    float expectedComponent = 2000.0f / std::sqrt(2.0f);
    EXPECT_NEAR(velocity[0], expectedComponent, 1.0f);
    EXPECT_NEAR(velocity[1], expectedComponent, 1.0f);
}

// =============================================================================
// FixNaN Tests
// =============================================================================

TEST(FixNaNTest, NoNaN) {
    Vec3 v(100.0f, 200.0f, 300.0f);
    bool fixed = fixNaN(v);
    
    EXPECT_FALSE(fixed);
    EXPECT_FLOAT_EQ(v[0], 100.0f);
}

TEST(FixNaNTest, WithNaN) {
    Vec3 v(std::nanf(""), 200.0f, 300.0f);
    bool fixed = fixNaN(v);
    
    EXPECT_TRUE(fixed);
    EXPECT_FLOAT_EQ(v[0], 0.0f);
    EXPECT_FLOAT_EQ(v[1], 200.0f);
}

// =============================================================================
// ContentType Tests
// =============================================================================

TEST(ContentTypeTest, Values) {
    EXPECT_EQ(static_cast<int>(ContentType::Empty), -1);
    EXPECT_EQ(static_cast<int>(ContentType::Solid), -2);
    EXPECT_EQ(static_cast<int>(ContentType::Water), -3);
    EXPECT_EQ(static_cast<int>(ContentType::Slime), -4);
    EXPECT_EQ(static_cast<int>(ContentType::Lava), -5);
}

TEST(ContentTypeTest, Names) {
    EXPECT_EQ(contentTypeName(ContentType::Empty), "empty");
    EXPECT_EQ(contentTypeName(ContentType::Solid), "solid");
    EXPECT_EQ(contentTypeName(ContentType::Water), "water");
    EXPECT_EQ(contentTypeName(ContentType::Lava), "lava");
}

TEST(ContentTypeTest, IsLiquid) {
    EXPECT_FALSE(isLiquid(ContentType::Empty));
    EXPECT_FALSE(isLiquid(ContentType::Solid));
    EXPECT_TRUE(isLiquid(ContentType::Water));
    EXPECT_TRUE(isLiquid(ContentType::Slime));
    EXPECT_TRUE(isLiquid(ContentType::Lava));
}

TEST(ContentTypeTest, IsSolid) {
    EXPECT_TRUE(isSolid(ContentType::Solid));
    EXPECT_FALSE(isSolid(ContentType::Empty));
    EXPECT_FALSE(isSolid(ContentType::Water));
}

TEST(ContentTypeTest, IsDamaging) {
    EXPECT_TRUE(isDamaging(ContentType::Slime));
    EXPECT_TRUE(isDamaging(ContentType::Lava));
    EXPECT_FALSE(isDamaging(ContentType::Water));
    EXPECT_FALSE(isDamaging(ContentType::Empty));
}

TEST(ContentTypeTest, IsCurrent) {
    EXPECT_TRUE(isCurrent(ContentType::Current0));
    EXPECT_TRUE(isCurrent(ContentType::CurrentUp));
    EXPECT_TRUE(isCurrent(ContentType::CurrentDown));
    EXPECT_FALSE(isCurrent(ContentType::Water));
    EXPECT_FALSE(isCurrent(ContentType::Solid));
}

// =============================================================================
// PushResult Tests
// =============================================================================

TEST(PushResultTest, Default) {
    PushResult result;
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.wasBlocked());
}

TEST(PushResultTest, Blocked) {
    PushResult result;
    result.blocked = BlockedFlags::Floor;
    EXPECT_TRUE(result.wasBlocked());
}

// =============================================================================
// StepMoveResult Tests
// =============================================================================

TEST(StepMoveResultTest, Default) {
    StepMoveResult result;
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.partialGround);
}

TEST(StepMoveResultTest, WalkedOffEdge) {
    StepMoveResult result;
    result.success = false;
    result.partialGround = false;
    EXPECT_TRUE(result.walkedOffEdge());
    
    result.partialGround = true;
    EXPECT_FALSE(result.walkedOffEdge());
}

// =============================================================================
// FlyMoveResult Tests
// =============================================================================

TEST(FlyMoveResultTest, Default) {
    FlyMoveResult result;
    EXPECT_EQ(result.blocked, 0);
    EXPECT_FALSE(result.hitFloor());
    EXPECT_FALSE(result.hitStep());
    EXPECT_FALSE(result.wasTrapped());
}

TEST(FlyMoveResultTest, HitFloor) {
    FlyMoveResult result;
    result.blocked = BlockedFlags::Floor;
    EXPECT_TRUE(result.hitFloor());
    EXPECT_FALSE(result.hitStep());
}

TEST(FlyMoveResultTest, Trapped) {
    FlyMoveResult result;
    result.blocked = BlockedFlags::Floor | BlockedFlags::Step | BlockedFlags::DeadStop;
    EXPECT_TRUE(result.wasTrapped());
}
