/*
 * ezQuake C++ Port - UTF-8 Implementation
 * 
 * Copyright (C) 2024 ezQuake Team
 */

#include "utf8.hpp"
#include <array>

namespace ezquake {
namespace text {
namespace utf8 {

const char* decode(const char* str, const char* strEnd, Char32& outCodePoint) {
    if (str >= strEnd) {
        outCodePoint = 0;
        return nullptr;
    }
    
    unsigned char c = static_cast<unsigned char>(*str);
    
    if ((c & 0x80) == 0x00) {
        // 1-byte (ASCII)
        outCodePoint = c;
        return str + 1;
    }
    
    if ((c & 0xE0) == 0xC0) {
        // 2-byte
        if (str + 2 > strEnd) return nullptr;
        unsigned char c1 = static_cast<unsigned char>(str[1]);
        if ((c1 & 0xC0) != 0x80) return nullptr;
        outCodePoint = ((c & 0x1F) << 6) | (c1 & 0x3F);
        return str + 2;
    }
    
    if ((c & 0xF0) == 0xE0) {
        // 3-byte
        if (str + 3 > strEnd) return nullptr;
        unsigned char c1 = static_cast<unsigned char>(str[1]);
        unsigned char c2 = static_cast<unsigned char>(str[2]);
        if ((c1 & 0xC0) != 0x80 || (c2 & 0xC0) != 0x80) return nullptr;
        outCodePoint = ((c & 0x0F) << 12) | ((c1 & 0x3F) << 6) | (c2 & 0x3F);
        return str + 3;
    }
    
    if ((c & 0xF8) == 0xF0) {
        // 4-byte
        if (str + 4 > strEnd) return nullptr;
        unsigned char c1 = static_cast<unsigned char>(str[1]);
        unsigned char c2 = static_cast<unsigned char>(str[2]);
        unsigned char c3 = static_cast<unsigned char>(str[3]);
        if ((c1 & 0xC0) != 0x80 || (c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80) return nullptr;
        outCodePoint = ((c & 0x07) << 18) | ((c1 & 0x3F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
        return str + 4;
    }
    
    // Invalid
    return nullptr;
}

Size encode(Char32 codePoint, char* buffer) {
    if (codePoint < 0x80) {
        buffer[0] = static_cast<char>(codePoint);
        return 1;
    }
    
    if (codePoint < 0x800) {
        buffer[0] = static_cast<char>(0xC0 | (codePoint >> 6));
        buffer[1] = static_cast<char>(0x80 | (codePoint & 0x3F));
        return 2;
    }
    
    if (codePoint < 0x10000) {
        buffer[0] = static_cast<char>(0xE0 | (codePoint >> 12));
        buffer[1] = static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
        buffer[2] = static_cast<char>(0x80 | (codePoint & 0x3F));
        return 3;
    }
    
    if (codePoint < 0x110000) {
        buffer[0] = static_cast<char>(0xF0 | (codePoint >> 18));
        buffer[1] = static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F));
        buffer[2] = static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
        buffer[3] = static_cast<char>(0x80 | (codePoint & 0x3F));
        return 4;
    }
    
    return 0; // Invalid code point
}

Size length(StringView s) {
    Size count = 0;
    const char* ptr = s.data();
    const char* end = ptr + s.size();
    
    while (ptr < end) {
        Char32 cp;
        ptr = decode(ptr, end, cp);
        if (!ptr) break;
        ++count;
    }
    
    return count;
}

bool isValid(StringView s) {
    const char* ptr = s.data();
    const char* end = ptr + s.size();
    
    while (ptr < end) {
        Char32 cp;
        const char* next = decode(ptr, end, cp);
        if (!next) return false;
        ptr = next;
    }
    
    return true;
}

//=============================================================================
// Encoding Conversion
//=============================================================================

String fromUtf16(WStringView ws) {
    String result;
    result.reserve(ws.size() * 3); // Worst case: 3 bytes per char
    
    for (Size i = 0; i < ws.size(); ++i) {
        Char32 cp;
        WChar w = ws[i];
        
        // Handle surrogate pairs
        if (w >= 0xD800 && w <= 0xDBFF && i + 1 < ws.size()) {
            WChar w2 = ws[i + 1];
            if (w2 >= 0xDC00 && w2 <= 0xDFFF) {
                cp = 0x10000 + ((static_cast<Char32>(w - 0xD800) << 10) | (w2 - 0xDC00));
                ++i;
            } else {
                cp = w; // Invalid surrogate, use as-is
            }
        } else {
            cp = w;
        }
        
        char buf[4];
        Size len = encode(cp, buf);
        result.append(buf, len);
    }
    
    return result;
}

WString toUtf16(StringView s) {
    WString result;
    result.reserve(s.size()); // Approximate
    
    const char* ptr = s.data();
    const char* end = ptr + s.size();
    
    while (ptr < end) {
        Char32 cp;
        ptr = decode(ptr, end, cp);
        if (!ptr) break;
        
        if (cp < 0x10000) {
            result.push_back(static_cast<WChar>(cp));
        } else {
            // Encode as surrogate pair
            cp -= 0x10000;
            result.push_back(static_cast<WChar>(0xD800 + (cp >> 10)));
            result.push_back(static_cast<WChar>(0xDC00 + (cp & 0x3FF)));
        }
    }
    
    return result;
}

String fromLatin1(StringView s) {
    String result;
    result.reserve(s.size() * 2); // Worst case
    
    for (char c : s) {
        unsigned char uc = static_cast<unsigned char>(c);
        if (uc < 0x80) {
            result.push_back(c);
        } else {
            result.push_back(static_cast<char>(0xC0 | (uc >> 6)));
            result.push_back(static_cast<char>(0x80 | (uc & 0x3F)));
        }
    }
    
    return result;
}

String toLatin1(StringView s) {
    String result;
    result.reserve(s.size());
    
    const char* ptr = s.data();
    const char* end = ptr + s.size();
    
    while (ptr < end) {
        Char32 cp;
        ptr = decode(ptr, end, cp);
        if (!ptr) break;
        
        if (cp < 0x100) {
            result.push_back(static_cast<char>(cp));
        } else {
            result.push_back('?'); // Replacement character
        }
    }
    
    return result;
}

// Quake character mapping table (for special characters 0x80-0xFF)
// These map to various Unicode code points for Quake's special font chars
static const std::array<Char32, 128> quakeToUnicodeTable = {
    // 0x80-0x8F: Various special characters
    0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022,
    0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022,
    // 0x90-0x9F: More special chars
    0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022,
    0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022,
    // 0xA0-0xAF
    0x00A0, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022,
    0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022,
    // 0xB0-0xBF: Brown/gold colored chars (display as regular)
    '0' + 0xE000, '1' + 0xE000, '2' + 0xE000, '3' + 0xE000, 
    '4' + 0xE000, '5' + 0xE000, '6' + 0xE000, '7' + 0xE000,
    '8' + 0xE000, '9' + 0xE000, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022,
    // 0xC0-0xCF
    0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022,
    0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022,
    // 0xD0-0xDF
    0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022,
    0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022,
    // 0xE0-0xEF
    0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022,
    0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022,
    // 0xF0-0xFF
    0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022,
    0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022, 0x2022,
};

Char32 quakeCharToUnicode(Byte c) {
    if (c < 0x80) {
        return c;
    }
    return quakeToUnicodeTable[c - 0x80];
}

Byte unicodeToQuakeChar(Char32 cp) {
    if (cp < 0x80) {
        return static_cast<Byte>(cp);
    }
    
    // Check private use area for Quake-specific chars
    if (cp >= 0xE030 && cp <= 0xE039) {
        return static_cast<Byte>(0xB0 + (cp - 0xE030));
    }
    
    // For other Unicode chars, try to find best match or return '?'
    return '?';
}

String fromQuake(StringView s) {
    String result;
    result.reserve(s.size() * 2);
    
    for (char c : s) {
        Char32 cp = quakeCharToUnicode(static_cast<Byte>(c));
        char buf[4];
        Size len = encode(cp, buf);
        result.append(buf, len);
    }
    
    return result;
}

String toQuake(StringView s) {
    String result;
    result.reserve(s.size());
    
    const char* ptr = s.data();
    const char* end = ptr + s.size();
    
    while (ptr < end) {
        Char32 cp;
        ptr = decode(ptr, end, cp);
        if (!ptr) break;
        
        result.push_back(static_cast<char>(unicodeToQuakeChar(cp)));
    }
    
    return result;
}

} // namespace utf8
} // namespace text
} // namespace ezquake
