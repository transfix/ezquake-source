/**
 * @file render_state_types.hpp
 * @brief Render state type definitions for ezQuake C++20 port
 * 
 * This file contains render state types and constants extracted from:
 * - r_state.h - Rendering state enums and structures
 * - r_buffers.h - Buffer types and usage
 * - r_vao.h - Vertex Array Object types
 * - gl_local.h - OpenGL-specific constants
 * 
 * Part of ezQuake C to C++20 Conversion - Iteration 23
 */

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <array>
#include "core/math/vec3.hpp"

namespace ezquake::render {

// =============================================================================
// Depth Function (from r_state.h)
// =============================================================================

/**
 * @brief Depth comparison function for depth testing
 */
enum class DepthFunc : uint8_t {
    Less = 0,           // GL_LESS (default)
    Equal = 1,          // GL_EQUAL
    LessOrEqual = 2,    // GL_LEQUAL
    
    Count = 3
};

/**
 * @brief Get depth function name
 */
[[nodiscard]] inline std::string_view depthFuncName(DepthFunc func) noexcept {
    switch (func) {
        case DepthFunc::Less: return "less";
        case DepthFunc::Equal: return "equal";
        case DepthFunc::LessOrEqual: return "less_or_equal";
        default: return "unknown";
    }
}

// =============================================================================
// Cull Face (from r_state.h)
// =============================================================================

/**
 * @brief Face culling mode
 */
enum class CullFace : uint8_t {
    Front = 0,  // Cull front faces
    Back = 1,   // Cull back faces
    
    Count = 2
};

/**
 * @brief Get cull face name
 */
[[nodiscard]] inline std::string_view cullFaceName(CullFace face) noexcept {
    return face == CullFace::Front ? "front" : "back";
}

// =============================================================================
// Blend Function (from r_state.h)
// =============================================================================

/**
 * @brief Predefined blending modes
 */
enum class BlendFunc : uint8_t {
    Overwrite = 0,              // src=ONE, dst=ZERO
    AdditiveBlending = 1,       // src=SRC_ALPHA, dst=ONE
    PremultipliedAlpha = 2,     // src=ONE, dst=ONE_MINUS_SRC_ALPHA
    SrcDstColorDestZero = 3,
    SrcDstColorDestOne = 4,
    SrcDstColorDestSrcColor = 5,
    SrcZeroDestOneMinusSrcColor = 6,
    SrcZeroDestSrcColor = 7,
    SrcOneDestZero = 8,
    SrcZeroDestOne = 9,
    SrcOneDestOneMinusSrcColor = 10,
    
    Count = 11
};

/**
 * @brief Get blend function name
 */
[[nodiscard]] inline std::string_view blendFuncName(BlendFunc func) noexcept {
    switch (func) {
        case BlendFunc::Overwrite: return "overwrite";
        case BlendFunc::AdditiveBlending: return "additive";
        case BlendFunc::PremultipliedAlpha: return "premultiplied_alpha";
        default: return "custom";
    }
}

/**
 * @brief Check if blend function is additive
 */
[[nodiscard]] constexpr bool isAdditiveBlend(BlendFunc func) noexcept {
    return func == BlendFunc::AdditiveBlending ||
           func == BlendFunc::SrcDstColorDestOne;
}

// =============================================================================
// Polygon Mode (from r_state.h)
// =============================================================================

/**
 * @brief Polygon rasterization mode
 */
enum class PolygonMode : uint8_t {
    Fill = 0,   // Filled triangles
    Line = 1,   // Wireframe
    
    Count = 2
};

// =============================================================================
// Polygon Offset (from r_state.h)
// =============================================================================

/**
 * @brief Polygon offset presets
 */
enum class PolygonOffset : uint8_t {
    Disabled = 0,
    Standard = 1,   // For decals
    Outlines = 2,   // For outline rendering
    
    Count = 3
};

// =============================================================================
// Alpha Test (from r_state.h)
// =============================================================================

/**
 * @brief Alpha test function
 */
enum class AlphaTestFunc : uint8_t {
    Always = 0,     // Always pass
    Greater = 1,    // Pass if alpha > reference
    
    Count = 2
};

// =============================================================================
// Texture Unit Mode (from r_state.h)
// =============================================================================

/**
 * @brief Fixed-function texture environment mode
 */
enum class TexUnitMode : uint8_t {
    Blend = 0,      // GL_BLEND
    Replace = 1,    // GL_REPLACE
    Modulate = 2,   // GL_MODULATE
    Decal = 3,      // GL_DECAL
    Add = 4,        // GL_ADD
    
    Count = 5
};

/**
 * @brief Get texture unit mode name
 */
[[nodiscard]] inline std::string_view texUnitModeName(TexUnitMode mode) noexcept {
    switch (mode) {
        case TexUnitMode::Blend: return "blend";
        case TexUnitMode::Replace: return "replace";
        case TexUnitMode::Modulate: return "modulate";
        case TexUnitMode::Decal: return "decal";
        case TexUnitMode::Add: return "add";
        default: return "unknown";
    }
}

// =============================================================================
// Fog Mode (from r_state.h)
// =============================================================================

/**
 * @brief Fog state
 */
enum class FogMode : uint8_t {
    Disabled = 0,
    Enabled = 1,
    
    Count = 2
};

// =============================================================================
// Buffer Types (from r_buffers.h)
// =============================================================================

/**
 * @brief Types of GPU buffers
 */
enum class BufferType : uint8_t {
    Unknown = 0,
    Vertex = 1,     // Vertex buffer (VBO)
    Index = 2,      // Index buffer (EBO)
    Storage = 3,    // Shader storage buffer (SSBO)
    Uniform = 4,    // Uniform buffer (UBO)
    Indirect = 5,   // Indirect draw buffer
    
    Count = 6
};

/**
 * @brief Get buffer type name
 */
[[nodiscard]] inline std::string_view bufferTypeName(BufferType type) noexcept {
    switch (type) {
        case BufferType::Vertex: return "vertex";
        case BufferType::Index: return "index";
        case BufferType::Storage: return "storage";
        case BufferType::Uniform: return "uniform";
        case BufferType::Indirect: return "indirect";
        default: return "unknown";
    }
}

// =============================================================================
// Buffer Usage (from r_buffers.h)
// =============================================================================

/**
 * @brief Expected usage pattern for buffers
 */
enum class BufferUsage : uint8_t {
    Unknown = 0,
    OncePerFrame = 1,       // Filled & used once per frame (GL_STREAM_DRAW)
    ReusePerFrame = 2,      // Filled & used many times per frame (GL_DYNAMIC_DRAW)
    ReuseManyFrames = 3,    // Filled once, used many frames (GL_DYNAMIC_DRAW)
    ConstantData = 4,       // Filled once, never updated (GL_STATIC_DRAW)
    
    Count = 5
};

/**
 * @brief Get buffer usage name
 */
[[nodiscard]] inline std::string_view bufferUsageName(BufferUsage usage) noexcept {
    switch (usage) {
        case BufferUsage::OncePerFrame: return "stream";
        case BufferUsage::ReusePerFrame: return "dynamic_frame";
        case BufferUsage::ReuseManyFrames: return "dynamic";
        case BufferUsage::ConstantData: return "static";
        default: return "unknown";
    }
}

/**
 * @brief Check if buffer should be double-buffered
 */
[[nodiscard]] constexpr bool shouldDoubleBuffer(BufferUsage usage) noexcept {
    return usage == BufferUsage::OncePerFrame || usage == BufferUsage::ReusePerFrame;
}

// =============================================================================
// Buffer IDs (from r_buffers.h)
// =============================================================================

/**
 * @brief Named buffer identifiers
 */
enum class BufferId : uint8_t {
    None = 0,
    
    // Alias model buffers
    AliasModelVertex,
    AliasModelSSBO,
    AliasModelPoseData,
    AliasModelDrawIndirect,
    AliasModelData,
    
    // Brush model buffers
    BrushModelVertex,
    BrushModelIndex,
    BrushModelSurface,
    BrushModelLightStyles,
    BrushModelSurfacesToLight,
    BrushModelWorldSamplers,
    BrushModelDrawIndirect,
    BrushModelDrawData,
    
    // Sprite buffers
    SpriteVertex,
    SpriteIndex,
    
    // Misc
    InstanceNumber,
    
    // HUD buffers
    HudImageVertex,
    HudImageIndex,
    HudCircleVertex,
    
    // Post-process
    PostProcessVertex,
    
    // Frame constants
    FrameConstants,
    
    Count
};

// =============================================================================
// VAO IDs (from r_vao.h)
// =============================================================================

/**
 * @brief Vertex Array Object identifiers
 */
enum class VaoId : uint8_t {
    None = 0,
    
    // Main VAOs
    AliasModel,
    BrushModel,
    Sprites3D,
    HudCircles,
    HudImages,
    HudLines,
    HudPolygons,
    PostProcess,
    
    // Classic OpenGL VAOs
    AliasModelPowerupShell,
    BrushModelDetails,
    BrushModelLightmapPass,
    BrushModelLmUnit1,
    BrushModelSimpleTex,
    HudImagesNonGlsl,
    
    Count
};

/**
 * @brief Get VAO name
 */
[[nodiscard]] inline std::string_view vaoName(VaoId id) noexcept {
    switch (id) {
        case VaoId::AliasModel: return "aliasmodel";
        case VaoId::BrushModel: return "brushmodel";
        case VaoId::Sprites3D: return "sprites_3d";
        case VaoId::HudImages: return "hud_images";
        case VaoId::PostProcess: return "postprocess";
        default: return "other";
    }
}

// =============================================================================
// Render State IDs (from r_state.h)
// =============================================================================

/**
 * @brief Named render state presets
 * 
 * These represent complete GPU state configurations for various rendering modes.
 */
enum class RenderStateId : uint16_t {
    Null = 0,
    
    // Base states
    DefaultOpenGL,
    Default3D,
    SpritesTextured,
    Default2D,
    BrightenScreen,
    Line,
    
    // HUD states
    HudImagesGLC,
    HudImagesGLCNonGlsl,
    HudImagesAlphaTestedGLC,
    HudImagesAlphaTestedGLCNonGlsl,
    PolyBlend,
    HudImagesGLM,
    HudPolygonsGLM,
    
    // Sky states
    SkyFast,
    SkydomeBackgroundPass,
    SkydomeCloudPass,
    SkydomeSinglePass,
    SkydomeSinglePassProgram,
    SkydomeZBufferPass,
    SkydomeZBufferPassFogged,
    Skybox,
    SkyFastBmodel,
    SkydomeSinglePassBmodel,
    SkydomeCloudPassBmodel,
    SkydomeBackgroundPassBmodel,
    
    // World rendering states
    WorldTextureChain,
    WorldTextureChainFullbright,
    WorldBlendLightmaps,
    WorldCaustics,
    WorldFastOpaqueWater,
    WorldFastTranslucentWater,
    WorldOpaqueWater,
    WorldTranslucentWater,
    WorldAlphaSurfaces,
    WorldFullbrights,
    WorldLumas,
    WorldDetails,
    WorldOutline,
    
    // World material states
    WorldSingletextureGLC,
    WorldMaterialLightmap,
    WorldMaterialLightmapLuma,
    WorldMaterialLightmapFb,
    WorldMaterialFbLightmap,
    WorldMaterialLumaLightmap,
    
    // Surface states (GLM)
    AlphaSurfacesOffsetGLM,
    OpaqueSurfacesOffsetGLM,
    AlphaSurfacesGLM,
    OpaqueSurfacesGLM,
    
    // Alias model states
    AliasModelCaustics,
    AliasModelPowerupShell,
    WeaponModelPowerupShell,
    AliasModelNoTextureOpaque,
    AliasModelNoTextureTransparent,
    AliasModelNoTextureAdditive,
    AliasModelSingleTextureOpaque,
    AliasModelSingleTextureTransparent,
    AliasModelSingleTextureAdditive,
    AliasModelMultiTextureOpaque,
    AliasModelMultiTextureTransparent,
    AliasModelTransparentZPass,
    WeaponModelSingleTextureOpaque,
    WeaponModelSingleTextureTransparent,
    WeaponModelSingleTextureAdditive,
    WeaponModelMultiTextureOpaque,
    WeaponModelMultiTextureTransparent,
    WeaponModelTransparentZPass,
    AliasModelShadows,
    AliasModelOutline,
    AliasModelOutlineSpec,
    WeaponModelOutline,
    AliasModelOpaqueBatch,
    AliasModelTranslucentBatch,
    AliasModelTranslucentBatchZPass,
    AliasModelAdditiveBatch,
    
    // Post-process
    PostProcessBloom1,
    PostProcessBloom2,
    PostProcessBloomDarkenPass,
    PostProcessBloomBlurPass,
    PostProcessBloomDownsample,
    PostProcessBloomDownsampleBlend,
    PostProcessBloomRestore,
    PostProcessBloomDrawEffect,
    
    // Misc rendering
    LightBubble,
    ChatIcon,
    ParticlesClassic,
    ParticlesQmbTexturedBlood,
    ParticlesQmbTextured,
    ParticlesQmbUntextured,
    Coronas,
    
    // Drawflat modes
    DrawflatWithLightmapsGLC,
    DrawflatWithoutLightmapsGLC,
    DrawflatWithoutLightmapsUnfoggedGLC,
    
    // Effects
    FxWorldGeometry,
    
    Count
};

// =============================================================================
// Viewport State
// =============================================================================

/**
 * @brief Viewport dimensions
 */
struct Viewport {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    
    [[nodiscard]] bool isValid() const noexcept {
        return width > 0 && height > 0;
    }
    
    [[nodiscard]] float aspectRatio() const noexcept {
        if (height == 0) return 1.0f;
        return static_cast<float>(width) / static_cast<float>(height);
    }
    
    [[nodiscard]] bool contains(int px, int py) const noexcept {
        return px >= x && px < x + width && py >= y && py < y + height;
    }
    
    [[nodiscard]] bool operator==(const Viewport& other) const noexcept = default;
};

// =============================================================================
// Depth State
// =============================================================================

/**
 * @brief Depth buffer state
 */
struct DepthState {
    DepthFunc func = DepthFunc::Less;
    double nearRange = 0.0;
    double farRange = 1.0;
    bool testEnabled = true;
    bool maskEnabled = true;  // Allow writes to depth buffer
    
    [[nodiscard]] bool isDefault() const noexcept {
        return func == DepthFunc::Less &&
               nearRange == 0.0 && farRange == 1.0 &&
               testEnabled && maskEnabled;
    }
};

// =============================================================================
// Line State
// =============================================================================

/**
 * @brief Line rendering state
 */
struct LineState {
    float width = 1.0f;
    bool smooth = false;
    bool flexibleWidth = false;  // Allow non-integer widths
    
    [[nodiscard]] bool isDefault() const noexcept {
        return width == 1.0f && !smooth && !flexibleWidth;
    }
};

// =============================================================================
// Fog State
// =============================================================================

/**
 * @brief Fog rendering state
 */
struct FogState {
    FogMode mode = FogMode::Disabled;
    std::array<float, 4> color{0.0f, 0.0f, 0.0f, 1.0f};
    float density = 0.0f;
    float start = 0.0f;
    float end = 1.0f;
    
    [[nodiscard]] bool isEnabled() const noexcept {
        return mode == FogMode::Enabled && density > 0.0f;
    }
    
    void disable() noexcept {
        mode = FogMode::Disabled;
    }
    
    void enable(float r, float g, float b, float d) noexcept {
        mode = FogMode::Enabled;
        color = {r, g, b, 1.0f};
        density = d;
    }
};

// =============================================================================
// Cull State
// =============================================================================

/**
 * @brief Face culling state
 */
struct CullState {
    CullFace mode = CullFace::Back;
    bool enabled = true;
    
    void enable(CullFace face = CullFace::Back) noexcept {
        enabled = true;
        mode = face;
    }
    
    void disable() noexcept {
        enabled = false;
    }
};

// =============================================================================
// Color Mask
// =============================================================================

/**
 * @brief Color write mask
 */
struct ColorMask {
    bool r = true;
    bool g = true;
    bool b = true;
    bool a = true;
    
    [[nodiscard]] bool allEnabled() const noexcept {
        return r && g && b && a;
    }
    
    [[nodiscard]] bool allDisabled() const noexcept {
        return !r && !g && !b && !a;
    }
    
    static constexpr ColorMask all() noexcept { return {true, true, true, true}; }
    static constexpr ColorMask none() noexcept { return {false, false, false, false}; }
    static constexpr ColorMask rgbOnly() noexcept { return {true, true, true, false}; }
    static constexpr ColorMask alphaOnly() noexcept { return {false, false, false, true}; }
};

// =============================================================================
// Clear Color
// =============================================================================

/**
 * @brief Color used for glClear
 */
struct ClearColor {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;
    
    [[nodiscard]] bool isBlack() const noexcept {
        return r == 0.0f && g == 0.0f && b == 0.0f;
    }
    
    static constexpr ClearColor black() noexcept { return {0.0f, 0.0f, 0.0f, 1.0f}; }
    static constexpr ClearColor white() noexcept { return {1.0f, 1.0f, 1.0f, 1.0f}; }
    static constexpr ClearColor gray() noexcept { return {0.5f, 0.5f, 0.5f, 1.0f}; }
};

// =============================================================================
// Buffer Reference
// =============================================================================

/**
 * @brief Handle to a GPU buffer
 */
struct BufferRef {
    BufferId id = BufferId::None;
    size_t offset = 0;
    size_t size = 0;
    
    [[nodiscard]] bool isValid() const noexcept {
        return id != BufferId::None;
    }
    
    [[nodiscard]] bool isEmpty() const noexcept {
        return size == 0;
    }
};

// =============================================================================
// Render Limits
// =============================================================================

namespace render_limits {
    constexpr int MAX_TEXTURE_UNITS = 4;        // GLC texture units
    constexpr int MAX_ATTRIBUTES = 16;          // Vertex attributes
    constexpr int MAX_LIGHTMAPS = 4;            // Lightmap layers
    constexpr int MAX_DLIGHTS = 32;             // Dynamic lights
    constexpr int MAX_TEXTURE_SIZE = 16384;     // Maximum texture dimension
    constexpr int MAX_BATCH_SIZE = 65536;       // Vertices per batch
}

// =============================================================================
// Dynamic Light Type Colors
// =============================================================================

/**
 * @brief Dynamic light type identifiers
 */
enum class DLightType : uint8_t {
    Default = 0,        // Yellow-white
    Red = 1,            // Red
    Blue = 2,           // Blue
    RedBlue = 3,        // Purple (quad damage)
    Green = 4,          // Green
    
    Count
};

/**
 * @brief Get default color for dynamic light type
 */
[[nodiscard]] inline std::array<uint8_t, 3> dlightColor(DLightType type) noexcept {
    switch (type) {
        case DLightType::Red: return {255, 80, 50};
        case DLightType::Blue: return {50, 80, 255};
        case DLightType::RedBlue: return {200, 50, 200};
        case DLightType::Green: return {50, 255, 80};
        default: return {255, 255, 200};  // Default warm white
    }
}

} // namespace ezquake::render
