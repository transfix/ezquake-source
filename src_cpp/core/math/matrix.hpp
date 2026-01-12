/*
 * ezQuake C++ Port - 3x3 Matrix Class
 * 
 * Modern C++17 implementation of 3x3 matrices for rotations
 * and transformations. Compatible with original matrix3x3_t.
 * 
 * Original: typedef vec_t matrix3x3_t[3][3];
 * 
 * Copyright (C) 1996-1997 Id Software, Inc.
 * Copyright (C) 2024 ezQuake Team
 */

#ifndef EZQUAKE_CORE_MATH_MATRIX_HPP
#define EZQUAKE_CORE_MATH_MATRIX_HPP

#include "../types.hpp"
#include "vec3.hpp"
#include <cmath>

namespace ezquake {
namespace math {

/**
 * @brief 3x3 Matrix class for rotations and transformations
 * 
 * Row-major storage for compatibility with original Quake code.
 * matrix[row][col]
 */
class Matrix3x3 {
public:
    using value_type = VecT;
    using size_type = Size;
    
    static constexpr size_type ROWS = 3;
    static constexpr size_type COLS = 3;
    static constexpr size_type SIZE = ROWS * COLS;

    //=========================================================================
    // Static Factories
    //=========================================================================
    
    /// Identity matrix
    [[nodiscard]] static constexpr Matrix3x3 identity() noexcept {
        return Matrix3x3{
            1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 1.0f
        };
    }
    
    /// Zero matrix
    [[nodiscard]] static constexpr Matrix3x3 zero() noexcept {
        return Matrix3x3{};
    }
    
    /// Create rotation matrix from angle (radians) and axis
    [[nodiscard]] static Matrix3x3 fromAxisAngle(VecT angle, const Vec3& axis) noexcept;
    
    /// Create rotation matrix from Euler angles
    [[nodiscard]] static Matrix3x3 fromEuler(const Vec3& angles) noexcept;

    //=========================================================================
    // Constructors
    //=========================================================================
    
    /// Default constructor - zero matrix
    constexpr Matrix3x3() noexcept : data_{} {}
    
    /// Construct from individual elements (row-major)
    constexpr Matrix3x3(
        VecT m00, VecT m01, VecT m02,
        VecT m10, VecT m11, VecT m12,
        VecT m20, VecT m21, VecT m22
    ) noexcept : data_{
        {m00, m01, m02},
        {m10, m11, m12},
        {m20, m21, m22}
    } {}
    
    /// Construct from row vectors
    constexpr Matrix3x3(const Vec3& row0, const Vec3& row1, const Vec3& row2) noexcept
        : data_{
            {row0.x(), row0.y(), row0.z()},
            {row1.x(), row1.y(), row1.z()},
            {row2.x(), row2.y(), row2.z()}
        } {}
    
    /// Construct from C-style 2D array (compatibility)
    explicit constexpr Matrix3x3(const VecT src[3][3]) noexcept
        : data_{
            {src[0][0], src[0][1], src[0][2]},
            {src[1][0], src[1][1], src[1][2]},
            {src[2][0], src[2][1], src[2][2]}
        } {}
    
    /// Copy/move constructors
    constexpr Matrix3x3(const Matrix3x3&) noexcept = default;
    constexpr Matrix3x3(Matrix3x3&&) noexcept = default;
    constexpr Matrix3x3& operator=(const Matrix3x3&) noexcept = default;
    constexpr Matrix3x3& operator=(Matrix3x3&&) noexcept = default;
    ~Matrix3x3() = default;

    //=========================================================================
    // Element Access
    //=========================================================================
    
    /// Access row
    [[nodiscard]] constexpr VecT* operator[](size_type row) noexcept {
        return data_[row];
    }
    
    [[nodiscard]] constexpr const VecT* operator[](size_type row) const noexcept {
        return data_[row];
    }
    
    /// Access element
    [[nodiscard]] constexpr VecT& operator()(size_type row, size_type col) noexcept {
        return data_[row][col];
    }
    
    [[nodiscard]] constexpr VecT operator()(size_type row, size_type col) const noexcept {
        return data_[row][col];
    }
    
    /// Get row as Vec3
    [[nodiscard]] constexpr Vec3 row(size_type i) const noexcept {
        return Vec3(data_[i][0], data_[i][1], data_[i][2]);
    }
    
    /// Get column as Vec3
    [[nodiscard]] constexpr Vec3 col(size_type i) const noexcept {
        return Vec3(data_[0][i], data_[1][i], data_[2][i]);
    }
    
    /// Set row
    constexpr void setRow(size_type i, const Vec3& v) noexcept {
        data_[i][0] = v.x();
        data_[i][1] = v.y();
        data_[i][2] = v.z();
    }
    
    /// Set column
    constexpr void setCol(size_type i, const Vec3& v) noexcept {
        data_[0][i] = v.x();
        data_[1][i] = v.y();
        data_[2][i] = v.z();
    }
    
    /// Raw data access (for C compatibility)
    [[nodiscard]] constexpr VecT* data() noexcept { return &data_[0][0]; }
    [[nodiscard]] constexpr const VecT* data() const noexcept { return &data_[0][0]; }
    
    /// Access as 2D array pointer type
    using ArrayType = VecT(*)[3];
    using ConstArrayType = const VecT(*)[3];
    
    /// Access as 2D array (for C compatibility)
    [[nodiscard]] constexpr ArrayType array() noexcept { return data_; }
    [[nodiscard]] constexpr ConstArrayType array() const noexcept { return data_; }

    //=========================================================================
    // Matrix Operations
    //=========================================================================
    
    /// Transpose
    [[nodiscard]] constexpr Matrix3x3 transposed() const noexcept {
        return Matrix3x3{
            data_[0][0], data_[1][0], data_[2][0],
            data_[0][1], data_[1][1], data_[2][1],
            data_[0][2], data_[1][2], data_[2][2]
        };
    }
    
    /// Transpose in place
    void transpose() noexcept {
        std::swap(data_[0][1], data_[1][0]);
        std::swap(data_[0][2], data_[2][0]);
        std::swap(data_[1][2], data_[2][1]);
    }
    
    /// Determinant
    [[nodiscard]] constexpr VecT determinant() const noexcept {
        return data_[0][0] * (data_[1][1] * data_[2][2] - data_[1][2] * data_[2][1])
             - data_[0][1] * (data_[1][0] * data_[2][2] - data_[1][2] * data_[2][0])
             + data_[0][2] * (data_[1][0] * data_[2][1] - data_[1][1] * data_[2][0]);
    }
    
    /// Inverse (returns identity if singular)
    [[nodiscard]] Matrix3x3 inverted() const noexcept;
    
    /// Matrix multiplication
    [[nodiscard]] constexpr Matrix3x3 operator*(const Matrix3x3& rhs) const noexcept {
        Matrix3x3 result;
        for (size_type i = 0; i < ROWS; ++i) {
            for (size_type j = 0; j < COLS; ++j) {
                result.data_[i][j] = 
                    data_[i][0] * rhs.data_[0][j] +
                    data_[i][1] * rhs.data_[1][j] +
                    data_[i][2] * rhs.data_[2][j];
            }
        }
        return result;
    }
    
    /// Matrix-vector multiplication
    [[nodiscard]] constexpr Vec3 operator*(const Vec3& v) const noexcept {
        return Vec3(
            data_[0][0] * v.x() + data_[0][1] * v.y() + data_[0][2] * v.z(),
            data_[1][0] * v.x() + data_[1][1] * v.y() + data_[1][2] * v.z(),
            data_[2][0] * v.x() + data_[2][1] * v.y() + data_[2][2] * v.z()
        );
    }
    
    /// Scalar multiplication
    [[nodiscard]] constexpr Matrix3x3 operator*(VecT scalar) const noexcept {
        Matrix3x3 result;
        for (size_type i = 0; i < ROWS; ++i) {
            for (size_type j = 0; j < COLS; ++j) {
                result.data_[i][j] = data_[i][j] * scalar;
            }
        }
        return result;
    }
    
    /// Matrix addition
    [[nodiscard]] constexpr Matrix3x3 operator+(const Matrix3x3& rhs) const noexcept {
        Matrix3x3 result;
        for (size_type i = 0; i < ROWS; ++i) {
            for (size_type j = 0; j < COLS; ++j) {
                result.data_[i][j] = data_[i][j] + rhs.data_[i][j];
            }
        }
        return result;
    }
    
    /// Matrix subtraction
    [[nodiscard]] constexpr Matrix3x3 operator-(const Matrix3x3& rhs) const noexcept {
        Matrix3x3 result;
        for (size_type i = 0; i < ROWS; ++i) {
            for (size_type j = 0; j < COLS; ++j) {
                result.data_[i][j] = data_[i][j] - rhs.data_[i][j];
            }
        }
        return result;
    }
    
    /// Compound assignment
    constexpr Matrix3x3& operator*=(const Matrix3x3& rhs) noexcept {
        *this = *this * rhs;
        return *this;
    }
    
    constexpr Matrix3x3& operator*=(VecT scalar) noexcept {
        for (size_type i = 0; i < ROWS; ++i) {
            for (size_type j = 0; j < COLS; ++j) {
                data_[i][j] *= scalar;
            }
        }
        return *this;
    }

    //=========================================================================
    // Comparison
    //=========================================================================
    
    [[nodiscard]] constexpr bool operator==(const Matrix3x3& rhs) const noexcept {
        for (size_type i = 0; i < ROWS; ++i) {
            for (size_type j = 0; j < COLS; ++j) {
                if (data_[i][j] != rhs.data_[i][j]) return false;
            }
        }
        return true;
    }
    
    [[nodiscard]] constexpr bool operator!=(const Matrix3x3& rhs) const noexcept {
        return !(*this == rhs);
    }
    
    /// Approximate equality
    [[nodiscard]] bool approxEqual(const Matrix3x3& rhs, VecT eps = constants::EPSILON) const noexcept {
        for (size_type i = 0; i < ROWS; ++i) {
            for (size_type j = 0; j < COLS; ++j) {
                if (std::abs(data_[i][j] - rhs.data_[i][j]) > eps) return false;
            }
        }
        return true;
    }

private:
    VecT data_[ROWS][COLS];
};

//=============================================================================
// Non-member operators
//=============================================================================

/// Scalar * Matrix
[[nodiscard]] inline constexpr Matrix3x3 operator*(VecT scalar, const Matrix3x3& m) noexcept {
    return m * scalar;
}

//=============================================================================
// Free Functions (C compatibility wrappers)
//=============================================================================

/// Create rotation matrix (Matrix3x3_CreateRotate replacement)
inline Matrix3x3 matrix3x3CreateRotate(VecT angle, const Vec3& axis) noexcept {
    return Matrix3x3::fromAxisAngle(angle, axis);
}

/// Matrix-vector multiply (Matrix3x3_MultiplyByVector replacement)
inline Vec3 matrix3x3MultiplyByVector(const Matrix3x3& m, const Vec3& v) noexcept {
    return m * v;
}

/// Concatenate rotation matrices (R_ConcatRotations replacement)
inline Matrix3x3 concatRotations(const Matrix3x3& a, const Matrix3x3& b) noexcept {
    return a * b;
}

} // namespace math

using math::Matrix3x3;

} // namespace ezquake

#endif // EZQUAKE_CORE_MATH_MATRIX_HPP
