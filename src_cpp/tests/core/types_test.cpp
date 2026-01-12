/*
 * ezQuake C++ Port - Core Types Unit Tests
 * 
 * Copyright (C) 2024 ezQuake Team
 */

#include <gtest/gtest.h>
#include "core/types.hpp"

using namespace ezquake;

//=============================================================================
// Type Size Tests
//=============================================================================

TEST(TypesTest, IntegerSizes) {
    EXPECT_EQ(sizeof(Int8), 1);
    EXPECT_EQ(sizeof(Int16), 2);
    EXPECT_EQ(sizeof(Int32), 4);
    EXPECT_EQ(sizeof(Int64), 8);
    
    EXPECT_EQ(sizeof(UInt8), 1);
    EXPECT_EQ(sizeof(UInt16), 2);
    EXPECT_EQ(sizeof(UInt32), 4);
    EXPECT_EQ(sizeof(UInt64), 8);
}

TEST(TypesTest, FloatSizes) {
    EXPECT_EQ(sizeof(Float), 4);
    EXPECT_EQ(sizeof(Double), 8);
    EXPECT_EQ(sizeof(VecT), 4);
}

TEST(TypesTest, CharSizes) {
    EXPECT_EQ(sizeof(Byte), 1);
    EXPECT_EQ(sizeof(Char), 1);
    EXPECT_EQ(sizeof(WChar), 2);
    EXPECT_EQ(sizeof(Char32), 4);
}

//=============================================================================
// Result Type Tests
//=============================================================================

TEST(ResultTest, SuccessResult) {
    auto result = Result<int>::success(42);
    
    EXPECT_TRUE(result.isOk());
    EXPECT_FALSE(result.isErr());
    EXPECT_EQ(result.value(), 42);
    EXPECT_TRUE(static_cast<bool>(result));
}

TEST(ResultTest, FailureResult) {
    auto result = Result<int>::failure("Something went wrong");
    
    EXPECT_FALSE(result.isOk());
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), "Something went wrong");
    EXPECT_FALSE(static_cast<bool>(result));
}

TEST(ResultTest, ValueOr) {
    auto success = Result<int>::success(42);
    auto failure = Result<int>::failure("error");
    
    EXPECT_EQ(success.valueOr(100), 42);
    EXPECT_EQ(failure.valueOr(100), 100);
}

TEST(ResultTest, ValueThrowsOnError) {
    auto result = Result<int>::failure("error");
    
    EXPECT_THROW(result.value(), std::runtime_error);
}

TEST(ResultTest, VoidResult) {
    auto success = Result<void>::success();
    auto failure = Result<void>::failure("error");
    
    EXPECT_TRUE(success.isOk());
    EXPECT_FALSE(failure.isOk());
}

//=============================================================================
// Smart Pointer Tests
//=============================================================================

TEST(SmartPointerTest, MakeUnique) {
    auto ptr = makeUnique<int>(42);
    EXPECT_EQ(*ptr, 42);
}

TEST(SmartPointerTest, MakeShared) {
    auto ptr = makeShared<int>(42);
    EXPECT_EQ(*ptr, 42);
    EXPECT_EQ(ptr.use_count(), 1);
}

//=============================================================================
// Constant Tests
//=============================================================================

TEST(ConstantsTest, MathConstants) {
    EXPECT_FLOAT_EQ(constants::PI, 3.14159265358979323846f);
    EXPECT_FLOAT_EQ(constants::TAU, constants::PI * 2.0f);
    EXPECT_FLOAT_EQ(constants::HALF_PI, constants::PI / 2.0f);
    EXPECT_FLOAT_EQ(constants::DEG_TO_RAD, constants::PI / 180.0f);
    EXPECT_FLOAT_EQ(constants::RAD_TO_DEG, 180.0f / constants::PI);
}

TEST(ConstantsTest, PathLimits) {
    EXPECT_EQ(MAX_QPATH, 64u);
    EXPECT_EQ(MAX_OSPATH, 260u);
}
