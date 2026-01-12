/*
 * ezQuake C++ Port - Console System
 * 
 * The console provides text output, scrollback, and notification display.
 * 
 * Copyright (C) 2024 ezQuake Team
 */

#ifndef EZQUAKE_CONSOLE_HPP
#define EZQUAKE_CONSOLE_HPP

#include "core/types.hpp"
#include "core/text/utf8.hpp"
#include <string>
#include <vector>
#include <deque>
#include <functional>
#include <mutex>
#include <chrono>
#include <optional>

namespace ezquake {

//=============================================================================
// Forward Declarations
//=============================================================================

class Console;
class ConsoleBuffer;

//=============================================================================
// Console Color
//=============================================================================

/// Color information for console text
struct ConsoleColor {
    Byte r = 255;
    Byte g = 255;
    Byte b = 255;
    Byte a = 255;
    
    static constexpr ConsoleColor white() { return {255, 255, 255, 255}; }
    static constexpr ConsoleColor red() { return {255, 0, 0, 255}; }
    static constexpr ConsoleColor green() { return {0, 255, 0, 255}; }
    static constexpr ConsoleColor blue() { return {0, 0, 255, 255}; }
    static constexpr ConsoleColor yellow() { return {255, 255, 0, 255}; }
    static constexpr ConsoleColor cyan() { return {0, 255, 255, 255}; }
    static constexpr ConsoleColor magenta() { return {255, 0, 255, 255}; }
    
    /// Parse color from &cRGB format
    static std::optional<ConsoleColor> fromHex(char r, char g, char b);
    
    bool operator==(const ConsoleColor& other) const {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }
    bool operator!=(const ConsoleColor& other) const { return !(*this == other); }
};

//=============================================================================
// Console Character
//=============================================================================

/// A single character in the console buffer with color
struct ConsoleChar {
    Char32 codepoint = ' ';
    ConsoleColor color{};
    bool highBit = false;  // Quake high-bit coloring
    
    ConsoleChar() = default;
    ConsoleChar(Char32 cp, ConsoleColor col = ConsoleColor::white(), bool high = false)
        : codepoint(cp), color(col), highBit(high) {}
};

//=============================================================================
// Console Line
//=============================================================================

/// A single line in the console buffer
class ConsoleLine {
public:
    ConsoleLine() = default;
    explicit ConsoleLine(Size width);
    
    /// Clear the line
    void clear();
    
    /// Set character at position
    void setChar(Size pos, const ConsoleChar& ch);
    
    /// Get character at position
    [[nodiscard]] const ConsoleChar& charAt(Size pos) const;
    
    /// Get line width
    [[nodiscard]] Size width() const { return chars_.size(); }
    
    /// Get as string (without colors)
    [[nodiscard]] String toString() const;
    
    /// Timestamp when this line was printed
    void setTimestamp(Double time) { timestamp_ = time; }
    [[nodiscard]] Double timestamp() const { return timestamp_; }
    
    /// Iterator access
    auto begin() { return chars_.begin(); }
    auto end() { return chars_.end(); }
    [[nodiscard]] auto begin() const { return chars_.begin(); }
    [[nodiscard]] auto end() const { return chars_.end(); }
    
private:
    Vector<ConsoleChar> chars_;
    Double timestamp_ = 0.0;
};

//=============================================================================
// Print Flags
//=============================================================================

/// Flags for console print operations
enum class PrintFlags : UInt32 {
    None        = 0,
    NoNotify    = 1 << 0,   // Don't show in notify area
    Log         = 1 << 1,   // Force log to file
    NoLog       = 1 << 2,   // Don't log to file
    Skip        = 1 << 3,   // Don't print to console
    NoReset     = 1 << 4,   // Don't reset flags after print
};

inline PrintFlags operator|(PrintFlags a, PrintFlags b) {
    return static_cast<PrintFlags>(static_cast<UInt32>(a) | static_cast<UInt32>(b));
}

inline PrintFlags operator&(PrintFlags a, PrintFlags b) {
    return static_cast<PrintFlags>(static_cast<UInt32>(a) & static_cast<UInt32>(b));
}

inline bool hasFlag(PrintFlags flags, PrintFlags test) {
    return (static_cast<UInt32>(flags) & static_cast<UInt32>(test)) != 0;
}

//=============================================================================
// Console Buffer
//=============================================================================

/// Scrollback buffer for console text
class ConsoleBuffer {
public:
    explicit ConsoleBuffer(Size lineWidth = 80, Size maxLines = 1024);
    
    /// Print text to the buffer
    void print(StringView text, ConsoleColor color = ConsoleColor::white());
    
    /// Print wide character text
    void printWide(const Vector<Char32>& text, ConsoleColor color = ConsoleColor::white());
    
    /// Clear the buffer
    void clear();
    
    /// Get number of lines
    [[nodiscard]] Size lineCount() const { return lines_.size(); }
    
    /// Get maximum lines
    [[nodiscard]] Size maxLines() const { return maxLines_; }
    
    /// Get line width
    [[nodiscard]] Size lineWidth() const { return lineWidth_; }
    
    /// Get a line (0 = most recent)
    [[nodiscard]] const ConsoleLine* line(Size index) const;
    
    /// Get current x position
    [[nodiscard]] Size cursorX() const { return cursorX_; }
    
    /// Set current time (for notification timestamps)
    void setCurrentTime(Double time) { currentTime_ = time; }
    
    /// Get lines visible for notification (within time limit)
    [[nodiscard]] Vector<const ConsoleLine*> getNotifyLines(Double maxAge, Size maxCount) const;
    
    /// Resize the buffer
    void resize(Size newWidth, Size newMaxLines);
    
    /// Set the left margin
    void setMargin(Size margin) { margin_ = margin; }
    [[nodiscard]] Size margin() const { return margin_; }
    
private:
    void newLine();
    void ensureCurrentLine();
    
    std::deque<ConsoleLine> lines_;
    Size lineWidth_;
    Size maxLines_;
    Size cursorX_ = 0;
    Size margin_ = 0;
    Double currentTime_ = 0.0;
    bool carriageReturn_ = false;
};

//=============================================================================
// Console Callbacks
//=============================================================================

/// Callback for logging console output
using ConsoleLogCallback = std::function<void(StringView text)>;

/// Callback for print events
using ConsolePrintCallback = std::function<void(StringView text, PrintFlags flags)>;

//=============================================================================
// Console
//=============================================================================

/// Main console class - manages text output and display
/// 
/// NOTE: This class is NOT a singleton. Create instances and pass by reference.
/// Legacy global functions (Con_Print, etc.) use a default instance for compatibility.
class Console {
public:
    /// Default constructor - creates uninitialized console
    Console();
    
    /// Destructor
    ~Console();
    
    // Non-copyable
    Console(const Console&) = delete;
    Console& operator=(const Console&) = delete;
    
    // Movable
    Console(Console&&) noexcept = default;
    Console& operator=(Console&&) noexcept = default;
    
    //=========================================================================
    // Initialization
    //=========================================================================
    
    /// Initialize the console
    void init(Size bufferSize = 65536);
    
    /// Shutdown the console
    void shutdown();
    
    /// Check if initialized
    [[nodiscard]] bool isInitialized() const { return initialized_; }
    
    //=========================================================================
    // Printing
    //=========================================================================
    
    /// Print text to console
    void print(StringView text);
    
    /// Print formatted text
    template<typename... Args>
    void printf(const char* format, Args&&... args);
    
    /// Print with specific color
    void printColored(StringView text, ConsoleColor color);
    
    /// Print with flags
    void printWithFlags(StringView text, PrintFlags flags);
    
    /// Safe print (can be called when screen updates disabled)
    void safePrint(StringView text);
    
    /// Set print flags for next print
    void setPrintFlags(PrintFlags flags) { currentFlags_ = flags; }
    
    /// Set color mask (high-bit for Quake colored text)
    void setColorMask(Byte mask) { colorMask_ = mask; }
    [[nodiscard]] Byte colorMask() const { return colorMask_; }
    
    //=========================================================================
    // Display
    //=========================================================================
    
    /// Get the console buffer
    [[nodiscard]] const ConsoleBuffer& buffer() const { return buffer_; }
    [[nodiscard]] ConsoleBuffer& buffer() { return buffer_; }
    
    /// Clear the console
    void clear();
    
    /// Clear notify lines
    void clearNotify();
    
    /// Check resize (call when video mode changes)
    void checkResize(Size newWidth);
    
    /// Get line width
    [[nodiscard]] Size lineWidth() const { return buffer_.lineWidth(); }
    
    /// Get total lines
    [[nodiscard]] Size totalLines() const { return buffer_.lineCount(); }
    
    //=========================================================================
    // Scrolling
    //=========================================================================
    
    /// Get current display line (for scrollback)
    [[nodiscard]] Size displayLine() const { return displayLine_; }
    
    /// Set display line
    void setDisplayLine(Size line);
    
    /// Scroll up
    void scrollUp(Size lines = 1);
    
    /// Scroll down
    void scrollDown(Size lines = 1);
    
    /// Scroll to top
    void scrollToTop();
    
    /// Scroll to bottom
    void scrollToBottom();
    
    /// Page up
    void pageUp();
    
    /// Page down
    void pageDown();
    
    //=========================================================================
    // Visibility
    //=========================================================================
    
    /// Toggle console visibility
    void toggle();
    
    /// Check if console is visible
    [[nodiscard]] bool isVisible() const { return visible_; }
    
    /// Set visibility
    void setVisible(bool visible) { visible_ = visible; }
    
    /// Get visible lines (for rendering)
    [[nodiscard]] Size visibleLines() const { return visibleLines_; }
    
    /// Set visible lines
    void setVisibleLines(Size lines) { visibleLines_ = lines; }
    
    //=========================================================================
    // Notification
    //=========================================================================
    
    /// Get notify lines for transparent overlay
    [[nodiscard]] Vector<const ConsoleLine*> getNotifyLines(Size maxCount = 4) const;
    
    /// Set notify time (seconds a line stays visible)
    void setNotifyTime(Float seconds) { notifyTime_ = seconds; }
    [[nodiscard]] Float notifyTime() const { return notifyTime_; }
    
    /// Update current time
    void updateTime(Double time) { currentTime_ = time; buffer_.setCurrentTime(time); }
    
    //=========================================================================
    // Logging
    //=========================================================================
    
    /// Set log callback
    void setLogCallback(ConsoleLogCallback callback) { logCallback_ = std::move(callback); }
    
    /// Set print callback
    void setPrintCallback(ConsolePrintCallback callback) { printCallback_ = std::move(callback); }
    
    /// Enable/disable logging
    void setLoggingEnabled(bool enabled) { loggingEnabled_ = enabled; }
    [[nodiscard]] bool isLoggingEnabled() const { return loggingEnabled_; }
    
    /// Suppress console output temporarily
    void setSuppressed(bool suppressed) { suppressed_ = suppressed; }
    [[nodiscard]] bool isSuppressed() const { return suppressed_; }
    
    //=========================================================================
    // Timestamps
    //=========================================================================
    
    /// Enable timestamps on messages
    void setTimestampsEnabled(bool enabled) { timestampsEnabled_ = enabled; }
    [[nodiscard]] bool timestampsEnabled() const { return timestampsEnabled_; }
    
private:
    /// Parse and handle color codes in text
    void processText(StringView text, ConsoleColor baseColor);
    
    /// Add timestamp prefix if enabled
    String prependTimestamp(StringView text) const;
    
    mutable std::mutex mutex_;
    ConsoleBuffer buffer_;
    
    bool initialized_ = false;
    bool visible_ = false;
    bool suppressed_ = false;
    bool loggingEnabled_ = true;
    bool timestampsEnabled_ = false;
    
    Size displayLine_ = 0;
    Size visibleLines_ = 0;
    Float notifyTime_ = 3.0f;
    Double currentTime_ = 0.0;
    
    PrintFlags currentFlags_ = PrintFlags::None;
    Byte colorMask_ = 0;
    
    ConsoleLogCallback logCallback_;
    ConsolePrintCallback printCallback_;
};

//=============================================================================
// Template Implementations
//=============================================================================

template<typename... Args>
void Console::printf(const char* format, Args&&... args) {
    char buffer[4096];
    std::snprintf(buffer, sizeof(buffer), format, std::forward<Args>(args)...);
    print(buffer);
}

//=============================================================================
// Global Functions (for C compatibility)
//=============================================================================

/// Print to console
void Con_Print(const char* text);

/// Print formatted to console
void Con_Printf(const char* format, ...);

/// Safe print (screen update may be disabled)
void Con_SafePrintf(const char* format, ...);

/// Clear console
void Con_Clear();

/// Toggle console
void Con_Toggle();

/// Initialize console
void Con_Init();

/// Shutdown console
void Con_Shutdown();

/// Check for resize
void Con_CheckResize();

/// Clear notify area
void Con_ClearNotify();

} // namespace ezquake

#endif // EZQUAKE_CONSOLE_HPP
