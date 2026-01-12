// token_parser.cpp - Token parsing implementation
// Part of the ezQuake C++ codebase (Iteration 8)

#include "token_parser.hpp"
#include <algorithm>

namespace ezquake::text {

std::string_view skipWhitespace(std::string_view input) {
    while (!input.empty() && isQuakeWhitespace(input[0])) {
        input.remove_prefix(1);
    }
    return input;
}

std::string_view skipLine(std::string_view input) {
    while (!input.empty() && input[0] != '\n') {
        input.remove_prefix(1);
    }
    if (!input.empty()) {
        input.remove_prefix(1); // Skip the newline itself
    }
    return input;
}

namespace {

// Skip C-style block comment /* ... */
std::string_view skipBlockComment(std::string_view input) {
    // Assume we're at the '*' after '/'
    input.remove_prefix(1);
    
    while (input.size() >= 2) {
        if (input[0] == '*' && input[1] == '/') {
            input.remove_prefix(2);
            return input;
        }
        input.remove_prefix(1);
    }
    
    // Unterminated block comment - consume rest of input
    return {};
}

bool isPunctuation(char c, std::string_view punctuation) {
    return !punctuation.empty() && punctuation.find(c) != std::string_view::npos;
}

} // anonymous namespace

ParseResult parseToken(std::string_view input, const ParseOptions& options) {
    std::size_t totalConsumed = 0;
    
    // Skip whitespace and comments
    while (true) {
        // Skip whitespace (but optionally preserve newlines)
        while (!input.empty()) {
            char c = input[0];
            
            if (options.preserveNewlines && c == '\n') {
                // Return newline as a token
                return ParseResult{
                    .token = "\n",
                    .type = TokenType::Newline,
                    .consumedBytes = totalConsumed + 1
                };
            }
            
            if (!isQuakeWhitespace(c)) {
                break;
            }
            
            input.remove_prefix(1);
            ++totalConsumed;
        }
        
        if (input.empty()) {
            return ParseResult{
                .token = "",
                .type = TokenType::Unknown,
                .consumedBytes = totalConsumed
            };
        }
        
        // Check for comments
        if (options.skipComments && input.size() >= 2) {
            if (input[0] == '/' && input[1] == '/') {
                // Line comment - skip to end of line
                while (!input.empty() && input[0] != '\n') {
                    input.remove_prefix(1);
                    ++totalConsumed;
                }
                continue;
            }
            
            if (input[0] == '/' && input[1] == '*') {
                // Block comment
                input.remove_prefix(2);
                totalConsumed += 2;
                
                while (input.size() >= 2) {
                    if (input[0] == '*' && input[1] == '/') {
                        input.remove_prefix(2);
                        totalConsumed += 2;
                        break;
                    }
                    input.remove_prefix(1);
                    ++totalConsumed;
                }
                continue;
            }
        }
        
        break;
    }
    
    if (input.empty()) {
        return ParseResult{
            .token = "",
            .type = TokenType::Unknown,
            .consumedBytes = totalConsumed
        };
    }
    
    // Check for punctuation
    if (isPunctuation(input[0], options.punctuation)) {
        return ParseResult{
            .token = std::string(1, input[0]),
            .type = TokenType::Punctuation,
            .consumedBytes = totalConsumed + 1
        };
    }
    
    // Handle quoted strings
    if (options.handleQuotes && input[0] == '\"') {
        input.remove_prefix(1);
        ++totalConsumed;
        
        std::string token;
        while (!input.empty() && input[0] != '\"') {
            token += input[0];
            input.remove_prefix(1);
            ++totalConsumed;
        }
        
        // Skip closing quote
        if (!input.empty() && input[0] == '\"') {
            ++totalConsumed;
        }
        
        return ParseResult{
            .token = std::move(token),
            .type = TokenType::String,
            .consumedBytes = totalConsumed
        };
    }
    
    // Handle curly brace blocks
    if (options.handleCurlyBraces && input[0] == '{') {
        input.remove_prefix(1);
        ++totalConsumed;
        
        std::string token;
        int braceDepth = 1;
        
        while (!input.empty() && braceDepth > 0) {
            char c = input[0];
            
            if (c == '{') {
                ++braceDepth;
            } else if (c == '}') {
                --braceDepth;
                if (braceDepth == 0) {
                    input.remove_prefix(1);
                    ++totalConsumed;
                    break;
                }
            }
            
            token += c;
            input.remove_prefix(1);
            ++totalConsumed;
        }
        
        return ParseResult{
            .token = std::move(token),
            .type = TokenType::String,
            .consumedBytes = totalConsumed
        };
    }
    
    // Parse regular word
    std::string token;
    while (!input.empty()) {
        char c = input[0];
        
        if (isQuakeWhitespace(c)) {
            break;
        }
        
        if (isPunctuation(c, options.punctuation)) {
            break;
        }
        
        token += c;
        input.remove_prefix(1);
        ++totalConsumed;
    }
    
    // Determine if it's a number
    TokenType type = TokenType::String;
    if (!token.empty()) {
        bool isNumber = true;
        bool hasDigit = false;
        bool hasDot = false;
        
        for (std::size_t i = 0; i < token.size(); ++i) {
            char c = token[i];
            
            if (c == '-' || c == '+') {
                if (i != 0) {
                    isNumber = false;
                    break;
                }
            } else if (c == '.') {
                if (hasDot) {
                    isNumber = false;
                    break;
                }
                hasDot = true;
            } else if (c >= '0' && c <= '9') {
                hasDigit = true;
            } else {
                isNumber = false;
                break;
            }
        }
        
        if (isNumber && hasDigit) {
            type = TokenType::Number;
        }
    }
    
    return ParseResult{
        .token = std::move(token),
        .type = type,
        .consumedBytes = totalConsumed
    };
}

std::vector<std::string> parseAllTokens(std::string_view input, const ParseOptions& options) {
    std::vector<std::string> result;
    
    while (!input.empty()) {
        auto parsed = parseToken(input, options);
        
        if (!parsed.valid()) {
            break;
        }
        
        result.push_back(std::move(parsed.token));
        
        if (parsed.consumedBytes == 0) {
            break; // Safety: prevent infinite loop
        }
        
        input.remove_prefix(parsed.consumedBytes);
    }
    
    return result;
}

std::vector<std::string> tokenize(std::string_view input) {
    return parseAllTokens(input, ParseOptions::quakeDefault());
}

} // namespace ezquake::text
