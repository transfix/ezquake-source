/*
 * ezQuake C++ Port - String Utilities Unit Tests
 * 
 * Copyright (C) 2024 ezQuake Team
 */

#include <gtest/gtest.h>
#include "core/text/string_utils.hpp"

using namespace ezquake;
using namespace ezquake::text;

//=============================================================================
// Case Conversion Tests
//=============================================================================

TEST(StringUtilsTest, ToUpper) {
    String s = "Hello World";
    toUpper(s);
    EXPECT_EQ(s, "HELLO WORLD");
}

TEST(StringUtilsTest, ToLower) {
    String s = "Hello World";
    toLower(s);
    EXPECT_EQ(s, "hello world");
}

TEST(StringUtilsTest, ToUpperCopy) {
    EXPECT_EQ(toUpperCopy("Hello"), "HELLO");
}

TEST(StringUtilsTest, ToLowerCopy) {
    EXPECT_EQ(toLowerCopy("Hello"), "hello");
}

//=============================================================================
// Comparison Tests
//=============================================================================

TEST(StringUtilsTest, CompareIgnoreCase) {
    EXPECT_EQ(compareIgnoreCase("hello", "HELLO"), 0);
    EXPECT_LT(compareIgnoreCase("apple", "Banana"), 0);
    EXPECT_GT(compareIgnoreCase("Zebra", "apple"), 0);
}

TEST(StringUtilsTest, EqualsIgnoreCase) {
    EXPECT_TRUE(equalsIgnoreCase("hello", "HELLO"));
    EXPECT_TRUE(equalsIgnoreCase("WORLD", "world"));
    EXPECT_FALSE(equalsIgnoreCase("hello", "world"));
}

TEST(StringUtilsTest, FindIgnoreCase) {
    EXPECT_EQ(findIgnoreCase("Hello World", "world"), 6u);
    EXPECT_EQ(findIgnoreCase("Hello World", "HELLO"), 0u);
    EXPECT_EQ(findIgnoreCase("Hello World", "xyz"), StringView::npos);
}

TEST(StringUtilsTest, ContainsIgnoreCase) {
    EXPECT_TRUE(containsIgnoreCase("Hello World", "world"));
    EXPECT_FALSE(containsIgnoreCase("Hello World", "xyz"));
}

//=============================================================================
// Trim Tests
//=============================================================================

TEST(StringUtilsTest, TrimLeft) {
    EXPECT_EQ(trimLeft("  hello"), "hello");
    EXPECT_EQ(trimLeft("\t\nhello"), "hello");
    EXPECT_EQ(trimLeft("hello"), "hello");
}

TEST(StringUtilsTest, TrimRight) {
    EXPECT_EQ(trimRight("hello  "), "hello");
    EXPECT_EQ(trimRight("hello\t\n"), "hello");
    EXPECT_EQ(trimRight("hello"), "hello");
}

TEST(StringUtilsTest, Trim) {
    EXPECT_EQ(trim("  hello  "), "hello");
    EXPECT_EQ(trim("\t\nhello\t\n"), "hello");
    EXPECT_EQ(trim("hello"), "hello");
}

//=============================================================================
// Split/Join Tests
//=============================================================================

TEST(StringUtilsTest, Split) {
    auto parts = split("a,b,c", ',');
    ASSERT_EQ(parts.size(), 3u);
    EXPECT_EQ(parts[0], "a");
    EXPECT_EQ(parts[1], "b");
    EXPECT_EQ(parts[2], "c");
}

TEST(StringUtilsTest, SplitEmpty) {
    auto parts = split("a,,c", ',');
    ASSERT_EQ(parts.size(), 3u);
    EXPECT_EQ(parts[0], "a");
    EXPECT_EQ(parts[1], "");
    EXPECT_EQ(parts[2], "c");
}

TEST(StringUtilsTest, SplitAny) {
    auto parts = splitAny("a,b;c d", ",; ");
    ASSERT_EQ(parts.size(), 4u);
    EXPECT_EQ(parts[0], "a");
    EXPECT_EQ(parts[1], "b");
    EXPECT_EQ(parts[2], "c");
    EXPECT_EQ(parts[3], "d");
}

TEST(StringUtilsTest, Join) {
    Vector<String> parts = {"a", "b", "c"};
    EXPECT_EQ(join(parts, ","), "a,b,c");
    EXPECT_EQ(join(parts, " - "), "a - b - c");
}

//=============================================================================
// Prefix/Suffix Tests
//=============================================================================

TEST(StringUtilsTest, StartsWith) {
    EXPECT_TRUE(startsWith("hello world", "hello"));
    EXPECT_FALSE(startsWith("hello world", "world"));
    EXPECT_TRUE(startsWith("hello", "hello"));
}

TEST(StringUtilsTest, EndsWith) {
    EXPECT_TRUE(endsWith("hello world", "world"));
    EXPECT_FALSE(endsWith("hello world", "hello"));
    EXPECT_TRUE(endsWith("hello", "hello"));
}

TEST(StringUtilsTest, StartsWithIgnoreCase) {
    EXPECT_TRUE(startsWithIgnoreCase("Hello World", "HELLO"));
    EXPECT_FALSE(startsWithIgnoreCase("Hello World", "WORLD"));
}

TEST(StringUtilsTest, EndsWithIgnoreCase) {
    EXPECT_TRUE(endsWithIgnoreCase("Hello World", "WORLD"));
    EXPECT_FALSE(endsWithIgnoreCase("Hello World", "HELLO"));
}

//=============================================================================
// Replace Tests
//=============================================================================

TEST(StringUtilsTest, Replace) {
    EXPECT_EQ(replace("hello world", "world", "universe"), "hello universe");
    EXPECT_EQ(replace("aaa", "a", "bb"), "bbbbbb");
    EXPECT_EQ(replace("hello", "xyz", "abc"), "hello");
}

//=============================================================================
// Glob Match Tests
//=============================================================================

TEST(StringUtilsTest, GlobMatch) {
    EXPECT_TRUE(globMatch("*.txt", "file.txt"));
    EXPECT_TRUE(globMatch("file.*", "file.txt"));
    EXPECT_TRUE(globMatch("*", "anything"));
    EXPECT_TRUE(globMatch("file?.txt", "file1.txt"));
    EXPECT_FALSE(globMatch("file?.txt", "file12.txt"));
    EXPECT_TRUE(globMatch("**/*.txt", "path/to/file.txt"));
}

//=============================================================================
// Parse Tests
//=============================================================================

TEST(StringUtilsTest, ParseInt) {
    EXPECT_EQ(parseInt("42"), 42);
    EXPECT_EQ(parseInt("-42"), -42);
    EXPECT_EQ(parseInt("0x2A"), 42);
    EXPECT_EQ(parseInt("0X2a"), 42);
    EXPECT_EQ(parseInt("  42"), 42);
    EXPECT_EQ(parseInt(""), 0);
}

TEST(StringUtilsTest, ParseFloat) {
    EXPECT_FLOAT_EQ(parseFloat("3.14"), 3.14f);
    EXPECT_FLOAT_EQ(parseFloat("-3.14"), -3.14f);
    EXPECT_FLOAT_EQ(parseFloat("  3.14"), 3.14f);
    EXPECT_FLOAT_EQ(parseFloat(""), 0.0f);
}

TEST(StringUtilsTest, FormatFloat) {
    EXPECT_EQ(formatFloat(3.0f), "3");
    EXPECT_EQ(formatFloat(3.5f), "3.5");
    // Note: formatFloat removes trailing zeros
}

//=============================================================================
// Hash Tests
//=============================================================================

TEST(StringUtilsTest, Hash) {
    // Same strings should have same hash
    EXPECT_EQ(hash("hello"), hash("hello"));
    
    // Case insensitive
    EXPECT_EQ(hash("Hello"), hash("hello"));
    
    // Different strings should (usually) have different hashes
    EXPECT_NE(hash("hello"), hash("world"));
}

//=============================================================================
// Safe Copy/Concat Tests
//=============================================================================

TEST(StringUtilsTest, SafeCopy) {
    char buf[10];
    
    // Normal copy
    Size len = safeCopy(buf, "hello", sizeof(buf));
    EXPECT_EQ(len, 5u);
    EXPECT_STREQ(buf, "hello");
    
    // Truncated copy - buffer is 10 chars, so 9 chars + null terminator
    len = safeCopy(buf, "verylongstring", sizeof(buf));
    EXPECT_EQ(len, 14u); // Returns source length
    EXPECT_STREQ(buf, "verylongs"); // Truncated to 9 chars + null
}

TEST(StringUtilsTest, SafeConcat) {
    char buf[10];
    
    safeCopy(buf, "hello", sizeof(buf));
    Size len = safeConcat(buf, " world", sizeof(buf));
    
    EXPECT_EQ(len, 11u); // Would be 11 if not truncated
    EXPECT_STREQ(buf, "hello wor"); // Truncated
}
