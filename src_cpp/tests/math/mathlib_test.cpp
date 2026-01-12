/*
 * ezQuake C++ Port - Math Library Unit Tests
 * 
 * Copyright (C) 2024 ezQuake Team
 */

#include <gtest/gtest.h>
#include <cmath>
#include "core/math/mathlib.hpp"

using namespace ezquake;
using namespace ezquake::math;

class MathLibTest : public ::testing::Test {
protected:
    static constexpr VecT EPSILON = 0.0001f;
    
    bool approxEqual(VecT a, VecT b, VecT eps = EPSILON) {
        return std::abs(a - b) < eps;
    }
};

//=============================================================================
// Plane Tests
//=============================================================================

TEST_F(MathLibTest, PlaneConstruction) {
    Vec3 normal(0.0f, 0.0f, 1.0f);
    VecT dist = 5.0f;
    
    Plane p(normal, dist);
    
    EXPECT_EQ(p.normal, normal);
    EXPECT_EQ(p.dist, dist);
    EXPECT_EQ(p.type, 2); // Z-aligned plane
}

TEST_F(MathLibTest, PlaneFromPoints) {
    Vec3 p1(0.0f, 0.0f, 0.0f);
    Vec3 p2(1.0f, 0.0f, 0.0f);
    Vec3 p3(0.0f, 1.0f, 0.0f);
    
    Plane p = Plane::fromPoints(p1, p2, p3);
    
    // Should be XY plane (normal along Z)
    EXPECT_TRUE(p.normal.approxEqual(Vec3(0.0f, 0.0f, 1.0f)));
    EXPECT_TRUE(approxEqual(p.dist, 0.0f));
}

TEST_F(MathLibTest, PlaneDistanceToPoint) {
    Plane p(Vec3(0.0f, 0.0f, 1.0f), 5.0f);
    
    Vec3 above(0.0f, 0.0f, 10.0f);
    Vec3 below(0.0f, 0.0f, 0.0f);
    Vec3 onPlane(0.0f, 0.0f, 5.0f);
    
    EXPECT_TRUE(approxEqual(p.distanceToPoint(above), 5.0f));
    EXPECT_TRUE(approxEqual(p.distanceToPoint(below), -5.0f));
    EXPECT_TRUE(approxEqual(p.distanceToPoint(onPlane), 0.0f));
}

TEST_F(MathLibTest, PlanePointSide) {
    Plane p(Vec3(0.0f, 0.0f, 1.0f), 5.0f);
    
    Vec3 front(0.0f, 0.0f, 10.0f);
    Vec3 back(0.0f, 0.0f, 0.0f);
    Vec3 onPlane(0.0f, 0.0f, 5.0f);
    
    EXPECT_EQ(p.pointSide(front), 1);   // Front
    EXPECT_EQ(p.pointSide(back), 2);    // Back
    EXPECT_EQ(p.pointSide(onPlane), 3); // On plane
}

//=============================================================================
// Box-Plane Tests
//=============================================================================

TEST_F(MathLibTest, BoxOnPlaneSideAxial) {
    // Z-aligned plane at z=5
    Plane p(Vec3(0.0f, 0.0f, 1.0f), 5.0f);
    p.updateType();
    
    // Box entirely above plane
    Vec3 minsAbove(0.0f, 0.0f, 6.0f);
    Vec3 maxsAbove(1.0f, 1.0f, 10.0f);
    EXPECT_EQ(boxOnPlaneSide(minsAbove, maxsAbove, p), 1);
    
    // Box entirely below plane
    Vec3 minsBelow(0.0f, 0.0f, 0.0f);
    Vec3 maxsBelow(1.0f, 1.0f, 4.0f);
    EXPECT_EQ(boxOnPlaneSide(minsBelow, maxsBelow, p), 2);
    
    // Box crossing plane
    Vec3 minsCross(0.0f, 0.0f, 0.0f);
    Vec3 maxsCross(1.0f, 1.0f, 10.0f);
    EXPECT_EQ(boxOnPlaneSide(minsCross, maxsCross, p), 3);
}

//=============================================================================
// Vector-Plane Operations
//=============================================================================

TEST_F(MathLibTest, ProjectPointOnPlane) {
    Vec3 point(1.0f, 2.0f, 3.0f);
    Vec3 normal(0.0f, 0.0f, 1.0f);
    
    Vec3 projected = projectPointOnPlane(point, normal);
    
    EXPECT_TRUE(approxEqual(projected.z(), 0.0f));
    EXPECT_TRUE(approxEqual(projected.x(), 1.0f));
    EXPECT_TRUE(approxEqual(projected.y(), 2.0f));
}

TEST_F(MathLibTest, PerpendicularVector) {
    Vec3 v(1.0f, 0.0f, 0.0f);
    Vec3 perp = perpendicularVector(v);
    
    // Should be perpendicular (dot product = 0)
    EXPECT_TRUE(approxEqual(v.dot(perp), 0.0f));
}

TEST_F(MathLibTest, RotatePointAroundVector) {
    Vec3 point(1.0f, 0.0f, 0.0f);
    Vec3 axis(0.0f, 0.0f, 1.0f);
    
    Vec3 rotated = rotatePointAroundVector(axis, point, 90.0f);
    
    EXPECT_TRUE(rotated.approxEqual(Vec3(0.0f, 1.0f, 0.0f), 0.001f));
}

TEST_F(MathLibTest, VectorVectors) {
    Vec3 forward(1.0f, 0.0f, 0.0f);
    Vec3 right, up;
    
    vectorVectors(forward, right, up);
    
    // All should be perpendicular
    EXPECT_TRUE(approxEqual(forward.dot(right), 0.0f));
    EXPECT_TRUE(approxEqual(forward.dot(up), 0.0f));
    EXPECT_TRUE(approxEqual(right.dot(up), 0.0f));
    
    // Should form right-handed system
    Vec3 cross = right.cross(forward);
    EXPECT_TRUE(cross.approxEqual(up, 0.001f));
}

//=============================================================================
// Angle Utilities
//=============================================================================

TEST_F(MathLibTest, AngleMod) {
    EXPECT_TRUE(approxEqual(angleMod(0.0f), 0.0f));
    EXPECT_TRUE(approxEqual(angleMod(360.0f), 0.0f, 0.01f));
    EXPECT_TRUE(approxEqual(angleMod(450.0f), 90.0f, 0.01f));
    EXPECT_TRUE(approxEqual(angleMod(-90.0f), 270.0f, 0.01f));
}

TEST_F(MathLibTest, AngleNormalize180) {
    EXPECT_TRUE(approxEqual(angleNormalize180(0.0f), 0.0f));
    EXPECT_TRUE(approxEqual(angleNormalize180(270.0f), -90.0f, 0.01f));
    EXPECT_TRUE(approxEqual(angleNormalize180(-270.0f), 90.0f, 0.01f));
}

TEST_F(MathLibTest, AngleDiff) {
    EXPECT_TRUE(approxEqual(angleDiff(0.0f, 90.0f), 90.0f, 0.01f));
    EXPECT_TRUE(approxEqual(angleDiff(0.0f, 270.0f), -90.0f, 0.01f));
    EXPECT_TRUE(approxEqual(angleDiff(350.0f, 10.0f), 20.0f, 0.01f));
}

//=============================================================================
// Number Theory
//=============================================================================

TEST_F(MathLibTest, FloorDivMod) {
    Int32 q, r;
    
    floorDivMod(10.0, 3.0, q, r);
    EXPECT_EQ(q, 3);
    EXPECT_EQ(r, 1);
    
    floorDivMod(-10.0, 3.0, q, r);
    EXPECT_EQ(q, -4);
    EXPECT_EQ(r, 2);
}

TEST_F(MathLibTest, GCD) {
    EXPECT_EQ(gcd(48, 18), 6);
    EXPECT_EQ(gcd(100, 25), 25);
    EXPECT_EQ(gcd(17, 13), 1);
    EXPECT_EQ(gcd(-48, 18), 6);
}

//=============================================================================
// Polygon Utilities
//=============================================================================

TEST_F(MathLibTest, PolyCentroid) {
    // Triangle with vertices at (0,0), (2,0), (1,2)
    Vec3 vertices[3] = {
        Vec3(0.0f, 0.0f, 0.0f),
        Vec3(2.0f, 0.0f, 0.0f),
        Vec3(1.0f, 2.0f, 0.0f)
    };
    
    VecT cx, cy, area;
    Int32 result = getPolyCentroid(vertices, 3, cx, cy, area);
    
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(approxEqual(cx, 1.0f));
    EXPECT_TRUE(approxEqual(cy, 2.0f / 3.0f));
    EXPECT_TRUE(approxEqual(area, 2.0f)); // Triangle area = 0.5 * base * height = 0.5 * 2 * 2 = 2
}

//=============================================================================
// Random Number Generation
//=============================================================================

TEST_F(MathLibTest, RandomFloat) {
    for (int i = 0; i < 100; ++i) {
        VecT r = randomFloat();
        EXPECT_GE(r, 0.0f);
        EXPECT_LT(r, 1.0f);
    }
}

TEST_F(MathLibTest, RandomFloatRange) {
    for (int i = 0; i < 100; ++i) {
        VecT r = randomFloat(5.0f, 10.0f);
        EXPECT_GE(r, 5.0f);
        EXPECT_LT(r, 10.0f);
    }
}

TEST_F(MathLibTest, RandomInt) {
    for (int i = 0; i < 100; ++i) {
        Int32 r = randomInt(10);
        EXPECT_GE(r, 0);
        EXPECT_LT(r, 10);
    }
}

TEST_F(MathLibTest, RandomUnitVector) {
    for (int i = 0; i < 100; ++i) {
        Vec3 v = randomUnitVector();
        EXPECT_TRUE(approxEqual(v.length(), 1.0f, 0.001f));
    }
}

TEST_F(MathLibTest, RandomInUnitSphere) {
    for (int i = 0; i < 100; ++i) {
        Vec3 v = randomInUnitSphere();
        EXPECT_LT(v.length(), 1.0f);
    }
}

//=============================================================================
// Utility Functions
//=============================================================================

TEST_F(MathLibTest, NextPowerOf2) {
    EXPECT_EQ(nextPowerOf2(1), 1u);
    EXPECT_EQ(nextPowerOf2(2), 2u);
    EXPECT_EQ(nextPowerOf2(3), 4u);
    EXPECT_EQ(nextPowerOf2(5), 8u);
    EXPECT_EQ(nextPowerOf2(100), 128u);
}

TEST_F(MathLibTest, IsPowerOf2) {
    EXPECT_TRUE(isPowerOf2(1));
    EXPECT_TRUE(isPowerOf2(2));
    EXPECT_TRUE(isPowerOf2(4));
    EXPECT_TRUE(isPowerOf2(1024));
    
    EXPECT_FALSE(isPowerOf2(0));
    EXPECT_FALSE(isPowerOf2(3));
    EXPECT_FALSE(isPowerOf2(6));
}

TEST_F(MathLibTest, Sign) {
    EXPECT_EQ(sign(5), 1);
    EXPECT_EQ(sign(-5), -1);
    EXPECT_EQ(sign(0), 0);
    EXPECT_EQ(sign(3.14f), 1);
    EXPECT_EQ(sign(-3.14f), -1);
}

TEST_F(MathLibTest, Clamp) {
    EXPECT_EQ(clamp(5, 0, 10), 5);
    EXPECT_EQ(clamp(-5, 0, 10), 0);
    EXPECT_EQ(clamp(15, 0, 10), 10);
    EXPECT_EQ(clamp(5.5f, 0.0f, 10.0f), 5.5f);
}

TEST_F(MathLibTest, Lerp) {
    EXPECT_EQ(lerp(0.0f, 10.0f, 0.0f), 0.0f);
    EXPECT_EQ(lerp(0.0f, 10.0f, 1.0f), 10.0f);
    EXPECT_EQ(lerp(0.0f, 10.0f, 0.5f), 5.0f);
    EXPECT_EQ(lerp(0.0f, 10.0f, 0.25f), 2.5f);
}

TEST_F(MathLibTest, SmoothStep) {
    EXPECT_EQ(smoothStep(0.0f, 1.0f, 0.0f), 0.0f);
    EXPECT_EQ(smoothStep(0.0f, 1.0f, 1.0f), 1.0f);
    EXPECT_EQ(smoothStep(0.0f, 1.0f, 0.5f), 0.5f);
    
    // Smooth step has zero derivative at edges
    VecT nearStart = smoothStep(0.0f, 1.0f, 0.01f);
    VecT nearEnd = smoothStep(0.0f, 1.0f, 0.99f);
    EXPECT_LT(nearStart, 0.01f); // Should be "slow" near start
    EXPECT_GT(nearEnd, 0.99f);   // Should be "slow" near end
}
