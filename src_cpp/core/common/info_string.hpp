// info_string.hpp - InfoString key-value parsing for userinfo/serverinfo
// Part of the ezQuake C++ codebase (Iteration 8)
//
// Original C: common.c - Info_ValueForKey, Info_SetValueForKey, etc.
// The original C implementation used static buffers and manual parsing.
// This C++ version uses std::unordered_map for clean key-value access.

#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <optional>
#include <vector>

namespace ezquake {

// Maximum key/value length in Quake info strings
inline constexpr std::size_t MaxInfoKey = 64;
inline constexpr std::size_t MaxInfoValue = 256;
inline constexpr std::size_t MaxInfoString = 1024;

/// InfoString parses and manipulates Quake-style info strings.
/// Format: "\key1\value1\key2\value2\key3\value3"
/// Used for userinfo (player settings) and serverinfo (server settings).
///
/// Example:
///   InfoString info("\\name\\player\\team\\red\\topcolor\\4");
///   auto name = info.get("name"); // returns "player"
///   info.set("name", "newname");
///   std::string s = info.toString(); // "\\name\\newname\\team\\red\\topcolor\\4"
class InfoString {
public:
    InfoString() = default;
    
    /// Parse an info string (with or without leading backslash)
    explicit InfoString(std::string_view infoStr);
    
    /// Get a value by key, returns empty optional if not found
    [[nodiscard]] std::optional<std::string_view> get(std::string_view key) const;
    
    /// Get a value by key with default fallback
    [[nodiscard]] std::string_view getOr(std::string_view key, std::string_view defaultValue) const;
    
    /// Set a key-value pair, returns false if key/value contains illegal chars
    /// Keys starting with '*' are "star keys" (server-controlled), normal set rejects them
    bool set(std::string_view key, std::string_view value);
    
    /// Set a star key (allows keys starting with '*')
    bool setStarKey(std::string_view key, std::string_view value);
    
    /// Remove a key
    void remove(std::string_view key);
    
    /// Remove all keys with a given prefix (e.g., '*' for star keys)
    void removeWithPrefix(char prefix);
    
    /// Check if key exists
    [[nodiscard]] bool contains(std::string_view key) const;
    
    /// Get number of key-value pairs
    [[nodiscard]] std::size_t size() const noexcept { return data_.size(); }
    
    /// Check if empty
    [[nodiscard]] bool empty() const noexcept { return data_.empty(); }
    
    /// Clear all key-value pairs
    void clear() noexcept { data_.clear(); }
    
    /// Serialize back to Quake info string format
    [[nodiscard]] std::string toString() const;
    
    /// Get the maximum size this info string can be when serialized
    [[nodiscard]] std::size_t maxSize() const noexcept { return maxSize_; }
    
    /// Set the maximum size constraint (default: MaxInfoString)
    void setMaxSize(std::size_t maxSize) noexcept { maxSize_ = maxSize; }
    
    /// Get all keys
    [[nodiscard]] std::vector<std::string_view> keys() const;
    
    /// Iterate over key-value pairs
    template<typename Func>
    void forEach(Func&& func) const {
        for (const auto& [key, value] : data_) {
            func(std::string_view{key}, std::string_view{value});
        }
    }
    
    /// Print info string in formatted form (for debugging)
    [[nodiscard]] std::string prettyPrint() const;

private:
    // Parse the input string
    void parse(std::string_view input);
    
    // Validate key/value (no backslashes, quotes, control chars)
    [[nodiscard]] static bool isValidKey(std::string_view key);
    [[nodiscard]] static bool isValidValue(std::string_view value);
    
    // Internal set without star key check
    bool setInternal(std::string_view key, std::string_view value);
    
    // Key-value storage (using string for ownership)
    std::unordered_map<std::string, std::string> data_;
    
    // Maximum serialized size
    std::size_t maxSize_ = MaxInfoString;
};

} // namespace ezquake
