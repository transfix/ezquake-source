/**
 * @file ruleset_types.hpp
 * @brief Ruleset and tournament mode types for competitive play
 * 
 * Modern C++20 implementation of ruleset types for competitive play modes
 * including smackdown, thunderdome, qcon, MTFL, and smackdrive.
 * 
 * Rulesets control what features are allowed/restricted during competitive play
 * to ensure fair competition.
 * 
 * @note Iteration 36: Ruleset Types
 */

#pragma once

#include <array>
#include <cstdint>
#include <string_view>
#include <optional>
#include <bitset>

namespace ezquake::ruleset {

//=============================================================================
// Ruleset Constants
//=============================================================================

namespace ruleset_limits {
    // MTFL ruleset constants
    inline constexpr float MTFL_FLASH_GAMMA = 0.55f;
    inline constexpr float MTFL_FLASH_CONTRAST = 1.0f;
    
    // Max FPS limits by ruleset
    inline constexpr float DEFAULT_MAX_FPS = 0.0f;      // No limit
    inline constexpr float SMACKDOWN_MAX_FPS = 500.0f;
    inline constexpr float THUNDERDOME_MAX_FPS = 500.0f;
    inline constexpr float QCON_MAX_FPS = 500.0f;
    inline constexpr float MTFL_MAX_FPS = 77.0f;
    inline constexpr float SMACKDRIVE_MAX_FPS = 500.0f;
    
    // Sequential wait commands limit
    inline constexpr int DEFAULT_MAX_WAITS = 0;         // No limit
    inline constexpr int COMPETITIVE_MAX_WAITS = 5;
    
    // Model outline scale
    inline constexpr float DEFAULT_OUTLINE_SCALE = 1.0f;
    inline constexpr float MTFL_OUTLINE_SCALE = 0.5f;
}

//=============================================================================
// Ruleset Enumeration
//=============================================================================

/**
 * @brief Ruleset mode for competitive play
 * 
 * Converted from: rulesets.h ruleset_t
 */
enum class RulesetMode : uint8_t {
    Default = 0,    ///< No restrictions (casual play)
    Smackdown,      ///< Smackdown tournament ruleset
    Thunderdome,    ///< Thunderdome tournament ruleset
    QCon,           ///< QuakeCon tournament ruleset
    MTFL,           ///< Match Tournament Federation League ruleset
    Smackdrive,     ///< Smackdrive tournament ruleset
    
    Count
};

/**
 * @brief Get human-readable ruleset name
 */
[[nodiscard]] constexpr std::string_view rulesetName(RulesetMode mode) noexcept {
    switch (mode) {
        case RulesetMode::Default:    return "default";
        case RulesetMode::Smackdown:  return "smackdown";
        case RulesetMode::Thunderdome: return "thunderdome";
        case RulesetMode::QCon:       return "qcon";
        case RulesetMode::MTFL:       return "mtfl";
        case RulesetMode::Smackdrive: return "smackdrive";
        default:                      return "unknown";
    }
}

/**
 * @brief Parse ruleset from string
 */
[[nodiscard]] inline std::optional<RulesetMode> parseRuleset(std::string_view name) noexcept {
    if (name == "default" || name.empty()) return RulesetMode::Default;
    if (name == "smackdown") return RulesetMode::Smackdown;
    if (name == "thunderdome") return RulesetMode::Thunderdome;
    if (name == "qcon") return RulesetMode::QCon;
    if (name == "mtfl") return RulesetMode::MTFL;
    if (name == "smackdrive") return RulesetMode::Smackdrive;
    return std::nullopt;
}

/**
 * @brief Check if ruleset is competitive (has restrictions)
 */
[[nodiscard]] constexpr bool isCompetitive(RulesetMode mode) noexcept {
    return mode != RulesetMode::Default;
}

//=============================================================================
// Restriction Flags
//=============================================================================

/**
 * @brief Feature restriction flags for rulesets
 */
enum class RestrictionFlag : uint32_t {
    Triggers = 0,       ///< Restrict message triggers
    Packet,             ///< Restrict packet manipulation
    Particles,          ///< Restrict particle effects
    Exec,               ///< Restrict exec command
    IPC,                ///< Restrict inter-process communication
    SetCalc,            ///< Restrict set_calc
    SetEval,            ///< Restrict set_eval
    SetEx,              ///< Restrict set_ex
    TCL,                ///< Restrict TCL scripting
    Timerefresh,        ///< Restrict timerefresh (benchmark)
    NoShadows,          ///< Restrict no shadows
    HudPicChange,       ///< Block HUD picture changes
    PolygonOffset,      ///< Restrict polygon offset
    AlternateModels,    ///< Restrict alternate models
    ModelOutline,       ///< Restrict model outline
    EdgeOutline,        ///< Restrict edge outline
    ExternalTextures,   ///< Restrict external textures
    LumaTextures,       ///< Restrict luma textures
    PowerupShell,       ///< Restrict powerup shell effects
    SimpleTextures,     ///< Restrict simple textures
    HardwareGamma,      ///< Restrict hardware gamma control
    Scripts,            ///< Restrict scripting
    FullbrightSkins,    ///< Restrict fullbright skins
    FakeShaft,          ///< Restrict fake shaft
    DelayPacket,        ///< Restrict packet delay
    IDrive,             ///< Restrict iDrive movement
    ToggleWhenFlashed,  ///< Restrict toggle when flashed
    FullbrightModels,   ///< Restrict fullbright models
    PlayerCountMacros,  ///< Restrict player count macros
    ConsoleLogging,     ///< Restrict console logging
    RollAngle,          ///< Restrict roll angle
    PlaySound,          ///< Restrict play command
    
    Count
};

/**
 * @brief Bitset for tracking active restrictions
 */
using RestrictionSet = std::bitset<static_cast<size_t>(RestrictionFlag::Count)>;

//=============================================================================
// Cvar Locking Types
//=============================================================================

/**
 * @brief Locked cvar value (must be exactly this value)
 */
struct LockedCvar {
    std::string_view name;
    std::string_view value;
    
    constexpr LockedCvar(std::string_view n, std::string_view v) : name(n), value(v) {}
};

/**
 * @brief Limited cvar with maximum value
 */
struct LimitedCvarMax {
    std::string_view name;
    float maxValue;
    
    constexpr LimitedCvarMax(std::string_view n, float max) : name(n), maxValue(max) {}
};

/**
 * @brief Limited cvar with minimum value
 */
struct LimitedCvarMin {
    std::string_view name;
    float minValue;
    
    constexpr LimitedCvarMin(std::string_view n, float min) : name(n), minValue(min) {}
};

//=============================================================================
// Ruleset Configuration
//=============================================================================

/**
 * @brief Complete ruleset configuration
 */
struct RulesetConfig {
    RulesetMode mode = RulesetMode::Default;
    RestrictionSet restrictions{};
    
    float maxFPS = 0.0f;                    ///< Max FPS (0 = unlimited)
    int maxSequentialWaits = 0;             ///< Max wait commands (0 = unlimited)
    float modelOutlineScale = 1.0f;
    float rollAngleLimit = 0.0f;            ///< Max roll angle
    
    // MTFL specific
    float flashGamma = 1.0f;
    float flashContrast = 1.0f;
    
    /**
     * @brief Check if a restriction is active
     */
    [[nodiscard]] bool isRestricted(RestrictionFlag flag) const noexcept {
        return restrictions.test(static_cast<size_t>(flag));
    }
    
    /**
     * @brief Enable a restriction
     */
    void restrict(RestrictionFlag flag) noexcept {
        restrictions.set(static_cast<size_t>(flag));
    }
    
    /**
     * @brief Disable a restriction
     */
    void allow(RestrictionFlag flag) noexcept {
        restrictions.reset(static_cast<size_t>(flag));
    }
    
    /**
     * @brief Check if triggers are restricted
     */
    [[nodiscard]] bool restrictsTriggers() const noexcept {
        return isRestricted(RestrictionFlag::Triggers);
    }
    
    /**
     * @brief Check if particles are restricted
     */
    [[nodiscard]] bool restrictsParticles() const noexcept {
        return isRestricted(RestrictionFlag::Particles);
    }
    
    /**
     * @brief Check if exec is restricted
     */
    [[nodiscard]] bool restrictsExec() const noexcept {
        return isRestricted(RestrictionFlag::Exec);
    }
    
    /**
     * @brief Check if timerefresh is allowed
     */
    [[nodiscard]] bool allowsTimerefresh() const noexcept {
        return !isRestricted(RestrictionFlag::Timerefresh);
    }
    
    /**
     * @brief Check if no shadows is allowed
     */
    [[nodiscard]] bool allowsNoShadows() const noexcept {
        return !isRestricted(RestrictionFlag::NoShadows);
    }
    
    /**
     * @brief Check if hardware gamma is allowed to be disabled
     */
    [[nodiscard]] bool allowsNoHardwareGamma() const noexcept {
        return !isRestricted(RestrictionFlag::HardwareGamma);
    }
    
    /**
     * @brief Check if FPS is capped
     */
    [[nodiscard]] constexpr bool hasFPSCap() const noexcept {
        return maxFPS > 0.0f;
    }
    
    /**
     * @brief Get effective FPS (applies cap if set)
     */
    [[nodiscard]] constexpr float effectiveFPS(float requestedFPS) const noexcept {
        if (maxFPS <= 0.0f) return requestedFPS;
        return (requestedFPS > maxFPS) ? maxFPS : requestedFPS;
    }
    
    /**
     * @brief Check if console logging is allowed
     */
    [[nodiscard]] bool canLogConsole() const noexcept {
        return !isRestricted(RestrictionFlag::ConsoleLogging);
    }
};

//=============================================================================
// Ruleset Factory Functions
//=============================================================================

/**
 * @brief Create default ruleset configuration (no restrictions)
 */
[[nodiscard]] inline RulesetConfig createDefaultRuleset() noexcept {
    RulesetConfig config;
    config.mode = RulesetMode::Default;
    return config;
}

/**
 * @brief Create common competitive base restrictions
 */
inline void applyCompetitiveBase(RulesetConfig& config) noexcept {
    config.restrict(RestrictionFlag::Triggers);
    config.restrict(RestrictionFlag::Packet);
    config.restrict(RestrictionFlag::Exec);
    config.restrict(RestrictionFlag::IPC);
    config.restrict(RestrictionFlag::SetCalc);
    config.restrict(RestrictionFlag::SetEval);
    config.restrict(RestrictionFlag::SetEx);
    config.restrict(RestrictionFlag::TCL);
    config.restrict(RestrictionFlag::Timerefresh);
    config.maxSequentialWaits = ruleset_limits::COMPETITIVE_MAX_WAITS;
}

/**
 * @brief Create smackdown ruleset configuration
 */
[[nodiscard]] inline RulesetConfig createSmackdownRuleset() noexcept {
    RulesetConfig config;
    config.mode = RulesetMode::Smackdown;
    config.maxFPS = ruleset_limits::SMACKDOWN_MAX_FPS;
    applyCompetitiveBase(config);
    return config;
}

/**
 * @brief Create thunderdome ruleset configuration
 */
[[nodiscard]] inline RulesetConfig createThunderdomeRuleset() noexcept {
    RulesetConfig config;
    config.mode = RulesetMode::Thunderdome;
    config.maxFPS = ruleset_limits::THUNDERDOME_MAX_FPS;
    applyCompetitiveBase(config);
    return config;
}

/**
 * @brief Create QuakeCon ruleset configuration
 */
[[nodiscard]] inline RulesetConfig createQConRuleset() noexcept {
    RulesetConfig config;
    config.mode = RulesetMode::QCon;
    config.maxFPS = ruleset_limits::QCON_MAX_FPS;
    applyCompetitiveBase(config);
    return config;
}

/**
 * @brief Create MTFL ruleset configuration
 */
[[nodiscard]] inline RulesetConfig createMTFLRuleset() noexcept {
    RulesetConfig config;
    config.mode = RulesetMode::MTFL;
    config.maxFPS = ruleset_limits::MTFL_MAX_FPS;
    config.flashGamma = ruleset_limits::MTFL_FLASH_GAMMA;
    config.flashContrast = ruleset_limits::MTFL_FLASH_CONTRAST;
    config.modelOutlineScale = ruleset_limits::MTFL_OUTLINE_SCALE;
    
    applyCompetitiveBase(config);
    
    // MTFL has additional restrictions
    config.restrict(RestrictionFlag::Particles);
    config.restrict(RestrictionFlag::NoShadows);
    config.restrict(RestrictionFlag::HudPicChange);
    config.restrict(RestrictionFlag::PolygonOffset);
    config.restrict(RestrictionFlag::AlternateModels);
    config.restrict(RestrictionFlag::ModelOutline);
    config.restrict(RestrictionFlag::EdgeOutline);
    config.restrict(RestrictionFlag::ExternalTextures);
    config.restrict(RestrictionFlag::LumaTextures);
    config.restrict(RestrictionFlag::PowerupShell);
    config.restrict(RestrictionFlag::SimpleTextures);
    config.restrict(RestrictionFlag::HardwareGamma);
    config.restrict(RestrictionFlag::FullbrightSkins);
    config.restrict(RestrictionFlag::FullbrightModels);
    config.restrict(RestrictionFlag::ToggleWhenFlashed);
    
    return config;
}

/**
 * @brief Create smackdrive ruleset configuration
 */
[[nodiscard]] inline RulesetConfig createSmackdriveRuleset() noexcept {
    RulesetConfig config;
    config.mode = RulesetMode::Smackdrive;
    config.maxFPS = ruleset_limits::SMACKDRIVE_MAX_FPS;
    applyCompetitiveBase(config);
    return config;
}

/**
 * @brief Create ruleset configuration by mode
 */
[[nodiscard]] inline RulesetConfig createRuleset(RulesetMode mode) noexcept {
    switch (mode) {
        case RulesetMode::Smackdown:  return createSmackdownRuleset();
        case RulesetMode::Thunderdome: return createThunderdomeRuleset();
        case RulesetMode::QCon:       return createQConRuleset();
        case RulesetMode::MTFL:       return createMTFLRuleset();
        case RulesetMode::Smackdrive: return createSmackdriveRuleset();
        default:                      return createDefaultRuleset();
    }
}

//=============================================================================
// Restriction Query Helpers
//=============================================================================

/**
 * @brief Get reason text for MTFL ban
 */
[[nodiscard]] constexpr std::string_view getMTFLBanReason() noexcept {
    return "banned by MTFL ruleset";
}

/**
 * @brief Check if model is allowed in ruleset
 */
struct ModelRestriction {
    bool allowFullbright = true;
    bool allowOutline = true;
    bool allowExternalTexture = true;
    bool allowLumaTexture = true;
    bool allowSimpleTexture = true;
    bool allowPowerupShell = true;
    
    [[nodiscard]] static ModelRestriction fromConfig(const RulesetConfig& config) noexcept {
        ModelRestriction r;
        r.allowFullbright = !config.isRestricted(RestrictionFlag::FullbrightModels);
        r.allowOutline = !config.isRestricted(RestrictionFlag::ModelOutline);
        r.allowExternalTexture = !config.isRestricted(RestrictionFlag::ExternalTextures);
        r.allowLumaTexture = !config.isRestricted(RestrictionFlag::LumaTextures);
        r.allowSimpleTexture = !config.isRestricted(RestrictionFlag::SimpleTextures);
        r.allowPowerupShell = !config.isRestricted(RestrictionFlag::PowerupShell);
        return r;
    }
};

/**
 * @brief Cvar change validation result
 */
enum class CvarChangeResult : uint8_t {
    Allowed,            ///< Change is allowed
    Blocked,            ///< Change is blocked by ruleset
    Clamped,            ///< Value was clamped to allowed range
};

/**
 * @brief Ruleset-aware cvar change validator
 */
struct CvarValidator {
    const RulesetConfig& config;
    
    explicit CvarValidator(const RulesetConfig& cfg) : config(cfg) {}
    
    /**
     * @brief Validate fullbright skins value
     */
    [[nodiscard]] CvarChangeResult validateFullbrightSkins(float& value) const noexcept {
        if (!config.isRestricted(RestrictionFlag::FullbrightSkins)) {
            return CvarChangeResult::Allowed;
        }
        if (value > 0.0f) {
            value = 0.0f;
            return CvarChangeResult::Clamped;
        }
        return CvarChangeResult::Allowed;
    }
    
    /**
     * @brief Validate fakeshaft value
     */
    [[nodiscard]] CvarChangeResult validateFakeShaft(float& value) const noexcept {
        if (!config.isRestricted(RestrictionFlag::FakeShaft)) {
            return CvarChangeResult::Allowed;
        }
        if (value != 0.0f) {
            value = 0.0f;
            return CvarChangeResult::Clamped;
        }
        return CvarChangeResult::Allowed;
    }
    
    /**
     * @brief Validate delay packet value
     */
    [[nodiscard]] CvarChangeResult validateDelayPacket(float& value) const noexcept {
        if (!config.isRestricted(RestrictionFlag::DelayPacket)) {
            return CvarChangeResult::Allowed;
        }
        if (value != 0.0f) {
            value = 0.0f;
            return CvarChangeResult::Clamped;
        }
        return CvarChangeResult::Allowed;
    }
};

//=============================================================================
// Ruleset State
//=============================================================================

/**
 * @brief Mutable ruleset state for runtime management
 */
class RulesetState {
public:
    RulesetState() = default;
    explicit RulesetState(RulesetMode mode) : config_(createRuleset(mode)) {}
    
    /**
     * @brief Get current mode
     */
    [[nodiscard]] RulesetMode mode() const noexcept { return config_.mode; }
    
    /**
     * @brief Get current configuration
     */
    [[nodiscard]] const RulesetConfig& config() const noexcept { return config_; }
    
    /**
     * @brief Get mutable configuration
     */
    [[nodiscard]] RulesetConfig& config() noexcept { return config_; }
    
    /**
     * @brief Switch to a different ruleset
     */
    void setRuleset(RulesetMode mode) noexcept {
        config_ = createRuleset(mode);
    }
    
    /**
     * @brief Get ruleset name
     */
    [[nodiscard]] std::string_view name() const noexcept {
        return rulesetName(config_.mode);
    }
    
    /**
     * @brief Check if currently in competitive mode
     */
    [[nodiscard]] bool isCompetitive() const noexcept {
        return ezquake::ruleset::isCompetitive(config_.mode);
    }
    
    /**
     * @brief Get max FPS for current ruleset
     */
    [[nodiscard]] float maxFPS() const noexcept {
        return config_.maxFPS;
    }
    
    /**
     * @brief Create validator for this ruleset
     */
    [[nodiscard]] CvarValidator validator() const noexcept {
        return CvarValidator(config_);
    }
    
private:
    RulesetConfig config_{};
};

/**
 * @brief Ruleset statistics for debugging/info
 */
struct RulesetStats {
    RulesetMode currentMode = RulesetMode::Default;
    int activeRestrictions = 0;
    float effectiveMaxFPS = 0.0f;
    bool isTournamentMode = false;
    
    [[nodiscard]] static RulesetStats from(const RulesetConfig& config) noexcept {
        RulesetStats stats;
        stats.currentMode = config.mode;
        stats.activeRestrictions = static_cast<int>(config.restrictions.count());
        stats.effectiveMaxFPS = config.maxFPS;
        stats.isTournamentMode = isCompetitive(config.mode);
        return stats;
    }
};

} // namespace ezquake::ruleset
