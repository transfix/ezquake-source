/*
 * ezQuake C++ Port - Vec3 Unit Tests
 * 
 * Comprehensive tests for the Vec3 class, validating all operations
 * match the original C mathlib behavior.
 * 
 * Copyright (C) 2024 ezQuake Team
 */

#include <gtest/gtest.h>
#include <cmath>
#include "core/math/vec3.hpp"

using namespace ezquake;
using namespace ezquake::math;

//=============================================================================
// Test Fixture
//=============================================================================

class Vec3Test : public ::testing::Test {
protected:
    static constexpr VecT EPSILON = 0.0001f;
    
    bool approxEqual(VecT a, VecT b, VecT eps = EPSILON) {
        return std::abs(a - b) < eps;
    }
    
    bool vecApproxEqual(const Vec3& a, const Vec3& b, VecT eps = EPSILON) {
        return a.approxEqual(b, eps);
    }
};

//=============================================================================
// Constructor Tests
//=============================================================================

TEST_F(Vec3Test, DefaultConstructorIsZero) {
    Vec3 v;
    EXPECT_EQ(v.x(), 0.0f);
    EXPECT_EQ(v.y(), 0.0f);
    EXPECT_EQ(v.z(), 0.0f);
}

TEST_F(Vec3Test, ComponentConstructor) {
    Vec3 v(1.0f, 2.0f, 3.0f);
    EXPECT_EQ(v.x(), 1.0f);
    EXPECT_EQ(v.y(), 2.0f);
    EXPECT_EQ(v.z(), 3.0f);
}

TEST_F(Vec3Test, ScalarConstructor) {
    Vec3 v(5.0f);
    EXPECT_EQ(v.x(), 5.0f);
    EXPECT_EQ(v.y(), 5.0f);
    EXPECT_EQ(v.z(), 5.0f);
}

TEST_F(Vec3Test, ArrayConstructor) {
    VecT arr[3] = {1.0f, 2.0f, 3.0f};
    Vec3 v(arr);
    EXPECT_EQ(v.x(), 1.0f);
    EXPECT_EQ(v.y(), 2.0f);
    EXPECT_EQ(v.z(), 3.0f);
}

TEST_F(Vec3Test, CopyConstructor) {
    Vec3 a(1.0f, 2.0f, 3.0f);
    Vec3 b(a);
    EXPECT_EQ(b.x(), 1.0f);
    EXPECT_EQ(b.y(), 2.0f);
    EXPECT_EQ(b.z(), 3.0f);
}

//=============================================================================
// Access Tests
//=============================================================================

TEST_F(Vec3Test, IndexAccess) {
    Vec3 v(1.0f, 2.0f, 3.0f);
    EXPECT_EQ(v[0], 1.0f);
    EXPECT_EQ(v[1], 2.0f);
    EXPECT_EQ(v[2], 3.0f);
    
    v[0] = 10.0f;
    EXPECT_EQ(v.x(), 10.0f);
}

TEST_F(Vec3Test, AxisEnumAccess) {
    Vec3 v(1.0f, 2.0f, 3.0f);
    EXPECT_EQ(v[Axis::X], 1.0f);
    EXPECT_EQ(v[Axis::Y], 2.0f);
    EXPECT_EQ(v[Axis::Z], 3.0f);
    EXPECT_EQ(v[Axis::Pitch], 1.0f);
    EXPECT_EQ(v[Axis::Yaw], 2.0f);
    EXPECT_EQ(v[Axis::Roll], 3.0f);
}

TEST_F(Vec3Test, AtBoundsCheck) {
    Vec3 v(1.0f, 2.0f, 3.0f);
    EXPECT_EQ(v.at(0), 1.0f);
    EXPECT_EQ(v.at(1), 2.0f);
    EXPECT_EQ(v.at(2), 3.0f);
    EXPECT_THROW(v.at(3), std::out_of_range);
}

TEST_F(Vec3Test, NamedAccessors) {
    Vec3 v(1.0f, 2.0f, 3.0f);
    EXPECT_EQ(v.x(), 1.0f);
    EXPECT_EQ(v.y(), 2.0f);
    EXPECT_EQ(v.z(), 3.0f);
    EXPECT_EQ(v.pitch(), 1.0f);
    EXPECT_EQ(v.yaw(), 2.0f);
    EXPECT_EQ(v.roll(), 3.0f);
}

TEST_F(Vec3Test, DataPointerCompatibility) {
    Vec3 v(1.0f, 2.0f, 3.0f);
    const VecT* data = v.data();
    EXPECT_EQ(data[0], 1.0f);
    EXPECT_EQ(data[1], 2.0f);
    EXPECT_EQ(data[2], 3.0f);
    
    // Modify through pointer
    VecT* mutableData = v.data();
    mutableData[0] = 10.0f;
    EXPECT_EQ(v.x(), 10.0f);
}

TEST_F(Vec3Test, Iterators) {
    Vec3 v(1.0f, 2.0f, 3.0f);
    
    VecT sum = 0;
    for (auto val : v) {
        sum += val;
    }
    EXPECT_EQ(sum, 6.0f);
    
    // Modify through iterator
    for (auto& val : v) {
        val *= 2.0f;
    }
    EXPECT_EQ(v.x(), 2.0f);
    EXPECT_EQ(v.y(), 4.0f);
    EXPECT_EQ(v.z(), 6.0f);
}

//=============================================================================
// Arithmetic Operator Tests
//=============================================================================

TEST_F(Vec3Test, Negation) {
    Vec3 v(1.0f, -2.0f, 3.0f);
    Vec3 neg = -v;
    EXPECT_EQ(neg.x(), -1.0f);
    EXPECT_EQ(neg.y(), 2.0f);
    EXPECT_EQ(neg.z(), -3.0f);
}

TEST_F(Vec3Test, Addition) {
    Vec3 a(1.0f, 2.0f, 3.0f);
    Vec3 b(4.0f, 5.0f, 6.0f);
    Vec3 c = a + b;
    EXPECT_EQ(c.x(), 5.0f);
    EXPECT_EQ(c.y(), 7.0f);
    EXPECT_EQ(c.z(), 9.0f);
}

TEST_F(Vec3Test, Subtraction) {
    Vec3 a(5.0f, 7.0f, 9.0f);
    Vec3 b(1.0f, 2.0f, 3.0f);
    Vec3 c = a - b;
    EXPECT_EQ(c.x(), 4.0f);
    EXPECT_EQ(c.y(), 5.0f);
    EXPECT_EQ(c.z(), 6.0f);
}

TEST_F(Vec3Test, ComponentWiseMultiplication) {
    Vec3 a(2.0f, 3.0f, 4.0f);
    Vec3 b(3.0f, 4.0f, 5.0f);
    Vec3 c = a * b;
    EXPECT_EQ(c.x(), 6.0f);
    EXPECT_EQ(c.y(), 12.0f);
    EXPECT_EQ(c.z(), 20.0f);
}

TEST_F(Vec3Test, ScalarMultiplication) {
    Vec3 v(1.0f, 2.0f, 3.0f);
    Vec3 scaled = v * 2.0f;
    EXPECT_EQ(scaled.x(), 2.0f);
    EXPECT_EQ(scaled.y(), 4.0f);
    EXPECT_EQ(scaled.z(), 6.0f);
    
    // Scalar on left
    Vec3 scaled2 = 3.0f * v;
    EXPECT_EQ(scaled2.x(), 3.0f);
    EXPECT_EQ(scaled2.y(), 6.0f);
    EXPECT_EQ(scaled2.z(), 9.0f);
}

TEST_F(Vec3Test, ScalarDivision) {
    Vec3 v(2.0f, 4.0f, 6.0f);
    Vec3 divided = v / 2.0f;
    EXPECT_EQ(divided.x(), 1.0f);
    EXPECT_EQ(divided.y(), 2.0f);
    EXPECT_EQ(divided.z(), 3.0f);
}

TEST_F(Vec3Test, CompoundAssignment) {
    Vec3 v(1.0f, 2.0f, 3.0f);
    
    v += Vec3(1.0f, 1.0f, 1.0f);
    EXPECT_EQ(v, Vec3(2.0f, 3.0f, 4.0f));
    
    v -= Vec3(1.0f, 1.0f, 1.0f);
    EXPECT_EQ(v, Vec3(1.0f, 2.0f, 3.0f));
    
    v *= 2.0f;
    EXPECT_EQ(v, Vec3(2.0f, 4.0f, 6.0f));
    
    v /= 2.0f;
    EXPECT_EQ(v, Vec3(1.0f, 2.0f, 3.0f));
}

//=============================================================================
// Comparison Tests
//=============================================================================

TEST_F(Vec3Test, Equality) {
    Vec3 a(1.0f, 2.0f, 3.0f);
    Vec3 b(1.0f, 2.0f, 3.0f);
    Vec3 c(1.0f, 2.0f, 4.0f);
    
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
    EXPECT_TRUE(a != c);
    EXPECT_FALSE(a != b);
}

TEST_F(Vec3Test, ApproximateEquality) {
    Vec3 a(1.0f, 2.0f, 3.0f);
    Vec3 b(1.00001f, 2.00001f, 3.00001f);
    Vec3 c(1.1f, 2.0f, 3.0f);
    
    EXPECT_TRUE(a.approxEqual(b, 0.001f));
    EXPECT_FALSE(a.approxEqual(c, 0.001f));
}

//=============================================================================
// Vector Operation Tests
//=============================================================================

TEST_F(Vec3Test, DotProduct) {
    Vec3 a(1.0f, 2.0f, 3.0f);
    Vec3 b(4.0f, 5.0f, 6.0f);
    
    // 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32
    EXPECT_EQ(a.dot(b), 32.0f);
    EXPECT_EQ(dotProduct(a, b), 32.0f);
    
    // Self dot = length squared
    EXPECT_EQ(a.dot(a), a.lengthSquared());
}

TEST_F(Vec3Test, CrossProduct) {
    Vec3 x = Vec3::UNIT_X;
    Vec3 y = Vec3::UNIT_Y;
    Vec3 z = Vec3::UNIT_Z;
    
    // X cross Y = Z
    EXPECT_TRUE(vecApproxEqual(x.cross(y), z));
    // Y cross Z = X
    EXPECT_TRUE(vecApproxEqual(y.cross(z), x));
    // Z cross X = Y
    EXPECT_TRUE(vecApproxEqual(z.cross(x), y));
    
    // Reverse order gives negative
    EXPECT_TRUE(vecApproxEqual(y.cross(x), -z));
}

TEST_F(Vec3Test, Length) {
    Vec3 v(3.0f, 4.0f, 0.0f);
    EXPECT_EQ(v.length(), 5.0f);
    EXPECT_EQ(v.lengthSquared(), 25.0f);
    
    Vec3 unit = Vec3::UNIT_X;
    EXPECT_EQ(unit.length(), 1.0f);
}

TEST_F(Vec3Test, Normalize) {
    Vec3 v(3.0f, 4.0f, 0.0f);
    VecT oldLen = v.normalize();
    
    EXPECT_EQ(oldLen, 5.0f);
    EXPECT_TRUE(approxEqual(v.length(), 1.0f));
    EXPECT_TRUE(vecApproxEqual(v, Vec3(0.6f, 0.8f, 0.0f)));
}

TEST_F(Vec3Test, NormalizeZeroVector) {
    Vec3 v(0.0f, 0.0f, 0.0f);
    VecT len = v.normalize();
    
    EXPECT_EQ(len, 0.0f);
    EXPECT_EQ(v, Vec3::ORIGIN); // Stays zero
}

TEST_F(Vec3Test, Normalized) {
    Vec3 v(3.0f, 4.0f, 0.0f);
    Vec3 n = v.normalized();
    
    // Original unchanged
    EXPECT_EQ(v, Vec3(3.0f, 4.0f, 0.0f));
    // Normalized version
    EXPECT_TRUE(approxEqual(n.length(), 1.0f));
}

TEST_F(Vec3Test, NormalizeFast) {
    Vec3 v(3.0f, 4.0f, 0.0f);
    v.normalizeFast();
    
    // Fast normalize is approximate
    EXPECT_TRUE(approxEqual(v.length(), 1.0f, 0.01f));
}

TEST_F(Vec3Test, Distance) {
    Vec3 a(0.0f, 0.0f, 0.0f);
    Vec3 b(3.0f, 4.0f, 0.0f);
    
    EXPECT_EQ(a.distanceTo(b), 5.0f);
    EXPECT_EQ(a.distanceSquaredTo(b), 25.0f);
}

TEST_F(Vec3Test, Lerp) {
    Vec3 a(0.0f, 0.0f, 0.0f);
    Vec3 b(10.0f, 10.0f, 10.0f);
    
    EXPECT_EQ(a.lerp(b, 0.0f), a);
    EXPECT_EQ(a.lerp(b, 1.0f), b);
    EXPECT_EQ(a.lerp(b, 0.5f), Vec3(5.0f, 5.0f, 5.0f));
}

TEST_F(Vec3Test, AngleLerp) {
    Vec3 a(0.0f, 0.0f, 0.0f);
    Vec3 b(350.0f, 350.0f, 350.0f); // Should wrap around, not go 0->350
    
    Vec3 mid = a.angleLerp(b, 0.5f);
    // 0 to 350 should go through -10 (355 degrees), midpoint at -5 or 355
    EXPECT_TRUE(mid.x() < 0.0f || mid.x() > 345.0f);
}

TEST_F(Vec3Test, ProjectOnto) {
    Vec3 v(3.0f, 4.0f, 0.0f);
    Vec3 onto(1.0f, 0.0f, 0.0f);
    
    Vec3 proj = v.projectOnto(onto);
    EXPECT_TRUE(vecApproxEqual(proj, Vec3(3.0f, 0.0f, 0.0f)));
}

TEST_F(Vec3Test, Reflect) {
    Vec3 v(1.0f, -1.0f, 0.0f);
    Vec3 normal(0.0f, 1.0f, 0.0f);
    
    Vec3 reflected = v.reflect(normal);
    EXPECT_TRUE(vecApproxEqual(reflected, Vec3(1.0f, 1.0f, 0.0f)));
}

//=============================================================================
// Utility Operation Tests
//=============================================================================

TEST_F(Vec3Test, Clear) {
    Vec3 v(1.0f, 2.0f, 3.0f);
    v.clear();
    EXPECT_EQ(v, Vec3::ORIGIN);
}

TEST_F(Vec3Test, Set) {
    Vec3 v;
    v.set(1.0f, 2.0f, 3.0f);
    EXPECT_EQ(v, Vec3(1.0f, 2.0f, 3.0f));
}

TEST_F(Vec3Test, CopyFromArray) {
    VecT arr[3] = {1.0f, 2.0f, 3.0f};
    Vec3 v;
    v.copyFrom(arr);
    EXPECT_EQ(v, Vec3(1.0f, 2.0f, 3.0f));
}

TEST_F(Vec3Test, CopyToArray) {
    Vec3 v(1.0f, 2.0f, 3.0f);
    VecT arr[3] = {0.0f, 0.0f, 0.0f};
    v.copyTo(arr);
    EXPECT_EQ(arr[0], 1.0f);
    EXPECT_EQ(arr[1], 2.0f);
    EXPECT_EQ(arr[2], 3.0f);
}

TEST_F(Vec3Test, IsZero) {
    Vec3 zero;
    Vec3 almostZero(0.00001f, 0.00001f, 0.00001f);
    Vec3 notZero(1.0f, 0.0f, 0.0f);
    
    EXPECT_TRUE(zero.isZero());
    EXPECT_TRUE(almostZero.isZero(0.001f));
    EXPECT_FALSE(notZero.isZero());
}

TEST_F(Vec3Test, HasNaN) {
    Vec3 normal(1.0f, 2.0f, 3.0f);
    Vec3 withNaN(1.0f, std::numeric_limits<VecT>::quiet_NaN(), 3.0f);
    
    EXPECT_FALSE(normal.hasNaN());
    EXPECT_TRUE(withNaN.hasNaN());
}

TEST_F(Vec3Test, HasInf) {
    Vec3 normal(1.0f, 2.0f, 3.0f);
    Vec3 withInf(1.0f, std::numeric_limits<VecT>::infinity(), 3.0f);
    
    EXPECT_FALSE(normal.hasInf());
    EXPECT_TRUE(withInf.hasInf());
}

TEST_F(Vec3Test, IsValid) {
    Vec3 normal(1.0f, 2.0f, 3.0f);
    Vec3 withNaN(1.0f, std::numeric_limits<VecT>::quiet_NaN(), 3.0f);
    Vec3 withInf(1.0f, std::numeric_limits<VecT>::infinity(), 3.0f);
    
    EXPECT_TRUE(normal.isValid());
    EXPECT_FALSE(withNaN.isValid());
    EXPECT_FALSE(withInf.isValid());
}

TEST_F(Vec3Test, Clamp) {
    Vec3 v(-1.0f, 5.0f, 10.0f);
    v.clamp(0.0f, 8.0f);
    EXPECT_EQ(v, Vec3(0.0f, 5.0f, 8.0f));
}

TEST_F(Vec3Test, Clamped) {
    Vec3 v(-1.0f, 5.0f, 10.0f);
    Vec3 clamped = v.clamped(0.0f, 8.0f);
    
    EXPECT_EQ(v, Vec3(-1.0f, 5.0f, 10.0f)); // Original unchanged
    EXPECT_EQ(clamped, Vec3(0.0f, 5.0f, 8.0f));
}

TEST_F(Vec3Test, MinMax) {
    Vec3 a(1.0f, 5.0f, 3.0f);
    Vec3 b(2.0f, 3.0f, 4.0f);
    
    EXPECT_EQ(a.min(b), Vec3(1.0f, 3.0f, 3.0f));
    EXPECT_EQ(a.max(b), Vec3(2.0f, 5.0f, 4.0f));
}

TEST_F(Vec3Test, Abs) {
    Vec3 v(-1.0f, 2.0f, -3.0f);
    EXPECT_EQ(v.abs(), Vec3(1.0f, 2.0f, 3.0f));
}

//=============================================================================
// Static Constant Tests
//=============================================================================

TEST_F(Vec3Test, StaticConstants) {
    EXPECT_EQ(Vec3::ORIGIN, Vec3(0.0f, 0.0f, 0.0f));
    EXPECT_EQ(Vec3::UNIT_X, Vec3(1.0f, 0.0f, 0.0f));
    EXPECT_EQ(Vec3::UNIT_Y, Vec3(0.0f, 1.0f, 0.0f));
    EXPECT_EQ(Vec3::UNIT_Z, Vec3(0.0f, 0.0f, 1.0f));
    
    EXPECT_TRUE(approxEqual(Vec3::UNIT_X.length(), 1.0f));
    EXPECT_TRUE(approxEqual(Vec3::UNIT_Y.length(), 1.0f));
    EXPECT_TRUE(approxEqual(Vec3::UNIT_Z.length(), 1.0f));
}

//=============================================================================
// AngleVectors Compatibility Test
//=============================================================================

TEST_F(Vec3Test, ToDirections) {
    // Test that AngleVectors works correctly
    Vec3 angles(0.0f, 0.0f, 0.0f); // Looking along +X
    Vec3 forward, right, up;
    
    angles.toDirections(forward, right, up);
    
    EXPECT_TRUE(vecApproxEqual(forward, Vec3(1.0f, 0.0f, 0.0f)));
    
    // 90 degree yaw should look along +Y
    Vec3 angles90(0.0f, 90.0f, 0.0f);
    angles90.toDirections(forward, right, up);
    
    EXPECT_TRUE(vecApproxEqual(forward, Vec3(0.0f, 1.0f, 0.0f)));
}

TEST_F(Vec3Test, ToForward) {
    Vec3 angles(0.0f, 0.0f, 0.0f);
    Vec3 forward = angles.toForward();
    
    EXPECT_TRUE(vecApproxEqual(forward, Vec3(1.0f, 0.0f, 0.0f)));
}

//=============================================================================
// Free Function Tests
//=============================================================================

TEST_F(Vec3Test, FreeFunctions) {
    Vec3 a(1.0f, 2.0f, 3.0f);
    Vec3 b(4.0f, 5.0f, 6.0f);
    
    EXPECT_EQ(dotProduct(a, b), a.dot(b));
    EXPECT_EQ(crossProduct(a, b), a.cross(b));
    EXPECT_EQ(vectorLength(a), a.length());
    
    Vec3 v = a;
    VecT len = vectorNormalize(v);
    EXPECT_TRUE(approxEqual(len, a.length()));
    EXPECT_TRUE(approxEqual(v.length(), 1.0f));
}

TEST_F(Vec3Test, VectorMA) {
    Vec3 a(1.0f, 2.0f, 3.0f);
    Vec3 b(1.0f, 1.0f, 1.0f);
    
    Vec3 result = vectorMA(a, 2.0f, b);
    EXPECT_EQ(result, Vec3(3.0f, 4.0f, 5.0f));
}

TEST_F(Vec3Test, DegRadConversion) {
    EXPECT_TRUE(approxEqual(degToRad(180.0f), constants::PI));
    EXPECT_TRUE(approxEqual(degToRad(90.0f), constants::HALF_PI));
    EXPECT_TRUE(approxEqual(radToDeg(constants::PI), 180.0f));
}
