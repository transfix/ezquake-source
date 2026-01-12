/*
 * ezQuake C++ Port - Command Registry
 * 
 * Non-singleton command system for dependency injection.
 * Commands are registered with callbacks, aliases can be defined, and
 * command text can be queued for execution.
 * 
 * NO SINGLETONS - Create CommandRegistry instances and pass by reference.
 * 
 * Copyright (C) 1996-1997 Id Software, Inc.
 * Copyright (C) 2024 ezQuake Team
 */

#ifndef EZQUAKE_CORE_CMD_REGISTRY_HPP
#define EZQUAKE_CORE_CMD_REGISTRY_HPP

#include "core/types.hpp"
#include "core/text/string_utils.hpp"
#include <unordered_map>
#include <functional>
#include <shared_mutex>
#include <deque>
#include <concepts>

namespace ezquake {

//=============================================================================
// Forward Declarations
//=============================================================================

class CommandRegistry;
class CommandBuffer;

//=============================================================================
// Command Arguments
//=============================================================================

/**
 * @brief Parsed command arguments
 * 
 * Holds the tokenized arguments from a command line.
 * Provides argc/argv style access similar to the C version.
 * This class is fully thread-safe (immutable after construction).
 */
class CommandArgs {
public:
    static constexpr Size MAX_ARGS = 80;
    
    /// Default constructor (empty args)
    CommandArgs() = default;
    
    /// Construct from a vector of arguments
    explicit CommandArgs(Vector<String> args);
    
    /// Parse a command string into arguments
    [[nodiscard]] static CommandArgs parse(StringView text);
    
    /// Get argument count
    [[nodiscard]] Int32 argc() const noexcept { 
        return static_cast<Int32>(args_.size()); 
    }
    
    /// Get argument by index (returns empty string if out of range)
    [[nodiscard]] StringView argv(Int32 index) const noexcept;
    
    /// Get all arguments from index 1 onwards as a single string
    [[nodiscard]] String args() const;
    
    /// Get all arguments from 'start' onwards as a single string
    [[nodiscard]] String argsFrom(Int32 start) const;
    
    /// Check if no arguments
    [[nodiscard]] bool empty() const noexcept { return args_.empty(); }
    
    /// Get the original command text
    [[nodiscard]] const String& text() const noexcept { return originalText_; }
    
    /// Get argument vector
    [[nodiscard]] const Vector<String>& argVector() const noexcept { return args_; }
    
    /// Access by index operator
    [[nodiscard]] StringView operator[](Int32 index) const noexcept { return argv(index); }
    
private:
    Vector<String> args_;
    String originalText_;
};

//=============================================================================
// Command Callback Types
//=============================================================================

/// Modern command callback (receives parsed arguments)
using CommandCallback = std::function<void(const CommandArgs&)>;

/// Legacy command callback (no arguments, uses global state)
using LegacyCommandCallback = std::function<void()>;

//=============================================================================
// Command Flags
//=============================================================================

/// Flags that control command behavior
enum class CommandFlags : UInt32 {
    None        = 0,
    ServerOnly  = 1 << 0,  // Can only be executed on server
    CheatCmd    = 1 << 1,  // Cheats must be enabled
    UserCreated = 1 << 2,  // Created at runtime (can be removed)
    Hidden      = 1 << 3,  // Don't show in help/completion
    Forward     = 1 << 4,  // Forward to server if function is null
};

/// Enable bitwise operations on CommandFlags
[[nodiscard]] inline constexpr CommandFlags operator|(CommandFlags a, CommandFlags b) noexcept {
    return static_cast<CommandFlags>(static_cast<UInt32>(a) | static_cast<UInt32>(b));
}
[[nodiscard]] inline constexpr CommandFlags operator&(CommandFlags a, CommandFlags b) noexcept {
    return static_cast<CommandFlags>(static_cast<UInt32>(a) & static_cast<UInt32>(b));
}
inline constexpr CommandFlags& operator|=(CommandFlags& a, CommandFlags b) noexcept {
    return a = a | b;
}
[[nodiscard]] inline constexpr bool hasFlag(CommandFlags flags, CommandFlags test) noexcept {
    return (static_cast<UInt32>(flags) & static_cast<UInt32>(test)) != 0;
}

//=============================================================================
// Command
//=============================================================================

/**
 * @brief A registered console command
 * 
 * Commands are registered with a name and callback. When the user types
 * the command, the callback is invoked with parsed arguments.
 */
class Command {
public:
    Command(String name, CommandCallback callback, CommandFlags flags = CommandFlags::None)
        : name_(std::move(name))
        , callback_(std::move(callback))
        , flags_(flags)
    {}
    
    Command(String name, LegacyCommandCallback callback, CommandFlags flags = CommandFlags::None)
        : name_(std::move(name))
        , legacyCallback_(std::move(callback))
        , flags_(flags)
    {}
    
    /// Execute the command with given arguments
    void execute(const CommandArgs& args) const {
        if (callback_) {
            callback_(args);
        } else if (legacyCallback_) {
            legacyCallback_();
        }
    }
    
    /// Get command name
    [[nodiscard]] const String& name() const noexcept { return name_; }
    
    /// Get command flags
    [[nodiscard]] CommandFlags flags() const noexcept { return flags_; }
    
    /// Check if command has a flag
    [[nodiscard]] bool hasFlag(CommandFlags flag) const noexcept {
        return ezquake::hasFlag(flags_, flag);
    }
    
    /// Check if command has a callback
    [[nodiscard]] bool hasCallback() const noexcept {
        return callback_ != nullptr || legacyCallback_ != nullptr;
    }
    
private:
    String name_;
    CommandCallback callback_;
    LegacyCommandCallback legacyCallback_;
    CommandFlags flags_ = CommandFlags::None;
};

//=============================================================================
// Alias Flags
//=============================================================================

/// Flags that control alias behavior
enum class AliasFlags : UInt32 {
    None        = 0,
    ServerSet   = 1 << 0,  // Created by server (can be deleted on disconnect)
    Archive     = 1 << 1,  // Save to config
    UserCreated = 1 << 2,  // Created by user (not hardcoded)
};

/// Enable bitwise operations
[[nodiscard]] inline constexpr AliasFlags operator|(AliasFlags a, AliasFlags b) noexcept {
    return static_cast<AliasFlags>(static_cast<UInt32>(a) | static_cast<UInt32>(b));
}
[[nodiscard]] inline constexpr AliasFlags operator&(AliasFlags a, AliasFlags b) noexcept {
    return static_cast<AliasFlags>(static_cast<UInt32>(a) & static_cast<UInt32>(b));
}

//=============================================================================
// Alias
//=============================================================================

/**
 * @brief A command alias
 */
struct Alias {
    String name;
    String value;
    AliasFlags flags = AliasFlags::None;
    
    Alias() = default;
    Alias(String n, String v, AliasFlags f = AliasFlags::None)
        : name(std::move(n)), value(std::move(v)), flags(f) {}
        
    [[nodiscard]] bool hasFlag(AliasFlags flag) const noexcept {
        return (static_cast<UInt32>(flags) & static_cast<UInt32>(flag)) != 0;
    }
};

//=============================================================================
// Command Buffer
//=============================================================================

/**
 * @brief Command execution buffer
 * 
 * Commands are queued in the buffer and executed in order.
 * Thread-safe for adding commands; execution should be single-threaded.
 */
class CommandBuffer {
public:
    /// Create a command buffer with given name (for debugging)
    explicit CommandBuffer(String name, Size maxSize = 8192);
    
    /// Add text to the end of the buffer
    void addText(StringView text);
    
    /// Insert text at the front of the buffer (for immediate execution)
    void insertText(StringView text);
    
    /// Execute all pending commands in the buffer
    /// @param registry The command registry to use for execution
    void execute(CommandRegistry& registry);
    
    /// Execute commands until buffer is empty (for startup)
    /// @param registry The command registry to use for execution
    void flush(CommandRegistry& registry);
    
    /// Clear all pending commands
    void clear();
    
    /// Check if buffer is empty
    [[nodiscard]] bool empty() const noexcept;
    
    /// Get current buffer size
    [[nodiscard]] Size size() const noexcept;
    
    /// Check if waiting (wait command was issued)
    [[nodiscard]] bool isWaiting() const noexcept { return wait_; }
    
    /// Clear wait state
    void clearWait() noexcept { wait_ = false; }
    
    /// Set wait state
    void setWait() noexcept { wait_ = true; }
    
    /// Get buffer name
    [[nodiscard]] const String& name() const noexcept { return name_; }
    
private:
    String name_;
    String buffer_;
    Size maxSize_;
    bool wait_ = false;
    Int32 waitCount_ = 0;
    Int32 runAwayLoop_ = 0;
    mutable std::mutex mutex_;
};

//=============================================================================
// Command Registry
//=============================================================================

/**
 * @brief Central registry for commands and aliases
 * 
 * NOT A SINGLETON - Create instances and pass by reference.
 * 
 * Thread Safety:
 *   - Registration/lookup use reader-writer lock
 *   - Command buffers have their own locks
 *   - Execution should generally be single-threaded
 * 
 * Usage:
 *   CommandRegistry commands;
 *   commands.addCommand("echo", [](const CommandArgs& args) {
 *       std::cout << args.args() << '\n';
 *   });
 *   commands.executeString("echo Hello World");
 */
class CommandRegistry {
public:
    /// Default constructor
    CommandRegistry();
    
    /// Destructor
    ~CommandRegistry();
    
    // Non-copyable, movable
    CommandRegistry(const CommandRegistry&) = delete;
    CommandRegistry& operator=(const CommandRegistry&) = delete;
    CommandRegistry(CommandRegistry&&) noexcept = default;
    CommandRegistry& operator=(CommandRegistry&&) noexcept = default;
    
    //=========================================================================
    // Command Registration
    //=========================================================================
    
    /// Register a command with modern callback
    Command& addCommand(String name, CommandCallback callback, 
                        CommandFlags flags = CommandFlags::None);
    
    /// Register a command with legacy callback
    Command& addCommand(String name, LegacyCommandCallback callback,
                        CommandFlags flags = CommandFlags::None);
    
    /// Remove a command (only works for user-created commands)
    bool removeCommand(StringView name);
    
    /// Check if a command exists
    [[nodiscard]] bool commandExists(StringView name) const;
    
    /// Find a command by name
    [[nodiscard]] Command* findCommand(StringView name);
    [[nodiscard]] const Command* findCommand(StringView name) const;
    
    //=========================================================================
    // Alias Management
    //=========================================================================
    
    /// Create or update an alias
    Alias& addAlias(String name, String value, AliasFlags flags = AliasFlags::None);
    
    /// Delete an alias
    bool removeAlias(StringView name);
    
    /// Find an alias
    [[nodiscard]] Alias* findAlias(StringView name);
    [[nodiscard]] const Alias* findAlias(StringView name) const;
    
    /// Get alias value (returns nullptr if not found)
    [[nodiscard]] const String* aliasValue(StringView name) const;
    
    /// Delete all server-created aliases
    void deleteServerAliases();
    
    //=========================================================================
    // Command Execution
    //=========================================================================
    
    /// Execute a single command string
    void executeString(StringView text);
    
    /// Execute a command string in a specific buffer context
    void executeStringEx(CommandBuffer& buffer, StringView text);
    
    /// Get the current command buffer being executed
    [[nodiscard]] CommandBuffer* currentBuffer() noexcept { return currentBuffer_; }
    
    //=========================================================================
    // Command Completion
    //=========================================================================
    
    /// Get completions for partial command name
    [[nodiscard]] Vector<String> completeCommand(StringView partial) const;
    
    /// Get completions for partial alias name
    [[nodiscard]] Vector<String> completeAlias(StringView partial) const;
    
    //=========================================================================
    // Iteration (using C++20 concepts)
    //=========================================================================
    
    /// Iterate over all commands
    template<std::invocable<Command&> Func>
    void forEachCommand(Func&& func) {
        std::shared_lock lock(mutex_);
        for (auto& [name, cmd] : commands_) {
            func(*cmd);
        }
    }
    
    /// Iterate over all commands (const version)
    template<std::invocable<const Command&> Func>
    void forEachCommand(Func&& func) const {
        std::shared_lock lock(mutex_);
        for (const auto& [name, cmd] : commands_) {
            func(*cmd);
        }
    }
    
    /// Iterate over all aliases
    template<std::invocable<Alias&> Func>
    void forEachAlias(Func&& func) {
        std::shared_lock lock(mutex_);
        for (auto& [name, alias] : aliases_) {
            func(alias);
        }
    }
    
    /// Iterate over all aliases (const version)
    template<std::invocable<const Alias&> Func>
    void forEachAlias(Func&& func) const {
        std::shared_lock lock(mutex_);
        for (const auto& [name, alias] : aliases_) {
            func(alias);
        }
    }
    
    //=========================================================================
    // Legacy Command Support
    //=========================================================================
    
    /// Add a legacy command mapping (old name -> new name)
    void addLegacyCommand(StringView oldName, StringView newName);
    
    /// Check if a command is a legacy command
    [[nodiscard]] bool isLegacyCommand(StringView name) const;
    
    //=========================================================================
    // Lifecycle
    //=========================================================================
    
    /// Clear all commands and aliases
    void clear();
    
    /// Get command count
    [[nodiscard]] Size commandCount() const;
    
    /// Get alias count
    [[nodiscard]] Size aliasCount() const;
    
    //=========================================================================
    // Built-in Command Buffers
    //=========================================================================
    
    /// Main command buffer
    [[nodiscard]] CommandBuffer& mainBuffer() { return mainBuffer_; }
    
    /// Safe command buffer (for triggers)
    [[nodiscard]] CommandBuffer& safeBuffer() { return safeBuffer_; }
    
    /// Server command buffer
    [[nodiscard]] CommandBuffer& serverBuffer() { return serverBuffer_; }
    
    /// Get the currently executing command's arguments
    [[nodiscard]] const CommandArgs* currentArgs() const {
        return currentArgs_.empty() ? nullptr : &currentArgs_;
    }
    
    //=========================================================================
    // Builtins Registration
    //=========================================================================
    
    /// Register built-in commands (call after construction)
    void registerBuiltins();
    
private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<String, UniquePtr<Command>> commands_;
    std::unordered_map<String, Alias> aliases_;
    std::unordered_map<String, String> legacyCommands_; // old -> new
    
    CommandBuffer mainBuffer_{"main"};
    CommandBuffer safeBuffer_{"safe"};
    CommandBuffer serverBuffer_{"server"};
    CommandBuffer* currentBuffer_ = nullptr;
    
    // Current command arguments (for legacy callbacks)
    CommandArgs currentArgs_;
};

} // namespace ezquake

#endif // EZQUAKE_CORE_CMD_REGISTRY_HPP
