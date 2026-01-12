// command_line.hpp - Command line argument parsing
// Part of the ezQuake C++ codebase (Iteration 8)
//
// Original C: common.c - COM_InitArgv, COM_Argc, COM_Argv, COM_CheckParm
// The original C implementation used global arrays and linear search.
// This C++ version provides a clean class-based interface with O(1) lookups.

#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <optional>
#include <span>

namespace ezquake {

/// CommandLine manages command-line argument parsing and access.
/// 
/// Quake-style command line format:
///   - Regular args: -basedir /path/to/base
///   - Commands with +: +connect server.example.com
///   - QW URLs: qw://server.example.com:27500
///
/// Example:
///   CommandLine cmdline(argc, argv);
///   if (cmdline.hasParam("-window")) { ... }
///   auto basedir = cmdline.getParamValue("-basedir");
class CommandLine {
public:
    CommandLine() = default;
    
    /// Initialize from main() arguments
    CommandLine(int argc, char** argv);
    
    /// Initialize from a span of string views
    explicit CommandLine(std::span<const std::string_view> args);
    
    /// Initialize from a vector of strings
    explicit CommandLine(std::vector<std::string> args);
    
    /// Parse arguments (can be called to re-parse)
    void parse(int argc, char** argv);
    void parse(std::span<const std::string_view> args);
    
    /// Get number of arguments
    [[nodiscard]] int argc() const noexcept { return static_cast<int>(args_.size()); }
    
    /// Get argument at index (returns empty string if out of bounds)
    [[nodiscard]] std::string_view argv(int index) const;
    
    /// Get all arguments
    [[nodiscard]] std::span<const std::string> args() const noexcept { return args_; }
    
    /// Check if a parameter exists (returns index, 0 if not found)
    /// Returns the 1-based index like the original C code for compatibility
    [[nodiscard]] int checkParam(std::string_view param) const;
    
    /// Check if a parameter exists (returns true/false)
    [[nodiscard]] bool hasParam(std::string_view param) const;
    
    /// Get the value following a parameter (e.g., -basedir /path -> "/path")
    [[nodiscard]] std::optional<std::string_view> getParamValue(std::string_view param) const;
    
    /// Get the value following a parameter with default
    [[nodiscard]] std::string_view getParamValueOr(std::string_view param, 
                                                    std::string_view defaultValue) const;
    
    /// Get multiple values following a parameter (useful for -game dir1 dir2)
    [[nodiscard]] std::vector<std::string_view> getParamValues(std::string_view param, 
                                                                int count) const;
    
    /// Clear an argument (set it to empty string, like COM_ClearArgv)
    void clearArgv(int index);
    
    /// Check if this looks like a QW URL (qw://...)
    [[nodiscard]] bool hasQwUrl() const;
    
    /// Get the QW URL if present
    [[nodiscard]] std::optional<std::string_view> getQwUrl() const;
    
    /// Get the original command line as a single string
    [[nodiscard]] const std::string& originalCmdline() const noexcept { return originalCmdline_; }
    
    /// Check if empty
    [[nodiscard]] bool empty() const noexcept { return args_.empty(); }

private:
    // Build index for fast parameter lookup
    void buildIndex();
    
    // Stored arguments (owning)
    std::vector<std::string> args_;
    
    // Index from parameter name to position (for O(1) lookup)
    std::unordered_map<std::string, int> paramIndex_;
    
    // Original command line string
    std::string originalCmdline_;
    
    // Cached QW URL index (-1 if none)
    int qwUrlIndex_ = -1;
};

/// va() style formatting function - returns a reference to an internal static buffer
/// This provides compatibility with the C va() function behavior.
/// Note: Uses a rotating buffer of 32 strings, so don't hold more than 32 at once.
///
/// Prefer std::format or fmt::format for new code!
[[nodiscard]] const char* va(const char* format, ...);

/// va() alternative that returns a std::string (safer, no static buffer issues)
template<typename... Args>
[[nodiscard]] std::string format(std::string_view fmt, Args&&... args);

} // namespace ezquake
