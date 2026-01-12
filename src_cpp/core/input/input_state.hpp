/*
 * ezQuake C++ Port - Input State Manager
 * 
 * Manages button states, key bindings, and input processing.
 * 
 * Copyright (C) 2024 ezQuake Team
 */

#pragma once

#include "core/input/input_types.hpp"
#include <array>
#include <string>
#include <functional>
#include <optional>

namespace ezquake {
namespace input {

//=============================================================================
// Movement Speeds (from cvars)
//=============================================================================

/**
 * @brief Movement speed configuration.
 */
struct MovementSpeeds {
    float forwardSpeed = 400.0f;
    float backSpeed = 400.0f;
    float sideSpeed = 400.0f;
    float upSpeed = 400.0f;
    float speedKey = 2.0f;         // cl_movespeedkey
    float yawSpeed = 140.0f;       // cl_yawspeed
    float pitchSpeed = 150.0f;     // cl_pitchspeed
    float angleSpeedKey = 1.5f;    // cl_anglespeedkey
};

//=============================================================================
// Mouse Configuration
//=============================================================================

/**
 * @brief Mouse sensitivity and acceleration settings.
 */
struct MouseConfig {
    float sensitivity = 12.0f;
    float pitchScale = 0.022f;     // m_pitch
    float yawScale = 0.022f;       // m_yaw
    float forwardScale = 1.0f;     // m_forward
    float sideScale = 0.8f;        // m_side
    float accel = 0.0f;            // m_accel
    float accelOffset = 0.0f;      // m_accel_offset
    float accelPower = 2.0f;       // m_accel_power
    float accelSensCap = 0.0f;     // m_accel_senscap
    bool freeLook = true;          // freelook
    bool lookSpring = false;       // lookspring
    bool lookStrafe = false;       // lookstrafe
};

//=============================================================================
// Key Binding
//=============================================================================

/**
 * @brief A key binding (command string bound to a key).
 */
class KeyBinding {
public:
    KeyBinding() noexcept = default;
    explicit KeyBinding(std::string command) : command_(std::move(command)) {}
    
    [[nodiscard]] bool empty() const noexcept { return command_.empty(); }
    [[nodiscard]] const std::string& command() const noexcept { return command_; }
    
    void set(std::string command) { command_ = std::move(command); }
    void clear() { command_.clear(); }
    
private:
    std::string command_;
};

//=============================================================================
// Input State
//=============================================================================

/**
 * @brief Manages all input state including buttons and key bindings.
 * 
 * This class tracks:
 * - Button states for all movement commands (+forward, +attack, etc.)
 * - Key bindings for all keys
 * - Key down/up states
 * - Current key destination (game, console, menu, etc.)
 */
class InputState {
public:
    static constexpr std::size_t MAX_KEYS = static_cast<std::size_t>(KeyCode::MaxKeys);
    static constexpr std::size_t NUM_BUTTONS = static_cast<std::size_t>(InputButton::Count);
    
    InputState() noexcept = default;
    
    //-------------------------------------------------------------------------
    // Button State
    //-------------------------------------------------------------------------
    
    /**
     * @brief Get a button by enum.
     */
    [[nodiscard]] KeyButton& button(InputButton btn) noexcept {
        return buttons_[static_cast<std::size_t>(btn)];
    }
    
    [[nodiscard]] const KeyButton& button(InputButton btn) const noexcept {
        return buttons_[static_cast<std::size_t>(btn)];
    }
    
    /**
     * @brief Process a button down event.
     */
    bool buttonDown(InputButton btn, int keyNum, double currentTime) noexcept {
        return button(btn).keyDown(keyNum, currentTime);
    }
    
    /**
     * @brief Process a button up event.
     */
    bool buttonUp(InputButton btn, int keyNum, double currentTime) noexcept {
        return button(btn).keyUp(keyNum, currentTime);
    }
    
    /**
     * @brief Clear edge states for all buttons.
     */
    void clearButtonEdges() noexcept {
        for (auto& btn : buttons_) {
            btn.clearEdges();
        }
    }
    
    /**
     * @brief Clear all button states.
     */
    void clearAllButtons() noexcept {
        for (auto& btn : buttons_) {
            btn.clear();
        }
    }
    
    //-------------------------------------------------------------------------
    // Key States
    //-------------------------------------------------------------------------
    
    /**
     * @brief Check if a key is currently pressed.
     */
    [[nodiscard]] bool isKeyDown(KeyCode key) const noexcept {
        auto k = static_cast<std::size_t>(key);
        if (k < MAX_KEYS) {
            return keyDown_[k];
        }
        return false;
    }
    
    /**
     * @brief Check if a key is currently pressed (by int).
     */
    [[nodiscard]] bool isKeyDown(int key) const noexcept {
        if (key >= 0 && static_cast<std::size_t>(key) < MAX_KEYS) {
            return keyDown_[static_cast<std::size_t>(key)];
        }
        return false;
    }
    
    /**
     * @brief Set key down state.
     */
    void setKeyDown(KeyCode key, bool down) noexcept {
        auto k = static_cast<std::size_t>(key);
        if (k < MAX_KEYS) {
            keyDown_[k] = down;
            if (down) {
                lastKeyPress_ = static_cast<int>(key);
            }
        }
    }
    
    /**
     * @brief Set key down state (by int).
     */
    void setKeyDown(int key, bool down) noexcept {
        if (key >= 0 && static_cast<std::size_t>(key) < MAX_KEYS) {
            keyDown_[static_cast<std::size_t>(key)] = down;
            if (down) {
                lastKeyPress_ = key;
            }
        }
    }
    
    /**
     * @brief Get the repeat count for a key.
     */
    [[nodiscard]] int keyRepeat(KeyCode key) const noexcept {
        auto k = static_cast<std::size_t>(key);
        if (k < MAX_KEYS) {
            return keyRepeats_[k];
        }
        return 0;
    }
    
    /**
     * @brief Increment repeat count for a key.
     */
    void incrementKeyRepeat(KeyCode key) noexcept {
        auto k = static_cast<std::size_t>(key);
        if (k < MAX_KEYS) {
            keyRepeats_[k]++;
        }
    }
    
    /**
     * @brief Reset repeat count for a key.
     */
    void resetKeyRepeat(KeyCode key) noexcept {
        auto k = static_cast<std::size_t>(key);
        if (k < MAX_KEYS) {
            keyRepeats_[k] = 0;
        }
    }
    
    /**
     * @brief Get last pressed key.
     */
    [[nodiscard]] int lastKeyPress() const noexcept { return lastKeyPress_; }
    
    /**
     * @brief Clear all key states.
     */
    void clearKeyStates() noexcept {
        keyDown_.fill(false);
        keyRepeats_.fill(0);
        lastKeyPress_ = 0;
    }
    
    //-------------------------------------------------------------------------
    // Key Bindings
    //-------------------------------------------------------------------------
    
    /**
     * @brief Get binding for a key.
     */
    [[nodiscard]] const KeyBinding& binding(KeyCode key) const noexcept {
        auto k = static_cast<std::size_t>(key);
        if (k < MAX_KEYS) {
            return bindings_[k];
        }
        static KeyBinding empty;
        return empty;
    }
    
    /**
     * @brief Get binding for a key (by int).
     */
    [[nodiscard]] const KeyBinding& binding(int key) const noexcept {
        if (key >= 0 && static_cast<std::size_t>(key) < MAX_KEYS) {
            return bindings_[static_cast<std::size_t>(key)];
        }
        static KeyBinding empty;
        return empty;
    }
    
    /**
     * @brief Set binding for a key.
     */
    void setBinding(KeyCode key, std::string command) {
        auto k = static_cast<std::size_t>(key);
        if (k < MAX_KEYS) {
            bindings_[k].set(std::move(command));
        }
    }
    
    /**
     * @brief Set binding for a key (by int).
     */
    void setBinding(int key, std::string command) {
        if (key >= 0 && static_cast<std::size_t>(key) < MAX_KEYS) {
            bindings_[static_cast<std::size_t>(key)].set(std::move(command));
        }
    }
    
    /**
     * @brief Unbind a key.
     */
    void unbind(KeyCode key) {
        auto k = static_cast<std::size_t>(key);
        if (k < MAX_KEYS) {
            bindings_[k].clear();
        }
    }
    
    /**
     * @brief Unbind all keys.
     */
    void unbindAll() {
        for (auto& binding : bindings_) {
            binding.clear();
        }
    }
    
    //-------------------------------------------------------------------------
    // Key Destination
    //-------------------------------------------------------------------------
    
    /**
     * @brief Get current key destination.
     */
    [[nodiscard]] KeyDestination destination() const noexcept { return destination_; }
    
    /**
     * @brief Set key destination.
     */
    void setDestination(KeyDestination dest) noexcept { destination_ = dest; }
    
    /**
     * @brief Check if we're in game input mode.
     */
    [[nodiscard]] bool isGameMode() const noexcept {
        return destination_ == KeyDestination::Game;
    }
    
    /**
     * @brief Check if we're in console mode.
     */
    [[nodiscard]] bool isConsoleMode() const noexcept {
        return destination_ == KeyDestination::Console;
    }
    
    /**
     * @brief Check if we're in menu mode.
     */
    [[nodiscard]] bool isMenuMode() const noexcept {
        return destination_ == KeyDestination::Menu;
    }
    
    //-------------------------------------------------------------------------
    // Impulse
    //-------------------------------------------------------------------------
    
    /**
     * @brief Get current impulse command.
     */
    [[nodiscard]] int impulse() const noexcept { return impulse_; }
    
    /**
     * @brief Set impulse command.
     */
    void setImpulse(int imp) noexcept { impulse_ = imp; }
    
    /**
     * @brief Clear impulse command.
     */
    void clearImpulse() noexcept { impulse_ = 0; }
    
    //-------------------------------------------------------------------------
    // Mouse State
    //-------------------------------------------------------------------------
    
    /**
     * @brief Get mouse state.
     */
    [[nodiscard]] MouseState& mouseState() noexcept { return mouse_; }
    [[nodiscard]] const MouseState& mouseState() const noexcept { return mouse_; }
    
    //-------------------------------------------------------------------------
    // Configuration
    //-------------------------------------------------------------------------
    
    /**
     * @brief Get movement speed configuration.
     */
    [[nodiscard]] MovementSpeeds& speeds() noexcept { return speeds_; }
    [[nodiscard]] const MovementSpeeds& speeds() const noexcept { return speeds_; }
    
    /**
     * @brief Get mouse configuration.
     */
    [[nodiscard]] MouseConfig& mouseConfig() noexcept { return mouseConfig_; }
    [[nodiscard]] const MouseConfig& mouseConfig() const noexcept { return mouseConfig_; }
    
    //-------------------------------------------------------------------------
    // Utility
    //-------------------------------------------------------------------------
    
    /**
     * @brief Check if mouse look is active.
     */
    [[nodiscard]] bool isMouseLookActive() const noexcept {
        return mouseConfig_.freeLook || button(InputButton::MLook).isDown();
    }
    
    /**
     * @brief Clear all state.
     */
    void clear() {
        clearAllButtons();
        clearKeyStates();
        unbindAll();
        destination_ = KeyDestination::Game;
        impulse_ = 0;
        mouse_.clear();
        speeds_ = MovementSpeeds{};
        mouseConfig_ = MouseConfig{};
    }
    
private:
    // Button states for movement commands
    std::array<KeyButton, NUM_BUTTONS> buttons_;
    
    // Key down states
    std::array<bool, MAX_KEYS> keyDown_ = {};
    
    // Key repeat counts
    std::array<int, MAX_KEYS> keyRepeats_ = {};
    
    // Key bindings
    std::array<KeyBinding, MAX_KEYS> bindings_;
    
    // Current key destination
    KeyDestination destination_ = KeyDestination::Game;
    
    // Last key pressed
    int lastKeyPress_ = 0;
    
    // Current impulse command
    int impulse_ = 0;
    
    // Mouse state
    MouseState mouse_;
    
    // Movement speed configuration
    MovementSpeeds speeds_;
    
    // Mouse configuration
    MouseConfig mouseConfig_;
};

} // namespace input
} // namespace ezquake
