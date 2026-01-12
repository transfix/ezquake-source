/**
 * @file video_types.hpp
 * @brief Video and display type definitions
 *
 * This file contains C++20 implementations of types from vid.h and render.h
 * for video mode management, refresh definitions, and display settings.
 */

#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include "../math/vec3.hpp"

namespace ezquake::video {

// ============================================================================
// Video Constants
// ============================================================================

namespace video_limits {
    inline constexpr int VID_CBITS = 6;
    inline constexpr int VID_GRADES = (1 << VID_CBITS);  // 64 color grades
    inline constexpr int MAX_DISPLAY_MODES = 256;
    inline constexpr int PALETTE_SIZE = 256;
    inline constexpr int GAMMA_RAMP_SIZE = 256;
    inline constexpr float DEFAULT_ASPECT = 4.0f / 3.0f;
    inline constexpr int DEFAULT_FOV = 90;
    inline constexpr int MIN_FOV = 10;
    inline constexpr int MAX_FOV = 170;
}

// ============================================================================
// Render Flags
// ============================================================================

/**
 * @brief Entity render flags
 *
 * Based on RF_* defines from render.h.
 */
enum class RenderFlag : uint32_t {
    None = 0,
    WeaponModel = 1,
    NoShadow = 2,
    LimitLerp = 4,
    PlayerModel = 8,
    NormalEnt = 16,
    Caustics = 32,
    AlphaBlend = 64,
    AdditiveBlend = 128,
    RocketPack = 256,
    LGPack = 512,
    BehindWall = 1024,
    VWepModel = 2048,
    ForceColourMod = 4096,

    // Combination masks
    BackpackFlags = RocketPack | LGPack
};

/**
 * @brief Combine render flags
 */
[[nodiscard]] constexpr RenderFlag operator|(RenderFlag a, RenderFlag b) noexcept {
    return static_cast<RenderFlag>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

[[nodiscard]] constexpr RenderFlag operator&(RenderFlag a, RenderFlag b) noexcept {
    return static_cast<RenderFlag>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

[[nodiscard]] constexpr bool hasFlag(RenderFlag flags, RenderFlag check) noexcept {
    return (static_cast<uint32_t>(flags) & static_cast<uint32_t>(check)) != 0;
}

// ============================================================================
// Video Rectangle
// ============================================================================

/**
 * @brief Video rectangle for screen regions
 *
 * Based on vrect_t from vid.h.
 */
struct VideoRect {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;

    /**
     * @brief Check if rectangle is valid (non-zero area)
     */
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return width > 0 && height > 0;
    }

    /**
     * @brief Get rectangle area
     */
    [[nodiscard]] constexpr int area() const noexcept {
        return width * height;
    }

    /**
     * @brief Get right edge X coordinate
     */
    [[nodiscard]] constexpr int right() const noexcept {
        return x + width;
    }

    /**
     * @brief Get bottom edge Y coordinate
     */
    [[nodiscard]] constexpr int bottom() const noexcept {
        return y + height;
    }

    /**
     * @brief Get aspect ratio
     */
    [[nodiscard]] constexpr float aspect() const noexcept {
        if (height == 0) return 1.0f;
        return static_cast<float>(width) / static_cast<float>(height);
    }

    /**
     * @brief Check if point is inside rectangle
     */
    [[nodiscard]] constexpr bool contains(int px, int py) const noexcept {
        return px >= x && px < right() && py >= y && py < bottom();
    }

    /**
     * @brief Check if rectangles overlap
     */
    [[nodiscard]] constexpr bool overlaps(const VideoRect& other) const noexcept {
        return x < other.right() && right() > other.x &&
               y < other.bottom() && bottom() > other.y;
    }

    /**
     * @brief Create rectangle from dimensions
     */
    static constexpr VideoRect fromSize(int w, int h) noexcept {
        return {0, 0, w, h};
    }

    /**
     * @brief Create rectangle from position and size
     */
    static constexpr VideoRect fromBounds(int x_, int y_, int w, int h) noexcept {
        return {x_, y_, w, h};
    }

    constexpr bool operator==(const VideoRect& other) const noexcept {
        return x == other.x && y == other.y && width == other.width && height == other.height;
    }
};

// ============================================================================
// Video Definition
// ============================================================================

/**
 * @brief Global video state definition
 *
 * Based on viddef_t from vid.h.
 */
struct VideoDefinition {
    int width = 640;
    int height = 480;
    float aspect = video_limits::DEFAULT_ASPECT;
    int numPages = 1;
    bool recalcRefdef = false;
    int conWidth = 640;
    int conHeight = 480;

    /**
     * @brief Get the total pixel count
     */
    [[nodiscard]] constexpr int pixelCount() const noexcept {
        return width * height;
    }

    /**
     * @brief Get console scale factor
     */
    [[nodiscard]] constexpr float consoleScale() const noexcept {
        if (conWidth == 0) return 1.0f;
        return static_cast<float>(width) / static_cast<float>(conWidth);
    }

    /**
     * @brief Check if widescreen aspect ratio
     */
    [[nodiscard]] constexpr bool isWidescreen() const noexcept {
        return aspect > 1.5f;
    }

    /**
     * @brief Update aspect ratio from dimensions
     */
    void updateAspect() noexcept {
        if (height > 0) {
            aspect = static_cast<float>(width) / static_cast<float>(height);
        }
    }

    /**
     * @brief Reset to default values
     */
    void reset() noexcept {
        width = 640;
        height = 480;
        aspect = video_limits::DEFAULT_ASPECT;
        numPages = 1;
        recalcRefdef = false;
        conWidth = 640;
        conHeight = 480;
    }
};

// ============================================================================
// Fog Calculation Types
// ============================================================================

/**
 * @brief Fog calculation method
 *
 * Based on fogcalc_t from render.h.
 */
enum class FogCalculation : int {
    None = 0,
    Linear = 1,
    Exponential = 2,
    ExponentialSquared = 3
};

/**
 * @brief Get fog calculation name
 */
[[nodiscard]] constexpr std::string_view fogCalculationName(FogCalculation type) noexcept {
    switch (type) {
        case FogCalculation::None: return "none";
        case FogCalculation::Linear: return "linear";
        case FogCalculation::Exponential: return "exp";
        case FogCalculation::ExponentialSquared: return "exp2";
        default: return "unknown";
    }
}

// ============================================================================
// Fog Settings
// ============================================================================

/**
 * @brief Fog rendering settings
 */
struct FogSettings {
    bool enabled = false;       // fog is enabled
    bool render = false;        // fog should be rendered
    FogCalculation calculation = FogCalculation::None;
    float linearStart = 0.0f;
    float linearEnd = 1000.0f;
    float density = 0.0f;
    std::array<float, 4> color{1.0f, 1.0f, 1.0f, 1.0f};
    std::array<float, 4> skyColor{1.0f, 1.0f, 1.0f, 1.0f};
    float skyFactor = 0.0f;     // 0..1 how much sky is affected

    /**
     * @brief Check if fog should be applied
     */
    [[nodiscard]] constexpr bool shouldApply() const noexcept {
        return enabled && render && calculation != FogCalculation::None;
    }

    /**
     * @brief Calculate linear fog factor at distance
     */
    [[nodiscard]] float calculateLinearFog(float distance) const noexcept {
        if (linearEnd <= linearStart) return 0.0f;
        float factor = (linearEnd - distance) / (linearEnd - linearStart);
        return std::clamp(factor, 0.0f, 1.0f);
    }

    /**
     * @brief Calculate exponential fog factor at distance
     */
    [[nodiscard]] float calculateExpFog(float distance) const noexcept {
        return std::exp(-density * distance);
    }

    /**
     * @brief Calculate exponential squared fog factor at distance
     */
    [[nodiscard]] float calculateExp2Fog(float distance) const noexcept {
        float factor = density * distance;
        return std::exp(-factor * factor);
    }

    void reset() noexcept {
        enabled = false;
        render = false;
        calculation = FogCalculation::None;
        linearStart = 0.0f;
        linearEnd = 1000.0f;
        density = 0.0f;
        color = {1.0f, 1.0f, 1.0f, 1.0f};
        skyColor = {1.0f, 1.0f, 1.0f, 1.0f};
        skyFactor = 0.0f;
    }
};

// ============================================================================
// Refresh Definition
// ============================================================================

/**
 * @brief Core refresh/render definition
 *
 * Based on refdef_t from render.h.
 */
struct RefreshDefinition {
    VideoRect viewRect{};
    VideoRect aliasViewRect{};      // scaled version for alias models
    int rectRight = 0;
    int rectBottom = 0;
    int aliasRectRight = 0;
    int aliasRectBottom = 0;
    float rectRightEdge = 0.0f;
    float floatRectX = 0.0f;
    float floatRectY = 0.0f;
    float floatRectXAdj = 0.0f;
    float floatRectYAdj = 0.0f;
    int rectXAdjShift20 = 0;
    int rectRightAdjShift20 = 0;
    float floatRectRightAdj = 0.0f;
    float floatRectBottomAdj = 0.0f;
    float floatRectRight = 0.0f;
    float floatRectBottom = 0.0f;
    float horizontalFov = 2.0f;     // at Z=1.0, how much X is visible (2.0 = 90 degrees)
    float xOrigin = 0.5f;
    float yOrigin = 0.5f;

    math::Vec3 viewOrg{};
    math::Vec3 viewAngles{};
    int viewHeightTest = 0;

    float fovX = 90.0f;
    float fovY = 73.74f;            // ~90 degrees vertical at 4:3

    int ambientLight = 0;

    /**
     * @brief Update derived values from view rect
     */
    void updateFromViewRect() noexcept {
        rectRight = viewRect.right();
        rectBottom = viewRect.bottom();
        floatRectX = static_cast<float>(viewRect.x);
        floatRectY = static_cast<float>(viewRect.y);
        floatRectRight = static_cast<float>(rectRight);
        floatRectBottom = static_cast<float>(rectBottom);
    }

    /**
     * @brief Calculate FOV Y from FOV X and aspect ratio
     */
    void calculateFovY(float aspect) noexcept {
        if (aspect <= 0.0f) return;
        float x = static_cast<float>(viewRect.width) / std::tan(fovX * 3.14159265f / 360.0f);
        fovY = std::atan(static_cast<float>(viewRect.height) / x) * 360.0f / 3.14159265f;
    }

    void reset() noexcept {
        viewRect = {};
        aliasViewRect = {};
        rectRight = 0;
        rectBottom = 0;
        aliasRectRight = 0;
        aliasRectBottom = 0;
        rectRightEdge = 0.0f;
        floatRectX = 0.0f;
        floatRectY = 0.0f;
        floatRectXAdj = 0.0f;
        floatRectYAdj = 0.0f;
        rectXAdjShift20 = 0;
        rectRightAdjShift20 = 0;
        floatRectRightAdj = 0.0f;
        floatRectBottomAdj = 0.0f;
        floatRectRight = 0.0f;
        floatRectBottom = 0.0f;
        horizontalFov = 2.0f;
        xOrigin = 0.5f;
        yOrigin = 0.5f;
        viewOrg = {};
        viewAngles = {};
        viewHeightTest = 0;
        fovX = 90.0f;
        fovY = 73.74f;
        ambientLight = 0;
    }
};

// ============================================================================
// Extended Refresh Definition
// ============================================================================

/**
 * @brief Extended refresh definition with rendering options
 *
 * Based on refdef2_t from render.h.
 */
struct ExtendedRefreshDefinition {
    double time = 0.0;
    bool allowCheats = false;
    bool allowLumas = false;
    float maxWaterVis = 1.0f;       // water transparency: 0..1
    float maxFbSkins = 1.0f;        // max player skin brightness: 0..1

    FogSettings fog{};

    float cosTime = 0.0f;
    float sinTime = 0.0f;

    float waterAlpha = 1.0f;
    bool drawFlatFloors = false;
    bool drawFlatWalls = false;
    bool solidTexTurb = false;
    bool drawCaustics = false;
    bool drawWorldOutlines = false;
    float distanceScale = 1.0f;

    math::Vec3 outlineVpn{};
    float outlineBase = 0.0f;

    std::array<float, 4> powerupScrollParams{};

    /**
     * @brief Update time-based values
     */
    void updateTime(double newTime) noexcept {
        time = newTime;
        cosTime = static_cast<float>(std::cos(newTime));
        sinTime = static_cast<float>(std::sin(newTime));
    }

    void reset() noexcept {
        time = 0.0;
        allowCheats = false;
        allowLumas = false;
        maxWaterVis = 1.0f;
        maxFbSkins = 1.0f;
        fog.reset();
        cosTime = 0.0f;
        sinTime = 0.0f;
        waterAlpha = 1.0f;
        drawFlatFloors = false;
        drawFlatWalls = false;
        solidTexTurb = false;
        drawCaustics = false;
        drawWorldOutlines = false;
        distanceScale = 1.0f;
        outlineVpn = {};
        outlineBase = 0.0f;
        powerupScrollParams = {};
    }
};

// ============================================================================
// Color Range Types
// ============================================================================

/**
 * @brief Player color range indices
 *
 * Based on TOP_RANGE and BOTTOM_RANGE from render.h.
 */
namespace color_range {
    inline constexpr int TOP = 16;      // soldier uniform top color start
    inline constexpr int BOTTOM = 96;   // soldier uniform bottom color start
    inline constexpr int RANGE_SIZE = 16; // colors per range
}

// ============================================================================
// Display Mode
// ============================================================================

/**
 * @brief Display mode information
 */
struct DisplayMode {
    int width = 0;
    int height = 0;
    int refreshRate = 60;
    int bitsPerPixel = 32;
    bool fullscreen = false;

    /**
     * @brief Get aspect ratio
     */
    [[nodiscard]] constexpr float aspect() const noexcept {
        if (height == 0) return 1.0f;
        return static_cast<float>(width) / static_cast<float>(height);
    }

    /**
     * @brief Check if this is a widescreen mode
     */
    [[nodiscard]] constexpr bool isWidescreen() const noexcept {
        return aspect() > 1.5f;
    }

    /**
     * @brief Get total pixel count
     */
    [[nodiscard]] constexpr int pixelCount() const noexcept {
        return width * height;
    }

    /**
     * @brief Format as string "WxH@Rz"
     */
    [[nodiscard]] std::string toString() const {
        return std::to_string(width) + "x" + std::to_string(height) +
               "@" + std::to_string(refreshRate) + "Hz";
    }

    constexpr bool operator==(const DisplayMode& other) const noexcept {
        return width == other.width && height == other.height &&
               refreshRate == other.refreshRate && bitsPerPixel == other.bitsPerPixel;
    }

    constexpr bool operator<(const DisplayMode& other) const noexcept {
        if (pixelCount() != other.pixelCount()) return pixelCount() < other.pixelCount();
        return refreshRate < other.refreshRate;
    }
};

// ============================================================================
// Gamma Settings
// ============================================================================

/**
 * @brief Gamma correction settings
 */
struct GammaSettings {
    float gamma = 1.0f;
    float contrast = 1.0f;
    bool hardwareGamma = true;
    std::array<uint16_t, video_limits::GAMMA_RAMP_SIZE> redRamp{};
    std::array<uint16_t, video_limits::GAMMA_RAMP_SIZE> greenRamp{};
    std::array<uint16_t, video_limits::GAMMA_RAMP_SIZE> blueRamp{};

    /**
     * @brief Calculate gamma-corrected value
     */
    [[nodiscard]] constexpr float applyGamma(float value) const noexcept {
        return std::pow(value, 1.0f / gamma) * contrast;
    }

    /**
     * @brief Generate gamma ramp
     */
    void generateRamp() noexcept {
        for (size_t i = 0; i < video_limits::GAMMA_RAMP_SIZE; ++i) {
            float normalized = static_cast<float>(i) / 255.0f;
            float corrected = applyGamma(normalized);
            corrected = std::clamp(corrected, 0.0f, 1.0f);
            uint16_t value = static_cast<uint16_t>(corrected * 65535.0f);
            redRamp[i] = value;
            greenRamp[i] = value;
            blueRamp[i] = value;
        }
    }

    void reset() noexcept {
        gamma = 1.0f;
        contrast = 1.0f;
        hardwareGamma = true;
        for (size_t i = 0; i < video_limits::GAMMA_RAMP_SIZE; ++i) {
            uint16_t value = static_cast<uint16_t>((i << 8) | i);
            redRamp[i] = value;
            greenRamp[i] = value;
            blueRamp[i] = value;
        }
    }
};

// ============================================================================
// VSync Settings
// ============================================================================

/**
 * @brief VSync mode enumeration
 */
enum class VSyncMode : int {
    Off = 0,
    On = 1,
    Adaptive = -1   // Adaptive VSync (if supported)
};

/**
 * @brief VSync configuration
 */
struct VSyncConfig {
    VSyncMode mode = VSyncMode::On;
    bool lagFix = false;       // VSync lag fix enabled
    bool lagEnabled = false;   // VSync lag currently active

    /**
     * @brief Check if VSync is active
     */
    [[nodiscard]] constexpr bool isActive() const noexcept {
        return mode != VSyncMode::Off;
    }
};

// ============================================================================
// Window State
// ============================================================================

/**
 * @brief Window state information
 */
struct WindowState {
    bool minimized = false;
    bool focused = true;
    bool fullscreen = false;
    bool cursorCaptured = false;
    bool cursorVisible = true;
    int posX = 0;
    int posY = 0;
    int width = 640;
    int height = 480;

    /**
     * @brief Check if window is active (focused and not minimized)
     */
    [[nodiscard]] constexpr bool isActive() const noexcept {
        return focused && !minimized;
    }

    void reset() noexcept {
        minimized = false;
        focused = true;
        fullscreen = false;
        cursorCaptured = false;
        cursorVisible = true;
        posX = 0;
        posY = 0;
        width = 640;
        height = 480;
    }
};

// ============================================================================
// Custom Model Color
// ============================================================================

/**
 * @brief Custom model color configuration
 *
 * Based on custom_model_color_t from render.h.
 */
struct CustomModelColor {
    std::array<float, 4> color{1.0f, 1.0f, 1.0f, 1.0f};
    bool fullbright = false;
    int modelHint = 0;
    RenderFlag renderFlags = RenderFlag::None;
    bool disableTexturing = false;

    /**
     * @brief Check if custom coloring is active
     */
    [[nodiscard]] constexpr bool isActive() const noexcept {
        return color[0] != 1.0f || color[1] != 1.0f ||
               color[2] != 1.0f || fullbright || disableTexturing;
    }
};

// ============================================================================
// Video State Manager
// ============================================================================

/**
 * @brief Complete video subsystem state
 */
struct VideoState {
    VideoDefinition definition{};
    RefreshDefinition refdef{};
    ExtendedRefreshDefinition refdef2{};
    std::optional<DisplayMode> currentMode{};
    GammaSettings gamma{};
    VSyncConfig vsync{};
    WindowState window{};
    bool initialized = false;

    /**
     * @brief Update video state for new resolution
     */
    void setResolution(int width, int height) noexcept {
        definition.width = width;
        definition.height = height;
        definition.updateAspect();
        definition.recalcRefdef = true;
    }

    /**
     * @brief Mark refdef as needing recalculation
     */
    void invalidateRefdef() noexcept {
        definition.recalcRefdef = true;
    }

    void reset() noexcept {
        definition.reset();
        refdef.reset();
        refdef2.reset();
        currentMode = std::nullopt;
        gamma.reset();
        vsync = VSyncConfig{};
        window.reset();
        initialized = false;
    }
};

// ============================================================================
// Factory Functions
// ============================================================================

/**
 * @brief Create common display modes
 */
[[nodiscard]] inline std::array<DisplayMode, 5> createCommonModes() noexcept {
    return {{
        {640, 480, 60, 32, false},
        {1280, 720, 60, 32, false},
        {1920, 1080, 60, 32, false},
        {2560, 1440, 60, 32, false},
        {3840, 2160, 60, 32, false}
    }};
}

/**
 * @brief Calculate FOV adjustment for non-4:3 aspect ratios
 */
[[nodiscard]] constexpr float calcFovAdjustment(float baseFov, float aspect) noexcept {
    // Standard Quake assumes 4:3, adjust for wider
    constexpr float BASE_ASPECT = 4.0f / 3.0f;
    if (aspect <= BASE_ASPECT) return baseFov;

    // Hor+ FOV adjustment
    float fovRad = baseFov * 3.14159265f / 180.0f;
    float ratio = aspect / BASE_ASPECT;
    float adjustedRad = 2.0f * std::atan(std::tan(fovRad / 2.0f) * ratio);
    return adjustedRad * 180.0f / 3.14159265f;
}

} // namespace ezquake::video
