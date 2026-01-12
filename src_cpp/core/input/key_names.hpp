/*
 * ezQuake C++ Port - Key Name Utilities
 * 
 * Convert between key codes and string names.
 * 
 * Copyright (C) 2024 ezQuake Team
 */

#pragma once

#include "core/input/input_types.hpp"
#include <string>
#include <string_view>
#include <optional>
#include <cctype>

namespace ezquake {
namespace input {

/**
 * @brief Key name entry for lookup tables.
 */
struct KeyName {
    std::string_view name;
    KeyCode code;
};

/**
 * @brief Table of key names for non-printable keys.
 */
inline constexpr KeyName KEY_NAMES[] = {
    {"TAB", KeyCode::Tab},
    {"ENTER", KeyCode::Enter},
    {"ESCAPE", KeyCode::Escape},
    {"SPACE", KeyCode::Space},
    {"BACKSPACE", KeyCode::Backspace},
    {"CAPSLOCK", KeyCode::CapsLock},
    {"PRINTSCREEN", KeyCode::PrintScreen},
    {"SCROLLLOCK", KeyCode::ScrollLock},
    {"PAUSE", KeyCode::Pause},
    
    {"UPARROW", KeyCode::UpArrow},
    {"DOWNARROW", KeyCode::DownArrow},
    {"LEFTARROW", KeyCode::LeftArrow},
    {"RIGHTARROW", KeyCode::RightArrow},
    
    {"ALT", KeyCode::Alt},
    {"LALT", KeyCode::LeftAlt},
    {"RALT", KeyCode::RightAlt},
    {"CTRL", KeyCode::Ctrl},
    {"LCTRL", KeyCode::LeftCtrl},
    {"RCTRL", KeyCode::RightCtrl},
    {"SHIFT", KeyCode::Shift},
    {"LSHIFT", KeyCode::LeftShift},
    {"RSHIFT", KeyCode::RightShift},
    
    {"F1", KeyCode::F1},
    {"F2", KeyCode::F2},
    {"F3", KeyCode::F3},
    {"F4", KeyCode::F4},
    {"F5", KeyCode::F5},
    {"F6", KeyCode::F6},
    {"F7", KeyCode::F7},
    {"F8", KeyCode::F8},
    {"F9", KeyCode::F9},
    {"F10", KeyCode::F10},
    {"F11", KeyCode::F11},
    {"F12", KeyCode::F12},
    
    {"INS", KeyCode::Insert},
    {"DEL", KeyCode::Delete},
    {"PGDN", KeyCode::PageDown},
    {"PGUP", KeyCode::PageUp},
    {"HOME", KeyCode::Home},
    {"END", KeyCode::End},
    
    {"WIN", KeyCode::Win},
    {"LWIN", KeyCode::LeftWin},
    {"RWIN", KeyCode::RightWin},
    {"MENU", KeyCode::Menu},
    
    {"KP_NUMLOCK", KeyCode::NumLock},
    {"KP_SLASH", KeyCode::NumpadSlash},
    {"KP_STAR", KeyCode::NumpadStar},
    {"KP_HOME", KeyCode::NumpadHome},
    {"KP_UPARROW", KeyCode::NumpadUp},
    {"KP_PGUP", KeyCode::NumpadPgUp},
    {"KP_MINUS", KeyCode::NumpadMinus},
    {"KP_LEFTARROW", KeyCode::NumpadLeft},
    {"KP_5", KeyCode::Numpad5},
    {"KP_RIGHTARROW", KeyCode::NumpadRight},
    {"KP_PLUS", KeyCode::NumpadPlus},
    {"KP_END", KeyCode::NumpadEnd},
    {"KP_DOWNARROW", KeyCode::NumpadDown},
    {"KP_PGDN", KeyCode::NumpadPgDn},
    {"KP_INS", KeyCode::NumpadIns},
    {"KP_DEL", KeyCode::NumpadDel},
    {"KP_ENTER", KeyCode::NumpadEnter},
    
    {"MOUSE1", KeyCode::Mouse1},
    {"MOUSE2", KeyCode::Mouse2},
    {"MOUSE3", KeyCode::Mouse3},
    {"MOUSE4", KeyCode::Mouse4},
    {"MOUSE5", KeyCode::Mouse5},
    {"MOUSE6", KeyCode::Mouse6},
    {"MOUSE7", KeyCode::Mouse7},
    {"MOUSE8", KeyCode::Mouse8},
    
    {"JOY1", KeyCode::Joy1},
    {"JOY2", KeyCode::Joy2},
    {"JOY3", KeyCode::Joy3},
    {"JOY4", KeyCode::Joy4},
    
    {"JOY_POVUP", KeyCode::JoyPovUp},
    {"JOY_POVRT", KeyCode::JoyPovRight},
    {"JOY_POVDN", KeyCode::JoyPovDown},
    {"JOY_POVLT", KeyCode::JoyPovLeft},
    
    {"AUX1", KeyCode::Aux1},
    {"AUX2", KeyCode::Aux2},
    {"AUX3", KeyCode::Aux3},
    {"AUX4", KeyCode::Aux4},
    {"AUX5", KeyCode::Aux5},
    {"AUX6", KeyCode::Aux6},
    {"AUX7", KeyCode::Aux7},
    {"AUX8", KeyCode::Aux8},
    {"AUX9", KeyCode::Aux9},
    {"AUX10", KeyCode::Aux10},
    {"AUX11", KeyCode::Aux11},
    {"AUX12", KeyCode::Aux12},
    {"AUX13", KeyCode::Aux13},
    {"AUX14", KeyCode::Aux14},
    {"AUX15", KeyCode::Aux15},
    {"AUX16", KeyCode::Aux16},
    {"AUX17", KeyCode::Aux17},
    {"AUX18", KeyCode::Aux18},
    {"AUX19", KeyCode::Aux19},
    {"AUX20", KeyCode::Aux20},
    {"AUX21", KeyCode::Aux21},
    {"AUX22", KeyCode::Aux22},
    {"AUX23", KeyCode::Aux23},
    {"AUX24", KeyCode::Aux24},
    {"AUX25", KeyCode::Aux25},
    {"AUX26", KeyCode::Aux26},
    {"AUX27", KeyCode::Aux27},
    {"AUX28", KeyCode::Aux28},
    
    {"MWHEELUP", KeyCode::MouseWheelUp},
    {"MWHEELDOWN", KeyCode::MouseWheelDown},
    
    {"SEMICOLON", static_cast<KeyCode>(';')},  // Special case
};

/**
 * @brief Case-insensitive string comparison.
 */
inline bool strEqualsIgnoreCase(std::string_view a, std::string_view b) noexcept {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(a[i])) != 
            std::tolower(static_cast<unsigned char>(b[i]))) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Convert a key name string to a key code.
 * 
 * @param name The key name (e.g., "MOUSE1", "a", "F5")
 * @return The key code, or std::nullopt if not found
 */
[[nodiscard]] inline std::optional<KeyCode> stringToKeyCode(std::string_view name) noexcept {
    if (name.empty()) {
        return std::nullopt;
    }
    
    // Check special key names first
    for (const auto& entry : KEY_NAMES) {
        if (strEqualsIgnoreCase(name, entry.name)) {
            return entry.code;
        }
    }
    
    // Single character - treat as ASCII
    if (name.size() == 1) {
        char c = static_cast<char>(std::tolower(static_cast<unsigned char>(name[0])));
        if (c >= 'a' && c <= 'z') {
            return static_cast<KeyCode>(c);
        }
        if ((c >= '0' && c <= '9') || c == '-' || c == '=' || c == '[' || 
            c == ']' || c == '\\' || c == '\'' || c == ',' || c == '.' || 
            c == '/' || c == '`') {
            return static_cast<KeyCode>(c);
        }
    }
    
    // Try parsing as a number (for raw keycodes)
    if (name.size() > 0 && (std::isdigit(static_cast<unsigned char>(name[0])) || name[0] == '-')) {
        try {
            int val = 0;
            std::size_t idx = 0;
            bool negative = false;
            
            if (name[0] == '-') {
                negative = true;
                idx = 1;
            }
            
            for (; idx < name.size(); ++idx) {
                if (!std::isdigit(static_cast<unsigned char>(name[idx]))) {
                    return std::nullopt;
                }
                val = val * 10 + (name[idx] - '0');
            }
            
            if (negative) val = -val;
            
            if (val >= 0 && val < static_cast<int>(KeyCode::MaxKeys)) {
                return static_cast<KeyCode>(val);
            }
        } catch (...) {
            // Fall through
        }
    }
    
    return std::nullopt;
}

/**
 * @brief Convert a key code to its string name.
 * 
 * @param key The key code
 * @return The key name string
 */
[[nodiscard]] inline std::string keyCodeToString(KeyCode key) {
    int keyNum = static_cast<int>(key);
    
    // Check for special key names
    for (const auto& entry : KEY_NAMES) {
        if (entry.code == key) {
            return std::string(entry.name);
        }
    }
    
    // Printable ASCII character
    if (keyNum >= 32 && keyNum < 127) {
        char c = static_cast<char>(keyNum);
        // Uppercase for display
        if (c >= 'a' && c <= 'z') {
            c = static_cast<char>(c - 'a' + 'A');
        }
        return std::string(1, c);
    }
    
    // Unknown key - return as number
    return "<KEY" + std::to_string(keyNum) + ">";
}

/**
 * @brief Convert a key code to a short display name.
 * 
 * Returns a shorter name suitable for display in bind lists.
 */
[[nodiscard]] inline std::string keyCodeToShortName(KeyCode key) {
    // Some special short names
    switch (key) {
        case KeyCode::LeftArrow:  return "LEFT";
        case KeyCode::RightArrow: return "RIGHT";
        case KeyCode::UpArrow:    return "UP";
        case KeyCode::DownArrow:  return "DOWN";
        case KeyCode::Backspace:  return "BKSP";
        case KeyCode::Insert:     return "INS";
        case KeyCode::Delete:     return "DEL";
        case KeyCode::PageUp:     return "PGUP";
        case KeyCode::PageDown:   return "PGDN";
        case KeyCode::MouseWheelUp:   return "MWUP";
        case KeyCode::MouseWheelDown: return "MWDN";
        default: break;
    }
    
    // Use standard name
    return keyCodeToString(key);
}

/**
 * @brief Check if a key is bindable.
 */
[[nodiscard]] inline bool isBindableKey(KeyCode key) noexcept {
    int k = static_cast<int>(key);
    
    // Can't bind negative or out-of-range keys
    if (k < 0 || k >= static_cast<int>(KeyCode::MaxKeys)) {
        return false;
    }
    
    // Can't bind escape
    if (key == KeyCode::Escape) {
        return false;
    }
    
    return true;
}

} // namespace input
} // namespace ezquake
