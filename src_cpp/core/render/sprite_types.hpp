/**
 * @file sprite_types.hpp
 * @brief Sprite, billboard, and sky rendering types
 * 
 * Converted from:
 * - spritegn.h: Sprite file format structures
 * - r_sprite3d.h: 3D sprite batch types
 * - r_sprite3d_internal.h: Internal sprite batch data
 * - r_brushmodel_sky.h: Sky rendering types
 * - modelgen.h: Model animation types
 */

#pragma once

#include <cstdint>
#include <array>
#include <string_view>
#include "core/math/vec3.hpp"

namespace ezquake::render {

// =============================================================================
// Sprite Constants
// =============================================================================

namespace sprite_limits {
    constexpr int32_t MAX_SPRITES_PER_BATCH = 16384;
    constexpr int32_t MAX_VERTS_PER_BATCH = MAX_SPRITES_PER_BATCH * 4;
    constexpr int32_t INDEXES_MAX_QUADS = 512;
    constexpr int32_t INDEXES_MAX_FLASHBLEND = 8;
    constexpr int32_t INDEXES_MAX_SPARKS = 16;
    constexpr int32_t MAX_SKYBOX_TEXTURES = 6;
    constexpr int32_t SPRITE_VERSION = 1;
    constexpr int32_t ALIAS_VERSION = 6;
    
    // Magic numbers
    constexpr uint32_t ID_SPRITE_HEADER = 0x50534449;  // "IDSP" little-endian
    constexpr uint32_t ID_POLY_HEADER = 0x4F504449;    // "IDPO" little-endian
    
    // Alias flags
    constexpr uint32_t ALIAS_ONSEAM = 0x0020;
    constexpr uint32_t DT_FACES_FRONT = 0x0010;
}

// =============================================================================
// Sync Type
// =============================================================================

/**
 * @brief Animation synchronization type
 */
enum class SyncType : uint8_t {
    Sync = 0,   // Synchronized animation
    Random = 1  // Random offset animation
};

// =============================================================================
// Sprite Orientation
// =============================================================================

/**
 * @brief Sprite orientation mode
 */
enum class SpriteOrientation : uint8_t {
    VpParallelUpright = 0,    // Parallel to view plane, upright
    FacingUpright = 1,        // Facing camera, upright
    VpParallel = 2,           // Parallel to view plane (billboard)
    Oriented = 3,             // Fixed orientation
    VpParallelOriented = 4    // Parallel with orientation
};

[[nodiscard]] constexpr const char* getSpriteOrientationName(SpriteOrientation orient) noexcept {
    switch (orient) {
        case SpriteOrientation::VpParallelUpright: return "VpParallelUpright";
        case SpriteOrientation::FacingUpright: return "FacingUpright";
        case SpriteOrientation::VpParallel: return "VpParallel";
        case SpriteOrientation::Oriented: return "Oriented";
        case SpriteOrientation::VpParallelOriented: return "VpParallelOriented";
        default: return "Unknown";
    }
}

// =============================================================================
// Sprite Frame Type
// =============================================================================

/**
 * @brief Sprite frame type (single or group)
 */
enum class SpriteFrameType : uint8_t {
    Single = 0,  // Single frame
    Group = 1    // Frame group (animation)
};

// =============================================================================
// Alias Frame Type
// =============================================================================

/**
 * @brief Alias model frame type
 */
enum class AliasFrameType : uint8_t {
    Single = 0,  // Single frame
    Group = 1    // Frame group
};

/**
 * @brief Alias skin type
 */
enum class AliasSkinType : uint8_t {
    Single = 0,  // Single skin
    Group = 1    // Skin group
};

// =============================================================================
// Primitive Type
// =============================================================================

/**
 * @brief Rendering primitive type
 */
enum class PrimitiveType : uint8_t {
    TriangleStrip = 0,
    TriangleFan = 1,
    Triangles = 2,
    
    Count = 3
};

[[nodiscard]] constexpr const char* getPrimitiveTypeName(PrimitiveType type) noexcept {
    switch (type) {
        case PrimitiveType::TriangleStrip: return "TriangleStrip";
        case PrimitiveType::TriangleFan: return "TriangleFan";
        case PrimitiveType::Triangles: return "Triangles";
        default: return "Unknown";
    }
}

// =============================================================================
// Sprite 3D Batch ID
// =============================================================================

/**
 * @brief Sprite batch identifier for grouping similar sprites
 */
enum class Sprite3DBatchId : uint8_t {
    // Core
    Entities = 0,
    ParticlesClassic = 1,
    
    // QMB Particles
    ParticleSpark = 2,
    ParticleSmoke = 3,
    ParticleFire = 4,
    ParticleBubble = 5,
    ParticleLavaSplash = 6,
    ParticleGunblast = 7,
    ParticleChunk = 8,
    ParticleShockwave = 9,
    ParticleInfernoFlame = 10,
    ParticleInfernoTrail = 11,
    ParticleSparkray = 12,
    ParticleStaticBubble = 13,
    ParticleTrailPart = 14,
    ParticleDpSmoke = 15,
    ParticleDpFire = 16,
    ParticleTeleflare = 17,
    ParticleBlood1 = 18,
    ParticleBlood2 = 19,
    ParticleBlood3 = 20,
    
    // Vult Particles
    ParticleRain = 21,
    ParticleAlphaTrail = 22,
    ParticleRailTrail = 23,
    ParticleStreak = 24,
    ParticleStreakTrail = 25,
    ParticleStreakWave = 26,
    ParticleLightningBeam = 27,
    ParticleVxBlood = 28,
    ParticleLavaTrail = 29,
    ParticleVxSmoke = 30,
    ParticleVxSmokeRed = 31,
    ParticleMuzzleFlash = 32,
    ParticleInferno = 33,
    Particle2DShockwave = 34,
    ParticleVxRocketSmoke = 35,
    ParticleTrailBleed = 36,
    ParticleBleedSpike = 37,
    ParticleFlame = 38,
    ParticleBubble2 = 39,
    ParticleBloodCloud = 40,
    ParticleChunkDir = 41,
    ParticleSmallSpark = 42,
    ParticleSlimeGlow = 43,
    ParticleSlimeBubble = 44,
    ParticleBlackLavaSmoke = 45,
    ParticleEntityTrail = 46,
    ParticleFlameTorch = 47,
    
    // Flashblend and Corona
    FlashblendLights = 48,
    CoronaStandard = 49,
    CoronaGunflash = 50,
    CoronaExplosionFlash1 = 51,
    CoronaExplosionFlash2 = 52,
    CoronaExplosionFlash3 = 53,
    CoronaExplosionFlash4 = 54,
    CoronaExplosionFlash5 = 55,
    CoronaExplosionFlash6 = 56,
    CoronaExplosionFlash7 = 57,
    
    // Chat icons
    ChatIconAfkChat = 58,
    ChatIconChat = 59,
    ChatIconAfk = 60,
    
    MaxBatches = 61
};

// New particles end at FlashblendLights
constexpr Sprite3DBatchId PARTICLES_NEW_LIMIT = Sprite3DBatchId::FlashblendLights;

[[nodiscard]] constexpr bool isParticleBatch(Sprite3DBatchId id) noexcept {
    return id >= Sprite3DBatchId::ParticlesClassic && 
           id < Sprite3DBatchId::FlashblendLights;
}

[[nodiscard]] constexpr bool isQMBParticleBatch(Sprite3DBatchId id) noexcept {
    return id >= Sprite3DBatchId::ParticleSpark && 
           id < Sprite3DBatchId::FlashblendLights;
}

[[nodiscard]] constexpr bool isCoronaBatch(Sprite3DBatchId id) noexcept {
    return id >= Sprite3DBatchId::CoronaStandard && 
           id <= Sprite3DBatchId::CoronaExplosionFlash7;
}

// =============================================================================
// Sprite Vertex Pool
// =============================================================================

/**
 * @brief Vertex pool identifier for sprite rendering
 */
enum class SpriteVertPoolId : uint8_t {
    GameEntities = 0,
    GameAnnotations = 1,
    SimpleParticles = 2,
    QmbParticlesNormal = 3,
    QmbParticlesEnvironment = 4,
    
    Count = 5
};

// =============================================================================
// Sprite Vertex
// =============================================================================

/**
 * @brief 3D sprite vertex structure (32 bytes aligned)
 */
struct Sprite3DVertex {
    std::array<float, 3> position{};   // 0-12 bytes
    std::array<float, 3> texCoord{};   // 12-24 bytes (u, v, layer)
    std::array<uint8_t, 4> color{255, 255, 255, 255}; // 24-28 bytes (RGBA)
    std::array<uint8_t, 4> padding{};  // 28-32 bytes (alignment)
    
    constexpr Sprite3DVertex() noexcept = default;
    
    constexpr void setPosition(float x, float y, float z) noexcept {
        position = {x, y, z};
    }
    
    constexpr void setTexCoord(float u, float v, float layer = 0.0f) noexcept {
        texCoord = {u, v, layer};
    }
    
    constexpr void setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) noexcept {
        color = {r, g, b, a};
    }
    
    [[nodiscard]] constexpr float x() const noexcept { return position[0]; }
    [[nodiscard]] constexpr float y() const noexcept { return position[1]; }
    [[nodiscard]] constexpr float z() const noexcept { return position[2]; }
    
    [[nodiscard]] constexpr float u() const noexcept { return texCoord[0]; }
    [[nodiscard]] constexpr float v() const noexcept { return texCoord[1]; }
    [[nodiscard]] constexpr float layer() const noexcept { return texCoord[2]; }
};

static_assert(sizeof(Sprite3DVertex) == 32, "Sprite3DVertex must be 32 bytes");

// =============================================================================
// Sprite File Header
// =============================================================================

/**
 * @brief Sprite file header (dsprite_t)
 */
struct SpriteHeader {
    uint32_t ident{};              // Magic number (IDSP)
    uint32_t version{};            // Sprite version
    uint32_t orientationType{};    // Orientation mode
    float boundingRadius{};        // Bounding radius
    int32_t width{};               // Sprite width
    int32_t height{};              // Sprite height
    int32_t numFrames{};           // Number of frames
    float beamLength{};            // Beam length (unused?)
    SyncType syncType{};           // Animation sync type
    
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return ident == sprite_limits::ID_SPRITE_HEADER &&
               version == sprite_limits::SPRITE_VERSION;
    }
    
    [[nodiscard]] constexpr SpriteOrientation orientation() const noexcept {
        return static_cast<SpriteOrientation>(orientationType);
    }
    
    [[nodiscard]] constexpr int32_t totalPixels() const noexcept {
        return width * height;
    }
};

// =============================================================================
// Sprite Frame Header
// =============================================================================

/**
 * @brief Sprite frame header
 */
struct SpriteFrameHeader {
    std::array<int32_t, 2> origin{};  // Origin offset
    int32_t width{};                  // Frame width
    int32_t height{};                 // Frame height
    
    [[nodiscard]] constexpr int32_t originX() const noexcept { return origin[0]; }
    [[nodiscard]] constexpr int32_t originY() const noexcept { return origin[1]; }
    
    [[nodiscard]] constexpr int32_t dataSize() const noexcept {
        return width * height;
    }
};

/**
 * @brief Sprite group header
 */
struct SpriteGroupHeader {
    int32_t numFrames{};
};

/**
 * @brief Sprite frame interval
 */
struct SpriteInterval {
    float interval{};
};

// =============================================================================
// Alias Model Header
// =============================================================================

/**
 * @brief Alias model header (mdl_t)
 */
struct AliasModelHeader {
    uint32_t ident{};              // Magic number (IDPO)
    uint32_t version{};            // MDL version
    math::Vec3 scale;              // Scaling factors
    math::Vec3 scaleOrigin;        // Scale origin
    float boundingRadius{};        // Bounding radius
    math::Vec3 eyePosition;        // Eye position
    int32_t numSkins{};            // Number of skins
    int32_t skinWidth{};           // Skin width
    int32_t skinHeight{};          // Skin height
    int32_t numVerts{};            // Number of vertices
    int32_t numTris{};             // Number of triangles
    int32_t numFrames{};           // Number of frames
    SyncType syncType{};           // Animation sync
    int32_t flags{};               // Model flags
    float size{};                  // Average triangle size
    
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return ident == sprite_limits::ID_POLY_HEADER &&
               version == sprite_limits::ALIAS_VERSION;
    }
    
    [[nodiscard]] constexpr int32_t skinPixels() const noexcept {
        return skinWidth * skinHeight;
    }
    
    [[nodiscard]] constexpr int32_t totalSkinBytes() const noexcept {
        return skinPixels() * numSkins;
    }
};

// =============================================================================
// Alias Vertex Types
// =============================================================================

/**
 * @brief Compressed alias vertex (4 bytes)
 */
struct TriVertex {
    std::array<uint8_t, 3> position{};  // Compressed position
    uint8_t lightNormalIndex{};          // Normal lookup index
    
    [[nodiscard]] constexpr uint8_t x() const noexcept { return position[0]; }
    [[nodiscard]] constexpr uint8_t y() const noexcept { return position[1]; }
    [[nodiscard]] constexpr uint8_t z() const noexcept { return position[2]; }
};

static_assert(sizeof(TriVertex) == 4, "TriVertex must be 4 bytes");

/**
 * @brief Expanded alias vertex with float precision
 */
struct ExpandedTriVertex {
    math::Vec3 position;
    math::Vec3 normal;
    uint8_t lightNormalIndex{};
};

/**
 * @brief Alias texture coordinate
 */
struct AliasTexCoord {
    int32_t onSeam{};  // On seam flag
    int32_t s{};       // S coordinate
    int32_t t{};       // T coordinate
    
    [[nodiscard]] constexpr bool isOnSeam() const noexcept {
        return onSeam != 0;
    }
};

/**
 * @brief Alias triangle
 */
struct AliasTriangle {
    int32_t facesFront{};              // Front-facing flag
    std::array<int32_t, 3> vertIndex{};  // Vertex indices
    
    [[nodiscard]] constexpr bool isFrontFacing() const noexcept {
        return (static_cast<uint32_t>(facesFront) & sprite_limits::DT_FACES_FRONT) != 0;
    }
    
    [[nodiscard]] constexpr int32_t vertex0() const noexcept { return vertIndex[0]; }
    [[nodiscard]] constexpr int32_t vertex1() const noexcept { return vertIndex[1]; }
    [[nodiscard]] constexpr int32_t vertex2() const noexcept { return vertIndex[2]; }
};

// =============================================================================
// Alias Frame Types
// =============================================================================

/**
 * @brief Alias frame header
 */
struct AliasFrameHeader {
    TriVertex bboxMin{};     // Bounding box minimum
    TriVertex bboxMax{};     // Bounding box maximum
    std::array<char, 16> name{};  // Frame name
    
    [[nodiscard]] std::string_view frameName() const noexcept {
        const char* end = std::find(name.begin(), name.end(), '\0');
        return {name.data(), static_cast<size_t>(end - name.data())};
    }
};

/**
 * @brief Alias frame group header
 */
struct AliasGroupHeader {
    int32_t numFrames{};
    TriVertex bboxMin{};
    TriVertex bboxMax{};
};

/**
 * @brief Alias frame interval
 */
struct AliasInterval {
    float interval{};
};

// =============================================================================
// Sky Types
// =============================================================================

/**
 * @brief Skybox face indices
 */
enum class SkyboxFace : uint8_t {
    Right = 0,   // +X
    Left = 1,    // -X
    Up = 2,      // +Y
    Down = 3,    // -Y
    Front = 4,   // +Z
    Back = 5     // -Z
};

[[nodiscard]] constexpr const char* getSkyboxFaceSuffix(SkyboxFace face) noexcept {
    switch (face) {
        case SkyboxFace::Right: return "rt";
        case SkyboxFace::Left: return "lf";
        case SkyboxFace::Up: return "up";
        case SkyboxFace::Down: return "dn";
        case SkyboxFace::Front: return "ft";
        case SkyboxFace::Back: return "bk";
        default: return "";
    }
}

/**
 * @brief Sky rendering state
 */
struct SkyState {
    bool skyboxLoaded{false};              // Skybox textures loaded
    std::array<uint32_t, 6> skyboxTextures{};  // Skybox texture IDs
    uint32_t solidSkyTexture{};            // Solid sky layer texture
    uint32_t alphaSkyTexture{};            // Alpha sky layer texture
    
    [[nodiscard]] constexpr bool hasSkybox() const noexcept {
        return skyboxLoaded && skyboxTextures[0] != 0;
    }
    
    [[nodiscard]] constexpr bool hasScrollingSky() const noexcept {
        return solidSkyTexture != 0;
    }
};

/**
 * @brief Sky wind parameters for animated sky
 */
struct SkyWind {
    math::Vec3 direction;
    float phase{};
    bool active{false};
    
    [[nodiscard]] constexpr bool isActive() const noexcept {
        return active;
    }
};

// =============================================================================
// Sprite Batch State
// =============================================================================

/**
 * @brief Sprite batch descriptor
 */
struct SpriteBatchInfo {
    Sprite3DBatchId id{Sprite3DBatchId::Entities};
    uint32_t renderState{};          // Render state ID
    PrimitiveType primitiveType{PrimitiveType::Triangles};
    uint32_t textureId{};            // Primary texture
    int32_t textureIndex{};          // Texture array index
    std::string_view name;           // Debug name
    uint32_t count{};                // Number of sprites in batch
    bool allSameTexture{true};       // Optimization flag
    
    [[nodiscard]] constexpr bool isEmpty() const noexcept {
        return count == 0;
    }
    
    [[nodiscard]] constexpr uint32_t vertexCount() const noexcept {
        // Assume quads (4 vertices per sprite)
        return count * 4;
    }
};

/**
 * @brief Global sprite rendering statistics
 */
struct SpriteRenderStats {
    uint32_t totalBatches{};
    uint32_t totalSprites{};
    uint32_t totalVertices{};
    uint32_t drawCalls{};
    
    constexpr void reset() noexcept {
        totalBatches = 0;
        totalSprites = 0;
        totalVertices = 0;
        drawCalls = 0;
    }
    
    constexpr void addBatch(uint32_t sprites, uint32_t verts) noexcept {
        totalBatches++;
        totalSprites += sprites;
        totalVertices += verts;
        drawCalls++;
    }
};

} // namespace ezquake::render
