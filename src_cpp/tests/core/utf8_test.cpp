/*
 * ezQuake C++ Port - UTF-8 Unit Tests
 * 
 * Copyright (C) 2024 ezQuake Team
 */

#include <gtest/gtest.h>
#include "core/text/utf8.hpp"

using namespace ezquake;
using namespace ezquake::text::utf8;

//=============================================================================
// Encoding Tests
//=============================================================================

TEST(Utf8Test, EncodeAscii) {
    char buf[4];
    EXPECT_EQ(encode('A', buf), 1u);
    EXPECT_EQ(buf[0], 'A');
}

TEST(Utf8Test, EncodeTwoByte) {
    char buf[4];
    // ñ = U+00F1
    EXPECT_EQ(encode(0x00F1, buf), 2u);
    EXPECT_EQ(static_cast<unsigned char>(buf[0]), 0xC3);
    EXPECT_EQ(static_cast<unsigned char>(buf[1]), 0xB1);
}

TEST(Utf8Test, EncodeThreeByte) {
    char buf[4];
    // € = U+20AC
    EXPECT_EQ(encode(0x20AC, buf), 3u);
    EXPECT_EQ(static_cast<unsigned char>(buf[0]), 0xE2);
    EXPECT_EQ(static_cast<unsigned char>(buf[1]), 0x82);
    EXPECT_EQ(static_cast<unsigned char>(buf[2]), 0xAC);
}

TEST(Utf8Test, EncodeFourByte) {
    char buf[4];
    // 😀 = U+1F600
    EXPECT_EQ(encode(0x1F600, buf), 4u);
    EXPECT_EQ(static_cast<unsigned char>(buf[0]), 0xF0);
    EXPECT_EQ(static_cast<unsigned char>(buf[1]), 0x9F);
    EXPECT_EQ(static_cast<unsigned char>(buf[2]), 0x98);
    EXPECT_EQ(static_cast<unsigned char>(buf[3]), 0x80);
}

//=============================================================================
// Decoding Tests
//=============================================================================

TEST(Utf8Test, DecodeAscii) {
    const char* str = "A";
    Char32 cp;
    const char* next = decode(str, str + 1, cp);
    EXPECT_EQ(cp, 'A');
    EXPECT_EQ(next, str + 1);
}

TEST(Utf8Test, DecodeTwoByte) {
    const char* str = "\xC3\xB1"; // ñ
    Char32 cp;
    const char* next = decode(str, str + 2, cp);
    EXPECT_EQ(cp, 0x00F1u);
    EXPECT_EQ(next, str + 2);
}

TEST(Utf8Test, DecodeThreeByte) {
    const char* str = "\xE2\x82\xAC"; // €
    Char32 cp;
    const char* next = decode(str, str + 3, cp);
    EXPECT_EQ(cp, 0x20ACu);
    EXPECT_EQ(next, str + 3);
}

TEST(Utf8Test, DecodeFourByte) {
    const char* str = "\xF0\x9F\x98\x80"; // 😀
    Char32 cp;
    const char* next = decode(str, str + 4, cp);
    EXPECT_EQ(cp, 0x1F600u);
    EXPECT_EQ(next, str + 4);
}

//=============================================================================
// Length Tests
//=============================================================================

TEST(Utf8Test, LengthAscii) {
    EXPECT_EQ(length("hello"), 5u);
}

TEST(Utf8Test, LengthMixed) {
    // "hëllo" - 5 code points, but 6 bytes
    EXPECT_EQ(length("h\xC3\xABllo"), 5u);
}

TEST(Utf8Test, LengthEmoji) {
    // "😀😀" - 2 code points, 8 bytes
    EXPECT_EQ(length("\xF0\x9F\x98\x80\xF0\x9F\x98\x80"), 2u);
}

//=============================================================================
// Validation Tests
//=============================================================================

TEST(Utf8Test, IsValidAscii) {
    EXPECT_TRUE(isValid("hello world"));
}

TEST(Utf8Test, IsValidUtf8) {
    EXPECT_TRUE(isValid("héllo wörld"));
    EXPECT_TRUE(isValid("\xF0\x9F\x98\x80")); // emoji
}

TEST(Utf8Test, IsValidInvalid) {
    // Invalid continuation byte
    EXPECT_FALSE(isValid("\x80"));
    // Incomplete sequence
    EXPECT_FALSE(isValid("\xC3"));
}

//=============================================================================
// Code Point Length Tests
//=============================================================================

TEST(Utf8Test, CodePointLength) {
    EXPECT_EQ(codePointLength('A'), 1u);
    EXPECT_EQ(codePointLength('\xC3'), 2u);
    EXPECT_EQ(codePointLength('\xE2'), 3u);
    EXPECT_EQ(codePointLength('\xF0'), 4u);
    EXPECT_EQ(codePointLength('\x80'), 0u); // Invalid (continuation byte)
}

//=============================================================================
// Iterator Tests
//=============================================================================

TEST(Utf8Test, IteratorBasic) {
    String s = "hello";
    Size count = 0;
    for (auto cp : codePoints(s)) {
        (void)cp;
        ++count;
    }
    EXPECT_EQ(count, 5u);
}

TEST(Utf8Test, IteratorMixed) {
    // héllo - 5 code points
    String s = "h\xC3\xABllo";
    Vector<Char32> cps;
    for (auto cp : codePoints(s)) {
        cps.push_back(cp);
    }
    ASSERT_EQ(cps.size(), 5u);
    EXPECT_EQ(cps[0], 'h');
    EXPECT_EQ(cps[1], 0x00EB); // ë
    EXPECT_EQ(cps[2], 'l');
    EXPECT_EQ(cps[3], 'l');
    EXPECT_EQ(cps[4], 'o');
}

//=============================================================================
// Conversion Tests
//=============================================================================

TEST(Utf8Test, FromUtf16) {
    WString ws;
    ws.push_back(u'h');
    ws.push_back(u'e');
    ws.push_back(u'l');
    ws.push_back(u'l');
    ws.push_back(u'o');
    
    String s = fromUtf16(ws);
    EXPECT_EQ(s, "hello");
}

TEST(Utf8Test, ToUtf16) {
    String s = "hello";
    WString ws = toUtf16(s);
    
    ASSERT_EQ(ws.size(), 5u);
    EXPECT_EQ(ws[0], u'h');
    EXPECT_EQ(ws[1], u'e');
    EXPECT_EQ(ws[2], u'l');
    EXPECT_EQ(ws[3], u'l');
    EXPECT_EQ(ws[4], u'o');
}

TEST(Utf8Test, FromLatin1) {
    // Latin-1 string with ñ (0xF1)
    String latin1 = "ma\xF1" "ana";
    String utf8 = fromLatin1(latin1);
    EXPECT_EQ(utf8, "mañana");
}

TEST(Utf8Test, ToLatin1) {
    String utf8 = "mañana";
    String latin1 = toLatin1(utf8);
    // Should be back to original Latin-1
    EXPECT_EQ(latin1, "ma\xF1" "ana");
}

TEST(Utf8Test, ToLatin1Lossy) {
    // String with character outside Latin-1 range
    String utf8 = "hello\xE2\x82\xAC"; // hello€
    String latin1 = toLatin1(utf8);
    EXPECT_EQ(latin1, "hello?"); // € replaced with ?
}

//=============================================================================
// Quake Encoding Tests
//=============================================================================

TEST(Utf8Test, QuakeCharToUnicode) {
    // ASCII characters unchanged
    EXPECT_EQ(quakeCharToUnicode('A'), 'A');
    EXPECT_EQ(quakeCharToUnicode('z'), 'z');
}

TEST(Utf8Test, UnicodeToQuakeChar) {
    // ASCII characters unchanged
    EXPECT_EQ(unicodeToQuakeChar('A'), 'A');
    EXPECT_EQ(unicodeToQuakeChar('z'), 'z');
    
    // Non-mappable characters become '?'
    EXPECT_EQ(unicodeToQuakeChar(0x20AC), '?'); // €
}

TEST(Utf8Test, QuakeRoundTrip) {
    // ASCII should round-trip perfectly
    String original = "Hello World 123";
    String quake = toQuake(original);
    String back = fromQuake(quake);
    
    // Note: roundtrip may not be perfect for all chars
    // but ASCII should work
    for (Size i = 0; i < original.size(); ++i) {
        if (static_cast<unsigned char>(original[i]) < 0x80) {
            EXPECT_EQ(quake[i], original[i]);
        }
    }
}
