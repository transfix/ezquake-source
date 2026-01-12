/*
 * ezQuake C++ Port - Console Unit Tests
 * 
 * Copyright (C) 2024 ezQuake Team
 */

#include <gtest/gtest.h>
#include "core/console/console.hpp"

using namespace ezquake;

//=============================================================================
// ConsoleColor Tests
//=============================================================================

TEST(ConsoleColorTest, DefaultConstruction) {
    ConsoleColor color;
    EXPECT_EQ(color.r, 255);
    EXPECT_EQ(color.g, 255);
    EXPECT_EQ(color.b, 255);
    EXPECT_EQ(color.a, 255);
}

TEST(ConsoleColorTest, StaticColors) {
    auto white = ConsoleColor::white();
    EXPECT_EQ(white.r, 255);
    EXPECT_EQ(white.g, 255);
    EXPECT_EQ(white.b, 255);
    
    auto red = ConsoleColor::red();
    EXPECT_EQ(red.r, 255);
    EXPECT_EQ(red.g, 0);
    EXPECT_EQ(red.b, 0);
    
    auto green = ConsoleColor::green();
    EXPECT_EQ(green.r, 0);
    EXPECT_EQ(green.g, 255);
    EXPECT_EQ(green.b, 0);
    
    auto blue = ConsoleColor::blue();
    EXPECT_EQ(blue.r, 0);
    EXPECT_EQ(blue.g, 0);
    EXPECT_EQ(blue.b, 255);
}

TEST(ConsoleColorTest, FromHex) {
    auto color = ConsoleColor::fromHex('f', 'f', 'f');
    ASSERT_TRUE(color.has_value());
    EXPECT_EQ(color->r, 255);
    EXPECT_EQ(color->g, 255);
    EXPECT_EQ(color->b, 255);
    
    color = ConsoleColor::fromHex('0', '0', '0');
    ASSERT_TRUE(color.has_value());
    EXPECT_EQ(color->r, 0);
    EXPECT_EQ(color->g, 0);
    EXPECT_EQ(color->b, 0);
    
    color = ConsoleColor::fromHex('f', '8', '0');
    ASSERT_TRUE(color.has_value());
    EXPECT_EQ(color->r, 255);
    EXPECT_EQ(color->g, 136);  // 8 * 17 = 136
    EXPECT_EQ(color->b, 0);
}

TEST(ConsoleColorTest, FromHexInvalid) {
    auto color = ConsoleColor::fromHex('x', 'y', 'z');
    EXPECT_FALSE(color.has_value());
    
    color = ConsoleColor::fromHex('g', '0', '0');
    EXPECT_FALSE(color.has_value());
}

TEST(ConsoleColorTest, Equality) {
    ConsoleColor a{100, 150, 200, 255};
    ConsoleColor b{100, 150, 200, 255};
    ConsoleColor c{100, 150, 201, 255};
    
    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

//=============================================================================
// ConsoleChar Tests
//=============================================================================

TEST(ConsoleCharTest, DefaultConstruction) {
    ConsoleChar ch;
    EXPECT_EQ(ch.codepoint, ' ');
    EXPECT_EQ(ch.color, ConsoleColor::white());
    EXPECT_FALSE(ch.highBit);
}

TEST(ConsoleCharTest, Construction) {
    ConsoleChar ch('A', ConsoleColor::red(), true);
    EXPECT_EQ(ch.codepoint, 'A');
    EXPECT_EQ(ch.color, ConsoleColor::red());
    EXPECT_TRUE(ch.highBit);
}

//=============================================================================
// ConsoleLine Tests
//=============================================================================

TEST(ConsoleLineTest, DefaultConstruction) {
    ConsoleLine line(80);
    EXPECT_EQ(line.width(), 80u);
}

TEST(ConsoleLineTest, Clear) {
    ConsoleLine line(10);
    line.setChar(5, ConsoleChar('X'));
    line.clear();
    EXPECT_EQ(line.charAt(5).codepoint, ' ');
}

TEST(ConsoleLineTest, SetGetChar) {
    ConsoleLine line(20);
    line.setChar(10, ConsoleChar('Z', ConsoleColor::green()));
    
    EXPECT_EQ(line.charAt(10).codepoint, 'Z');
    EXPECT_EQ(line.charAt(10).color, ConsoleColor::green());
    
    // Out of bounds returns space
    EXPECT_EQ(line.charAt(100).codepoint, ' ');
}

TEST(ConsoleLineTest, ToString) {
    ConsoleLine line(10);
    line.setChar(0, ConsoleChar('H'));
    line.setChar(1, ConsoleChar('e'));
    line.setChar(2, ConsoleChar('l'));
    line.setChar(3, ConsoleChar('l'));
    line.setChar(4, ConsoleChar('o'));
    
    EXPECT_EQ(line.toString(), "Hello");
}

TEST(ConsoleLineTest, Timestamp) {
    ConsoleLine line(10);
    EXPECT_EQ(line.timestamp(), 0.0);
    
    line.setTimestamp(123.456);
    EXPECT_EQ(line.timestamp(), 123.456);
}

//=============================================================================
// ConsoleBuffer Tests
//=============================================================================

TEST(ConsoleBufferTest, Construction) {
    ConsoleBuffer buffer(80, 100);
    EXPECT_EQ(buffer.lineWidth(), 80u);
    EXPECT_EQ(buffer.maxLines(), 100u);
    EXPECT_EQ(buffer.lineCount(), 0u);
}

TEST(ConsoleBufferTest, PrintSimple) {
    ConsoleBuffer buffer(80, 100);
    buffer.print("Hello World");
    
    EXPECT_GE(buffer.lineCount(), 1u);
    
    auto* line = buffer.line(0);
    ASSERT_NE(line, nullptr);
    EXPECT_EQ(line->toString(), "Hello World");
}

TEST(ConsoleBufferTest, PrintNewline) {
    ConsoleBuffer buffer(80, 100);
    buffer.print("Line 1\nLine 2\nLine 3\n");
    
    // Lines are stored newest first
    EXPECT_GE(buffer.lineCount(), 3u);
}

TEST(ConsoleBufferTest, PrintMultiple) {
    ConsoleBuffer buffer(80, 100);
    buffer.print("Hello ");
    buffer.print("World");
    
    auto* line = buffer.line(0);
    ASSERT_NE(line, nullptr);
    EXPECT_EQ(line->toString(), "Hello World");
}

TEST(ConsoleBufferTest, WordWrap) {
    ConsoleBuffer buffer(10, 100);
    buffer.print("This is a very long line that should wrap");
    
    EXPECT_GT(buffer.lineCount(), 1u);
}

TEST(ConsoleBufferTest, Clear) {
    ConsoleBuffer buffer(80, 100);
    buffer.print("Some text\n");
    buffer.print("More text\n");
    
    buffer.clear();
    
    EXPECT_EQ(buffer.lineCount(), 0u);
}

TEST(ConsoleBufferTest, MaxLines) {
    ConsoleBuffer buffer(80, 5);
    
    for (int i = 0; i < 10; i++) {
        buffer.print("Line\n");
    }
    
    EXPECT_LE(buffer.lineCount(), 5u);
}

TEST(ConsoleBufferTest, CarriageReturn) {
    ConsoleBuffer buffer(80, 100);
    buffer.print("AAAA\rBBBB");
    
    auto* line = buffer.line(0);
    ASSERT_NE(line, nullptr);
    // Carriage return overwrites, so should have BBBB
    EXPECT_EQ(line->toString(), "BBBB");
}

TEST(ConsoleBufferTest, Margin) {
    ConsoleBuffer buffer(80, 100);
    buffer.setMargin(4);
    buffer.print("Test\n");
    buffer.print("Another");
    
    EXPECT_EQ(buffer.margin(), 4u);
}

TEST(ConsoleBufferTest, Resize) {
    ConsoleBuffer buffer(80, 100);
    buffer.print("Some text");
    
    buffer.resize(40, 50);
    
    EXPECT_EQ(buffer.lineWidth(), 40u);
    EXPECT_EQ(buffer.maxLines(), 50u);
}

TEST(ConsoleBufferTest, GetNotifyLines) {
    ConsoleBuffer buffer(80, 100);
    buffer.setCurrentTime(10.0);
    buffer.print("Line 1\n");
    buffer.setCurrentTime(11.0);
    buffer.print("Line 2\n");
    buffer.setCurrentTime(12.0);
    buffer.print("Line 3\n");
    
    auto notify = buffer.getNotifyLines(5.0, 10);
    EXPECT_GE(notify.size(), 2u);  // Lines within 5 seconds
}

TEST(ConsoleBufferTest, ColorCodes) {
    ConsoleBuffer buffer(80, 100);
    buffer.print("Normal &cf00Red &r Back to normal");
    
    auto* line = buffer.line(0);
    ASSERT_NE(line, nullptr);
    // Text should be printed without color codes
    String str = line->toString();
    EXPECT_TRUE(str.find("Normal") != String::npos);
    EXPECT_TRUE(str.find("Red") != String::npos);
    // Color codes should be stripped
    EXPECT_TRUE(str.find("&cf00") == String::npos);
}

//=============================================================================
// PrintFlags Tests
//=============================================================================

TEST(PrintFlagsTest, BitwiseOperations) {
    PrintFlags flags = PrintFlags::None;
    EXPECT_FALSE(hasFlag(flags, PrintFlags::NoNotify));
    
    flags = PrintFlags::NoNotify | PrintFlags::NoLog;
    EXPECT_TRUE(hasFlag(flags, PrintFlags::NoNotify));
    EXPECT_TRUE(hasFlag(flags, PrintFlags::NoLog));
    EXPECT_FALSE(hasFlag(flags, PrintFlags::Skip));
}

//=============================================================================
// Console Tests
//=============================================================================

class ConsoleTest : public ::testing::Test {
protected:
    Console console_;
    
    void SetUp() override {
        console_.init(32768);
    }
    
    void TearDown() override {
        console_.shutdown();
    }
};

TEST_F(ConsoleTest, Initialization) {
    EXPECT_TRUE(console_.isInitialized());
}

TEST_F(ConsoleTest, Print) {
    console_.print("Test message\n");
    
    EXPECT_GE(console_.buffer().lineCount(), 1u);
}

TEST_F(ConsoleTest, Printf) {
    console_.printf("Value: %d\n", 42);
    
    // Find the line containing "42" (may not be line 0 due to init message)
    bool found = false;
    for (Size i = 0; i < console_.buffer().lineCount(); ++i) {
        auto* line = console_.buffer().line(i);
        if (line && line->toString().find("42") != String::npos) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

TEST_F(ConsoleTest, Clear) {
    console_.print("Some text\n");
    console_.clear();
    
    EXPECT_EQ(console_.buffer().lineCount(), 0u);
}

TEST_F(ConsoleTest, Visibility) {
    EXPECT_FALSE(console_.isVisible());
    
    console_.toggle();
    EXPECT_TRUE(console_.isVisible());
    
    console_.toggle();
    EXPECT_FALSE(console_.isVisible());
    
    console_.setVisible(true);
    EXPECT_TRUE(console_.isVisible());
}

TEST_F(ConsoleTest, Scrolling) {
    // Add many lines
    for (int i = 0; i < 100; i++) {
        console_.printf("Line %d\n", i);
    }
    
    console_.scrollToBottom();
    EXPECT_EQ(console_.displayLine(), 0u);
    
    console_.scrollUp(5);
    EXPECT_EQ(console_.displayLine(), 5u);
    
    console_.scrollDown(3);
    EXPECT_EQ(console_.displayLine(), 2u);
    
    console_.scrollToTop();
    EXPECT_GT(console_.displayLine(), 0u);
}

TEST_F(ConsoleTest, Suppression) {
    console_.setSuppressed(true);
    Size beforeCount = console_.buffer().lineCount();
    
    console_.print("Suppressed message\n");
    
    // Line count shouldn't change when suppressed
    EXPECT_EQ(console_.buffer().lineCount(), beforeCount);
    
    console_.setSuppressed(false);
}

TEST_F(ConsoleTest, NotifyTime) {
    console_.setNotifyTime(5.0f);
    EXPECT_EQ(console_.notifyTime(), 5.0f);
}

TEST_F(ConsoleTest, Timestamps) {
    console_.setTimestampsEnabled(true);
    EXPECT_TRUE(console_.timestampsEnabled());
    
    console_.print("Message with timestamp\n");
    
    // Find the line with timestamps (line 0 is most recent)
    bool found = false;
    for (Size i = 0; i < console_.buffer().lineCount(); ++i) {
        auto* line = console_.buffer().line(i);
        if (line) {
            String str = line->toString();
            // Should have timestamp format [HH:MM:SS]
            if (str.find('[') != String::npos && str.find(']') != String::npos) {
                found = true;
                break;
            }
        }
    }
    EXPECT_TRUE(found);
    
    console_.setTimestampsEnabled(false);
}

TEST_F(ConsoleTest, LogCallback) {
    String logged;
    console_.setLogCallback([&logged](StringView text) {
        logged = String(text);
    });
    
    console_.print("Logged message");
    
    EXPECT_EQ(logged, "Logged message");
}

TEST_F(ConsoleTest, PrintCallback) {
    String printed;
    PrintFlags receivedFlags = PrintFlags::None;
    
    console_.setPrintCallback([&](StringView text, PrintFlags flags) {
        printed = String(text);
        receivedFlags = flags;
    });
    
    console_.printWithFlags("Test", PrintFlags::NoNotify);
    
    EXPECT_EQ(printed, "Test");
    EXPECT_TRUE(hasFlag(receivedFlags, PrintFlags::NoNotify));
}

TEST_F(ConsoleTest, ColoredPrint) {
    console_.printColored("Red text", ConsoleColor::red());
    
    auto* line = console_.buffer().line(0);
    ASSERT_NE(line, nullptr);
    EXPECT_EQ(line->charAt(0).color, ConsoleColor::red());
}

TEST_F(ConsoleTest, CheckResize) {
    console_.checkResize(40);
    EXPECT_EQ(console_.lineWidth(), 40u);
}

//=============================================================================
// Global Function Tests
// Note: These test the legacy global API which uses an internal default console
//=============================================================================

TEST(GlobalConsoleFunctionTest, ConPrint) {
    // Initialize default console through global API
    Con_Init();
    Con_Clear();  // Start fresh
    
    Con_Print("Test print");
    
    // Use Con_Clear to verify the console is working - we can't access
    // the buffer directly without the singleton pattern
    // For a complete test, we'd need to add a getter for the default console
    
    Con_Shutdown();
}

TEST(GlobalConsoleFunctionTest, ConPrintf) {
    Con_Init();
    Con_Clear();
    Con_Printf("Number: %d, String: %s", 123, "hello");
    Con_Shutdown();
}

TEST(GlobalConsoleFunctionTest, ConClearAndToggle) {
    Con_Init();
    Con_Print("Some text\n");
    Con_Clear();
    Con_Toggle();
    Con_Shutdown();
}
