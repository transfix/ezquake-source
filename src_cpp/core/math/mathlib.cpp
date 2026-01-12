/*
 * ezQuake C++ Port - Math Library Implementation
 * 
 * Copyright (C) 1996-1997 Id Software, Inc.
 * Copyright (C) 2024 ezQuake Team
 */

#include "mathlib.hpp"
#include <cmath>
#include <random>

namespace ezquake {
namespace math {

//=============================================================================
// Box-Plane Tests
//=============================================================================

Int32 boxOnPlaneSide(const Vec3& mins, const Vec3& maxs, const Plane& plane) noexcept {
    // Fast path for axial planes
    if (plane.type < 3) {
        if (plane.dist <= mins[plane.type]) return 1;
        if (plane.dist >= maxs[plane.type]) return 2;
        return 3;
    }
    
    // General case using sign bits
    Vec3 corners[2] = {maxs, mins};
    
    // Select corners based on plane normal signs
    VecT dist1 = plane.normal.x() * corners[(plane.signbits >> 0) & 1].x()
               + plane.normal.y() * corners[(plane.signbits >> 1) & 1].y()
               + plane.normal.z() * corners[(plane.signbits >> 2) & 1].z();
               
    VecT dist2 = plane.normal.x() * corners[1 - ((plane.signbits >> 0) & 1)].x()
               + plane.normal.y() * corners[1 - ((plane.signbits >> 1) & 1)].y()
               + plane.normal.z() * corners[1 - ((plane.signbits >> 2) & 1)].z();
    
    Int32 sides = 0;
    if (dist1 >= plane.dist) sides = 1;
    if (dist2 < plane.dist) sides |= 2;
    
    return sides;
}

//=============================================================================
// Vector-Plane Operations
//=============================================================================

Vec3 projectPointOnPlane(const Vec3& point, const Vec3& planeNormal) noexcept {
    VecT invDenom = VecT(1) / planeNormal.dot(planeNormal);
    VecT d = planeNormal.dot(point) * invDenom;
    
    Vec3 n = planeNormal * invDenom;
    return point - n * d;
}

Vec3 perpendicularVector(const Vec3& src) noexcept {
    if (src.x() == 0.0f) {
        return Vec3(1.0f, 0.0f, 0.0f);
    } else if (src.y() == 0.0f) {
        return Vec3(0.0f, 1.0f, 0.0f);
    } else if (src.z() == 0.0f) {
        return Vec3(0.0f, 0.0f, 1.0f);
    } else {
        Vec3 result(-src.y(), src.x(), 0.0f);
        result.normalizeFast();
        return result;
    }
}

Vec3 rotatePointAroundVector(const Vec3& dir, const Vec3& point, VecT degrees) noexcept {
    VecT angle = degrees * constants::DEG_TO_RAD;
    VecT c = std::cos(angle);
    VecT s = std::sin(angle);
    
    Vec3 vf = dir;
    Vec3 vr, vu;
    vectorVectors(vf, vr, vu);
    
    VecT t0, t1;
    Vec3 result;
    
    t0 = vr.x() * c + vu.x() * -s;
    t1 = vr.x() * s + vu.x() * c;
    result.x() = (t0 * vr.x() + t1 * vu.x() + vf.x() * vf.x()) * point.x()
               + (t0 * vr.y() + t1 * vu.y() + vf.x() * vf.y()) * point.y()
               + (t0 * vr.z() + t1 * vu.z() + vf.x() * vf.z()) * point.z();
    
    t0 = vr.y() * c + vu.y() * -s;
    t1 = vr.y() * s + vu.y() * c;
    result.y() = (t0 * vr.x() + t1 * vu.x() + vf.y() * vf.x()) * point.x()
               + (t0 * vr.y() + t1 * vu.y() + vf.y() * vf.y()) * point.y()
               + (t0 * vr.z() + t1 * vu.z() + vf.y() * vf.z()) * point.z();
    
    t0 = vr.z() * c + vu.z() * -s;
    t1 = vr.z() * s + vu.z() * c;
    result.z() = (t0 * vr.x() + t1 * vu.x() + vf.z() * vf.x()) * point.x()
               + (t0 * vr.y() + t1 * vu.y() + vf.z() * vf.y()) * point.y()
               + (t0 * vr.z() + t1 * vu.z() + vf.z() * vf.z()) * point.z();
    
    return result;
}

void makeNormalVectors(const Vec3& forward, Vec3& right, Vec3& up) noexcept {
    // This rotate and negate guarantees a vector not colinear with the original
    right.y() = -forward.x();
    right.z() = forward.y();
    right.x() = forward.z();
    
    VecT d = right.dot(forward);
    right = right - forward * d;
    right.normalize();
    
    up = right.cross(forward);
}

void vectorVectors(const Vec3& forward, Vec3& right, Vec3& up) noexcept {
    right = perpendicularVector(forward);
    up = right.cross(forward);
}

//=============================================================================
// Number Theory
//=============================================================================

void floorDivMod(Double numer, Double denom, Int32& quotient, Int32& remainder) noexcept {
    if (numer >= 0.0) {
        quotient = static_cast<Int32>(numer / denom);
        remainder = static_cast<Int32>(std::fmod(numer, denom));
    } else {
        Double x = std::floor(numer / denom);
        quotient = static_cast<Int32>(x);
        remainder = static_cast<Int32>(numer - x * denom);
    }
}

Int32 gcd(Int32 a, Int32 b) noexcept {
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    
    while (b != 0) {
        Int32 t = b;
        b = a % b;
        a = t;
    }
    return a;
}

Fixed16 invert24To16(Fixed16 val) noexcept {
    if (val < 256) return static_cast<Fixed16>(-1); // Max value for fixed16
    return static_cast<Fixed16>(
        (static_cast<Int64>(1) << 40) / static_cast<Int64>(val)
    );
}

//=============================================================================
// Polygon Utilities
//=============================================================================

Int32 getPolyCentroid(
    const Vec3* vertices,
    Int32 numVertices,
    VecT& centroidX,
    VecT& centroidY,
    VecT& area
) noexcept {
    if (numVertices < 3) return -1;
    
    VecT cx = 0.0f;
    VecT cy = 0.0f;
    VecT a = 0.0f;
    
    for (Int32 i = 0; i < numVertices; ++i) {
        Int32 j = (i + 1) % numVertices;
        VecT cross = vertices[i].x() * vertices[j].y() - vertices[j].x() * vertices[i].y();
        a += cross;
        cx += (vertices[i].x() + vertices[j].x()) * cross;
        cy += (vertices[i].y() + vertices[j].y()) * cross;
    }
    
    a *= 0.5f;
    if (std::abs(a) < constants::EPSILON) return -1;
    
    VecT factor = 1.0f / (6.0f * a);
    centroidX = cx * factor;
    centroidY = cy * factor;
    area = std::abs(a);
    
    return 0;
}

//=============================================================================
// Random Number Generation
//=============================================================================

namespace {
    thread_local std::mt19937 g_rng{std::random_device{}()};
    thread_local std::uniform_real_distribution<VecT> g_dist01{0.0f, 1.0f};
}

VecT randomFloat() noexcept {
    return g_dist01(g_rng);
}

VecT randomFloat(VecT min, VecT max) noexcept {
    return min + (max - min) * randomFloat();
}

Int32 randomInt(Int32 max) noexcept {
    if (max <= 0) return 0;
    std::uniform_int_distribution<Int32> dist{0, max - 1};
    return dist(g_rng);
}

Vec3 randomUnitVector() noexcept {
    // Use spherical coordinates for uniform distribution
    VecT z = randomFloat(-1.0f, 1.0f);
    VecT a = randomFloat(0.0f, constants::TAU);
    VecT r = std::sqrt(1.0f - z * z);
    return Vec3(r * std::cos(a), r * std::sin(a), z);
}

Vec3 randomInUnitSphere() noexcept {
    // Rejection sampling
    Vec3 p;
    do {
        p = Vec3(
            randomFloat(-1.0f, 1.0f),
            randomFloat(-1.0f, 1.0f),
            randomFloat(-1.0f, 1.0f)
        );
    } while (p.lengthSquared() >= 1.0f);
    return p;
}

} // namespace math
} // namespace ezquake
