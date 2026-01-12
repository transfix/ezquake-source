// command_line.cpp - Command line argument parsing implementation
// Part of the ezQuake C++ codebase (Iteration 8)

#include "command_line.hpp"
#include <cstdarg>
#include <cstdio>
#include <algorithm>

namespace ezquake {

CommandLine::CommandLine(int argc, char** argv) {
    parse(argc, argv);
}

CommandLine::CommandLine(std::span<const std::string_view> args) {
    parse(args);
}

CommandLine::CommandLine(std::vector<std::string> args)
    : args_(std::move(args)) 
{
    buildIndex();
}

void CommandLine::parse(int argc, char** argv) {
    args_.clear();
    originalCmdline_.clear();
    
    for (int i = 0; i < argc; ++i) {
        if (argv[i]) {
            // Handle QW URLs - prepend +qwurl command
            if (i > 0 && args_.size() >= 1) {
                std::string_view arg{argv[i]};
                std::string_view prevArg = args_.back();
                
                // Check for qw:// URL without +qwurl command
                if (arg.starts_with("qw://") && prevArg != "+qwurl") {
                    args_.emplace_back("+qwurl");
                }
            }
            
            args_.emplace_back(argv[i]);
            
            // Build original command line string
            if (i > 0) {
                originalCmdline_ += ' ';
            }
            originalCmdline_ += argv[i];
        } else {
            args_.emplace_back("");
        }
    }
    
    buildIndex();
}

void CommandLine::parse(std::span<const std::string_view> args) {
    args_.clear();
    originalCmdline_.clear();
    
    for (std::size_t i = 0; i < args.size(); ++i) {
        args_.emplace_back(args[i]);
        
        if (i > 0) {
            originalCmdline_ += ' ';
        }
        originalCmdline_ += args[i];
    }
    
    buildIndex();
}

void CommandLine::buildIndex() {
    paramIndex_.clear();
    qwUrlIndex_ = -1;
    
    for (int i = 0; i < static_cast<int>(args_.size()); ++i) {
        const auto& arg = args_[static_cast<std::size_t>(i)];
        
        if (!arg.empty()) {
            // Index parameters that start with - or +
            if (arg[0] == '-' || arg[0] == '+') {
                // Only store first occurrence
                if (paramIndex_.find(arg) == paramIndex_.end()) {
                    paramIndex_[arg] = i;
                }
            }
            
            // Check for QW URL
            if (arg.starts_with("qw://") && qwUrlIndex_ < 0) {
                qwUrlIndex_ = i;
            }
        }
    }
}

std::string_view CommandLine::argv(int index) const {
    if (index < 0 || index >= static_cast<int>(args_.size())) {
        return "";
    }
    return args_[static_cast<std::size_t>(index)];
}

int CommandLine::checkParam(std::string_view param) const {
    auto it = paramIndex_.find(std::string{param});
    if (it != paramIndex_.end()) {
        return it->second;
    }
    return 0;
}

bool CommandLine::hasParam(std::string_view param) const {
    return paramIndex_.find(std::string{param}) != paramIndex_.end();
}

std::optional<std::string_view> CommandLine::getParamValue(std::string_view param) const {
    int idx = checkParam(param);
    if (idx > 0 && idx + 1 < static_cast<int>(args_.size())) {
        return args_[static_cast<std::size_t>(idx + 1)];
    }
    return std::nullopt;
}

std::string_view CommandLine::getParamValueOr(std::string_view param, 
                                               std::string_view defaultValue) const {
    auto value = getParamValue(param);
    return value.value_or(defaultValue);
}

std::vector<std::string_view> CommandLine::getParamValues(std::string_view param, 
                                                           int count) const {
    std::vector<std::string_view> result;
    
    int idx = checkParam(param);
    if (idx > 0) {
        int maxIdx = static_cast<int>(args_.size());
        for (int i = 1; i <= count && idx + i < maxIdx; ++i) {
            result.push_back(args_[static_cast<std::size_t>(idx + i)]);
        }
    }
    
    return result;
}

void CommandLine::clearArgv(int index) {
    if (index >= 0 && index < static_cast<int>(args_.size())) {
        // Remove from index if it was a parameter
        auto it = paramIndex_.find(args_[static_cast<std::size_t>(index)]);
        if (it != paramIndex_.end() && it->second == index) {
            paramIndex_.erase(it);
        }
        
        args_[static_cast<std::size_t>(index)].clear();
    }
}

bool CommandLine::hasQwUrl() const {
    return qwUrlIndex_ >= 0;
}

std::optional<std::string_view> CommandLine::getQwUrl() const {
    if (qwUrlIndex_ >= 0) {
        return args_[static_cast<std::size_t>(qwUrlIndex_)];
    }
    return std::nullopt;
}

// va() implementation - compatible with original C version
const char* va(const char* format, ...) {
    static char string[32][2048];
    static int idx = 0;
    
    idx = (idx + 1) % 32;
    
    va_list argptr;
    va_start(argptr, format);
    std::vsnprintf(string[idx], sizeof(string[idx]), format, argptr);
    va_end(argptr);
    
    return string[idx];
}

} // namespace ezquake
