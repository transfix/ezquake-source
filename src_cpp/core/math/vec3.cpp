/*
 * ezQuake C++ Port - 3D Vector Implementation
 * 
 * Copyright (C) 1996-1997 Id Software, Inc.
 * Copyright (C) 2024 ezQuake Team
 */

#include "vec3.hpp"
#include <cmath>

namespace ezquake {
namespace math {

//=============================================================================
// Static Constants Definition
//=============================================================================

const Vec3 Vec3::ORIGIN{0.0f, 0.0f, 0.0f};
const Vec3 Vec3::UNIT_X{1.0f, 0.0f, 0.0f};
const Vec3 Vec3::UNIT_Y{0.0f, 1.0f, 0.0f};
const Vec3 Vec3::UNIT_Z{0.0f, 0.0f, 1.0f};
const Vec3 Vec3::UP{0.0f, 0.0f, 1.0f};
const Vec3 Vec3::DOWN{0.0f, 0.0f, -1.0f};
const Vec3 Vec3::FORWARD{1.0f, 0.0f, 0.0f};
const Vec3 Vec3::BACK{-1.0f, 0.0f, 0.0f};
const Vec3 Vec3::LEFT{0.0f, 1.0f, 0.0f};
const Vec3 Vec3::RIGHT{0.0f, -1.0f, 0.0f};

//=============================================================================
// Implementation
//=============================================================================

void Vec3::normalizeFast() noexcept {
    // Fast inverse square root (Quake's famous algorithm)
    VecT lengthSq = lengthSquared();
    if (lengthSq > 0) {
        // Initial approximation using bit manipulation
        union {
            float f;
            std::int32_t i;
        } conv;
        
        conv.f = lengthSq;
        conv.i = 0x5f375a86 - (conv.i >> 1); // Magic number for inverse sqrt
        
        // One iteration of Newton-Raphson refinement
        VecT invLen = conv.f;
        invLen = invLen * (1.5f - (0.5f * lengthSq * invLen * invLen));
        
        data_[0] *= invLen;
        data_[1] *= invLen;
        data_[2] *= invLen;
    }
}

Vec3 Vec3::angleLerp(const Vec3& target, VecT t) const noexcept {
    Vec3 delta = target - *this;
    
    // Normalize angle differences to [-180, 180] range
    auto normalizeAngle = [](VecT angle) -> VecT {
        while (angle > 180.0f) angle -= 360.0f;
        while (angle < -180.0f) angle += 360.0f;
        return angle;
    };
    
    return Vec3(
        data_[0] + t * normalizeAngle(delta.data_[0]),
        data_[1] + t * normalizeAngle(delta.data_[1]),
        data_[2] + t * normalizeAngle(delta.data_[2])
    );
}

void Vec3::toDirections(Vec3& forward, Vec3& right, Vec3& up) const noexcept {
    // Convert Euler angles (pitch, yaw, roll) to direction vectors
    // Matches the original AngleVectors function
    
    const VecT pitch = data_[0] * constants::DEG_TO_RAD;
    const VecT yaw = data_[1] * constants::DEG_TO_RAD;
    const VecT roll = data_[2] * constants::DEG_TO_RAD;
    
    const VecT sp = std::sin(pitch);
    const VecT cp = std::cos(pitch);
    const VecT sy = std::sin(yaw);
    const VecT cy = std::cos(yaw);
    const VecT sr = std::sin(roll);
    const VecT cr = std::cos(roll);
    
    forward.set(cp * cy, cp * sy, -sp);
    right.set(
        -sr * sp * cy + cr * sy,
        -sr * sp * sy - cr * cy,
        -sr * cp
    );
    up.set(
        cr * sp * cy + sr * sy,
        cr * sp * sy - sr * cy,
        cr * cp
    );
}

Vec3 Vec3::toForward() const noexcept {
    const VecT pitch = data_[0] * constants::DEG_TO_RAD;
    const VecT yaw = data_[1] * constants::DEG_TO_RAD;
    
    const VecT cp = std::cos(pitch);
    
    return Vec3(
        cp * std::cos(yaw),
        cp * std::sin(yaw),
        -std::sin(pitch)
    );
}

//=============================================================================
// Free Functions
//=============================================================================

void angleVectors(const Vec3& angles, Vec3& forward, Vec3& right, Vec3& up) noexcept {
    angles.toDirections(forward, right, up);
}

} // namespace math
} // namespace ezquake
