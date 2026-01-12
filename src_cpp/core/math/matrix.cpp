/*
 * ezQuake C++ Port - 3x3 Matrix Implementation
 * 
 * Copyright (C) 1996-1997 Id Software, Inc.
 * Copyright (C) 2024 ezQuake Team
 */

#include "matrix.hpp"
#include <cmath>

namespace ezquake {
namespace math {

Matrix3x3 Matrix3x3::fromAxisAngle(VecT angle, const Vec3& axis) noexcept {
    // Rodrigues' rotation formula
    const VecT c = std::cos(angle);
    const VecT s = std::sin(angle);
    const VecT t = VecT(1) - c;
    
    const VecT x = axis.x();
    const VecT y = axis.y();
    const VecT z = axis.z();
    
    return Matrix3x3{
        t*x*x + c,      t*x*y - s*z,    t*x*z + s*y,
        t*x*y + s*z,    t*y*y + c,      t*y*z - s*x,
        t*x*z - s*y,    t*y*z + s*x,    t*z*z + c
    };
}

Matrix3x3 Matrix3x3::fromEuler(const Vec3& angles) noexcept {
    // Convert Euler angles (pitch, yaw, roll) to rotation matrix
    const VecT pitch = angles.pitch() * constants::DEG_TO_RAD;
    const VecT yaw = angles.yaw() * constants::DEG_TO_RAD;
    const VecT roll = angles.roll() * constants::DEG_TO_RAD;
    
    const VecT sp = std::sin(pitch);
    const VecT cp = std::cos(pitch);
    const VecT sy = std::sin(yaw);
    const VecT cy = std::cos(yaw);
    const VecT sr = std::sin(roll);
    const VecT cr = std::cos(roll);
    
    return Matrix3x3{
        cy*cp,                      sy*cp,                      -sp,
        cy*sp*sr - sy*cr,           sy*sp*sr + cy*cr,           cp*sr,
        cy*sp*cr + sy*sr,           sy*sp*cr - cy*sr,           cp*cr
    };
}

Matrix3x3 Matrix3x3::inverted() const noexcept {
    const VecT det = determinant();
    
    if (std::abs(det) < constants::EPSILON) {
        // Singular matrix, return identity
        return identity();
    }
    
    const VecT invDet = VecT(1) / det;
    
    return Matrix3x3{
        (data_[1][1] * data_[2][2] - data_[1][2] * data_[2][1]) * invDet,
        (data_[0][2] * data_[2][1] - data_[0][1] * data_[2][2]) * invDet,
        (data_[0][1] * data_[1][2] - data_[0][2] * data_[1][1]) * invDet,
        
        (data_[1][2] * data_[2][0] - data_[1][0] * data_[2][2]) * invDet,
        (data_[0][0] * data_[2][2] - data_[0][2] * data_[2][0]) * invDet,
        (data_[0][2] * data_[1][0] - data_[0][0] * data_[1][2]) * invDet,
        
        (data_[1][0] * data_[2][1] - data_[1][1] * data_[2][0]) * invDet,
        (data_[0][1] * data_[2][0] - data_[0][0] * data_[2][1]) * invDet,
        (data_[0][0] * data_[1][1] - data_[0][1] * data_[1][0]) * invDet
    };
}

} // namespace math
} // namespace ezquake
