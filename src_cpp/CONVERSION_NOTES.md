# ezQuake C to C++20 Conversion Notes

## Table of Contents

1. [Overview](#overview)
2. [Design Principles](#design-principles)
3. [Converted Modules](#converted-modules)
   - [Iteration 1: Core Types and Math Library](#iteration-1-core-types-and-math-library)
   - [Iteration 2: Cvar System](#iteration-2-cvar-system)
   - [Iteration 3: Command System](#iteration-3-command-system)
   - [Iteration 4: Console System](#iteration-4-console-system)
   - [Iteration 5: Thread Safety & Dependency Injection](#iteration-5-thread-safety--dependency-injection)
   - [Iteration 6: Filesystem](#iteration-6-filesystem)
   - [Iteration 7: Network](#iteration-7-network)
   - [Iteration 8: Common Utilities](#iteration-8-common-utilities)
   - [Iteration 9: Client System Foundation](#iteration-9-client-system-foundation)
   - [Iteration 10: Client State](#iteration-10-client-state)
   - [Iteration 11: Input System Foundation](#iteration-11-input-system-foundation)
   - [Iteration 12: Client Prediction](#iteration-12-client-prediction)
   - [Iteration 13: Protocol and Message Parsing](#iteration-13-protocol-and-message-parsing)
   - [Iteration 14: Connection State and Server Lists](#iteration-14-connection-state-and-server-lists)
   - [Iteration 15: Demo System](#iteration-15-demo-system)
   - [Iteration 16: View and Camera System](#iteration-16-view-and-camera-system)
   - [Iteration 17: Temporary Entities](#iteration-17-temporary-entities)
   - [Iteration 18: Server Core](#iteration-18-server-core)
   - [Iteration 19: Server Physics](#iteration-19-server-physics)
   - [Iteration 20: Server Networking](#iteration-20-server-networking)
   - [Iteration 21: Model Types](#iteration-21-model-types)
   - [Iteration 22: Texture Types](#iteration-22-texture-types)
   - [Iteration 23: Render State Types](#iteration-23-render-state-types)
   - [Iteration 24: Particle Types](#iteration-24-particle-types)
   - [Iteration 25: Lightmap Types](#iteration-25-lightmap-types)
   - [Iteration 26: HUD/UI Types](#iteration-26-hudui-types)
   - [Iteration 27: Sound System Types](#iteration-27-sound-system-types)
   - [Iteration 28: Archive/Wad Types](#iteration-28-archivewad-types)
   - [Iteration 29: Sprite/Sky Types](#iteration-29-spritesky-types)
   - [Iteration 30: QuakeC VM Types](#iteration-30-quakec-vm-types)
   - [Iteration 31: Menu System Types](#iteration-31-menu-system-types)
   - [Iteration 32: Config System Types](#iteration-32-config-system-types)
   - [Iteration 33: BSP/Map Types](#iteration-33-bspmap-types)
   - [Iteration 34: Browser/Server Types](#iteration-34-browserserver-types)
   - [Iteration 35: Effects Types](#iteration-35-effects-types)
   - [Iteration 36: Ruleset Types](#iteration-36-ruleset-types)
   - [Iteration 37: Teamplay Types](#iteration-37-teamplay-types)
   - [Iteration 38: MVD Types](#iteration-38-mvd-types)
   - [Iteration 39: Video Types](#iteration-39-video-types)
   - [Iteration 40: FragStats Types](#iteration-40-fragstats-types)
   - [Iteration 41: Draw Types](#iteration-41-draw-types)
   - [Iteration 42: Trigger Types](#iteration-42-trigger-types)
   - [Iteration 43: Logging Types](#iteration-43-logging-types)
   - [Iteration 44: Image Types](#iteration-44-image-types)
   - [Iteration 45: Hash Types](#iteration-45-hash-types)
   - [Iteration 46: Tracker Types](#iteration-46-tracker-types)
   - [Iteration 47: Charset Types](#iteration-47-charset-types)
   - [Iteration 48: Shader Types](#iteration-48-shader-types)
   - [Iteration 49: System Types](#iteration-49-system-types)
   - [Iteration 50: Corona Types](#iteration-50-corona-types)
4. [CUDA Acceleration Opportunities](#cuda-acceleration-opportunities)
   - [High Priority](#high-priority-excellent-gpu-candidates)
   - [Medium Priority](#medium-priority-good-gpu-candidates)
   - [Lower Priority](#lower-priority-limited-gpu-benefit)
   - [Proposed CUDA Infrastructure](#proposed-cuda-infrastructure)
5. [C Code Compatibility Testing](#c-code-compatibility-testing)
6. [Type Mapping](#type-mapping)
7. [Namespace Structure](#namespace-structure)
8. [Build System](#build-system)
9. [Next Steps (Planned)](#next-steps-planned)
   - [Iteration 21-25: Rendering Foundation](#iteration-21-25-rendering-foundation)
   - [Iteration 26-30: Rendering Advanced](#iteration-26-30-rendering-advanced)
   - [Iteration 31-35: UI/HUD System](#iteration-31-35-uihud-system)
10. [C Compatibility](#c-compatibility)
11. [Known Limitations](#known-limitations)

---

## Overview

This document tracks the conversion of the ezQuake codebase from C (gnu89) to modern C++20.
The goal is to create a modern, type-safe, RAII-based codebase that can interoperate with
the existing C code during the transition period.

## Progress Statistics

| Metric | Value |
|--------|-------|
| **Original C Codebase** | ~226,000 lines across 305 files |
| **C++ Ported So Far** | ~68,500 lines across 171 files |
| **Completion (by lines)** | ~30.3% |
| **Iterations Completed** | 49 |
| **Unit Tests Passing** | 2499 |
| **Estimated Remaining Iterations** | 4-14 |

### Breakdown by Module Category

| Category | C Lines | Estimated Iterations | Complexity |
|----------|---------|---------------------|------------|
| Client Core (cl_*.c) | 27,500 | 6-8 | High |
| Server (sv_*.c) | 24,000 | 5-7 | High |
| Rendering (gl_*.c, r_*.c) | 18,000 | 6-8 | Very High |
| UI/HUD (hud*, menu*, ez_*) | 33,000 | 5-6 | Medium |
| QuakeC VM (pr_*, vm_*) | 13,800 | 3-4 | Very High |
| Remaining Utilities | 15,000 | 3-4 | Medium |
| Sound/Input | 8,000 | 2-3 | Medium |

*Note: At ~3-5K C lines ported per iteration, full conversion estimated at 30-40 more iterations.*

## Design Principles

1. **Modern C++20**: Use STL containers, algorithms, smart pointers, concepts, std::span
2. **RAII**: All resources managed through object lifetimes
3. **Type Safety**: Strong types replacing raw arrays and void pointers
4. **UTF-8 Native**: Full Unicode support with backwards compatibility to Quake charset
5. **Test-Driven**: Every module has comprehensive GoogleTest unit tests
6. **C Compatibility**: Bridge layer allows gradual migration
7. **Dependency Injection**: Context-based DI for testability
8. **No Singletons**: All core classes use constructor injection. The only global state lives in `compat/c_bridge.cpp` for C interop. Use `CvarRegistry`/`CommandRegistry` instead of `CvarManager::instance()`/`CommandManager::instance()`
9. **Thread Safety**: All shared state protected with appropriate synchronization
10. **GPU-Ready**: Flagged compute-intensive code for optional CUDA acceleration

## Converted Modules

### Iteration 1: Core Types and Math Library

**Date**: 2025

**Files Created**:
- `core/types.hpp` / `core/types.cpp` - Core type definitions
- `core/math/vec3.hpp` / `core/math/vec3.cpp` - Vec3 class
- `core/math/matrix.hpp` / `core/math/matrix.cpp` - Matrix3x3 class
- `core/math/mathlib.hpp` / `core/math/mathlib.cpp` - Math utilities
- `core/memory/allocator.hpp` / `core/memory/allocator.cpp` - Memory allocator
- `core/memory/arena.hpp` / `core/memory/arena.cpp` - Arena allocator
- `core/text/string_utils.hpp` / `core/text/string_utils.cpp` - String utilities
- `core/text/utf8.hpp` / `core/text/utf8.cpp` - UTF-8 support
- `compat/c_bridge.h` / `compat/c_bridge.cpp` - C compatibility layer

**Original C Code Replaced**:

| Original C File/Location | C++ Replacement | Notes |
|-------------------------|-----------------|-------|
| `mathlib.h` - `vec3_t` (float[3]) | `math::Vec3` class | Full class with operations, iterators, constexpr |
| `mathlib.h` - `matrix3x3_t` | `math::Matrix3x3` class | Rotation matrices, factories, operations |
| `mathlib.c` - `VectorNormalize` | `Vec3::normalize()` | Returns old length, handles zero vector |
| `mathlib.c` - `AngleVectors` | `Vec3::toDirections()` | Converts Euler angles to basis vectors |
| `mathlib.c` - `RotatePointAroundVector` | `rotatePointAroundVector()` | Rodrigues' rotation |
| `mathlib.c` - `BoxOnPlaneSide` | `boxOnPlaneSide()` | Optimized plane-box test |
| `mathlib.c` - `R_ConcatRotations` | `Matrix3x3::operator*` | Matrix multiplication |
| `zone.h` - `Q_malloc` etc. | `memory::allocate()` | Optional debug tracking |
| `zone.h` - Hunk allocator | `memory::Arena` class | RAII mark/restore via ArenaScope |
| `q_shared.h` - `Q_atoi`, `Q_atof` | `text::parseInt/parseFloat` | Handles hex, whitespace |
| `q_shared.h` - `Q_strcasecmp` | `text::compareIgnoreCase` | Case-insensitive compare |
| `common.c` - `Com_HashKey` | `text::hash()` | Case-insensitive hash |
| Various - string handling | `core/text/string_utils.hpp` | Modern string operations |
| Various - UTF-8 (new) | `core/text/utf8.hpp` | Full UTF-8 encoding/decoding |

**Test Coverage**: 160 unit tests

| Test Suite | # Tests | Description |
|------------|---------|-------------|
| Vec3Test | 50 | Vector operations, math, utilities |
| Matrix3x3Test | 20 | Matrix operations, factories, inverse |
| MathLibTest | 26 | Planes, angles, random, math utilities |
| TypesTest | 3 | Type sizes |
| ResultTest | 5 | Result<T> error handling |
| SmartPointerTest | 2 | Smart pointer factories |
| ConstantsTest | 2 | Math constants, limits |
| StringUtilsTest | 27 | String operations |
| Utf8Test | 25 | UTF-8 encoding/decoding |

### Iteration 2: Cvar System

**Date**: 2025

**Files Created**:
- `core/cvar/cvar.hpp` / `core/cvar/cvar.cpp` - Console variable system
- `tests/cvar/cvar_test.cpp` - Cvar unit tests

**Original C Code Replaced**:

| Original C File/Location | C++ Replacement | Notes |
|-------------------------|-----------------|-------|
| `cvar.h` - `cvar_t` struct | `Cvar` class | Full OOP with callbacks, latching |
| `cvar.c` - `Cvar_Find` | `CvarManager::find()` | Thread-safe singleton lookup |
| `cvar.c` - `Cvar_Set` | `Cvar::set()` | Type-safe setters, validation |
| `cvar.c` - `Cvar_Register` | `CvarManager::create()` | Modern registration |
| `cvar.h` - flags macros | `CvarFlags` enum class | Bitwise operators |
| Color parsing | `CvarColor` struct | RGB/RGBA from string |

**Key Features**:
- `CvarFlags` enum with 23 flags (ServerInfo, UserInfo, Archive, etc.)
- `CvarColor` for RGBA color values with string parsing
- `Cvar` class with callbacks, latching, ruleset limits
- `CvarManager` singleton for registration and lookup
- `TypedCvar<T>` wrapper for type-safe access (Float, Int32, Bool, String)
- Thread-safe with std::mutex

**Test Coverage**: 61 new tests (221 total)

| Test Suite | # Tests | Description |
|------------|---------|-------------|
| CvarColorTest | 7 | Color parsing and conversion |
| CvarFlagsTest | 4 | Bitwise flag operations |
| CvarTest | 29 | Cvar value setting, callbacks, latching |
| CvarManagerTest | 21 | Registration, lookup, iteration |

### Iteration 3: Command System

**Date**: 2025

**Files Created**:
- `core/cmd/cmd.hpp` / `core/cmd/cmd.cpp` - Command system
- `tests/cmd/cmd_test.cpp` - Command unit tests

**Original C Code Replaced**:

| Original C File/Location | C++ Replacement | Notes |
|-------------------------|-----------------|-------|
| `cmd.h` - `cbuf_t` struct | `CommandBuffer` class | Command buffer with wait support |
| `cmd.h` - `cmd_function_t` | `Command` class | std::function callbacks |
| `cmd.c` - `Cbuf_AddText` | `CommandBuffer::addText()` | Buffer management |
| `cmd.c` - `Cbuf_Execute` | `CommandBuffer::execute()` | Command parsing and execution |
| `cmd.c` - `Cmd_AddCommand` | `CommandManager::addCommand()` | Modern registration |
| `cmd.h` - `alias_t` | `Alias` struct | Alias with flags |
| `cmd.c` - tokenization | `CommandArgs::parse()` | Quote-aware tokenization |

**Key Features**:
- `CommandArgs` class for parsed command arguments
- `CommandFlags` enum (ServerOnly, CheatCmd, UserCreated, Hidden, Forward)
- `Command` class with modern/legacy callback support
- `AliasFlags` enum (Archive, TeamPlay, Server)
- `CommandBuffer` class with wait/insert/add operations
- `CommandManager` singleton for command/alias registration
- Global compatibility functions (Cbuf_AddText, Cmd_ExecuteString, etc.)
- Built-in commands: echo, wait, alias, unalias
- Cvar integration (can set cvars via command line)

**Test Coverage**: 44 new tests (265 total)

| Test Suite | # Tests | Description |
|------------|---------|-------------|
| CommandArgsTest | 11 | Argument parsing, quoting |
| CommandTest | 3 | Command creation, callbacks |
| CommandFlagsTest | 1 | Bitwise flag operations |
| AliasTest | 1 | Alias construction |
| CommandBufferTest | 5 | Buffer operations, wait |
| CommandManagerTest | 23 | Registration, execution, completion |

### Iteration 4: Console System

**Date**: 2025

**Files Created**:
- `core/console/console.hpp` / `core/console/console.cpp` - Console system
- `tests/console/console_test.cpp` - Console unit tests

**Original C Code Replaced**:

| Original C File/Location | C++ Replacement | Notes |
|-------------------------|-----------------|-------|
| `console.h` - `console_t` struct | `ConsoleBuffer` class | Scrollback buffer |
| `console.c` - `Con_Print` | `Console::print()` | Thread-safe singleton |
| `console.c` - `Con_Clear` | `Console::clear()` | Clears buffer |
| `console.c` - `Con_Init` | `Console::init()` | Initialization |
| `console.c` - `Con_CheckResize` | `Console::checkResize()` | Video mode change |
| `console.c` - color codes | `ConsoleColor::fromHex()` | &cRGB color parsing |

**Key Features**:
- `ConsoleColor` struct for RGBA colors with hex parsing
- `ConsoleChar` struct for character + color
- `ConsoleLine` class for a single line with timestamp
- `ConsoleBuffer` class for scrollback with word wrap
- `PrintFlags` enum (NoNotify, Log, NoLog, Skip, NoReset)
- `Console` singleton with:
  - print/printf/printColored methods
  - Scrolling (up, down, page, top, bottom)
  - Visibility toggle
  - Notification system for transparent overlay
  - Log and print callbacks
  - Timestamp support
- Global compatibility functions (Con_Print, Con_Printf, Con_Clear, etc.)

**Test Coverage**: 42 new tests (307 total)

| Test Suite | # Tests | Description |
|------------|---------|-------------|
| ConsoleColorTest | 5 | Color parsing and construction |
| ConsoleCharTest | 2 | Character construction |
| ConsoleLineTest | 5 | Line operations |
| ConsoleBufferTest | 12 | Buffer printing, wrapping |
| PrintFlagsTest | 1 | Bitwise flag operations |
| ConsoleTest | 13 | Console operations |
| GlobalConsoleFunctionTest | 4 | Global C-compatible functions |

### Iteration 5: Thread Safety & Dependency Injection

**Date**: 2025

**Files Modified/Created**:
- `core/memory/allocator.hpp/cpp` - Thread-safe allocation tracking
- `core/memory/arena.hpp/cpp` - Optional thread-safe arena mode
- `core/context.hpp` / `core/context.cpp` - Dependency injection context
- `compat/global_instances.hpp` - Global instance accessors for C compat
- `tests/memory/allocator_thread_test.cpp` - Thread safety tests

**Architectural Changes**:

| Change | Before | After | Notes |
|--------|--------|-------|-------|
| Memory tracking | DEBUG-only, not thread-safe | AllocationHeader with atomic counters | Tracks size in header, allows dealloc without size |
| Arena allocator | Not thread-safe | ThreadSafe mode with shared_mutex | Read/write locks for concurrent access |
| Singletons | Direct static instances | Context-based dependency injection | Testable, thread-safe, no global state |
| Global Console | In console.cpp | In compat/c_bridge.cpp | Only for C compatibility |

**Key Features**:
- `AllocationHeader` stores size + magic number before each allocation
- `Arena` template parameter `ThreadSafe` enables mutex protection  
- `Context` class holds all subsystem instances
- `getDefaultConsole()` provides global access for C bridge only
- Comprehensive thread safety tests with multiple concurrent threads

**Test Coverage**: 28 new tests (335 total)

| Test Suite | # Tests | Description |
|------------|---------|-------------|
| AllocatorThreadSafetyTest | 6 | Concurrent allocation/deallocation |
| ArenaThreadSafetyTest | 6 | Thread-safe arena operations |
| MemoryIntegrationTest | 4 | Multi-system thread coordination |
| AllocationTrackingTest | 7 | Header-based size tracking |
| ArenaAllocatorTest | 5 | Arena basic operations (updated) |

### Iteration 6: Filesystem

**Date**: 2025

**Files Created**:
- `core/fs/virtual_file.hpp` / `core/fs/virtual_file.cpp` - Abstract file interface
- `core/fs/os_file.hpp` / `core/fs/os_file.cpp` - OS file implementation
- `core/fs/memory_file.hpp` / `core/fs/memory_file.cpp` - In-memory file implementation
- `core/fs/filesystem.hpp` / `core/fs/filesystem.cpp` - FileSystem manager
- `core/fs/fs.hpp` - Convenience header
- `tests/fs/fs_test.cpp` - Comprehensive filesystem tests

**Original C Code Replaced**:

| Original C File/Location | C++ Replacement | Notes |
|-------------------------|-----------------|-------|
| `fs.h` - `vfsfile_t` struct | `VirtualFile` abstract class | RAII, polymorphic |
| `fs.c` - `VFS_READ/WRITE/SEEK` | `VirtualFile::read/write/seek()` | Return Expected<T, FileError> |
| `fs.c` - `FS_OpenVFS()` | `FileSystem::openRead/Write()` | Returns unique_ptr |
| `fs.c` - `searchpath_t` | `SearchPath` struct | std::filesystem integration |
| `fs.c` - `FS_FLocateFile()` | `FileSystem::locate()` | Returns optional<FileLocation> |
| `fs.h` - `relativeto_t` enum | `RelativeTo` enum class | Type-safe |
| `fs.h` - `vfserrno_t` | `FileError` enum class | Comprehensive error codes |

**Key Features**:
- `Expected<T, E>` template (C++20 compatible std::expected alternative)
- `VirtualFile` abstract base with read/write/seek/tell/size operations
- `OsFile` - Regular OS file implementation with std::fstream
- `MemoryFile` - In-memory file for decompressed data, testing
- `FileSystem` class with:
  - Multiple search paths with priorities
  - File location caching
  - Home/base directory separation
  - Path security checks (unsafe filename detection)
  - File enumeration with glob patterns
- Thread-safe for concurrent access
- `std::filesystem` integration throughout

**Test Coverage**: 48 new tests (383 total)

| Test Suite | # Tests | Description |
|------------|---------|-------------|
| FileErrorTest | 1 | Error string conversion |
| FileFlagsTest | 1 | Bitwise flag operations |
| MemoryFileTest | 18 | In-memory file operations |
| OsFileTest | 11 | OS file operations |
| FileSystemTest | 15 | Filesystem manager operations |
| MemoryFileThreadSafetyTest | 1 | Concurrent memory file access |
| FileSystemThreadSafetyTest | 1 | Concurrent file system access |

### Iteration 7: Network

**Date**: 2026

**Files Created**:
- `core/net/net_types.hpp` / `core/net/net_types.cpp` - Network type definitions
- `core/net/socket.hpp` / `core/net/socket.cpp` - Cross-platform socket abstraction
- `core/net/netchan.hpp` / `core/net/netchan.cpp` - Reliable message channel
- `core/net/net.hpp` - Convenience header
- `tests/net/net_test.cpp` - Comprehensive network tests

**Original C Code Replaced**:

| Original C File/Location | C++ Replacement | Notes |
|-------------------------|-----------------|-------|
| `net.h` - `netadr_t` struct | `NetAddress` class | IPv4/IPv6-ready, constexpr |
| `net.h` - `netsrc_t` enum | `NetSource` enum class | Type-safe |
| `net.h` - `netadrtype_t` enum | `AddressType` enum class | Type-safe |
| `net.c` - `NET_StringToAdr()` | `NetAddress::parse()` | Returns Expected<> |
| `net.c` - `NET_AdrToString()` | `NetAddress::toString()` | std::string |
| `net.c` - `NET_CompareAdr()` | `NetAddress::operator==` | constexpr |
| `net.c` - socket functions | `Socket` class | RAII UDP/TCP wrapper |
| `net.h` - `netchan_t` struct | `NetChannel` class | Reliable message channel |
| `net.c` - `Netchan_Transmit()` | `NetChannel::transmit()` | Type-safe |
| `net.c` - `Netchan_Process()` | `NetChannel::process()` | Returns bool |
| `net.c` - `loopback_t` struct | `Loopback` class | Local client-server comm |
| `common.c` - `sizebuf_t` | `MessageBuffer` class | Read/write buffer |
| `common.c` - `MSG_Write*()` | `MessageBuffer::write*()` | Type-safe writes |
| `common.c` - `MSG_Read*()` | `MessageBuffer::read*()` | Returns Expected<> |

**Key Features**:
- `NetAddress` class with:
  - IPv4 address parsing and formatting
  - Loopback and local network detection
  - Network/host byte order conversions
  - constexpr construction
- `Socket` class with:
  - RAII socket management (auto-close on destruction)
  - UDP and TCP support
  - Non-blocking I/O
  - Platform abstraction (Windows/Linux)
  - Send/receive with Expected<> error handling
- `NetChannel` class with:
  - Reliable message delivery over UDP
  - Sequence numbers and acknowledgments
  - Rate limiting
  - Latency tracking
  - Out-of-band messaging
- `MessageBuffer` class with:
  - Read/write cursor management
  - Quake coordinate/angle encoding
  - Overflow/underflow detection
  - Type-safe read/write operations
- `Loopback` class for local client-server communication
- Cross-platform: Windows (Winsock2) and Linux/Unix

**Test Coverage**: 44 new tests (427 total)

| Test Suite | # Tests | Description |
|------------|---------|-------------|
| NetErrorTest | 1 | Error string conversion |
| NetAddressTest | 12 | Address parsing, comparison, formatting |
| MessageBufferTest | 14 | Read/write operations |
| LoopbackTest | 4 | Local message passing |
| NetChannelTest | 4 | Channel setup, rate limiting |
| SocketTest | 7 | Socket operations, send/receive |
| NetIntegrationTest | 1 | Integration scenarios |
| SocketThreadSafetyTest | 1 | Concurrent socket access |

### Iteration 8: Common Utilities

**Date**: 2026

**Files Created**:
- `core/common/info_string.hpp` / `core/common/info_string.cpp` - Info string key-value parsing
- `core/common/token_parser.hpp` / `core/common/token_parser.cpp` - Token parsing utilities
- `core/common/command_line.hpp` / `core/common/command_line.cpp` - Command line argument parsing
- `core/common/common.hpp` - Convenience header
- `tests/common/common_test.cpp` - Comprehensive common utility tests

**Original C Code Replaced**:

| Original C File/Location | C++ Replacement | Notes |
|-------------------------|-----------------|-------|
| `common.c` - `Info_ValueForKey()` | `InfoString::get()` | std::optional return |
| `common.c` - `Info_SetValueForKey()` | `InfoString::set()` | Validates key/value |
| `common.c` - `Info_SetValueForStarKey()` | `InfoString::setStarKey()` | Server-controlled keys |
| `common.c` - `Info_RemoveKey()` | `InfoString::remove()` | Simple removal |
| `common.c` - `Info_RemovePrefixedKeys()` | `InfoString::removeWithPrefix()` | Remove by prefix |
| `common.c` - `Info_Print()` | `InfoString::prettyPrint()` | Formatted output |
| `common.c` - `COM_Parse()` | `text::parseToken()` | Returns ParseResult |
| `common.c` - `COM_ParseToken()` | `text::parseToken()` with options | Punctuation support |
| `common.c` - `COM_ParseEx()` | `text::parseToken()` with curly braces | Curly brace blocks |
| `common.c` - `COM_InitArgv()` | `CommandLine` constructor | Full argument parsing |
| `common.c` - `COM_Argc()` | `CommandLine::argc()` | Argument count |
| `common.c` - `COM_Argv()` | `CommandLine::argv()` | Get argument by index |
| `common.c` - `COM_CheckParm()` | `CommandLine::checkParam()` | O(1) lookup |
| `common.c` - `COM_ClearArgv()` | `CommandLine::clearArgv()` | Clear argument |
| `common.c` - `va()` | `ezquake::va()` | Compatible rotating buffer |

**Key Features**:
- `InfoString` class with:
  - Parse and serialize Quake info strings (`\key\value\key2\value2`)
  - O(1) key lookup via std::unordered_map
  - Key/value validation (no backslash, quotes, control chars)
  - Star key (*) support for server-controlled values
  - Prefix removal for clearing server keys
  - Maximum size enforcement
  - Pretty printing for debugging
- `text::parseToken()` with:
  - Quote-aware string parsing
  - Curly brace block support
  - Comment skipping (// and /* */)
  - Punctuation token support
  - Newline preservation option
  - Token type detection (String, Number, Punctuation)
  - Consumed byte tracking for continuation
- `CommandLine` class with:
  - Parse from argc/argv or string vectors
  - O(1) parameter lookup via index
  - Value extraction (single or multiple)
  - QW URL detection and extraction
  - Original command line preservation
  - Plus command (+connect) support

**Test Coverage**: 55 new tests (482 total)

| Test Suite | # Tests | Description |
|------------|---------|-------------|
| InfoStringTest | 19 | Parse, get, set, remove, star keys, validation |
| TokenParserTest | 18 | Quotes, comments, punctuation, numbers |
| CommandLineTest | 15 | argc/argv, params, values, QW URLs |
| VaTest | 3 | Formatting, rotating buffer |

### Iteration 9: Client System Foundation

**Date**: 2026

**Files Created**:
- `core/client/client_types.hpp` - Core client types, enums, and small classes
- `core/client/player_state.hpp` - PlayerState and PlayerInfo classes
- `core/client/frame.hpp` - Frame, EntityState, CEntity, PacketEntities, FrameBuffer
- `tests/client/client_types_test.cpp` - Comprehensive client type tests

**Original C Code Replaced**:

| Original C File/Location | C++ Replacement | Notes |
|-------------------------|-----------------|-------|
| `protocol.h` - `usercmd_t` | `UserCommand` class | Builder pattern, button queries |
| `protocol.h` - button defines | `client::Button` enum | Scoped enum with bitwise ops |
| `client.h` - `cshift_t` | `ColorShift` class | Decay animation |
| `client.h` - `dlight_t` | `DynamicLight` class | Update/decay/isActive |
| `client.h` - `static_sound_t` | `StaticSound` class | Ambient sound position |
| `client.h` - `lightstyle_t` | `LightStyle` class | Animated light pattern |
| `client.h` - `player_state_t` | `PlayerState` class | Antilag debug, move types |
| `client.h` - `player_info_t` | `PlayerInfo` class | Stats, colors, FPS tracking |
| `client.h` - `entity_state_t` | `EntityState` class | Network entity state |
| `client.h` - `centity_t` | `CEntity` class | Interpolation support |
| `client.h` - `packet_entities_t` | `PacketEntities` class | Vector-based, find by number |
| `client.h` - `frame_t` | `Frame` class | RTT calculation, validity |
| `client.h` - connection states | `client::ConnectionState` enum | Disconnected→Active |
| `client.h` - game types | `client::GameType` enum | Coop, Deathmatch |
| `client.h` - gender defines | `client::Gender` enum | Player gender |
| `client.h` - camera modes | `client::CameraMode` enum | None, Track |
| `client.h` - light types | `client::DynamicLightType` enum | Muzzleflash, Explosion, etc. |
| `client.h` - download types | `client::DownloadType` enum | Model, Sound, Skin |
| `client.h` - demo seeking | `client::DemoSeekType` enum | Normal, DemoMark, Status |

**Key Features**:
- `UserCommand` class with:
  - Builder pattern (`withMsec().withForwardMove()`)
  - Button state queries (`isAttacking()`, `isJumping()`)
  - Constexpr operations for compile-time optimization
  - Comparison operators
- `PlayerState` class with:
  - Movement type enum (Normal, Fly, NoClip, Dead, Spectator)
  - Antilag debug position tracking
  - FTE extension support (alpha, colormod)
  - Convenience methods (`isDead()`, `isSpectator()`)
- `PlayerInfo` class with:
  - Scoreboard stats (frags, ping, packet loss)
  - Color tracking (displayed vs real)
  - Stats array access
  - Health tracking for HUD
  - Authentication info (login name, flag)
- `Frame` class with:
  - Round-trip time calculation
  - Player states for all MAX_CLIENTS
  - Packet entities collection
  - Delta sequence tracking
- `FrameBuffer` ring buffer for UPDATE_BACKUP frames
- `CEntity` with smooth interpolation
- All enums scoped and strongly typed

**Test Coverage**: 48 new tests (530 total)

| Test Suite | # Tests | Description |
|------------|---------|-------------|
| UserCommandTest | 6 | Construction, mutators, builder, buttons |
| ColorShiftTest | 3 | Construction, decay |
| DynamicLightTest | 4 | Properties, update, isActive |
| StaticSoundTest | 2 | Construction |
| LightStyleTest | 3 | Pattern, valueAt |
| PlayerStateTest | 5 | Origin, velocity, move type, clear |
| PlayerInfoTest | 6 | Name, stats, colors |
| EntityStateTest | 2 | Properties |
| CEntityTest | 3 | Baseline, current, interpolation |
| FrameTest | 6 | Timing, command, player states |
| FrameBufferTest | 3 | Size, access, clear |
| PacketEntitiesTest | 3 | Add, find, clear |
| ButtonFlagsTest | 2 | Bitwise operations |

### Iteration 10: Client State

**Date**: 2026

**Files Created**:
- `core/client/client_persistent.hpp` - Persistent client state classes
- `core/client/client_state.hpp` - Per-level client state classes
- Extended `tests/client/client_types_test.cpp` with new test suites

**Original C Code Replaced**:

| Original C File/Location | C++ Replacement | Notes |
|-------------------------|-----------------|-------|
| `client.h` - `clientPersistent_t` | `ClientPersistent` class | Survives server changes |
| `client.h` - `clientState_t` | `ClientState` class | Wiped at level change |
| `client.h` - download tracking | `DownloadState` class | Progress, rate, temp files |
| `client.h` - upload tracking | `UploadState` class | Progress, rate |
| `client.h` - demo state | `DemoState` class | Record/playback/timedemo |
| `client.h` - qtv state | `QtvState` class | QuakeTV extension state |
| `client.h` - protocol extensions | `ProtocolExtensions` class | FTE/MVD extensions |
| `client.h` - FPS tracking | `PerformanceStats` class | FPS, latency stats |
| `client.h` - serverinfo parsing | `ServerInfo` class | Game settings |
| `client.h` - sequence tracking | `SequenceState` class | Delta compression |
| `client.h` - view state | `ViewState` class | Angles, simulation |
| `client.h` - movement vars | `MovementParams` class | Gravity, maxspeed |
| `client.h` - spectator state | `SpectatorState` class | Camera tracking |
| `client.h` - game flags | `GameStateFlags` class | Pause, intermission |

**Key Features**:
- `DownloadState` class with:
  - Download type (Model, Sound, Skin, VWep)
  - Progress percentage and transfer rate
  - Temp file name management
  - Start/finish/cancel lifecycle
- `UploadState` class with:
  - Position-based progress tracking
  - Rate calculation
  - File vs data upload distinction
- `DemoState` class with:
  - Recording/playback state
  - MVD/QTV playback support
  - Seeking with seek type enum
  - Rewinding for demo seeking
  - Timedemo modes (Classic, FixedFps)
- `PerformanceStats` class with:
  - FPS and min FPS tracking
  - Frametime averaging
  - Latency tracking
- `QtvState` class with:
  - Server version tracking
  - ezQuake extension flags
  - Buffering control
- `ProtocolExtensions` class with:
  - FTE1, FTE2, MVD1 extension flags
  - Extension query methods
- `ServerInfo` class with:
  - Server count and protocol version
  - Game type, deathmatch, teamplay
  - Time/frag limits
  - FPD, Z_EXT flags
  - Pitch limits, max FPS
- `SequenceState` class with:
  - Parse count with mod calculation
  - Valid/delta sequence tracking
  - Old sequence for delta validation
- `ViewState` class with:
  - View angles, sim position/velocity
  - Movement state (onground, crouch, waterlevel)
  - Pitch drifting parameters
  - View effects (punch, roll, blur)
  - Color shifts array
- `MovementParams` class with:
  - Entity gravity
  - Max speed
  - Bunny speed cap
- `SpectatorState` class with:
  - Camera mode (None, Track)
  - Tracked player index
  - Locked state
- `GameStateFlags` class with:
  - Pause flags (Server, Demo)
  - Standby, countdown, racing states
  - Intermission tracking
- `ClientState` class with:
  - Timing (time, servertime, gametime)
  - Player identification
  - Level name, CD track, checksum
  - Stats array access
  - All sub-state accessors
  - Player array for all clients
  - Static sounds vector
  - Light styles array
  - Dynamic light allocation
- `ClientPersistent` class with:
  - Connection state machine
  - Netchan for networking
  - Server address
  - All persistent sub-states

**Test Coverage**: 39 new tests (569 total)

| Test Suite | # Tests | Description |
|------------|---------|-------------|
| DownloadStateTest | 4 | Start, progress, complete |
| DemoStateTest | 4 | Recording, playback, seeking |
| PerformanceStatsTest | 2 | FPS, latency |
| QtvStateTest | 2 | Server version, source |
| ClientPersistentTest | 3 | State, connection cycle, clear |
| ServerInfoTest | 2 | Default, setters |
| SequenceStateTest | 2 | Parse count, valid sequence |
| ViewStateTest | 3 | Angles, simulation, color shift |
| MovementParamsTest | 2 | Defaults, setters |
| SpectatorStateTest | 2 | Default, tracking |
| GameStateFlagsTest | 2 | Pause, intermission |
| ClientStateTest | 11 | Timing, players, sounds, lights, clear |

### Iteration 11: Input System Foundation

**Date**: 2026

**Files Created**:
- `core/input/input_types.hpp` - KeyCode enum, KeyButton class, InputButton enum
- `core/input/input_state.hpp` - InputState manager, bindings, mouse config
- `core/input/key_names.hpp` - Key name to code conversion utilities
- `core/input/input.hpp` - Unified input header
- `tests/input/input_test.cpp` - Comprehensive input system tests

**Original C Code Replaced**:

| Original C File/Location | C++ Replacement | Notes |
|-------------------------|-----------------|-------|
| `keys.h` - `keynum_t` enum | `KeyCode` enum class | ~100 key codes with helpers |
| `keys.h` - `keydest_t` | `KeyDestination` enum | Game, Console, Menu, etc. |
| `keys.h` - `chat_type` | `ChatType` enum | Say, SayTeam, IRC |
| `keys.h` - `mouse_state_t` | `MouseState` struct | Position, delta, buttons |
| `input.h` - `kbutton_t` struct | `KeyButton` class | Multi-key tracking, edge detect |
| `cl_input.c` - button globals | `InputButton` enum | Forward, Attack, Jump, etc. |
| `keys.c` - key bindings | `KeyBinding` class | Command strings per key |
| `keys.c` - `Key_StringToKeynum` | `stringToKeyCode()` | Case-insensitive lookup |
| `keys.c` - `Key_KeynumToString` | `keyCodeToString()` | Key name generation |
| `cl_input.c` - speed cvars | `MovementSpeeds` struct | Speed configuration |
| `cl_input.c` - mouse cvars | `MouseConfig` struct | Sensitivity, accel |

**Key Features**:
- `KeyCode` enum class with:
  - All keyboard keys (Tab through F12)
  - Mouse buttons (Mouse1-Mouse8, wheel)
  - Joystick buttons (Joy1-4, POV, Aux1-28)
  - Helper functions: `isMouseButton()`, `isModifierKey()`, `isFunctionKey()`
- `KeyButton` class with:
  - Multi-key tracking (two keys can hold same button)
  - Edge-triggered state (just pressed, just released)
  - Down/up time tracking
  - `calculateKeyState()` for smooth movement
  - Clear edge states after processing
- `InputButton` enum with:
  - All movement buttons (Forward, Back, MoveLeft, etc.)
  - Action buttons (Attack, Attack2, Jump, Use)
  - Look buttons (LookUp, LookDown, MLook, KLook)
  - Modifier buttons (Speed, Strafe)
  - Name conversion functions
- `KeyBinding` class with:
  - Command string storage
  - Empty check
- `InputState` class with:
  - Button state array for all InputButtons
  - Key down/up state tracking
  - Key repeat counters
  - Key binding array
  - Key destination management
  - Impulse command tracking
  - Mouse state management
  - Movement and mouse configuration
  - Mouse look detection
- Key name utilities with:
  - Case-insensitive string to keycode
  - Keycode to string conversion
  - Short names for display
  - Bindability check

**Test Coverage**: 45 new tests (614 total)

| Test Suite | # Tests | Description |
|------------|---------|-------------|
| KeyCodeTest | 6 | Mouse, joystick, function, modifier detection |
| KeyDestinationTest | 1 | Startup demo mode check |
| MouseStateTest | 4 | Position, delta, buttons, clear |
| KeyButtonTest | 9 | Down/up, multi-key, edges, state calc |
| InputButtonTest | 2 | Button names, parsing |
| KeyNamesTest | 7 | String to code, code to string, round-trip |
| InputStateTest | 14 | Buttons, keys, bindings, destination |
| MovementSpeedsTest | 1 | Default values |
| MouseConfigTest | 1 | Default values |

### Iteration 12: Client Prediction

**Date**: 2026

**Files Created**:
- `core/prediction/pmove_types.hpp` - Player movement types, enums, TraceResult, MoveVars, PlayerMoveState
- `core/prediction/player_movement.hpp` - PlayerMovement physics, TraceProvider interface, SimpleTraceProvider
- `core/prediction/crouch_lerp.hpp` - CrouchCalculator, LerpMoveCalculator for view smoothing
- `core/prediction/prediction.hpp` - Unified prediction header
- `tests/prediction/prediction_test.cpp` - Comprehensive prediction system tests

**Original C Code Replaced**:

| Original C File/Location | C++ Replacement | Notes |
|-------------------------|-----------------|-------|
| `pmove.h` - `pmtype_t` enum | `PMoveType` enum class | Normal, Spectator, Dead, Fly, Lock |
| `pmove.h` - `movevars_t` struct | `MoveVars` struct | Gravity, friction, speed limits |
| `pmove.h` - `playermove_t` struct | `PlayerMoveState` class | Full movement state |
| `pmove.c` - `PM_PlayerMove()` | `PlayerMovement::playerMove()` | Main movement entry point |
| `pmove.c` - `PM_AirMove()` | `PlayerMovement::airMove()` | Air strafing physics |
| `pmove.c` - `PM_WaterMove()` | `PlayerMovement::waterMove()` | Swimming physics |
| `pmove.c` - `PM_FlyMove()` | `PlayerMovement::flyMove()` | Noclip/fly movement |
| `pmove.c` - `PM_SpectatorMove()` | `PlayerMovement::spectatorMove()` | Spectator mode |
| `pmove.c` - `PM_CategorizePosition()` | `PlayerMovement::categorizePosition()` | Ground/water detection |
| `pmove.c` - `PM_CheckJump()` | `PlayerMovement::checkJump()` | Jump logic |
| `pmove.c` - `PM_CheckWaterJump()` | `PlayerMovement::checkWaterJump()` | Water jump detection |
| `pmove.c` - `PM_SlideMove()` | `PlayerMovement::slideMove()` | Collision sliding |
| `pmove.c` - `PM_StepSlideMove()` | `PlayerMovement::stepSlideMove()` | Stair stepping |
| `pmove.c` - `PM_Friction()` | `PlayerMovement::friction()` | Ground friction |
| `pmove.c` - `PM_Accelerate()` | `PlayerMovement::accelerate()` | Ground acceleration |
| `pmove.c` - `PM_AirAccelerate()` | `PlayerMovement::airAccelerate()` | Air control |
| `pmove.c` - `PM_ClipVelocity()` | `clipVelocity()` | Velocity clipping against plane |
| `pmove.c` - `PM_NudgePosition()` | `PlayerMovement::nudgePosition()` | Unstick from walls |
| `pmovetst.c` - `PM_PlayerTrace()` | `TraceProvider::playerTrace()` | Trace interface |
| `pmovetst.c` - `PM_PointContents()` | `TraceProvider::pointContents()` | Contents check |
| `cl_pred.c` - `CL_CalcCrouch()` | `CrouchCalculator` class | Stair step view smoothing |
| `cl_pred.c` - `CL_LerpMove()` | `LerpMoveCalculator` class | Position interpolation |
| `cl_pred.c` - `CL_PredictUsercmd()` | `ClientPredictor::predictUserCmd()` | Prediction (optional) |

**Key Features**:
- `PMoveType` enum class with:
  - Normal, OldSpectator, Spectator, Dead, Fly, None, Lock modes
  - Conversion to/from integers
- `Blocked` flags with:
  - Floor, Step, Wall flags for collision response
  - Bitwise operators for flag combining
- `WaterLevel` enum with:
  - None, Feet, Waist, Eyes levels
  - Comparison operators
- `Contents` enum with:
  - Empty, Solid, Water, Slime, Lava, Sky
  - `isLiquid()` helper function
- `Plane` struct with:
  - Normal vector and distance
  - `distanceTo()`, `isInFront()` methods
- `TraceResult` struct with:
  - Start/end positions, fraction, plane hit
  - Entity number, all-solid/start-solid flags
  - `didHit()`, `isStuck()`, `hitFloor()`, `hitWall()` helpers
- `MoveVars` struct with:
  - Gravity, stopSpeed, maxSpeed, friction
  - Accelerate, airAccelerate, waterAccelerate
  - Spectator speed, bunny hop, slide fix
  - Water friction, step size
  - `effectiveGravity()` calculated property
- `PlayerMoveState` class with:
  - Origin, velocity, angles, view offset
  - Move type, water level, on-ground flag
  - User command, touch list, physics entities
  - Jump tracking (msec, held, water jump time)
  - Full getter/setter interface
- `clipVelocity()` function with:
  - Velocity clipping against plane normal
  - Overbounce factor for collision response
  - Epsilon snapping to prevent oscillation
- `TraceProvider` abstract interface with:
  - `playerTrace()` - Player hull traces
  - `lineTrace()` - Line segment traces
  - `pointContents()` - Point contents check
  - `testPlayerPosition()` - Collision test
- `SimpleTraceProvider` for testing with:
  - Configurable ground plane height
  - Solid below ground detection
- `PlayerMovement` class with:
  - `playerMove()` - Main movement simulation
  - `categorizePosition()` - Ground/water detection
  - `slideMove()` - Collision response
  - `stepSlideMove()` - Stair step handling
  - `checkJump()` - Jump execution with capping
  - `checkWaterJump()` - Water exit detection
  - `friction()` - Ground friction application
  - `accelerate()` - Ground acceleration
  - `airAccelerate()` - Air strafing physics
  - `waterMove()`, `flyMove()`, `airMove()`, `spectatorMove()`
  - `nudgePosition()` - Unstick from geometry
- `CrouchCalculator` class with:
  - Stair step-up view smoothing
  - Teleport detection and reset
  - Falling recovery handling
  - Extra crouch for tall steps
- `LerpMoveCalculator` class with:
  - Position interpolation between samples
  - Angle interpolation with wrap-around
  - Teleport detection (>100 unit jumps)
  - Sequence reset handling
- `ClientPredictor` class (conditional compilation) with:
  - Full `PlayerState` integration (when available)
  - `predictUserCmd()` for client prediction
  - Long move splitting for accuracy

**Test Coverage**: 66 new tests (680 total)

| Test Suite | # Tests | Description |
|------------|---------|-------------|
| PMoveTypeTest | 4 | Enum values, integer conversion |
| BlockedFlagsTest | 4 | Flag operations, combining |
| WaterLevelTest | 3 | Comparison operators |
| ContentsTest | 3 | Liquid detection |
| PlaneTest | 4 | Distance, front check |
| TraceResultTest | 6 | Hit detection, floor/wall |
| MoveVarsTest | 4 | Defaults, effective gravity |
| PlayerMoveStateTest | 9 | Origin, velocity, moveType, touch list |
| ClipVelocityTest | 5 | Parallel, floor, wall, corner clipping |
| SimpleTraceProviderTest | 3 | Trace provider testing |
| PlayerMovementTest | 7 | Ground, gravity, jump, spectator, friction |
| CrouchCalculatorTest | 7 | Stair climb, teleport, reset, recovery |
| LerpMoveCalculatorTest | 7 | Samples, interpolation, teleport |
| PhysEntityTest | 2 | Default construction, bounds |
| PhysicsNormalTest | 3 | Normal construction, flags |
| PredictionIntegrationTest | 4 | Full sequences, jump/land, strafe |

### Iteration 13: Protocol and Message Parsing

**Date**: 2026

**Files Created**:
- `core/protocol/protocol_types.hpp` - Protocol constants, ServerCommand, ClientCommand enums, extension flags
- `core/protocol/message_reader.hpp` - MessageReader/MessageWriter classes for network I/O
- `core/protocol/entity_state.hpp` - NetworkEntityState, NetworkPacketEntities, NetworkPlayerState, NetworkEntityBaselines
- `core/protocol/protocol.hpp` - Unified protocol header
- `tests/protocol/protocol_test.cpp` - Comprehensive protocol system tests

**Original C Code Replaced**:

| Original C File/Location | C++ Replacement | Notes |
|-------------------------|-----------------|-------|
| `protocol.h` - `svc_*` defines | `ServerCommand` enum class | 50+ server message types |
| `protocol.h` - `clc_*` defines | `ClientCommand` enum class | 10 client message types |
| `protocol.h` - `PROTOCOL_VERSION` | `protocol::VERSION` | Protocol version 28 |
| `protocol.h` - `PF_*` flags | `PlayerFlags` namespace | Player info update flags |
| `protocol.h` - `U_*` flags | `EntityFlags` namespace | Entity update flags |
| `protocol.h` - `CM_*` flags | `CommandFlags` namespace | User command flags |
| `protocol.h` - `DF_*` flags | `DemoFlags` namespace | Demo recording flags |
| `protocol.h` - `Z_EXT_*` flags | `ZExtensions` namespace | ZQuake protocol extensions |
| `protocol.h` - `FTE_PEXT_*` flags | `FTEExtensions` namespace | FTE protocol extensions |
| `protocol.h` - `MVD_PEXT*` flags | `MVDExtensions` namespace | MVD extensions |
| `protocol.h` - `TE_*` values | `TempEntityType` enum class | Temp entity types |
| `com_msg.c` - `MSG_ReadByte()` | `MessageReader::readByte()` | Safe buffer reading |
| `com_msg.c` - `MSG_ReadShort()` | `MessageReader::readShort()` | Little-endian shorts |
| `com_msg.c` - `MSG_ReadLong()` | `MessageReader::readLong()` | Little-endian longs |
| `com_msg.c` - `MSG_ReadFloat()` | `MessageReader::readFloat()` | IEEE float reading |
| `com_msg.c` - `MSG_ReadString()` | `MessageReader::readString()` | Null-terminated strings |
| `com_msg.c` - `MSG_ReadCoord()` | `MessageReader::readCoord()` | Short/Float coord modes |
| `com_msg.c` - `MSG_ReadAngle()` | `MessageReader::readAngle()` | Byte/Short angle modes |
| `com_msg.c` - `MSG_WriteByte()` | `MessageWriter::writeByte()` | Safe buffer writing |
| `com_msg.c` - `MSG_WriteShort()` | `MessageWriter::writeShort()` | Little-endian shorts |
| `com_msg.c` - `MSG_WriteCoord()` | `MessageWriter::writeCoord()` | Coord encoding |
| `quakedef.h` - `entity_state_t` | `NetworkEntityState` class | Entity network state |
| `cl_ents.c` - `packet_entities_t` | `NetworkPacketEntities` class | Entity collection |
| `cl_ents.c` - baseline arrays | `NetworkEntityBaselines` class | Baseline management |
| `cl_parse.c` - player_state_t | `NetworkPlayerState` class | Player network state |

**Key Features**:
- `protocol` namespace with:
  - VERSION = 28 (QuakeWorld protocol)
  - PORT_MASTER, PORT_CLIENT, PORT_SERVER constants
  - VERSION_FTE, VERSION_FTE2, VERSION_MVD magic numbers
- `ServerCommand` enum class with:
  - 50+ commands (Bad through FTE_VoiceChat)
  - `serverCommandName()` for debug output
- `ClientCommand` enum class with:
  - Move, StringCmd, Delta, Upload commands
  - `clientCommandName()` for debug output
- `PlayerFlags` namespace with:
  - MSEC, COMMAND, VELOCITY, MODEL, DEAD, ONGROUND, GIB flags
  - PMC_SHIFT, PMC_MASK for move type extraction
  - `extractMoveType()` helper function
- `EntityFlags` namespace with:
  - ORIGIN1/2/3, ANGLE1/2/3, FRAME, MODEL flags
  - REMOVE, MOREBITS, ENTITY_MASK for delta encoding
  - `extractEntityNum()` helper function
- `ZExtensions` namespace with:
  - PM_TYPE, VIEWHEIGHT, SERVERTIME, VWEP extensions
  - CLIENT/SERVER composite extension sets
- `FTEExtensions` namespace with:
  - TRANS, FLOATCOORDS, ENTITYDBL, CHUNKEDDOWNLOADS
  - MODELDBL, COLOURMOD, SPLITSCREEN
- `MVDExtensions` namespace with:
  - FLOATCOORDS, HIGHLAGTELEPORT, HIDDEN_MESSAGES
- `CoordMode` enum (Short: 1/8 precision, Float: full)
- `AngleMode` enum (Byte: 256 steps, Short: 65536 steps)
- `MessageReader` class with:
  - Buffer management: reset(), rewind(), seek(), skip()
  - Primitive reads: readChar(), readByte(), readShort(), readLong(), readFloat()
  - String reads: readString(), readLine() with length limits
  - Protocol reads: readCoord(), readAngle(), readCoordVec(), readAngleVec()
  - Utility: peekByte(), readBytes(), readBytesView(), readEntityWord()
  - Error handling: badRead(), clearError(), atEnd()
- `MessageWriter` class with:
  - writeByte(), writeShort(), writeUShort(), writeLong(), writeFloat()
  - writeString(), writeCoord(), writeAngle()
  - clear(), setMaxSize(), overflow() detection
- `NetworkEntityState` class with:
  - Entity number, origin, angles, model/frame/colormap/skin/effects
  - FTE extensions: alpha, scale, colormod
  - `diffFlags()` for delta compression
  - `copyFrom()`, `clear()`, `isValid()` helpers
- `NetworkPacketEntities` class with:
  - MAX_ENTITIES = 300 collection
  - add(), find(), clear(), iteration support
- `NetworkPlayerState` class with:
  - Origin, velocity, viewAngles, cmdAngles
  - Model, frame, skinnum, effects, weaponFrame
  - Movement: forwardMove, sideMove, upMove
  - Flags, buttons, impulse, msec
  - isDead(), onGround(), isGib(), moveType() helpers
- `NetworkEntityBaselines` class with:
  - MAX_EDICTS = 2048 baseline storage
  - get(), set() for baseline management
  - Entity number wrapping

**Test Coverage**: 80 new tests (760 total)

| Test Suite | # Tests | Description |
|------------|---------|-------------|
| ProtocolConstantsTest | 3 | Version, ports, FTE magic |
| ServerCommandTest | 4 | Basic commands, entity commands, names |
| ClientCommandTest | 2 | Basic commands, names |
| PlayerFlagsTest | 2 | Flag values, move type extraction |
| EntityFlagsTest | 2 | Update flags, entity number extraction |
| CommandFlagsTest | 1 | Move command flags |
| ZExtensionsTest | 1 | Extension flags |
| PrintLevelTest | 1 | Print levels |
| TempEntityTypeTest | 1 | Temp entity types |
| NetworkLimitsTest | 1 | Network limits |
| MessageReaderTest | 24 | All read operations, buffer management |
| MessageWriterTest | 11 | All write operations, overflow |
| ProtocolEntityStateTest | 10 | Entity state operations |
| ProtocolPacketEntitiesTest | 5 | Packet entity operations |
| NetworkPlayerStateTest | 6 | Player state operations |
| EntityBaselinesTest | 3 | Baseline management |
| ProtocolIntegrationTest | 3 | Round-trip, simulation, precision |

---

### Iteration 14: Connection State and Server Lists

**Date**: 2026

**Files Created**:
- `core/connection/connection_state.hpp` - Connection state enums, download types, protocol extensions
- `core/connection/server_list.hpp` - ServerEntry, ServerList, ServerListManager classes
- `core/connection/connection.hpp` - Unified connection header
- `tests/connection/connection_test.cpp` - Comprehensive connection tests

**Original C Code Replaced**:

| Original C File/Location | C++ Replacement | Notes |
|-------------------------|-----------------|-------|
| `client.h` - `cactive_t` enum | `ConnectionState` enum class | 5 states: Disconnected, DemoStart, Connected, OnServer, Active |
| `client.h` - `dltype_t` enum | `DownloadType` enum class | None, Model, VwepModel, Sound, Skin, Single |
| `client.h` - `demopacket_*` | `DownloadMethod` enum | None, Standard, Chunked |
| `client.h` - `demoseekingtype_t` | `DemoSeekingType` enum class | None through FoundNoRewind |
| `client.h` - `DEMOSEEKINGMATCH_*` | `DemoSeekMatchType` enum class | Equal, NotEqual, LessThan, etc. |
| `client.h` - `mvd_playback_t` | `MvdPlayback` enum class | None, File, Qtv |
| `cl_demo.c` - `TIMEDEMO_*` | `TimedemoMode` enum class | Off, Classic, FixedFps |
| `cl_demo.c` - timedemo consts | `timedemo` namespace | DEFAULT_FPS=308, MIN_FPS=20, MAX_FPS=10000 |
| `cl_main.c` - frame timing | `ConnectionTiming` struct | realtime, demotime, frametime, frameCount |
| `cl_main.c` - FTE extensions | `ProtocolExtensions` struct | fte1, fte2, mvd1 extension fields |
| `cl_slist.c` - server entry | `ServerEntry` class | Address + optional description |
| `cl_slist.c` - server list | `ServerList` class | 512 max entries, file I/O, sorting |
| `cl_slist.c` - slist macros | `server_list` namespace | MAX_ENTRIES, DEFAULT_FILENAME, FIELD_SEPARATOR |
| `cl_slist.h` - favorites | `CategorizedServerList` class | Category + named list |
| `cl_slist.h` - server manager | `ServerListManager` class | Favorites, Recent, Custom lists |

**Key Features**:
- `ConnectionState` enum class with:
  - Disconnected (0), DemoStart (1), Connected (2), OnServer (3), Active (4)
  - `connectionStateName()` for debug output
  - `isConnected()` helper (>= Connected)
  - `canRenderFrames()` helper (== Active only)
- `DownloadType` enum class with:
  - None, Model, VwepModel, Sound, Skin, Single
  - `downloadTypeName()` for logging
- `DownloadMethod` enum class:
  - None, Standard, Chunked download modes
- `DemoSeekingType` enum class:
  - None, Normal, DemoMark, Status, Found, End, FoundNoRewind
- `DemoSeekMatchType` enum class:
  - Equal, NotEqual, LessThan, GreaterThan, BitOn, BitOff
- `MvdPlayback` enum class:
  - None, File, Qtv playback modes
- `TimedemoMode` enum class:
  - Off, Classic, FixedFps benchmark modes
- `timedemo` namespace with:
  - DEFAULT_FPS = 308, MIN_FPS = 20, MAX_FPS = 10000
- `ConnectionTiming` struct with:
  - realtime, demotime, trueFrametime, frametime, frameCount
  - `advanceFrame()` with optional demo speed
  - `reset()` to clear all timing
- `ProtocolExtensions` struct with:
  - fte1, fte2, mvd1 extension bitfields
  - `any()` to check if any extensions enabled
  - `clear()` to reset all
- `ServerEntry` class with:
  - address_, description_ strings
  - `isEmpty()`, `isValid()`, `hasDescription()`
  - `matchesAddress()` for lookup
  - `displayString()` for UI
  - `toFileFormat()` and `fromFileFormat()` for persistence
  - Equality based on address only
- `ServerList` class with:
  - `MAX_ENTRIES = 512` from original C code
  - `add()`, `addOrUpdate()`, `set()`, `insert()` methods
  - `remove()`, `removeAt()`, `resetAt()` methods
  - `swap()`, `move()` for reordering
  - `find()`, `contains()`, `indexOf()` for lookup
  - `get()` (pointer), `at()` (optional ref) for access
  - `sortByAddress()`, `sortByDescription()` sorting
  - `countValid()`, `countWithDescriptions()` stats
  - `compact()` to remove empty entries
  - `findIf()` with predicate for filtering
  - `load()`, `save()` file persistence
  - Modified flag tracking
  - Full iterator support (begin/end/cbegin/cend)
- `server_list` namespace constants:
  - `MAX_ENTRIES = 512`
  - `DEFAULT_FILENAME = "servers.txt"`
  - `FIELD_SEPARATOR = '\t'`
- `ServerCategory` enum class:
  - Favorites, Recent, Custom
- `CategorizedServerList` class:
  - Combines ServerList with Category and name
- `ServerListManager` class with:
  - `favorites()` and `recent()` lists
  - `isFavorite()`, `toggleFavorite()` methods
  - `addToRecent()` with auto-promotion to front
  - `getList()` by category
  - `addCustomList()` for user-defined lists
  - `saveAll()`, `loadAll()` for persistence

**Test Coverage**: 84 new tests (844 total)

| Test Suite | # Tests | Description |
|------------|---------|-------------|
| ConnectionStateTest | 4 | Enum values, names, helper functions |
| DownloadTypeTest | 3 | Enum values, names, methods |
| DemoSeekingTypeTest | 2 | Seeking and match type values |
| MvdPlaybackTest | 1 | Playback mode values |
| TimedemoModeTest | 2 | Mode values, constants |
| ConnectionTimingTest | 5 | Init, advance, reset, demo speed |
| ProtocolExtensionsTest | 3 | Default, any(), clear() |
| ServerEntryTest | 15 | Constructor, set, clear, equality, file format |
| ServerListTest | 30 | All CRUD operations, sorting, iteration |
| ServerListFileTest | 6 | Save/load, comments, whitespace handling |
| CategorizedServerListTest | 2 | Category, name, list access |
| ServerListManagerTest | 8 | Favorites, recent, toggle, custom lists |
| ServerListConstantsTest | 3 | MAX_ENTRIES, filename, separator |

---

### Iteration 15: Demo System

**Objective**: Convert the demo recording and playback system from cl_demo.c, cl_nqdemo.c, and demo_controls.c.

**Source Files Analyzed**:
| Original C File | Lines | Description |
|-----------------|-------|-------------|
| cl_demo.c | ~5,500 | Core demo recording/playback, MVD support |
| cl_nqdemo.c | ~1,400 | NetQuake demo format support |
| demo_controls.c | ~460 | Demo playback UI controls |
| protocol.h | (types) | Demo message type constants |

**Files Created**:
| C++ File | Lines | Purpose |
|----------|-------|---------|
| core/demo/demo_types.hpp | ~745 | All demo type definitions |
| core/demo/demo_recorder.hpp | ~205 | Recording interface |
| core/demo/demo_player.hpp | ~490 | Playback interface |
| core/demo/demo.hpp | ~20 | Unified header |
| core/demo/demo_recorder.cpp | ~340 | Recording implementation |
| core/demo/demo_player.cpp | ~430 | Playback implementation |
| tests/demo/demo_test.cpp | ~860 | Comprehensive tests |

**Key Types Converted**:

- `DemoMessageType` enum class:
  - Cmd(0), Read(1), Set(2), Multiple(3), Single(4), Stats(5), All(6)
  - `demoMessageTypeName()` for debugging
  - `isMvdMessageType()` to detect MVD-specific types
- `DemoFormat` enum class:
  - None, Qwd, Mvd, Qwz, Dem, Qtv
  - `demoFormatExtension()` returns file extension
  - `demoFormatName()` for display
  - `detectDemoFormat()` from filename
- `DemoPlaybackMode` enum class:
  - None, Qwd, Nq, MvdFile, MvdQtv
  - `isPlayingDemo()`, `isMvdPlayback()`, `isQtvPlayback()` helpers
- `DemoSeekType` enum class:
  - None, Normal, DemoMark, Status, Found, End, FoundNoRewind
  - `isSeeking()`, `isSeekingComplete()` helpers
- `SeekMatchType` enum class:
  - Equal, NotEqual, LessThan, GreaterThan, BitOn, BitOff
- `SeekCondition` struct with:
  - matchType, targetValue, currentValue
  - `evaluate()` to check if condition is met
- `SeekStatus` struct with:
  - type, conditions vector
  - `isSeeking()`, `isComplete()`, `addCondition()`, `clear()`
- `DemoTiming` struct with:
  - oldTime, nextTime, startTime, currentTime, length
  - speed (0.0-10.0 range, 0.0=paused)
  - `progressPercent()`, `elapsedTime()`, `isPaused()`
  - `setSpeed()` with clamping, `reset()`
- `timedemo` namespace constants:
  - DEFAULT_FPS = 308, MIN_FPS = 20, MAX_FPS = 10000
- `TimedemoMode` enum class:
  - Off, Classic, FixedFps
- `TimedemoStats` struct with:
  - mode, startTime, startFrame, maxFrametimeMs, maxFrametimeFrame
  - frametimeStats[1000] histogram
  - `isActive()`, `recordFrametime()`, `calculateFps()`, `reset()`
- `RecordingMode` enum class:
  - None, Manual, Auto, Easy
- `RecordingState` struct with:
  - mode, filename, bytesWritten
  - `isRecording()`, `reset()`
- `PlaybackState` struct with:
  - mode, format, timing, seekStatus, timedemo
  - paused flag, adjustSpeed setting
  - `isPlaying()`, `isSeeking()`, `isPaused()`, `reset()`
- `QtvState` enum class:
  - Disconnected, Connecting, Connected, Buffering, Playing, Error
- `QtvInfo` struct with:
  - state, source, bufferSize, bufferTarget, serverVersion
  - `isConnected()`, `isPlaying()`, `bufferProgress()`, `reset()`
- `nq_demo` namespace:
  - PROTOCOL_VERSION = 15, MAX_CLIENTS = 16, SIGNONS = 4, MAX_EDICTS = 600
  - `update` sub-namespace with entity update flags
- `NqDemoState` struct with:
  - signonCount, maxClients, maxEdicts
  - `isSignonComplete()`, `reset()`
- `demo_cache` namespace:
  - MIN_SIZE = 2MB, FLUSH_SIZE = 1MB
- `DemoCache` class with:
  - buffer management with configurable size
  - `resize()`, `write()`, `flush()`, `clear()`
  - `needsFlush()`, `available()`, `isEnabled()`
- `DemoHeader` struct with:
  - time (float), type (DemoMessageType)
- `DemoUserCmd` struct with:
  - angles[3], forwardMove, sideMove, upMove
  - buttons, impulse, msec
  - `serialize()`, `deserialize()` little-endian encoding
  - USER_CMD_SIZE = 33 constant
- `DemoRecorder` class with:
  - `start()`, `stop()`, `isRecording()`
  - `writeUserCmd()`, `writeMessage()`, `writeSetMessage()`
  - `setFlushCallback()` for progress notification
  - Optional caching support for write performance
- `AutoRecorder` class with:
  - `enable()`, `disable()`, `isEnabled()`
  - `onMatchStart()`, `onMatchEnd()` hooks
  - `generateFilename()` with timestamp and server name
- `DemoReadResult` struct with:
  - success, endOfDemo flags
  - type, time, data, target
  - Static `ok()`, `eof()`, `error()` factory methods
- `DemoPlayer` class with:
  - `start()`, `stop()`, `isPlaying()`
  - `readMessage()` returns DemoReadResult
  - `setSpeed()`, `pause()`, `resume()` playback control
  - `startSeeking()`, `stopSeeking()`, `rewind()`
  - `startTimedemo()`, `stopTimedemo()`, `recordTimedemoFrame()`
  - `progress()`, `length()`, `format()` info methods
  - `setEndCallback()` for playlist support
- `DemoPlaylist` class with:
  - MAX_DEMOS = 8, MAX_NAME_LENGTH = 16
  - `add()`, `clear()`, `current()`, `next()`, `previous()`
  - `empty()`, `size()`, `index()`, `atEnd()`

**Test Coverage**: 85 new tests (929 total)

| Test Suite | # Tests | Description |
|------------|---------|-------------|
| DemoMessageTypeTest | 3 | Enum values, names, isMvdMessageType |
| DemoFormatTest | 4 | Enum values, extensions, names, detection |
| DemoPlaybackModeTest | 4 | Enum values, isPlaying, isMvd, isQtv |
| DemoSeekTypeTest | 3 | Enum values, isSeeking, isComplete |
| SeekConditionTest | 6 | All match types evaluation |
| SeekStatusTest | 3 | Default, add conditions, clear |
| DemoTimingTest | 7 | Default, progress, elapsed, pause, speed, reset |
| TimedemoStatsTest | 7 | Default, active, record, calculate fps, reset |
| PlaybackStateTest | 2 | isSeeking, pause during seek |
| QtvInfoTest | 6 | Default, connected, playing, buffer, reset |
| NqDemoStateTest | 4 | Default, signon, constants, update flags |
| DemoCacheTest | 10 | Default, resize, write, flush, available |
| DemoUserCmdTest | 3 | Default, serialize/deserialize, short data |
| DemoReadResultTest | 3 | Ok, eof, error factory methods |
| DemoPlaylistTest | 9 | Default, add, current, next, previous, at end |
| AutoRecorderTest | 4 | Default, enable, generate filename |

### Iteration 16: View and Camera System

**Date**: 2025

**Files Created**:
- `core/view/view_types.hpp` - View rendering and screen effect types
- `core/view/camera_types.hpp` - Camera modes and spectator tracking types
- `core/view/view.hpp` - Unified view/camera header

**Source Mapping**:
| C File | Lines | C++ File | Status |
|--------|-------|----------|--------|
| cl_view.c | ~1,227 | view_types.hpp | Types extracted |
| cl_view.h | ~25 | view_types.hpp | Types extracted |
| cl_cam.c | ~1,055 | camera_types.hpp | Types extracted |
| vx_camera.c | ~129 | camera_types.hpp | Types extracted |

**Types Converted**:

*View System (from cl_view.c):*
- `view_bob` namespace:
  - DEFAULT_BOB=0, DEFAULT_BOBCYCLE=0, DEFAULT_BOBUP=0
  - MIN_BOB=-7, MAX_BOB=4
- `view_roll` namespace:
  - DEFAULT_SPEED=200, DEFAULT_ANGLE=0, MAX_ROLL=45
- `view_kick` namespace:
  - DEFAULT_KICKTIME/ROLL/PITCH=0
- `view_idle` namespace:
  - DEFAULT_CYCLE/PITCH/ROLL/YAW=0, MAX_*=10/5/5/5
- `view_height` namespace:
  - DEFAULT=22, GIB=8, DEAD=-16
- `ColorShiftType` enum:
  - Contents(0), Damage(1), Bonus(2), Powerup(3)
  - NUM_COLOR_SHIFTS=4
  - colorShiftTypeName() utility
- `ViewColorShiftState` struct:
  - destColor RGB array, percent (0-150)
  - set(), decay(), alpha(), normalizedColor(), reset(), isActive()
- `color_shifts` namespace:
  - EMPTY_COLOR/PERCENT, WATER_COLOR/PERCENT
  - LAVA_COLOR/PERCENT, SLIME_COLOR/PERCENT
  - Bonus and powerup colors (QUAD, RING, PENT, SUIT)
- `ContentsType` enum:
  - Empty(-1), Solid(-2), Water(-3), Slime(-4), Lava(-5), Sky(-6)
  - getContentsColorShift() factory
- `ViewBlend` struct:
  - RGBA components, addBlend(), toArray(), reset()
- `ViewBob` struct:
  - bob, lastBob, time, airTime
  - calculate() with sine-based bobbing
  - reset()
- `ViewRoll` struct:
  - current, target, speed, angle
  - calculateTarget() with velocity-based roll
  - update() with time-based smoothing
  - reset()
- `DamageKick` struct:
  - pitch, roll, time, kickTime, kickRoll, kickPitch
  - applyDamage(), currentRoll(), currentPitch()
  - update(), reset()
- `PitchDrift` struct:
  - drifting flag, speed, tolerance
  - start(), stop(), update(), reset()
- `IdleSway` struct:
  - pitchScale, yawScale, rollScale, cycle
  - calculate() returns Vec3 angles
  - reset()
- `PunchAngle` struct:
  - current, target, velocity, decayTime
  - apply(), update(), reset()
- `FlashState` struct:
  - gamma, contrast, duration (for TF flashgrenades)
  - apply(), clear()
- `ViewModelState` struct:
  - bobOffset, entity reference
  - shouldDraw(), reset()
- `ViewState` struct:
  - Aggregates all view sub-states
  - colorShifts array, shift() accessors
  - calculateFinalBlend(), update(), reset()

*Camera System (from cl_cam.c, vx_camera.c):*
- `SpectatorCameraMode` enum:
  - None(0), Track(1) (from CAM_NONE, CAM_TRACK)
  - spectatorCameraModeName() utility
- `ExternalCameraMode` enum:
  - Normal(0), ChaseCam(1), External(2)
  - externalCameraModeName() utility
- `camera` namespace:
  - DEFAULT_CHASE_DIST=100, DEFAULT_CHASE_HEIGHT=32
  - SPEC_UPDATE_INTERVAL=0.5f
  - FLYBY_MIN/MAX_DISTANCE=32/800, FLYBY_MOVE_SPEED=64
- `SpectatorTrackingState` struct:
  - mode, userId, locked, autotrack, killerId
  - lock(), unlock(), setKiller(), reset()
  - isTracking(), isLocked()
- `ChaseCameraState` struct:
  - distance, height, pitchOffset
  - isFirstPerson(), shouldShowPlayer(), reset()
- `ExternalCameraState` struct:
  - mode, active, lastPos, lastAngles
  - setNormal(), setChaseCam(), setExternal()
  - isExternal(), reset()
- `FlybyPosition` struct:
  - position, distance
  - isValid(), isBetterThan()
- `MultiviewSlot` struct:
  - active, userId, viewport rect
- `MultiviewState` struct:
  - enabled, layout (2x1, 2x2, 3x1, 3x2, 4x2)
  - slots array, current slot
  - setSlotTrack(), clearSlot(), reset()
- `HighTrackerState` struct:
  - enabled, tolerance
- `CameraInputState` struct:
  - attackPressed, jumpPressed, attackWasPressed, jumpWasPressed
  - attackEdge(), jumpEdge(), reset()
- `CameraState` struct:
  - Aggregates tracking, chase, external, multiview, input
  - isSpectating(), shouldDrawPlayer(), reset()
- `vectorToAngles()` utility function

**Test Coverage**: 97 new tests (1026 total)

| Test Suite | # Tests | Description |
|------------|---------|-------------|
| ViewBobConstantsTest | 1 | Default values |
| ViewRollConstantsTest | 1 | Default values |
| ColorShiftTypeTest | 2 | Enum values, names |
| ViewColorShiftTest | 7 | Default, set, clamp, alpha, normalized, decay, reset |
| ContentsTypeTest | 2 | Enum values, getContentsColorShift |
| ViewBlendTest | 5 | Default, add, multiple, toArray, reset |
| ViewBobTest | 5 | Default, no bob, air, clamp, reset |
| ViewRollTest | 5 | Default, target pos/neg, clamp, update smooth |
| DamageKickTest | 4 | Default, apply, current values, reset |
| PitchDriftTest | 3 | Default, start/stop, update |
| IdleSwayTest | 3 | Default, calculate, zero scale |
| PunchAngleTest | 4 | Default, apply, small kick decay, reset |
| FlashStateTest | 3 | Default, apply, clear |
| ViewModelStateTest | 3 | Default, shouldDraw, invisible flag |
| ViewEffectsStateTest | 4 | Default, shift access, update, reset |
| SpectatorCameraModeTest | 2 | Values, names |
| ExternalCameraModeTest | 2 | Values, names |
| CameraConstantsTest | 1 | Values |
| SpectatorTrackingStateTest | 7 | Default, lock, tracking, locked, unlock, killer, reset |
| ChaseCameraStateTest | 3 | Default, isFirstPerson, shouldShowPlayer |
| ExternalCameraStateTest | 4 | Default, setChaseCam, setExternal, reset |
| FlybyPositionTest | 4 | Default invalid, valid, invalid dist, isBetterThan |
| MultiviewStateTest | 4 | Default, setSlotTrack, clearSlot, reset |
| HighTrackerStateTest | 2 | Default, enabled |
| CameraInputStateTest | 5 | Default, attack pressed/released, jump, reset |
| CameraStateTest | 4 | Default, isSpectator, shouldDrawPlayer, reset |
| VectorToAnglesTest | 4 | Pointing up, down, forward, right |

### Iteration 17: Temporary Entities

**Date**: 2026

**Files Created**:
- `core/entity/temp_entity_types.hpp` - Temporary entity type definitions
- `core/entity/entity.hpp` - Unified entity header

**Source Mapping**:
| C File | Lines | C++ File | Status |
|--------|-------|----------|--------|
| cl_tent.c | ~963 | temp_entity_types.hpp | Types extracted |
| protocol.h (temp_entity_t) | ~20 | temp_entity_types.hpp | Types extracted |
| quakedef.h (trail_type_t) | ~15 | temp_entity_types.hpp | Types extracted |

**Types Converted**:

*Temp Entity Types (from protocol.h):*
- `TempEntityType` enum:
  - Spike(0), SuperSpike(1), Gunshot(2), Explosion(3), TarExplosion(4)
  - Lightning1(5), Lightning2(6), WizSpike(7), KnightSpike(8), Lightning3(9)
  - LavaSplash(10), Teleport(11), Blood(12), LightningBlood(13)
  - tempEntityTypeName(), isLightningType(), isPositionOnlyType()
- `NqTempEntityType` enum (NetQuake-specific)

*Trail Types (from quakedef.h):*
- `TrailType` enum:
  - Rocket, Grenade, AltRocket, Blood, BigBlood
  - Tracer1, Tracer2, Voor, Rail, Rail2
  - Lava, AmfRocket, Bleeding, Bleeding2
  - trailTypeName()

*Explosion Types (from r_explosiontype cvar):*
- `ExplosionType` enum:
  - Normal, Sprite, Teleport, LightningBlood, BigBlood
  - DoubleGunshot, Blob, Detpack, FuelRod, None
  - explosionTypeName()

*Constants (from cl_tent.c):*
- `temp_entity` namespace:
  - MAX_TEMP_ENTITIES=32, MAX_BEAMS=32, MAX_EXPLOSIONS=32
  - BEAM_DURATION=0.2f, BEAM_SEGMENT_LENGTH=30
  - Blood color defaults (LG=225, RL_small=225, RL_big=73, SG=73)
  - SPIKE_PARTICLES=10, SUPERSPIKE_PARTICLES=20

*Saved Temp Entity (from protocol.h):*
- `SavedTempEntity` struct with position, time, type, isExpired()
- `TempEntityHistory` class:
  - Ring buffer for demo replay
  - add(), at(), size(), clear()

*Explosion System (from cl_tent.c):*
- `ExplosionState` struct:
  - origin, startTime, active
  - currentFrame(), isComplete(), start(), clear()
- `ExplosionPool` class:
  - Pool allocator with LRU eviction
  - allocate(), update(), activeCount(), forEachActive(), clear()

*Beam System (from cl_tent.c):*
- `BeamModelType` enum (Bolt1, Bolt2, Bolt3, Beam)
- `BeamState` struct:
  - entity, modelType, endTime, start, end
  - isActive(), length(), direction(), angles()
  - update(), clear()
- `BeamPool` class:
  - Entity-keyed beam management
  - findOrAllocate(), updateBeam(), activeCount()
  - getPlayerBeamEnd() for cl_fakeshaft, forEachActive(), clear()

*Rail Trail System (instagib):*
- `RailColor` enum (White, Blue, Green, LightBlue, Red, Magenta, Yellow)
- railColorToPalette() conversion
- `RailTrailInfo` struct with decode() for entity number encoding
- `InstagibTrailType` enum with instagibToTrailType() conversion

*Spark Effect:*
- `SparkParams` struct with factory methods:
  - wizSpike(), knightSpike(), lightning()

*Combined State:*
- `TempEntityState` struct aggregating beams, explosions, history

**Test Coverage**: 57 new tests (1083 total)

| Test Suite | # Tests | Description |
|------------|---------|-------------|
| TempEntityTypeTest | 5 | Values, names, isLightning, isPositionOnly, types |
| TrailTypeTest | 2 | Values, names |
| ExplosionTypeTest | 2 | Values, names |
| TempEntityConstantsTest | 2 | Values, blood colors |
| SavedTempEntityTest | 2 | Default, isExpired |
| TempEntityHistoryTest | 4 | Default, add, ring buffer, clear |
| ExplosionStateTest | 5 | Default, currentFrame, isComplete, start, clear |
| ExplosionPoolTest | 6 | Default, allocate, multiple, update, forEach, clear |
| BeamStateTest | 9 | Default, isActive, length, direction, angles, update, clear |
| BeamPoolTest | 9 | Default, update, sameEntity, multiple, expire, playerBeam, forEach, clear |
| RailColorTest | 1 | ToPalette |
| RailTrailInfoTest | 4 | DecodeValid, colorBits, invalid, paletteIndex |
| InstagibTrailTypeTest | 1 | ToTrailType |
| SparkParamsTest | 3 | WizSpike, knightSpike, lightning |
| TempEntityStateTest | 3 | Default, clear, update |

### Iteration 18: Server Core

**Date**: 2026

**Files Created**:
- `core/server/server_types.hpp` - Server type definitions
- `core/server/server.hpp` - Unified server header

**Source Mapping**:
| C File | Lines | C++ File | Status |
|--------|-------|----------|--------|
| server.h | ~1108 | server_types.hpp | Types extracted |
| sv_main.c | ~4144 | server_types.hpp | Types extracted |
| sv_init.c | ~653 | server_types.hpp | Types extracted |
| sv_user.c | ~4860 | server_types.hpp | Types extracted |
| sv_phys.c | ~1136 | server_types.hpp | Types extracted |

**Types Converted**:

*Server State (from server.h):*
- `ServerState` enum (Dead, Loading, Active)
- serverStateName(), isServerRunning(), canPrecache()

*Client State (from server.h):*
- `ClientState` enum (Free, Zombie, Preconnected, Connected, Spawned)
- clientStateName(), isClientActive(), isClientConnected(), canReuse()

*Move Types (from server.h):*
- `MoveType` enum (None through Lock, 11 values)
- moveTypeName(), affectedByGravity(), isProjectileType()

*Solid Types (from server.h):*
- `SolidType` enum (Not, Trigger, BBox, SlideBox, BSP)
- solidTypeName(), isBlocking(), isTouchable()

*Entity Flags (from server.h):*
- `EntityFlags` namespace with Fly, Swim, Client, Monster, OnGround, etc.
- hasFlag(), isPlayer(), isMonster(), canFly()

*Spawn Flags (from server.h):*
- `SpawnFlags` namespace with NotEasy, NotMedium, NotHard, NotDeathmatch
- shouldSpawn() helper

*Damage Types (from server.h):*
- `DamageType` enum (No, Yes, Aim)
- damageTypeName(), canBeDamaged()

*Multicast Types (from server.h):*
- `MulticastType` enum (All, PHS, PVS, AllReliable, PHSReliable, etc.)
- multicastTypeName(), isReliable(), usesVisibility()

*Filter Types (from sv_main.c):*
- `IpFilterType` enum (Ban, Safe)
- `PenaltyType` enum (Mute, Cuff)
- `IpFilter` struct with matches(), isExpired()
- `PenaltyFilter` struct with matchesIp()

*Redirect Types (from server.h):*
- `RedirectType` enum (None, Client, Packet, Mod)

*Demo Destination Types (from server.h):*
- `DemoDestType` enum (None, File, BufferedFile, Stream)
- isFileBased()

*Voice Chat Types (from server.h):*
- `VoiceTarget` enum (Team, All, NonMuted, PlayerSlot0)

*Spec Print Flags (from server.h):*
- `SpecPrint` namespace with CenterPrint, Sprint, StuffCmd

*Server Constants (from server.h):*
- `server_constants` namespace with MAX_CLIENTS, MAX_MODELS, MAX_EDICTS, etc.

*Structures:*
- `Challenge` struct with isValid(), isExpired(), clear()
- `AntilagPosition` struct with isValid()
- `ClientFrame` struct with wasSent(), clear()
- `DemoClientFrame` struct with full demo state
- `ServerStats` struct with latch(), reset(), activePercent()
- `DateTime` struct with format()
- `EntityBaseline` struct with isPlayer(), clear()
- `ServerTime` struct with deltaTime(), pauseDuration(), advance()

**Test Coverage**: 68 new tests (1151 total)

| Test Suite | # Tests | Description |
|------------|---------|-------------|
| ServerStateTest | 4 | Values, names, isRunning, canPrecache |
| ClientStateTest | 5 | Values, names, isActive, isConnected, canReuse |
| MoveTypeTest | 4 | Values, names, gravity, projectile |
| SolidTypeTest | 4 | Values, names, blocking, touchable |
| EntityFlagsTest | 4 | Values, hasFlag, isPlayer, canFly |
| SpawnFlagsTest | 2 | Values, shouldSpawn |
| DamageTypeTest | 2 | Values, canBeDamaged |
| MulticastTypeTest | 4 | Values, names, reliable, visibility |
| IpFilterTypeTest | 1 | Names |
| PenaltyTypeTest | 1 | Names |
| RedirectTypeTest | 2 | Values, names |
| DemoDestTypeTest | 2 | Values, isFileBased |
| VoiceTargetTest | 2 | Values, names |
| SpecPrintTest | 1 | Values |
| ServerConstantsTest | 2 | Values, flags |
| IpFilterTest | 3 | Default, matches, expired |
| PenaltyFilterTest | 2 | Default, matchesIp |
| ChallengeTest | 3 | Default, expired, clear |
| AntilagPositionTest | 2 | Default, valid |
| ClientFrameTest | 2 | Default, clear |
| DemoClientFrameTest | 2 | Default, clear |
| ServerStatsTest | 5 | Default, totalTime, percent, latch, reset |
| DateTimeTest | 2 | Default, format |
| EntityBaselineTest | 3 | Default, isPlayer, clear |
| ServerTimeTest | 4 | Default, delta, advance, pause |

### Iteration 19: Server Physics

**Date**: 2025

**Files Created**:
- `core/physics/physics_types.hpp` - Physics and collision type definitions (~550 lines)
- `core/physics/physics.hpp` - Unified physics header
- `tests/physics/physics_test.cpp` - Physics tests (~570 lines, 57 tests)

**Source Mapping**:
| C File | Lines | C++ File | Status |
|--------|-------|----------|--------|
| sv_phys.c | ~1136 | physics_types.hpp | Types extracted |
| sv_world.c | ~778 | physics_types.hpp | Types extracted |
| sv_world.h | ~86 | physics_types.hpp | Types extracted |
| sv_move.c | ~417 | physics_types.hpp | Types extracted |

**Types Converted**:

*Trace Types (from sv_world.h):*
- `TraceType` enum (Normal=0, NoMonsters=1, Missile=2, Lagged=64)
- traceTypeName(), blocksMonsters(), isMissileTrace()

*Area Types (from sv_world.h):*
- `AreaType` enum (Solid=0, Triggers=1)
- areaTypeName()

*Physics Constants (from sv_phys.c):*
- `physics_constants` namespace with STOP_EPSILON, MAX_CLIP_PLANES, STEPSIZE
- Default movement values: GRAVITY=800, MAXSPEED=320, FRICTION=4, etc.

*Blocked Flags (from sv_phys.c):*
- `BlockedFlags` namespace with None=0, Floor=1, Step=2, DeadStop=4
- hitFloor(), hitStep(), isStuck()

*Content Types (from bspfile.h):*
- `ContentType` enum (Empty=-1, Solid=-2, Water=-3, Slime=-4, Lava=-5, Sky=-6)
- Current types: Current0=-9 through CurrentUp=-14
- contentName(), isLiquid(), causeDamage(), isSolid()

*Structures:*
- `TracePlane` struct with distance(), classifyPoint()
- `TraceResult` struct with didHit(), hitWorld(), hitEntity()
- `BoundingBox` struct with isEmpty(), center(), size(), contains(), intersects(), expand()
- `MoveClip` struct with isMissile(), isLagged()
- `AreaNode` struct with isLeaf(), axisName()
- `MoveVars` struct with all movement parameters, setDefaults()
- `PushResult` struct with pushedAny(), hadBlocking()
- `StepMoveResult` struct with movedUp(), hitStep()
- `FlyMoveResult` struct with wasTrapped(), hitFloor(), hitEntity()

*Functions:*
- `clipVelocity()` - Clip velocity against surface normal
- `applyGravity()` - Apply gravity to velocity
- `isFloorNormal()` / `isWallNormal()` - Surface classification
- `clampVelocity()` - Clamp velocity magnitude
- `fixNaN()` - Fix NaN values in vectors

**Test Coverage**: 57 new tests (1208 total)

| Test Suite | # Tests | Description |
|------------|---------|-------------|
| TraceTypeTest | 4 | Values, names, blocksMonsters, missile |
| AreaTypeTest | 2 | Values, names |
| PhysicsConstantsTest | 3 | StopEpsilon, maxPlanes, defaults |
| PhysicsBlockedFlagsTest | 3 | HitFloor, hitStep, isStuck |
| TracePlaneTest | 3 | Default, distance, classify |
| PhysicsTraceResultTest | 4 | Default, didHit, hitWorld, hitEntity |
| BoundingBoxTest | 6 | Default, center, size, contains, intersects, expand |
| MoveClipTest | 3 | Default, missile, lagged |
| AreaNodeTest | 2 | Default, isLeaf |
| PhysicsClipVelocityTest | 3 | HitFloor, hitWall, overbounce |
| ApplyGravityTest | 2 | Standard, scaled |
| FloorCheckTest | 2 | IsFloorNormal, isWallNormal |
| PhysicsMoveVarsTest | 2 | Defaults, setDefaults |
| ClampVelocityTest | 2 | BelowMax, aboveMax |
| FixNaNTest | 3 | NoNaN, hasNaN, infinity |
| ContentTypeTest | 5 | Values, names, isLiquid, damage, solid |
| PushResultTest | 2 | Default, pushedBlocking |
| StepMoveResultTest | 2 | Default, movedHitStep |
| FlyMoveResultTest | 3 | Default, hitFloor, trapped |

### Iteration 20: Server Networking

**Date**: 2025

**Files Created**:
- `core/net/server_net_types.hpp` - Server networking type definitions (~550 lines)
- `core/net/server_net.hpp` - Unified server networking header
- `tests/net/server_net_test.cpp` - Server networking tests (~470 lines, 47 tests)

**Source Mapping**:
| C File | Lines | C++ File | Status |
|--------|-------|----------|--------|
| sv_send.c | ~1458 | server_net_types.hpp | Types extracted |
| sv_nchan.c | ~196 | server_net_types.hpp | Types extracted |
| sv_ents.c | ~1127 | server_net_types.hpp | Types extracted |
| protocol.h | ~612 | server_net_types.hpp | Types extracted |

**Types Converted**:

*Print Levels (from protocol.h):*
- `PrintLevel` enum (Low=0, Medium=1, High=2, Chat=3)
- printLevelName(), isChatMessage()

*Broadcast Flags (from server.h):*
- `BroadcastFlags` namespace with IgnoreInDemo, IgnoreClients, QtvOnly, IgnoreConsole
- shouldPutInDemo(), shouldSendToClients(), shouldPutInConsole()

*Sound Channels (from sv_send.c):*
- `SoundChannel` enum (Auto, Weapon, Voice, Item, Body)
- soundChannelName()

*Sound Flags (from protocol.h):*
- `SoundFlags` namespace with Volume, Attenuation, defaults

*Protocol Constants (from protocol.h):*
- `protocol_constants` namespace with VERSION, ports, limits
- MAX_CLIENTS=32, MAX_PACKET_ENTITIES=64, UPDATE_BACKUP=64

*Out-of-band Messages (from protocol.h):*
- `oob_message` namespace with S2C_CHALLENGE, S2C_CONNECTION, A2A_PING, etc.

*Server Messages (from protocol.h):*
- `ServerMessage` enum (54 message types)
- serverMessageName()

*Client Messages (from protocol.h):*
- `ClientMessage` enum (Bad, Nop, Move, StringCmd, Delta, TMove, Upload)
- clientMessageName()

*Player Flags (from protocol.h):*
- `PlayerFlags` namespace with MSec, Command, Velocity, Model, etc.

*Entity Update Flags (from protocol.h):*
- `EntityUpdateFlags` namespace with Origin1-3, Angle1-3, Model, etc.

*Client Move Flags (from protocol.h):*
- `ClientMoveFlags` namespace with Angle, Forward, Side, Up, Buttons

*MVD Demo Flags (from protocol.h):*
- `MvdDemoFlags` namespace with Origin, Angles, Effects, etc.

*Structures:*
- `EntityState` struct with isValid(), isWorld(), clear()
- `UserCmd` struct with hasMovement(), hasButtons(), hasImpulse()
- `BackBufferState` struct with hasBackBuffers(), isFull(), totalSize()
- `EntityDelta` struct with hasOriginChange(), hasAngleChange(), isRemove()
- `SoundEvent` struct with hasCustomVolume(), encodeChannel()
- `NailUpdate` struct with encode() for 48-bit compressed projectiles

**Test Coverage**: 47 new tests (1255 total)

| Test Suite | # Tests | Description |
|------------|---------|-------------|
| PrintLevelTest | 3 | Values, names, isChat |
| BroadcastFlagsTest | 4 | Values, shouldPutInDemo, toClients, toConsole |
| SoundChannelTest | 2 | Values, names |
| SoundFlagsTest | 1 | Values |
| ServerProtocolConstantsTest | 3 | Version, ports, limits |
| OobMessageTest | 1 | Values |
| ServerMessageTest | 2 | Values, names |
| ClientMessageTest | 2 | Values, names |
| PlayerFlagsTest | 1 | Values |
| EntityUpdateFlagsTest | 2 | Values, extended |
| ServerEntityStateTest | 4 | Default, isValid, isWorld, clear |
| ServerUserCmdTest | 4 | Default, hasMovement, buttons, impulse |
| BackBufferStateTest | 5 | Default, has, full, totalSize, clear |
| EntityDeltaTest | 5 | Default, origin, angle, model, remove |
| SoundEventTest | 4 | Default, customVolume, attenuation, encode |
| NailUpdateTest | 3 | Default, encode, encodeNonZero |

---

### Iteration 21: Model Types

**Date**: 2025

**Files Created**:
- `core/render/model_types.hpp` - Model and BSP type definitions (~560 lines)
- `core/render/model.hpp` - Unified model header
- `tests/render/model_test.cpp` - Model type tests (~450 lines, 52 tests)

**Source Mapping**:
| C File | Lines | C++ File | Status |
|--------|-------|----------|--------|
| gl_model.h | ~651 | model_types.hpp | Types extracted |
| modelgen.h | ~141 | model_types.hpp | Types extracted |
| spritegn.h | ~111 | model_types.hpp | Types extracted |
| bspfile.h | ~300 | model_types.hpp | Types extracted |

**Types Converted**:

*Model Types (from gl_model.h):*
- `ModelType` enum (Brush=0, Sprite=1, Alias=2, Alias3=3)
- modelTypeName(), isBrushModel(), isAliasModel()

*Model Hints (from gl_model.h):*
- `ModelHint` enum (42 values: Normal, Player, Eyes, Flame, Rocket, Quad, etc.)
- isProjectile(), isPowerup()

*Sync Types (from modelgen.h):*
- `SyncType` enum (Sync=0, Random=1)

*Sprite Types (from spritegn.h):*
- `SpriteType` enum (VpParallelUpright, FacingUpright, VpParallel, Oriented, VpParallelOriented)
- `SpriteFrameType` enum (Single=0, Group=1)
- spriteTypeName(), isUpright()

*Alias Types (from modelgen.h):*
- `AliasFrameType` enum (Single=0, Group=1)
- `AliasSkinType` enum (Single=0, Group=1)

*Entity Effects (from gl_model.h):*
- `EntityEffects` namespace with Rocket=1, Grenade=2, Gib=4, Rotate=8, etc.
- hasTrail(), shouldRotate()

*Surface Flags (from gl_model.h):*
- `SurfaceFlags` namespace with PlaneBack=2, DrawSky=4, DrawTurb=16, etc.
- isSky(), isWater(), isTransparent(), needsSpecialRendering()

*BSP Constants (from bspfile.h):*
- `bsp_constants` namespace with Q1_VERSION=29, HL_VERSION=30, limits
- MAX_HULLS=4, MAX_MODELS=4096, MAX_NODES=32767, MIP_LEVELS=4

*BSP Lumps (from bspfile.h):*
- `BspLump` enum (15 lump types: Entities, Planes, Textures, etc.)
- bspLumpName()

*Plane Types (from bspfile.h):*
- `PlaneType` enum (X=0, Y=1, Z=2, AnyX=3, AnyY=4, AnyZ=5)
- isAxialPlane(), planeAxis()

*Alias Constants (from modelgen.h):*
- `alias_constants` namespace with VERSION=6, MAX_VERTS=2048, MAX_FRAMES=256
- HEADER="IDPO" (0x4F504449)

*Sprite Constants (from spritegn.h):*
- `sprite_constants` namespace with VERSION=1, MAX_FRAMES=512
- HEADER="IDSP" (0x50534449)

*Structures:*
- `ModelBounds` struct with center(), size(), contains(), expand(), calculateRadius()
- `TextureVecs` struct with calcS(), calcT()
- `BspPlane` struct with distanceToPoint(), isInFront(), isAxial()
- `WorldspawnInfo` struct with hasFog(), hasSkybox(), clear()
- `SpriteFrame` struct with isValid(), aspectRatio()
- `AliasFrameDesc` struct with isGroup()
- `Trivertex` struct with decompress()
- `CustomModelId` enum (Explosion, Bolt, Beam, Lightning, BubbleModel, Count)

**Test Coverage**: 52 new tests (1307 total)

| Test Suite | # Tests | Description |
|------------|---------|-------------|
| ModelTypeTest | 4 | Values, names, isBrush, isAlias |
| ModelHintTest | 3 | Values, isProjectile, isPowerup |
| SpriteTypeTest | 3 | Values, names, isUpright |
| EntityEffectsTest | 3 | Values, hasTrail, shouldRotate |
| SurfaceFlagsTest | 4 | Values, isSky, isWater, needsSpecial |
| BspConstantsTest | 2 | Versions, limits |
| BspLumpTest | 2 | Values, names |
| PlaneTypeTest | 2 | IsAxial, planeAxis |
| AliasConstantsTest | 2 | Values, header |
| SpriteConstantsTest | 2 | Values, header |
| ModelBoundsTest | 6 | Default, center, size, contains, expand, calculateRadius |
| TextureVecsTest | 2 | Default, calcCoords |
| BspPlaneTest | 4 | Default, distanceToPoint, isInFront, isAxial |
| WorldspawnInfoTest | 4 | Default, hasFog, hasSkybox, clear |
| SpriteFrameTest | 3 | Default, isValid, aspectRatio |
| AliasFrameDescTest | 2 | Default, isGroup |
| TrivertexTest | 2 | Default, decompress |
| CustomModelIdTest | 2 | Values, names |

---

### Iteration 22: Texture Types

**Date**: 2025

**Files Created**:
- `core/render/texture_types.hpp` - Texture type definitions (~530 lines)
- `tests/render/texture_test.cpp` - Texture type tests (~420 lines, 50 tests)

**Source Mapping**:
| C File | Lines | C++ File | Status |
|--------|-------|----------|--------|
| r_texture.h | ~147 | texture_types.hpp | Types extracted |
| r_texture_internal.h | ~54 | texture_types.hpp | Types extracted |
| r_texture_cvars.c | ~203 | texture_types.hpp | Types extracted |
| wad.h | ~58 | texture_types.hpp | Types extracted |
| quakedef.h | texture enums | texture_types.hpp | Types extracted |

**Types Converted**:

*Texture Types (from quakedef.h):*
- `TextureType` enum (Tex2D, Tex2DArray, CubeMap, Tex2DMultisampled)
- `MinFilter` enum (6 filter modes)
- `MagFilter` enum (Nearest, Linear)

*Texture Flags (from r_texture.h):*
- `TextureFlags` namespace (15 flags: Complain, Mipmap, Alpha, Luma, Fullbright, etc.)
- hasMipmaps(), hasAlpha(), isFloat(), shouldScale(), isLuma()

*Texture Limits:*
- `texture_limits` namespace (MAX_TEXTURES=8192, MIP_LEVELS=4, etc.)

*WAD Types (from wad.h):*
- `WadCompression` enum (None, LZSS)
- `WadLumpType` enum (None, Label, Palette, QTex, QPic, Sound, Miptex)

*Turb Types (from gl_model.h):*
- `TurbType` enum (None, Water, Slime, Lava, Teleport, Sky)
- turbTypeName(), isLiquid()

*Structures:*
- `TextureRef` struct with isValid(), isNull(), invalidate()
- `FilterMode` struct for named filter modes
- `TextureInfo` struct with aspectRatio(), totalTexels(), estimatedMemory()
- `BrushTexture` struct with getName(), isAnimated(), isTurbulent(), isSky()
- `QPic` struct for WAD picture data
- `Charset` struct with getGlyphUVs()
- `TextureArrayRef` struct for array texture references

*Helper Functions:*
- isPowerOfTwo(), nextPowerOfTwo()
- calculateMipLevels(), clampTextureDimension()
- findFilterMode(), minFilterName()

**Test Coverage**: 50 new tests (1357 total)

| Test Suite | # Tests | Description |
|------------|---------|-------------|
| TextureTypeTest | 1 | Values |
| MinFilterTest | 2 | Values, names |
| MagFilterTest | 1 | Values |
| TextureFlagsTest | 6 | Values, hasMipmaps, hasAlpha, isFloat, shouldScale, isLuma |
| TextureLimitsTest | 1 | Values |
| WadLumpTypeTest | 2 | Values, names |
| TextureRefTest | 5 | Default, isValid, invalidate, equality, null |
| FilterModeTest | 4 | FindByName, trilinear, notFound, usesMipmaps |
| TurbTypeTest | 3 | Values, names, isLiquid |
| TextureInfoTest | 5 | Default, isValid, aspectRatio, totalTexels, flags |
| BrushTextureTest | 6 | Default, getName, isAnimated, isTurbulent, isSky, hasFullbright |
| QPicTest | 3 | Default, isValid, dataSize |
| CharsetTest | 2 | Default, glyphUVs |
| TextureArrayRefTest | 2 | Default, isValid |
| IsPowerOfTwoTest | 2 | Powers, nonPowers |
| NextPowerOfTwoTest | 1 | Values |
| CalculateMipLevelsTest | 1 | Values |
| ClampTextureDimensionTest | 1 | Values |

---

### Iteration 23: Render State Types

**Date**: 2025

**Files Created**:
- `core/render/render_state_types.hpp` - Render state type definitions (~580 lines)
- `tests/render/render_state_test.cpp` - Render state tests (~370 lines, 44 tests)

**Source Mapping**:
| C File | Lines | C++ File | Status |
|--------|-------|----------|--------|
| r_state.h | ~350 | render_state_types.hpp | Types extracted |
| r_buffers.h | ~92 | render_state_types.hpp | Types extracted |
| r_vao.h | ~58 | render_state_types.hpp | Types extracted |
| gl_local.h (partial) | ~200 | render_state_types.hpp | Types extracted |

**Types Converted**:

*Depth Function (from r_state.h):*
- `DepthFunc` enum (Less, Equal, LessOrEqual)
- depthFuncName()

*Cull Face (from r_state.h):*
- `CullFace` enum (Front, Back)
- cullFaceName()

*Blend Function (from r_state.h):*
- `BlendFunc` enum (11 blend modes)
- blendFuncName(), isAdditiveBlend()

*Polygon and Alpha (from r_state.h):*
- `PolygonMode` enum (Fill, Line)
- `PolygonOffset` enum (Disabled, Standard, Outlines)
- `AlphaTestFunc` enum (Always, Greater)

*Texture Unit (from r_state.h):*
- `TexUnitMode` enum (Blend, Replace, Modulate, Decal, Add)
- texUnitModeName()

*Fog (from r_state.h):*
- `FogMode` enum (Disabled, Enabled)

*Buffer Types (from r_buffers.h):*
- `BufferType` enum (Vertex, Index, Storage, Uniform, Indirect)
- `BufferUsage` enum (OncePerFrame, ReusePerFrame, ReuseManyFrames, ConstantData)
- `BufferId` enum (26 named buffer IDs)
- bufferTypeName(), bufferUsageName(), shouldDoubleBuffer()

*VAO Types (from r_vao.h):*
- `VaoId` enum (14 VAO types)
- vaoName()

*Render States (from r_state.h):*
- `RenderStateId` enum (100+ predefined render states)
- States for: sky, world, aliasmodel, weaponmodel, HUD, particles, postprocess

*Structures:*
- `Viewport` struct with isValid(), aspectRatio(), contains()
- `DepthState` struct with isDefault()
- `LineState` struct with isDefault()
- `FogState` struct with isEnabled(), enable(), disable()
- `CullState` struct with enable(), disable()
- `ColorMask` struct with allEnabled(), allDisabled(), presets
- `ClearColor` struct with isBlack(), presets
- `BufferRef` struct with isValid(), isEmpty()

*Constants:*
- `render_limits` namespace (MAX_TEXTURE_UNITS, MAX_ATTRIBUTES, etc.)
- `DLightType` enum with dlightColor()

**Test Coverage**: 44 new tests (1401 total)

| Test Suite | # Tests | Description |
|------------|---------|-------------|
| DepthFuncTest | 2 | Values, names |
| CullFaceTest | 2 | Values, names |
| BlendFuncTest | 3 | Values, names, isAdditive |
| PolygonModeTest | 1 | Values |
| AlphaTestFuncTest | 1 | Values |
| TexUnitModeTest | 2 | Values, names |
| BufferTypeTest | 2 | Values, names |
| BufferUsageTest | 3 | Values, names, shouldDoubleBuffer |
| BufferIdTest | 1 | Values |
| VaoIdTest | 2 | Values, names |
| RenderStateIdTest | 2 | Values, count |
| ViewportTest | 5 | Default, isValid, aspectRatio, contains, equality |
| DepthStateTest | 2 | Default, isDefault |
| LineStateTest | 2 | Default, isDefault |
| FogStateTest | 3 | Default, enable, disable |
| CullStateTest | 2 | Default, enableDisable |
| ColorMaskTest | 2 | Default, presets |
| ClearColorTest | 2 | Default, presets |
| BufferRefTest | 2 | Default, isValid |
| RenderLimitsTest | 1 | Values |
| DLightTypeTest | 2 | Values, colors |

---

## CUDA Acceleration Opportunities

The following compute-intensive areas have been identified as candidates for GPU acceleration.
Each can be toggled between CPU and CUDA implementations at runtime via `gpu_*` cvars.

### High Priority (Excellent GPU candidates)

| Area | File(s) | Functions | Data Parallelism | Notes |
|------|---------|-----------|------------------|-------|
| **QMB Particles** | `particles_qmb.c` | `QMB_UpdateParticles()`, `QMB_ProcessParticle()` | 32,768 particles | Perfect SIMD workload, independent particles |
| **Dynamic Lightmaps** | `r_lightmaps.c` | `R_BuildLightMap()`, `R_AddDynamicLights()` | 16K texels × 50 surfaces | Per-texel math, classic GPU workload |

### Medium Priority (Good GPU candidates)

| Area | File(s) | Functions | Data Parallelism | Notes |
|------|---------|-----------|------------------|-------|
| **Collision Tracing** | `pmove.c`, `cmodel.c` | `PM_PlayerTrace()`, `CM_HullTrace()` | 10K-30K traces/frame | Batch ray traces, some branch divergence |
| **PVS Calculation** | `cmodel.c` | `CM_FatPVS()`, `CM_LeafPVS()` | 1K-8K leaves | Bit operations, parallel leaf checks |
| **Classic Particles** | `r_part.c` | `Classic_ParticleTrail()` | 8,192 particles | Simpler than QMB but still parallel |

### Lower Priority (Limited GPU benefit)

| Area | File(s) | Functions | Notes |
|------|---------|-----------|-------|
| Audio Mixing | `snd_mix.c` | `S_PaintChannels()` | Latency-sensitive, CPU SIMD better |
| Delta Compression | `sv_send.c` | `SV_WriteDelta()` | Sequential output limits gains |

### Proposed CUDA Infrastructure

```cpp
namespace ezquake::gpu {
    // Runtime toggle for GPU acceleration
    enum class ComputeBackend { CPU, CUDA };
    
    // Per-subsystem GPU toggle cvars
    // gpu_particles 0/1
    // gpu_lightmaps 0/1
    // gpu_collision 0/1
    
    // Compute kernel abstraction
    template<typename KernelFunc>
    class ComputeKernel {
    public:
        void execute(ComputeBackend backend);
        bool isCudaAvailable() const;
    };
}
```

## C Code Compatibility Testing

To validate the C++ implementations match the original C behavior **before** full integration,
we create parallel test suites that call the original C code using the same test fixtures.

### Strategy

1. **Shared Test Data**: Define test inputs/outputs in a common format
2. **C++ Tests**: Test the new C++ implementation (existing)
3. **C Reference Tests**: Call original C functions with identical inputs
4. **Comparison Tests**: Verify C++ output matches C output exactly

### Implementation Approach

```cpp
// tests/compat/c_reference_tests.cpp
// Links against original ezQuake C code

extern "C" {
    #include "mathlib.h"  // Original C headers
}

// Use same test fixtures as C++ tests
class VectorCompatTest : public ::testing::Test {
protected:
    // Shared test vectors
    std::vector<std::array<float, 3>> testVectors = {
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {3.0f, 4.0f, 0.0f},
        {1.0f, 2.0f, 3.0f}
    };
};

TEST_F(VectorCompatTest, NormalizeBehaviorMatches) {
    for (auto& v : testVectors) {
        // C version
        vec3_t c_vec = {v[0], v[1], v[2]};
        float c_length = VectorNormalize(c_vec);
        
        // C++ version
        ezquake::math::Vec3 cpp_vec(v[0], v[1], v[2]);
        float cpp_length = cpp_vec.normalize();
        
        // Must match exactly
        EXPECT_FLOAT_EQ(c_length, cpp_length);
        EXPECT_FLOAT_EQ(c_vec[0], cpp_vec.x);
        EXPECT_FLOAT_EQ(c_vec[1], cpp_vec.y);
        EXPECT_FLOAT_EQ(c_vec[2], cpp_vec.z);
    }
}
```

### Build Configuration

```cmake
# CMakeLists.txt addition for C compat tests
if(EZQUAKE_CPP_BUILD_COMPAT_TESTS)
    # Link against original C code (compiled as C)
    add_library(ezquake_c_reference STATIC
        ${EZQUAKE_C_SOURCES}
    )
    set_target_properties(ezquake_c_reference PROPERTIES
        C_STANDARD 11
        C_EXTENSIONS ON
    )
    
    add_executable(ezquake_compat_tests
        tests/compat/math_compat_test.cpp
        tests/compat/cvar_compat_test.cpp
        tests/compat/cmd_compat_test.cpp
        tests/compat/fs_compat_test.cpp
    )
    target_link_libraries(ezquake_compat_tests
        ezquake_core
        ezquake_c_reference
        GTest::gtest_main
    )
endif()
```

### Test Categories

| Module | C Functions | C++ Equivalents | Test Focus |
|--------|-------------|-----------------|------------|
| Math | `VectorNormalize`, `AngleVectors`, `BoxOnPlaneSide` | `Vec3::normalize()`, `Vec3::toDirections()`, `boxOnPlaneSide()` | Numerical accuracy |
| Cvar | `Cvar_Set`, `Cvar_Find`, `Cvar_SetValue` | `Cvar::set()`, `CvarManager::find()` | Value parsing, flags |
| Cmd | `Cmd_TokenizeString`, `Cmd_Argc/Argv` | `CommandArgs::parse()` | Quote handling, edge cases |
| FS | `FS_FLocateFile`, `VFS_READ` | `FileSystem::locate()`, `VirtualFile::read()` | Path resolution |

### Benefits

1. **Early Bug Detection**: Find discrepancies before integration
2. **Edge Case Discovery**: Original C may have undocumented behaviors
3. **Regression Prevention**: Any future C++ changes verified against C
4. **Documentation**: Tests document expected C behavior
5. **Confidence**: Full integration can proceed with validated equivalence

## Type Mapping

| Original C | C++ Replacement |
|------------|-----------------|
| `qbool` | `ezquake::Bool` (bool) |
| `byte` | `ezquake::Byte` (uint8_t) |
| `vec3_t` | `ezquake::math::Vec3` |
| `matrix3x3_t` | `ezquake::math::Matrix3x3` |
| `mplane_t` | `ezquake::math::Plane` |
| Raw pointers | `UniquePtr<T>`, `SharedPtr<T>` |
| `char*` | `std::string`, `std::string_view` |

## Namespace Structure

```
ezquake::
├── math::        # Math types and operations
│   ├── Vec3
│   ├── Matrix3x3
│   ├── Plane
│   └── (free functions)
├── memory::      # Memory management
│   ├── allocate/deallocate
│   ├── Arena
│   ├── ArenaScope
│   └── TempArena
├── text::        # String and text handling
│   ├── string_utils.hpp (free functions)
│   └── utf8:: (UTF-8 utilities)
├── fs::          # Filesystem abstraction
│   ├── VirtualFile     # Abstract file interface
│   ├── OsFile          # OS file implementation
│   ├── MemoryFile      # In-memory file
│   ├── FileSystem      # Search paths, file location
│   ├── FileError       # Error enum
│   ├── FileResult<T>   # Expected<T, FileError>
│   └── FileLocation    # Located file info
├── constants::   # Math constants
├── Cvar          # Console variable
├── CvarRegistry  # Cvar registry (dependency injected)
├── TypedCvar<T>  # Type-safe cvar wrapper
├── Command       # Console command
├── CommandArgs   # Parsed command arguments
├── CommandBuffer # Command buffer with execution
├── CommandRegistry # Command registry (dependency injected)
├── Console       # Console output (dependency injected)
├── ConsoleBuffer # Console scrollback buffer
├── ConsoleLine   # Single console line
├── ConsoleColor  # Console text color
├── Context       # Dependency injection container
├── net::         # Network abstraction
│   ├── NetAddress      # IP address (IPv4/IPv6)
│   ├── NetError        # Network error codes
│   ├── NetResult<T>    # Expected<T, NetError>
│   ├── Socket          # RAII UDP/TCP socket
│   ├── NetChannel      # Reliable message channel
│   ├── MessageBuffer   # Read/write message buffer
│   └── Loopback        # Local client-server comm
├── InfoString    # Info string key-value parsing
├── CommandLine   # Command line argument parsing
├── va()          # Compatible formatting function
└── gpu::         # GPU compute abstractions (planned)
    └── ComputeKernel<T>
```

## Build System

```bash
cd src_cpp
mkdir build && cd build
cmake .. -DEZQUAKE_CPP_BUILD_TESTS=ON
make -j$(nproc)
./ezquake_tests
```

**CMake Options**:
- `EZQUAKE_CPP_BUILD_TESTS` - Build unit tests (ON by default)
- `EZQUAKE_CPP_BUILD_BENCHMARKS` - Build benchmarks (OFF by default)
- `EZQUAKE_CPP_ASAN` - Enable AddressSanitizer
- `EZQUAKE_CPP_UBSAN` - Enable UndefinedBehaviorSanitizer
- `EZQUAKE_CPP_CUDA` - Enable CUDA GPU acceleration (OFF by default)

## Next Steps (Planned)

### Roadmap Overview

| Phase | Iterations | Focus | Status |
|-------|------------|-------|--------|
| **Core Types & Math** | 1-8 | Vec3, Cvar, Cmd, Console, FS, Net | ✅ Complete |
| **Client Core** | 9-17 | State, Input, Prediction, Protocol, Demo, View, Entities | ✅ Complete |
| **Server** | 18-20 | Server core, Physics, Networking | ✅ Complete |
| **Rendering Foundation** | 21-25 | Model loading, textures, state | 🔄 In Progress |
| **Rendering Advanced** | 26-30 | Particles, lightmaps, effects | Planned |
| **UI/HUD** | 31-35 | Menus, HUD elements, controls | Planned |
| **QuakeC VM** | 36-38 | Bytecode interpreter, builtins | Planned |
| **GPU Compute** | 39-40 | CUDA acceleration layer | Planned |

### Iteration 14: Client Connection ✅

**Target**: Connection state machine and handshake

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `cl_main.c` - connection logic | `connection_types.hpp` | State machine ✅ |
| `cl_main.c` - signon handling | `SignonState` enum | Connection sequence ✅ |
| `cl_slist.c` - server list | `ServerInfo` struct | Server browser data ✅ |

### Iteration 15: Demo System ✅ COMPLETED

**Target**: Demo recording and playback - **COMPLETED**

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `cl_demo.c` - demo core | `DemoRecorder`, `DemoPlayer` | Record/playback ✅ |
| `cl_nqdemo.c` - NQ demos | `NqDemoState` struct | NetQuake format ✅ |
| `demo_controls.c` - demo UI | `DemoTiming`, `SeekStatus` | Seeking, speed ✅ |

### Iteration 16: View and Camera ✅ COMPLETED

**Target**: View rendering and camera control - **COMPLETED**

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `cl_view.c` - view setup | `view_types.hpp` | View bob, roll, color shifts ✅ |
| `cl_cam.c` - camera modes | `camera_types.hpp` | Spectator, chase cam ✅ |
| `vx_camera.c` - camera effects | `camera_types.hpp` | External camera modes ✅ |

### Iteration 17: Temporary Entities ✅

**Target**: Explosions, projectiles, effects

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `cl_tent.c` - temp entities | `temp_entity_types.hpp` | Beams, explosions ✅ |
| `cl_tent.c` - beam entities | `BeamPool` class | Lightning, grapple ✅ |

### Iteration 18: Server Core ✅

**Target**: Server initialization and main loop

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `server.h` - server types | `server_types.hpp` | State, types, constants ✅ |
| `sv_main.c` - server main | `server_types.hpp` | Enums, filters ✅ |
| `sv_init.c` - initialization | `server_types.hpp` | Baseline, time ✅ |
| `sv_user.c` - client handling | `server_types.hpp` | Client state ✅ |

### Iteration 19: Server Physics ✅

**Target**: Server-side physics and collision types

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `sv_phys.c` - physics | `physics_types.hpp` | Trace, movement types ✅ |
| `sv_move.c` - movement | `physics_types.hpp` | Step, blocked flags ✅ |
| `sv_world.c` - world state | `physics_types.hpp` | Collision, areas ✅ |
| `sv_world.h` - world header | `physics_types.hpp` | Constants ✅ |

### Iteration 20: Server Networking ✅

**Target**: Server network protocol types

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `sv_send.c` - sending | `server_net_types.hpp` | Print, sound, multicast ✅ |
| `sv_nchan.c` - netchan | `server_net_types.hpp` | Back buffer state ✅ |
| `sv_ents.c` - entity sync | `server_net_types.hpp` | Entity delta, updates ✅ |
| `protocol.h` - protocol | `server_net_types.hpp` | Messages, flags ✅ |

### Iteration 21: Model Types ✅

**Target**: Model and BSP type definitions

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `gl_model.h` - model types | `model_types.hpp` | ModelType, ModelHint, effects ✅ |
| `modelgen.h` - alias types | `model_types.hpp` | Alias constants, trivertex ✅ |
| `spritegn.h` - sprite types | `model_types.hpp` | Sprite types, frames ✅ |
| `bspfile.h` - BSP types | `model_types.hpp` | BSP lumps, planes, limits ✅ |

### Iteration 22: Texture Types ✅

**Target**: Texture type definitions

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `r_texture.h` - texture flags | `texture_types.hpp` | Flags, limits ✅ |
| `r_texture_internal.h` - gltexture_t | `texture_types.hpp` | TextureInfo struct ✅ |
| `r_texture_cvars.c` - filter modes | `texture_types.hpp` | FilterMode, MinFilter ✅ |
| `wad.h` - WAD types | `texture_types.hpp` | WadLumpType, QPic ✅ |

### Iteration 23: Render State Types ✅

**Target**: Render state type definitions

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `r_state.h` - state enums | `render_state_types.hpp` | DepthFunc, BlendFunc, etc. ✅ |
| `r_buffers.h` - buffer types | `render_state_types.hpp` | BufferType, BufferId ✅ |
| `r_vao.h` - VAO types | `render_state_types.hpp` | VaoId enum ✅ |
| `gl_local.h` - GL constants | `render_state_types.hpp` | DLightType, limits ✅ |

### Iteration 24: Particle Types ✅

**Target**: Particle system type definitions (37 tests)

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `r_particles_qmb.h` - part_type_t | `particle_types.hpp` | ParticleType enum (49 types) ✅ |
| `r_particles_qmb.h` - part_move_t | `particle_types.hpp` | ParticleMove enum (10 modes) ✅ |
| `r_particles_qmb.h` - part_tex_t | `particle_types.hpp` | ParticleTexture enum (13 types) ✅ |
| `r_particles_qmb.h` - part_draw_t | `particle_types.hpp` | ParticleDraw enum (9 modes) ✅ |
| `r_particles_qmb.h` - part_blend_id | `particle_types.hpp` | ParticleBlend enum (5 modes) ✅ |
| `r_particles_qmb.h` - particle_t | `particle_types.hpp` | Particle struct ✅ |
| `r_particles_qmb.h` - particle_type_t | `particle_types.hpp` | ParticleTypeDef struct ✅ |
| `r_particles_qmb.h` - particle_texture_t | `particle_types.hpp` | ParticleTextureInfo struct ✅ |
| `r_part.c` - ramp arrays | `particle_types.hpp` | PARTICLE_RAMP1/2/3 constexpr ✅ |
| `r_part.c` - qparticle_t | `particle_types.hpp` | ClassicParticle struct ✅ |

**Key Types Created**:
- `ParticleType` - 49 QMB particle types (fire, smoke, spark, blood, etc.)
- `ParticleMove` - Movement behaviors (none, linear, grav, bounce, etc.)
- `ParticleTexture` - Texture slots (solid, smoke, bubble, lightning, etc.)
- `ParticleDraw` - Drawing modes (default, billboard, beam, spark, etc.)
- `ParticleBlend` - Blend modes (blend, add, multiply, additive)
- `particle_limits` - Pool sizes and limits namespace
- `PARTICLE_RAMP1/2/3` - Classic particle color ramp arrays
- `ParticleColor` - RGBA color with palette index support
- `Particle` - Full particle instance with position, velocity, appearance
- `ParticleTypeDef` - Particle type definition with all properties
- `ParticleTextureInfo` - Texture atlas information with UV coords
- `ClassicParticle` - Simple classic Quake particle
- `TrailType` - Trail effect types (rocket, grenade, blood, etc.)
- `ExplosionType` - Explosion effect types (normal, blood, detpack, etc.)

### Iteration 25: Lightmap Types ✅

**Target**: Lightmap and dynamic lighting type definitions (47 tests)

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `r_lightmaps_internal.h` - constants | `lightmap_types.hpp` | lightmap_limits namespace ✅ |
| `r_lightmaps_internal.h` - glRect_t | `lightmap_types.hpp` | LightmapRect struct ✅ |
| `r_lightmaps_internal.h` - lightmap_data_t | `lightmap_types.hpp` | LightmapData struct ✅ |
| `r_lighting.h` - rgb9e5tab | `lightmap_types.hpp` | RGB9E5_TABLE, decodeRGB9E5() ✅ |
| `client.h` - dlighttype_t | `lightmap_types.hpp` | DLightColorType enum ✅ |
| `client.h` - dlight_t | `lightmap_types.hpp` | DynamicLight struct ✅ |
| `client.h` - customlight_t | `lightmap_types.hpp` | CustomLight struct ✅ |
| `client.h` - lightstyle_t | `lightmap_types.hpp` | LightStyle struct ✅ |

**Key Types Created**:
- `lightmap_limits` - Constants namespace (sizes, limits)
- `DLightColorType` - Dynamic light color types (12 types)
- `LightColor` - RGB color with conversion utilities
- `DynamicLight` - Point light with decay/radius
- `CustomLight` - Custom light settings for entities
- `LightStyle` - Animated light pattern ('a'-'z' brightness)
- `LightmapRect` - Dirty region tracking rectangle
- `LightmapData` - Full lightmap texture data with allocation
- `RGB9E5_TABLE` - HDR lightmap decoder table
- `SurfaceLightInfo` - Per-surface lighting data
- `VertexLight` / `VertexLightMode` - Vertex lighting
- `DynamicLightMode` - Lighting computation mode
- `LightEntityParams` - Map light entity parameters
- `DynamicLightPool` - Active light pool with bit tracking
- `LightStylePool` - Light style pool with brightness values

### Iteration 26: HUD/UI Types ✅

**Target**: HUD, Menu, and UI type definitions (45 tests)

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `hud.h` - HUD flags | `hud_types.hpp` | HudFlags namespace ✅ |
| `hud.h` - HUD_PLACE_* | `hud_types.hpp` | HudPlacement enum ✅ |
| `hud.h` - HUD_ALIGN_* | `hud_types.hpp` | HudAlign enum ✅ |
| `hud.h` - hud_t drawing | `hud_types.hpp` | HudDrawInfo struct ✅ |
| `hud_common.h` - sort_teams_info_t | `hud_types.hpp` | SortedTeamInfo struct ✅ |
| `hud_common.h` - sort_players_info_t | `hud_types.hpp` | SortedPlayerInfo struct ✅ |
| `menu.h` - m_state_t | `hud_types.hpp` | MenuState enum ✅ |
| `Ctrl.h` - ScrollBar | `hud_types.hpp` | ScrollbarState struct ✅ |

**Key Types Created**:
- `hud_limits` - HUD constants namespace (max elements, letter sizes)
- `HudFlags` - HUD element flags (NoDraw, OnIntermission, etc.)
- `HudPlacement` - Element placement modes (Screen, Sbar, View, etc.)
- `HudAlign` - Alignment modes (Left, Center, Right, Before, After)
- `ScoreboardFlags` - Scoreboard update flags
- `HudRect` - Position/size rectangle with hit testing
- `HudFrame` - Frame/border padding
- `HudColor` - RGBA color with presets and opacity
- `MenuState` - Menu state enum (None, Main, Options, etc.)
- `SliderRange` - Slider value specification
- `ScrollbarState` - Scrollbar position and dimensions
- `SortedTeamInfo` - Team scoreboard data (frags, pings, items)
- `SortedPlayerInfo` - Player scoreboard reference
- `HudNumberStyle` - Number rendering styles
- `HudMinState` - Minimum client state for drawing
- `TextBoxSpec` - Text box dimensions
- `MouseState` / `MouseButton` - UI mouse interaction
- `HudDrawInfo` - Complete HUD element draw state
- `calculateStack()` / `getArmorType()` - Stack calculation helpers

### Iteration 27: Sound System Types ✅

**Target**: Sound and audio type definitions (47 tests)

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `qsound.h` - sfx_t | `sound_types.hpp` | SoundEffect struct ✅ |
| `qsound.h` - snd_format_t | `sound_types.hpp` | SoundFormat struct ✅ |
| `qsound.h` - sfxcache_t | `sound_types.hpp` | SoundCache struct ✅ |
| `qsound.h` - channel_t | `sound_types.hpp` | SoundChannel struct ✅ |
| `qsound.h` - soundhw_t | `sound_types.hpp` | SoundHardware struct ✅ |
| `qsound.h` - wavinfo_t | `sound_types.hpp` | WavInfo struct ✅ |
| `cdaudio.h` - CD state | `sound_types.hpp` | CDAudioState struct ✅ |

**Key Types Created**:
- `sound_limits` - Constants namespace (channels, rates, volumes)
- `SoundFormat` - Audio sample format (rate, width, channels)
- `SoundEffect` - Sound effect reference with name and buffer
- `SoundCache` - Cached sound data with loop point
- `WavInfo` - WAV file header information
- `SoundChannel` - Active sound channel with 3D positioning
- `SoundHardware` - Sound driver/hardware state
- `SoundAttenuation` - Attenuation presets (None, Normal, Static)
- `EntityChannel` - Entity channel slots (Auto, Weapon, Voice)
- `AmbientType` - Ambient sounds (Water, Sky, Slime, Lava)
- `VolumeSettings` - Volume levels for sound types
- `CDAudioState` - CD audio playback state
- `SoundListener` - 3D listener position and orientation
- `ResampleStyle` - Audio resampling methods
- `SoundPlayParams` - Sound playback parameters
- `SoundChannelPool` - Active channel pool with search

### Iteration 28: Archive/Wad Types ✅

**Target**: Archive, WAD, PAK, and VFS type definitions (36 tests)

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `wad.h` - CMP_* | `archive_types.hpp` | WadCompression enum ✅ |
| `wad.h` - TYP_* | `archive_types.hpp` | WadLumpType enum ✅ |
| `wad.h` - qpic_t | `archive_types.hpp` | QPicHeader struct ✅ |
| `vfs.h` - packfile_t | `archive_types.hpp` | PackFile struct ✅ |
| `vfs.h` - flocation_t | `archive_types.hpp` | FileLocation struct ✅ |
| `vfs.h` - searchpath_t | `archive_types.hpp` | SearchPathInfo struct ✅ |

**Key Types Created**:
- `archive_limits` - Constants namespace (MAX_QPATH, magic numbers)
- `WadCompression` - Compression type (None, LZSS)
- `WadLumpType` - WAD lump types (Palette, QPic, MipTex, Sound)
- `WadLump` - WAD lump directory entry
- `WadHeader` - WAD file header with magic validation
- `QPicHeader` - QPic image header (width, height)
- `PackFile` - PAK file entry (name, position, length)
- `PackHeader` - PAK file header with magic validation
- `FileLocation` - VFS file location result
- `SearchPathInfo` - Search path with flags
- `MipTexHeader` - Mipmap texture header with 4 mip levels
- `ArchiveType` - Archive type (Directory, Pak, Zip, Wad, Gzip)
- `FileOpenMode` - File open modes (Read, Write, Append)
- `VfsError` - VFS error codes with descriptions
- `FileHashEntry` - File hash table entry
- `ArchiveStats` - Archive statistics with compression ratio

### Iteration 29: Sprite/Sky Types ✅

**Target**: Sprite, billboard, and sky rendering type definitions (40 tests)

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `spritegn.h` - dsprite_t | `sprite_types.hpp` | SpriteHeader struct ✅ |
| `spritegn.h` - dspriteframe_t | `sprite_types.hpp` | SpriteFrameHeader struct ✅ |
| `spritegn.h` - SPR_* | `sprite_types.hpp` | SpriteOrientation enum ✅ |
| `modelgen.h` - mdl_t | `sprite_types.hpp` | AliasModelHeader struct ✅ |
| `modelgen.h` - trivertx_t | `sprite_types.hpp` | TriVertex struct ✅ |
| `r_sprite3d.h` - sprite3d_batch_id | `sprite_types.hpp` | Sprite3DBatchId enum ✅ |
| `r_sprite3d.h` - r_sprite3d_vert_t | `sprite_types.hpp` | Sprite3DVertex struct ✅ |
| `r_brushmodel_sky.h` - skybox | `sprite_types.hpp` | SkyState struct ✅ |

**Key Types Created**:
- `sprite_limits` - Constants namespace (batch sizes, magic numbers)
- `SyncType` - Animation synchronization (Sync, Random)
- `SpriteOrientation` - Sprite orientation modes (5 types)
- `SpriteFrameType` - Frame type (Single, Group)
- `PrimitiveType` - Rendering primitive types
- `Sprite3DBatchId` - 61 batch identifiers (particles, corona, chat icons)
- `SpriteVertPoolId` - Vertex pool identifiers
- `Sprite3DVertex` - 32-byte aligned vertex structure
- `SpriteHeader` - Sprite file header with validation
- `SpriteFrameHeader` - Frame dimensions and origin
- `AliasModelHeader` - MDL file header
- `TriVertex` - 4-byte compressed vertex
- `AliasTexCoord` - Texture coordinate with seam flag
- `AliasTriangle` - Triangle with front-face flag
- `AliasFrameHeader` - Frame with bounding box and name
- `SkyboxFace` - Skybox face identifiers with suffixes
- `SkyState` - Skybox and scrolling sky state
- `SkyWind` - Animated sky wind parameters
- `SpriteBatchInfo` - Batch descriptor
- `SpriteRenderStats` - Rendering statistics

### Iteration 30: QuakeC VM Types ✅

**Target**: QuakeC Virtual Machine type definitions (43 tests)

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `pr_comp.h` - etype_t | `vm_types.hpp` | EType enum ✅ |
| `pr_comp.h` - dstatement_t | `vm_types.hpp` | Statement struct ✅ |
| `pr_comp.h` - ddef_t | `vm_types.hpp` | Definition struct ✅ |
| `pr_comp.h` - dfunction_t | `vm_types.hpp` | FunctionDef struct ✅ |
| `pr_comp.h` - dprograms_t | `vm_types.hpp` | ProgsHeader struct ✅ |
| `progs.h` - eval_t | `vm_types.hpp` | EvalValue union ✅ |
| `vm_local.h` - opcode_t | `vm_types.hpp` | Q3Opcode enum ✅ |
| `vm_local.h` - vmHeader_t | `vm_types.hpp` | VMHeader struct ✅ |
| `vm_local.h` - instruction_t | `vm_types.hpp` | VMInstruction struct ✅ |

**Key Types Created**:
- `vm_limits` - Constants namespace (versions, stack sizes, offsets)
- `EType` - QuakeC type enumeration (8 types)
- `EvalValue` - Union for QC value storage
- `PR1Opcode` - QuakeC bytecode opcodes (66 opcodes)
- `Q3Opcode` - Q3 VM bytecode opcodes (60 opcodes)
- `Statement` - PR1 bytecode instruction (8 bytes)
- `Definition` - Global/field definition with save flag
- `FunctionDef` - Function definition with parameters
- `ProgsHeader` - Progs file header
- `VMHeader` - Q3 VM file header
- `VMInstruction` - Q3 VM instruction
- `VMSymbol` - Debug symbol
- `VMIndex` - VM instance identifier (Game, CGame, UI)
- `VMState` - VM execution state
- `EdictLink` - Linked list node for area links
- `ServerEdict` - Server-side edict state
- `ExtEntVars` - Extended entity variables (alpha, colormod)
- `BuiltinInfo` - Builtin function metadata
- `FieldOffsets` - Cached field offset lookup
- `VMStats` - Execution statistics

### Iteration 31: Menu System Types ✅

**Target**: Menu system, keys, and settings page type definitions (45 tests)

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `keys.h` - keynum_t | `menu_types.hpp` | KeyCode enum ✅ |
| `keys.h` - keydest_t | `menu_types.hpp` | KeyDest enum ✅ |
| `keys.h` - mouse_state_t | `menu_types.hpp` | MouseState struct ✅ |
| `menu.h` - m_state_t | `menu_types.hpp` | MenuState enum ✅ |
| `settings.h` - setting_type | `menu_types.hpp` | SettingType enum ✅ |
| `settings.h` - setting | `menu_types.hpp` | SettingDef struct ✅ |
| `settings.h` - settings_page | `menu_types.hpp` | SettingsPageState struct ✅ |

**Key Types Created**:
- `menu_limits` - Constants namespace (command line, button counts)
- `KeyCode` - Keyboard and input key codes (~90 keys)
- `KeyDest` - Key input routing destination
- `MenuState` - Current menu page/state (13 states)
- `ChatType` - Chat message type (say, say_team, IRC)
- `MouseState` - Mouse pointer state for menus
- `SettingType` - Setting control types (15 types)
- `SettingsPageMode` - Settings page interaction mode
- `SettingDef` - Setting definition with range/steps
- `SettingsPageState` - Settings page navigation state
- `KeyBinding` - Key binding entry
- `KeyState` - Global key state tracking
- `MenuDrawInfo` - Menu drawing parameters
- `SliderControl` - Slider control state with position
- `FlashingCursor` - Animated cursor for menus
- `PlayerColor` - Player color selection (top/bottom)

### Iteration 32: Config System Types ✅

**Target**: Configuration, teamplay, triggers, and macro types (35 tests)

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `teamplay.h` - msgtype_t | `config_types.hpp` | MessageType namespace ✅ |
| `teamplay.h` - FPD_* flags | `config_types.hpp` | FPDFlags namespace ✅ |
| `teamplay.h` - it_* flags | `config_types.hpp` | ItemFlags namespace ✅ |
| `teamplay.h` - locdata_t | `config_types.hpp` | LocationEntry struct ✅ |
| `tp_triggers.h` - RE_* flags | `config_types.hpp` | TriggerFlags namespace ✅ |
| `tp_triggers.h` - pcre_trigger_t | `config_types.hpp` | TriggerDef struct ✅ |
| `config_manager.h` - cfg_save_unchanged | `config_types.hpp` | ConfigSaveSettings ✅ |

**Key Types Created**:
- `config_limits` - Constants namespace (MAX_LOC_NAME, MAX_MACRO_STRING)
- `MessageType` - Teamplay message types (death, took, point, etc.)
- `FPDFlags` - Fairplay detection flags (observer checks, rule enforcement)
- `TriggerFlags` - Regex trigger flags (print types, matching modes)
- `FairplaySettings` - FPD settings with flag checks
- `TriggerDef` - PCRE trigger with timing and enabled state
- `ItemFlags` - Item visibility flags (RA, YA, GA, Quad, weapons, etc.)
- `ItemVisibility` - Complete item visibility state
- `LocationEntry` - Location file entry with distance calculation
- `SkinForcingConfig` - Skin name forcing settings
- `ColorForcingConfig` - Color forcing for teams
- `ConfigSaveOption` - Save options enum
- `ConfigSaveSettings` - Full configuration save state
- `AliasDef` - Console alias definition
- `BindDef` - Key binding definition
- `MacroState` - Macro expansion state
- `TeamplayState` - Complete teamplay configuration state
- `ConfigStats` - Configuration statistics and counts

### Iteration 33: BSP/Map Types ✅

**Target**: BSP file format parsing and map structure types (37 tests)

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `bspfile.h` - version defines | `bsp_types.hpp` | BSPVersion enum ✅ |
| `bspfile.h` - LUMP_* | `bsp_types.hpp` | LumpType enum ✅ |
| `bspfile.h` - PLANE_* | `bsp_types.hpp` | PlaneType enum ✅ |
| `bspfile.h` - CONTENTS_* | `bsp_types.hpp` | Contents enum ✅ |
| `bspfile.h` - dnode_t | `bsp_types.hpp` | DNode, DNodeBSP2 ✅ |
| `bspfile.h` - dface_t | `bsp_types.hpp` | DFace, DFaceBSP2 ✅ |
| `bspfile.h` - dleaf_t | `bsp_types.hpp` | DLeaf, DLeafBSP2 ✅ |
| `bspfile.h` - miptex_t | `bsp_types.hpp` | MipTex struct ✅ |

**Key Types Created**:
- `bsp` namespace - Version constants (Q1, HL, BSP2, BSP29A)
- `map_limits` namespace - Maximum limits for map elements
- `BSPVersion` - BSP format version enum with detection
- `LumpType` - BSP lump types (15 lumps)
- `PlaneType` - Axial plane classification
- `Contents` - Leaf contents (solid, water, lava, etc.)
- `AmbientType` - Ambient sound channels
- `Lump` - File lump descriptor
- `BSPHeader` - BSP file header with lump array
- `DVertex`, `DPlane` - Disk format vertex and plane
- `DNode`, `DNodeBSP2` - BSP nodes (standard and extended)
- `DClipnode` - Collision clipnode
- `DLeaf`, `DLeafBSP2` - BSP leafs (standard and extended)
- `DEdge`, `DEdgeBSP2` - BSP edges
- `DFace`, `DFaceBSP2` - BSP faces
- `DModel` - BSP submodel/brush entity
- `MipTexHeader`, `MipTex` - Texture definitions with mipmaps
- `Texinfo` - Texture mapping info
- `LightmapInfo` - Extended lightmap info (BSPX)
- `BSPXHeader` - BSPX extension header
- `BSPStats` - BSP file statistics for limit checking

### Iteration 34: Browser/Server Types ✅

**Target**: Server browser and server list types (45 tests)

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `EX_browser.h` - server_data | `browser_types.hpp` | ServerData struct ✅ |
| `EX_browser.h` - source_data | `browser_types.hpp` | SourceData struct ✅ |
| `EX_browser.h` - sb_player | `browser_types.hpp` | BrowserPlayerInfo struct ✅ |
| `EX_browser.h` - sb_source_type_e | `browser_types.hpp` | SourceType enum ✅ |
| `cl_slist.h` - server_entry_t | `browser_types.hpp` | ServerEntry struct ✅ |

**Key Types Created**:
- `browser_limits` namespace - Browser constants (MAX_SERVERS, MAX_SOURCES, etc.)
- `ServerOccupancy` - Server occupancy state with helpers
- `SourceType` - Server source type (Master, File, Url, Dummy)
- `BrowserTrigger` namespace - Browser event flags
- `BrowserPlayerInfo` - Player info for server browser
- `ServerColumns` - Pre-formatted display columns
- `ServerKeyValue` - Key-value pair for server info
- `NetAddress` - Simplified IP:port address
- `ServerData` - Complete server information
- `SourceTime` - Timestamp for source updates
- `SourceData` - Server source with address list
- `PlayerHost`, `BrowserWindow` - Browser UI state
- `ServerEntry` - Server list entry
- `PingResult` - Ping measurement result
- `BrowserState` - Overall browser state with triggers
- `ServerSortMode`, `PlayerSortMode` - Sort mode enums
- `FilterSettings` - Server filter configuration
- `BrowserStats` - Browser statistics

### Iteration 35: Effects Types ✅

**Target**: Particle systems, dynamic lights, beams, explosions (53 tests)

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `particles_classic.h` - ptype_t | `effects_types.hpp` | ClassicParticleType enum ✅ |
| `r_particles_qmb.h` - part_type_t | `effects_types.hpp` | QMBParticleType enum ✅ |
| `r_particles_qmb.h` - part_move_t | `effects_types.hpp` | ParticleMovement enum ✅ |
| `r_particles_qmb.h` - part_tex_t | `effects_types.hpp` | ParticleTexture enum ✅ |
| `r_particles_qmb.h` - part_draw_t | `effects_types.hpp` | ParticleDrawType enum ✅ |
| `r_particles_qmb.h` - particle_t | `effects_types.hpp` | QMBParticle struct ✅ |
| `quakedef.h` - trail_type_t | `effects_types.hpp` | TrailType enum ✅ |
| `client.h` - dlighttype_t | `effects_types.hpp` | DynamicLightType enum ✅ |
| `client.h` - dlight_t | `effects_types.hpp` | DynamicLight struct ✅ |
| `cl_tent.c` - beam_t | `effects_types.hpp` | Beam struct ✅ |
| `cl_tent.c` - explosion_t | `effects_types.hpp` | Explosion struct ✅ |

**Key Types Created**:
- `effect_limits` namespace - Particle and effect constants
- `ClassicParticleType` - Original Quake particle types (Static, Gravity, Fire, Explode...)
- `QMBParticleType` - Extended QMB particle types (47 types including VULT particles)
- `ParticleMovement` - Physics behavior (Static, Normal, Bounce, Die, Float, Rain...)
- `ParticleTexture` - Texture identifiers (Smoke, Bubble, Blood, Lightning...)
- `ParticleDrawType` - Rendering method (Spark, Billboard, Beam, Torch...)
- `ParticleBlendMode` - Blend modes (additive, alpha, etc.)
- `TrailType` - Entity trail types (Rocket, Blood, Rail, Lava...)
- `DynamicLightType` - Light colors (Default, MuzzleFlash, Explosion, Red, Blue...)
- `color_ramp` namespace - Fire/explosion color ramps (palette indices)
- `ParticleColor` - RGBA color with fromFloat/toFloat conversion
- `ClassicParticle` - Original Quake particle struct
- `QMBParticle` - Extended particle with rotation, growth, caching
- `ParticleTypeConfig` - Particle type rendering properties
- `ParticleTextureCoords` - Texture atlas coordinates
- `DynamicLight` - Dynamic light source with decay
- `CustomLight` - Entity-attached custom light
- `LightStyle` - Flickering light pattern
- `Beam` - Lightning bolt/beam effect
- `Explosion` - Explosion sprite animation
- `EffectStats` - Particle system statistics
- `ParticleSpawnParams` - Fluent spawn configuration
- `EffectConfig` - Effect system toggle configuration

### Iteration 36: Ruleset Types ✅

**Target**: Competitive ruleset system types (40 tests)

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `rulesets.h` - ruleset_t | `ruleset_types.hpp` | RulesetMode enum ✅ |
| `rulesets.h` - locked_cvar_t | `ruleset_types.hpp` | LockedCvar struct ✅ |
| `rulesets.h` - limited_cvar_max_t | `ruleset_types.hpp` | LimitedCvarMax struct ✅ |
| `rulesets.h` - limited_cvar_min_t | `ruleset_types.hpp` | LimitedCvarMin struct ✅ |

**Key Types Created**:
- `ruleset_limits` namespace - Ruleset constants (FPS limits, flash settings)
- `RulesetMode` - Ruleset modes (Default, Smackdown, Thunderdome, QCon, MTFL, Smackdrive)
- `RestrictionFlag` - Feature restriction flags (32 flags)
- `RestrictionSet` - Bitset for tracking active restrictions
- `LockedCvar` - Cvar locked to specific value
- `LimitedCvarMax`, `LimitedCvarMin` - Cvar value limits
- `RulesetConfig` - Complete ruleset configuration with all restrictions
- `ModelRestriction` - Model-specific restriction flags
- `CvarValidator` - Ruleset-aware cvar change validator
- `CvarChangeResult` - Validation result enum
- `RulesetState` - Mutable runtime ruleset state
- `RulesetStats` - Ruleset statistics for debugging

**Factory Functions**:
- `createDefaultRuleset()` - No restrictions
- `createSmackdownRuleset()` - Smackdown tournament
- `createThunderdomeRuleset()` - Thunderdome tournament
- `createQConRuleset()` - QuakeCon tournament
- `createMTFLRuleset()` - MTFL with full restrictions
- `createSmackdriveRuleset()` - Smackdrive tournament
- `createRuleset(mode)` - Create by mode enum
- `applyCompetitiveBase()` - Apply common competitive restrictions

### Iteration 37: Teamplay Types ✅

**Target**: Team communication and coordination types (42 tests)

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `teamplay.h` - item_vis_t | `teamplay_types.hpp` | ItemVisibility struct ✅ |
| `teamplay.h` - tvars_t | `teamplay_types.hpp` | TeamplayState struct ✅ |
| `teamplay.h` - locdata_t | `teamplay_types.hpp` | LocationData struct ✅ |
| `teamplay.h` - it_* flags | `teamplay_types.hpp` | ItemFlag enum ✅ |

**Key Types Created**:
- `teamplay_limits` namespace - Teamplay constants (MAX_LOC_NAME, NUM_ITEMFLAGS)
- `ItemFlag` - 32 item flags (Quad, Pent, Ring, Suit, Weapons, Ammo, etc.)
- `ItemFlags` - Bitset for tracking held/visible items
- `item_masks` namespace - Item combination masks (POWERUPS, WEAPONS, ARMOR, AMMO, RUNES)
- `FPDFlag` - Fake Player Detection flags (NoTimers, NoForceSkin, NoForceColor, etc.)
- `MessageType` - Chat message categorization (Normal, Team, Spectator, QTV)
- `ItemVisibility` - Item visibility check with calculate(), isInFront(), angleToEntity()
- `LocationData` - .loc file entry with distanceTo()
- `TeamplayState` - tvars_t equivalent with reset(), tookEmpty(), hasPowerups(), itemsChanged()
- `SkinForceType` - 9 skin types (Normal, Team, Enemy, TeamQuad, EnemyQuad, etc.)
- `SkinForceConfig` - Skin forcing configuration
- `ColorForceConfig` - Color forcing configuration
- `TeamplayConfig` - Full teamplay configuration
- `NeedAssessment` - Player need with priority calculation
- `calculateNeed()` - Health/armor/ammo/weapon need assessment

### Iteration 38: MVD Types ✅

**Target**: Multi-View Demo playback and autotrack types (53 tests)

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `mvd_utils_common.h` - mvd_info_t | `mvd_types.hpp` | MvdPlayerStats struct ✅ |
| `mvd_utils_common.h` - mvd_new_info_t | `mvd_types.hpp` | MvdPlayerInfo struct ✅ |
| `mvd_utils_common.h` - mvd_event_t | `mvd_types.hpp` | MvdEvent struct ✅ |
| `mvd_autotrack.c` - pl_values | `mvd_types.hpp` | AutotrackWeights struct ✅ |

**Key Types Created**:
- `mvd_limits` namespace - MVD constants (MAX_CLIENTS, MVD_INFO_TYPES, etc.)
- `MvdInfoType` - Item/weapon info type enum (15 types)
- `GameType` - Game type enum (1on1, 2on2, 3on3, 4on4, Unknown)
- `MvdEventType` - Event types (Spawn, Death, Kill, TeamKill, ItemTook, PowerupEnd)
- `AddClockMode` - Respawn clock display mode
- `WeaponKillStats`, `KillStats` - Kill tracking with powerup combos
- `DeathAliveStats` - Death/alive time tracking
- `RunStats`, `AverageRunStats` - Spawn-to-death run statistics
- `ItemPossessionStats` - Powerup/item possession tracking
- `MvdEvent` - Single MVD event record
- `MvdPlayerStats` - Complete player statistics
- `MvdPlayerInfo` - Full player tracking with events
- `MvdGameInfo` - Match/game information
- `MvdWeaponInfo` - Weapon display info for MVD
- `AutotrackWeights` - Player value weights for autotracking
- `AutotrackConfig` - Autotrack formula and settings
- `MultitrackConfig` - Multi-camera autotrack config
- `MvdState` - Complete MVD playback state

**Helper Functions**:
- `isWeapon()`, `isPowerup()`, `isArmor()` - Type classification
- `gameTypeName()`, `playersPerTeam()` - Game type utilities
- `gameTypeFromPlayerCount()` - Auto-detect game type

### Iteration 39: Video Types ✅

**Target**: Video and display subsystem types (51 tests)

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `vid.h` - viddef_t | `video_types.hpp` | VideoDefinition struct ✅ |
| `vid.h` - vrect_t | `video_types.hpp` | VideoRect struct ✅ |
| `render.h` - refdef_t | `video_types.hpp` | RefreshDefinition struct ✅ |
| `render.h` - refdef2_t | `video_types.hpp` | ExtendedRefreshDefinition struct ✅ |
| `render.h` - fogcalc_t | `video_types.hpp` | FogCalculation enum ✅ |
| `render.h` - RF_* flags | `video_types.hpp` | RenderFlag enum ✅ |

**Key Types Created**:
- `video_limits` namespace - Video constants (VID_CBITS, VID_GRADES, etc.)
- `RenderFlag` - Entity render flags (WeaponModel, NoShadow, AlphaBlend, etc.)
- `VideoRect` - Screen region with contains(), overlaps(), aspect()
- `VideoDefinition` - Global video state (width, height, aspect, console size)
- `FogCalculation` - Fog calculation method enum (None, Linear, Exp, Exp2)
- `FogSettings` - Fog rendering settings with calculation methods
- `RefreshDefinition` - Core refresh/render definition with view angles, FOV
- `ExtendedRefreshDefinition` - Extended render options (fog, water, caustics)
- `DisplayMode` - Display mode info (resolution, refresh rate, bpp)
- `GammaSettings` - Gamma correction with ramp generation
- `VSyncMode`, `VSyncConfig` - VSync mode configuration
- `WindowState` - Window state tracking (minimized, focused, fullscreen)
- `CustomModelColor` - Custom model color configuration
- `VideoState` - Complete video subsystem state

**Helper Functions**:
- `hasFlag()` - Check render flags
- `fogCalculationName()` - Get fog type name
- `createCommonModes()` - Factory for standard resolutions
- `calcFovAdjustment()` - Hor+ FOV adjustment for widescreen

### Iteration 40: FragStats Types ✅

**Target**: Frag statistics and tracking types (36 tests)

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `fragstats.c` - msgtype_t | `fragstats_types.hpp` | FragMessageType enum ✅ |
| `fragstats.c` - wclass_t | `fragstats_types.hpp` | WeaponClassInfo struct ✅ |
| `fragstats.c` - fragmsg_t | `fragstats_types.hpp` | FragMessage struct ✅ |
| `fragstats.c` - fragdef_t | `fragstats_types.hpp` | FragfileDefinition struct ✅ |

**Key Types Created**:
- `fragstats_limits` namespace - Constants (MAX_FRAG_DEFINITIONS, etc.)
- `FragMessageType` - 12 message types (Fragged, Frags, TeamKill, Suicide, FlagCapture, etc.)
- `WeaponClass` - 15 weapon classes (Axe through LightningGun, plus Telefrag, Lava, etc.)
- `WeaponClassInfo` - Weapon class definition (keyword, name, shortName, imageName)
- `FragMessage` - Frag message pattern (message1, message2, type, weaponClassIndex)
- `FragfileDefinition` - Complete fragfile (title, author, messages, weapon classes)
- `WeaponFragStats` - Per-weapon statistics (kills, deaths, team kills, suicides)
- `PlayerFragStats` - Complete player stats with weapon breakdown and flags
- `quad_markers` namespace - Quad damage string markers for parsing
- `FragEvent` - Single frag event record (killer, victim, weapon, time)
- `FragParserState` - Frag message parser state
- `FragLog` - Event log with filtering (by player, by kill type)

**Helper Functions**:
- `messageTypeName()` - Get message type name
- `isKillEvent()`, `isDeathEvent()`, `isFlagEvent()` - Event classification
- `weaponClassName()`, `weaponClassFullName()` - Weapon name helpers
- `isActualWeapon()` - Distinguish weapons from environment
- `hasQuadMarker()` - Detect quad damage in frag messages
- `createDefaultWeaponClasses()` - Factory for standard weapon classes

### Iteration 41: Draw Types ✅

**Target**: 2D drawing and picture types (45 tests)

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `draw.h` - mpic_t | `draw_types.hpp` | Picture struct ✅ |
| `draw.h` - color_t | `draw_types.hpp` | Color class ✅ |
| `draw.h` - clrinfo_t | `draw_types.hpp` | ColorInfo struct ✅ |
| `draw.h` - text_alignment_t | `draw_types.hpp` | TextAlignment enum ✅ |
| `draw.h` - cache_pic_id_t | `draw_types.hpp` | CachedPicId enum ✅ |
| `draw.h` - WADPIC_* | `draw_types.hpp` | WadPicId enum ✅ |
| `draw.h` - wadpic_t | `draw_types.hpp` | WadPicture struct ✅ |

**Key Types Created**:
- `Color` class - 32-bit packed RGBA with component access and premultiply
- `TextAlignment` enum - Left, Center, Right for string rendering
- `ColorInfo` struct - Color + index for multi-colored text
- `TextureRef` struct - Opaque texture handle
- `Picture` struct - 2D picture with UV coordinates for atlas support
- `CachedPicId` enum - Menu/UI graphic identifiers with path lookup
- `WadPicId` enum - Status bar and HUD graphic identifiers
- `WadPicture` struct - WAD picture with name and picture pointer
- `ScissorRect` struct - Scissor clipping rectangle
- `DrawState` struct - Global draw state (alpha, scissor)
- `PictureCache` class - Picture cache manager
- `BigfontCharCoords` struct - Big font character layout
- `DrawRect` struct - 2D rectangle with fit/overlap utilities

**Helper Functions**:
- `textAlignmentName()` - Get alignment name
- `cachedPicPath()` - Get file path for cached pic ID
- `wadPicName()` - Get WAD picture name
- `getBigfontSourceCoords()` - Big font character coordinates
- `createDefaultDrawState()` - Factory for draw state
- `colorFromRGBA()`, `colorFromFloatRGBA()` - Color factories

### Iteration 42: Trigger Types ✅

**Target**: Regex trigger and message trigger types (39 tests)

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `tp_triggers.h` - RE_* | `trigger_types.hpp` | TriggerFlag enum ✅ |
| `tp_triggers.h` - pcre_trigger_t | `trigger_types.hpp` | RegexTrigger struct ✅ |
| `tp_triggers.h` - pcre_internal_trigger_t | `trigger_types.hpp` | InternalTrigger struct ✅ |

**Key Types Created**:
- `TriggerFlag` enum - Bitfield for trigger behavior (PrintLow, PrintHigh, Final, RemoveString, etc.)
- `MessageType` enum - Message types for filtering (Low, Medium, High, Chat, Center, Echo, Internal)
- `TriggerMatchResult` struct - Result of trigger match with flags
- `RegexTrigger` struct - User-defined regex trigger with rate limiting
- `InternalTrigger` struct - Programmatic trigger with callback
- `TriggerState` struct - Trigger manager state with find/remove operations
- `MessageTrigger` struct - Legacy pattern-action trigger
- `TriggerStats` struct - Statistics (messages processed, match rate)

**Helper Functions**:
- `hasFlag()` - Check if flag is set
- `messageTypeName()`, `messageTypeToFlag()` - Message type utilities
- `parseTriggerFlags()` - Parse flags from string ("lhcfr")
- `triggerFlagsToString()` - Convert flags to string
- `createTrigger()` - Factory for RegexTrigger
- `createMatchResult()` - Factory for match results

### Iteration 43: Logging Types ✅

**Target**: Logging, ignore, and flood protection types (38 tests)

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `sv_log.h` - log_t | `logging_types.hpp` | LogConfig struct ✅ |
| `sv_log.h` - log enum | `logging_types.hpp` | LogType enum ✅ |
| `ignore.c` - flood_t | `logging_types.hpp` | FloodProtection struct ✅ |
| `ignore.c` - ignore_t | `logging_types.hpp` | IgnoreEntry struct ✅ |

**Key Types Created**:
- `LogType` enum - Log types (Console, Error, Rcon, Telnet, Frag, Player, ModFrag)
- `LogLevel` enum - Severity (Debug, Info, Warning, Error, Critical, None)
- `LogEntry` struct - Single log entry with message, type, level, timestamp
- `LogConfig` struct - Per-log configuration (matches log_t from sv_log.h)
- `LogManagerState` struct - State for all server logs with enable/disable operations
- `FloodEntry` struct - Single flood tracking entry
- `FloodProtection` struct - Circular buffer flood detection (matches flood_t)
- `IgnoreMode` enum - Ignore modes (Disabled, Normal, Absolute)
- `SpectatorIgnore` enum - Spectator ignore modes (None, WhenPlaying, Always)
- `IgnoreEntry` struct - Per-player ignore state
- `IgnoreState` struct - Full ignore management with player/team ignore
- `ConsoleLogDest` enum - Console log destinations (None, File, Screen, Both)
- `ConsoleLogConfig` struct - Console log configuration

**Helper Functions**:
- `logTypeName()`, `logTypeDefaultFilename()` - Log type utilities
- `logLevelName()`, `shouldLog()` - Log level utilities
- `ignoreModeName()` - Ignore mode name
- `hasDestination()` - Check console log destination
- `createDefaultLogConfigs()` - Factory for all log configs
- `createDefaultIgnoreState()` - Factory for ignore state
- `formatLogEntry()` - Format log entry with optional timestamp

### Iteration 44: Image Types ✅

**Target**: Image format, screenshot, and skin types (40 tests)

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `client.h` - image_format_t | `image_types.hpp` | ImageFormat enum ✅ |
| `client.h` - scr_sshot_target_t | `image_types.hpp` | ScreenshotTarget struct ✅ |
| `client.h` - skin_t | `image_types.hpp` | Skin struct ✅ |
| `client.h` - skin_texture_t | `image_types.hpp` | SkinTextureSlot enum ✅ |
| `skin.c` - player_skin_t | `image_types.hpp` | PlayerSkin struct ✅ |

**Key Types Created**:
- `ImageFormat` enum - Image formats (PCX, TGA, JPEG, PNG)
- `ScreenshotTarget` struct - Screenshot capture target with buffer management
- `ScreenshotState` enum - Capture state machine
- `ScreenshotRequest` struct - Request with target and state
- `ScreenshotQueue` struct - Queue manager for screenshot requests
- `SkinTextureSlot` enum - Skin texture slots (Base, Teammate, Fullbright, Dead)
- `SkinTextureRef` struct - Opaque texture reference
- `Skin` struct - Player skin data (matches skin_t)
- `PlayerSkin` struct - Player's current skin assignment
- `SkinCache` struct - Cached skin manager
- `ImageData` struct - Raw image pixel data with manipulation

**Helper Functions**:
- `imageFormatExtension()`, `imageFormatMimeType()` - Format metadata
- `imageFormatSupportsAlpha()`, `imageFormatIsLossy()` - Format capabilities
- `skinTextureSlotName()`, `skinSlotIsTeammate()` - Slot utilities
- `parseImageFormat()` - Parse format from extension
- `generateScreenshotFilename()` - Auto-numbered filename generation
- `createDefaultPlayerSkins()` - Factory for player skin array
- `calculateImageBytes()` - Calculate buffer size needed

### Iteration 45: Hash Types ✅

**Target**: Hash, checksum, and digest types (34 tests)

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `crc.h` - CRC functions | `hash_types.hpp` | CRC16Context struct ✅ |
| `sha1.h` - SHA1_CTX | `hash_types.hpp` | SHA1Context struct ✅ |
| `md4.c` - MD4_CTX | `hash_types.hpp` | MD4Context struct ✅ |

**Key Types Created**:
- `HashAlgorithm` enum - Algorithm types (CRC16, MD4, SHA1, SHA256, XXHash)
- `Digest<N>` template - Fixed-size digest with hex conversion
- `MD4Digest`, `SHA1Digest`, `SHA256Digest` - Type aliases for common sizes
- `CRC16Context` struct - CRC-16-CCITT calculation context
- `MD4Context` struct - MD4 hash context (matches MD4_CTX)
- `SHA1Context` struct - SHA1 hash context (matches SHA1_CTX)
- `HashResult` struct - Generic hash result container
- `FileChecksum` struct - File checksum with metadata
- `KnownChecksum` struct - Expected checksum for verification
- `ChecksumStatus` enum - Verification result status
- `ChecksumEntry` struct - Verification entry with computed/expected values

**Helper Functions**:
- `hashDigestSize()`, `hashAlgorithmName()` - Algorithm metadata
- `hashIsCryptographic()` - Security classification
- `calculateCRC16()` - One-shot CRC calculation
- `compareDigestsConstantTime()` - Secure comparison
- `makeHashResult()` - Factory for hash results

### Iteration 46: Tracker Types ✅

**Target**: Frag/kill tracker HUD element types (42 tests)

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `vx_tracker.c` - trackmsg_t | `tracker_types.hpp` | TrackerMessage struct ✅ |
| `vx_tracker.c` - killing_streak_t | `tracker_types.hpp` | KillingStreak struct ✅ |
| `vx_tracker.c` - weapon_label_t | `tracker_types.hpp` | WeaponLabel struct ✅ |

**Key Types Created**:
- `SegmentFlag` enum - Bitfield flags for segments (Weapon, Highlight, Own, TeamKill)
- `TrackerAlignment` enum - Text alignment (Left, Center, Right)
- `TrackerColor` struct - RGBA color with predefined colors
- `TrackerSegment` struct - Single segment with text, color, image
- `TrackerMessage` struct - Complete message with multiple segments (matches trackmsg_t)
- `KillingStreak` struct - Streak definition with threshold and sound
- `WeaponLabel` struct - Weapon display with color sections
- `TrackerConfig` struct - Display configuration (enabled, colors, strings)
- `TrackerState` struct - Full tracker state with message queue
- `TrackerEventType` enum - Event types (Frag, Suicide, TeamKill, FlagCapture, etc.)
- `TrackerEvent` struct - Event for processing

**Helper Functions**:
- `hasFlag()` - Check segment flags
- `createDefaultStreaks()` - Factory for streak definitions
- `findStreak()` - Find streak for frag count
- `trackerEventTypeName()` - Get event type name
- `parseTrackerColor()` - Parse color from "0-9" format
- `shouldFilterEvent()` - Check if event should be filtered

### Iteration 47: Charset Types ✅

**Target**: Character set and font rendering types (36 tests)

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `r_texture.h` - charset_t | `charset_types.hpp` | Charset struct ✅ |
| `fonts.c` - gradient_def_t | `charset_types.hpp` | FontGradient struct ✅ |
| `fonts.c` - glyphinfo_t | `charset_types.hpp` | GlyphInfo struct ✅ |

**Key Types Created**:
- `GlyphCoords` struct - Texture coordinates (s1, t1, s2, t2) for glyph UV mapping
- `GlyphInfo` struct - Glyph metrics (width, height, advance, bearing_x, bearing_y)
- `CharsetTextureRef` struct - Opaque texture reference for charset textures
- `CharsetPic` struct - Picture data matching mpic_t for individual glyphs
- `Charset` struct - Full character set with 256 glyphs (matches charset_t)
- `FontColor` struct - RGBA color with lerp interpolation and predefined colors
- `FontGradient` struct - Gradient definition (top/bottom color, start position)
- `FontConfig` struct - Font rendering configuration (scale, spacing, smoothing)
- `CharsetManager` struct - Manager for all charsets and proportional fonts
- `CharCategory` enum - Character classification (Printable, Control, Whitespace, etc.)
- `TextMetrics` struct - Text measurement result (width, height, line count)

**Constants**:
- `MAX_CHARSETS = 256` - Maximum number of charsets
- `MAX_USER_CHARSETS = 0xE0` - User-loadable charsets
- `CHARSET_CHARS = 256` - Characters per charset
- `CHARSET_CHARS_PER_ROW = 16` - Characters per row in texture

**Helper Functions**:
- `measureText()` - Calculate text dimensions
- `fixedWidth()` - Get fixed-width character size
- `charsetFromWideChar()` - Map wide character to charset/glyph index
- `glyphIndexFromWideChar()` - Get glyph index within charset
- `categorizeChar()` - Classify character type
- `createDefaultFontConfig()` - Factory for font configuration

### Iteration 48: Shader Types ✅

**Target**: Shader and GPU program management types (49 tests)

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `r_program.h` - r_program_id | `shader_types.hpp` | ProgramId enum ✅ |
| `r_program.h` - r_program_uniform_id | `shader_types.hpp` | UniformId enum ✅ |
| `r_program.h` - r_program_attribute_id | `shader_types.hpp` | AttributeId enum ✅ |
| `gl_framebuffer.h` - framebuffer_id | `shader_types.hpp` | FramebufferId enum ✅ |
| `gl_framebuffer.h` - fbtex_id | `shader_types.hpp` | FramebufferTextureId enum ✅ |

**Key Types Created**:
- `ShaderType` enum - Shader stage types (Vertex, Fragment, Geometry, Compute, etc.)
- `ProgramId` enum - All shader program identifiers (matches r_program_id)
- `UniformId` enum - All uniform variable identifiers (matches r_program_uniform_id)
- `AttributeId` enum - Vertex attribute identifiers (matches r_program_attribute_id)
- `MemoryBarrierId` enum - Compute shader memory barriers
- `UniformType` enum - Data types for uniforms (Float1, Vec3, Mat4, Sampler2D, etc.)
- `UniformValue` struct - Type-safe union for uniform values
- `ShaderSource` struct - Reference to shader source code
- `ProgramDefinition` struct - Definition for creating shader programs
- `ProgramState` struct - Runtime state with cached uniform/attribute locations
- `ShaderManager` struct - Manages all shader programs
- `ComputeDispatchInfo` struct - Compute shader dispatch configuration
- `FramebufferId` enum - Framebuffer identifiers (matches framebuffer_id)
- `FramebufferTextureId` enum - FBO texture attachments (matches fbtex_id)
- `FramebufferState` struct - Runtime framebuffer state

**Helper Functions**:
- `getShaderTypeName()` - Get shader type display name
- `getShaderExtension()` - Get GLSL file extension
- `getProgramName()` - Get program display name
- `isGlcProgram()` - Check if program is GLC (classic) variant
- `isComputeProgram()` - Check if program is compute shader
- `getUniformName()` - Get uniform variable name
- `needsDepthBuffer()` - Check if framebuffer needs depth attachment
- `isMultisampled()` - Check if framebuffer is multisampled
- `createDispatch2D()` - Create compute dispatch for 2D workload
- `createDispatch1D()` - Create compute dispatch for 1D workload

### Iteration 49: System Types ✅

**Target**: System/platform types for time, files, and errors (43 tests)

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `localtime.h` - SYSTEMTIME | `system_types.hpp` | SystemTime struct ✅ |
| `sys.h` - timerresolution_session_t | `system_types.hpp` | TimerResolutionSession ✅ |
| `sys.h` - file_t | `system_types.hpp` | FileInfo struct ✅ |

**Key Types Created**:
- `SystemTime` struct - Cross-platform time (matches Windows SYSTEMTIME)
- `TimerResolutionSession` struct - High-precision timer session for Windows
- `FileInfo` struct - File information entry (matches file_t)
- `OSType` enum - Operating system detection (Windows, Linux, macOS, FreeBSD)
- `CPUArch` enum - CPU architecture detection (x86, x86_64, ARM, ARM64)
- `BuildType` enum - Build configuration type
- `PlatformInfo` struct - Platform information with SIMD detection
- `ErrorLevel` enum - System error levels (None, Warning, Error, Fatal)
- `SystemError` struct - System error with level, code, and message
- `ThreadPriority` enum - Thread priority levels
- `ThreadInfo` struct - Thread information
- `PerformanceCounter` struct - High-resolution timing
- `FrameTiming` struct - Frame timing with FPS calculation

**Constants**:
- `MAX_DIRFILES = 4096` - Maximum directory entries
- `MAX_DEMO_NAME = 196` - Maximum demo filename length
- `MAX_PATH_LENGTH = 4096` - Maximum path length

**Helper Functions**:
- `getOSName()` - Get OS display name
- `getCurrentOS()` - Detect current OS at compile time
- `getCPUArchName()` - Get architecture name
- `getCurrentCPUArch()` - Detect CPU architecture at compile time
- `shouldTerminate()` - Check if error level is fatal
- `makeSystemTime()` - Create SystemTime from components
- `diffSeconds()` - Calculate time difference
- `formatTimeHMS()` - Format time as HH:MM:SS

### Iteration 50: Corona Types ✅

**Target**: Corona and visual effect types for rendering (74 tests)

| Original C | C++ Replacement | Notes |
|------------|-----------------|-------|
| `vx_stuff.h` - coronatype_t | `corona_types.hpp` | CoronaType enum ✅ |
| `vx_stuff.h` - corona_texture_id | `corona_types.hpp` | CoronaTextureId enum ✅ |
| `vx_stuff.h` - corona_texture_t | `corona_types.hpp` | CoronaTexture struct ✅ |
| `vx_stuff.h` - cameramode_t | `corona_types.hpp` | CameraMode enum ✅ |

**Key Types Created**:
- `CoronaType` enum - Corona visual types (Flash, Lightning, Fire, Explode, etc.)
- `CoronaTextureId` enum - Corona texture identifiers with explosion frames
- `CoronaColor` struct - RGB color with preset colors and blending
- `TextureHandle` struct - Placeholder texture reference handle
- `CoronaTexture` struct - Texture with array texture support
- `CameraMode` enum - Camera viewing modes (Normal, ChaseCam, External)
- `Position3D` struct - 3D world position
- `Corona` struct - Active corona instance with lifetime, animation
- `CoronaTextureArray` class - Container for all corona textures
- `CoronaStats` struct - Corona rendering statistics
- `CoronaConfig` struct - Corona effect configuration
- `WeatherType` enum - Weather effect types (None, Rain, Snow, Sleet)
- `WeatherConfig` struct - Weather effect configuration
- `LightningConfig` struct - Lightning beam configuration
- `VisualEffectState` struct - Combined visual effect state

**Helper Functions**:
- `coronaTypeName()` - Get corona type display name
- `isFlashType()` - Check if corona is flash type
- `isLightningType()` - Check if corona is lightning
- `isEnemyTracerType()` - Check if corona is enemy tracer
- `isAnimatedType()` - Check if corona uses animation
- `coronaTextureName()` - Get texture name
- `isExplosionTexture()` - Check if texture is explosion frame
- `getExplosionFrame()` - Get frame index from texture ID
- `explosionFrameTexture()` - Get texture for frame number
- `getCoronaColor()` - Get default color for type
- `getCoronaRadius()` - Get default radius for type
- `getCoronaLifetime()` - Get default lifetime for type
- `cameraModeName()` - Get camera mode name
- `isThirdPerson()` - Check if camera is third-person
- `shouldShowPlayerModel()` - Check if player model should render
- `weatherTypeName()` - Get weather type name

**Constants**:
- `MAX_CORONAS = 1024` - Maximum simultaneous coronas
- `DEFAULT_RADIUS = 32.0f` - Default corona radius
- `EXPLOSION_FRAMES = 7` - Explosion animation frame count
- `EXPLOSION_FRAME_TIME = 0.05f` - Time per explosion frame

### Iteration 26-30: Rendering Advanced

- Particle systems (Classic, QMB)
- Lightmap calculation and updates
- Dynamic lighting
- Sky rendering
- Water/warp surfaces
- Bloom and post-processing

### Iteration 31-35: UI/HUD System

- HUD element framework
- Menu system
- Server browser
- Settings pages
- EZ controls (buttons, lists, etc.)

### Iteration 36-38: QuakeC VM

- Bytecode interpreter
- Builtin functions
- Edict management
- String table

### Iteration 39-40: GPU Compute

- CUDA/compute abstraction
- Particle physics kernels
- Lightmap update kernels
- Runtime CPU/GPU toggle

## C Compatibility

The `compat/c_bridge.h` header provides C-callable wrappers around C++ functionality:

```c
// In C code:
#include "compat/c_bridge.h"

float v[3] = {1, 0, 0};
VectorNormalize_CPP(v);  // Uses C++ Vec3 internally
```

This allows gradual migration where C code can start using C++ implementations
before being fully converted.

**Global Instances**: For C compatibility, global instances are created in `c_bridge.cpp`
and accessed via `compat/global_instances.hpp`. These are the ONLY globals in the codebase.

## Known Limitations

1. ~~`std::span` is C++20, so we use a custom `Span<T>` for C++17~~ **Resolved: Upgraded to C++20**
2. ~~Some `constexpr` functions use `std::swap` which isn't constexpr until C++20~~ **Resolved: C++20**
3. Warnings from GoogleTest headers are suppressed automatically
4. CUDA requires NVIDIA GPU and CUDA toolkit 11.0+
