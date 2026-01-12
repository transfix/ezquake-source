/**
 * @file vm_test.cpp
 * @brief Tests for QuakeC Virtual Machine types
 */

#include <gtest/gtest.h>
#include "core/vm/vm_types.hpp"

using namespace ezquake::vm;

// =============================================================================
// VM Limits Tests
// =============================================================================

TEST(VMLimitsTest, ProgVersion) {
    EXPECT_EQ(vm_limits::PROG_VERSION, 6);
}

TEST(VMLimitsTest, MagicNumbers) {
    EXPECT_EQ(vm_limits::VM_MAGIC, 0x12721444);
    EXPECT_EQ(vm_limits::VM_MAGIC_VER2, 0x12721445);
}

TEST(VMLimitsTest, StackLimits) {
    EXPECT_EQ(vm_limits::MAX_PARMS, 8);
    EXPECT_EQ(vm_limits::MAX_OPSTACK_SIZE, 512);
    EXPECT_EQ(vm_limits::PROGRAM_STACK_SIZE, 0x10000);
}

TEST(VMLimitsTest, Offsets) {
    EXPECT_EQ(vm_limits::OFS_NULL, 0);
    EXPECT_EQ(vm_limits::OFS_RETURN, 1);
    EXPECT_EQ(vm_limits::OFS_PARM0, 4);
    EXPECT_EQ(vm_limits::RESERVED_OFS, 28);
}

// =============================================================================
// EType Tests
// =============================================================================

TEST(ETypeTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(EType::Void), 0);
    EXPECT_EQ(static_cast<uint8_t>(EType::String), 1);
    EXPECT_EQ(static_cast<uint8_t>(EType::Float), 2);
    EXPECT_EQ(static_cast<uint8_t>(EType::Vector), 3);
    EXPECT_EQ(static_cast<uint8_t>(EType::Entity), 4);
}

TEST(ETypeTest, Names) {
    EXPECT_STREQ(getTypeName(EType::Void), "void");
    EXPECT_STREQ(getTypeName(EType::Float), "float");
    EXPECT_STREQ(getTypeName(EType::Vector), "vector");
    EXPECT_STREQ(getTypeName(EType::Function), "function");
}

// =============================================================================
// EvalValue Tests
// =============================================================================

TEST(EvalValueTest, DefaultConstruction) {
    EvalValue val;
    EXPECT_EQ(val.intVal, 0);
}

TEST(EvalValueTest, VectorAccess) {
    EvalValue val;
    val.vector = {1.0f, 2.0f, 3.0f};
    
    EXPECT_FLOAT_EQ(val.x(), 1.0f);
    EXPECT_FLOAT_EQ(val.y(), 2.0f);
    EXPECT_FLOAT_EQ(val.z(), 3.0f);
}

// =============================================================================
// PR1 Opcode Tests
// =============================================================================

TEST(PR1OpcodeTest, BasicOpcodes) {
    EXPECT_EQ(static_cast<uint16_t>(PR1Opcode::Done), 0);
    EXPECT_EQ(static_cast<uint16_t>(PR1Opcode::MulF), 1);
    EXPECT_EQ(static_cast<uint16_t>(PR1Opcode::AddF), 6);
}

TEST(PR1OpcodeTest, CallOpcodes) {
    EXPECT_EQ(static_cast<uint16_t>(PR1Opcode::Call0), 51);
    EXPECT_EQ(static_cast<uint16_t>(PR1Opcode::Call8), 59);
}

TEST(PR1OpcodeTest, IsCallOpcode) {
    EXPECT_FALSE(isCallOpcode(PR1Opcode::Done));
    EXPECT_FALSE(isCallOpcode(PR1Opcode::Return));
    EXPECT_TRUE(isCallOpcode(PR1Opcode::Call0));
    EXPECT_TRUE(isCallOpcode(PR1Opcode::Call5));
    EXPECT_TRUE(isCallOpcode(PR1Opcode::Call8));
}

TEST(PR1OpcodeTest, GetCallArgCount) {
    EXPECT_EQ(getCallArgCount(PR1Opcode::Call0), 0);
    EXPECT_EQ(getCallArgCount(PR1Opcode::Call3), 3);
    EXPECT_EQ(getCallArgCount(PR1Opcode::Call8), 8);
    EXPECT_EQ(getCallArgCount(PR1Opcode::Done), -1);
}

// =============================================================================
// Q3 Opcode Tests
// =============================================================================

TEST(Q3OpcodeTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(Q3Opcode::Undef), 0);
    EXPECT_EQ(static_cast<uint8_t>(Q3Opcode::Enter), 3);
    EXPECT_EQ(static_cast<uint8_t>(Q3Opcode::Call), 5);
    EXPECT_EQ(static_cast<uint8_t>(Q3Opcode::Max), 60);
}

// =============================================================================
// Statement Tests
// =============================================================================

TEST(StatementTest, Size) {
    EXPECT_EQ(sizeof(Statement), 8);
}

TEST(StatementTest, IsDone) {
    Statement stmt;
    EXPECT_TRUE(stmt.isDone());
    
    stmt.op = 1;
    EXPECT_FALSE(stmt.isDone());
}

TEST(StatementTest, IsJump) {
    Statement stmt;
    stmt.op = static_cast<uint16_t>(PR1Opcode::Goto);
    EXPECT_TRUE(stmt.isJump());
    
    stmt.op = static_cast<uint16_t>(PR1Opcode::If);
    EXPECT_TRUE(stmt.isJump());
    
    stmt.op = static_cast<uint16_t>(PR1Opcode::MulF);
    EXPECT_FALSE(stmt.isJump());
}

TEST(StatementTest, IsCall) {
    Statement stmt;
    stmt.op = static_cast<uint16_t>(PR1Opcode::Call3);
    EXPECT_TRUE(stmt.isCall());
    
    stmt.op = static_cast<uint16_t>(PR1Opcode::Return);
    EXPECT_FALSE(stmt.isCall());
}

// =============================================================================
// Definition Tests
// =============================================================================

TEST(DefinitionTest, Size) {
    EXPECT_EQ(sizeof(Definition), 8);
}

TEST(DefinitionTest, BaseType) {
    Definition def;
    def.type = static_cast<uint16_t>(EType::Vector);
    
    EXPECT_EQ(def.baseType(), EType::Vector);
}

TEST(DefinitionTest, NeedsSave) {
    Definition def;
    EXPECT_FALSE(def.needsSave());
    
    def.type = vm_limits::DEF_SAVEGLOBAL | static_cast<uint16_t>(EType::Float);
    EXPECT_TRUE(def.needsSave());
    EXPECT_EQ(def.baseType(), EType::Float);
}

// =============================================================================
// Function Definition Tests
// =============================================================================

TEST(FunctionDefTest, IsBuiltin) {
    FunctionDef func;
    EXPECT_FALSE(func.isBuiltin());
    
    func.firstStatement = -15;
    EXPECT_TRUE(func.isBuiltin());
    EXPECT_EQ(func.builtinNumber(), 15);
}

TEST(FunctionDefTest, TotalParmSize) {
    FunctionDef func;
    func.numParms = 3;
    func.parmSize = {3, 1, 3, 0, 0, 0, 0, 0};  // vec, float, vec
    
    EXPECT_EQ(func.totalParmSize(), 7);
}

// =============================================================================
// Progs Header Tests
// =============================================================================

TEST(ProgsHeaderTest, IsValid) {
    ProgsHeader header;
    EXPECT_FALSE(header.isValid());
    
    header.version = vm_limits::PROG_VERSION;
    EXPECT_TRUE(header.isValid());
}

TEST(ProgsHeaderTest, EntitySize) {
    ProgsHeader header;
    header.entityFields = 105;
    
    EXPECT_EQ(header.entitySize(), 420);  // 105 * 4 bytes
}

// =============================================================================
// VM Header Tests
// =============================================================================

TEST(VMHeaderTest, IsValid) {
    VMHeader header;
    EXPECT_FALSE(header.isValid());
    
    header.magic = vm_limits::VM_MAGIC;
    EXPECT_TRUE(header.isValid());
    EXPECT_FALSE(header.isVersion2());
    
    header.magic = vm_limits::VM_MAGIC_VER2;
    EXPECT_TRUE(header.isValid());
    EXPECT_TRUE(header.isVersion2());
}

TEST(VMHeaderTest, TotalDataSize) {
    VMHeader header;
    header.dataLength = 1000;
    header.bssLength = 500;
    
    EXPECT_EQ(header.totalDataSize(), 1500);
}

TEST(VMHeaderTest, DataSwapLength) {
    VMHeader header;
    header.dataLength = 1000;
    header.litLength = 200;
    
    EXPECT_EQ(header.dataSwapLength(), 800);
}

// =============================================================================
// VM Instruction Tests
// =============================================================================

TEST(VMInstructionTest, Opcode) {
    VMInstruction inst;
    inst.op = static_cast<uint8_t>(Q3Opcode::Add);
    
    EXPECT_EQ(inst.opcode(), Q3Opcode::Add);
}

TEST(VMInstructionTest, Flags) {
    VMInstruction inst;
    EXPECT_FALSE(inst.isJumpTarget());
    EXPECT_FALSE(inst.isSwitch());
    
    inst.flags = 0x01;
    EXPECT_TRUE(inst.isJumpTarget());
    EXPECT_FALSE(inst.isSwitch());
    
    inst.flags = 0x03;
    EXPECT_TRUE(inst.isJumpTarget());
    EXPECT_TRUE(inst.isSwitch());
}

// =============================================================================
// VM Index Tests
// =============================================================================

TEST(VMIndexTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(VMIndex::Game), 0);
    EXPECT_EQ(static_cast<uint8_t>(VMIndex::CGame), 1);
    EXPECT_EQ(static_cast<uint8_t>(VMIndex::UI), 2);
}

TEST(VMIndexTest, Names) {
    EXPECT_STREQ(getVMIndexName(VMIndex::Game), "game");
    EXPECT_STREQ(getVMIndexName(VMIndex::CGame), "cgame");
    EXPECT_STREQ(getVMIndexName(VMIndex::UI), "ui");
}

// =============================================================================
// VM State Tests
// =============================================================================

TEST(VMStateTest, IsActive) {
    VMState state;
    EXPECT_FALSE(state.isActive());
    
    state.instructionCount = 100;
    EXPECT_TRUE(state.isActive());
}

// =============================================================================
// Edict Link Tests
// =============================================================================

TEST(EdictLinkTest, IsValid) {
    EdictLink link;
    EXPECT_FALSE(link.isValid());
    
    link.edictNum = 0;
    EXPECT_TRUE(link.isValid());
}

// =============================================================================
// Server Edict Tests
// =============================================================================

TEST(ServerEdictTest, FreeStatus) {
    ServerEdict edict;
    EXPECT_TRUE(edict.isFree());
    EXPECT_FALSE(edict.isActive());
    
    edict.free = false;
    EXPECT_FALSE(edict.isFree());
    EXPECT_TRUE(edict.isActive());
}

// =============================================================================
// Extended Entity Variables Tests
// =============================================================================

TEST(ExtEntVarsTest, IsOpaque) {
    ExtEntVars ext;
    EXPECT_TRUE(ext.isOpaque());  // Default alpha = 1.0
    
    ext.alpha = 0.5f;
    EXPECT_FALSE(ext.isOpaque());
    
    ext.alpha = 0.0f;
    EXPECT_TRUE(ext.isOpaque());
}

TEST(ExtEntVarsTest, HasColorMod) {
    ExtEntVars ext;
    EXPECT_FALSE(ext.hasColorMod());
    
    ext.colorMod[0] = 0.5f;
    EXPECT_TRUE(ext.hasColorMod());
}

// =============================================================================
// Builtin Info Tests
// =============================================================================

TEST(BuiltinInfoTest, IsVariadic) {
    BuiltinInfo info;
    info.minArgs = 2;
    info.maxArgs = 2;
    EXPECT_FALSE(info.isVariadic());
    
    info.maxArgs = 8;
    EXPECT_TRUE(info.isVariadic());
}

TEST(BuiltinInfoTest, AcceptsArgCount) {
    BuiltinInfo info;
    info.minArgs = 1;
    info.maxArgs = 3;
    
    EXPECT_FALSE(info.acceptsArgCount(0));
    EXPECT_TRUE(info.acceptsArgCount(1));
    EXPECT_TRUE(info.acceptsArgCount(2));
    EXPECT_TRUE(info.acceptsArgCount(3));
    EXPECT_FALSE(info.acceptsArgCount(4));
}

// =============================================================================
// Field Offsets Tests
// =============================================================================

TEST(FieldOffsetsTest, HasFields) {
    FieldOffsets offsets;
    EXPECT_FALSE(offsets.hasItems2());
    EXPECT_FALSE(offsets.hasVWep());
    EXPECT_FALSE(offsets.hasMovement());
    
    offsets.items2 = 100;
    offsets.vwIndex = 101;
    
    EXPECT_TRUE(offsets.hasItems2());
    EXPECT_TRUE(offsets.hasVWep());
    EXPECT_FALSE(offsets.hasMovement());
}

// =============================================================================
// VM Stats Tests
// =============================================================================

TEST(VMStatsTest, Reset) {
    VMStats stats;
    stats.instructionsExecuted = 1000;
    stats.functionCalls = 100;
    
    stats.reset();
    
    EXPECT_EQ(stats.instructionsExecuted, 0);
    EXPECT_EQ(stats.functionCalls, 0);
}

TEST(VMStatsTest, AddInstruction) {
    VMStats stats;
    stats.addInstruction();
    stats.addInstruction();
    stats.addInstruction();
    
    EXPECT_EQ(stats.instructionsExecuted, 3);
}

TEST(VMStatsTest, AddCall) {
    VMStats stats;
    stats.addCall(false);  // Not builtin
    stats.addCall(true);   // Builtin
    stats.addCall(true);   // Builtin
    
    EXPECT_EQ(stats.functionCalls, 3);
    EXPECT_EQ(stats.builtinCalls, 2);
}

TEST(VMStatsTest, UpdateStackDepth) {
    VMStats stats;
    stats.updateStackDepth(10);
    EXPECT_EQ(stats.stackMaxDepth, 10);
    
    stats.updateStackDepth(5);  // Smaller, should not update
    EXPECT_EQ(stats.stackMaxDepth, 10);
    
    stats.updateStackDepth(20);  // Larger, should update
    EXPECT_EQ(stats.stackMaxDepth, 20);
}
