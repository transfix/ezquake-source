/**
 * @file view_types.hpp
 * @brief View rendering and effect type definitions
 * 
 * Converted from cl_view.c and cl_view.h
 * Core types for view calculations, color shifts, and screen effects.
 */

#pragma once

#include <cstdint>
#include <array>
#include <algorithm>
#include <cmath>
#include <string_view>

namespace ezquake {

//=============================================================================
// View Movement Constants (from cl_view.c)
//=============================================================================

namespace view_bob {
    constexpr float DEFAULT_BOB = 0.0f;          ///< cl_bob default
    constexpr float DEFAULT_BOBCYCLE = 0.0f;     ///< cl_bobcycle default
    constexpr float DEFAULT_BOBUP = 0.0f;        ///< cl_bobup default
    constexpr float MIN_BOB = -7.0f;             ///< Minimum bob offset
    constexpr float MAX_BOB = 4.0f;              ///< Maximum bob offset
}

namespace view_roll {
    constexpr float DEFAULT_SPEED = 200.0f;      ///< cl_rollspeed default
    constexpr float DEFAULT_ANGLE = 0.0f;        ///< cl_rollangle default
    constexpr float DEFAULT_ALPHA = 20.0f;       ///< cl_rollalpha default
    constexpr float MAX_ROLL = 45.0f;            ///< Maximum roll angle
}

namespace view_kick {
    constexpr float DEFAULT_KICKTIME = 0.0f;     ///< v_kicktime default
    constexpr float DEFAULT_KICKROLL = 0.0f;     ///< v_kickroll default
    constexpr float DEFAULT_KICKPITCH = 0.0f;    ///< v_kickpitch default
}

namespace view_idle {
    constexpr float DEFAULT_YAW_CYCLE = 2.0f;    ///< v_iyaw_cycle
    constexpr float DEFAULT_ROLL_CYCLE = 0.5f;   ///< v_iroll_cycle
    constexpr float DEFAULT_PITCH_CYCLE = 1.0f;  ///< v_ipitch_cycle
    constexpr float DEFAULT_YAW_LEVEL = 0.3f;    ///< v_iyaw_level
    constexpr float DEFAULT_ROLL_LEVEL = 0.1f;   ///< v_iroll_level
    constexpr float DEFAULT_PITCH_LEVEL = 0.3f;  ///< v_ipitch_level
}

namespace view_height {
    constexpr float DEFAULT = 22.0f;             ///< DEFAULT_VIEWHEIGHT
    constexpr float GIB = 8.0f;                  ///< Gib view height offset
    constexpr float DEAD = -16.0f;               ///< Dead/corpse view height offset
    constexpr float MIN_OFFSET = -7.0f;          ///< Minimum v_viewheight
    constexpr float MAX_OFFSET = 4.0f;           ///< Maximum v_viewheight
}

//=============================================================================
// Color Shift Types (from cl_view.c)
//=============================================================================

/**
 * @brief Color shift indices for screen tinting effects
 * 
 * Original: CSHIFT_* defines in quakedef.h
 */
enum class ColorShiftType : uint8_t {
    Contents = 0,   ///< Water, lava, slime content tint
    Damage = 1,     ///< Damage received flash
    Bonus = 2,      ///< Item pickup flash
    Powerup = 3     ///< Active powerup tint (quad, pent, etc)
};

constexpr size_t NUM_COLOR_SHIFTS = 4;

/**
 * @brief Get name of color shift type for debugging
 */
constexpr std::string_view colorShiftTypeName(ColorShiftType type) noexcept {
    switch (type) {
        case ColorShiftType::Contents: return "contents";
        case ColorShiftType::Damage:   return "damage";
        case ColorShiftType::Bonus:    return "bonus";
        case ColorShiftType::Powerup:  return "powerup";
        default:                       return "unknown";
    }
}

/**
 * @brief Color shift state for screen tinting
 * 
 * Original: cshift_t
 * Note: Named ViewColorShiftState to avoid conflict with client::ColorShift
 */
struct ViewColorShiftState {
    std::array<uint8_t, 3> destColor{0, 0, 0};  ///< Target RGB color (0-255)
    int percent{0};                              ///< Blend percentage (0-150)
    
    /**
     * @brief Check if shift is active
     */
    [[nodiscard]] bool isActive() const noexcept {
        return percent > 0;
    }
    
    /**
     * @brief Get blend alpha (0.0 - 1.0 range)
     */
    [[nodiscard]] float alpha() const noexcept {
        return static_cast<float>(std::clamp(percent, 0, 150)) / 255.0f;
    }
    
    /**
     * @brief Get normalized color (0.0 - 1.0 range)
     */
    [[nodiscard]] std::array<float, 3> normalizedColor() const noexcept {
        return {
            static_cast<float>(destColor[0]) / 255.0f,
            static_cast<float>(destColor[1]) / 255.0f,
            static_cast<float>(destColor[2]) / 255.0f
        };
    }
    
    /**
     * @brief Reset to no shift
     */
    void reset() noexcept {
        destColor = {0, 0, 0};
        percent = 0;
    }
    
    /**
     * @brief Decay the shift by a delta (for damage/bonus fade)
     */
    void decay(float amount) noexcept {
        percent = std::max(0, percent - static_cast<int>(amount));
    }
    
    /**
     * @brief Set color shift
     */
    void set(uint8_t r, uint8_t g, uint8_t b, int pct) noexcept {
        destColor = {r, g, b};
        percent = std::clamp(pct, 0, 150);
    }
};

/**
 * @brief Predefined color shift values
 */
namespace color_shifts {
    // Content color shifts
    constexpr std::array<uint8_t, 3> EMPTY_COLOR = {130, 80, 50};
    constexpr int EMPTY_PERCENT = 0;
    
    constexpr std::array<uint8_t, 3> WATER_COLOR = {130, 80, 50};
    constexpr int WATER_PERCENT = 128;
    
    constexpr std::array<uint8_t, 3> SLIME_COLOR = {0, 25, 5};
    constexpr int SLIME_PERCENT = 150;
    
    constexpr std::array<uint8_t, 3> LAVA_COLOR = {255, 80, 0};
    constexpr int LAVA_PERCENT = 150;
    
    // Bonus flash
    constexpr std::array<uint8_t, 3> BONUS_COLOR = {215, 186, 69};
    constexpr int BONUS_PERCENT = 50;
    
    // Powerup colors
    constexpr std::array<uint8_t, 3> QUAD_COLOR = {0, 0, 255};
    constexpr std::array<uint8_t, 3> SUIT_COLOR = {0, 255, 0};
    constexpr std::array<uint8_t, 3> RING_COLOR = {100, 100, 100};
    constexpr std::array<uint8_t, 3> PENT_COLOR = {255, 255, 0};
}

//=============================================================================
// Contents Type (for content-based color shifts)
//=============================================================================

/**
 * @brief World contents types
 * 
 * Original: CONTENTS_* defines
 */
enum class ContentsType : int8_t {
    Empty = -1,
    Solid = -2,
    Water = -3,
    Slime = -4,
    Lava = -5,
    Sky = -6
};

/**
 * @brief Get default color shift for contents type
 */
inline ViewColorShiftState getContentsColorShift(ContentsType contents) noexcept {
    ViewColorShiftState shift;
    switch (contents) {
        case ContentsType::Empty:
            shift.destColor = color_shifts::EMPTY_COLOR;
            shift.percent = color_shifts::EMPTY_PERCENT;
            break;
        case ContentsType::Lava:
            shift.destColor = color_shifts::LAVA_COLOR;
            shift.percent = color_shifts::LAVA_PERCENT;
            break;
        case ContentsType::Solid:
        case ContentsType::Slime:
            shift.destColor = color_shifts::SLIME_COLOR;
            shift.percent = color_shifts::SLIME_PERCENT;
            break;
        case ContentsType::Water:
        case ContentsType::Sky:
        default:
            shift.destColor = color_shifts::WATER_COLOR;
            shift.percent = color_shifts::WATER_PERCENT;
            break;
    }
    return shift;
}

//=============================================================================
// View Blend State
//=============================================================================

/**
 * @brief Final blended screen color (RGBA)
 * 
 * Original: v_blend[4]
 */
struct ViewBlend {
    float r{0.0f};
    float g{0.0f};
    float b{0.0f};
    float a{0.0f};
    
    /**
     * @brief Reset to no blend
     */
    void reset() noexcept {
        r = g = b = a = 0.0f;
    }
    
    /**
     * @brief Check if any blending is active
     */
    [[nodiscard]] bool isActive() const noexcept {
        return a > 0.0f;
    }
    
    /**
     * @brief Add a color blend contribution
     */
    void addBlend(float cr, float cg, float cb, float ca) noexcept {
        if (ca <= 0.0f) return;
        
        a = a + ca * (1.0f - a);
        if (a <= 0.0f) return;
        
        float a2 = ca / a;
        r = r * (1.0f - a2) + cr * a2;
        g = g * (1.0f - a2) + cg * a2;
        b = b * (1.0f - a2) + cb * a2;
    }
    
    /**
     * @brief Get as array [r, g, b, a]
     */
    [[nodiscard]] std::array<float, 4> toArray() const noexcept {
        return {r, g, b, std::clamp(a, 0.0f, 1.0f)};
    }
};

//=============================================================================
// View Bob State
//=============================================================================

/**
 * @brief View bob calculation state
 * 
 * Handles head bobbing while walking/running
 */
struct ViewBob {
    double bobTime{0.0};    ///< Accumulated bob time
    float lastBob{0.0f};    ///< Last calculated bob value
    
    // Parameters (from cvars)
    float bobAmount{view_bob::DEFAULT_BOB};
    float bobCycle{view_bob::DEFAULT_BOBCYCLE};
    float bobUp{view_bob::DEFAULT_BOBUP};
    
    /**
     * @brief Calculate bob offset for current frame
     * 
     * @param onGround Whether player is on ground
     * @param velocity XY velocity magnitude
     * @param frameTime Frame time delta
     * @return Bob offset to apply to view
     */
    float calculate(bool onGround, float velocityXY, double frameTime) noexcept {
        if (!onGround) {
            return lastBob;  // Keep last value when airborne
        }
        
        if (bobCycle <= 0.0f) {
            return 0.0f;
        }
        
        bobTime += frameTime;
        double bobCycleD = static_cast<double>(bobCycle);
        float cycle = static_cast<float>(bobTime - static_cast<double>(static_cast<int>(bobTime / bobCycleD)) * bobCycleD);
        cycle /= bobCycle;
        
        float sinValue;
        if (cycle < bobUp) {
            sinValue = std::sin(static_cast<float>(M_PI) * cycle / bobUp);
        } else {
            sinValue = std::sin(static_cast<float>(M_PI) + static_cast<float>(M_PI) * (cycle - bobUp) / (1.0f - bobUp));
        }
        
        float bob = velocityXY * bobAmount;
        bob = bob * 0.3f + bob * 0.7f * sinValue;
        lastBob = std::clamp(bob, view_bob::MIN_BOB, view_bob::MAX_BOB);
        
        return lastBob;
    }
    
    /**
     * @brief Reset bob state
     */
    void reset() noexcept {
        bobTime = 0.0;
        lastBob = 0.0f;
    }
};

//=============================================================================
// View Roll State
//=============================================================================

/**
 * @brief View roll calculation state
 * 
 * Handles screen roll when strafing
 */
struct ViewRoll {
    float rollAngle{0.0f};  ///< Current smoothed roll angle
    
    // Parameters (from cvars)
    float rollSpeed{view_roll::DEFAULT_SPEED};
    float maxRollAngle{view_roll::DEFAULT_ANGLE};
    float rollAlpha{view_roll::DEFAULT_ALPHA};
    
    /**
     * @brief Calculate roll angle from velocity
     * 
     * @param rightDir Right direction vector component of velocity
     * @return Target roll angle (before smoothing)
     */
    [[nodiscard]] float calculateTarget(float rightVelocity) const noexcept {
        float sign = (rightVelocity < 0) ? -1.0f : 1.0f;
        float side = std::abs(rightVelocity);
        
        float angle;
        if (side < rollSpeed) {
            angle = side * maxRollAngle / rollSpeed;
        } else {
            angle = maxRollAngle;
        }
        
        return std::min(angle, view_roll::MAX_ROLL) * sign;
    }
    
    /**
     * @brief Update roll with smoothing
     * 
     * @param targetRoll Target roll from calculateTarget
     * @param frameTime Frame time delta
     */
    void update(float targetRoll, float frameTime) noexcept {
        float adjSpeed = rollAlpha * std::clamp(maxRollAngle, 2.0f, 45.0f);
        
        if (targetRoll > rollAngle) {
            rollAngle += frameTime * adjSpeed;
            if (rollAngle > targetRoll) rollAngle = targetRoll;
        } else if (targetRoll < rollAngle) {
            rollAngle -= frameTime * adjSpeed;
            if (rollAngle < targetRoll) rollAngle = targetRoll;
        }
    }
    
    /**
     * @brief Reset roll state
     */
    void reset() noexcept {
        rollAngle = 0.0f;
    }
};

//=============================================================================
// Damage Kick State
//=============================================================================

/**
 * @brief View kick from damage
 * 
 * Handles screen shake/kick when taking damage
 */
struct DamageKick {
    float time{0.0f};       ///< Remaining kick time
    float roll{0.0f};       ///< Kick roll angle
    float pitch{0.0f};      ///< Kick pitch angle
    
    // Parameters (from cvars)
    float kickTime{view_kick::DEFAULT_KICKTIME};
    float kickRoll{view_kick::DEFAULT_KICKROLL};
    float kickPitch{view_kick::DEFAULT_KICKPITCH};
    
    /**
     * @brief Check if kick is active
     */
    [[nodiscard]] bool isActive() const noexcept {
        return time > 0.0f;
    }
    
    /**
     * @brief Get current roll contribution
     */
    [[nodiscard]] float currentRoll() const noexcept {
        if (kickTime <= 0.0f || time <= 0.0f) return 0.0f;
        return (time / kickTime) * roll;
    }
    
    /**
     * @brief Get current pitch contribution
     */
    [[nodiscard]] float currentPitch() const noexcept {
        if (kickTime <= 0.0f || time <= 0.0f) return 0.0f;
        return (time / kickTime) * pitch;
    }
    
    /**
     * @brief Apply damage to kick state
     * 
     * @param damage Damage amount
     * @param side Side direction of damage
     * @param forward Forward direction of damage
     */
    void applyDamage(float damage, float side, float forward) noexcept {
        roll = damage * side * kickRoll;
        pitch = damage * forward * kickPitch;
        time = kickTime;
    }
    
    /**
     * @brief Update kick (decay over time)
     */
    void update(float frameTime) noexcept {
        if (time > 0.0f) {
            time -= frameTime;
            if (time < 0.0f) time = 0.0f;
        }
    }
    
    /**
     * @brief Reset kick state
     */
    void reset() noexcept {
        time = roll = pitch = 0.0f;
    }
};

//=============================================================================
// Pitch Drift State
//=============================================================================

/**
 * @brief Automatic pitch centering state
 * 
 * Gradually centers view pitch when not manually looking up/down
 */
struct PitchDrift {
    bool enabled{false};        ///< Whether drift is active (nodrift = !enabled)
    float velocity{0.0f};       ///< Current drift velocity
    float moveAccum{0.0f};      ///< Accumulated movement for drift trigger
    double lastStop{0.0};       ///< Last time drift was stopped
    
    // Parameters (from cvars)
    float centerMove{0.15f};    ///< v_centermove - movement threshold to start drift
    float centerSpeed{500.0f};  ///< v_centerspeed - drift speed
    
    /**
     * @brief Start pitch drifting
     */
    void start(double currentTime) noexcept {
        if (lastStop == currentTime) return;
        
        if (!enabled || velocity == 0.0f) {
            velocity = centerSpeed;
            enabled = true;
            moveAccum = 0.0f;
        }
    }
    
    /**
     * @brief Stop pitch drifting
     */
    void stop(double currentTime) noexcept {
        lastStop = currentTime;
        enabled = false;
        velocity = 0.0f;
    }
    
    /**
     * @brief Update drift and return pitch adjustment
     * 
     * @param currentPitch Current view pitch
     * @param forwardMove Forward movement amount
     * @param onGround Whether on ground
     * @param demoPlayback Whether in demo playback
     * @param frameTime Frame time delta
     * @return Pitch adjustment to apply
     */
    float update(float currentPitch, float forwardMove, bool onGround, 
                 bool demoPlayback, float frameTime) noexcept {
        if (!onGround || demoPlayback) {
            moveAccum = 0.0f;
            velocity = 0.0f;
            return 0.0f;
        }
        
        if (!enabled) {
            if (std::abs(forwardMove) < 200.0f) {
                moveAccum = 0.0f;
            } else {
                moveAccum += frameTime;
            }
            
            if (moveAccum > centerMove) {
                start(0.0);  // Time doesn't matter for auto-start
            }
            return 0.0f;
        }
        
        float delta = 0.0f - currentPitch;
        if (delta == 0.0f) {
            velocity = 0.0f;
            return 0.0f;
        }
        
        float move = frameTime * velocity;
        velocity += frameTime * centerSpeed;
        
        if (delta > 0.0f) {
            if (move > delta) {
                velocity = 0.0f;
                return delta;
            }
            return move;
        } else {
            if (move > -delta) {
                velocity = 0.0f;
                return delta;
            }
            return -move;
        }
    }
    
    /**
     * @brief Reset drift state
     */
    void reset() noexcept {
        enabled = false;
        velocity = 0.0f;
        moveAccum = 0.0f;
        lastStop = 0.0;
    }
};

//=============================================================================
// Idle Sway State
//=============================================================================

/**
 * @brief Idle view swaying parameters
 * 
 * Sinusoidal sway when v_idlescale > 0
 */
struct IdleSway {
    float scale{0.0f};  ///< v_idlescale - overall sway amount
    
    // Cycle parameters
    float yawCycle{view_idle::DEFAULT_YAW_CYCLE};
    float rollCycle{view_idle::DEFAULT_ROLL_CYCLE};
    float pitchCycle{view_idle::DEFAULT_PITCH_CYCLE};
    
    // Level parameters
    float yawLevel{view_idle::DEFAULT_YAW_LEVEL};
    float rollLevel{view_idle::DEFAULT_ROLL_LEVEL};
    float pitchLevel{view_idle::DEFAULT_PITCH_LEVEL};
    
    /**
     * @brief Check if idle sway is enabled
     */
    [[nodiscard]] bool isEnabled() const noexcept {
        return scale > 0.0f;
    }
    
    /**
     * @brief Calculate sway offsets for current time
     * 
     * @param time Current time
     * @return Array of [pitch, yaw, roll] offsets
     */
    [[nodiscard]] std::array<float, 3> calculate(double time) const noexcept {
        if (scale <= 0.0f) return {0.0f, 0.0f, 0.0f};
        
        float t = static_cast<float>(time);
        return {
            scale * std::sin(t * pitchCycle) * pitchLevel,  // Pitch
            scale * std::sin(t * yawCycle) * yawLevel,      // Yaw
            scale * std::sin(t * rollCycle) * rollLevel     // Roll
        };
    }
};

//=============================================================================
// Flash Settings (TeamFortress)
//=============================================================================

/**
 * @brief TeamFortress flash grenade constants
 */
namespace tf_flash {
    constexpr float GAMMA = 3.0f;       ///< MTFL_FLASH_GAMMA
    constexpr float CONTRAST = 3.0f;    ///< MTFL_FLASH_CONTRAST
    constexpr int OWN_PERCENT = 160;    ///< Own flash percent
    constexpr int NORMAL_PERCENT = 240; ///< Normal TF flash
    constexpr int ANGEL_PERCENT = 255;  ///< Angel TF flash
    constexpr float DURATION_OWN = 10.0f;
    constexpr float DURATION_OTHER = 20.0f;
}

/**
 * @brief Flash grenade effect state
 */
struct FlashState {
    bool flashed{false};
    double lastOwnFlashTime{0.0};
    double lastOtherFlashTime{0.0};
    float savedGamma{1.0f};
    float savedContrast{1.0f};
    
    /**
     * @brief Check if currently flashed
     */
    [[nodiscard]] bool isFlashed() const noexcept { return flashed; }
    
    /**
     * @brief Get block time based on flash type
     */
    [[nodiscard]] float blockTime() const noexcept {
        return (lastOtherFlashTime > lastOwnFlashTime) 
            ? tf_flash::DURATION_OTHER 
            : tf_flash::DURATION_OWN;
    }
    
    /**
     * @brief Reset flash state
     */
    void reset() noexcept {
        flashed = false;
        lastOwnFlashTime = 0.0;
        lastOtherFlashTime = 0.0;
    }
};

//=============================================================================
// Punch Angle State (weapon kick)
//=============================================================================

/**
 * @brief Weapon punch/kick angle state
 */
struct PunchAngle {
    float current{0.0f};    ///< Current punch angle
    float ideal{0.0f};      ///< Target punch angle (-2 = small, other = big)
    
    /**
     * @brief Update punch angle decay
     */
    void update(float frameTime) noexcept {
        if (ideal < current) {
            // Decaying towards ideal
            float rate = (ideal == -2.0f) ? 20.0f : 40.0f;  // Small vs big kick
            current -= rate * frameTime;
            
            if (current <= ideal) {
                current = ideal;
                ideal = 0.0f;
            }
        } else {
            // Returning to zero
            current += 20.0f * frameTime;
            if (current > 0.0f) {
                current = 0.0f;
            }
        }
    }
    
    /**
     * @brief Apply a punch
     */
    void apply(float angle) noexcept {
        ideal = angle;
    }
    
    /**
     * @brief Reset punch state
     */
    void reset() noexcept {
        current = ideal = 0.0f;
    }
};

//=============================================================================
// View Model State
//=============================================================================

/**
 * @brief View weapon model rendering state
 */
struct ViewModelState {
    bool drawEnabled{true};             ///< r_drawviewmodel
    bool drawWhenInvisible{false};      ///< r_drawviewmodel_invisible
    float size{1.0f};                   ///< r_viewmodelSize
    std::array<float, 3> offset{0.0f, 0.0f, 0.0f};  ///< r_viewmodeloffset
    
    int currentModel{0};                ///< Current weapon model index
    int lastFired{0};                   ///< Last fired weapon (for r_viewmodellastfired)
    int lastViewPlayer{-1};             ///< Last view player num
    
    /**
     * @brief Check if viewmodel should be drawn
     */
    [[nodiscard]] bool shouldDraw(bool isDead, bool isGib, bool isInvisible, 
                                   bool canDrawViewModel, int health) const noexcept {
        if (!drawEnabled) return false;
        if (isDead || isGib) return false;
        if (!drawWhenInvisible && isInvisible) return false;
        if (health <= 0) return false;
        if (!canDrawViewModel) return false;
        return true;
    }
};

//=============================================================================
// Complete View State
//=============================================================================

/**
 * @brief Complete view calculation state
 * 
 * Aggregates all view-related state
 */
struct ViewState {
    // Color shifts
    std::array<ViewColorShiftState, NUM_COLOR_SHIFTS> colorShifts;
    ViewBlend blend;
    
    // Movement effects
    ViewBob bob;
    ViewRoll roll;
    DamageKick damageKick;
    PitchDrift pitchDrift;
    IdleSway idleSway;
    PunchAngle punchAngle;
    
    // TF effects
    FlashState flash;
    bool concussioned{false};  ///< TF concussion grenade effect
    double hurtBlurTime{0.0};  ///< Motion blur from damage
    double faceAnimTime{0.0};  ///< Face animation time for HUD
    
    // View model
    ViewModelState viewModel;
    
    /**
     * @brief Get color shift by type
     */
    [[nodiscard]] ViewColorShiftState& shift(ColorShiftType type) noexcept {
        return colorShifts[static_cast<size_t>(type)];
    }
    
    [[nodiscard]] const ViewColorShiftState& shift(ColorShiftType type) const noexcept {
        return colorShifts[static_cast<size_t>(type)];
    }
    
    /**
     * @brief Calculate final blend from all color shifts
     */
    void calculateBlend(float cshiftPercent, bool polyblend, bool teamFortress) noexcept {
        blend.reset();
        
        for (size_t i = 0; i < NUM_COLOR_SHIFTS; ++i) {
            auto type = static_cast<ColorShiftType>(i);
            const auto& cs = colorShifts[i];
            
            if ((!cshiftPercent || !polyblend) && type != ColorShiftType::Contents) {
                continue;
            }
            
            float a2;
            if (type == ColorShiftType::Contents) {
                a2 = static_cast<float>(cs.percent) / 100.0f / 255.0f;
            } else {
                a2 = (static_cast<float>(cs.percent) * cshiftPercent / 100.0f) / 255.0f;
            }
            
            if (a2 <= 0.0f) continue;
            
            blend.addBlend(
                static_cast<float>(cs.destColor[0]) / 255.0f,
                static_cast<float>(cs.destColor[1]) / 255.0f,
                static_cast<float>(cs.destColor[2]) / 255.0f,
                a2
            );
        }
    }
    
    /**
     * @brief Update time-based effects
     */
    void updateEffects(float frameTime) noexcept {
        // Decay damage shift
        shift(ColorShiftType::Damage).decay(frameTime * 150.0f);
        
        // Decay bonus shift
        shift(ColorShiftType::Bonus).decay(frameTime * 100.0f);
        
        // Update damage kick
        damageKick.update(frameTime);
        
        // Update punch angle
        punchAngle.update(frameTime);
    }
    
    /**
     * @brief Reset all view state
     */
    void reset() noexcept {
        for (auto& cs : colorShifts) cs.reset();
        blend.reset();
        bob.reset();
        roll.reset();
        damageKick.reset();
        pitchDrift.reset();
        punchAngle.reset();
        flash.reset();
        concussioned = false;
        hurtBlurTime = 0.0;
        faceAnimTime = 0.0;
    }
};

} // namespace ezquake
