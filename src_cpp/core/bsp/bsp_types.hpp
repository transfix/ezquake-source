/*
 * ezQuake C++ Port - BSP File Types
 *
 * Modern C++20 replacements for bspfile.h structures.
 * Covers BSP file format parsing, map structure types, and limits.
 *
 * Original C sources:
 *   - bspfile.h (BSP file format definitions)
 *   - cmodel.h (hull and plane types)
 *
 * Copyright (C) 1996-1997 Id Software, Inc.
 * Copyright (C) 2024 ezQuake Team
 */

#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace ezquake {

//=============================================================================
// BSP Version Constants
//=============================================================================

namespace bsp {

// BSP format version numbers
inline constexpr std::int32_t Q1_VERSION = 29;
inline constexpr std::int32_t HL_VERSION = 30;
inline constexpr std::int32_t Q1_VERSION_29A = (('2') + ('P' << 8) + ('S' << 16) + ('B' << 24));
inline constexpr std::int32_t Q1_VERSION_BSP2 = (('B') + ('S' << 8) + ('P' << 16) + ('2' << 24));

} // namespace bsp

//=============================================================================
// Map Limits
//=============================================================================

namespace map_limits {

inline constexpr int MAX_HULLS = 4;
inline constexpr int MAX_MODELS = 4096;
inline constexpr int MAX_BRUSHES = 4096;
inline constexpr int MAX_ENTITIES = 1024;
inline constexpr int MAX_ENTSTRING = 65536;
inline constexpr int MAX_PLANES = 8192;
inline constexpr int MAX_NODES = 32767;       // Negative shorts are contents
inline constexpr int MAX_CLIPNODES = 32767;
inline constexpr int MAX_LEAFS = 32767;
inline constexpr int MAX_VERTS = 65535;
inline constexpr int MAX_FACES = 65535;
inline constexpr int MAX_MARKSURFACES = 65535;
inline constexpr int MAX_TEXINFO = 4096;
inline constexpr int MAX_EDGES = 256000;
inline constexpr int MAX_SURFEDGES = 512000;
inline constexpr int MAX_MIPTEX = 0x200000;
inline constexpr int MAX_LIGHTING = 0x100000;
inline constexpr int MAX_VISIBILITY = 0x100000;

// Entity key/value sizes
inline constexpr int MAX_KEY_LENGTH = 32;
inline constexpr int MAX_VALUE_LENGTH = 1024;

// Mipmap levels
inline constexpr int MIP_LEVELS = 4;

// Lightmaps per surface
inline constexpr int MAX_LIGHTMAPS = 4;

// Ambient sound types
inline constexpr int NUM_AMBIENTS = 4;

} // namespace map_limits

//=============================================================================
// BSPVersion - BSP format version enum
//=============================================================================

/**
 * @brief BSP file format version.
 */
enum class BSPVersion : std::int32_t {
    Unknown = 0,
    Quake1 = bsp::Q1_VERSION,
    HalfLife = bsp::HL_VERSION,
    BSP29A = bsp::Q1_VERSION_29A,
    BSP2 = bsp::Q1_VERSION_BSP2
};

/**
 * @brief Detect BSP version from integer.
 */
[[nodiscard]] constexpr BSPVersion detectBSPVersion(std::int32_t version) noexcept {
    switch (version) {
        case bsp::Q1_VERSION:      return BSPVersion::Quake1;
        case bsp::HL_VERSION:      return BSPVersion::HalfLife;
        case bsp::Q1_VERSION_29A:  return BSPVersion::BSP29A;
        case bsp::Q1_VERSION_BSP2: return BSPVersion::BSP2;
        default:                   return BSPVersion::Unknown;
    }
}

/**
 * @brief Get name of BSP version.
 */
[[nodiscard]] constexpr std::string_view bspVersionName(BSPVersion version) noexcept {
    switch (version) {
        case BSPVersion::Unknown:  return "unknown";
        case BSPVersion::Quake1:   return "Quake 1";
        case BSPVersion::HalfLife: return "Half-Life";
        case BSPVersion::BSP29A:   return "BSP2-RMQ";
        case BSPVersion::BSP2:     return "BSP2";
    }
    return "unknown";
}

/**
 * @brief Check if version is extended (BSP2 family).
 */
[[nodiscard]] constexpr bool isExtendedBSP(BSPVersion version) noexcept {
    return version == BSPVersion::BSP29A || version == BSPVersion::BSP2;
}

//=============================================================================
// LumpType - BSP lump indices
//=============================================================================

/**
 * @brief BSP lump types.
 * 
 * Original: LUMP_* defines
 */
enum class LumpType : std::uint8_t {
    Entities = 0,
    Planes = 1,
    Textures = 2,
    Vertexes = 3,
    Visibility = 4,
    Nodes = 5,
    Texinfo = 6,
    Faces = 7,
    Lighting = 8,
    Clipnodes = 9,
    Leafs = 10,
    Marksurfaces = 11,
    Edges = 12,
    Surfedges = 13,
    Models = 14,
    Count = 15   // HEADER_LUMPS
};

/**
 * @brief Get name of lump type.
 */
[[nodiscard]] constexpr std::string_view lumpTypeName(LumpType type) noexcept {
    switch (type) {
        case LumpType::Entities:     return "entities";
        case LumpType::Planes:       return "planes";
        case LumpType::Textures:     return "textures";
        case LumpType::Vertexes:     return "vertexes";
        case LumpType::Visibility:   return "visibility";
        case LumpType::Nodes:        return "nodes";
        case LumpType::Texinfo:      return "texinfo";
        case LumpType::Faces:        return "faces";
        case LumpType::Lighting:     return "lighting";
        case LumpType::Clipnodes:    return "clipnodes";
        case LumpType::Leafs:        return "leafs";
        case LumpType::Marksurfaces: return "marksurfaces";
        case LumpType::Edges:        return "edges";
        case LumpType::Surfedges:    return "surfedges";
        case LumpType::Models:       return "models";
        default:                     return "unknown";
    }
}

//=============================================================================
// PlaneType - Axial plane classification
//=============================================================================

/**
 * @brief Plane axis classification for optimization.
 * 
 * Original: PLANE_X, PLANE_Y, etc.
 */
enum class PlaneType : std::uint8_t {
    X = 0,      // Axial X plane
    Y = 1,      // Axial Y plane
    Z = 2,      // Axial Z plane
    AnyX = 3,   // Non-axial, mostly X
    AnyY = 4,   // Non-axial, mostly Y
    AnyZ = 5    // Non-axial, mostly Z
};

/**
 * @brief Check if plane is axial (aligned to X, Y, or Z).
 */
[[nodiscard]] constexpr bool isAxialPlane(PlaneType type) noexcept {
    return static_cast<std::uint8_t>(type) <= 2;
}

//=============================================================================
// Contents - BSP leaf contents
//=============================================================================

/**
 * @brief BSP leaf content types.
 * 
 * Original: CONTENTS_* defines
 */
enum class Contents : std::int32_t {
    Empty = -1,
    Solid = -2,
    Water = -3,
    Slime = -4,
    Lava = -5,
    Sky = -6
};

/**
 * @brief Get name of content type.
 */
[[nodiscard]] constexpr std::string_view contentsName(Contents c) noexcept {
    switch (c) {
        case Contents::Empty: return "empty";
        case Contents::Solid: return "solid";
        case Contents::Water: return "water";
        case Contents::Slime: return "slime";
        case Contents::Lava:  return "lava";
        case Contents::Sky:   return "sky";
    }
    return "unknown";
}

/**
 * @brief Check if contents is liquid.
 */
[[nodiscard]] constexpr bool isLiquid(Contents c) noexcept {
    return c == Contents::Water || c == Contents::Slime || c == Contents::Lava;
}

/**
 * @brief Check if contents damages player.
 */
[[nodiscard]] constexpr bool isDamaging(Contents c) noexcept {
    return c == Contents::Slime || c == Contents::Lava;
}

//=============================================================================
// AmbientType - Ambient sound channels
//=============================================================================

/**
 * @brief Ambient sound type indices.
 * 
 * Original: AMBIENT_* defines
 */
enum class AmbientType : std::uint8_t {
    Water = 0,
    Sky = 1,
    Slime = 2,
    Lava = 3
};

//=============================================================================
// TexFlags - Texture info flags
//=============================================================================

/**
 * @brief Texture info flags.
 * 
 * Original: TEX_SPECIAL
 */
namespace TexFlags {
    inline constexpr int None = 0;
    inline constexpr int Special = 1;  // Sky or slime, no lightmap or 256 subdivision
}

//=============================================================================
// Lump - File lump descriptor
//=============================================================================

/**
 * @brief BSP file lump location.
 * 
 * Original: lump_t
 */
struct Lump {
    std::int32_t offset = 0;
    std::int32_t length = 0;
    
    constexpr Lump() noexcept = default;
    constexpr Lump(std::int32_t off, std::int32_t len) noexcept : offset(off), length(len) {}
    
    /**
     * @brief Check if lump is empty.
     */
    [[nodiscard]] constexpr bool isEmpty() const noexcept {
        return length == 0;
    }
    
    /**
     * @brief Get end offset.
     */
    [[nodiscard]] constexpr std::int32_t end() const noexcept {
        return offset + length;
    }
};

//=============================================================================
// BSPHeader - BSP file header
//=============================================================================

/**
 * @brief BSP file header.
 * 
 * Original: dheader_t
 */
struct BSPHeader {
    std::int32_t version = 0;
    std::array<Lump, static_cast<size_t>(LumpType::Count)> lumps{};
    
    /**
     * @brief Get the BSP version enum.
     */
    [[nodiscard]] constexpr BSPVersion getVersion() const noexcept {
        return detectBSPVersion(version);
    }
    
    /**
     * @brief Check if header is valid.
     */
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return getVersion() != BSPVersion::Unknown;
    }
    
    /**
     * @brief Get a specific lump.
     */
    [[nodiscard]] constexpr const Lump& getLump(LumpType type) const noexcept {
        return lumps[static_cast<size_t>(type)];
    }
};

//=============================================================================
// DVertex - BSP vertex (disk format)
//=============================================================================

/**
 * @brief BSP vertex position.
 * 
 * Original: dvertex_t
 */
struct DVertex {
    std::array<float, 3> point{};
    
    constexpr DVertex() noexcept = default;
    constexpr DVertex(float x, float y, float z) noexcept : point{x, y, z} {}
    
    [[nodiscard]] constexpr float x() const noexcept { return point[0]; }
    [[nodiscard]] constexpr float y() const noexcept { return point[1]; }
    [[nodiscard]] constexpr float z() const noexcept { return point[2]; }
};

//=============================================================================
// DPlane - BSP plane (disk format)
//=============================================================================

/**
 * @brief BSP plane definition.
 * 
 * Original: dplane_t
 */
struct DPlane {
    std::array<float, 3> normal{};
    float dist = 0.0f;
    std::int32_t type = 0;  // PlaneType
    
    constexpr DPlane() noexcept = default;
    
    /**
     * @brief Get the plane type.
     */
    [[nodiscard]] constexpr PlaneType getType() const noexcept {
        return static_cast<PlaneType>(type);
    }
    
    /**
     * @brief Compute point-to-plane distance.
     */
    [[nodiscard]] constexpr float distance(float x, float y, float z) const noexcept {
        return normal[0] * x + normal[1] * y + normal[2] * z - dist;
    }
    
    /**
     * @brief Determine which side of plane the point is on.
     * @return SIDE_FRONT (0), SIDE_BACK (1), or SIDE_ON (2)
     */
    [[nodiscard]] constexpr int pointSide(float x, float y, float z, float epsilon = 0.01f) const noexcept {
        float d = distance(x, y, z);
        if (d > epsilon) return 0;   // SIDE_FRONT
        if (d < -epsilon) return 1;  // SIDE_BACK
        return 2;                     // SIDE_ON
    }
};

//=============================================================================
// Side - Plane side constants
//=============================================================================

namespace Side {
    inline constexpr int Front = 0;
    inline constexpr int Back = 1;
    inline constexpr int On = 2;
}

//=============================================================================
// DNode - BSP node (disk format)
//=============================================================================

/**
 * @brief BSP node (original format).
 * 
 * Original: dnode_t
 */
struct DNode {
    std::int32_t planeNum = 0;
    std::array<std::int16_t, 2> children{};  // Negative = -(leafs+1)
    std::array<std::int16_t, 3> mins{};
    std::array<std::int16_t, 3> maxs{};
    std::uint16_t firstFace = 0;
    std::uint16_t numFaces = 0;
    
    /**
     * @brief Check if child is a leaf.
     */
    [[nodiscard]] constexpr bool childIsLeaf(int index) const noexcept {
        return children[static_cast<size_t>(index)] < 0;
    }
    
    /**
     * @brief Get leaf number from child index.
     */
    [[nodiscard]] constexpr int leafNum(int index) const noexcept {
        return -children[static_cast<size_t>(index)] - 1;
    }
};

//=============================================================================
// DNodeBSP2 - BSP2 node (extended format)
//=============================================================================

/**
 * @brief BSP2 node (extended limits).
 * 
 * Original: dnode_bsp2_t
 */
struct DNodeBSP2 {
    std::int32_t planeNum = 0;
    std::array<std::int32_t, 2> children{};
    std::array<float, 3> mins{};
    std::array<float, 3> maxs{};
    std::uint32_t firstFace = 0;
    std::uint32_t numFaces = 0;
    
    [[nodiscard]] constexpr bool childIsLeaf(int index) const noexcept {
        return children[static_cast<size_t>(index)] < 0;
    }
    
    [[nodiscard]] constexpr int leafNum(int index) const noexcept {
        return -children[static_cast<size_t>(index)] - 1;
    }
};

//=============================================================================
// DClipnode - Collision clipnode
//=============================================================================

/**
 * @brief Collision clipnode (original format).
 * 
 * Original: dclipnode_t
 */
struct DClipnode {
    std::int32_t planeNum = 0;
    std::array<std::int16_t, 2> children{};  // Negative = contents
    
    /**
     * @brief Check if child is a contents value.
     */
    [[nodiscard]] constexpr bool childIsContents(int index) const noexcept {
        return children[static_cast<size_t>(index)] < 0;
    }
    
    /**
     * @brief Get contents from child.
     */
    [[nodiscard]] constexpr Contents childContents(int index) const noexcept {
        return static_cast<Contents>(children[static_cast<size_t>(index)]);
    }
};

//=============================================================================
// DLeaf - BSP leaf (disk format)
//=============================================================================

/**
 * @brief BSP leaf (original format).
 * 
 * Original: dleaf_t
 */
struct DLeaf {
    std::int32_t contents = static_cast<std::int32_t>(Contents::Empty);
    std::int32_t visOffset = -1;  // -1 = no visibility info
    std::array<std::int16_t, 3> mins{};
    std::array<std::int16_t, 3> maxs{};
    std::uint16_t firstMarkSurface = 0;
    std::uint16_t numMarkSurfaces = 0;
    std::array<std::uint8_t, map_limits::NUM_AMBIENTS> ambientLevel{};
    
    /**
     * @brief Get contents enum.
     */
    [[nodiscard]] constexpr Contents getContents() const noexcept {
        return static_cast<Contents>(contents);
    }
    
    /**
     * @brief Check if leaf has visibility info.
     */
    [[nodiscard]] constexpr bool hasVisibility() const noexcept {
        return visOffset >= 0;
    }
    
    /**
     * @brief Get ambient level for a sound type.
     */
    [[nodiscard]] constexpr std::uint8_t getAmbient(AmbientType type) const noexcept {
        return ambientLevel[static_cast<size_t>(type)];
    }
};

//=============================================================================
// DLeafBSP2 - BSP2 leaf (extended format)
//=============================================================================

/**
 * @brief BSP2 leaf (extended limits).
 * 
 * Original: dleaf_bsp2_t
 */
struct DLeafBSP2 {
    std::int32_t contents = static_cast<std::int32_t>(Contents::Empty);
    std::int32_t visOffset = -1;
    std::array<float, 3> mins{};
    std::array<float, 3> maxs{};
    std::uint32_t firstMarkSurface = 0;
    std::uint32_t numMarkSurfaces = 0;
    std::array<std::uint8_t, map_limits::NUM_AMBIENTS> ambientLevel{};
    
    [[nodiscard]] constexpr Contents getContents() const noexcept {
        return static_cast<Contents>(contents);
    }
    
    [[nodiscard]] constexpr bool hasVisibility() const noexcept {
        return visOffset >= 0;
    }
};

//=============================================================================
// DEdge - BSP edge
//=============================================================================

/**
 * @brief BSP edge (original format).
 * 
 * Original: dedge_t
 */
struct DEdge {
    std::array<std::uint16_t, 2> v{};  // Vertex numbers
    
    constexpr DEdge() noexcept = default;
    constexpr DEdge(std::uint16_t v0, std::uint16_t v1) noexcept : v{v0, v1} {}
    
    [[nodiscard]] constexpr std::uint16_t vertex(int index) const noexcept {
        return v[index & 1];
    }
};

/**
 * @brief BSP2 edge (extended format).
 * 
 * Original: dedge29a_t
 */
struct DEdgeBSP2 {
    std::array<std::uint32_t, 2> v{};
    
    [[nodiscard]] constexpr std::uint32_t vertex(int index) const noexcept {
        return v[index & 1];
    }
};

//=============================================================================
// DFace - BSP face (disk format)
//=============================================================================

/**
 * @brief BSP face (original format).
 * 
 * Original: dface_t
 */
struct DFace {
    std::int16_t planeNum = 0;
    std::int16_t side = 0;
    std::int32_t firstEdge = 0;
    std::int16_t numEdges = 0;
    std::int16_t texinfo = 0;
    std::array<std::uint8_t, map_limits::MAX_LIGHTMAPS> styles{};
    std::int32_t lightOffset = 0;
    
    /**
     * @brief Check if face is backfacing.
     */
    [[nodiscard]] constexpr bool isBackFace() const noexcept {
        return side != 0;
    }
    
    /**
     * @brief Check if face has lightmap.
     */
    [[nodiscard]] constexpr bool hasLightmap() const noexcept {
        return lightOffset >= 0;
    }
};

//=============================================================================
// DFaceBSP2 - BSP2 face (extended format)
//=============================================================================

/**
 * @brief BSP2 face (extended limits).
 * 
 * Original: dface29a_t
 */
struct DFaceBSP2 {
    std::int32_t planeNum = 0;
    std::int32_t side = 0;
    std::int32_t firstEdge = 0;
    std::int32_t numEdges = 0;
    std::int32_t texinfo = 0;
    std::array<std::uint8_t, map_limits::MAX_LIGHTMAPS> styles{};
    std::int32_t lightOffset = 0;
    
    [[nodiscard]] constexpr bool isBackFace() const noexcept {
        return side != 0;
    }
    
    [[nodiscard]] constexpr bool hasLightmap() const noexcept {
        return lightOffset >= 0;
    }
};

//=============================================================================
// DModel - BSP submodel
//=============================================================================

/**
 * @brief BSP submodel (brush entity).
 * 
 * Original: dmodel_t
 */
struct DModel {
    std::array<float, 3> mins{};
    std::array<float, 3> maxs{};
    std::array<float, 3> origin{};
    std::array<std::int32_t, map_limits::MAX_HULLS> headNode{};
    std::int32_t visLeafs = 0;  // Not including solid leaf 0
    std::int32_t firstFace = 0;
    std::int32_t numFaces = 0;
    
    /**
     * @brief Get head node for a hull.
     */
    [[nodiscard]] constexpr std::int32_t getHeadNode(int hull) const noexcept {
        if (hull >= 0 && hull < map_limits::MAX_HULLS) {
            return headNode[static_cast<size_t>(hull)];
        }
        return 0;
    }
};

//=============================================================================
// MipTexHeader - Miptex lump header
//=============================================================================

/**
 * @brief Miptex lump header.
 * 
 * Original: dmiptexlump_t
 */
struct MipTexHeader {
    std::int32_t numMipTex = 0;
    // dataofs[numMipTex] follows
    
    /**
     * @brief Calculate size of offset array.
     */
    [[nodiscard]] constexpr size_t offsetArraySize() const noexcept {
        return static_cast<size_t>(numMipTex) * sizeof(std::int32_t);
    }
};

//=============================================================================
// MipTex - Texture definition
//=============================================================================

/**
 * @brief Texture definition with mipmaps.
 * 
 * Original: miptex_t
 */
struct MipTex {
    std::array<char, 16> name{};
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::array<std::uint32_t, map_limits::MIP_LEVELS> offsets{};
    
    /**
     * @brief Get texture name as string_view.
     */
    [[nodiscard]] std::string_view getName() const {
        // Find null terminator
        size_t len = 0;
        while (len < name.size() && name[len] != '\0') len++;
        return std::string_view(name.data(), len);
    }
    
    /**
     * @brief Check if this is a sky texture.
     */
    [[nodiscard]] bool isSky() const {
        return getName().starts_with("sky");
    }
    
    /**
     * @brief Check if this is a liquid texture.
     */
    [[nodiscard]] bool isLiquid() const {
        auto n = getName();
        return n.starts_with("*") || n.starts_with("water") || 
               n.starts_with("slime") || n.starts_with("lava");
    }
    
    /**
     * @brief Get size of mip level.
     */
    [[nodiscard]] constexpr std::uint32_t mipSize(int level) const noexcept {
        if (level < 0 || level >= map_limits::MIP_LEVELS) return 0;
        std::uint32_t divisor = 1u << level;
        return (width / divisor) * (height / divisor);
    }
};

//=============================================================================
// Texinfo - Texture info
//=============================================================================

/**
 * @brief Texture mapping info.
 * 
 * Original: texinfo_t
 */
struct Texinfo {
    std::array<std::array<float, 4>, 2> vecs{};  // [s/t][xyz offset]
    std::int32_t mipTex = 0;
    std::int32_t flags = 0;
    
    /**
     * @brief Check if this is a special texture (sky, etc).
     */
    [[nodiscard]] constexpr bool isSpecial() const noexcept {
        return (flags & TexFlags::Special) != 0;
    }
    
    /**
     * @brief Calculate S texture coordinate.
     */
    [[nodiscard]] constexpr float calcS(float x, float y, float z) const noexcept {
        return x * vecs[0][0] + y * vecs[0][1] + z * vecs[0][2] + vecs[0][3];
    }
    
    /**
     * @brief Calculate T texture coordinate.
     */
    [[nodiscard]] constexpr float calcT(float x, float y, float z) const noexcept {
        return x * vecs[1][0] + y * vecs[1][1] + z * vecs[1][2] + vecs[1][3];
    }
};

//=============================================================================
// LightmapInfo - Extended lightmap info (BSPX)
//=============================================================================

/**
 * @brief Extended lightmap info from BSPX.
 * 
 * Original: dlminfo_t
 */
struct LightmapInfo {
    std::uint16_t width = 0;
    std::uint16_t height = 0;
    std::int32_t lightOffset = 0;
    std::array<std::array<float, 4>, 2> vecs{};
    
    /**
     * @brief Get lightmap size in bytes (RGB).
     */
    [[nodiscard]] constexpr std::uint32_t sizeBytes() const noexcept {
        return static_cast<std::uint32_t>(width) * height * 3;
    }
};

//=============================================================================
// BSPXHeader - BSPX extension header
//=============================================================================

/**
 * @brief BSPX extension header.
 * 
 * Original: bspx_header_t
 */
struct BSPXHeader {
    std::array<char, 4> id{};  // "BSPX"
    std::int32_t numLumps = 0;
    
    /**
     * @brief Check if this is a valid BSPX header.
     */
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return id[0] == 'B' && id[1] == 'S' && id[2] == 'P' && id[3] == 'X';
    }
};

//=============================================================================
// BSPStats - BSP file statistics
//=============================================================================

/**
 * @brief Statistics about a loaded BSP.
 */
struct BSPStats {
    BSPVersion version = BSPVersion::Unknown;
    std::int32_t numModels = 0;
    std::int32_t numPlanes = 0;
    std::int32_t numVertexes = 0;
    std::int32_t numNodes = 0;
    std::int32_t numClipnodes = 0;
    std::int32_t numLeafs = 0;
    std::int32_t numFaces = 0;
    std::int32_t numEdges = 0;
    std::int32_t numTexinfos = 0;
    std::int32_t numTextures = 0;
    std::int32_t lightingSize = 0;
    std::int32_t visibilitySize = 0;
    std::int32_t entityStringSize = 0;
    
    /**
     * @brief Check if map is within standard limits.
     */
    [[nodiscard]] constexpr bool withinStandardLimits() const noexcept {
        return numPlanes <= map_limits::MAX_PLANES &&
               numNodes <= map_limits::MAX_NODES &&
               numClipnodes <= map_limits::MAX_CLIPNODES &&
               numLeafs <= map_limits::MAX_LEAFS &&
               numFaces <= map_limits::MAX_FACES &&
               numEdges <= map_limits::MAX_EDGES;
    }
    
    /**
     * @brief Check if map requires BSP2 format.
     */
    [[nodiscard]] constexpr bool requiresBSP2() const noexcept {
        return !withinStandardLimits();
    }
};

} // namespace ezquake
