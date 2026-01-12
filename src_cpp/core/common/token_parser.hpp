// token_parser.hpp - Token parsing utilities for Quake config/script files
// Part of the ezQuake C++ codebase (Iteration 8)
//
// Original C: common.c - COM_Parse, COM_ParseToken, COM_ParseEx
// The original C implementation used a global com_token buffer.
// This C++ version returns tokens as strings with proper ownership.

#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <vector>

namespace ezquake::text {

/// Token types returned by parseToken
enum class TokenType {
    Unknown,        ///< No token (end of input)
    String,         ///< Regular word or quoted string
    Comment,        ///< Comment (skipped by default)
    Newline,        ///< Newline token (if preserveNewlines=true)
    Punctuation,    ///< Single punctuation character
    Number          ///< Numeric value
};

/// Result of parsing a single token
struct ParseResult {
    std::string token;          ///< The parsed token
    TokenType type;             ///< Token type
    std::size_t consumedBytes;  ///< Number of bytes consumed from input
    
    /// Check if parse was successful
    [[nodiscard]] bool valid() const noexcept {
        return type != TokenType::Unknown;
    }
    
    /// Check if at end of input
    [[nodiscard]] bool endOfInput() const noexcept {
        return type == TokenType::Unknown && consumedBytes == 0;
    }
    
    explicit operator bool() const noexcept { return valid(); }
};

/// Options for token parsing
struct ParseOptions {
    bool handleQuotes = true;           ///< Treat "..." as single token
    bool handleCurlyBraces = false;     ///< Treat {...} as single token
    bool skipComments = true;           ///< Skip // and /* */ comments
    bool preserveNewlines = false;      ///< Treat newlines as tokens
    std::string_view punctuation = "";  ///< Characters to treat as single-char tokens
    
    /// Default options matching COM_Parse
    static constexpr ParseOptions quakeDefault() {
        return ParseOptions{};
    }
    
    /// Options matching COM_ParseToken
    static constexpr ParseOptions withPunctuation() {
        return ParseOptions{
            .handleQuotes = true,
            .handleCurlyBraces = false,
            .skipComments = true,
            .preserveNewlines = false,
            .punctuation = "(,{})(\':;=!><&|+"
        };
    }
    
    /// Options for parsing with curly brace blocks
    static constexpr ParseOptions withCurlyBraces() {
        return ParseOptions{
            .handleQuotes = true,
            .handleCurlyBraces = true,
            .skipComments = true,
            .preserveNewlines = false,
            .punctuation = ""
        };
    }
};

/// Parse a single token from the input string
/// 
/// @param input The input string to parse from
/// @param options Parsing options
/// @return ParseResult containing the token and metadata
///
/// Example:
///   std::string_view input = "name \"John Doe\" value 42";
///   auto result = parseToken(input);
///   // result.token = "name", result.consumedBytes = 5
///   input = input.substr(result.consumedBytes);
///   result = parseToken(input);
///   // result.token = "John Doe", result.consumedBytes = 12
[[nodiscard]] ParseResult parseToken(std::string_view input, 
                                      const ParseOptions& options = ParseOptions::quakeDefault());

/// Parse all tokens from input into a vector
///
/// @param input The input string to parse
/// @param options Parsing options
/// @return Vector of all tokens
[[nodiscard]] std::vector<std::string> parseAllTokens(std::string_view input,
                                                       const ParseOptions& options = ParseOptions::quakeDefault());

/// Simple tokenizer that splits on whitespace, respecting quotes
/// Similar to parseAllTokens but more lightweight
///
/// @param input The input string to tokenize
/// @return Vector of tokens
[[nodiscard]] std::vector<std::string> tokenize(std::string_view input);

/// Skip whitespace and return remaining string
[[nodiscard]] std::string_view skipWhitespace(std::string_view input);

/// Skip a line (until newline) and return remaining string
[[nodiscard]] std::string_view skipLine(std::string_view input);

/// Check if character is a Quake whitespace character
[[nodiscard]] constexpr bool isQuakeWhitespace(char c) noexcept {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

/// Check if character is a quote character
[[nodiscard]] constexpr bool isQuote(char c) noexcept {
    return c == '\"';
}

} // namespace ezquake::text
