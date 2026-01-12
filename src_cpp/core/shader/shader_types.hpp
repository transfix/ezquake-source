/**
 * @file shader_types.hpp
 * @brief GPU shader and program types for ezQuake C++20 port
 * 
 * Provides modern C++ equivalents of r_program.h shader management types.
 */

#pragma once

#include <array>
#include <cstdint>
#include <string_view>
#include <optional>
#include <bitset>

namespace ezquake::shader {

// =============================================================================
// Constants
// =============================================================================

namespace limits {
    inline constexpr size_t MAX_PROGRAMS = 32;
    inline constexpr size_t MAX_UNIFORMS = 128;
    inline constexpr size_t MAX_ATTRIBUTES = 16;
    inline constexpr size_t MAX_SUB_PROGRAMS = 4;
    inline constexpr size_t MAX_SHADER_SOURCES = 8;
    inline constexpr size_t MAX_UNIFORM_NAME_LEN = 64;
} // namespace limits

// =============================================================================
// ShaderType - Type of shader stage
// =============================================================================

/**
 * Type of shader stage in the pipeline.
 */
enum class ShaderType : uint8_t {
    Vertex = 0,
    Fragment,
    Geometry,
    Compute,
    TessControl,
    TessEval,
    
    Count
};

/**
 * Get display name for shader type.
 */
[[nodiscard]] constexpr const char* getShaderTypeName(ShaderType type) noexcept {
    switch (type) {
        case ShaderType::Vertex: return "vertex";
        case ShaderType::Fragment: return "fragment";
        case ShaderType::Geometry: return "geometry";
        case ShaderType::Compute: return "compute";
        case ShaderType::TessControl: return "tess_control";
        case ShaderType::TessEval: return "tess_eval";
        default: return "unknown";
    }
}

/**
 * Get GLSL shader extension for type.
 */
[[nodiscard]] constexpr const char* getShaderExtension(ShaderType type) noexcept {
    switch (type) {
        case ShaderType::Vertex: return ".vert";
        case ShaderType::Fragment: return ".frag";
        case ShaderType::Geometry: return ".geom";
        case ShaderType::Compute: return ".comp";
        case ShaderType::TessControl: return ".tesc";
        case ShaderType::TessEval: return ".tese";
        default: return ".glsl";
    }
}

// =============================================================================
// ProgramId - Shader program identifiers
// =============================================================================

/**
 * Shader program identifiers.
 * Original: r_program_id from r_program.h
 */
enum class ProgramId : uint8_t {
    None = 0,
    
    // Modern programs
    AliasModel,
    BrushModel,
    Sprite3D,
    HudImages,
    HudCircles,
    PostProcess,
    
    // GLC (Classic OpenGL) programs
    PostProcessGlc,
    SkyGlc,
    TurbGlc,
    CausticsGlc,
    AliasModelStdGlc,
    AliasModelShellGlc,
    AliasModelShadowGlc,
    AliasModelOutlineGlc,
    WorldDrawflatGlc,
    WorldTexturedGlc,
    WorldSecondpassGlc,
    SpritesGlc,
    HudImagesGlc,
    
    // Compute shaders
    LightmapCompute,
    
    // FX programs
    FxWorldGeometry,
    BrushModelAlphaTested,
    Simple,
    Simple3D,
    
    Count
};

/**
 * Get display name for program.
 */
[[nodiscard]] constexpr const char* getProgramName(ProgramId id) noexcept {
    switch (id) {
        case ProgramId::None: return "none";
        case ProgramId::AliasModel: return "aliasmodel";
        case ProgramId::BrushModel: return "brushmodel";
        case ProgramId::Sprite3D: return "sprite3d";
        case ProgramId::HudImages: return "hud_images";
        case ProgramId::HudCircles: return "hud_circles";
        case ProgramId::PostProcess: return "post_process";
        case ProgramId::PostProcessGlc: return "post_process_glc";
        case ProgramId::SkyGlc: return "sky_glc";
        case ProgramId::TurbGlc: return "turb_glc";
        case ProgramId::CausticsGlc: return "caustics_glc";
        case ProgramId::AliasModelStdGlc: return "aliasmodel_std_glc";
        case ProgramId::AliasModelShellGlc: return "aliasmodel_shell_glc";
        case ProgramId::AliasModelShadowGlc: return "aliasmodel_shadow_glc";
        case ProgramId::AliasModelOutlineGlc: return "aliasmodel_outline_glc";
        case ProgramId::WorldDrawflatGlc: return "world_drawflat_glc";
        case ProgramId::WorldTexturedGlc: return "world_textured_glc";
        case ProgramId::WorldSecondpassGlc: return "world_secondpass_glc";
        case ProgramId::SpritesGlc: return "sprites_glc";
        case ProgramId::HudImagesGlc: return "hud_images_glc";
        case ProgramId::LightmapCompute: return "lightmap_compute";
        case ProgramId::FxWorldGeometry: return "fx_world_geometry";
        case ProgramId::BrushModelAlphaTested: return "brushmodel_alphatested";
        case ProgramId::Simple: return "simple";
        case ProgramId::Simple3D: return "simple3d";
        default: return "unknown";
    }
}

/**
 * Check if program is a GLC (classic) variant.
 */
[[nodiscard]] constexpr bool isGlcProgram(ProgramId id) noexcept {
    switch (id) {
        case ProgramId::PostProcessGlc:
        case ProgramId::SkyGlc:
        case ProgramId::TurbGlc:
        case ProgramId::CausticsGlc:
        case ProgramId::AliasModelStdGlc:
        case ProgramId::AliasModelShellGlc:
        case ProgramId::AliasModelShadowGlc:
        case ProgramId::AliasModelOutlineGlc:
        case ProgramId::WorldDrawflatGlc:
        case ProgramId::WorldTexturedGlc:
        case ProgramId::WorldSecondpassGlc:
        case ProgramId::SpritesGlc:
        case ProgramId::HudImagesGlc:
            return true;
        default:
            return false;
    }
}

/**
 * Check if program is a compute shader.
 */
[[nodiscard]] constexpr bool isComputeProgram(ProgramId id) noexcept {
    return id == ProgramId::LightmapCompute;
}

// =============================================================================
// UniformId - Shader uniform identifiers
// =============================================================================

/**
 * Shader uniform identifiers.
 * Original: r_program_uniform_id from r_program.h
 */
enum class UniformId : uint8_t {
    // AliasModel
    AliasModelDrawMode = 0,
    
    // BrushModel
    BrushModelOutlines,
    BrushModelSampler,
    
    // Sprite3D
    Sprite3DAlphaTest,
    
    // HUD
    HudCircleMatrix,
    HudCircleColor,
    
    // Post-process GLC
    PostProcessGlcGamma,
    PostProcessGlcBase,
    PostProcessGlcOverlay,
    PostProcessGlcVBlend,
    PostProcessGlcContrast,
    PostProcessGlcInvWidth,
    PostProcessGlcInvHeight,
    
    // Sky GLC
    SkyGlcCameraPosition,
    SkyGlcSpeedScale,
    SkyGlcSpeedScale2,
    SkyGlcSkyTex,
    SkyGlcSkyWind,
    SkyGlcSkyDomeTex,
    SkyGlcSkyDomeCloudTex,
    SkyGlcFogSkyFogMix,
    
    // Turb GLC
    TurbGlcTexSampler,
    TurbGlcTime,
    TurbGlcAlpha,
    TurbGlcColor,
    
    // Caustics GLC
    CausticsGlcTexSampler,
    CausticsGlcTime,
    
    // AliasModel GLC
    AliasModelStdGlcAngleVector,
    AliasModelStdGlcShadeLight,
    AliasModelStdGlcAmbientLight,
    AliasModelStdGlcTexSampler,
    AliasModelStdGlcTextureEnabled,
    AliasModelStdGlcMinLumaMix,
    AliasModelStdGlcTime,
    AliasModelStdGlcCausticEffects,
    AliasModelStdGlcLerpFraction,
    AliasModelStdGlcCausticsSampler,
    AliasModelShellGlcBaseColor1,
    AliasModelShellGlcBaseColor2,
    AliasModelShellGlcTexSampler,
    AliasModelShellGlcLerpFraction,
    AliasModelShellGlcScroll,
    AliasModelShadowGlcLerpFraction,
    AliasModelShadowGlcShadeVector,
    AliasModelShadowGlcLHeight,
    
    // World GLC
    WorldDrawflatGlcWallColor,
    WorldDrawflatGlcFloorColor,
    WorldDrawflatGlcSkyColor,
    WorldDrawflatGlcWaterColor,
    WorldDrawflatGlcSlimeColor,
    WorldDrawflatGlcLavaColor,
    WorldDrawflatGlcTeleColor,
    WorldTexturedGlcLightmapSampler,
    WorldTexturedGlcTexSampler,
    WorldTexturedGlcLumaSampler,
    WorldTexturedGlcCausticSampler,
    WorldTexturedGlcDetailSampler,
    WorldTexturedGlcTime,
    WorldTexturedGlcLumaScale,
    WorldTexturedGlcFbScale,
    WorldTexturedGlcFloorColor,
    WorldTexturedGlcWallColor,
    WorldTexturedGlcTextureMultiplier,
    
    // Sprites GLC
    SpritesGlcMaterialSampler,
    SpritesGlcAlphaThreshold,
    
    // HUD Images GLC
    HudImagesGlcPrimarySampler,
    HudImagesGlcSecondarySampler,
    
    // Outline
    OutlineColor,
    OutlineDepthThreshold,
    OutlineNormalThreshold,
    OutlineScale,
    AliasModelOutlineGlcLerpFraction,
    AliasModelOutlineGlcOutlineScale,
    AliasModelOutlineColorModel,
    AliasModelOutlineColorTeam,
    AliasModelOutlineColorEnemy,
    AliasModelOutlineUsePlayerColor,
    AliasModelOutlineScale,
    
    // BrushModel AlphaTested
    BrushModelAlphaTestedOutlines,
    BrushModelAlphaTestedSampler,
    
    // Simple
    SimpleColor,
    Simple3DColor,
    
    // Lighting
    LightingFirstLightmap,
    
    Count
};

/**
 * Get uniform name string (for shader lookup).
 */
[[nodiscard]] constexpr const char* getUniformName(UniformId id) noexcept {
    switch (id) {
        case UniformId::AliasModelDrawMode: return "u_drawMode";
        case UniformId::BrushModelOutlines: return "u_outlines";
        case UniformId::BrushModelSampler: return "u_sampler";
        case UniformId::Sprite3DAlphaTest: return "u_alphaTest";
        case UniformId::HudCircleMatrix: return "u_matrix";
        case UniformId::HudCircleColor: return "u_color";
        case UniformId::SimpleColor: return "u_color";
        case UniformId::Simple3DColor: return "u_color";
        default: return "u_unknown";
    }
}

// =============================================================================
// AttributeId - Shader attribute identifiers
// =============================================================================

/**
 * Shader vertex attribute identifiers.
 * Original: r_program_attribute_id from r_program.h
 */
enum class AttributeId : uint8_t {
    AliasModelStdGlcFlags = 0,
    AliasModelShellGlcFlags,
    AliasModelShadowGlcFlags,
    AliasModelOutlineGlcFlags,
    WorldDrawflatStyle,
    WorldTexturedStyle,
    WorldTexturedDetailCoord,
    
    Count
};

[[nodiscard]] constexpr const char* getAttributeName(AttributeId id) noexcept {
    switch (id) {
        case AttributeId::AliasModelStdGlcFlags: return "a_flags";
        case AttributeId::AliasModelShellGlcFlags: return "a_flags";
        case AttributeId::AliasModelShadowGlcFlags: return "a_flags";
        case AttributeId::AliasModelOutlineGlcFlags: return "a_flags";
        case AttributeId::WorldDrawflatStyle: return "a_style";
        case AttributeId::WorldTexturedStyle: return "a_style";
        case AttributeId::WorldTexturedDetailCoord: return "a_detailCoord";
        default: return "a_unknown";
    }
}

// =============================================================================
// MemoryBarrierId - Compute shader memory barriers
// =============================================================================

/**
 * Memory barrier types for compute shaders.
 * Original: r_program_memory_barrier_id from r_program.h
 */
enum class MemoryBarrierId : uint8_t {
    ImageAccess = 0,
    TextureAccess,
    
    Count
};

// =============================================================================
// UniformType - Data type of a uniform
// =============================================================================

/**
 * Data type of a shader uniform.
 */
enum class UniformType : uint8_t {
    Int1 = 0,
    Float1,
    Float2,
    Float3,
    Float4,
    Mat3,
    Mat4,
    Sampler2D,
    SamplerCube,
    
    Count
};

/**
 * Get element count for uniform type.
 */
[[nodiscard]] constexpr size_t getUniformElementCount(UniformType type) noexcept {
    switch (type) {
        case UniformType::Int1: return 1;
        case UniformType::Float1: return 1;
        case UniformType::Float2: return 2;
        case UniformType::Float3: return 3;
        case UniformType::Float4: return 4;
        case UniformType::Mat3: return 9;
        case UniformType::Mat4: return 16;
        case UniformType::Sampler2D: return 1;
        case UniformType::SamplerCube: return 1;
        default: return 0;
    }
}

// =============================================================================
// UniformValue - Type-safe uniform value storage
// =============================================================================

/**
 * Storage for uniform values.
 */
struct UniformValue {
    UniformType type{UniformType::Float1};
    union {
        int32_t i[4];
        float f[16];
    } data{};
    
    constexpr UniformValue() noexcept = default;
    
    [[nodiscard]] static constexpr UniformValue fromInt(int32_t v) noexcept {
        UniformValue uv;
        uv.type = UniformType::Int1;
        uv.data.i[0] = v;
        return uv;
    }
    
    [[nodiscard]] static constexpr UniformValue fromFloat(float v) noexcept {
        UniformValue uv;
        uv.type = UniformType::Float1;
        uv.data.f[0] = v;
        return uv;
    }
    
    [[nodiscard]] static constexpr UniformValue fromVec2(float x, float y) noexcept {
        UniformValue uv;
        uv.type = UniformType::Float2;
        uv.data.f[0] = x;
        uv.data.f[1] = y;
        return uv;
    }
    
    [[nodiscard]] static constexpr UniformValue fromVec3(float x, float y, float z) noexcept {
        UniformValue uv;
        uv.type = UniformType::Float3;
        uv.data.f[0] = x;
        uv.data.f[1] = y;
        uv.data.f[2] = z;
        return uv;
    }
    
    [[nodiscard]] static constexpr UniformValue fromVec4(float x, float y, float z, float w) noexcept {
        UniformValue uv;
        uv.type = UniformType::Float4;
        uv.data.f[0] = x;
        uv.data.f[1] = y;
        uv.data.f[2] = z;
        uv.data.f[3] = w;
        return uv;
    }
    
    [[nodiscard]] constexpr int32_t asInt() const noexcept {
        return data.i[0];
    }
    
    [[nodiscard]] constexpr float asFloat() const noexcept {
        return data.f[0];
    }
};

// =============================================================================
// ShaderSource - Shader source code reference
// =============================================================================

/**
 * Reference to shader source code.
 */
struct ShaderSource {
    ShaderType type{ShaderType::Vertex};
    std::string_view filename{};
    std::string_view source{};
    bool isEmbedded{false};
    
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return !filename.empty() || !source.empty();
    }
    
    [[nodiscard]] constexpr bool hasSource() const noexcept {
        return !source.empty();
    }
};

// =============================================================================
// ProgramDefinition - Definition for creating a shader program
// =============================================================================

/**
 * Definition for creating a shader program.
 */
struct ProgramDefinition {
    ProgramId id{ProgramId::None};
    std::string_view name{};
    std::array<ShaderSource, static_cast<size_t>(ShaderType::Count)> shaders{};
    bool requiresCompute{false};
    
    [[nodiscard]] constexpr bool hasShader(ShaderType type) const noexcept {
        return shaders[static_cast<size_t>(type)].isValid();
    }
    
    [[nodiscard]] constexpr const ShaderSource& getShader(ShaderType type) const noexcept {
        return shaders[static_cast<size_t>(type)];
    }
};

// =============================================================================
// ProgramState - Runtime state of a compiled program
// =============================================================================

/**
 * Runtime state of a compiled shader program.
 */
struct ProgramState {
    ProgramId id{ProgramId::None};
    uint32_t glHandle{0};
    uint32_t options{0};
    int32_t subProgramIndex{0};
    bool isCompiled{false};
    bool needsRecompile{false};
    
    // Uniform locations (cached for performance)
    std::array<int32_t, static_cast<size_t>(UniformId::Count)> uniformLocations{};
    std::array<int32_t, static_cast<size_t>(AttributeId::Count)> attributeLocations{};
    
    constexpr ProgramState() noexcept {
        uniformLocations.fill(-1);
        attributeLocations.fill(-1);
    }
    
    [[nodiscard]] constexpr bool isReady() const noexcept {
        return isCompiled && glHandle != 0;
    }
    
    [[nodiscard]] constexpr bool hasUniform(UniformId uniform) const noexcept {
        auto idx = static_cast<size_t>(uniform);
        return idx < uniformLocations.size() && uniformLocations[idx] >= 0;
    }
    
    [[nodiscard]] constexpr int32_t getUniformLocation(UniformId uniform) const noexcept {
        auto idx = static_cast<size_t>(uniform);
        return idx < uniformLocations.size() ? uniformLocations[idx] : -1;
    }
    
    [[nodiscard]] constexpr int32_t getAttributeLocation(AttributeId attr) const noexcept {
        auto idx = static_cast<size_t>(attr);
        return idx < attributeLocations.size() ? attributeLocations[idx] : -1;
    }
    
    void invalidate() noexcept {
        isCompiled = false;
        needsRecompile = true;
        glHandle = 0;
        uniformLocations.fill(-1);
        attributeLocations.fill(-1);
    }
};

// =============================================================================
// ShaderManager - Manages all shader programs
// =============================================================================

/**
 * Manages all shader programs.
 */
struct ShaderManager {
    std::array<ProgramState, static_cast<size_t>(ProgramId::Count)> programs{};
    ProgramId currentProgram{ProgramId::None};
    uint32_t programSwitchCount{0};
    uint32_t uniformSetCount{0};
    
    [[nodiscard]] constexpr ProgramState& getProgram(ProgramId id) noexcept {
        return programs[static_cast<size_t>(id)];
    }
    
    [[nodiscard]] constexpr const ProgramState& getProgram(ProgramId id) const noexcept {
        return programs[static_cast<size_t>(id)];
    }
    
    [[nodiscard]] constexpr bool isProgramReady(ProgramId id) const noexcept {
        return getProgram(id).isReady();
    }
    
    [[nodiscard]] constexpr bool isCurrentProgram(ProgramId id) const noexcept {
        return currentProgram == id;
    }
    
    void resetStats() noexcept {
        programSwitchCount = 0;
        uniformSetCount = 0;
    }
    
    void invalidateAll() noexcept {
        for (auto& prog : programs) {
            prog.invalidate();
        }
        currentProgram = ProgramId::None;
    }
};

// =============================================================================
// ComputeDispatchInfo - Info for compute shader dispatch
// =============================================================================

/**
 * Information for compute shader dispatch.
 */
struct ComputeDispatchInfo {
    uint32_t groupsX{1};
    uint32_t groupsY{1};
    uint32_t groupsZ{1};
    std::bitset<static_cast<size_t>(MemoryBarrierId::Count)> barriers{};
    
    [[nodiscard]] constexpr uint32_t totalGroups() const noexcept {
        return groupsX * groupsY * groupsZ;
    }
    
    void setBarrier(MemoryBarrierId id) noexcept {
        barriers.set(static_cast<size_t>(id));
    }
    
    [[nodiscard]] bool hasBarrier(MemoryBarrierId id) const noexcept {
        return barriers.test(static_cast<size_t>(id));
    }
};

// =============================================================================
// FramebufferId - Framebuffer object identifiers
// =============================================================================

/**
 * Framebuffer identifiers.
 * Original: framebuffer_id from gl_framebuffer.h
 */
enum class FramebufferId : uint8_t {
    None = 0,
    Standard,
    StandardMS,         // Multisampled
    Hud,
    HudMS,
    StandardBlit,       // For resolving multisample
    StandardBlitMS,
    HudBlit,
    HudBlitMS,
    
    Count
};

[[nodiscard]] constexpr const char* getFramebufferName(FramebufferId id) noexcept {
    switch (id) {
        case FramebufferId::None: return "none";
        case FramebufferId::Standard: return "std";
        case FramebufferId::StandardMS: return "std-ms";
        case FramebufferId::Hud: return "hud";
        case FramebufferId::HudMS: return "hud-ms";
        case FramebufferId::StandardBlit: return "std-blit";
        case FramebufferId::StandardBlitMS: return "std-blit-ms";
        case FramebufferId::HudBlit: return "hud-blit";
        case FramebufferId::HudBlitMS: return "hud-blit-ms";
        default: return "unknown";
    }
}

[[nodiscard]] constexpr bool needsDepthBuffer(FramebufferId id) noexcept {
    return id == FramebufferId::Standard || id == FramebufferId::StandardMS;
}

[[nodiscard]] constexpr bool isMultisampled(FramebufferId id) noexcept {
    switch (id) {
        case FramebufferId::StandardMS:
        case FramebufferId::HudMS:
        case FramebufferId::StandardBlitMS:
        case FramebufferId::HudBlitMS:
            return true;
        default:
            return false;
    }
}

// =============================================================================
// FramebufferTextureId - Framebuffer texture attachment types
// =============================================================================

/**
 * Framebuffer texture attachment types.
 * Original: fbtex_id from gl_framebuffer.h
 */
enum class FramebufferTextureId : uint8_t {
    Standard = 0,
    Depth,
    Bloom,
    WorldNormals,
    
    Count
};

[[nodiscard]] constexpr const char* getFramebufferTextureName(FramebufferTextureId id) noexcept {
    switch (id) {
        case FramebufferTextureId::Standard: return "std";
        case FramebufferTextureId::Depth: return "depth";
        case FramebufferTextureId::Bloom: return "bloom";
        case FramebufferTextureId::WorldNormals: return "normals";
        default: return "unknown";
    }
}

// =============================================================================
// FramebufferState - Runtime state of a framebuffer
// =============================================================================

/**
 * Framebuffer runtime state.
 */
struct FramebufferState {
    FramebufferId id{FramebufferId::None};
    uint32_t glHandle{0};
    int32_t width{0};
    int32_t height{0};
    int32_t samples{0};
    uint32_t depthBuffer{0};
    uint32_t status{0};  // GL_FRAMEBUFFER_COMPLETE etc.
    std::array<uint32_t, static_cast<size_t>(FramebufferTextureId::Count)> textures{};
    
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return glHandle != 0 && width > 0 && height > 0;
    }
    
    [[nodiscard]] constexpr bool isComplete() const noexcept {
        return status == 0x8CD5; // GL_FRAMEBUFFER_COMPLETE
    }
    
    [[nodiscard]] constexpr bool hasTexture(FramebufferTextureId tex) const noexcept {
        return textures[static_cast<size_t>(tex)] != 0;
    }
    
    [[nodiscard]] constexpr uint32_t getTexture(FramebufferTextureId tex) const noexcept {
        return textures[static_cast<size_t>(tex)];
    }
};

// =============================================================================
// Helper functions
// =============================================================================

/**
 * Create a compute dispatch info for a 2D work area.
 */
[[nodiscard]] constexpr ComputeDispatchInfo createDispatch2D(
    uint32_t width, uint32_t height, 
    uint32_t localSizeX = 8, uint32_t localSizeY = 8) noexcept 
{
    ComputeDispatchInfo info;
    info.groupsX = (width + localSizeX - 1) / localSizeX;
    info.groupsY = (height + localSizeY - 1) / localSizeY;
    info.groupsZ = 1;
    return info;
}

/**
 * Create a compute dispatch info for a 1D work area.
 */
[[nodiscard]] constexpr ComputeDispatchInfo createDispatch1D(
    uint32_t count, uint32_t localSize = 64) noexcept 
{
    ComputeDispatchInfo info;
    info.groupsX = (count + localSize - 1) / localSize;
    info.groupsY = 1;
    info.groupsZ = 1;
    return info;
}

} // namespace ezquake::shader
