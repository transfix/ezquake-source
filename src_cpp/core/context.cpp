/*
 * ezQuake C++ Port - Application Context Implementation
 * 
 * Copyright (C) 2024 ezQuake Team
 */

#include "context.hpp"
#include "core/cvar/cvar_registry.hpp"
#include "core/cmd/cmd_registry.hpp"
#include "core/console/console.hpp"

namespace ezquake {

Context::Context() = default;

Context::~Context() {
    if (initialized_) {
        shutdown();
    }
}

UniquePtr<Context> Context::create() {
    // Using new directly since make_unique can't access private constructor
    return UniquePtr<Context>(new Context());
}

void Context::init() {
    std::unique_lock lock(mutex_);
    
    if (initialized_) {
        return;
    }
    
    // Create subsystems in proper order (dependencies first)
    // 1. Console (output) - no dependencies
    console_ = makeUnique<Console>();
    console_->init();
    
    // 2. Cvar registry - depends on console for output
    cvars_ = makeUnique<CvarRegistry>();
    
    // 3. Command registry - depends on cvars and console
    commands_ = makeUnique<CommandRegistry>();
    
    initialized_ = true;
}

void Context::shutdown() {
    std::unique_lock lock(mutex_);
    
    if (!initialized_) {
        return;
    }
    
    // Shutdown in reverse order of initialization
    
    // 3. Commands first
    if (commands_) {
        commands_->clear();
        commands_.reset();
    }
    
    // 2. Cvars next
    if (cvars_) {
        cvars_->clear();
        cvars_.reset();
    }
    
    // 1. Console last
    if (console_) {
        console_->shutdown();
        console_.reset();
    }
    
    initialized_ = false;
}

} // namespace ezquake
