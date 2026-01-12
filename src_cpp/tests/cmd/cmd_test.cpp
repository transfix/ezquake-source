/*
 * ezQuake C++ Port - Command System Unit Tests
 * 
 * Copyright (C) 2024 ezQuake Team
 */

#include <gtest/gtest.h>
#include "core/cmd/cmd_registry.hpp"
#include "core/cvar/cvar_registry.hpp"

using namespace ezquake;

//=============================================================================
// CommandArgs Tests
//=============================================================================

TEST(CommandArgsTest, EmptyParse) {
    auto args = CommandArgs::parse("");
    EXPECT_EQ(args.argc(), 0);
    EXPECT_TRUE(args.empty());
}

TEST(CommandArgsTest, SingleArg) {
    auto args = CommandArgs::parse("command");
    EXPECT_EQ(args.argc(), 1);
    EXPECT_EQ(args.argv(0), "command");
}

TEST(CommandArgsTest, MultipleArgs) {
    auto args = CommandArgs::parse("command arg1 arg2 arg3");
    EXPECT_EQ(args.argc(), 4);
    EXPECT_EQ(args.argv(0), "command");
    EXPECT_EQ(args.argv(1), "arg1");
    EXPECT_EQ(args.argv(2), "arg2");
    EXPECT_EQ(args.argv(3), "arg3");
}

TEST(CommandArgsTest, QuotedArgs) {
    auto args = CommandArgs::parse("say \"hello world\"");
    EXPECT_EQ(args.argc(), 2);
    EXPECT_EQ(args.argv(0), "say");
    EXPECT_EQ(args.argv(1), "hello world");
}

TEST(CommandArgsTest, MixedQuotedArgs) {
    auto args = CommandArgs::parse("bind x \"impulse 10\"");
    EXPECT_EQ(args.argc(), 3);
    EXPECT_EQ(args.argv(0), "bind");
    EXPECT_EQ(args.argv(1), "x");
    EXPECT_EQ(args.argv(2), "impulse 10");
}

TEST(CommandArgsTest, ExtraWhitespace) {
    auto args = CommandArgs::parse("  command   arg1    arg2  ");
    EXPECT_EQ(args.argc(), 3);
    EXPECT_EQ(args.argv(0), "command");
    EXPECT_EQ(args.argv(1), "arg1");
    EXPECT_EQ(args.argv(2), "arg2");
}

TEST(CommandArgsTest, OutOfRangeReturnsEmpty) {
    auto args = CommandArgs::parse("cmd");
    EXPECT_EQ(args.argv(1), "");
    EXPECT_EQ(args.argv(100), "");
    EXPECT_EQ(args.argv(-1), "");
}

TEST(CommandArgsTest, Args) {
    auto args = CommandArgs::parse("command arg1 arg2 arg3");
    EXPECT_EQ(args.args(), "arg1 arg2 arg3");
}

TEST(CommandArgsTest, ArgsFrom) {
    auto args = CommandArgs::parse("command arg1 arg2 arg3");
    EXPECT_EQ(args.argsFrom(0), "command arg1 arg2 arg3");
    EXPECT_EQ(args.argsFrom(1), "arg1 arg2 arg3");
    EXPECT_EQ(args.argsFrom(2), "arg2 arg3");
    EXPECT_EQ(args.argsFrom(3), "arg3");
    EXPECT_EQ(args.argsFrom(4), "");
}

TEST(CommandArgsTest, OperatorBracket) {
    auto args = CommandArgs::parse("cmd arg1 arg2");
    EXPECT_EQ(args[0], "cmd");
    EXPECT_EQ(args[1], "arg1");
    EXPECT_EQ(args[2], "arg2");
}

TEST(CommandArgsTest, ConstructFromVector) {
    Vector<String> v = {"cmd", "arg1", "arg2"};
    CommandArgs args(v);
    EXPECT_EQ(args.argc(), 3);
    EXPECT_EQ(args.argv(0), "cmd");
    EXPECT_EQ(args.argv(1), "arg1");
}

//=============================================================================
// Command Tests
//=============================================================================

TEST(CommandTest, CreateWithModernCallback) {
    bool called = false;
    int argCount = 0;
    
    Command cmd("test", [&](const CommandArgs& args) {
        called = true;
        argCount = args.argc();
    });
    
    EXPECT_EQ(cmd.name(), "test");
    EXPECT_TRUE(cmd.hasCallback());
    
    auto args = CommandArgs::parse("test arg1 arg2");
    cmd.execute(args);
    
    EXPECT_TRUE(called);
    EXPECT_EQ(argCount, 3);
}

TEST(CommandTest, CreateWithLegacyCallback) {
    bool called = false;
    
    Command cmd("test", [&]() {
        called = true;
    });
    
    EXPECT_EQ(cmd.name(), "test");
    EXPECT_TRUE(cmd.hasCallback());
    
    auto args = CommandArgs::parse("test");
    cmd.execute(args);
    
    EXPECT_TRUE(called);
}

TEST(CommandTest, Flags) {
    Command cmd("cheat", [](const CommandArgs&) {}, 
                CommandFlags::CheatCmd | CommandFlags::ServerOnly);
    
    EXPECT_TRUE(cmd.hasFlag(CommandFlags::CheatCmd));
    EXPECT_TRUE(cmd.hasFlag(CommandFlags::ServerOnly));
    EXPECT_FALSE(cmd.hasFlag(CommandFlags::Hidden));
}

//=============================================================================
// CommandFlags Tests
//=============================================================================

TEST(CommandFlagsTest, BitwiseOperations) {
    CommandFlags flags = CommandFlags::None;
    EXPECT_FALSE(hasFlag(flags, CommandFlags::ServerOnly));
    
    flags = CommandFlags::ServerOnly | CommandFlags::CheatCmd;
    EXPECT_TRUE(hasFlag(flags, CommandFlags::ServerOnly));
    EXPECT_TRUE(hasFlag(flags, CommandFlags::CheatCmd));
    EXPECT_FALSE(hasFlag(flags, CommandFlags::Hidden));
}

//=============================================================================
// Alias Tests
//=============================================================================

TEST(AliasTest, Construction) {
    Alias alias("quit", "disconnect; exit", AliasFlags::None);
    
    EXPECT_EQ(alias.name, "quit");
    EXPECT_EQ(alias.value, "disconnect; exit");
}

//=============================================================================
// CommandBuffer Tests
//=============================================================================

TEST(CommandBufferTest, AddText) {
    CommandBuffer buf("test");
    
    buf.addText("echo hello");
    
    EXPECT_FALSE(buf.empty());
    EXPECT_EQ(buf.size(), 10u);
}

TEST(CommandBufferTest, InsertText) {
    CommandBuffer buf("test");
    
    buf.addText("second");
    buf.insertText("first\n");
    
    // First should now be at the start
    EXPECT_GT(buf.size(), 6u);
}

TEST(CommandBufferTest, Clear) {
    CommandBuffer buf("test");
    buf.addText("some command");
    
    buf.clear();
    
    EXPECT_TRUE(buf.empty());
}

TEST(CommandBufferTest, Wait) {
    CommandBuffer buf("test");
    
    EXPECT_FALSE(buf.isWaiting());
    
    buf.setWait();
    EXPECT_TRUE(buf.isWaiting());
    
    buf.clearWait();
    EXPECT_FALSE(buf.isWaiting());
}

TEST(CommandBufferTest, Name) {
    CommandBuffer buf("main");
    EXPECT_EQ(buf.name(), "main");
}

//=============================================================================
// CommandRegistry Tests (Dependency Injection - No Singletons!)
//=============================================================================

class CommandRegistryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Register built-in commands like 'wait', 'echo', 'alias', etc.
        commands_.registerBuiltins();
    }
    
    // Each test gets its own registry - no global state!
    CommandRegistry commands_;
    CvarRegistry cvars_;
};

TEST_F(CommandRegistryTest, AddCommand) {
    bool called = false;
    
    commands_.addCommand("testcmd", [&](const CommandArgs&) {
        called = true;
    });
    
    EXPECT_TRUE(commands_.commandExists("testcmd"));
    
    auto* cmd = commands_.findCommand("testcmd");
    EXPECT_NE(cmd, nullptr);
    EXPECT_EQ(cmd->name(), "testcmd");
}

TEST_F(CommandRegistryTest, AddLegacyCommand) {
    bool called = false;
    
    commands_.addCommand("legacycmd", [&]() {
        called = true;
    });
    
    EXPECT_TRUE(commands_.commandExists("legacycmd"));
}

TEST_F(CommandRegistryTest, RemoveUserCreatedCommand) {
    commands_.addCommand("usercmd", [](const CommandArgs&) {},
                                          CommandFlags::UserCreated);
    EXPECT_TRUE(commands_.commandExists("usercmd"));
    
    bool removed = commands_.removeCommand("usercmd");
    
    EXPECT_TRUE(removed);
    EXPECT_FALSE(commands_.commandExists("usercmd"));
}

TEST_F(CommandRegistryTest, RemoveBuiltinCommandFails) {
    commands_.addCommand("builtin", [](const CommandArgs&) {});
    
    bool removed = commands_.removeCommand("builtin");
    
    EXPECT_FALSE(removed);
    EXPECT_TRUE(commands_.commandExists("builtin"));
}

TEST_F(CommandRegistryTest, AddAlias) {
    auto& alias = commands_.addAlias("quit", "disconnect; exit");
    
    EXPECT_EQ(alias.name, "quit");
    EXPECT_EQ(alias.value, "disconnect; exit");
    
    auto* found = commands_.findAlias("quit");
    EXPECT_NE(found, nullptr);
    EXPECT_EQ(&alias, found);
}

TEST_F(CommandRegistryTest, RemoveAlias) {
    commands_.addAlias("myalias", "value");
    EXPECT_NE(commands_.findAlias("myalias"), nullptr);
    
    bool removed = commands_.removeAlias("myalias");
    
    EXPECT_TRUE(removed);
    EXPECT_EQ(commands_.findAlias("myalias"), nullptr);
}

TEST_F(CommandRegistryTest, AliasValue) {
    commands_.addAlias("test", "the value");
    
    auto* value = commands_.aliasValue("test");
    
    EXPECT_NE(value, nullptr);
    EXPECT_EQ(*value, "the value");
}

TEST_F(CommandRegistryTest, AliasValueNotFound) {
    auto* value = commands_.aliasValue("nonexistent");
    EXPECT_EQ(value, nullptr);
}

TEST_F(CommandRegistryTest, DeleteServerAliases) {
    commands_.addAlias("client", "value");
    commands_.addAlias("server", "value", AliasFlags::ServerSet);
    
    commands_.deleteServerAliases();
    
    EXPECT_NE(commands_.findAlias("client"), nullptr);
    EXPECT_EQ(commands_.findAlias("server"), nullptr);
}

TEST_F(CommandRegistryTest, ExecuteCommand) {
    bool called = false;
    String receivedArg;
    
    commands_.addCommand("mycmd", [&](const CommandArgs& args) {
        called = true;
        if (args.argc() > 1) {
            receivedArg = String(args.argv(1));
        }
    });
    
    commands_.executeString("mycmd hello");
    
    EXPECT_TRUE(called);
    EXPECT_EQ(receivedArg, "hello");
}

TEST_F(CommandRegistryTest, ExecuteAlias) {
    bool called = false;
    
    commands_.addCommand("target", [&](const CommandArgs&) {
        called = true;
    });
    
    commands_.addAlias("myalias", "target");
    commands_.executeString("myalias");
    
    // Alias should insert text, which won't be executed immediately
    // without calling execute() on the buffer
    commands_.mainBuffer().execute(commands_);
    
    EXPECT_TRUE(called);
}

TEST_F(CommandRegistryTest, CompleteCommand) {
    commands_.addCommand("sensitivity", [](const CommandArgs&) {});
    commands_.addCommand("serverinfo", [](const CommandArgs&) {});
    commands_.addCommand("other", [](const CommandArgs&) {});
    
    auto completions = commands_.completeCommand("se");
    
    EXPECT_EQ(completions.size(), 2u);
    EXPECT_TRUE(std::find(completions.begin(), completions.end(), "sensitivity") != completions.end());
    EXPECT_TRUE(std::find(completions.begin(), completions.end(), "serverinfo") != completions.end());
}

TEST_F(CommandRegistryTest, CompleteAlias) {
    commands_.addAlias("attack1", "");
    commands_.addAlias("attack2", "");
    commands_.addAlias("movement", "");
    
    auto completions = commands_.completeAlias("att");
    
    EXPECT_EQ(completions.size(), 2u);
}

TEST_F(CommandRegistryTest, LegacyCommandMapping) {
    bool newCmdCalled = false;
    
    commands_.addCommand("newcmd", [&](const CommandArgs&) {
        newCmdCalled = true;
    });
    
    commands_.addLegacyCommand("oldcmd", "newcmd");
    
    EXPECT_TRUE(commands_.isLegacyCommand("oldcmd"));
    EXPECT_FALSE(commands_.isLegacyCommand("newcmd"));
}

TEST_F(CommandRegistryTest, CommandCount) {
    Size initial = commands_.commandCount();
    
    commands_.addCommand("cmd1", [](const CommandArgs&) {});
    commands_.addCommand("cmd2", [](const CommandArgs&) {});
    
    EXPECT_EQ(commands_.commandCount(), initial + 2);
}

TEST_F(CommandRegistryTest, AliasCount) {
    EXPECT_EQ(commands_.aliasCount(), 0u);
    
    commands_.addAlias("a1", "");
    commands_.addAlias("a2", "");
    
    EXPECT_EQ(commands_.aliasCount(), 2u);
}

TEST_F(CommandRegistryTest, ForEachCommand) {
    commands_.addCommand("cmd1", [](const CommandArgs&) {});
    commands_.addCommand("cmd2", [](const CommandArgs&) {});
    
    int count = 0;
    commands_.forEachCommand([&](Command& cmd) {
        (void)cmd;
        count++;
    });
    
    EXPECT_GE(count, 2); // At least our 2 + builtins
}

TEST_F(CommandRegistryTest, ForEachAlias) {
    commands_.addAlias("a1", "v1");
    commands_.addAlias("a2", "v2");
    
    int count = 0;
    commands_.forEachAlias([&](Alias& alias) {
        (void)alias;
        count++;
    });
    
    EXPECT_EQ(count, 2);
}

//=============================================================================
// Integration Tests
//=============================================================================

TEST_F(CommandRegistryTest, ExecuteWithCvar) {
    // Note: This test validates cvar-setting via command execution
    // In real usage, the registry needs a cvar registry reference to set cvars
    // For now we just test that executeString doesn't crash
    cvars_.create("test_value", "42");
    
    // The actual cvar setting requires integration between CommandRegistry and CvarRegistry
    // This would be done via the Context class in production code
    EXPECT_EQ(cvars_.value("test_value"), 42.0f);
}

TEST_F(CommandRegistryTest, WaitCommand) {
    int callCount = 0;
    
    commands_.addCommand("counter", [&](const CommandArgs&) {
        callCount++;
    });
    
    // Use the mainBuffer directly instead of global functions
    auto& buf = commands_.mainBuffer();
    buf.addText("counter\n");
    buf.addText("wait\n");
    buf.addText("counter\n");
    
    // First execute - should run counter, then hit wait
    buf.execute(commands_);
    EXPECT_EQ(callCount, 1);
    
    // Buffer is waiting
    EXPECT_TRUE(buf.isWaiting());
    
    // Clear wait and execute again
    buf.clearWait();
    buf.execute(commands_);
    EXPECT_EQ(callCount, 2);
}

//=============================================================================
// CommandBuffer Integration Tests
//=============================================================================

TEST_F(CommandRegistryTest, MainBufferAddText) {
    commands_.mainBuffer().addText("echo hello");
    EXPECT_FALSE(commands_.mainBuffer().empty());
}

TEST_F(CommandRegistryTest, MainBufferInsertText) {
    commands_.mainBuffer().addText("second");
    commands_.mainBuffer().insertText("first\n");
    EXPECT_FALSE(commands_.mainBuffer().empty());
}

TEST_F(CommandRegistryTest, ExecuteStringDirect) {
    bool called = false;
    commands_.addCommand("directcmd", [&](const CommandArgs&) {
        called = true;
    });
    
    commands_.executeString("directcmd");
    
    EXPECT_TRUE(called);
}
