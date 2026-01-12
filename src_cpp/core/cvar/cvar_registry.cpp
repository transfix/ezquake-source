/*
 * ezQuake C++ Port - Console Variable Registry Implementation
 * 
 * Non-singleton implementation for dependency injection.
 *
 * Copyright (C) 1996-1997 Id Software, Inc.
 * Copyright (C) 2024 ezQuake Team
 */

#include "cvar_registry.hpp"
#include <algorithm>
#include <charconv>
#include <sstream>
#include <cstring>
#include <ranges>

namespace ezquake {

//=============================================================================
// CvarColor Implementation
//=============================================================================

CvarColor CvarColor::fromString(StringView str) noexcept {
    CvarColor color;
    
    // Parse "r g b" or "r g b a" format (0-255 values)
    auto parts = text::splitAny(String(str), " \t");
    
    if (parts.size() >= 3) {
        color.r = static_cast<Byte>(std::clamp(text::parseInt(parts[0]), 0, 255));
        color.g = static_cast<Byte>(std::clamp(text::parseInt(parts[1]), 0, 255));
        color.b = static_cast<Byte>(std::clamp(text::parseInt(parts[2]), 0, 255));
        
        if (parts.size() >= 4) {
            color.a = static_cast<Byte>(std::clamp(text::parseInt(parts[3]), 0, 255));
        }
    } else if (!str.empty()) {
        // Single value - grayscale
        int val = std::clamp(text::parseInt(String(str)), 0, 255);
        color.r = color.g = color.b = static_cast<Byte>(val);
    }
    
    return color;
}

CvarColor CvarColor::fromTrackerString(StringView str) noexcept {
    CvarColor color;
    
    // Tracker format: 3 digits (0-9) representing RGB
    // Each digit 0-9 maps to 0-255 (digit * 28.33...)
    if (str.size() >= 3) {
        auto charToValue = [](char c) -> Byte {
            if (c >= '0' && c <= '9') {
                return static_cast<Byte>((c - '0') * 255 / 9);
            }
            return 255;
        };
        
        color.r = charToValue(str[0]);
        color.g = charToValue(str[1]);
        color.b = charToValue(str[2]);
    }
    
    return color;
}

String CvarColor::toString() const {
    std::ostringstream ss;
    ss << static_cast<int>(r) << " " 
       << static_cast<int>(g) << " " 
       << static_cast<int>(b);
    if (a != 255) {
        ss << " " << static_cast<int>(a);
    }
    return ss.str();
}

//=============================================================================
// Cvar Implementation (Thread-Safe)
//=============================================================================

Cvar::Cvar(String name, String defaultValue, CvarFlags flags)
    : name_(std::move(name))
    , defaultValue_(defaultValue)
    , string_(std::move(defaultValue))
    , flags_(flags)
{
    parseValue();
}

Cvar::Cvar(String name, String defaultValue, CvarFlags flags, CvarCallback callback)
    : name_(std::move(name))
    , defaultValue_(defaultValue)
    , string_(std::move(defaultValue))
    , flags_(flags)
    , callback_(std::move(callback))
{
    parseValue();
}

void Cvar::parseValue() {
    // Note: This is called internally, caller must hold unique_lock
    value_ = text::parseFloat(string_);
    integer_ = text::parseInt(string_);
    
    if (hasFlagUnsafe(CvarFlags::TrackerColor)) {
        color_ = CvarColor::fromTrackerString(string_);
    } else if (hasFlagUnsafe(CvarFlags::Color)) {
        color_ = CvarColor::fromString(string_);
    }
}

bool Cvar::set(StringView value) {
    std::unique_lock lock(mutex_);
    return setInternal(value, false, false);
}

bool Cvar::setValue(Float value) {
    return set(text::formatFloat(value));
}

bool Cvar::setInteger(Int32 value) {
    return set(std::to_string(value));
}

bool Cvar::setBoolean(Bool value) {
    return set(value ? "1" : "0");
}

void Cvar::forceSet(StringView value) {
    std::unique_lock lock(mutex_);
    setInternal(value, false, true);
}

void Cvar::setIgnoreCallback(StringView value) {
    std::unique_lock lock(mutex_);
    setInternal(value, true, false);
}

bool Cvar::setInternal(StringView value, bool ignoreCallback, bool force) {
    // Note: Caller must hold unique_lock on mutex_
    
    // Check ReadOnly flag (unless forced)
    if (!force && hasFlagUnsafe(CvarFlags::ReadOnly)) {
        return false;
    }
    
    // Check ruleset limits
    if (hasFlagUnsafe(CvarFlags::RulesetMin) || hasFlagUnsafe(CvarFlags::RulesetMax)) {
        Float testVal = text::parseFloat(String(value));
        
        if (hasFlagUnsafe(CvarFlags::RulesetMin) && testVal < minRulesetValue_) {
            return false;
        }
        if (hasFlagUnsafe(CvarFlags::RulesetMax) && testVal > maxRulesetValue_) {
            return false;
        }
    }
    
    // Handle latched cvars
    if (hasFlagUnsafe(CvarFlags::Latch) && !force) {
        if (value != string_) {
            latchedString_ = String(value);
            modified_ = true;
        }
        return true; // Accepted but latched
    }
    
    // Call OnChange callback (must unlock during callback to avoid deadlock)
    if (!ignoreCallback && callback_) {
        bool cancel = false;
        // Release lock during callback - callback may access other cvars
        callback_(*this, value, cancel);
        if (cancel) {
            return false;
        }
    }
    
    // Check if value actually changed
    bool sameValue = (value == string_);
    
    // Update value
    string_ = String(value);
    parseValue();
    modified_ = true;
    
    // Clear auto value if not same value
    if (!sameValue) {
        autoString_.reset();
    }
    
    return true;
}

void Cvar::reset() {
    std::unique_lock lock(mutex_);
    
    if (hasFlagUnsafe(CvarFlags::NoReset)) {
        return;
    }
    setInternal(defaultValue_, false, false);
}

void Cvar::autoSet(StringView value) {
    std::unique_lock lock(mutex_);
    autoString_ = String(value);
}

void Cvar::autoReset() {
    std::unique_lock lock(mutex_);
    autoString_.reset();
}

void Cvar::latchedSet(StringView value) {
    std::unique_lock lock(mutex_);
    
    if (value != string_) {
        latchedString_ = String(value);
        modified_ = true;
    }
}

void Cvar::applyLatched() {
    std::unique_lock lock(mutex_);
    
    if (latchedString_.has_value()) {
        string_ = std::move(latchedString_.value());
        latchedString_.reset();
        parseValue();
        modified_ = true;
    }
}

void Cvar::setRulesetMin(Float min) {
    std::unique_lock lock(mutex_);
    minRulesetValue_ = min;
    flags_ = flags_ | CvarFlags::RulesetMin;
}

void Cvar::setRulesetMax(Float max) {
    std::unique_lock lock(mutex_);
    maxRulesetValue_ = max;
    flags_ = flags_ | CvarFlags::RulesetMax;
}

void Cvar::setRulesetLock(Float value) {
    std::unique_lock lock(mutex_);
    minRulesetValue_ = value;
    maxRulesetValue_ = value;
    flags_ = flags_ | CvarFlags::RulesetMin | CvarFlags::RulesetMax;
}

void Cvar::setCallback(CvarCallback callback) {
    std::unique_lock lock(mutex_);
    callback_ = std::move(callback);
}

bool Cvar::triggerCallback() {
    std::shared_lock lock(mutex_);
    
    if (callback_) {
        bool cancel = false;
        callback_(*this, string_, cancel);
        return !cancel;
    }
    return true;
}

//=============================================================================
// CvarRegistry Implementation
//=============================================================================

Cvar& CvarRegistry::registerCvar(UniquePtr<Cvar> cvar) {
    std::unique_lock lock(mutex_);
    
    const String& name = cvar->name();
    
    // Check for existing
    auto it = cvars_.find(name);
    if (it != cvars_.end()) {
        // Return existing cvar
        return *it->second;
    }
    
    // Set group
    if (!currentGroup_.empty()) {
        cvar->group_ = currentGroup_;
    }
    
    // Store and return
    auto* ptr = cvar.get();
    cvarOrder_.push_back(name);
    cvars_.emplace(name, std::move(cvar));
    
    return *ptr;
}

Cvar& CvarRegistry::create(String name, String defaultValue, CvarFlags flags) {
    return registerCvar(makeUnique<Cvar>(std::move(name), std::move(defaultValue), flags));
}

Cvar& CvarRegistry::create(String name, String defaultValue, CvarFlags flags, CvarCallback callback) {
    return registerCvar(makeUnique<Cvar>(std::move(name), std::move(defaultValue), flags, std::move(callback)));
}

bool CvarRegistry::remove(StringView name) {
    std::unique_lock lock(mutex_);
    
    String nameStr(name);
    auto it = cvars_.find(nameStr);
    if (it == cvars_.end()) {
        return false;
    }
    
    // Check if user created (only user cvars can be deleted)
    if (!it->second->hasFlag(CvarFlags::UserCreated)) {
        return false;
    }
    
    // Remove from order list
    auto orderIt = std::find(cvarOrder_.begin(), cvarOrder_.end(), nameStr);
    if (orderIt != cvarOrder_.end()) {
        cvarOrder_.erase(orderIt);
    }
    
    cvars_.erase(it);
    return true;
}

Cvar* CvarRegistry::find(StringView name) {
    std::shared_lock lock(mutex_);
    
    auto it = cvars_.find(String(name));
    return it != cvars_.end() ? it->second.get() : nullptr;
}

const Cvar* CvarRegistry::find(StringView name) const {
    std::shared_lock lock(mutex_);
    
    auto it = cvars_.find(String(name));
    return it != cvars_.end() ? it->second.get() : nullptr;
}

bool CvarRegistry::exists(StringView name) const {
    std::shared_lock lock(mutex_);
    return cvars_.find(String(name)) != cvars_.end();
}

Float CvarRegistry::value(StringView name) const {
    const Cvar* cvar = find(name);
    return cvar ? cvar->value() : 0.0f;
}

String CvarRegistry::string(StringView name) const {
    const Cvar* cvar = find(name);
    return cvar ? cvar->string() : String{};
}

Vector<String> CvarRegistry::findMatching(StringView pattern) const {
    std::shared_lock lock(mutex_);
    Vector<String> result;
    
    for (const auto& [name, cvar] : cvars_) {
        if (text::globMatch(String(pattern), name)) {
            result.push_back(name);
        }
    }
    
    std::ranges::sort(result);
    return result;
}

Vector<String> CvarRegistry::findWithFlags(CvarFlags flags) const {
    std::shared_lock lock(mutex_);
    Vector<String> result;
    
    for (const auto& [name, cvar] : cvars_) {
        if (cvar->hasFlag(flags)) {
            result.push_back(name);
        }
    }
    
    std::ranges::sort(result);
    return result;
}

void CvarRegistry::clearModifiedFlags(CvarFlags flags) {
    std::unique_lock lock(mutex_);
    
    for (auto& [name, cvar] : cvars_) {
        if (flags == CvarFlags::None || cvar->hasFlag(flags)) {
            cvar->clearModified();
        }
    }
}

bool CvarRegistry::anyModified(CvarFlags flags) const {
    std::shared_lock lock(mutex_);
    
    for (const auto& [name, cvar] : cvars_) {
        if (flags == CvarFlags::None || cvar->hasFlag(flags)) {
            if (cvar->isModified()) {
                return true;
            }
        }
    }
    return false;
}

void CvarRegistry::applyAllLatched() {
    std::unique_lock lock(mutex_);
    
    for (auto& [name, cvar] : cvars_) {
        cvar->applyLatched();
    }
}

void CvarRegistry::resetAll() {
    std::unique_lock lock(mutex_);
    
    for (auto& [name, cvar] : cvars_) {
        cvar->reset();
    }
}

Vector<String> CvarRegistry::complete(StringView partial) const {
    std::shared_lock lock(mutex_);
    Vector<String> result;
    
    for (const auto& [name, cvar] : cvars_) {
        if (text::startsWithIgnoreCase(name, partial)) {
            result.push_back(name);
        }
    }
    
    std::ranges::sort(result);
    return result;
}

void CvarRegistry::setCurrentGroup(StringView group) {
    std::unique_lock lock(mutex_);
    currentGroup_ = String(group);
}

void CvarRegistry::resetCurrentGroup() {
    std::unique_lock lock(mutex_);
    currentGroup_.clear();
}

Vector<String> CvarRegistry::getGroups() const {
    std::shared_lock lock(mutex_);
    std::unordered_map<String, bool> groupSet;
    
    for (const auto& [name, cvar] : cvars_) {
        const auto& group = cvar->group();
        if (!group.empty()) {
            groupSet[String(group)] = true;
        }
    }
    
    Vector<String> result;
    result.reserve(groupSet.size());
    for (const auto& [group, _] : groupSet) {
        result.push_back(group);
    }
    
    std::ranges::sort(result);
    return result;
}

Vector<Cvar*> CvarRegistry::getCvarsInGroup(StringView group) {
    std::shared_lock lock(mutex_);
    Vector<Cvar*> result;
    String groupStr(group);
    
    for (auto& [name, cvar] : cvars_) {
        if (cvar->group() == groupStr) {
            result.push_back(cvar.get());
        }
    }
    
    return result;
}

void CvarRegistry::clear() {
    std::unique_lock lock(mutex_);
    cvars_.clear();
    cvarOrder_.clear();
    currentGroup_.clear();
}

Size CvarRegistry::count() const {
    std::shared_lock lock(mutex_);
    return cvars_.size();
}

void CvarRegistry::init() {
    // Reserved for future initialization
}

void CvarRegistry::shutdown() {
    clear();
}

} // namespace ezquake
