/*
 * ezQuake C++ Port - Console System Implementation
 * 
 * Copyright (C) 2024 ezQuake Team
 */

#include "core/console/console.hpp"
#include <cstdarg>
#include <cstring>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace ezquake {

//=============================================================================
// ConsoleColor Implementation
//=============================================================================

std::optional<ConsoleColor> ConsoleColor::fromHex(char r, char g, char b) {
    auto hexToInt = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };
    
    int rv = hexToInt(r);
    int gv = hexToInt(g);
    int bv = hexToInt(b);
    
    if (rv < 0 || gv < 0 || bv < 0) {
        return std::nullopt;
    }
    
    return ConsoleColor{
        static_cast<Byte>(rv * 17),  // 0-15 -> 0-255
        static_cast<Byte>(gv * 17),
        static_cast<Byte>(bv * 17),
        255
    };
}

//=============================================================================
// ConsoleLine Implementation
//=============================================================================

ConsoleLine::ConsoleLine(Size width) : chars_(width) {
    clear();
}

void ConsoleLine::clear() {
    for (auto& ch : chars_) {
        ch = ConsoleChar(' ', ConsoleColor::white());
    }
}

void ConsoleLine::setChar(Size pos, const ConsoleChar& ch) {
    if (pos < chars_.size()) {
        chars_[pos] = ch;
    }
}

const ConsoleChar& ConsoleLine::charAt(Size pos) const {
    static const ConsoleChar empty(' ');
    if (pos < chars_.size()) {
        return chars_[pos];
    }
    return empty;
}

String ConsoleLine::toString() const {
    String result;
    result.reserve(chars_.size());
    
    for (const auto& ch : chars_) {
        // Convert to UTF-8
        if (ch.codepoint < 128) {
            result += static_cast<char>(ch.codepoint);
        } else {
            char buf[5];
            Size len = text::utf8::encode(ch.codepoint, buf);
            result.append(buf, len);
        }
    }
    
    // Trim trailing spaces
    auto end = result.find_last_not_of(' ');
    if (end != String::npos) {
        result.resize(end + 1);
    } else {
        result.clear();
    }
    
    return result;
}

//=============================================================================
// ConsoleBuffer Implementation
//=============================================================================

ConsoleBuffer::ConsoleBuffer(Size lineWidth, Size maxLines)
    : lineWidth_(lineWidth), maxLines_(maxLines) {
}

void ConsoleBuffer::ensureCurrentLine() {
    if (lines_.empty()) {
        lines_.emplace_back(lineWidth_);
        lines_.back().setTimestamp(currentTime_);
    }
}

void ConsoleBuffer::newLine() {
    // Mark timestamp on current line
    if (!lines_.empty()) {
        lines_.front().setTimestamp(currentTime_);
    }
    
    // Add new line at front
    lines_.emplace_front(lineWidth_);
    lines_.front().setTimestamp(currentTime_);
    
    // Trim excess lines
    while (lines_.size() > maxLines_) {
        lines_.pop_back();
    }
    
    cursorX_ = margin_;
    carriageReturn_ = false;
}

void ConsoleBuffer::print(StringView text, ConsoleColor color) {
    // Convert UTF-8 to codepoints
    Vector<Char32> codepoints;
    codepoints.reserve(text.size());
    
    for (auto cp : text::utf8::codePoints(text)) {
        codepoints.push_back(cp);
    }
    
    printWide(codepoints, color);
}

void ConsoleBuffer::printWide(const Vector<Char32>& text, ConsoleColor color) {
    ensureCurrentLine();
    
    ConsoleColor currentColor = color;
    
    for (Size i = 0; i < text.size(); ++i) {
        Char32 c = text[i];
        
        // Check for color codes: &cRGB
        if (c == '&' && i + 4 < text.size()) {
            if (text[i + 1] == 'c') {
                char r = static_cast<char>(text[i + 2]);
                char g = static_cast<char>(text[i + 3]);
                char b = static_cast<char>(text[i + 4]);
                
                if (auto col = ConsoleColor::fromHex(r, g, b)) {
                    currentColor = *col;
                    i += 4;  // Skip the color code
                    continue;
                }
            } else if (text[i + 1] == 'r') {
                currentColor = color;  // Reset to base color
                i += 1;
                continue;
            }
        }
        
        // Handle special characters
        switch (c) {
            case '\n':
                newLine();
                break;
                
            case '\r':
                cursorX_ = margin_;
                carriageReturn_ = true;
                break;
                
            case '\t':
                // Tab to next 8-char boundary
                cursorX_ = ((cursorX_ + 8) / 8) * 8;
                if (cursorX_ >= lineWidth_) {
                    newLine();
                }
                break;
                
            default:
                // Handle carriage return (overwrite current line)
                if (carriageReturn_) {
                    carriageReturn_ = false;
                    // Don't create new line, just reset position
                }
                
                // Word wrap
                if (cursorX_ >= lineWidth_) {
                    newLine();
                }
                
                // Print character
                if (!lines_.empty()) {
                    lines_.front().setChar(cursorX_, ConsoleChar(c, currentColor));
                }
                cursorX_++;
                break;
        }
    }
}

void ConsoleBuffer::clear() {
    lines_.clear();
    cursorX_ = margin_;
    carriageReturn_ = false;
}

const ConsoleLine* ConsoleBuffer::line(Size index) const {
    if (index < lines_.size()) {
        return &lines_[index];
    }
    return nullptr;
}

Vector<const ConsoleLine*> ConsoleBuffer::getNotifyLines(Double maxAge, Size maxCount) const {
    Vector<const ConsoleLine*> result;
    result.reserve(maxCount);
    
    for (const auto& line : lines_) {
        if (result.size() >= maxCount) {
            break;
        }
        
        Double age = currentTime_ - line.timestamp();
        if (age >= 0 && age < maxAge && line.timestamp() > 0) {
            result.push_back(&line);
        }
    }
    
    // Reverse so oldest is first (for display)
    std::reverse(result.begin(), result.end());
    return result;
}

void ConsoleBuffer::resize(Size newWidth, Size newMaxLines) {
    // Simple resize - just change dimensions
    // In a full implementation, we'd reflow text
    lineWidth_ = newWidth;
    maxLines_ = newMaxLines;
    
    // Trim if necessary
    while (lines_.size() > maxLines_) {
        lines_.pop_back();
    }
}

//=============================================================================
// Console Implementation
//=============================================================================

Console::Console() : buffer_(80, 1024) {
}

Console::~Console() {
    if (initialized_) {
        shutdown();
    }
}

void Console::init(Size bufferSize) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (initialized_) {
            return;
        }
        
        // Calculate lines from buffer size (assuming ~80 chars per line)
        Size maxLines = bufferSize / 80;
        if (maxLines < 128) maxLines = 128;
        if (maxLines > 16384) maxLines = 16384;
        
        buffer_ = ConsoleBuffer(80, maxLines);
        
        initialized_ = true;
        displayLine_ = 0;
        visible_ = false;
    }
    
    // Print outside of lock to avoid deadlock
    print("Console initialized\n");
}

void Console::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    initialized_ = false;
    logCallback_ = nullptr;
    printCallback_ = nullptr;
}

void Console::print(StringView text) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (suppressed_ || !initialized_) {
        // Still log even if suppressed
        if (logCallback_ && loggingEnabled_ && !hasFlag(currentFlags_, PrintFlags::NoLog)) {
            logCallback_(text);
        }
        
        if (!hasFlag(currentFlags_, PrintFlags::NoReset)) {
            currentFlags_ = PrintFlags::None;
        }
        return;
    }
    
    // Handle logging
    if (logCallback_ && loggingEnabled_ && !hasFlag(currentFlags_, PrintFlags::NoLog)) {
        logCallback_(text);
    }
    
    // Handle print callback
    if (printCallback_) {
        printCallback_(text, currentFlags_);
    }
    
    // Skip actual console output if flagged
    if (hasFlag(currentFlags_, PrintFlags::Skip)) {
        if (!hasFlag(currentFlags_, PrintFlags::NoReset)) {
            currentFlags_ = PrintFlags::None;
        }
        return;
    }
    
    // Add timestamp if enabled
    String textWithTimestamp;
    if (timestampsEnabled_) {
        textWithTimestamp = prependTimestamp(text);
        text = textWithTimestamp;
    }
    
    processText(text, ConsoleColor::white());
    
    // Reset flags unless NoReset is set
    if (!hasFlag(currentFlags_, PrintFlags::NoReset)) {
        currentFlags_ = PrintFlags::None;
    }
}

void Console::printColored(StringView text, ConsoleColor color) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (suppressed_ || !initialized_) {
        return;
    }
    
    processText(text, color);
}

void Console::printWithFlags(StringView text, PrintFlags flags) {
    currentFlags_ = flags;
    print(text);
}

void Console::safePrint(StringView text) {
    // Safe print - can be called during screen updates
    print(text);
}

void Console::processText(StringView text, ConsoleColor baseColor) {
    buffer_.print(text, baseColor);
}

String Console::prependTimestamp(StringView text) const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "[%H:%M:%S] ");
    ss << text;
    
    return ss.str();
}

void Console::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    buffer_.clear();
    displayLine_ = 0;
}

void Console::clearNotify() {
    // In the C++ version, we don't need to clear timestamps
    // The notify system checks against current time
}

void Console::checkResize(Size newWidth) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (newWidth != buffer_.lineWidth()) {
        buffer_.resize(newWidth, buffer_.maxLines());
    }
}

void Console::setDisplayLine(Size line) {
    displayLine_ = std::min(line, buffer_.lineCount());
}

void Console::scrollUp(Size lines) {
    if (displayLine_ + lines < buffer_.lineCount()) {
        displayLine_ += lines;
    } else {
        displayLine_ = buffer_.lineCount() > 0 ? buffer_.lineCount() - 1 : 0;
    }
}

void Console::scrollDown(Size lines) {
    if (displayLine_ >= lines) {
        displayLine_ -= lines;
    } else {
        displayLine_ = 0;
    }
}

void Console::scrollToTop() {
    displayLine_ = buffer_.lineCount() > 0 ? buffer_.lineCount() - 1 : 0;
}

void Console::scrollToBottom() {
    displayLine_ = 0;
}

void Console::pageUp() {
    scrollUp(visibleLines_ > 2 ? visibleLines_ - 2 : 1);
}

void Console::pageDown() {
    scrollDown(visibleLines_ > 2 ? visibleLines_ - 2 : 1);
}

void Console::toggle() {
    visible_ = !visible_;
}

Vector<const ConsoleLine*> Console::getNotifyLines(Size maxCount) const {
    return buffer_.getNotifyLines(static_cast<Double>(notifyTime_), maxCount);
}

// NOTE: Global functions (Con_Print, etc.) are implemented in compat/c_bridge.cpp
// The static Console instance lives there as well.

} // namespace ezquake
