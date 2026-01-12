/*
 * ezQuake C++ Port - String Utilities
 * 
 * Modern C++ string handling with UTF-8 support.
 * Compatible with original Quake string functions.
 * 
 * Copyright (C) 1996-1997 Id Software, Inc.
 * Copyright (C) 2024 ezQuake Team
 */

#ifndef EZQUAKE_CORE_TEXT_STRING_UTILS_HPP
#define EZQUAKE_CORE_TEXT_STRING_UTILS_HPP

#include "../types.hpp"
#include <algorithm>
#include <cctype>

namespace ezquake {
namespace text {

/**
 * @brief Convert string to uppercase (in place)
 */
inline String& toUpper(String& s) {
    std::transform(s.begin(), s.end(), s.begin(), 
                   [](unsigned char c) { return std::toupper(c); });
    return s;
}

/**
 * @brief Convert string to lowercase (in place)
 */
inline String& toLower(String& s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}

/**
 * @brief Return uppercase copy
 */
[[nodiscard]] inline String toUpperCopy(StringView s) {
    String result(s);
    toUpper(result);
    return result;
}

/**
 * @brief Return lowercase copy
 */
[[nodiscard]] inline String toLowerCopy(StringView s) {
    String result(s);
    toLower(result);
    return result;
}

/**
 * @brief Case-insensitive string comparison
 */
[[nodiscard]] inline int compareIgnoreCase(StringView a, StringView b) {
    auto itA = a.begin();
    auto itB = b.begin();
    
    while (itA != a.end() && itB != b.end()) {
        int diff = std::tolower(static_cast<unsigned char>(*itA)) - 
                   std::tolower(static_cast<unsigned char>(*itB));
        if (diff != 0) return diff;
        ++itA;
        ++itB;
    }
    
    if (itA != a.end()) return 1;
    if (itB != b.end()) return -1;
    return 0;
}

/**
 * @brief Case-insensitive equality check
 */
[[nodiscard]] inline bool equalsIgnoreCase(StringView a, StringView b) {
    return compareIgnoreCase(a, b) == 0;
}

/**
 * @brief Case-insensitive substring search
 */
[[nodiscard]] StringView::size_type findIgnoreCase(StringView haystack, StringView needle);

/**
 * @brief Check if string contains substring (case-insensitive)
 */
[[nodiscard]] inline bool containsIgnoreCase(StringView haystack, StringView needle) {
    return findIgnoreCase(haystack, needle) != StringView::npos;
}

/**
 * @brief Trim whitespace from start
 */
[[nodiscard]] inline StringView trimLeft(StringView s) {
    auto it = std::find_if(s.begin(), s.end(), 
                           [](unsigned char c) { return !std::isspace(c); });
    return s.substr(static_cast<Size>(it - s.begin()));
}

/**
 * @brief Trim whitespace from end
 */
[[nodiscard]] inline StringView trimRight(StringView s) {
    auto it = std::find_if(s.rbegin(), s.rend(),
                           [](unsigned char c) { return !std::isspace(c); });
    return s.substr(0, static_cast<Size>(s.rend() - it));
}

/**
 * @brief Trim whitespace from both ends
 */
[[nodiscard]] inline StringView trim(StringView s) {
    return trimRight(trimLeft(s));
}

/**
 * @brief Split string by delimiter
 */
[[nodiscard]] Vector<StringView> split(StringView s, char delimiter);

/**
 * @brief Split string by any of the delimiter characters
 */
[[nodiscard]] Vector<StringView> splitAny(StringView s, StringView delimiters);

/**
 * @brief Join strings with delimiter
 */
template<typename Container>
[[nodiscard]] String join(const Container& parts, StringView delimiter) {
    String result;
    bool first = true;
    for (const auto& part : parts) {
        if (!first) {
            result.append(delimiter);
        }
        result.append(part);
        first = false;
    }
    return result;
}

/**
 * @brief Check if string starts with prefix
 */
[[nodiscard]] inline bool startsWith(StringView s, StringView prefix) {
    return s.size() >= prefix.size() && s.substr(0, prefix.size()) == prefix;
}

/**
 * @brief Check if string ends with suffix
 */
[[nodiscard]] inline bool endsWith(StringView s, StringView suffix) {
    return s.size() >= suffix.size() && 
           s.substr(s.size() - suffix.size()) == suffix;
}

/**
 * @brief Check if string starts with prefix (case-insensitive)
 */
[[nodiscard]] inline bool startsWithIgnoreCase(StringView s, StringView prefix) {
    return s.size() >= prefix.size() && 
           equalsIgnoreCase(s.substr(0, prefix.size()), prefix);
}

/**
 * @brief Check if string ends with suffix (case-insensitive)
 */
[[nodiscard]] inline bool endsWithIgnoreCase(StringView s, StringView suffix) {
    return s.size() >= suffix.size() &&
           equalsIgnoreCase(s.substr(s.size() - suffix.size()), suffix);
}

/**
 * @brief Replace all occurrences of 'from' with 'to'
 */
[[nodiscard]] String replace(StringView s, StringView from, StringView to);

/**
 * @brief Glob/wildcard pattern matching
 * 
 * Supports:
 *   * - matches any sequence of characters
 *   ? - matches any single character
 */
[[nodiscard]] bool globMatch(StringView pattern, StringView text);

/**
 * @brief Parse integer from string (Q_atoi replacement)
 */
[[nodiscard]] Int32 parseInt(StringView s);

/**
 * @brief Parse float from string (Q_atof replacement)
 */
[[nodiscard]] Float parseFloat(StringView s);

/**
 * @brief Format float without trailing zeros (Q_ftos replacement)
 */
[[nodiscard]] String formatFloat(Float value);

/**
 * @brief Compute hash of string (Com_HashKey replacement)
 */
[[nodiscard]] UInt32 hash(StringView s);

/**
 * @brief Safe string copy with size limit (strlcpy replacement)
 */
Size safeCopy(char* dst, const char* src, Size dstSize);

/**
 * @brief Safe string concatenation (strlcat replacement)
 */
Size safeConcat(char* dst, const char* src, Size dstSize);

} // namespace text
} // namespace ezquake

#endif // EZQUAKE_CORE_TEXT_STRING_UTILS_HPP
