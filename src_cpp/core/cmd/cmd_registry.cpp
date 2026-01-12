/*
 * ezQuake C++ Port - Command Registry Implementation
 * 
 * Non-singleton implementation for dependency injection.
 *
 * Copyright (C) 1996-1997 Id Software, Inc.
 * Copyright (C) 2024 ezQuake Team
 */

#include "cmd_registry.hpp"
#include <algorithm>
#include <sstream>
#include <cctype>
#include <ranges>
#include <mutex>

namespace ezquake {

//=============================================================================
// CommandArgs Implementation
//=============================================================================

CommandArgs::CommandArgs(Vector<String> args) 
    : args_(std::move(args)) 
{
    // Reconstruct original text
    std::ostringstream ss;
    for (Size i = 0; i < args_.size(); ++i) {
        if (i > 0) ss << ' ';
        // Quote if contains spaces
        if (args_[i].find(' ') != String::npos) {
            ss << '"' << args_[i] << '"';
        } else {
            ss << args_[i];
        }
    }
    originalText_ = ss.str();
}

CommandArgs CommandArgs::parse(StringView text) {
    CommandArgs result;
    result.originalText_ = String(text);
    
    if (text.empty()) {
        return result;
    }
    
    const char* data = text.data();
    const char* end = data + text.size();
    
    while (data < end && result.args_.size() < MAX_ARGS) {
        // Skip whitespace
        while (data < end && std::isspace(static_cast<unsigned char>(*data))) {
            ++data;
        }
        
        if (data >= end) break;
        
        String arg;
        
        if (*data == '"') {
            // Quoted string
            ++data;
            while (data < end && *data != '"') {
                arg += *data++;
            }
            if (data < end) ++data; // Skip closing quote
        } else {
            // Unquoted token
            while (data < end && !std::isspace(static_cast<unsigned char>(*data))) {
                arg += *data++;
            }
        }
        
        if (!arg.empty()) {
            result.args_.push_back(std::move(arg));
        }
    }
    
    return result;
}

StringView CommandArgs::argv(Int32 index) const noexcept {
    if (index < 0 || static_cast<Size>(index) >= args_.size()) {
        static const String empty;
        return empty;
    }
    return args_[static_cast<Size>(index)];
}

String CommandArgs::args() const {
    return argsFrom(1);
}

String CommandArgs::argsFrom(Int32 start) const {
    if (start < 0 || static_cast<Size>(start) >= args_.size()) {
        return {};
    }
    
    std::ostringstream ss;
    for (Size i = static_cast<Size>(start); i < args_.size(); ++i) {
        if (i > static_cast<Size>(start)) ss << ' ';
        ss << args_[i];
    }
    return ss.str();
}

//=============================================================================
// CommandBuffer Implementation
//=============================================================================

CommandBuffer::CommandBuffer(String name, Size maxSize)
    : name_(std::move(name))
    , maxSize_(maxSize)
{
    buffer_.reserve(maxSize);
}

void CommandBuffer::addText(StringView text) {
    std::lock_guard lock(mutex_);
    
    if (buffer_.size() + text.size() > maxSize_) {
        // Buffer overflow - truncate
        Size available = maxSize_ - buffer_.size();
        if (available > 0) {
            buffer_.append(text.data(), available);
        }
        return;
    }
    buffer_.append(text.data(), text.size());
}

void CommandBuffer::insertText(StringView text) {
    std::lock_guard lock(mutex_);
    
    // Insert at the front by prepending
    String newBuffer;
    newBuffer.reserve(text.size() + buffer_.size() + 1);
    newBuffer.append(text.data(), text.size());
    if (!text.empty() && text.back() != '\n') {
        newBuffer += '\n';
    }
    newBuffer += buffer_;
    
    if (newBuffer.size() > maxSize_) {
        newBuffer.resize(maxSize_);
    }
    
    buffer_ = std::move(newBuffer);
}

void CommandBuffer::execute(CommandRegistry& registry) {
    if (wait_) {
        return;
    }
    
    ++runAwayLoop_;
    if (runAwayLoop_ > 1000000) {
        // Runaway loop protection
        clear();
        runAwayLoop_ = 0;
        return;
    }
    
    // Note: We do NOT hold mutex_ during executeStringEx to avoid deadlocks
    while (!wait_) {
        String line;
        
        {
            std::lock_guard lock(mutex_);
            
            if (buffer_.empty()) {
                break;
            }
            
            // Find the end of the current command (newline or semicolon outside quotes)
            Size cmdEnd = 0;
            bool inQuotes = false;
            bool inComment = false;
            
            for (Size i = 0; i < buffer_.size(); ++i) {
                char c = buffer_[i];
                
                if (c == '\n') {
                    cmdEnd = i;
                    break;
                }
                
                if (c == '"' && !inComment) {
                    inQuotes = !inQuotes;
                }
                
                if (!inQuotes && !inComment) {
                    // Check for comment
                    if (c == '/' && i + 1 < buffer_.size() && buffer_[i + 1] == '/') {
                        inComment = true;
                    }
                    // Check for command separator
                    else if (c == ';') {
                        cmdEnd = i;
                        break;
                    }
                }
                
                // If we reach the end without finding a separator
                if (i == buffer_.size() - 1) {
                    cmdEnd = buffer_.size();
                }
            }
            
            if (cmdEnd == 0 && !buffer_.empty() && buffer_[0] != '\n' && buffer_[0] != ';') {
                // No complete command yet
                break;
            }
            
            // Extract the command
            line = buffer_.substr(0, cmdEnd);
            
            // Remove the command from buffer
            if (cmdEnd < buffer_.size()) {
                buffer_.erase(0, cmdEnd + 1);
            } else {
                buffer_.clear();
            }
        } // Release mutex before executing
        
        // Trim the line
        while (!line.empty() && std::isspace(static_cast<unsigned char>(line.back()))) {
            line.pop_back();
        }
        while (!line.empty() && std::isspace(static_cast<unsigned char>(line.front()))) {
            line.erase(0, 1);
        }
        
        // Remove comment portion
        Size commentPos = String::npos;
        bool inQuotes = false;
        for (Size i = 0; i < line.size(); ++i) {
            if (line[i] == '"') {
                inQuotes = !inQuotes;
            } else if (!inQuotes && line[i] == '/' && i + 1 < line.size() && line[i + 1] == '/') {
                commentPos = i;
                break;
            }
        }
        if (commentPos != String::npos) {
            line.resize(commentPos);
            // Trim again
            while (!line.empty() && std::isspace(static_cast<unsigned char>(line.back()))) {
                line.pop_back();
            }
        }
        
        // Execute the command
        if (!line.empty()) {
            registry.executeStringEx(*this, line);
        }
    }
    
    --runAwayLoop_;
}

void CommandBuffer::flush(CommandRegistry& registry) {
    while (!empty()) {
        wait_ = false;
        execute(registry);
    }
}

void CommandBuffer::clear() {
    std::lock_guard lock(mutex_);
    buffer_.clear();
    wait_ = false;
    waitCount_ = 0;
}

bool CommandBuffer::empty() const noexcept {
    std::lock_guard lock(mutex_);
    return buffer_.empty();
}

Size CommandBuffer::size() const noexcept {
    std::lock_guard lock(mutex_);
    return buffer_.size();
}

//=============================================================================
// CommandRegistry Implementation
//=============================================================================

CommandRegistry::CommandRegistry() = default;

CommandRegistry::~CommandRegistry() {
    clear();
}

Command& CommandRegistry::addCommand(String name, CommandCallback callback, CommandFlags flags) {
    std::unique_lock lock(mutex_);
    
    // Check for existing
    auto it = commands_.find(name);
    if (it != commands_.end()) {
        return *it->second;
    }
    
    auto cmd = makeUnique<Command>(name, std::move(callback), flags);
    auto* ptr = cmd.get();
    commands_.emplace(std::move(name), std::move(cmd));
    return *ptr;
}

Command& CommandRegistry::addCommand(String name, LegacyCommandCallback callback, CommandFlags flags) {
    std::unique_lock lock(mutex_);
    
    auto it = commands_.find(name);
    if (it != commands_.end()) {
        return *it->second;
    }
    
    auto cmd = makeUnique<Command>(name, std::move(callback), flags);
    auto* ptr = cmd.get();
    commands_.emplace(std::move(name), std::move(cmd));
    return *ptr;
}

bool CommandRegistry::removeCommand(StringView name) {
    std::unique_lock lock(mutex_);
    
    auto it = commands_.find(String(name));
    if (it == commands_.end()) {
        return false;
    }
    
    if (!it->second->hasFlag(CommandFlags::UserCreated)) {
        return false;
    }
    
    commands_.erase(it);
    return true;
}

bool CommandRegistry::commandExists(StringView name) const {
    std::shared_lock lock(mutex_);
    return commands_.find(String(name)) != commands_.end();
}

Command* CommandRegistry::findCommand(StringView name) {
    std::shared_lock lock(mutex_);
    auto it = commands_.find(String(name));
    return it != commands_.end() ? it->second.get() : nullptr;
}

const Command* CommandRegistry::findCommand(StringView name) const {
    std::shared_lock lock(mutex_);
    auto it = commands_.find(String(name));
    return it != commands_.end() ? it->second.get() : nullptr;
}

Alias& CommandRegistry::addAlias(String name, String value, AliasFlags flags) {
    std::unique_lock lock(mutex_);
    
    auto [it, inserted] = aliases_.try_emplace(name, name, std::move(value), flags);
    if (!inserted) {
        it->second.value = std::move(value);
        it->second.flags = flags;
    }
    return it->second;
}

bool CommandRegistry::removeAlias(StringView name) {
    std::unique_lock lock(mutex_);
    return aliases_.erase(String(name)) > 0;
}

Alias* CommandRegistry::findAlias(StringView name) {
    std::shared_lock lock(mutex_);
    auto it = aliases_.find(String(name));
    return it != aliases_.end() ? &it->second : nullptr;
}

const Alias* CommandRegistry::findAlias(StringView name) const {
    std::shared_lock lock(mutex_);
    auto it = aliases_.find(String(name));
    return it != aliases_.end() ? &it->second : nullptr;
}

const String* CommandRegistry::aliasValue(StringView name) const {
    const Alias* alias = findAlias(name);
    return alias ? &alias->value : nullptr;
}

void CommandRegistry::deleteServerAliases() {
    std::unique_lock lock(mutex_);
    
    for (auto it = aliases_.begin(); it != aliases_.end(); ) {
        if (it->second.hasFlag(AliasFlags::ServerSet)) {
            it = aliases_.erase(it);
        } else {
            ++it;
        }
    }
}

void CommandRegistry::executeString(StringView text) {
    executeStringEx(mainBuffer_, text);
}

void CommandRegistry::executeStringEx(CommandBuffer& buffer, StringView text) {
    CommandBuffer* savedBuffer = currentBuffer_;
    currentBuffer_ = &buffer;
    
    // Parse the command
    currentArgs_ = CommandArgs::parse(text);
    
    if (currentArgs_.empty()) {
        currentBuffer_ = savedBuffer;
        return;
    }
    
    StringView cmdName = currentArgs_.argv(0);
    
    // Check for command
    Command* cmd = findCommand(cmdName);
    if (cmd) {
        cmd->execute(currentArgs_);
        currentBuffer_ = savedBuffer;
        return;
    }
    
    // Check for alias
    const Alias* alias = findAlias(cmdName);
    if (alias) {
        // Insert alias value into buffer
        buffer.insertText(alias->value);
        currentBuffer_ = savedBuffer;
        return;
    }
    
    // NOTE: Cvar lookup is now caller's responsibility via Context
    // The old singleton pattern is gone. If you need cvar lookup,
    // inject CvarRegistry into executeStringEx or use a callback.
    
    // Check for legacy command
    {
        std::shared_lock lock(mutex_);
        auto it = legacyCommands_.find(String(cmdName));
        if (it != legacyCommands_.end()) {
            // Execute as the new command name
            String newCmd = it->second + " " + currentArgs_.args();
            currentBuffer_ = savedBuffer;
            executeString(newCmd);
            return;
        }
    }
    
    // Unknown command
    // In real implementation, would print error or forward to server
    
    currentBuffer_ = savedBuffer;
}

Vector<String> CommandRegistry::completeCommand(StringView partial) const {
    std::shared_lock lock(mutex_);
    Vector<String> result;
    
    for (const auto& [name, cmd] : commands_) {
        if (text::startsWithIgnoreCase(name, partial)) {
            if (!cmd->hasFlag(CommandFlags::Hidden)) {
                result.push_back(name);
            }
        }
    }
    
    std::ranges::sort(result);
    return result;
}

Vector<String> CommandRegistry::completeAlias(StringView partial) const {
    std::shared_lock lock(mutex_);
    Vector<String> result;
    
    for (const auto& [name, alias] : aliases_) {
        if (text::startsWithIgnoreCase(name, partial)) {
            result.push_back(name);
        }
    }
    
    std::ranges::sort(result);
    return result;
}

void CommandRegistry::addLegacyCommand(StringView oldName, StringView newName) {
    std::unique_lock lock(mutex_);
    legacyCommands_[String(oldName)] = String(newName);
}

bool CommandRegistry::isLegacyCommand(StringView name) const {
    std::shared_lock lock(mutex_);
    return legacyCommands_.find(String(name)) != legacyCommands_.end();
}

void CommandRegistry::clear() {
    std::unique_lock lock(mutex_);
    commands_.clear();
    aliases_.clear();
    legacyCommands_.clear();
    mainBuffer_.clear();
    safeBuffer_.clear();
    serverBuffer_.clear();
}

Size CommandRegistry::commandCount() const {
    std::shared_lock lock(mutex_);
    return commands_.size();
}

Size CommandRegistry::aliasCount() const {
    std::shared_lock lock(mutex_);
    return aliases_.size();
}

void CommandRegistry::registerBuiltins() {
    // echo command
    addCommand("echo", [](const CommandArgs& args) {
        // Would print to console in real implementation
        (void)args;
    });
    
    // wait command
    addCommand("wait", [this](const CommandArgs& args) {
        (void)args;
        if (currentBuffer_) {
            currentBuffer_->setWait();
        }
    });
    
    // alias command
    addCommand("alias", [this](const CommandArgs& args) {
        if (args.argc() < 2) {
            // List all aliases
            return;
        }
        
        String name(args.argv(1));
        if (args.argc() == 2) {
            // Show alias value
            return;
        }
        
        String value = args.argsFrom(2);
        addAlias(std::move(name), std::move(value));
    });
    
    // unalias command
    addCommand("unalias", [this](const CommandArgs& args) {
        if (args.argc() < 2) {
            return;
        }
        removeAlias(args.argv(1));
    });
}

} // namespace ezquake
