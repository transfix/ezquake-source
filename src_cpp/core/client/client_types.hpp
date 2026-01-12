/*
 * ezQuake C++ Port - Client Core Types
 * 
 * Modern C++20 replacements for client.h structures.
 * These types represent the core client-side game state.
 * 
 * Copyright (C) 2024 ezQuake Team
 */

#pragma once

#include "core/types.hpp"
#include "core/math/vec3.hpp"
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <chrono>
#include <optional>

namespace ezquake {

//=============================================================================
// Forward Declarations
//=============================================================================

class UserCommand;
class PlayerState;
class PlayerInfo;
class Frame;
class Entity;
class ClientState;

//=============================================================================
// Constants
//=============================================================================

namespace client {

// Maximum values matching original Quake protocol
inline constexpr int MAX_CLIENTS = 32;
inline constexpr int MAX_MODELS = 512;
inline constexpr int MAX_SOUNDS = 512;
inline constexpr int MAX_VWEP_MODELS = 32;
inline constexpr int MAX_STATIC_SOUNDS = 256;
inline constexpr int MAX_STATIC_ENTITIES = 2048;
inline constexpr int MAX_DLIGHTS = 32;
inline constexpr int MAX_PROJECTILES = 32;
inline constexpr int MAX_WEAPONS = 10;
inline constexpr int MAX_CL_STATS = 32;
inline constexpr int MAX_SCOREBOARDNAME = 16;
inline constexpr int MAX_INFO_STRING = 196;
inline constexpr int UPDATE_BACKUP = 64;
inline constexpr int MV_VIEWS = 4;

// Button bits for user commands
enum class Button : std::uint8_t {
    None    = 0,
    Attack  = 1 << 0,
    Jump    = 1 << 1,
    Use     = 1 << 2,
    Attack2 = 1 << 3,
};

// Enable bitwise operations on Button
[[nodiscard]] constexpr Button operator|(Button lhs, Button rhs) noexcept {
    return static_cast<Button>(static_cast<std::uint8_t>(lhs) | static_cast<std::uint8_t>(rhs));
}
[[nodiscard]] constexpr Button operator&(Button lhs, Button rhs) noexcept {
    return static_cast<Button>(static_cast<std::uint8_t>(lhs) & static_cast<std::uint8_t>(rhs));
}
[[nodiscard]] constexpr bool hasButton(Button buttons, Button check) noexcept {
    return (static_cast<std::uint8_t>(buttons) & static_cast<std::uint8_t>(check)) != 0;
}

// Connection states
enum class ConnectionState : std::uint8_t {
    Disconnected,   // Full screen console with no connection
    DemoStart,      // Starting up a demo
    Connected,      // Netchan established, waiting for svc_serverdata
    OnServer,       // Processing data lists, downloading, etc
    Active          // Everything is in, frames can be rendered
};

// Game types
enum class GameType : std::uint8_t {
    Coop = 0,
    Deathmatch = 1,
};

// Gender for player info
enum class Gender : std::uint8_t {
    Unknown = 0,
    Male,
    Female,
    Neutral
};

// Camera modes
enum class CameraMode : std::uint8_t {
    None = 0,
    Track = 1,
};

// Dynamic light types
enum class DynamicLightType : std::uint8_t {
    Default,
    MuzzleFlash,
    Explosion,
    Rocket,
    Red,
    Blue,
    RedBlue,
    Green,
    RedGreen,
    BlueGreen,
    White,
    Custom,
    Count
};

// Color shift types
enum class ColorShiftType : std::uint8_t {
    Contents = 0,
    Damage,
    Bonus,
    Powerup,
    Count
};

// Download types
enum class DownloadType : std::uint8_t {
    None,
    Model,
    VWepModel,
    Sound,
    Skin,
    Single
};

// Demo seeking types
enum class DemoSeekType : std::uint8_t {
    None = 0,
    Normal,
    DemoMark,
    Status,
    Found,
    End,
    FoundNoRewind
};

} // namespace client

//=============================================================================
// UserCommand - Input command from player
//=============================================================================

/**
 * @brief Represents a single user input command.
 * 
 * This is sent from client to server each frame, containing
 * player inputs like movement and button presses.
 * 
 * Original: usercmd_t in protocol.h
 */
class UserCommand {
public:
    constexpr UserCommand() noexcept = default;
    
    // Accessors
    [[nodiscard]] constexpr std::uint8_t msec() const noexcept { return msec_; }
    [[nodiscard]] constexpr const Vec3& angles() const noexcept { return angles_; }
    [[nodiscard]] constexpr std::int16_t forwardMove() const noexcept { return forwardMove_; }
    [[nodiscard]] constexpr std::int16_t sideMove() const noexcept { return sideMove_; }
    [[nodiscard]] constexpr std::int16_t upMove() const noexcept { return upMove_; }
    [[nodiscard]] constexpr client::Button buttons() const noexcept { return buttons_; }
    [[nodiscard]] constexpr std::uint8_t impulse() const noexcept { return impulse_; }
    
    // Button queries
    [[nodiscard]] constexpr bool isAttacking() const noexcept {
        return client::hasButton(buttons_, client::Button::Attack);
    }
    [[nodiscard]] constexpr bool isJumping() const noexcept {
        return client::hasButton(buttons_, client::Button::Jump);
    }
    [[nodiscard]] constexpr bool isUsing() const noexcept {
        return client::hasButton(buttons_, client::Button::Use);
    }
    [[nodiscard]] constexpr bool isAttacking2() const noexcept {
        return client::hasButton(buttons_, client::Button::Attack2);
    }
    
    // Mutators
    constexpr void setMsec(std::uint8_t ms) noexcept { msec_ = ms; }
    constexpr void setAngles(const Vec3& angles) noexcept { angles_ = angles; }
    constexpr void setForwardMove(std::int16_t move) noexcept { forwardMove_ = move; }
    constexpr void setSideMove(std::int16_t move) noexcept { sideMove_ = move; }
    constexpr void setUpMove(std::int16_t move) noexcept { upMove_ = move; }
    constexpr void setButtons(client::Button buttons) noexcept { buttons_ = buttons; }
    constexpr void setImpulse(std::uint8_t imp) noexcept { impulse_ = imp; }
    
    // Builder pattern for fluent API
    [[nodiscard]] constexpr UserCommand withMsec(std::uint8_t ms) const noexcept {
        UserCommand cmd = *this;
        cmd.msec_ = ms;
        return cmd;
    }
    [[nodiscard]] constexpr UserCommand withAngles(const Vec3& angles) const noexcept {
        UserCommand cmd = *this;
        cmd.angles_ = angles;
        return cmd;
    }
    [[nodiscard]] constexpr UserCommand withForwardMove(std::int16_t move) const noexcept {
        UserCommand cmd = *this;
        cmd.forwardMove_ = move;
        return cmd;
    }
    [[nodiscard]] constexpr UserCommand withSideMove(std::int16_t move) const noexcept {
        UserCommand cmd = *this;
        cmd.sideMove_ = move;
        return cmd;
    }
    [[nodiscard]] constexpr UserCommand withUpMove(std::int16_t move) const noexcept {
        UserCommand cmd = *this;
        cmd.upMove_ = move;
        return cmd;
    }
    [[nodiscard]] constexpr UserCommand withButtons(client::Button buttons) const noexcept {
        UserCommand cmd = *this;
        cmd.buttons_ = buttons;
        return cmd;
    }
    [[nodiscard]] constexpr UserCommand withImpulse(std::uint8_t imp) const noexcept {
        UserCommand cmd = *this;
        cmd.impulse_ = imp;
        return cmd;
    }
    
    // Clear all state
    constexpr void clear() noexcept {
        msec_ = 0;
        angles_ = Vec3{};
        forwardMove_ = 0;
        sideMove_ = 0;
        upMove_ = 0;
        buttons_ = client::Button::None;
        impulse_ = 0;
    }
    
    // Check if empty (no input)
    [[nodiscard]] constexpr bool empty() const noexcept {
        return msec_ == 0 && forwardMove_ == 0 && sideMove_ == 0 && 
               upMove_ == 0 && buttons_ == client::Button::None && impulse_ == 0;
    }
    
    // Comparison
    [[nodiscard]] constexpr bool operator==(const UserCommand& other) const noexcept = default;
    
private:
    std::uint8_t msec_ = 0;              // Duration of this command in milliseconds
    Vec3 angles_;                         // View angles
    std::int16_t forwardMove_ = 0;        // Forward/back movement (-400 to 400)
    std::int16_t sideMove_ = 0;           // Left/right strafe (-400 to 400)
    std::int16_t upMove_ = 0;             // Up/down movement (swim/fly)
    client::Button buttons_ = client::Button::None;  // Button state
    std::uint8_t impulse_ = 0;            // Weapon switch impulse
};

//=============================================================================
// ColorShift - Screen color overlay
//=============================================================================

/**
 * @brief Represents a screen color shift (damage flash, powerups, etc.)
 * 
 * Original: cshift_t
 */
class ColorShift {
public:
    constexpr ColorShift() noexcept = default;
    constexpr ColorShift(std::uint8_t r, std::uint8_t g, std::uint8_t b, float pct) noexcept
        : destColor_{r, g, b}, percent_(pct) {}
    
    [[nodiscard]] constexpr std::uint8_t red() const noexcept { return destColor_[0]; }
    [[nodiscard]] constexpr std::uint8_t green() const noexcept { return destColor_[1]; }
    [[nodiscard]] constexpr std::uint8_t blue() const noexcept { return destColor_[2]; }
    [[nodiscard]] constexpr float percent() const noexcept { return percent_; }
    
    constexpr void setColor(std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept {
        destColor_[0] = r;
        destColor_[1] = g;
        destColor_[2] = b;
    }
    constexpr void setPercent(float pct) noexcept { percent_ = pct; }
    
    // Interpolate towards zero
    constexpr void decay(float amount) noexcept {
        percent_ -= amount;
        if (percent_ < 0.0f) percent_ = 0.0f;
    }
    
    [[nodiscard]] constexpr bool isActive() const noexcept { return percent_ > 0.0f; }
    
private:
    std::array<std::uint8_t, 3> destColor_ = {0, 0, 0};
    float percent_ = 0.0f;  // 0-256 range
};

//=============================================================================
// DynamicLight - Dynamic light source
//=============================================================================

/**
 * @brief Represents a dynamic light in the scene.
 * 
 * Original: dlight_t
 */
class DynamicLight {
public:
    constexpr DynamicLight() noexcept = default;
    
    // Accessors
    [[nodiscard]] constexpr int key() const noexcept { return key_; }
    [[nodiscard]] constexpr const Vec3& origin() const noexcept { return origin_; }
    [[nodiscard]] constexpr float radius() const noexcept { return radius_; }
    [[nodiscard]] constexpr float dieTime() const noexcept { return dieTime_; }
    [[nodiscard]] constexpr float decay() const noexcept { return decay_; }
    [[nodiscard]] constexpr float minLight() const noexcept { return minLight_; }
    [[nodiscard]] constexpr bool isBubble() const noexcept { return bubble_; }
    [[nodiscard]] constexpr client::DynamicLightType type() const noexcept { return type_; }
    [[nodiscard]] constexpr std::uint8_t colorRed() const noexcept { return color_[0]; }
    [[nodiscard]] constexpr std::uint8_t colorGreen() const noexcept { return color_[1]; }
    [[nodiscard]] constexpr std::uint8_t colorBlue() const noexcept { return color_[2]; }
    
    // Mutators
    constexpr void setKey(int key) noexcept { key_ = key; }
    constexpr void setOrigin(const Vec3& origin) noexcept { origin_ = origin; }
    constexpr void setRadius(float radius) noexcept { radius_ = radius; }
    constexpr void setDieTime(float time) noexcept { dieTime_ = time; }
    constexpr void setDecay(float decay) noexcept { decay_ = decay; }
    constexpr void setMinLight(float minLight) noexcept { minLight_ = minLight; }
    constexpr void setBubble(bool bubble) noexcept { bubble_ = bubble; }
    constexpr void setType(client::DynamicLightType type) noexcept { type_ = type; }
    constexpr void setColor(std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept {
        color_[0] = r;
        color_[1] = g;
        color_[2] = b;
    }
    
    // Update light for a time delta
    void update(float deltaTime, float currentTime) noexcept {
        radius_ -= decay_ * deltaTime;
        if (radius_ < minLight_) {
            radius_ = 0.0f;
        }
    }
    
    [[nodiscard]] constexpr bool isActive(float currentTime) const noexcept {
        return radius_ > 0.0f && (dieTime_ == 0.0f || currentTime < dieTime_);
    }
    
    void clear() noexcept {
        key_ = 0;
        origin_ = Vec3{};
        radius_ = 0.0f;
        dieTime_ = 0.0f;
        decay_ = 0.0f;
        minLight_ = 0.0f;
        bubble_ = false;
        type_ = client::DynamicLightType::Default;
        color_ = {0, 0, 0};
    }
    
private:
    int key_ = 0;                        // Entity key for reuse
    Vec3 origin_;                        // Position in world
    float radius_ = 0.0f;                // Light radius
    float dieTime_ = 0.0f;               // Stop lighting after this time
    float decay_ = 0.0f;                 // Radius reduction per second
    float minLight_ = 0.0f;              // Don't add when contributing less
    bool bubble_ = false;                // No flashblend bubble
    client::DynamicLightType type_ = client::DynamicLightType::Default;
    std::array<std::uint8_t, 3> color_ = {0, 0, 0};  // Custom color
};

//=============================================================================
// StaticSound - Ambient sound in the world
//=============================================================================

/**
 * @brief Represents a static/ambient sound in the world.
 * 
 * Original: static_sound_t
 */
class StaticSound {
public:
    constexpr StaticSound() noexcept = default;
    constexpr StaticSound(const Vec3& origin, int soundNum, int vol, int atten) noexcept
        : origin_(origin), soundNum_(soundNum), volume_(vol), attenuation_(atten) {}
    
    [[nodiscard]] constexpr const Vec3& origin() const noexcept { return origin_; }
    [[nodiscard]] constexpr int soundNum() const noexcept { return soundNum_; }
    [[nodiscard]] constexpr int volume() const noexcept { return volume_; }
    [[nodiscard]] constexpr int attenuation() const noexcept { return attenuation_; }
    
    constexpr void setOrigin(const Vec3& origin) noexcept { origin_ = origin; }
    constexpr void setSoundNum(int num) noexcept { soundNum_ = num; }
    constexpr void setVolume(int vol) noexcept { volume_ = vol; }
    constexpr void setAttenuation(int atten) noexcept { attenuation_ = atten; }
    
private:
    Vec3 origin_;
    int soundNum_ = 0;
    int volume_ = 0;
    int attenuation_ = 0;
};

//=============================================================================
// LightStyle - Animated light style
//=============================================================================

/**
 * @brief Represents an animated light style.
 * 
 * Original: lightstyle_t
 */
class LightStyle {
public:
    static constexpr int MAX_STYLE_STRING = 64;
    
    constexpr LightStyle() noexcept = default;
    
    void setPattern(std::string_view pattern) {
        pattern_ = pattern.substr(0, MAX_STYLE_STRING);
    }
    
    [[nodiscard]] std::string_view pattern() const noexcept { return pattern_; }
    [[nodiscard]] std::size_t length() const noexcept { return pattern_.length(); }
    [[nodiscard]] bool empty() const noexcept { return pattern_.empty(); }
    
    // Get light value at a given time (0-255 range based on 'a'-'z')
    [[nodiscard]] int valueAt(double time) const noexcept {
        if (pattern_.empty()) return 255;
        
        std::size_t index = static_cast<std::size_t>(time * 10.0) % pattern_.length();
        char c = pattern_[index];
        // 'a' = 0, 'z' = 25, maps to 0-255
        return (c - 'a') * 22;
    }
    
private:
    std::string pattern_;
};

} // namespace ezquake
