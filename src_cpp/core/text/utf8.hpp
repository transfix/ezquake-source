/*
 * ezQuake C++ Port - UTF-8 Support
 * 
 * Full UTF-8 encoding/decoding with support for conversion
 * to/from other encodings (UTF-16, Latin-1, etc.)
 * 
 * Copyright (C) 2024 ezQuake Team
 */

#ifndef EZQUAKE_CORE_TEXT_UTF8_HPP
#define EZQUAKE_CORE_TEXT_UTF8_HPP

#include "../types.hpp"

namespace ezquake {
namespace text {
namespace utf8 {

/**
 * @brief Decode a UTF-8 code point from string
 * 
 * @param str Pointer to UTF-8 string
 * @param strEnd End of string
 * @param outCodePoint Output code point
 * @return Pointer to next character, or nullptr on error
 */
const char* decode(const char* str, const char* strEnd, Char32& outCodePoint);

/**
 * @brief Encode a code point to UTF-8
 * 
 * @param codePoint Unicode code point
 * @param buffer Output buffer (must be at least 4 bytes)
 * @return Number of bytes written (1-4), or 0 on error
 */
Size encode(Char32 codePoint, char* buffer);

/**
 * @brief Get length of UTF-8 string in code points
 */
[[nodiscard]] Size length(StringView s);

/**
 * @brief Check if string is valid UTF-8
 */
[[nodiscard]] bool isValid(StringView s);

/**
 * @brief Get byte length of a UTF-8 code point from first byte
 */
[[nodiscard]] inline Size codePointLength(char firstByte) {
    unsigned char c = static_cast<unsigned char>(firstByte);
    if ((c & 0x80) == 0x00) return 1;
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 0; // Invalid
}

/**
 * @brief Advance pointer to next UTF-8 code point
 */
[[nodiscard]] inline const char* next(const char* str, const char* end) {
    if (str >= end) return str;
    Size len = codePointLength(*str);
    return (len > 0 && str + len <= end) ? str + len : str + 1;
}

/**
 * @brief Move pointer to previous UTF-8 code point
 */
[[nodiscard]] inline const char* prev(const char* str, const char* begin) {
    if (str <= begin) return str;
    --str;
    while (str > begin && (*str & 0xC0) == 0x80) {
        --str;
    }
    return str;
}

/**
 * @brief Iterator for UTF-8 strings
 */
class Iterator {
public:
    using value_type = Char32;
    using difference_type = PtrDiff;
    using pointer = const Char32*;
    using reference = Char32;
    using iterator_category = std::bidirectional_iterator_tag;
    
    Iterator() = default;
    Iterator(const char* pos, const char* begin, const char* end)
        : pos_(pos), begin_(begin), end_(end) {}
    
    [[nodiscard]] Char32 operator*() const {
        Char32 cp;
        decode(pos_, end_, cp);
        return cp;
    }
    
    Iterator& operator++() {
        pos_ = next(pos_, end_);
        return *this;
    }
    
    Iterator operator++(int) {
        Iterator tmp = *this;
        ++(*this);
        return tmp;
    }
    
    Iterator& operator--() {
        pos_ = prev(pos_, begin_);
        return *this;
    }
    
    Iterator operator--(int) {
        Iterator tmp = *this;
        --(*this);
        return tmp;
    }
    
    [[nodiscard]] bool operator==(const Iterator& other) const {
        return pos_ == other.pos_;
    }
    
    [[nodiscard]] bool operator!=(const Iterator& other) const {
        return pos_ != other.pos_;
    }
    
    [[nodiscard]] const char* ptr() const { return pos_; }

private:
    const char* pos_ = nullptr;
    const char* begin_ = nullptr;
    const char* end_ = nullptr;
};

/**
 * @brief Range for iterating UTF-8 code points
 */
class Range {
public:
    explicit Range(StringView s)
        : data_(s.data()), size_(s.size()) {}
    
    [[nodiscard]] Iterator begin() const {
        return Iterator(data_, data_, data_ + size_);
    }
    
    [[nodiscard]] Iterator end() const {
        return Iterator(data_ + size_, data_, data_ + size_);
    }

private:
    const char* data_;
    Size size_;
};

/**
 * @brief Create UTF-8 range for iteration
 */
[[nodiscard]] inline Range codePoints(StringView s) {
    return Range(s);
}

//=============================================================================
// Encoding Conversion
//=============================================================================

/**
 * @brief Convert UTF-16 (WString) to UTF-8
 */
[[nodiscard]] String fromUtf16(WStringView ws);

/**
 * @brief Convert UTF-8 to UTF-16
 */
[[nodiscard]] WString toUtf16(StringView s);

/**
 * @brief Convert Latin-1 (ISO-8859-1) to UTF-8
 */
[[nodiscard]] String fromLatin1(StringView s);

/**
 * @brief Convert UTF-8 to Latin-1 (lossy - non-Latin1 chars become '?')
 */
[[nodiscard]] String toLatin1(StringView s);

/**
 * @brief Convert from Quake's character encoding to UTF-8
 * 
 * Quake uses an extended ASCII encoding with special characters
 * in the 0x80-0xFF range for colored text and special symbols.
 */
[[nodiscard]] String fromQuake(StringView s);

/**
 * @brief Convert UTF-8 to Quake's character encoding
 */
[[nodiscard]] String toQuake(StringView s);

/**
 * @brief Convert single Quake character to Unicode code point
 */
[[nodiscard]] Char32 quakeCharToUnicode(Byte c);

/**
 * @brief Convert Unicode code point to Quake character
 */
[[nodiscard]] Byte unicodeToQuakeChar(Char32 cp);

} // namespace utf8
} // namespace text
} // namespace ezquake

#endif // EZQUAKE_CORE_TEXT_UTF8_HPP
