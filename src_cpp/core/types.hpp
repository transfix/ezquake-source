/*
 * ezQuake C++ Port - Core Type Definitions
 * 
 * This file defines the fundamental types used throughout the C++ port.
 * It maintains compatibility with the original C codebase while providing
 * modern C++ type safety and convenience.
 * 
 * Copyright (C) 1996-1997 Id Software, Inc.
 * Copyright (C) 2024 ezQuake Team
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef EZQUAKE_CORE_TYPES_HPP
#define EZQUAKE_CORE_TYPES_HPP

#include <cstdint>
#include <cstddef>
#include <string>
#include <string_view>
#include <memory>
#include <optional>
#include <variant>
#include <array>
#include <vector>
#include <span>
#include <functional>
#include <stdexcept>
#include <concepts>
#include <source_location>

namespace ezquake {

//=============================================================================
// Fundamental Types - Compatible with original Quake types
//=============================================================================

/// Boolean type (replaces qbool)
using Bool = bool;

/// Byte type (replaces byte)
using Byte = std::uint8_t;

/// Character types
using Char = char;
using WChar = char16_t;  // 16-bit Unicode char (replaces wchar)
using Char32 = char32_t; // Full Unicode code point

/// Integer types with explicit sizes
using Int8 = std::int8_t;
using Int16 = std::int16_t;
using Int32 = std::int32_t;
using Int64 = std::int64_t;

using UInt8 = std::uint8_t;
using UInt16 = std::uint16_t;
using UInt32 = std::uint32_t;
using UInt64 = std::uint64_t;

/// Size type
using Size = std::size_t;

/// Pointer difference type
using PtrDiff = std::ptrdiff_t;

/// Floating point types
using Float = float;
using Double = double;

/// Fixed point types (for legacy compatibility)
using Fixed4 = std::int32_t;
using Fixed8 = std::int32_t;
using Fixed16 = std::int32_t;

//=============================================================================
// Math Vector Type Aliases (actual classes defined in math/)
//=============================================================================

/// Scalar type for vector math (matches original vec_t)
using VecT = float;

//=============================================================================
// String Types with UTF-8 Support
//=============================================================================

/// Standard string (UTF-8 encoded)
using String = std::string;

/// String view (non-owning reference)
using StringView = std::string_view;

/// Wide string for legacy compatibility
using WString = std::u16string;
using WStringView = std::u16string_view;

/// UTF-32 string for full Unicode support
using U32String = std::u32string;
using U32StringView = std::u32string_view;

//=============================================================================
// Smart Pointer Aliases
//=============================================================================

template<typename T>
using UniquePtr = std::unique_ptr<T>;

template<typename T>
using SharedPtr = std::shared_ptr<T>;

template<typename T>
using WeakPtr = std::weak_ptr<T>;

/// Factory functions for smart pointers
template<typename T, typename... Args>
[[nodiscard]] inline UniquePtr<T> makeUnique(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T, typename... Args>
[[nodiscard]] inline SharedPtr<T> makeShared(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

//=============================================================================
// Container Aliases
//=============================================================================

template<typename T, Size N>
using Array = std::array<T, N>;

template<typename T>
using Vector = std::vector<T>;

// C++20: Use std::span directly
template<typename T>
using Span = std::span<T>;

template<typename T>
using Optional = std::optional<T>;

//=============================================================================
// Callback/Function Types
//=============================================================================

template<typename Signature>
using Function = std::function<Signature>;

//=============================================================================
// Path Limits (compatible with original defines)
//=============================================================================

constexpr Size MAX_QPATH = 64;   // Max length of a Quake game pathname
constexpr Size MAX_OSPATH = 260; // Max length of a filesystem pathname

//=============================================================================
// Result Type for Error Handling
//=============================================================================

/// Represents success or failure with an optional error message
template<typename T>
class Result {
public:
    /// Create a successful result
    static Result success(T value) {
        return Result(std::move(value));
    }
    
    /// Create a failed result
    static Result failure(String error) {
        return Result(std::move(error), false);
    }
    
    /// Check if result is successful
    [[nodiscard]] bool isOk() const noexcept { return hasValue_; }
    [[nodiscard]] bool isErr() const noexcept { return !hasValue_; }
    
    /// Get the value (throws if error)
    [[nodiscard]] const T& value() const& {
        if (!hasValue_) throw std::runtime_error(error_);
        return std::get<T>(data_);
    }
    
    [[nodiscard]] T&& value() && {
        if (!hasValue_) throw std::runtime_error(error_);
        return std::move(std::get<T>(data_));
    }
    
    /// Get the error message
    [[nodiscard]] const String& error() const noexcept { return error_; }
    
    /// Get value or default
    [[nodiscard]] T valueOr(T defaultValue) const& {
        return hasValue_ ? std::get<T>(data_) : std::move(defaultValue);
    }
    
    /// Explicit bool conversion
    explicit operator bool() const noexcept { return hasValue_; }

private:
    explicit Result(T value) : data_(std::move(value)), hasValue_(true) {}
    explicit Result(String error, bool) : error_(std::move(error)), hasValue_(false) {}
    
    std::variant<std::monostate, T> data_;
    String error_;
    bool hasValue_ = false;
};

/// Specialization for void results
template<>
class Result<void> {
public:
    static Result success() { return Result(true); }
    static Result failure(String error) { return Result(std::move(error)); }
    
    [[nodiscard]] bool isOk() const noexcept { return success_; }
    [[nodiscard]] bool isErr() const noexcept { return !success_; }
    [[nodiscard]] const String& error() const noexcept { return error_; }
    
    explicit operator bool() const noexcept { return success_; }

private:
    explicit Result(bool success) : success_(success) {}
    explicit Result(String error) : error_(std::move(error)), success_(false) {}
    
    String error_;
    bool success_ = true;
};

//=============================================================================
// Non-Copyable Base Class (for RAII resources)
//=============================================================================

class NonCopyable {
protected:
    NonCopyable() = default;
    ~NonCopyable() = default;
    
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
    
    NonCopyable(NonCopyable&&) = default;
    NonCopyable& operator=(NonCopyable&&) = default;
};

//=============================================================================
// C++20 Concepts
//=============================================================================

/// Concept for numeric types
template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

/// Concept for string-like types
template<typename T>
concept StringLike = std::convertible_to<T, StringView>;

/// Concept for callable with specific signature
template<typename F, typename R, typename... Args>
concept Callable = std::invocable<F, Args...> && 
                   std::convertible_to<std::invoke_result_t<F, Args...>, R>;

/// Concept for range types
template<typename T>
concept Range = requires(T& t) {
    std::begin(t);
    std::end(t);
};

//=============================================================================
// Compile-time Constants
//=============================================================================

namespace constants {
    constexpr Float PI = 3.14159265358979323846f;
    constexpr Float TAU = PI * 2.0f;
    constexpr Float HALF_PI = PI / 2.0f;
    constexpr Float DEG_TO_RAD = PI / 180.0f;
    constexpr Float RAD_TO_DEG = 180.0f / PI;
    constexpr Float EPSILON = 0.0001f;
    constexpr Float ON_EPSILON = 0.1f;  // Point on plane side epsilon
}

} // namespace ezquake

#endif // EZQUAKE_CORE_TYPES_HPP
