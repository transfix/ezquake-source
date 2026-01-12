/**
 * @file vm_types.hpp
 * @brief QuakeC Virtual Machine types and structures
 * 
 * Converted from:
 * - pr_comp.h: Progs file format and bytecode
 * - progs.h: Edict and function definitions
 * - vm_local.h: VM execution structures
 */

#pragma once

#include <cstdint>
#include <array>
#include <string_view>
#include <variant>
#include "core/math/vec3.hpp"

namespace ezquake::vm {

// =============================================================================
// VM Constants
// =============================================================================

namespace vm_limits {
    // Progs version
    constexpr int32_t PROG_VERSION = 6;
    
    // VM magic numbers
    constexpr uint32_t VM_MAGIC = 0x12721444;
    constexpr uint32_t VM_MAGIC_VER2 = 0x12721445;
    
    // Parameter and stack limits
    constexpr int32_t MAX_PARMS = 8;
    constexpr int32_t MAX_OPSTACK_SIZE = 512;
    constexpr int32_t PROC_OPSTACK_SIZE = 30;
    constexpr int32_t PROGRAM_STACK_SIZE = 0x10000;
    constexpr int32_t PROGRAM_STACK_EXTRA = 32 * 1024;
    
    // String table limits
    constexpr int32_t MAX_PRSTR = 1024;
    
    // Entity limits
    constexpr int32_t MAX_ENT_LEAFS = 16;
    
    // Reserved offsets
    constexpr int32_t OFS_NULL = 0;
    constexpr int32_t OFS_RETURN = 1;
    constexpr int32_t OFS_PARM0 = 4;
    constexpr int32_t OFS_PARM1 = 7;
    constexpr int32_t OFS_PARM2 = 10;
    constexpr int32_t OFS_PARM3 = 13;
    constexpr int32_t OFS_PARM4 = 16;
    constexpr int32_t OFS_PARM5 = 19;
    constexpr int32_t OFS_PARM6 = 22;
    constexpr int32_t OFS_PARM7 = 25;
    constexpr int32_t RESERVED_OFS = 28;
    
    // Definition flags
    constexpr uint16_t DEF_SAVEGLOBAL = (1 << 15);
}

// =============================================================================
// Value Types
// =============================================================================

/**
 * @brief QuakeC type enumeration
 */
enum class EType : uint8_t {
    Void = 0,
    String = 1,
    Float = 2,
    Vector = 3,
    Entity = 4,
    Field = 5,
    Function = 6,
    Pointer = 7
};

[[nodiscard]] constexpr const char* getTypeName(EType type) noexcept {
    switch (type) {
        case EType::Void: return "void";
        case EType::String: return "string";
        case EType::Float: return "float";
        case EType::Vector: return "vector";
        case EType::Entity: return "entity";
        case EType::Field: return "field";
        case EType::Function: return "function";
        case EType::Pointer: return "pointer";
        default: return "unknown";
    }
}

// =============================================================================
// Eval Union (PR1 value type)
// =============================================================================

/**
 * @brief Union for QuakeC value storage
 */
union EvalValue {
    int32_t stringRef;     // String reference
    float floatVal;        // Float value
    std::array<float, 3> vector;  // Vector value
    int32_t function;      // Function reference
    int32_t intVal;        // Integer value
    int32_t edict;         // Edict reference
    
    constexpr EvalValue() noexcept : intVal{0} {}
    
    [[nodiscard]] constexpr float x() const noexcept { return vector[0]; }
    [[nodiscard]] constexpr float y() const noexcept { return vector[1]; }
    [[nodiscard]] constexpr float z() const noexcept { return vector[2]; }
};

// =============================================================================
// PR1 Opcodes (QuakeC bytecode)
// =============================================================================

/**
 * @brief PR1 (QuakeC) bytecode opcodes
 */
enum class PR1Opcode : uint16_t {
    Done = 0,
    MulF = 1,
    MulV = 2,
    MulFV = 3,
    MulVF = 4,
    DivF = 5,
    AddF = 6,
    AddV = 7,
    SubF = 8,
    SubV = 9,
    
    EqF = 10,
    EqV = 11,
    EqS = 12,
    EqE = 13,
    EqFnc = 14,
    
    NeF = 15,
    NeV = 16,
    NeS = 17,
    NeE = 18,
    NeFnc = 19,
    
    Le = 20,
    Ge = 21,
    Lt = 22,
    Gt = 23,
    
    LoadF = 24,
    LoadV = 25,
    LoadS = 26,
    LoadEnt = 27,
    LoadFld = 28,
    LoadFnc = 29,
    
    Address = 30,
    
    StoreF = 31,
    StoreV = 32,
    StoreS = 33,
    StoreEnt = 34,
    StoreFld = 35,
    StoreFnc = 36,
    
    StorepF = 37,
    StorepV = 38,
    StorepS = 39,
    StorepEnt = 40,
    StorepFld = 41,
    StorepFnc = 42,
    
    Return = 43,
    NotF = 44,
    NotV = 45,
    NotS = 46,
    NotEnt = 47,
    NotFnc = 48,
    If = 49,
    IfNot = 50,
    Call0 = 51,
    Call1 = 52,
    Call2 = 53,
    Call3 = 54,
    Call4 = 55,
    Call5 = 56,
    Call6 = 57,
    Call7 = 58,
    Call8 = 59,
    State = 60,
    Goto = 61,
    And = 62,
    Or = 63,
    
    BitAnd = 64,
    BitOr = 65
};

[[nodiscard]] constexpr bool isCallOpcode(PR1Opcode op) noexcept {
    return op >= PR1Opcode::Call0 && op <= PR1Opcode::Call8;
}

[[nodiscard]] constexpr int getCallArgCount(PR1Opcode op) noexcept {
    if (!isCallOpcode(op)) return -1;
    return static_cast<int>(op) - static_cast<int>(PR1Opcode::Call0);
}

// =============================================================================
// Q3 VM Opcodes
// =============================================================================

/**
 * @brief Q3 VM bytecode opcodes
 */
enum class Q3Opcode : uint8_t {
    Undef = 0,
    Ignore = 1,
    Break = 2,
    Enter = 3,
    Leave = 4,
    Call = 5,
    Push = 6,
    Pop = 7,
    Const = 8,
    Local = 9,
    Jump = 10,
    
    Eq = 11,
    Ne = 12,
    
    LtI = 13,
    LeI = 14,
    GtI = 15,
    GeI = 16,
    
    LtU = 17,
    LeU = 18,
    GtU = 19,
    GeU = 20,
    
    EqF = 21,
    NeF = 22,
    
    LtF = 23,
    LeF = 24,
    GtF = 25,
    GeF = 26,
    
    Load1 = 27,
    Load2 = 28,
    Load4 = 29,
    Store1 = 30,
    Store2 = 31,
    Store4 = 32,
    Arg = 33,
    
    BlockCopy = 34,
    
    Sex8 = 35,
    Sex16 = 36,
    
    NegI = 37,
    Add = 38,
    Sub = 39,
    DivI = 40,
    DivU = 41,
    ModI = 42,
    ModU = 43,
    MulI = 44,
    MulU = 45,
    
    Band = 46,
    Bor = 47,
    Bxor = 48,
    Bcom = 49,
    
    Lsh = 50,
    RshI = 51,
    RshU = 52,
    
    NegF = 53,
    AddF = 54,
    SubF = 55,
    DivF = 56,
    MulF = 57,
    
    CvIF = 58,
    CvFI = 59,
    
    Max = 60
};

// =============================================================================
// Statement (PR1 instruction)
// =============================================================================

/**
 * @brief PR1 bytecode statement/instruction
 */
struct Statement {
    uint16_t op{};    // Opcode
    int16_t a{};      // First operand
    int16_t b{};      // Second operand
    int16_t c{};      // Third operand/destination
    
    [[nodiscard]] constexpr PR1Opcode opcode() const noexcept {
        return static_cast<PR1Opcode>(op);
    }
    
    [[nodiscard]] constexpr bool isDone() const noexcept {
        return op == 0;
    }
    
    [[nodiscard]] constexpr bool isJump() const noexcept {
        return opcode() == PR1Opcode::Goto || 
               opcode() == PR1Opcode::If || 
               opcode() == PR1Opcode::IfNot;
    }
    
    [[nodiscard]] constexpr bool isCall() const noexcept {
        return isCallOpcode(opcode());
    }
};

static_assert(sizeof(Statement) == 8, "Statement must be 8 bytes");

// =============================================================================
// Definition (global/field definition)
// =============================================================================

/**
 * @brief Global or field definition
 */
struct Definition {
    uint16_t type{};      // Type with flags
    uint16_t offset{};    // Offset in globals/entity
    int32_t nameOffset{}; // String offset for name
    
    [[nodiscard]] constexpr EType baseType() const noexcept {
        return static_cast<EType>(type & 0x7FFF);
    }
    
    [[nodiscard]] constexpr bool needsSave() const noexcept {
        return (type & vm_limits::DEF_SAVEGLOBAL) != 0;
    }
};

static_assert(sizeof(Definition) == 8, "Definition must be 8 bytes");

// =============================================================================
// Function Definition
// =============================================================================

/**
 * @brief Function definition structure
 */
struct FunctionDef {
    int32_t firstStatement{};  // Negative = builtin number
    int32_t parmStart{};       // First parameter offset
    int32_t locals{};          // Total ints of parms + locals
    int32_t profile{};         // Runtime profile counter
    int32_t nameOffset{};      // String offset for name
    int32_t fileOffset{};      // String offset for source file
    int32_t numParms{};        // Number of parameters
    std::array<uint8_t, 8> parmSize{};  // Size of each parameter
    
    [[nodiscard]] constexpr bool isBuiltin() const noexcept {
        return firstStatement < 0;
    }
    
    [[nodiscard]] constexpr int builtinNumber() const noexcept {
        return isBuiltin() ? -firstStatement : 0;
    }
    
    [[nodiscard]] constexpr int totalParmSize() const noexcept {
        int total = 0;
        for (int i = 0; i < numParms && i < 8; ++i) {
            total += parmSize[static_cast<size_t>(i)];
        }
        return total;
    }
};

// =============================================================================
// Progs Header
// =============================================================================

/**
 * @brief Progs file header (dprograms_t)
 */
struct ProgsHeader {
    int32_t version{};
    int32_t crc{};
    
    int32_t statementsOffset{};
    int32_t numStatements{};
    
    int32_t globalDefsOffset{};
    int32_t numGlobalDefs{};
    
    int32_t fieldDefsOffset{};
    int32_t numFieldDefs{};
    
    int32_t functionsOffset{};
    int32_t numFunctions{};
    
    int32_t stringsOffset{};
    int32_t numStrings{};
    
    int32_t globalsOffset{};
    int32_t numGlobals{};
    
    int32_t entityFields{};
    
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return version == vm_limits::PROG_VERSION;
    }
    
    [[nodiscard]] constexpr int32_t entitySize() const noexcept {
        return entityFields * static_cast<int32_t>(sizeof(float));
    }
};

// =============================================================================
// Q3 VM Header
// =============================================================================

/**
 * @brief Q3 VM file header
 */
struct VMHeader {
    uint32_t magic{};
    int32_t instructionCount{};
    int32_t codeOffset{};
    int32_t codeLength{};
    int32_t dataOffset{};
    int32_t dataLength{};
    int32_t litLength{};   // dataLength - litLength should be byteswapped
    int32_t bssLength{};   // Zero-filled memory appended
    // VER2 only:
    int32_t jtrgLength{};  // Jump table targets
    
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return magic == vm_limits::VM_MAGIC || magic == vm_limits::VM_MAGIC_VER2;
    }
    
    [[nodiscard]] constexpr bool isVersion2() const noexcept {
        return magic == vm_limits::VM_MAGIC_VER2;
    }
    
    [[nodiscard]] constexpr int32_t dataSwapLength() const noexcept {
        return dataLength - litLength;
    }
    
    [[nodiscard]] constexpr int32_t totalDataSize() const noexcept {
        return dataLength + bssLength;
    }
};

// =============================================================================
// Q3 VM Instruction
// =============================================================================

/**
 * @brief Q3 VM instruction
 */
struct VMInstruction {
    int32_t value{};      // Operand value
    uint8_t op{};         // Opcode
    uint8_t opStack{};    // Stack offset
    uint8_t flags{};      // jused:1, swtch:1
    uint8_t padding{};
    
    [[nodiscard]] constexpr Q3Opcode opcode() const noexcept {
        return static_cast<Q3Opcode>(op);
    }
    
    [[nodiscard]] constexpr bool isJumpTarget() const noexcept {
        return (flags & 0x01) != 0;
    }
    
    [[nodiscard]] constexpr bool isSwitch() const noexcept {
        return (flags & 0x02) != 0;
    }
};

// =============================================================================
// VM Symbol
// =============================================================================

/**
 * @brief VM debug symbol
 */
struct VMSymbol {
    int32_t value{};          // Symbol address
    int32_t profileCount{};   // Profile hit count
    std::string_view name;    // Symbol name
};

// =============================================================================
// VM Index
// =============================================================================

/**
 * @brief VM instance identifier
 */
enum class VMIndex : uint8_t {
    Game = 0,
    CGame = 1,
    UI = 2,
    
    Count = 3
};

[[nodiscard]] constexpr const char* getVMIndexName(VMIndex index) noexcept {
    switch (index) {
        case VMIndex::Game: return "game";
        case VMIndex::CGame: return "cgame";
        case VMIndex::UI: return "ui";
        default: return "unknown";
    }
}

// =============================================================================
// VM State
// =============================================================================

/**
 * @brief VM execution state
 */
struct VMState {
    uint32_t programStack{};     // Current stack position
    int32_t instructionCount{};  // Total instructions
    bool compiled{false};        // JIT compiled?
    bool tracing{false};         // Trace execution?
    
    [[nodiscard]] constexpr bool isActive() const noexcept {
        return instructionCount > 0;
    }
};

// =============================================================================
// Edict Link
// =============================================================================

/**
 * @brief Linked list node for edict area links
 */
struct EdictLink {
    int32_t edictNum{-1};     // Edict number (-1 = none)
    int32_t prevIndex{-1};    // Previous link index
    int32_t nextIndex{-1};    // Next link index
    
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return edictNum >= 0;
    }
};

// =============================================================================
// Server Edict
// =============================================================================

/**
 * @brief Server-side edict state
 */
struct ServerEdict {
    bool free{true};                  // Entity is free
    int32_t edictNum{};               // Entity number
    int32_t numLeafs{};               // Number of touching leafs
    std::array<int16_t, 16> leafNums{};  // Touching leaf numbers
    float freeTime{};                 // Time entity was freed
    double lastRunTime{};             // Last SV_RunEntity time
    
    [[nodiscard]] constexpr bool isFree() const noexcept {
        return free;
    }
    
    [[nodiscard]] constexpr bool isActive() const noexcept {
        return !free;
    }
};

// =============================================================================
// Extended Entity Variables
// =============================================================================

/**
 * @brief Extended entity variables (not in QC)
 */
struct ExtEntVars {
    float alpha{1.0f};                // Transparency (0 = opaque in QW!)
    std::array<float, 3> colorMod{1.0f, 1.0f, 1.0f};  // Color modulation
    
    [[nodiscard]] constexpr bool isOpaque() const noexcept {
        return alpha <= 0.0f || alpha >= 1.0f;
    }
    
    [[nodiscard]] constexpr bool hasColorMod() const noexcept {
        return colorMod[0] != 1.0f || colorMod[1] != 1.0f || colorMod[2] != 1.0f;
    }
};

// =============================================================================
// Builtin Function Info
// =============================================================================

/**
 * @brief Information about a builtin function
 */
struct BuiltinInfo {
    int32_t number{};         // Builtin number
    std::string_view name;    // Function name
    int32_t minArgs{};        // Minimum arguments
    int32_t maxArgs{};        // Maximum arguments
    
    [[nodiscard]] constexpr bool isVariadic() const noexcept {
        return maxArgs > minArgs;
    }
    
    [[nodiscard]] constexpr bool acceptsArgCount(int count) const noexcept {
        return count >= minArgs && count <= maxArgs;
    }
};

// =============================================================================
// Field Offset Cache
// =============================================================================

/**
 * @brief Cached field offsets for common fields
 */
struct FieldOffsets {
    int32_t items2{-1};        // ZQ_ITEMS2 extension
    int32_t vwIndex{-1};       // ZQ_VWEP viewmodel index
    int32_t movement{-1};      // Movement vector
    int32_t gravity{-1};       // Entity gravity
    int32_t maxSpeed{-1};      // Entity max speed
    int32_t hideEntity{-1};    // Hide from clients
    int32_t trackEnt{-1};      // Track entity
    int32_t visibility{-1};    // Visibility mask
    int32_t hidePlayers{-1};   // Hide from players
    int32_t teleported{-1};    // Teleported flag
    
    [[nodiscard]] constexpr bool hasItems2() const noexcept { return items2 >= 0; }
    [[nodiscard]] constexpr bool hasVWep() const noexcept { return vwIndex >= 0; }
    [[nodiscard]] constexpr bool hasMovement() const noexcept { return movement >= 0; }
};

// =============================================================================
// VM Statistics
// =============================================================================

/**
 * @brief VM execution statistics
 */
struct VMStats {
    uint64_t instructionsExecuted{};
    uint64_t functionCalls{};
    uint64_t builtinCalls{};
    uint64_t stackMaxDepth{};
    
    constexpr void reset() noexcept {
        instructionsExecuted = 0;
        functionCalls = 0;
        builtinCalls = 0;
        stackMaxDepth = 0;
    }
    
    constexpr void addInstruction() noexcept {
        ++instructionsExecuted;
    }
    
    constexpr void addCall(bool isBuiltin) noexcept {
        ++functionCalls;
        if (isBuiltin) ++builtinCalls;
    }
    
    constexpr void updateStackDepth(uint64_t depth) noexcept {
        if (depth > stackMaxDepth) stackMaxDepth = depth;
    }
};

} // namespace ezquake::vm
