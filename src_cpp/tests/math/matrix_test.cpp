/*
 * ezQuake C++ Port - Matrix3x3 Unit Tests
 * 
 * Copyright (C) 2024 ezQuake Team
 */

#include <gtest/gtest.h>
#include <cmath>
#include "core/math/matrix.hpp"

using namespace ezquake;
using namespace ezquake::math;

class Matrix3x3Test : public ::testing::Test {
protected:
    static constexpr VecT EPSILON = 0.0001f;
    
    bool approxEqual(VecT a, VecT b, VecT eps = EPSILON) {
        return std::abs(a - b) < eps;
    }
};

//=============================================================================
// Constructor Tests
//=============================================================================

TEST_F(Matrix3x3Test, DefaultConstructorIsZero) {
    Matrix3x3 m;
    for (Size i = 0; i < 3; ++i) {
        for (Size j = 0; j < 3; ++j) {
            EXPECT_EQ(m(i, j), 0.0f);
        }
    }
}

TEST_F(Matrix3x3Test, IdentityFactory) {
    auto m = Matrix3x3::identity();
    
    EXPECT_EQ(m(0, 0), 1.0f);
    EXPECT_EQ(m(1, 1), 1.0f);
    EXPECT_EQ(m(2, 2), 1.0f);
    
    EXPECT_EQ(m(0, 1), 0.0f);
    EXPECT_EQ(m(0, 2), 0.0f);
    EXPECT_EQ(m(1, 0), 0.0f);
    EXPECT_EQ(m(1, 2), 0.0f);
    EXPECT_EQ(m(2, 0), 0.0f);
    EXPECT_EQ(m(2, 1), 0.0f);
}

TEST_F(Matrix3x3Test, ElementConstructor) {
    Matrix3x3 m(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
        7.0f, 8.0f, 9.0f
    );
    
    EXPECT_EQ(m(0, 0), 1.0f);
    EXPECT_EQ(m(0, 1), 2.0f);
    EXPECT_EQ(m(0, 2), 3.0f);
    EXPECT_EQ(m(1, 0), 4.0f);
    EXPECT_EQ(m(1, 1), 5.0f);
    EXPECT_EQ(m(1, 2), 6.0f);
    EXPECT_EQ(m(2, 0), 7.0f);
    EXPECT_EQ(m(2, 1), 8.0f);
    EXPECT_EQ(m(2, 2), 9.0f);
}

TEST_F(Matrix3x3Test, RowVectorConstructor) {
    Vec3 r0(1.0f, 2.0f, 3.0f);
    Vec3 r1(4.0f, 5.0f, 6.0f);
    Vec3 r2(7.0f, 8.0f, 9.0f);
    
    Matrix3x3 m(r0, r1, r2);
    
    EXPECT_EQ(m.row(0), r0);
    EXPECT_EQ(m.row(1), r1);
    EXPECT_EQ(m.row(2), r2);
}

//=============================================================================
// Access Tests
//=============================================================================

TEST_F(Matrix3x3Test, RowAccess) {
    Matrix3x3 m(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
        7.0f, 8.0f, 9.0f
    );
    
    EXPECT_EQ(m.row(0), Vec3(1.0f, 2.0f, 3.0f));
    EXPECT_EQ(m.row(1), Vec3(4.0f, 5.0f, 6.0f));
    EXPECT_EQ(m.row(2), Vec3(7.0f, 8.0f, 9.0f));
}

TEST_F(Matrix3x3Test, ColAccess) {
    Matrix3x3 m(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
        7.0f, 8.0f, 9.0f
    );
    
    EXPECT_EQ(m.col(0), Vec3(1.0f, 4.0f, 7.0f));
    EXPECT_EQ(m.col(1), Vec3(2.0f, 5.0f, 8.0f));
    EXPECT_EQ(m.col(2), Vec3(3.0f, 6.0f, 9.0f));
}

TEST_F(Matrix3x3Test, BracketAccess) {
    Matrix3x3 m(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
        7.0f, 8.0f, 9.0f
    );
    
    EXPECT_EQ(m[0][0], 1.0f);
    EXPECT_EQ(m[1][1], 5.0f);
    EXPECT_EQ(m[2][2], 9.0f);
    
    m[0][0] = 100.0f;
    EXPECT_EQ(m(0, 0), 100.0f);
}

//=============================================================================
// Matrix Operations
//=============================================================================

TEST_F(Matrix3x3Test, Transpose) {
    Matrix3x3 m(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
        7.0f, 8.0f, 9.0f
    );
    
    Matrix3x3 t = m.transposed();
    
    EXPECT_EQ(t(0, 0), 1.0f);
    EXPECT_EQ(t(0, 1), 4.0f);
    EXPECT_EQ(t(0, 2), 7.0f);
    EXPECT_EQ(t(1, 0), 2.0f);
    EXPECT_EQ(t(1, 1), 5.0f);
    EXPECT_EQ(t(1, 2), 8.0f);
    EXPECT_EQ(t(2, 0), 3.0f);
    EXPECT_EQ(t(2, 1), 6.0f);
    EXPECT_EQ(t(2, 2), 9.0f);
}

TEST_F(Matrix3x3Test, TransposeInPlace) {
    Matrix3x3 m(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
        7.0f, 8.0f, 9.0f
    );
    
    m.transpose();
    
    EXPECT_EQ(m(0, 1), 4.0f);
    EXPECT_EQ(m(1, 0), 2.0f);
}

TEST_F(Matrix3x3Test, Determinant) {
    auto identity = Matrix3x3::identity();
    EXPECT_EQ(identity.determinant(), 1.0f);
    
    Matrix3x3 m(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
        7.0f, 8.0f, 9.0f
    );
    // This matrix is singular (det = 0)
    EXPECT_TRUE(approxEqual(m.determinant(), 0.0f));
    
    Matrix3x3 m2(
        1.0f, 2.0f, 3.0f,
        0.0f, 1.0f, 4.0f,
        5.0f, 6.0f, 0.0f
    );
    // det = 1*(1*0 - 4*6) - 2*(0*0 - 4*5) + 3*(0*6 - 1*5)
    //     = 1*(-24) - 2*(-20) + 3*(-5)
    //     = -24 + 40 - 15 = 1
    EXPECT_TRUE(approxEqual(m2.determinant(), 1.0f));
}

TEST_F(Matrix3x3Test, Inverse) {
    Matrix3x3 m(
        1.0f, 2.0f, 3.0f,
        0.0f, 1.0f, 4.0f,
        5.0f, 6.0f, 0.0f
    );
    
    Matrix3x3 inv = m.inverted();
    Matrix3x3 product = m * inv;
    
    EXPECT_TRUE(product.approxEqual(Matrix3x3::identity()));
}

TEST_F(Matrix3x3Test, InverseSingular) {
    Matrix3x3 singular(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
        7.0f, 8.0f, 9.0f
    );
    
    // Singular matrix returns identity
    Matrix3x3 inv = singular.inverted();
    EXPECT_EQ(inv, Matrix3x3::identity());
}

//=============================================================================
// Matrix Multiplication
//=============================================================================

TEST_F(Matrix3x3Test, MatrixMultiplication) {
    auto I = Matrix3x3::identity();
    Matrix3x3 m(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
        7.0f, 8.0f, 9.0f
    );
    
    // Identity multiplication
    EXPECT_EQ(I * m, m);
    EXPECT_EQ(m * I, m);
    
    // Actual multiplication
    Matrix3x3 a(
        1.0f, 0.0f, 0.0f,
        0.0f, 2.0f, 0.0f,
        0.0f, 0.0f, 3.0f
    );
    Matrix3x3 b(
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f
    );
    
    Matrix3x3 result = a * b;
    EXPECT_EQ(result(0, 0), 1.0f);
    EXPECT_EQ(result(1, 1), 2.0f);
    EXPECT_EQ(result(2, 2), 3.0f);
}

TEST_F(Matrix3x3Test, MatrixVectorMultiplication) {
    auto I = Matrix3x3::identity();
    Vec3 v(1.0f, 2.0f, 3.0f);
    
    // Identity multiplication
    EXPECT_EQ(I * v, v);
    
    // Scale matrix
    Matrix3x3 scale(
        2.0f, 0.0f, 0.0f,
        0.0f, 2.0f, 0.0f,
        0.0f, 0.0f, 2.0f
    );
    
    Vec3 scaled = scale * v;
    EXPECT_EQ(scaled, Vec3(2.0f, 4.0f, 6.0f));
}

TEST_F(Matrix3x3Test, ScalarMultiplication) {
    Matrix3x3 m(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
        7.0f, 8.0f, 9.0f
    );
    
    Matrix3x3 scaled = m * 2.0f;
    EXPECT_EQ(scaled(0, 0), 2.0f);
    EXPECT_EQ(scaled(1, 1), 10.0f);
    EXPECT_EQ(scaled(2, 2), 18.0f);
    
    // Scalar on left
    Matrix3x3 scaled2 = 3.0f * m;
    EXPECT_EQ(scaled2(0, 0), 3.0f);
}

//=============================================================================
// Rotation Matrix Tests
//=============================================================================

TEST_F(Matrix3x3Test, FromAxisAngle) {
    // Rotation around Z axis by 90 degrees
    Matrix3x3 rot = Matrix3x3::fromAxisAngle(constants::HALF_PI, Vec3::UNIT_Z);
    
    // X axis should become Y axis
    Vec3 x = rot * Vec3::UNIT_X;
    EXPECT_TRUE(x.approxEqual(Vec3::UNIT_Y, 0.001f));
    
    // Y axis should become -X axis
    Vec3 y = rot * Vec3::UNIT_Y;
    EXPECT_TRUE(y.approxEqual(-Vec3::UNIT_X, 0.001f));
    
    // Z axis unchanged
    Vec3 z = rot * Vec3::UNIT_Z;
    EXPECT_TRUE(z.approxEqual(Vec3::UNIT_Z, 0.001f));
}

TEST_F(Matrix3x3Test, FromEuler) {
    // Zero angles = identity-like behavior
    Matrix3x3 m = Matrix3x3::fromEuler(Vec3(0.0f, 0.0f, 0.0f));
    Vec3 forward = m * Vec3::UNIT_X;
    EXPECT_TRUE(forward.approxEqual(Vec3::UNIT_X, 0.001f));
}

//=============================================================================
// Comparison Tests
//=============================================================================

TEST_F(Matrix3x3Test, Equality) {
    Matrix3x3 a(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
        7.0f, 8.0f, 9.0f
    );
    Matrix3x3 b(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
        7.0f, 8.0f, 9.0f
    );
    Matrix3x3 c(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
        7.0f, 8.0f, 10.0f
    );
    
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
    EXPECT_TRUE(a != c);
}

TEST_F(Matrix3x3Test, ApproximateEquality) {
    Matrix3x3 a = Matrix3x3::identity();
    Matrix3x3 b(
        1.00001f, 0.00001f, 0.0f,
        0.00001f, 1.00001f, 0.0f,
        0.0f, 0.0f, 1.00001f
    );
    
    EXPECT_TRUE(a.approxEqual(b, 0.001f));
}

//=============================================================================
// Free Function Tests
//=============================================================================

TEST_F(Matrix3x3Test, ConcatRotations) {
    Matrix3x3 a = Matrix3x3::fromAxisAngle(constants::HALF_PI, Vec3::UNIT_Z);
    Matrix3x3 b = Matrix3x3::fromAxisAngle(constants::HALF_PI, Vec3::UNIT_Z);
    
    // Two 90 degree rotations = 180 degree rotation
    Matrix3x3 combined = concatRotations(a, b);
    
    Vec3 x = combined * Vec3::UNIT_X;
    EXPECT_TRUE(x.approxEqual(-Vec3::UNIT_X, 0.001f));
}
