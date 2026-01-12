/*
 * ezQuake C++ Port - Application Context
 * 
 * Dependency injection container for all subsystems.
 * NO SINGLETONS - all subsystems are owned and injected through this context.
 * 
 * Copyright (C) 2024 ezQuake Team
 */

#ifndef EZQUAKE_CORE_CONTEXT_HPP
#define EZQUAKE_CORE_CONTEXT_HPP

#include "core/types.hpp"
#include <mutex>
#include <shared_mutex>

namespace ezquake {

// Forward declarations for non-singleton subsystems
class CvarRegistry;      // core/cvar/cvar_registry.hpp
class CommandRegistry;   // core/cmd/cmd_registry.hpp
class Console;           // core/console/console.hpp

/**
 * @brief Application context - owns all subsystems
 * 
 * This class provides dependency injection for all ezQuake subsystems.
 * Instead of singletons, subsystems are created and owned here, then
 * passed by reference to code that needs them.
 * 
 * Thread Safety: The context itself is thread-safe. Individual subsystems
 * manage their own thread safety.
 * 
 * Usage:
 *   // Create context (typically once at startup)
 *   auto ctx = ezquake::Context::create();
 *   
 *   // Access subsystems
 *   ctx->cvars().create("sensitivity", "3.0");
 *   ctx->console().print("Hello world\n");
 *   
 *   // Pass to subsystems that need it
 *   void MyFunction(ezquake::Context& ctx) {
 *       ctx.commands().execute("map dm4");
 *   }
 */
class Context {
public:
    /// Create a new context with all subsystems initialized
    [[nodiscard]] static UniquePtr<Context> create();
    
    /// Destructor - cleans up in reverse order of initialization
    ~Context();
    
    // Non-copyable, non-movable (subsystems hold references to each other)
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(Context&&) = delete;
    
    //=========================================================================
    // Subsystem Access
    //=========================================================================
    
    /// Console variable registry
    [[nodiscard]] CvarRegistry& cvars() noexcept { return *cvars_; }
    [[nodiscard]] const CvarRegistry& cvars() const noexcept { return *cvars_; }
    
    /// Command registry
    [[nodiscard]] CommandRegistry& commands() noexcept { return *commands_; }
    [[nodiscard]] const CommandRegistry& commands() const noexcept { return *commands_; }
    
    /// Console output
    [[nodiscard]] Console& console() noexcept { return *console_; }
    [[nodiscard]] const Console& console() const noexcept { return *console_; }
    
    //=========================================================================
    // Lifecycle
    //=========================================================================
    
    /// Initialize all subsystems (called automatically by create())
    void init();
    
    /// Shutdown all subsystems
    void shutdown();
    
    /// Check if context is initialized
    [[nodiscard]] bool isInitialized() const noexcept { return initialized_; }
    
private:
    /// Private constructor - use create()
    Context();
    
    /// Subsystems in order of initialization
    UniquePtr<Console> console_;
    UniquePtr<CvarRegistry> cvars_;
    UniquePtr<CommandRegistry> commands_;
    
    bool initialized_ = false;
    mutable std::shared_mutex mutex_;
};

/**
 * @brief RAII guard for context lifecycle
 * 
 * Creates a context on construction and shuts it down on destruction.
 * Useful for tests and standalone tools.
 */
class ContextGuard {
public:
    ContextGuard() : context_(Context::create()) {
        context_->init();
    }
    
    ~ContextGuard() {
        if (context_) {
            context_->shutdown();
        }
    }
    
    // Non-copyable
    ContextGuard(const ContextGuard&) = delete;
    ContextGuard& operator=(const ContextGuard&) = delete;
    
    // Movable
    ContextGuard(ContextGuard&&) = default;
    ContextGuard& operator=(ContextGuard&&) = default;
    
    [[nodiscard]] Context& get() noexcept { return *context_; }
    [[nodiscard]] const Context& get() const noexcept { return *context_; }
    
    [[nodiscard]] Context* operator->() noexcept { return context_.get(); }
    [[nodiscard]] const Context* operator->() const noexcept { return context_.get(); }
    
private:
    UniquePtr<Context> context_;
};

} // namespace ezquake

#endif // EZQUAKE_CORE_CONTEXT_HPP
