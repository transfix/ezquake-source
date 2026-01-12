/*
 * ezQuake C++ Port - Math Library Functions
 * 
 * Additional math utilities and free functions that don't belong
 * to a specific class. Compatible with original mathlib.c functions.
 * 
 * Copyright (C) 1996-1997 Id Software, Inc.
 * Copyright (C) 2024 ezQuake Team
 */

#ifndef EZQUAKE_CORE_MATH_MATHLIB_HPP
#define EZQUAKE_CORE_MATH_MATHLIB_HPP

#include "../types.hpp"
#include "vec3.hpp"
#include "matrix.hpp"

namespace ezquake {
namespace math {

//=============================================================================
// Plane Structure
//=============================================================================

/**
 * @brief Represents a plane in 3D space
 * 
 * Compatible with original mplane_t structure.
 */
struct Plane {
    Vec3 normal;
    VecT dist = 0.0f;      // Distance from origin along normal
    Byte type = 0;         // 0-2 are axial planes (x/y/z), 3+ are non-axial
    Byte signbits = 0;     // signx + signy*2 + signz*4 for fast box on plane side
    Byte pad[2] = {};      // Padding for alignment
    
    /// Default constructor
    constexpr Plane() noexcept = default;
    
    /// Construct from normal and distance
    constexpr Plane(const Vec3& n, VecT d) noexcept 
        : normal(n), dist(d) {
        updateType();
    }
    
    /// Construct from normal and point on plane
    Plane(const Vec3& n, const Vec3& point) noexcept 
        : normal(n.normalized()), dist(n.dot(point)) {
        updateType();
    }
    
    /// Construct from three points
    static Plane fromPoints(const Vec3& p1, const Vec3& p2, const Vec3& p3) noexcept {
        Vec3 v1 = p2 - p1;
        Vec3 v2 = p3 - p1;
        Vec3 n = v1.cross(v2).normalized();
        return Plane(n, p1);
    }
    
    /// Update type and signbits based on normal
    constexpr void updateType() noexcept {
        // Determine plane type
        if (normal.x() == 1.0f || normal.x() == -1.0f) {
            type = 0; // YZ plane (perpendicular to X)
        } else if (normal.y() == 1.0f || normal.y() == -1.0f) {
            type = 1; // XZ plane (perpendicular to Y)
        } else if (normal.z() == 1.0f || normal.z() == -1.0f) {
            type = 2; // XY plane (perpendicular to Z)
        } else {
            type = 3; // Non-axial
        }
        
        // Calculate sign bits
        signbits = 0;
        if (normal.x() < 0) signbits |= 1;
        if (normal.y() < 0) signbits |= 2;
        if (normal.z() < 0) signbits |= 4;
    }
    
    /// Distance from point to plane (positive = front, negative = back)
    [[nodiscard]] constexpr VecT distanceToPoint(const Vec3& point) const noexcept {
        if (type < 3) {
            return point[type] - dist;
        }
        return normal.dot(point) - dist;
    }
    
    /// Check which side of plane a point is on
    /// Returns: 1 = front, 2 = back, 3 = on plane
    [[nodiscard]] Int32 pointSide(const Vec3& point, VecT epsilon = constants::ON_EPSILON) const noexcept {
        VecT d = distanceToPoint(point);
        if (d > epsilon) return 1;  // Front
        if (d < -epsilon) return 2; // Back
        return 3;                   // On plane
    }
};

//=============================================================================
// Box-Plane Tests
//=============================================================================

/**
 * @brief Test box against plane (BoxOnPlaneSide replacement)
 * 
 * @param mins Box minimum corner
 * @param maxs Box maximum corner
 * @param plane The plane to test against
 * @return 1 = entirely in front, 2 = entirely behind, 3 = crossing
 */
Int32 boxOnPlaneSide(const Vec3& mins, const Vec3& maxs, const Plane& plane) noexcept;

//=============================================================================
// Vector-Plane Operations
//=============================================================================

/**
 * @brief Project point onto plane
 */
Vec3 projectPointOnPlane(const Vec3& point, const Vec3& planeNormal) noexcept;

/**
 * @brief Get perpendicular vector to given vector
 */
Vec3 perpendicularVector(const Vec3& src) noexcept;

/**
 * @brief Rotate point around arbitrary axis
 */
Vec3 rotatePointAroundVector(const Vec3& dir, const Vec3& point, VecT degrees) noexcept;

/**
 * @brief Create orthonormal basis from forward vector
 */
void makeNormalVectors(const Vec3& forward, Vec3& right, Vec3& up) noexcept;

/**
 * @brief Create orthonormal basis (VectorVectors replacement)
 */
void vectorVectors(const Vec3& forward, Vec3& right, Vec3& up) noexcept;

//=============================================================================
// Angle Utilities
//=============================================================================

/**
 * @brief Normalize angle to [0, 360) range
 */
[[nodiscard]] inline constexpr VecT angleMod(VecT angle) noexcept {
    return (360.0f / 65536.0f) * (static_cast<Int32>(angle * (65536.0f / 360.0f)) & 65535);
}

/**
 * @brief Normalize angle to [-180, 180) range
 */
[[nodiscard]] inline constexpr VecT angleNormalize180(VecT angle) noexcept {
    angle = angleMod(angle);
    if (angle > 180.0f) angle -= 360.0f;
    return angle;
}

/**
 * @brief Calculate shortest angle difference
 */
[[nodiscard]] inline constexpr VecT angleDiff(VecT from, VecT to) noexcept {
    return angleNormalize180(to - from);
}

//=============================================================================
// Number Theory
//=============================================================================

/**
 * @brief Floor division with remainder (for texture coordinates etc.)
 */
void floorDivMod(Double numer, Double denom, Int32& quotient, Int32& remainder) noexcept;

/**
 * @brief Greatest common divisor
 */
[[nodiscard]] Int32 gcd(Int32 a, Int32 b) noexcept;

/**
 * @brief Invert 24-bit fixed point to 16-bit fixed point
 */
[[nodiscard]] Fixed16 invert24To16(Fixed16 val) noexcept;

//=============================================================================
// Polygon Utilities
//=============================================================================

/**
 * @brief Calculate centroid of a polygon
 * 
 * @param vertices Array of vertices
 * @param numVertices Number of vertices
 * @param centroidX Output X coordinate of centroid
 * @param centroidY Output Y coordinate of centroid
 * @param area Output area of polygon
 * @return 0 on success, non-zero on error
 */
Int32 getPolyCentroid(
    const Vec3* vertices, 
    Int32 numVertices,
    VecT& centroidX,
    VecT& centroidY,
    VecT& area
) noexcept;

//=============================================================================
// Random Number Generation
//=============================================================================

/**
 * @brief Get random float in [0, 1) range
 */
[[nodiscard]] VecT randomFloat() noexcept;

/**
 * @brief Get random float in [min, max) range
 */
[[nodiscard]] VecT randomFloat(VecT min, VecT max) noexcept;

/**
 * @brief Get random integer in [0, max) range
 */
[[nodiscard]] Int32 randomInt(Int32 max) noexcept;

/**
 * @brief Get random unit vector
 */
[[nodiscard]] Vec3 randomUnitVector() noexcept;

/**
 * @brief Get random vector in unit sphere
 */
[[nodiscard]] Vec3 randomInUnitSphere() noexcept;

//=============================================================================
// Utility Functions
//=============================================================================

/**
 * @brief Round up to next power of 2
 */
[[nodiscard]] inline constexpr UInt32 nextPowerOf2(UInt32 v) noexcept {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

/**
 * @brief Check if value is power of 2
 */
[[nodiscard]] inline constexpr bool isPowerOf2(UInt32 v) noexcept {
    return v && !(v & (v - 1));
}

/**
 * @brief Sign of a value (-1, 0, or 1)
 */
template<typename T>
[[nodiscard]] inline constexpr Int32 sign(T val) noexcept {
    return (T(0) < val) - (val < T(0));
}

/**
 * @brief Clamp value to range
 */
template<typename T>
[[nodiscard]] inline constexpr T clamp(T val, T min, T max) noexcept {
    return val < min ? min : (val > max ? max : val);
}

/**
 * @brief Linear interpolation
 */
template<typename T>
[[nodiscard]] inline constexpr T lerp(T a, T b, VecT t) noexcept {
    return a + (b - a) * t;
}

/**
 * @brief Smooth step interpolation
 */
[[nodiscard]] inline constexpr VecT smoothStep(VecT edge0, VecT edge1, VecT x) noexcept {
    VecT t = clamp((x - edge0) / (edge1 - edge0), VecT(0), VecT(1));
    return t * t * (VecT(3) - VecT(2) * t);
}

} // namespace math
} // namespace ezquake

#endif // EZQUAKE_CORE_MATH_MATHLIB_HPP
