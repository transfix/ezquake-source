/**
 * @file model_types.hpp
 * @brief Model and rendering type definitions
 * 
 * Converted from gl_model.h, modelgen.h, spritegn.h, bspfile.h
 * Part of ezQuake C to C++20 Conversion - Iteration 21
 */

#pragma once

#include <cstdint>
#include <string_view>
#include <array>
#include <string>
#include "core/math/vec3.hpp"

namespace ezquake::render {

using math::Vec3;

// =============================================================================
// Model Type (from gl_model.h)
// =============================================================================

/**
 * Type of model data
 */
enum class ModelType : uint8_t {
    Brush = 0,    // BSP brush model (world, doors, etc.)
    Sprite = 1,   // Sprite model (.spr)
    Alias = 2,    // Alias model (.mdl - Quake 1 format)
    Alias3 = 3,   // MD3 model (Quake 3 format)
    Unknown = 255
};

[[nodiscard]] constexpr std::string_view modelTypeName(ModelType type) noexcept {
    switch (type) {
        case ModelType::Brush:   return "brush";
        case ModelType::Sprite:  return "sprite";
        case ModelType::Alias:   return "alias";
        case ModelType::Alias3:  return "alias3";
        default:                 return "unknown";
    }
}

[[nodiscard]] constexpr bool isBrushModel(ModelType type) noexcept {
    return type == ModelType::Brush;
}

[[nodiscard]] constexpr bool isAliasModel(ModelType type) noexcept {
    return type == ModelType::Alias || type == ModelType::Alias3;
}

// =============================================================================
// Model Hints (from gl_model.h)
// =============================================================================

/**
 * Special model type hints for rendering
 */
enum class ModelHint : uint8_t {
    Normal = 0,
    Player,
    Eyes,
    Flame,
    Thunderbolt,
    Backpack,
    Flag,
    Spike,
    TfTrail,
    Grenade,
    Tesla,
    SentryGun,
    Detpack,
    Laser,
    Demon,
    Soldier,
    Ogre,
    Enforcer,
    VoorSpike,
    LavaBall,
    Rail,
    Rail2,
    BuildingGibs,
    Cluster,
    Shambler,
    TeleportDestination,
    Gib,
    VModel,
    Rocket,
    Armor,
    MegaHealth,
    RocketLauncher,
    LightningGun,
    Quad,
    Pent,
    Ring,
    Flame0,
    Flame2,
    Flame3,
    Suit,
    GrenadeLauncher,
    
    Count  // Number of hints
};

[[nodiscard]] constexpr std::string_view modelHintName(ModelHint hint) noexcept {
    switch (hint) {
        case ModelHint::Normal:       return "normal";
        case ModelHint::Player:       return "player";
        case ModelHint::Eyes:         return "eyes";
        case ModelHint::Flame:        return "flame";
        case ModelHint::Thunderbolt:  return "thunderbolt";
        case ModelHint::Backpack:     return "backpack";
        case ModelHint::Flag:         return "flag";
        case ModelHint::Spike:        return "spike";
        case ModelHint::Grenade:      return "grenade";
        case ModelHint::Rocket:       return "rocket";
        case ModelHint::Quad:         return "quad";
        case ModelHint::Pent:         return "pent";
        case ModelHint::Ring:         return "ring";
        default:                      return "other";
    }
}

[[nodiscard]] constexpr bool isProjectile(ModelHint hint) noexcept {
    return hint == ModelHint::Spike || hint == ModelHint::Grenade ||
           hint == ModelHint::Rocket || hint == ModelHint::VoorSpike ||
           hint == ModelHint::LavaBall || hint == ModelHint::Laser;
}

[[nodiscard]] constexpr bool isPowerup(ModelHint hint) noexcept {
    return hint == ModelHint::Quad || hint == ModelHint::Pent ||
           hint == ModelHint::Ring || hint == ModelHint::Suit;
}

// =============================================================================
// Sync Type (from modelgen.h/spritegn.h)
// =============================================================================

/**
 * Animation synchronization type
 */
enum class SyncType : uint8_t {
    Sync = 0,   // Synchronized animation
    Rand = 1    // Random start time
};

// =============================================================================
// Sprite Types (from spritegn.h)
// =============================================================================

/**
 * Sprite orientation type
 */
enum class SpriteType : uint8_t {
    VpParallelUpright    = 0,  // Faces camera, upright
    FacingUpright        = 1,  // Faces camera origin, upright
    VpParallel           = 2,  // Faces camera
    Oriented             = 3,  // Uses entity angles
    VpParallelOriented   = 4   // Parallel with roll
};

[[nodiscard]] constexpr std::string_view spriteTypeName(SpriteType type) noexcept {
    switch (type) {
        case SpriteType::VpParallelUpright:  return "vp_parallel_upright";
        case SpriteType::FacingUpright:      return "facing_upright";
        case SpriteType::VpParallel:         return "vp_parallel";
        case SpriteType::Oriented:           return "oriented";
        case SpriteType::VpParallelOriented: return "vp_parallel_oriented";
        default:                              return "unknown";
    }
}

[[nodiscard]] constexpr bool isUpright(SpriteType type) noexcept {
    return type == SpriteType::VpParallelUpright || 
           type == SpriteType::FacingUpright;
}

/**
 * Sprite frame type
 */
enum class SpriteFrameType : uint8_t {
    Single = 0,
    Group = 1
};

// =============================================================================
// Alias Model Types (from modelgen.h)
// =============================================================================

/**
 * Alias frame type
 */
enum class AliasFrameType : uint8_t {
    Single = 0,
    Group = 1
};

/**
 * Alias skin type
 */
enum class AliasSkinType : uint8_t {
    Single = 0,
    Group = 1
};

// =============================================================================
// Entity Effects (from gl_model.h)
// =============================================================================

/**
 * Entity effect flags for trails and rotation
 */
namespace EntityEffects {
    constexpr uint32_t None     = 0;
    constexpr uint32_t Rocket   = 1;    // Leave rocket trail
    constexpr uint32_t Grenade  = 2;    // Leave grenade trail
    constexpr uint32_t Gib      = 4;    // Leave gib trail
    constexpr uint32_t Rotate   = 8;    // Rotate (bonus items)
    constexpr uint32_t Tracer   = 16;   // Green split trail
    constexpr uint32_t ZomGib   = 32;   // Small blood trail
    constexpr uint32_t Tracer2  = 64;   // Orange split trail + rotate
    constexpr uint32_t Tracer3  = 128;  // Purple trail
    
    [[nodiscard]] constexpr bool hasTrail(uint32_t effects) noexcept {
        return (effects & (Rocket | Grenade | Gib | Tracer | ZomGib | Tracer2 | Tracer3)) != 0;
    }
    
    [[nodiscard]] constexpr bool shouldRotate(uint32_t effects) noexcept {
        return (effects & (Rotate | Tracer2)) != 0;
    }
}

// =============================================================================
// Surface Flags (from gl_model.h)
// =============================================================================

/**
 * Surface rendering flags
 */
namespace SurfaceFlags {
    constexpr uint32_t PlaneBack      = 2;    // Surface on back of plane
    constexpr uint32_t DrawSky        = 4;    // Sky surface
    constexpr uint32_t DrawSprite     = 8;    // Sprite surface
    constexpr uint32_t DrawTurb       = 16;   // Turbulent (water, lava)
    constexpr uint32_t DrawTiled      = 32;   // Tiled texture
    constexpr uint32_t DrawBackground = 64;   // Background
    constexpr uint32_t Underwater     = 128;  // Underwater
    constexpr uint32_t DrawAlpha      = 256;  // Alpha-blended
    constexpr uint32_t DrawFlatFloor  = 512;  // Flat shaded floor
    
    [[nodiscard]] constexpr bool isSky(uint32_t flags) noexcept {
        return (flags & DrawSky) != 0;
    }
    
    [[nodiscard]] constexpr bool isWater(uint32_t flags) noexcept {
        return (flags & DrawTurb) != 0;
    }
    
    [[nodiscard]] constexpr bool isTransparent(uint32_t flags) noexcept {
        return (flags & DrawAlpha) != 0;
    }
    
    [[nodiscard]] constexpr bool needsSpecialRendering(uint32_t flags) noexcept {
        return (flags & (DrawSky | DrawTurb | DrawAlpha)) != 0;
    }
}

// =============================================================================
// BSP Constants (from bspfile.h)
// =============================================================================

namespace bsp_constants {
    // BSP versions
    constexpr int32_t Q1_VERSION    = 29;
    constexpr int32_t HL_VERSION    = 30;
    
    // Map limits
    constexpr int32_t MAX_HULLS        = 4;
    constexpr int32_t MAX_MODELS       = 4096;
    constexpr int32_t MAX_BRUSHES      = 4096;
    constexpr int32_t MAX_ENTITIES     = 1024;
    constexpr int32_t MAX_ENTSTRING    = 65536;
    constexpr int32_t MAX_PLANES       = 8192;
    constexpr int32_t MAX_NODES        = 32767;
    constexpr int32_t MAX_CLIPNODES    = 32767;
    constexpr int32_t MAX_LEAFS        = 32767;
    constexpr int32_t MAX_VERTS        = 65535;
    constexpr int32_t MAX_FACES        = 65535;
    constexpr int32_t MAX_MARKSURFACES = 65535;
    constexpr int32_t MAX_TEXINFO      = 4096;
    constexpr int32_t MAX_EDGES        = 256000;
    constexpr int32_t MAX_SURFEDGES    = 512000;
    constexpr int32_t MAX_MIPTEX       = 0x200000;
    constexpr int32_t MAX_LIGHTING     = 0x100000;
    constexpr int32_t MAX_VISIBILITY   = 0x100000;
    
    // Mipmap levels
    constexpr int32_t MIP_LEVELS = 4;
    
    // Lightmaps
    constexpr int32_t MAX_LIGHTMAPS = 4;
    constexpr int32_t DEFAULT_LMSHIFT = 4;
    
    // Ambient sounds
    constexpr int32_t NUM_AMBIENTS = 4;
    constexpr int32_t AMBIENT_WATER = 0;
    constexpr int32_t AMBIENT_SKY   = 1;
    constexpr int32_t AMBIENT_SLIME = 2;
    constexpr int32_t AMBIENT_LAVA  = 3;
}

// =============================================================================
// BSP Lump Types (from bspfile.h)
// =============================================================================

enum class BspLump : uint8_t {
    Entities     = 0,
    Planes       = 1,
    Textures     = 2,
    Vertexes     = 3,
    Visibility   = 4,
    Nodes        = 5,
    TexInfo      = 6,
    Faces        = 7,
    Lighting     = 8,
    ClipNodes    = 9,
    Leafs        = 10,
    MarkSurfaces = 11,
    Edges        = 12,
    SurfEdges    = 13,
    Models       = 14,
    
    Count = 15
};

[[nodiscard]] constexpr std::string_view bspLumpName(BspLump lump) noexcept {
    switch (lump) {
        case BspLump::Entities:     return "entities";
        case BspLump::Planes:       return "planes";
        case BspLump::Textures:     return "textures";
        case BspLump::Vertexes:     return "vertexes";
        case BspLump::Visibility:   return "visibility";
        case BspLump::Nodes:        return "nodes";
        case BspLump::TexInfo:      return "texinfo";
        case BspLump::Faces:        return "faces";
        case BspLump::Lighting:     return "lighting";
        case BspLump::ClipNodes:    return "clipnodes";
        case BspLump::Leafs:        return "leafs";
        case BspLump::MarkSurfaces: return "marksurfaces";
        case BspLump::Edges:        return "edges";
        case BspLump::SurfEdges:    return "surfedges";
        case BspLump::Models:       return "models";
        default:                    return "unknown";
    }
}

// =============================================================================
// Plane Types (from bspfile.h)
// =============================================================================

/**
 * Plane axis type for BSP
 */
enum class PlaneType : uint8_t {
    X      = 0,   // Axial plane aligned with X
    Y      = 1,   // Axial plane aligned with Y
    Z      = 2,   // Axial plane aligned with Z
    AnyX   = 3,   // Non-axial, mostly X
    AnyY   = 4,   // Non-axial, mostly Y
    AnyZ   = 5    // Non-axial, mostly Z
};

[[nodiscard]] constexpr bool isAxialPlane(PlaneType type) noexcept {
    return type <= PlaneType::Z;
}

[[nodiscard]] constexpr int32_t planeAxis(PlaneType type) noexcept {
    return static_cast<int32_t>(type) % 3;
}

// =============================================================================
// Alias Model Constants
// =============================================================================

namespace alias_constants {
    constexpr int32_t VERSION = 6;
    constexpr int32_t MAX_VERTS = 2048;
    constexpr int32_t MAX_FRAMES = 256;
    constexpr int32_t MAX_TRIS = 2048;
    constexpr int32_t MAX_SKINS = 32;
    constexpr int32_t ONSEAM_FLAG = 0x0020;
    
    // Magic header "IDPO"
    constexpr uint32_t HEADER = (('O' << 24) + ('P' << 16) + ('D' << 8) + 'I');
}

// =============================================================================
// Sprite Constants
// =============================================================================

namespace sprite_constants {
    constexpr int32_t VERSION = 1;
    constexpr int32_t MAX_FRAMES = 512;
    
    // Magic header "IDSP"
    constexpr uint32_t HEADER = (('P' << 24) + ('S' << 16) + ('D' << 8) + 'I');
}

// =============================================================================
// Model Bounds
// =============================================================================

/**
 * Axis-aligned bounding box for models
 */
struct ModelBounds {
    Vec3 mins{};
    Vec3 maxs{};
    float radius = 0.0f;
    
    [[nodiscard]] constexpr Vec3 center() const noexcept {
        return (mins + maxs) * 0.5f;
    }
    
    [[nodiscard]] constexpr Vec3 size() const noexcept {
        return maxs - mins;
    }
    
    [[nodiscard]] constexpr bool isEmpty() const noexcept {
        return mins[0] >= maxs[0] || mins[1] >= maxs[1] || mins[2] >= maxs[2];
    }
    
    [[nodiscard]] bool contains(const Vec3& point) const noexcept {
        return point[0] >= mins[0] && point[0] <= maxs[0] &&
               point[1] >= mins[1] && point[1] <= maxs[1] &&
               point[2] >= mins[2] && point[2] <= maxs[2];
    }
    
    void expand(const Vec3& point) noexcept {
        for (size_t i = 0; i < 3; ++i) {
            if (point[i] < mins[i]) mins[i] = point[i];
            if (point[i] > maxs[i]) maxs[i] = point[i];
        }
    }
    
    void calculateRadius() noexcept {
        Vec3 corner;
        for (size_t i = 0; i < 3; ++i) {
            float a = std::abs(mins[i]);
            float b = std::abs(maxs[i]);
            corner[i] = (a > b) ? a : b;
        }
        radius = corner.length();
    }
};

// =============================================================================
// Texture Info
// =============================================================================

/**
 * Texture mapping vectors
 */
struct TextureVecs {
    std::array<float, 4> s{};  // S texture axis [xyz, offset]
    std::array<float, 4> t{};  // T texture axis [xyz, offset]
    
    [[nodiscard]] float calcS(const Vec3& pos) const noexcept {
        return pos[0] * s[0] + pos[1] * s[1] + pos[2] * s[2] + s[3];
    }
    
    [[nodiscard]] float calcT(const Vec3& pos) const noexcept {
        return pos[0] * t[0] + pos[1] * t[1] + pos[2] * t[2] + t[3];
    }
};

// =============================================================================
// BSP Plane
// =============================================================================

/**
 * BSP plane for collision and rendering
 */
struct BspPlane {
    Vec3 normal{};
    float dist = 0.0f;
    PlaneType type = PlaneType::X;
    
    [[nodiscard]] float distanceToPoint(const Vec3& point) const noexcept {
        return normal.dot(point) - dist;
    }
    
    [[nodiscard]] bool isInFront(const Vec3& point) const noexcept {
        return distanceToPoint(point) > 0.0f;
    }
    
    [[nodiscard]] bool isAxial() const noexcept {
        return isAxialPlane(type);
    }
};

// =============================================================================
// Worldspawn Info
// =============================================================================

/**
 * Worldspawn entity properties
 */
struct WorldspawnInfo {
    std::string skyboxName;
    float fogDensity = 0.0f;
    Vec3 fogColor{};
    float fogSky = 0.0f;
    
    float waterAlpha = 1.0f;
    float lavaAlpha = 1.0f;
    float teleAlpha = 1.0f;
    float slimeAlpha = 1.0f;
    
    [[nodiscard]] bool hasFog() const noexcept {
        return fogDensity > 0.0f;
    }
    
    [[nodiscard]] bool hasSkybox() const noexcept {
        return !skyboxName.empty();
    }
    
    void clear() noexcept {
        skyboxName.clear();
        fogDensity = 0.0f;
        fogColor = Vec3{};
        fogSky = 0.0f;
        waterAlpha = lavaAlpha = teleAlpha = slimeAlpha = 1.0f;
    }
};

// =============================================================================
// Sprite Frame
// =============================================================================

/**
 * Single sprite frame
 */
struct SpriteFrame {
    int32_t width = 0;
    int32_t height = 0;
    float up = 0.0f;
    float down = 0.0f;
    float left = 0.0f;
    float right = 0.0f;
    uint32_t textureId = 0;
    
    [[nodiscard]] bool isValid() const noexcept {
        return width > 0 && height > 0;
    }
    
    [[nodiscard]] float aspectRatio() const noexcept {
        return height > 0 ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;
    }
};

// =============================================================================
// Alias Frame Descriptor
// =============================================================================

/**
 * Describes a frame or frame group in an alias model
 */
struct AliasFrameDesc {
    int32_t firstPose = 0;
    int32_t numPoses = 1;
    float interval = 0.1f;
    ModelBounds bounds{};
    std::array<char, 16> name{};
    std::array<char, 16> groupName{};
    int32_t groupNumber = 0;
    int32_t nextPose = -1;
    
    [[nodiscard]] bool isGroup() const noexcept {
        return numPoses > 1;
    }
    
    [[nodiscard]] std::string_view getName() const noexcept {
        return std::string_view(name.data());
    }
    
    [[nodiscard]] std::string_view getGroupName() const noexcept {
        return std::string_view(groupName.data());
    }
};

// =============================================================================
// Trivertex (Alias Model Vertex)
// =============================================================================

/**
 * Compressed vertex for alias models
 */
struct Trivertex {
    std::array<uint8_t, 3> v{};   // Compressed position
    uint8_t lightNormalIndex = 0;
    
    [[nodiscard]] Vec3 decompress(const Vec3& scale, const Vec3& origin) const noexcept {
        return Vec3(
            static_cast<float>(v[0]) * scale[0] + origin[0],
            static_cast<float>(v[1]) * scale[1] + origin[1],
            static_cast<float>(v[2]) * scale[2] + origin[2]
        );
    }
};

// =============================================================================
// Custom Model IDs
// =============================================================================

/**
 * Special custom model identifiers
 */
enum class CustomModelId : uint8_t {
    Explosion = 0,
    Bolt,
    Bolt2,
    Bolt3,
    Beam,
    Flame0,
    
    Count
};

[[nodiscard]] constexpr std::string_view customModelName(CustomModelId id) noexcept {
    switch (id) {
        case CustomModelId::Explosion: return "explosion";
        case CustomModelId::Bolt:      return "bolt";
        case CustomModelId::Bolt2:     return "bolt2";
        case CustomModelId::Bolt3:     return "bolt3";
        case CustomModelId::Beam:      return "beam";
        case CustomModelId::Flame0:    return "flame0";
        default:                       return "unknown";
    }
}

} // namespace ezquake::render
