// common_test.cpp - Unit tests for common utilities (Iteration 8)
// Tests InfoString, token parsing, and CommandLine

#include <gtest/gtest.h>
#include "core/common/common.hpp"

using namespace ezquake;
using namespace ezquake::text;

// =============================================================================
// InfoString Tests
// =============================================================================

class InfoStringTest : public ::testing::Test {
protected:
    void SetUp() override {}
};

TEST_F(InfoStringTest, DefaultConstruction) {
    InfoString info;
    EXPECT_TRUE(info.empty());
    EXPECT_EQ(info.size(), 0u);
    EXPECT_EQ(info.toString(), "");
}

TEST_F(InfoStringTest, ParseSimple) {
    InfoString info("\\name\\player\\team\\red");
    
    EXPECT_EQ(info.size(), 2u);
    EXPECT_EQ(info.get("name").value(), "player");
    EXPECT_EQ(info.get("team").value(), "red");
}

TEST_F(InfoStringTest, ParseWithoutLeadingBackslash) {
    InfoString info("name\\player\\team\\blue");
    
    EXPECT_EQ(info.get("name").value(), "player");
    EXPECT_EQ(info.get("team").value(), "blue");
}

TEST_F(InfoStringTest, GetNonExistent) {
    InfoString info("\\name\\player");
    
    EXPECT_FALSE(info.get("nonexistent").has_value());
    EXPECT_EQ(info.getOr("nonexistent", "default"), "default");
}

TEST_F(InfoStringTest, SetValue) {
    InfoString info;
    
    EXPECT_TRUE(info.set("name", "player"));
    EXPECT_TRUE(info.set("team", "red"));
    
    EXPECT_EQ(info.get("name").value(), "player");
    EXPECT_EQ(info.get("team").value(), "red");
}

TEST_F(InfoStringTest, SetReplacesExisting) {
    InfoString info("\\name\\oldname");
    
    EXPECT_TRUE(info.set("name", "newname"));
    EXPECT_EQ(info.get("name").value(), "newname");
    EXPECT_EQ(info.size(), 1u);
}

TEST_F(InfoStringTest, SetRejectsStarKeys) {
    InfoString info;
    
    EXPECT_FALSE(info.set("*spectator", "1"));
    EXPECT_FALSE(info.contains("*spectator"));
}

TEST_F(InfoStringTest, SetStarKeyAllowed) {
    InfoString info;
    
    EXPECT_TRUE(info.setStarKey("*spectator", "1"));
    EXPECT_EQ(info.get("*spectator").value(), "1");
}

TEST_F(InfoStringTest, SetRejectsBackslash) {
    InfoString info;
    
    EXPECT_FALSE(info.set("name", "player\\evil"));
    EXPECT_FALSE(info.set("bad\\key", "value"));
}

TEST_F(InfoStringTest, SetRejectsQuotes) {
    InfoString info;
    
    EXPECT_FALSE(info.set("name", "player\"evil"));
    EXPECT_FALSE(info.set("bad\"key", "value"));
}

TEST_F(InfoStringTest, RemoveKey) {
    InfoString info("\\name\\player\\team\\red");
    
    info.remove("name");
    
    EXPECT_FALSE(info.contains("name"));
    EXPECT_TRUE(info.contains("team"));
}

TEST_F(InfoStringTest, RemoveWithPrefix) {
    InfoString info;
    info.setStarKey("*spectator", "1");
    info.setStarKey("*version", "2.0");
    info.set("name", "player");
    
    info.removeWithPrefix('*');
    
    EXPECT_FALSE(info.contains("*spectator"));
    EXPECT_FALSE(info.contains("*version"));
    EXPECT_TRUE(info.contains("name"));
}

TEST_F(InfoStringTest, ToString) {
    InfoString info;
    info.set("name", "player");
    
    std::string str = info.toString();
    EXPECT_TRUE(str.find("\\name\\player") != std::string::npos);
}

TEST_F(InfoStringTest, Contains) {
    InfoString info("\\name\\player");
    
    EXPECT_TRUE(info.contains("name"));
    EXPECT_FALSE(info.contains("team"));
}

TEST_F(InfoStringTest, Keys) {
    InfoString info("\\a\\1\\b\\2\\c\\3");
    
    auto keys = info.keys();
    EXPECT_EQ(keys.size(), 3u);
}

TEST_F(InfoStringTest, ForEach) {
    InfoString info("\\a\\1\\b\\2");
    
    int count = 0;
    info.forEach([&count](std::string_view, std::string_view) {
        ++count;
    });
    
    EXPECT_EQ(count, 2);
}

TEST_F(InfoStringTest, MaxSizeEnforced) {
    InfoString info;
    info.setMaxSize(50);
    
    EXPECT_TRUE(info.set("short", "value"));
    
    // Try to set a value that would exceed the limit
    std::string longValue(100, 'x');
    EXPECT_FALSE(info.set("long", longValue));
}

TEST_F(InfoStringTest, EmptyValueRemoves) {
    InfoString info("\\name\\player");
    
    info.set("name", "");
    EXPECT_FALSE(info.contains("name"));
}

TEST_F(InfoStringTest, ParseComplexUserinfo) {
    // Real-world userinfo string
    InfoString info("\\name\\Player^1\\team\\red\\topcolor\\4\\bottomcolor\\4\\*spectator\\0");
    
    EXPECT_EQ(info.get("name").value(), "Player^1");
    EXPECT_EQ(info.get("team").value(), "red");
    EXPECT_EQ(info.get("topcolor").value(), "4");
    EXPECT_EQ(info.get("*spectator").value(), "0");
}

// =============================================================================
// Token Parser Tests
// =============================================================================

class TokenParserTest : public ::testing::Test {
protected:
    void SetUp() override {}
};

TEST_F(TokenParserTest, ParseSimpleWord) {
    auto result = parseToken("hello world");
    
    EXPECT_TRUE(result.valid());
    EXPECT_EQ(result.token, "hello");
    EXPECT_EQ(result.type, TokenType::String);
}

TEST_F(TokenParserTest, ParseQuotedString) {
    auto result = parseToken("\"hello world\"");
    
    EXPECT_TRUE(result.valid());
    EXPECT_EQ(result.token, "hello world");
    EXPECT_EQ(result.type, TokenType::String);
}

TEST_F(TokenParserTest, ParseNumber) {
    auto result = parseToken("42");
    
    EXPECT_TRUE(result.valid());
    EXPECT_EQ(result.token, "42");
    EXPECT_EQ(result.type, TokenType::Number);
}

TEST_F(TokenParserTest, ParseNegativeNumber) {
    auto result = parseToken("-3.14");
    
    EXPECT_TRUE(result.valid());
    EXPECT_EQ(result.token, "-3.14");
    EXPECT_EQ(result.type, TokenType::Number);
}

TEST_F(TokenParserTest, SkipsWhitespace) {
    auto result = parseToken("   \t\n  hello");
    
    EXPECT_TRUE(result.valid());
    EXPECT_EQ(result.token, "hello");
}

TEST_F(TokenParserTest, SkipsLineComment) {
    auto result = parseToken("// comment\nhello");
    
    EXPECT_TRUE(result.valid());
    EXPECT_EQ(result.token, "hello");
}

TEST_F(TokenParserTest, SkipsBlockComment) {
    auto result = parseToken("/* block comment */ hello");
    
    EXPECT_TRUE(result.valid());
    EXPECT_EQ(result.token, "hello");
}

TEST_F(TokenParserTest, EmptyInput) {
    auto result = parseToken("");
    
    EXPECT_FALSE(result.valid());
    EXPECT_TRUE(result.endOfInput());
}

TEST_F(TokenParserTest, OnlyWhitespace) {
    auto result = parseToken("   \t\n   ");
    
    EXPECT_FALSE(result.valid());
}

TEST_F(TokenParserTest, ParseAllTokens) {
    auto tokens = parseAllTokens("name \"John Doe\" age 42");
    
    ASSERT_EQ(tokens.size(), 4u);
    EXPECT_EQ(tokens[0], "name");
    EXPECT_EQ(tokens[1], "John Doe");
    EXPECT_EQ(tokens[2], "age");
    EXPECT_EQ(tokens[3], "42");
}

TEST_F(TokenParserTest, ParseWithPunctuation) {
    auto opts = ParseOptions::withPunctuation();
    auto result = parseToken("name=value", opts);
    
    EXPECT_EQ(result.token, "name");
    
    // Continue parsing
    std::string_view remaining = "name=value";
    remaining.remove_prefix(result.consumedBytes);
    
    auto result2 = parseToken(remaining, opts);
    EXPECT_EQ(result2.token, "=");
    EXPECT_EQ(result2.type, TokenType::Punctuation);
}

TEST_F(TokenParserTest, ParseCurlyBraces) {
    auto opts = ParseOptions::withCurlyBraces();
    auto result = parseToken("{nested {block} content}", opts);
    
    EXPECT_TRUE(result.valid());
    EXPECT_EQ(result.token, "nested {block} content");
}

TEST_F(TokenParserTest, Tokenize) {
    auto tokens = tokenize("one two three");
    
    ASSERT_EQ(tokens.size(), 3u);
    EXPECT_EQ(tokens[0], "one");
    EXPECT_EQ(tokens[1], "two");
    EXPECT_EQ(tokens[2], "three");
}

TEST_F(TokenParserTest, SkipWhitespace) {
    std::string_view input = "   hello";
    auto result = skipWhitespace(input);
    
    EXPECT_EQ(result, "hello");
}

TEST_F(TokenParserTest, SkipLine) {
    std::string_view input = "first line\nsecond line";
    auto result = skipLine(input);
    
    EXPECT_EQ(result, "second line");
}

TEST_F(TokenParserTest, IsQuakeWhitespace) {
    EXPECT_TRUE(isQuakeWhitespace(' '));
    EXPECT_TRUE(isQuakeWhitespace('\t'));
    EXPECT_TRUE(isQuakeWhitespace('\r'));
    EXPECT_TRUE(isQuakeWhitespace('\n'));
    EXPECT_FALSE(isQuakeWhitespace('a'));
    EXPECT_FALSE(isQuakeWhitespace('\0'));
}

TEST_F(TokenParserTest, ConsumedBytesIncludesQuotes) {
    auto result = parseToken("\"quoted string\" next");
    
    EXPECT_EQ(result.token, "quoted string");
    // Should have consumed: quote + 13 chars + quote = 15
    EXPECT_EQ(result.consumedBytes, 15u);
}

TEST_F(TokenParserTest, PreserveNewlines) {
    auto opts = ParseOptions{.preserveNewlines = true};
    auto result = parseToken("\n", opts);
    
    EXPECT_EQ(result.type, TokenType::Newline);
    EXPECT_EQ(result.token, "\n");
}

// =============================================================================
// CommandLine Tests
// =============================================================================

class CommandLineTest : public ::testing::Test {
protected:
    void SetUp() override {}
};

TEST_F(CommandLineTest, DefaultConstruction) {
    CommandLine cmdline;
    
    EXPECT_TRUE(cmdline.empty());
    EXPECT_EQ(cmdline.argc(), 0);
}

TEST_F(CommandLineTest, ParseFromArgcArgv) {
    const char* args[] = {"ezquake", "-window", "-width", "1920"};
    CommandLine cmdline(4, const_cast<char**>(args));
    
    EXPECT_EQ(cmdline.argc(), 4);
    EXPECT_EQ(cmdline.argv(0), "ezquake");
    EXPECT_EQ(cmdline.argv(1), "-window");
}

TEST_F(CommandLineTest, ParseFromVector) {
    std::vector<std::string> args = {"ezquake", "-basedir", "/path"};
    CommandLine cmdline(std::move(args));
    
    EXPECT_EQ(cmdline.argc(), 3);
    EXPECT_EQ(cmdline.argv(2), "/path");
}

TEST_F(CommandLineTest, ArgvOutOfBounds) {
    const char* args[] = {"ezquake"};
    CommandLine cmdline(1, const_cast<char**>(args));
    
    EXPECT_EQ(cmdline.argv(-1), "");
    EXPECT_EQ(cmdline.argv(100), "");
}

TEST_F(CommandLineTest, CheckParam) {
    const char* args[] = {"ezquake", "-window", "-width", "1920"};
    CommandLine cmdline(4, const_cast<char**>(args));
    
    EXPECT_EQ(cmdline.checkParam("-window"), 1);
    EXPECT_EQ(cmdline.checkParam("-width"), 2);
    EXPECT_EQ(cmdline.checkParam("-nonexistent"), 0);
}

TEST_F(CommandLineTest, HasParam) {
    const char* args[] = {"ezquake", "-window", "-width", "1920"};
    CommandLine cmdline(4, const_cast<char**>(args));
    
    EXPECT_TRUE(cmdline.hasParam("-window"));
    EXPECT_TRUE(cmdline.hasParam("-width"));
    EXPECT_FALSE(cmdline.hasParam("-fullscreen"));
}

TEST_F(CommandLineTest, GetParamValue) {
    const char* args[] = {"ezquake", "-width", "1920", "-height", "1080"};
    CommandLine cmdline(5, const_cast<char**>(args));
    
    auto width = cmdline.getParamValue("-width");
    EXPECT_TRUE(width.has_value());
    EXPECT_EQ(width.value(), "1920");
    
    auto height = cmdline.getParamValue("-height");
    EXPECT_TRUE(height.has_value());
    EXPECT_EQ(height.value(), "1080");
}

TEST_F(CommandLineTest, GetParamValueNone) {
    const char* args[] = {"ezquake", "-window"};
    CommandLine cmdline(2, const_cast<char**>(args));
    
    // -window has no value after it
    auto value = cmdline.getParamValue("-window");
    EXPECT_FALSE(value.has_value());
}

TEST_F(CommandLineTest, GetParamValueOr) {
    const char* args[] = {"ezquake"};
    CommandLine cmdline(1, const_cast<char**>(args));
    
    auto value = cmdline.getParamValueOr("-width", "800");
    EXPECT_EQ(value, "800");
}

TEST_F(CommandLineTest, GetParamValues) {
    const char* args[] = {"ezquake", "-userdir", "base", "id1"};
    CommandLine cmdline(4, const_cast<char**>(args));
    
    auto values = cmdline.getParamValues("-userdir", 2);
    ASSERT_EQ(values.size(), 2u);
    EXPECT_EQ(values[0], "base");
    EXPECT_EQ(values[1], "id1");
}

TEST_F(CommandLineTest, ClearArgv) {
    const char* args[] = {"ezquake", "-window"};
    CommandLine cmdline(2, const_cast<char**>(args));
    
    EXPECT_TRUE(cmdline.hasParam("-window"));
    
    cmdline.clearArgv(1);
    
    // Parameter should no longer be found
    EXPECT_FALSE(cmdline.hasParam("-window"));
    EXPECT_EQ(cmdline.argv(1), "");
}

TEST_F(CommandLineTest, QwUrl) {
    const char* args[] = {"ezquake", "qw://server.example.com:27500"};
    CommandLine cmdline(2, const_cast<char**>(args));
    
    EXPECT_TRUE(cmdline.hasQwUrl());
    
    auto url = cmdline.getQwUrl();
    EXPECT_TRUE(url.has_value());
    EXPECT_EQ(url.value(), "qw://server.example.com:27500");
}

TEST_F(CommandLineTest, NoQwUrl) {
    const char* args[] = {"ezquake", "-window"};
    CommandLine cmdline(2, const_cast<char**>(args));
    
    EXPECT_FALSE(cmdline.hasQwUrl());
    EXPECT_FALSE(cmdline.getQwUrl().has_value());
}

TEST_F(CommandLineTest, OriginalCmdline) {
    const char* args[] = {"ezquake", "-window", "-width", "1920"};
    CommandLine cmdline(4, const_cast<char**>(args));
    
    auto original = cmdline.originalCmdline();
    EXPECT_TRUE(original.find("ezquake") != std::string::npos);
    EXPECT_TRUE(original.find("-window") != std::string::npos);
}

TEST_F(CommandLineTest, PlusCommands) {
    const char* args[] = {"ezquake", "+connect", "server.com"};
    CommandLine cmdline(3, const_cast<char**>(args));
    
    EXPECT_TRUE(cmdline.hasParam("+connect"));
    auto server = cmdline.getParamValue("+connect");
    EXPECT_TRUE(server.has_value());
    EXPECT_EQ(server.value(), "server.com");
}

// =============================================================================
// va() Tests
// =============================================================================

TEST(VaTest, BasicFormatting) {
    const char* result = va("Hello %s", "World");
    EXPECT_STREQ(result, "Hello World");
}

TEST(VaTest, NumericFormatting) {
    const char* result = va("Value: %d, Float: %.2f", 42, 3.14);
    EXPECT_STREQ(result, "Value: 42, Float: 3.14");
}

TEST(VaTest, MultipleCallsRotatingBuffer) {
    const char* r1 = va("one");
    const char* r2 = va("two");
    const char* r3 = va("three");
    
    // All should be valid (within the 32-buffer rotation)
    EXPECT_STREQ(r1, "one");
    EXPECT_STREQ(r2, "two");
    EXPECT_STREQ(r3, "three");
}
