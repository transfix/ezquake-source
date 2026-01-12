/*
 * ezQuake C++ Port - 3D Vector Class
 * 
 * Modern C++17 implementation of the Quake vec3_t type.
 * Provides a type-safe, RAII-compliant vector class with full
 * compatibility with the original C code.
 * 
 * Original: typedef vec_t vec3_t[3];
 * 
 * Copyright (C) 1996-1997 Id Software, Inc.
 * Copyright (C) 2024 ezQuake Team
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef EZQUAKE_CORE_MATH_VEC3_HPP
#define EZQUAKE_CORE_MATH_VEC3_HPP

#include "../types.hpp"
#include <cmath>
#include <algorithm>
#include <stdexcept>

namespace ezquake {
namespace math {

/// Axis indices (compatible with PITCH, YAW, ROLL defines)
enum class Axis : Int32 {
    X = 0,      // PITCH - up/down
    Y = 1,      // YAW - left/right  
    Z = 2,      // ROLL - fall over
    Pitch = 0,
    Yaw = 1,
    Roll = 2
};

/**
 * @brief 3D Vector class - core math primitive for Quake engine
 * 
 * This class replaces the C-style vec3_t (float[3]) with a proper
 * C++ class that provides:
 * - Type safety
 * - Operator overloading
 * - Constexpr operations where possible
 * - SIMD-friendly data layout
 * - Full compatibility with C vec3_t through data() method
 */
class Vec3 {
public:
    //=========================================================================
    // Type aliases
    //=========================================================================
    using value_type = VecT;
    using size_type = Size;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = pointer;
    using const_iterator = const_pointer;

    //=========================================================================
    // Static constants
    //=========================================================================
    static constexpr size_type SIZE = 3;
    
    /// Origin vector (0, 0, 0)
    static const Vec3 ORIGIN;
    
    /// Unit vectors
    static const Vec3 UNIT_X;
    static const Vec3 UNIT_Y;
    static const Vec3 UNIT_Z;
    static const Vec3 UP;
    static const Vec3 DOWN;
    static const Vec3 FORWARD;
    static const Vec3 BACK;
    static const Vec3 LEFT;
    static const Vec3 RIGHT;

    //=========================================================================
    // Constructors
    //=========================================================================
    
    /// Default constructor - zero-initializes
    constexpr Vec3() noexcept : data_{0.0f, 0.0f, 0.0f} {}
    
    /// Construct from components
    constexpr Vec3(VecT x, VecT y, VecT z) noexcept : data_{x, y, z} {}
    
    /// Construct from single value (all components same)
    explicit constexpr Vec3(VecT scalar) noexcept : data_{scalar, scalar, scalar} {}
    
    /// Construct from C-style array (vec3_t compatibility)
    explicit constexpr Vec3(const VecT* arr) noexcept 
        : data_{arr[0], arr[1], arr[2]} {}
    
    /// Construct from std::array
    explicit constexpr Vec3(const Array<VecT, 3>& arr) noexcept
        : data_{arr[0], arr[1], arr[2]} {}
    
    /// Copy constructor
    constexpr Vec3(const Vec3&) noexcept = default;
    
    /// Move constructor
    constexpr Vec3(Vec3&&) noexcept = default;
    
    /// Copy assignment
    constexpr Vec3& operator=(const Vec3&) noexcept = default;
    
    /// Move assignment
    constexpr Vec3& operator=(Vec3&&) noexcept = default;
    
    /// Destructor
    ~Vec3() = default;

    //=========================================================================
    // Element Access
    //=========================================================================
    
    /// Access by index (no bounds checking)
    [[nodiscard]] constexpr reference operator[](size_type i) noexcept {
        return data_[i];
    }
    
    [[nodiscard]] constexpr const_reference operator[](size_type i) const noexcept {
        return data_[i];
    }
    
    /// Access by axis enum
    [[nodiscard]] constexpr reference operator[](Axis axis) noexcept {
        return data_[static_cast<size_type>(axis)];
    }
    
    [[nodiscard]] constexpr const_reference operator[](Axis axis) const noexcept {
        return data_[static_cast<size_type>(axis)];
    }
    
    /// Safe access with bounds checking
    [[nodiscard]] reference at(size_type i) {
        if (i >= SIZE) throw std::out_of_range("Vec3 index out of range");
        return data_[i];
    }
    
    [[nodiscard]] const_reference at(size_type i) const {
        if (i >= SIZE) throw std::out_of_range("Vec3 index out of range");
        return data_[i];
    }
    
    /// Named component accessors
    [[nodiscard]] constexpr reference x() noexcept { return data_[0]; }
    [[nodiscard]] constexpr const_reference x() const noexcept { return data_[0]; }
    
    [[nodiscard]] constexpr reference y() noexcept { return data_[1]; }
    [[nodiscard]] constexpr const_reference y() const noexcept { return data_[1]; }
    
    [[nodiscard]] constexpr reference z() noexcept { return data_[2]; }
    [[nodiscard]] constexpr const_reference z() const noexcept { return data_[2]; }
    
    /// Angle accessors (pitch, yaw, roll)
    [[nodiscard]] constexpr reference pitch() noexcept { return data_[0]; }
    [[nodiscard]] constexpr const_reference pitch() const noexcept { return data_[0]; }
    
    [[nodiscard]] constexpr reference yaw() noexcept { return data_[1]; }
    [[nodiscard]] constexpr const_reference yaw() const noexcept { return data_[1]; }
    
    [[nodiscard]] constexpr reference roll() noexcept { return data_[2]; }
    [[nodiscard]] constexpr const_reference roll() const noexcept { return data_[2]; }
    
    /// Raw data access (for C compatibility)
    [[nodiscard]] constexpr pointer data() noexcept { return data_; }
    [[nodiscard]] constexpr const_pointer data() const noexcept { return data_; }
    
    /// Size (always 3)
    [[nodiscard]] static constexpr size_type size() noexcept { return SIZE; }

    //=========================================================================
    // Iterators
    //=========================================================================
    
    [[nodiscard]] constexpr iterator begin() noexcept { return data_; }
    [[nodiscard]] constexpr const_iterator begin() const noexcept { return data_; }
    [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return data_; }
    
    [[nodiscard]] constexpr iterator end() noexcept { return data_ + SIZE; }
    [[nodiscard]] constexpr const_iterator end() const noexcept { return data_ + SIZE; }
    [[nodiscard]] constexpr const_iterator cend() const noexcept { return data_ + SIZE; }

    //=========================================================================
    // Arithmetic Operators
    //=========================================================================
    
    /// Unary negation
    [[nodiscard]] constexpr Vec3 operator-() const noexcept {
        return Vec3(-data_[0], -data_[1], -data_[2]);
    }
    
    /// Unary plus (identity)
    [[nodiscard]] constexpr Vec3 operator+() const noexcept {
        return *this;
    }
    
    /// Vector addition
    [[nodiscard]] constexpr Vec3 operator+(const Vec3& rhs) const noexcept {
        return Vec3(data_[0] + rhs.data_[0], 
                   data_[1] + rhs.data_[1], 
                   data_[2] + rhs.data_[2]);
    }
    
    /// Vector subtraction
    [[nodiscard]] constexpr Vec3 operator-(const Vec3& rhs) const noexcept {
        return Vec3(data_[0] - rhs.data_[0], 
                   data_[1] - rhs.data_[1], 
                   data_[2] - rhs.data_[2]);
    }
    
    /// Component-wise multiplication
    [[nodiscard]] constexpr Vec3 operator*(const Vec3& rhs) const noexcept {
        return Vec3(data_[0] * rhs.data_[0], 
                   data_[1] * rhs.data_[1], 
                   data_[2] * rhs.data_[2]);
    }
    
    /// Component-wise division
    [[nodiscard]] constexpr Vec3 operator/(const Vec3& rhs) const noexcept {
        return Vec3(data_[0] / rhs.data_[0], 
                   data_[1] / rhs.data_[1], 
                   data_[2] / rhs.data_[2]);
    }
    
    /// Scalar multiplication
    [[nodiscard]] constexpr Vec3 operator*(VecT scalar) const noexcept {
        return Vec3(data_[0] * scalar, data_[1] * scalar, data_[2] * scalar);
    }
    
    /// Scalar division
    [[nodiscard]] constexpr Vec3 operator/(VecT scalar) const noexcept {
        const VecT inv = VecT(1) / scalar;
        return Vec3(data_[0] * inv, data_[1] * inv, data_[2] * inv);
    }
    
    /// Compound assignment operators
    constexpr Vec3& operator+=(const Vec3& rhs) noexcept {
        data_[0] += rhs.data_[0];
        data_[1] += rhs.data_[1];
        data_[2] += rhs.data_[2];
        return *this;
    }
    
    constexpr Vec3& operator-=(const Vec3& rhs) noexcept {
        data_[0] -= rhs.data_[0];
        data_[1] -= rhs.data_[1];
        data_[2] -= rhs.data_[2];
        return *this;
    }
    
    constexpr Vec3& operator*=(const Vec3& rhs) noexcept {
        data_[0] *= rhs.data_[0];
        data_[1] *= rhs.data_[1];
        data_[2] *= rhs.data_[2];
        return *this;
    }
    
    constexpr Vec3& operator/=(const Vec3& rhs) noexcept {
        data_[0] /= rhs.data_[0];
        data_[1] /= rhs.data_[1];
        data_[2] /= rhs.data_[2];
        return *this;
    }
    
    constexpr Vec3& operator*=(VecT scalar) noexcept {
        data_[0] *= scalar;
        data_[1] *= scalar;
        data_[2] *= scalar;
        return *this;
    }
    
    constexpr Vec3& operator/=(VecT scalar) noexcept {
        const VecT inv = VecT(1) / scalar;
        data_[0] *= inv;
        data_[1] *= inv;
        data_[2] *= inv;
        return *this;
    }

    //=========================================================================
    // Comparison Operators
    //=========================================================================
    
    [[nodiscard]] constexpr bool operator==(const Vec3& rhs) const noexcept {
        return data_[0] == rhs.data_[0] && 
               data_[1] == rhs.data_[1] && 
               data_[2] == rhs.data_[2];
    }
    
    [[nodiscard]] constexpr bool operator!=(const Vec3& rhs) const noexcept {
        return !(*this == rhs);
    }

    //=========================================================================
    // Vector Operations
    //=========================================================================
    
    /// Dot product
    [[nodiscard]] constexpr VecT dot(const Vec3& rhs) const noexcept {
        return data_[0] * rhs.data_[0] + 
               data_[1] * rhs.data_[1] + 
               data_[2] * rhs.data_[2];
    }
    
    /// Cross product
    [[nodiscard]] constexpr Vec3 cross(const Vec3& rhs) const noexcept {
        return Vec3(
            data_[1] * rhs.data_[2] - data_[2] * rhs.data_[1],
            data_[2] * rhs.data_[0] - data_[0] * rhs.data_[2],
            data_[0] * rhs.data_[1] - data_[1] * rhs.data_[0]
        );
    }
    
    /// Squared length (faster than length())
    [[nodiscard]] constexpr VecT lengthSquared() const noexcept {
        return dot(*this);
    }
    
    /// Vector length/magnitude
    [[nodiscard]] VecT length() const noexcept {
        return std::sqrt(lengthSquared());
    }
    
    /// Normalize in place, returns old length
    VecT normalize() noexcept {
        const VecT len = length();
        if (len > 0) {
            const VecT inv = VecT(1) / len;
            data_[0] *= inv;
            data_[1] *= inv;
            data_[2] *= inv;
        }
        return len;
    }
    
    /// Return normalized copy
    [[nodiscard]] Vec3 normalized() const noexcept {
        Vec3 result = *this;
        result.normalize();
        return result;
    }
    
    /// Fast normalize using inverse square root approximation
    void normalizeFast() noexcept;
    
    /// Distance to another vector
    [[nodiscard]] VecT distanceTo(const Vec3& other) const noexcept {
        return (*this - other).length();
    }
    
    /// Squared distance to another vector (faster)
    [[nodiscard]] constexpr VecT distanceSquaredTo(const Vec3& other) const noexcept {
        return (*this - other).lengthSquared();
    }
    
    /// Linear interpolation
    [[nodiscard]] constexpr Vec3 lerp(const Vec3& target, VecT t) const noexcept {
        return *this + (target - *this) * t;
    }
    
    /// Angle interpolation (handles wraparound)
    [[nodiscard]] Vec3 angleLerp(const Vec3& target, VecT t) const noexcept;
    
    /// Project this vector onto another
    [[nodiscard]] Vec3 projectOnto(const Vec3& onto) const noexcept {
        const VecT denom = onto.lengthSquared();
        if (denom < constants::EPSILON) return Vec3::ORIGIN;
        return onto * (dot(onto) / denom);
    }
    
    /// Reflect this vector around a normal
    [[nodiscard]] constexpr Vec3 reflect(const Vec3& normal) const noexcept {
        return *this - normal * (VecT(2) * dot(normal));
    }

    //=========================================================================
    // Utility Operations
    //=========================================================================
    
    /// Set all components to zero
    constexpr void clear() noexcept {
        data_[0] = data_[1] = data_[2] = VecT(0);
    }
    
    /// Set all components
    constexpr void set(VecT x, VecT y, VecT z) noexcept {
        data_[0] = x;
        data_[1] = y;
        data_[2] = z;
    }
    
    /// Copy from C-style array
    constexpr void copyFrom(const VecT* src) noexcept {
        data_[0] = src[0];
        data_[1] = src[1];
        data_[2] = src[2];
    }
    
    /// Copy to C-style array
    constexpr void copyTo(VecT* dst) const noexcept {
        dst[0] = data_[0];
        dst[1] = data_[1];
        dst[2] = data_[2];
    }
    
    /// Check if approximately equal within margin
    [[nodiscard]] bool approxEqual(const Vec3& other, VecT margin = constants::EPSILON) const noexcept {
        return std::abs(data_[0] - other.data_[0]) < margin &&
               std::abs(data_[1] - other.data_[1]) < margin &&
               std::abs(data_[2] - other.data_[2]) < margin;
    }
    
    /// Check if vector is approximately zero
    [[nodiscard]] bool isZero(VecT margin = constants::EPSILON) const noexcept {
        return approxEqual(ORIGIN, margin);
    }
    
    /// Check if any component is NaN
    [[nodiscard]] bool hasNaN() const noexcept {
        return std::isnan(data_[0]) || std::isnan(data_[1]) || std::isnan(data_[2]);
    }
    
    /// Check if any component is infinite
    [[nodiscard]] bool hasInf() const noexcept {
        return std::isinf(data_[0]) || std::isinf(data_[1]) || std::isinf(data_[2]);
    }
    
    /// Check if vector is valid (no NaN or Inf)
    [[nodiscard]] bool isValid() const noexcept {
        return !hasNaN() && !hasInf();
    }
    
    /// Clamp all components to range
    constexpr void clamp(VecT minVal, VecT maxVal) noexcept {
        data_[0] = std::clamp(data_[0], minVal, maxVal);
        data_[1] = std::clamp(data_[1], minVal, maxVal);
        data_[2] = std::clamp(data_[2], minVal, maxVal);
    }
    
    /// Return clamped copy
    [[nodiscard]] constexpr Vec3 clamped(VecT minVal, VecT maxVal) const noexcept {
        return Vec3(
            std::clamp(data_[0], minVal, maxVal),
            std::clamp(data_[1], minVal, maxVal),
            std::clamp(data_[2], minVal, maxVal)
        );
    }
    
    /// Component-wise minimum
    [[nodiscard]] constexpr Vec3 min(const Vec3& other) const noexcept {
        return Vec3(
            std::min(data_[0], other.data_[0]),
            std::min(data_[1], other.data_[1]),
            std::min(data_[2], other.data_[2])
        );
    }
    
    /// Component-wise maximum
    [[nodiscard]] constexpr Vec3 max(const Vec3& other) const noexcept {
        return Vec3(
            std::max(data_[0], other.data_[0]),
            std::max(data_[1], other.data_[1]),
            std::max(data_[2], other.data_[2])
        );
    }
    
    /// Get absolute values
    [[nodiscard]] Vec3 abs() const noexcept {
        return Vec3(std::abs(data_[0]), std::abs(data_[1]), std::abs(data_[2]));
    }

    //=========================================================================
    // Angle/Direction Operations
    //=========================================================================
    
    /// Convert angles to direction vectors (forward, right, up)
    void toDirections(Vec3& forward, Vec3& right, Vec3& up) const noexcept;
    
    /// Convert angles to forward direction only
    [[nodiscard]] Vec3 toForward() const noexcept;

private:
    VecT data_[SIZE];
};

//=============================================================================
// Non-member Operators
//=============================================================================

/// Scalar multiplication (scalar on left)
[[nodiscard]] inline constexpr Vec3 operator*(VecT scalar, const Vec3& vec) noexcept {
    return vec * scalar;
}

//=============================================================================
// Free Functions (compatible with C macro names)
//=============================================================================

/// Dot product (DotProduct macro replacement)
[[nodiscard]] inline constexpr VecT dotProduct(const Vec3& a, const Vec3& b) noexcept {
    return a.dot(b);
}

/// Cross product (CrossProduct macro replacement)
[[nodiscard]] inline constexpr Vec3 crossProduct(const Vec3& a, const Vec3& b) noexcept {
    return a.cross(b);
}

/// Vector length (VectorLength replacement)
[[nodiscard]] inline VecT vectorLength(const Vec3& v) noexcept {
    return v.length();
}

/// Normalize (VectorNormalize replacement)
inline VecT vectorNormalize(Vec3& v) noexcept {
    return v.normalize();
}

/// Linear interpolation
[[nodiscard]] inline constexpr Vec3 vectorLerp(const Vec3& a, VecT t, const Vec3& b) noexcept {
    return a.lerp(b, t);
}

/// Vector multiply-add: result = a + scale * b (VectorMA replacement)
[[nodiscard]] inline constexpr Vec3 vectorMA(const Vec3& a, VecT scale, const Vec3& b) noexcept {
    return a + b * scale;
}

/// Angle vectors conversion
void angleVectors(const Vec3& angles, Vec3& forward, Vec3& right, Vec3& up) noexcept;

/// Convert degrees to radians
[[nodiscard]] inline constexpr VecT degToRad(VecT degrees) noexcept {
    return degrees * constants::DEG_TO_RAD;
}

/// Convert radians to degrees
[[nodiscard]] inline constexpr VecT radToDeg(VecT radians) noexcept {
    return radians * constants::RAD_TO_DEG;
}

} // namespace math

// Bring commonly used types into ezquake namespace
using math::Vec3;

} // namespace ezquake

#endif // EZQUAKE_CORE_MATH_VEC3_HPP
