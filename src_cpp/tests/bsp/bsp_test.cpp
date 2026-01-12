/*
 * ezQuake C++ Port - BSP Types Tests
 *
 * Unit tests for core/bsp/bsp_types.hpp
 */

#include <gtest/gtest.h>
#include "core/bsp/bsp_types.hpp"
#include <cstring>

using namespace ezquake;

//=============================================================================
// BSP Version Tests
//=============================================================================

TEST(BSPVersionTest, DetectVersion) {
    EXPECT_EQ(detectBSPVersion(29), BSPVersion::Quake1);
    EXPECT_EQ(detectBSPVersion(30), BSPVersion::HalfLife);
    EXPECT_EQ(detectBSPVersion(bsp::Q1_VERSION_29A), BSPVersion::BSP29A);
    EXPECT_EQ(detectBSPVersion(bsp::Q1_VERSION_BSP2), BSPVersion::BSP2);
    EXPECT_EQ(detectBSPVersion(0), BSPVersion::Unknown);
    EXPECT_EQ(detectBSPVersion(99), BSPVersion::Unknown);
}

TEST(BSPVersionTest, VersionNames) {
    EXPECT_EQ(bspVersionName(BSPVersion::Quake1), "Quake 1");
    EXPECT_EQ(bspVersionName(BSPVersion::HalfLife), "Half-Life");
    EXPECT_EQ(bspVersionName(BSPVersion::BSP2), "BSP2");
    EXPECT_EQ(bspVersionName(BSPVersion::Unknown), "unknown");
}

TEST(BSPVersionTest, IsExtended) {
    EXPECT_FALSE(isExtendedBSP(BSPVersion::Quake1));
    EXPECT_FALSE(isExtendedBSP(BSPVersion::HalfLife));
    EXPECT_TRUE(isExtendedBSP(BSPVersion::BSP29A));
    EXPECT_TRUE(isExtendedBSP(BSPVersion::BSP2));
}

//=============================================================================
// Lump Type Tests
//=============================================================================

TEST(LumpTypeTest, Names) {
    EXPECT_EQ(lumpTypeName(LumpType::Entities), "entities");
    EXPECT_EQ(lumpTypeName(LumpType::Planes), "planes");
    EXPECT_EQ(lumpTypeName(LumpType::Textures), "textures");
    EXPECT_EQ(lumpTypeName(LumpType::Nodes), "nodes");
    EXPECT_EQ(lumpTypeName(LumpType::Lighting), "lighting");
    EXPECT_EQ(lumpTypeName(LumpType::Visibility), "visibility");
    EXPECT_EQ(lumpTypeName(LumpType::Models), "models");
}

TEST(LumpTypeTest, Count) {
    EXPECT_EQ(static_cast<int>(LumpType::Count), 15);
}

//=============================================================================
// Plane Type Tests
//=============================================================================

TEST(BSPPlaneTypeTest, IsAxial) {
    EXPECT_TRUE(isAxialPlane(PlaneType::X));
    EXPECT_TRUE(isAxialPlane(PlaneType::Y));
    EXPECT_TRUE(isAxialPlane(PlaneType::Z));
    EXPECT_FALSE(isAxialPlane(PlaneType::AnyX));
    EXPECT_FALSE(isAxialPlane(PlaneType::AnyY));
    EXPECT_FALSE(isAxialPlane(PlaneType::AnyZ));
}

//=============================================================================
// Contents Tests
//=============================================================================

TEST(BSPContentsTest, Names) {
    EXPECT_EQ(contentsName(Contents::Empty), "empty");
    EXPECT_EQ(contentsName(Contents::Solid), "solid");
    EXPECT_EQ(contentsName(Contents::Water), "water");
    EXPECT_EQ(contentsName(Contents::Slime), "slime");
    EXPECT_EQ(contentsName(Contents::Lava), "lava");
    EXPECT_EQ(contentsName(Contents::Sky), "sky");
}

TEST(BSPContentsTest, IsLiquid) {
    EXPECT_FALSE(isLiquid(Contents::Empty));
    EXPECT_FALSE(isLiquid(Contents::Solid));
    EXPECT_TRUE(isLiquid(Contents::Water));
    EXPECT_TRUE(isLiquid(Contents::Slime));
    EXPECT_TRUE(isLiquid(Contents::Lava));
    EXPECT_FALSE(isLiquid(Contents::Sky));
}

TEST(BSPContentsTest, IsDamaging) {
    EXPECT_FALSE(isDamaging(Contents::Empty));
    EXPECT_FALSE(isDamaging(Contents::Water));
    EXPECT_TRUE(isDamaging(Contents::Slime));
    EXPECT_TRUE(isDamaging(Contents::Lava));
}

//=============================================================================
// Lump Tests
//=============================================================================

TEST(LumpTest, DefaultConstruction) {
    Lump l;
    EXPECT_EQ(l.offset, 0);
    EXPECT_EQ(l.length, 0);
    EXPECT_TRUE(l.isEmpty());
}

TEST(LumpTest, ValueConstruction) {
    Lump l(1000, 512);
    EXPECT_EQ(l.offset, 1000);
    EXPECT_EQ(l.length, 512);
    EXPECT_FALSE(l.isEmpty());
    EXPECT_EQ(l.end(), 1512);
}

//=============================================================================
// BSP Header Tests
//=============================================================================

TEST(BSPHeaderTest, DefaultConstruction) {
    BSPHeader h;
    EXPECT_EQ(h.version, 0);
    EXPECT_FALSE(h.isValid());
    EXPECT_EQ(h.getVersion(), BSPVersion::Unknown);
}

TEST(BSPHeaderTest, GetLump) {
    BSPHeader h;
    h.version = bsp::Q1_VERSION;
    h.lumps[static_cast<size_t>(LumpType::Entities)] = Lump(100, 200);
    
    EXPECT_TRUE(h.isValid());
    EXPECT_EQ(h.getVersion(), BSPVersion::Quake1);
    
    const Lump& entityLump = h.getLump(LumpType::Entities);
    EXPECT_EQ(entityLump.offset, 100);
    EXPECT_EQ(entityLump.length, 200);
}

//=============================================================================
// DVertex Tests
//=============================================================================

TEST(DVertexTest, DefaultConstruction) {
    DVertex v;
    EXPECT_EQ(v.x(), 0.0f);
    EXPECT_EQ(v.y(), 0.0f);
    EXPECT_EQ(v.z(), 0.0f);
}

TEST(DVertexTest, ValueConstruction) {
    DVertex v(1.0f, 2.0f, 3.0f);
    EXPECT_EQ(v.x(), 1.0f);
    EXPECT_EQ(v.y(), 2.0f);
    EXPECT_EQ(v.z(), 3.0f);
}

//=============================================================================
// DPlane Tests
//=============================================================================

TEST(DPlaneTest, Distance) {
    DPlane p;
    p.normal = {1.0f, 0.0f, 0.0f};  // X-axis normal
    p.dist = 10.0f;
    
    EXPECT_FLOAT_EQ(p.distance(10.0f, 0.0f, 0.0f), 0.0f);
    EXPECT_FLOAT_EQ(p.distance(15.0f, 0.0f, 0.0f), 5.0f);
    EXPECT_FLOAT_EQ(p.distance(5.0f, 0.0f, 0.0f), -5.0f);
}

TEST(DPlaneTest, PointSide) {
    DPlane p;
    p.normal = {0.0f, 0.0f, 1.0f};  // Z-axis normal
    p.dist = 100.0f;
    
    EXPECT_EQ(p.pointSide(0.0f, 0.0f, 150.0f), Side::Front);
    EXPECT_EQ(p.pointSide(0.0f, 0.0f, 50.0f), Side::Back);
    EXPECT_EQ(p.pointSide(0.0f, 0.0f, 100.0f), Side::On);
}

//=============================================================================
// DNode Tests
//=============================================================================

TEST(DNodeTest, ChildIsLeaf) {
    DNode n;
    n.children[0] = 10;   // Node
    n.children[1] = -5;   // Leaf
    
    EXPECT_FALSE(n.childIsLeaf(0));
    EXPECT_TRUE(n.childIsLeaf(1));
    EXPECT_EQ(n.leafNum(1), 4);  // -((-5) + 1) = 4
}

//=============================================================================
// DClipnode Tests
//=============================================================================

TEST(DClipnodeTest, ChildContents) {
    DClipnode c;
    c.children[0] = 5;    // Node
    c.children[1] = -2;   // Contents::Solid
    
    EXPECT_FALSE(c.childIsContents(0));
    EXPECT_TRUE(c.childIsContents(1));
    EXPECT_EQ(c.childContents(1), Contents::Solid);
}

//=============================================================================
// DLeaf Tests
//=============================================================================

TEST(DLeafTest, DefaultConstruction) {
    DLeaf leaf;
    EXPECT_EQ(leaf.getContents(), Contents::Empty);
    EXPECT_FALSE(leaf.hasVisibility());
}

TEST(DLeafTest, Contents) {
    DLeaf leaf;
    leaf.contents = static_cast<std::int32_t>(Contents::Water);
    leaf.visOffset = 100;
    
    EXPECT_EQ(leaf.getContents(), Contents::Water);
    EXPECT_TRUE(leaf.hasVisibility());
}

TEST(DLeafTest, AmbientLevels) {
    DLeaf leaf;
    leaf.ambientLevel[0] = 128;  // Water
    leaf.ambientLevel[1] = 64;   // Sky
    
    EXPECT_EQ(leaf.getAmbient(AmbientType::Water), 128);
    EXPECT_EQ(leaf.getAmbient(AmbientType::Sky), 64);
}

//=============================================================================
// DEdge Tests
//=============================================================================

TEST(DEdgeTest, Construction) {
    DEdge e(100, 200);
    EXPECT_EQ(e.vertex(0), 100);
    EXPECT_EQ(e.vertex(1), 200);
}

//=============================================================================
// DFace Tests
//=============================================================================

TEST(DFaceTest, BackFace) {
    DFace f;
    f.side = 0;
    EXPECT_FALSE(f.isBackFace());
    
    f.side = 1;
    EXPECT_TRUE(f.isBackFace());
}

TEST(DFaceTest, Lightmap) {
    DFace f;
    f.lightOffset = -1;
    EXPECT_FALSE(f.hasLightmap());
    
    f.lightOffset = 1000;
    EXPECT_TRUE(f.hasLightmap());
}

//=============================================================================
// DModel Tests
//=============================================================================

TEST(DModelTest, HeadNode) {
    DModel m;
    m.headNode[0] = 0;
    m.headNode[1] = 10;
    m.headNode[2] = 20;
    m.headNode[3] = 30;
    
    EXPECT_EQ(m.getHeadNode(0), 0);
    EXPECT_EQ(m.getHeadNode(1), 10);
    EXPECT_EQ(m.getHeadNode(2), 20);
    EXPECT_EQ(m.getHeadNode(3), 30);
    EXPECT_EQ(m.getHeadNode(5), 0);  // Out of range
}

//=============================================================================
// MipTex Tests
//=============================================================================

TEST(MipTexTest, GetName) {
    MipTex mt;
    std::strcpy(mt.name.data(), "brick01");
    EXPECT_EQ(mt.getName(), "brick01");
}

TEST(MipTexTest, TextureTypes) {
    MipTex mt;
    
    std::strcpy(mt.name.data(), "sky1");
    EXPECT_TRUE(mt.isSky());
    EXPECT_FALSE(mt.isLiquid());
    
    std::strcpy(mt.name.data(), "*water1");
    EXPECT_FALSE(mt.isSky());
    EXPECT_TRUE(mt.isLiquid());
    
    std::strcpy(mt.name.data(), "brick01");
    EXPECT_FALSE(mt.isSky());
    EXPECT_FALSE(mt.isLiquid());
}

TEST(MipTexTest, MipSize) {
    MipTex mt;
    mt.width = 128;
    mt.height = 64;
    
    EXPECT_EQ(mt.mipSize(0), 128 * 64);      // Full size
    EXPECT_EQ(mt.mipSize(1), 64 * 32);       // Half size
    EXPECT_EQ(mt.mipSize(2), 32 * 16);       // Quarter size
    EXPECT_EQ(mt.mipSize(3), 16 * 8);        // Eighth size
    EXPECT_EQ(mt.mipSize(4), 0u);            // Out of range
}

//=============================================================================
// Texinfo Tests
//=============================================================================

TEST(TexinfoTest, IsSpecial) {
    Texinfo ti;
    ti.flags = 0;
    EXPECT_FALSE(ti.isSpecial());
    
    ti.flags = TexFlags::Special;
    EXPECT_TRUE(ti.isSpecial());
}

TEST(TexinfoTest, CalcTexCoords) {
    Texinfo ti;
    ti.vecs[0] = {1.0f, 0.0f, 0.0f, 0.0f};  // S = x
    ti.vecs[1] = {0.0f, 1.0f, 0.0f, 0.0f};  // T = y
    
    EXPECT_FLOAT_EQ(ti.calcS(10.0f, 20.0f, 30.0f), 10.0f);
    EXPECT_FLOAT_EQ(ti.calcT(10.0f, 20.0f, 30.0f), 20.0f);
}

TEST(TexinfoTest, CalcTexCoordsWithOffset) {
    Texinfo ti;
    ti.vecs[0] = {0.5f, 0.0f, 0.0f, 16.0f};  // S = 0.5*x + 16
    ti.vecs[1] = {0.0f, 0.5f, 0.0f, 32.0f};  // T = 0.5*y + 32
    
    EXPECT_FLOAT_EQ(ti.calcS(100.0f, 0.0f, 0.0f), 66.0f);  // 0.5*100 + 16
    EXPECT_FLOAT_EQ(ti.calcT(0.0f, 100.0f, 0.0f), 82.0f);  // 0.5*100 + 32
}

//=============================================================================
// LightmapInfo Tests
//=============================================================================

TEST(LightmapInfoTest, SizeBytes) {
    LightmapInfo lm;
    lm.width = 16;
    lm.height = 16;
    EXPECT_EQ(lm.sizeBytes(), 16 * 16 * 3);  // RGB
}

//=============================================================================
// BSPXHeader Tests
//=============================================================================

TEST(BSPXHeaderTest, Validation) {
    BSPXHeader h;
    h.id = {'B', 'S', 'P', 'X'};
    EXPECT_TRUE(h.isValid());
    
    h.id = {'N', 'O', 'P', 'E'};
    EXPECT_FALSE(h.isValid());
}

//=============================================================================
// BSPStats Tests
//=============================================================================

TEST(BSPStatsTest, WithinLimits) {
    BSPStats s;
    s.numPlanes = 1000;
    s.numNodes = 10000;
    s.numClipnodes = 10000;
    s.numLeafs = 5000;
    s.numFaces = 50000;
    s.numEdges = 100000;
    
    EXPECT_TRUE(s.withinStandardLimits());
    EXPECT_FALSE(s.requiresBSP2());
}

TEST(BSPStatsTest, ExceedsLimits) {
    BSPStats s;
    s.numNodes = 40000;  // Exceeds MAX_NODES (32767)
    
    EXPECT_FALSE(s.withinStandardLimits());
    EXPECT_TRUE(s.requiresBSP2());
}

//=============================================================================
// Map Limits Constants Tests
//=============================================================================

TEST(MapLimitsTest, StandardValues) {
    EXPECT_EQ(map_limits::MAX_HULLS, 4);
    EXPECT_EQ(map_limits::MAX_MODELS, 4096);
    EXPECT_EQ(map_limits::MAX_NODES, 32767);
    EXPECT_EQ(map_limits::MAX_LEAFS, 32767);
    EXPECT_EQ(map_limits::MAX_FACES, 65535);
    EXPECT_EQ(map_limits::MIP_LEVELS, 4);
    EXPECT_EQ(map_limits::MAX_LIGHTMAPS, 4);
    EXPECT_EQ(map_limits::NUM_AMBIENTS, 4);
}
