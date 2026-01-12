// info_string.cpp - InfoString implementation
// Part of the ezQuake C++ codebase (Iteration 8)

#include "info_string.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace ezquake {

InfoString::InfoString(std::string_view infoStr) {
    parse(infoStr);
}

void InfoString::parse(std::string_view input) {
    data_.clear();
    
    if (input.empty()) {
        return;
    }
    
    // Skip leading backslash if present
    std::size_t pos = 0;
    if (input[0] == '\\') {
        pos = 1;
    }
    
    while (pos < input.size()) {
        // Find key
        std::size_t keyStart = pos;
        while (pos < input.size() && input[pos] != '\\') {
            ++pos;
        }
        
        if (keyStart == pos) {
            // Empty key, skip
            if (pos < input.size()) ++pos;
            continue;
        }
        
        std::string key{input.substr(keyStart, pos - keyStart)};
        
        if (pos >= input.size()) {
            // Key without value - skip
            break;
        }
        
        ++pos; // Skip backslash separator
        
        // Find value
        std::size_t valueStart = pos;
        while (pos < input.size() && input[pos] != '\\') {
            ++pos;
        }
        
        std::string value{input.substr(valueStart, pos - valueStart)};
        
        // Store key-value pair
        data_[std::move(key)] = std::move(value);
        
        // Skip backslash if present
        if (pos < input.size() && input[pos] == '\\') {
            ++pos;
        }
    }
}

std::optional<std::string_view> InfoString::get(std::string_view key) const {
    auto it = data_.find(std::string{key});
    if (it != data_.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::string_view InfoString::getOr(std::string_view key, std::string_view defaultValue) const {
    auto it = data_.find(std::string{key});
    if (it != data_.end()) {
        return it->second;
    }
    return defaultValue;
}

bool InfoString::isValidKey(std::string_view key) {
    if (key.empty() || key.size() >= MaxInfoKey) {
        return false;
    }
    
    for (char c : key) {
        if (c == '\\' || c == '\"' || c <= 13) {
            return false;
        }
    }
    
    return true;
}

bool InfoString::isValidValue(std::string_view value) {
    if (value.size() >= MaxInfoValue) {
        return false;
    }
    
    for (char c : value) {
        if (c == '\\' || c == '\"' || (c > 0 && c <= 13)) {
            return false;
        }
    }
    
    return true;
}

bool InfoString::setInternal(std::string_view key, std::string_view value) {
    if (!isValidKey(key) || !isValidValue(value)) {
        return false;
    }
    
    // Check if adding this would exceed maxSize
    std::size_t newEntrySize = 1 + key.size() + 1 + value.size(); // \key\value
    
    // Calculate current size without this key
    std::size_t currentSize = 0;
    for (const auto& [k, v] : data_) {
        if (k != key) {
            currentSize += 1 + k.size() + 1 + v.size();
        }
    }
    
    if (currentSize + newEntrySize > maxSize_) {
        return false;
    }
    
    if (value.empty()) {
        // Setting to empty value removes the key
        data_.erase(std::string{key});
    } else {
        data_[std::string{key}] = std::string{value};
    }
    
    return true;
}

bool InfoString::set(std::string_view key, std::string_view value) {
    // Reject star keys in normal set
    if (!key.empty() && key[0] == '*') {
        return false;
    }
    
    return setInternal(key, value);
}

bool InfoString::setStarKey(std::string_view key, std::string_view value) {
    return setInternal(key, value);
}

void InfoString::remove(std::string_view key) {
    data_.erase(std::string{key});
}

void InfoString::removeWithPrefix(char prefix) {
    for (auto it = data_.begin(); it != data_.end(); ) {
        if (!it->first.empty() && it->first[0] == prefix) {
            it = data_.erase(it);
        } else {
            ++it;
        }
    }
}

bool InfoString::contains(std::string_view key) const {
    return data_.find(std::string{key}) != data_.end();
}

std::string InfoString::toString() const {
    std::string result;
    result.reserve(maxSize_);
    
    for (const auto& [key, value] : data_) {
        result += '\\';
        result += key;
        result += '\\';
        result += value;
    }
    
    return result;
}

std::vector<std::string_view> InfoString::keys() const {
    std::vector<std::string_view> result;
    result.reserve(data_.size());
    
    for (const auto& [key, value] : data_) {
        result.push_back(key);
    }
    
    return result;
}

std::string InfoString::prettyPrint() const {
    std::ostringstream oss;
    
    // Find longest key for alignment
    std::size_t maxKeyLen = 0;
    for (const auto& [key, value] : data_) {
        maxKeyLen = std::max(maxKeyLen, key.size());
    }
    
    constexpr std::size_t columnWidth = 20;
    std::size_t width = std::max(maxKeyLen, columnWidth);
    
    for (const auto& [key, value] : data_) {
        oss << std::left << std::setw(static_cast<int>(width)) << key << value << '\n';
    }
    
    return oss.str();
}

} // namespace ezquake
