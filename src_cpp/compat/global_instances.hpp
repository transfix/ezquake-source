/*
 * ezQuake C++ Port - Global Instance Accessors
 * 
 * This header provides access to global instances used for C compatibility.
 * The actual instances live in c_bridge.cpp.
 * 
 * NOTE: Prefer dependency injection over these global accessors for new code.
 * These exist primarily for:
 * 1. C compatibility functions
 * 2. Legacy code migration
 * 3. Static initialization contexts where DI isn't available
 * 
 * Copyright (C) 2024 ezQuake Team
 */

#ifndef EZQUAKE_COMPAT_GLOBAL_INSTANCES_HPP
#define EZQUAKE_COMPAT_GLOBAL_INSTANCES_HPP

namespace ezquake {

// Forward declarations
class Console;

/// Get the default console instance (for C compatibility)
/// The console instance is owned by c_bridge.cpp
Console& getDefaultConsole();

} // namespace ezquake

#endif // EZQUAKE_COMPAT_GLOBAL_INSTANCES_HPP
