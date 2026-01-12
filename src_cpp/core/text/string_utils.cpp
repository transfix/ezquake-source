/*
 * ezQuake C++ Port - String Utilities Implementation
 * 
 * Copyright (C) 1996-1997 Id Software, Inc.
 * Copyright (C) 2024 ezQuake Team
 */

#include "string_utils.hpp"
#include <cstring>
#include <charconv>
#include <sstream>
#include <iomanip>

namespace ezquake {
namespace text {

StringView::size_type findIgnoreCase(StringView haystack, StringView needle) {
    if (needle.empty()) return 0;
    if (needle.size() > haystack.size()) return StringView::npos;
    
    for (Size i = 0; i <= haystack.size() - needle.size(); ++i) {
        if (equalsIgnoreCase(haystack.substr(i, needle.size()), needle)) {
            return i;
        }
    }
    return StringView::npos;
}

Vector<StringView> split(StringView s, char delimiter) {
    Vector<StringView> result;
    Size start = 0;
    Size end = 0;
    
    while ((end = s.find(delimiter, start)) != StringView::npos) {
        result.push_back(s.substr(start, end - start));
        start = end + 1;
    }
    result.push_back(s.substr(start));
    
    return result;
}

Vector<StringView> splitAny(StringView s, StringView delimiters) {
    Vector<StringView> result;
    Size start = 0;
    Size end = 0;
    
    while ((end = s.find_first_of(delimiters, start)) != StringView::npos) {
        if (end > start) {
            result.push_back(s.substr(start, end - start));
        }
        start = end + 1;
    }
    if (start < s.size()) {
        result.push_back(s.substr(start));
    }
    
    return result;
}

String replace(StringView s, StringView from, StringView to) {
    if (from.empty()) return String(s);
    
    String result;
    Size pos = 0;
    Size lastPos = 0;
    
    while ((pos = s.find(from, lastPos)) != StringView::npos) {
        result.append(s.substr(lastPos, pos - lastPos));
        result.append(to);
        lastPos = pos + from.size();
    }
    result.append(s.substr(lastPos));
    
    return result;
}

bool globMatch(StringView pattern, StringView text) {
    Size pi = 0, ti = 0;
    Size starIdx = StringView::npos;
    Size matchIdx = 0;
    
    while (ti < text.size()) {
        if (pi < pattern.size() && 
            (pattern[pi] == '?' || pattern[pi] == text[ti])) {
            ++pi;
            ++ti;
        } else if (pi < pattern.size() && pattern[pi] == '*') {
            starIdx = pi;
            matchIdx = ti;
            ++pi;
        } else if (starIdx != StringView::npos) {
            pi = starIdx + 1;
            ++matchIdx;
            ti = matchIdx;
        } else {
            return false;
        }
    }
    
    while (pi < pattern.size() && pattern[pi] == '*') {
        ++pi;
    }
    
    return pi == pattern.size();
}

Int32 parseInt(StringView s) {
    // Skip leading whitespace
    s = trimLeft(s);
    if (s.empty()) return 0;
    
    Int32 result = 0;
    Int32 sign = 1;
    Size i = 0;
    
    // Handle sign
    if (s[0] == '-') {
        sign = -1;
        ++i;
    } else if (s[0] == '+') {
        ++i;
    }
    
    // Handle hex prefix
    if (i + 1 < s.size() && s[i] == '0' && (s[i+1] == 'x' || s[i+1] == 'X')) {
        i += 2;
        while (i < s.size()) {
            char c = s[i];
            Int32 digit;
            if (c >= '0' && c <= '9') digit = c - '0';
            else if (c >= 'a' && c <= 'f') digit = c - 'a' + 10;
            else if (c >= 'A' && c <= 'F') digit = c - 'A' + 10;
            else break;
            result = result * 16 + digit;
            ++i;
        }
        return result * sign;
    }
    
    // Handle decimal
    while (i < s.size() && s[i] >= '0' && s[i] <= '9') {
        result = result * 10 + (s[i] - '0');
        ++i;
    }
    
    return result * sign;
}

Float parseFloat(StringView s) {
    s = trimLeft(s);
    if (s.empty()) return 0.0f;
    
    // Use std::from_chars for modern parsing
    Float result = 0.0f;
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), result);
    
    if (ec != std::errc()) {
        // Fallback to sscanf-style parsing for edge cases
        std::string str(s);
        std::istringstream iss(str);
        iss >> result;
    }
    
    return result;
}

String formatFloat(Float value) {
    std::ostringstream oss;
    oss << std::setprecision(6) << value;
    String result = oss.str();
    
    // Remove trailing zeros after decimal point
    if (result.find('.') != String::npos) {
        Size end = result.find_last_not_of('0');
        if (end != String::npos) {
            if (result[end] == '.') {
                result = result.substr(0, end);
            } else {
                result = result.substr(0, end + 1);
            }
        }
    }
    
    return result;
}

UInt32 hash(StringView s) {
    UInt32 hashval = 0;
    
    for (char c : s) {
        hashval = (hashval << 4) + static_cast<UInt32>(std::tolower(static_cast<unsigned char>(c)));
        UInt32 g = hashval & 0xf0000000;
        if (g) {
            hashval ^= (g >> 24);
        }
        hashval &= ~g;
    }
    
    return hashval;
}

Size safeCopy(char* dst, const char* src, Size dstSize) {
    if (dstSize == 0) return std::strlen(src);
    
    Size srcLen = std::strlen(src);
    Size copyLen = (srcLen < dstSize - 1) ? srcLen : dstSize - 1;
    
    std::memcpy(dst, src, copyLen);
    dst[copyLen] = '\0';
    
    return srcLen;
}

Size safeConcat(char* dst, const char* src, Size dstSize) {
    Size dstLen = std::strlen(dst);
    Size srcLen = std::strlen(src);
    
    if (dstLen >= dstSize) {
        return dstSize + srcLen;
    }
    
    Size copyLen = (dstLen + srcLen < dstSize - 1) ? srcLen : dstSize - dstLen - 1;
    std::memcpy(dst + dstLen, src, copyLen);
    dst[dstLen + copyLen] = '\0';
    
    return dstLen + srcLen;
}

} // namespace text
} // namespace ezquake
