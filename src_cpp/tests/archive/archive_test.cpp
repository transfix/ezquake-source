/**
 * @file archive_test.cpp
 * @brief Tests for archive and VFS types
 */

#include <gtest/gtest.h>
#include "core/archive/archive_types.hpp"

using namespace ezquake::archive;

// =============================================================================
// Archive Limits Tests
// =============================================================================

TEST(ArchiveLimitsTest, Constants) {
    EXPECT_EQ(archive_limits::MAX_QPATH, 64);
    EXPECT_EQ(archive_limits::MAX_OSPATH, 256);
    EXPECT_EQ(archive_limits::MAX_WAD_LUMPS, 4096);
    EXPECT_EQ(archive_limits::MAX_PACK_FILES, 16384);
}

TEST(ArchiveLimitsTest, MagicNumbers) {
    EXPECT_EQ(archive_limits::WAD2_MAGIC, 0x32444157);
    EXPECT_EQ(archive_limits::WAD3_MAGIC, 0x33444157);
    EXPECT_EQ(archive_limits::PAK_MAGIC, 0x4B434150);
}

// =============================================================================
// WAD Compression Tests
// =============================================================================

TEST(WadCompressionTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(WadCompression::None), 0);
    EXPECT_EQ(static_cast<uint8_t>(WadCompression::LZSS), 1);
}

// =============================================================================
// WAD Lump Type Tests
// =============================================================================

TEST(WadLumpTypeTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(WadLumpType::None), 0);
    EXPECT_EQ(static_cast<uint8_t>(WadLumpType::Label), 1);
    EXPECT_EQ(static_cast<uint8_t>(WadLumpType::Palette), 64);
    EXPECT_EQ(static_cast<uint8_t>(WadLumpType::QPic), 66);
    EXPECT_EQ(static_cast<uint8_t>(WadLumpType::MipTex), 68);
}

TEST(WadLumpTypeTest, TypeNames) {
    EXPECT_STREQ(getLumpTypeName(WadLumpType::None), "None");
    EXPECT_STREQ(getLumpTypeName(WadLumpType::QPic), "QPic");
    EXPECT_STREQ(getLumpTypeName(WadLumpType::MipTex), "MipTex");
}

TEST(WadLumpTypeTest, IsLumpyType) {
    EXPECT_FALSE(isLumpyType(WadLumpType::None));
    EXPECT_FALSE(isLumpyType(WadLumpType::Label));
    EXPECT_TRUE(isLumpyType(WadLumpType::Palette));
    EXPECT_TRUE(isLumpyType(WadLumpType::QPic));
}

// =============================================================================
// WAD Lump Tests
// =============================================================================

TEST(WadLumpTest, DefaultConstruction) {
    WadLump lump;
    EXPECT_EQ(lump.filePos, 0);
    EXPECT_EQ(lump.type, WadLumpType::None);
    EXPECT_FALSE(lump.isCompressed());
}

TEST(WadLumpTest, IsCompressed) {
    WadLump lump;
    EXPECT_FALSE(lump.isCompressed());
    
    lump.compression = WadCompression::LZSS;
    EXPECT_TRUE(lump.isCompressed());
}

TEST(WadLumpTest, CompressionRatio) {
    WadLump lump;
    lump.size = 1000;
    lump.diskSize = 500;
    
    EXPECT_NEAR(lump.compressionRatio(), 0.5f, 0.001f);
}

// =============================================================================
// WAD Header Tests
// =============================================================================

TEST(WadHeaderTest, IsWad2) {
    WadHeader header;
    header.magic = archive_limits::WAD2_MAGIC;
    
    EXPECT_TRUE(header.isWad2());
    EXPECT_FALSE(header.isWad3());
    EXPECT_TRUE(header.isValid());
    EXPECT_EQ(header.version(), 2);
}

TEST(WadHeaderTest, IsWad3) {
    WadHeader header;
    header.magic = archive_limits::WAD3_MAGIC;
    
    EXPECT_FALSE(header.isWad2());
    EXPECT_TRUE(header.isWad3());
    EXPECT_TRUE(header.isValid());
    EXPECT_EQ(header.version(), 3);
}

TEST(WadHeaderTest, InvalidMagic) {
    WadHeader header;
    header.magic = 0x12345678;
    
    EXPECT_FALSE(header.isValid());
    EXPECT_EQ(header.version(), 0);
}

// =============================================================================
// QPic Header Tests
// =============================================================================

TEST(QPicHeaderTest, TotalPixels) {
    QPicHeader pic;
    pic.width = 320;
    pic.height = 200;
    
    EXPECT_EQ(pic.totalPixels(), 64000);
    EXPECT_EQ(pic.dataSize(), 64000);
}

TEST(QPicHeaderTest, IsValid) {
    QPicHeader valid;
    valid.width = 256;
    valid.height = 128;
    EXPECT_TRUE(valid.isValid());
    
    QPicHeader tooLarge;
    tooLarge.width = 4096;
    tooLarge.height = 256;
    EXPECT_FALSE(tooLarge.isValid());
    
    QPicHeader zero;
    EXPECT_FALSE(zero.isValid());
}

// =============================================================================
// Pack File Tests
// =============================================================================

TEST(PackFileTest, EndPos) {
    PackFile file;
    file.filePos = 1000;
    file.fileLen = 500;
    
    EXPECT_EQ(file.endPos(), 1500);
}

// =============================================================================
// Pack Header Tests
// =============================================================================

TEST(PackHeaderTest, IsValid) {
    PackHeader header;
    EXPECT_FALSE(header.isValid());
    
    header.magic = archive_limits::PAK_MAGIC;
    EXPECT_TRUE(header.isValid());
}

TEST(PackHeaderTest, NumFiles) {
    PackHeader header;
    header.tableSize = 64 * 10; // 10 files
    
    EXPECT_EQ(header.numFiles(), 10);
}

// =============================================================================
// File Location Tests
// =============================================================================

TEST(FileLocationTest, IsValid) {
    FileLocation loc;
    EXPECT_FALSE(loc.isValid());
    
    loc.searchPathIndex = 0;
    loc.length = 100;
    EXPECT_TRUE(loc.isValid());
}

TEST(FileLocationTest, IsRawFile) {
    FileLocation loc;
    EXPECT_FALSE(loc.isRawFile());
    
    loc.rawName[0] = 'f';
    EXPECT_TRUE(loc.isRawFile());
}

// =============================================================================
// Search Path Flags Tests
// =============================================================================

TEST(SearchPathFlagsTest, Values) {
    EXPECT_EQ(SearchPathFlags::None, 0);
    EXPECT_EQ(SearchPathFlags::CopyProtected, 1);
    EXPECT_EQ(SearchPathFlags::Temporary, 2);
    EXPECT_EQ(SearchPathFlags::Pure, 4);
}

// =============================================================================
// Archive Type Tests
// =============================================================================

TEST(ArchiveTypeTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(ArchiveType::Directory), 0);
    EXPECT_EQ(static_cast<uint8_t>(ArchiveType::Pak), 1);
    EXPECT_EQ(static_cast<uint8_t>(ArchiveType::Zip), 2);
}

TEST(ArchiveTypeTest, Names) {
    EXPECT_STREQ(getArchiveTypeName(ArchiveType::Directory), "Directory");
    EXPECT_STREQ(getArchiveTypeName(ArchiveType::Pak), "PAK");
    EXPECT_STREQ(getArchiveTypeName(ArchiveType::Zip), "ZIP");
}

TEST(ArchiveTypeTest, Extensions) {
    EXPECT_STREQ(getArchiveExtension(ArchiveType::Pak), ".pak");
    EXPECT_STREQ(getArchiveExtension(ArchiveType::Zip), ".pk3");
    EXPECT_STREQ(getArchiveExtension(ArchiveType::Wad), ".wad");
}

// =============================================================================
// Search Path Info Tests
// =============================================================================

TEST(SearchPathInfoTest, Flags) {
    SearchPathInfo path;
    path.flags = SearchPathFlags::CopyProtected | SearchPathFlags::WriteEnabled;
    
    EXPECT_TRUE(path.isCopyProtected());
    EXPECT_TRUE(path.isWritable());
    EXPECT_FALSE(path.isTemporary());
}

// =============================================================================
// Mipmap Texture Header Tests
// =============================================================================

TEST(MipTexHeaderTest, MipDimensions) {
    MipTexHeader tex;
    tex.width = 256;
    tex.height = 128;
    
    EXPECT_EQ(tex.mipWidth(0), 256);
    EXPECT_EQ(tex.mipHeight(0), 128);
    EXPECT_EQ(tex.mipWidth(1), 128);
    EXPECT_EQ(tex.mipHeight(1), 64);
    EXPECT_EQ(tex.mipWidth(2), 64);
    EXPECT_EQ(tex.mipHeight(2), 32);
    EXPECT_EQ(tex.mipWidth(3), 32);
    EXPECT_EQ(tex.mipHeight(3), 16);
}

TEST(MipTexHeaderTest, MipSize) {
    MipTexHeader tex;
    tex.width = 64;
    tex.height = 64;
    
    EXPECT_EQ(tex.mipSize(0), 4096);
    EXPECT_EQ(tex.mipSize(1), 1024);
    EXPECT_EQ(tex.mipSize(2), 256);
    EXPECT_EQ(tex.mipSize(3), 64);
}

TEST(MipTexHeaderTest, TotalMipSize) {
    MipTexHeader tex;
    tex.width = 64;
    tex.height = 64;
    
    // Sum of all mip levels: 4096 + 1024 + 256 + 64 = 5440
    EXPECT_EQ(tex.totalMipSize(), 5440);
}

TEST(MipTexHeaderTest, IsValid) {
    MipTexHeader valid;
    valid.width = 64;
    valid.height = 64;
    EXPECT_TRUE(valid.isValid());
    
    MipTexHeader notPow2;
    notPow2.width = 100;
    notPow2.height = 64;
    EXPECT_FALSE(notPow2.isValid());
}

// =============================================================================
// File Open Mode Tests
// =============================================================================

TEST(FileOpenModeTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(FileOpenMode::Read), 0);
    EXPECT_EQ(static_cast<uint8_t>(FileOpenMode::Write), 1);
}

TEST(FileOpenModeTest, ModeStrings) {
    EXPECT_STREQ(getModeString(FileOpenMode::Read), "rb");
    EXPECT_STREQ(getModeString(FileOpenMode::Write), "wb");
    EXPECT_STREQ(getModeString(FileOpenMode::Append), "ab");
    EXPECT_STREQ(getModeString(FileOpenMode::ReadWrite), "r+b");
}

// =============================================================================
// VFS Error Tests
// =============================================================================

TEST(VfsErrorTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(VfsError::None), 0);
    EXPECT_EQ(static_cast<uint8_t>(VfsError::FileNotFound), 1);
}

TEST(VfsErrorTest, ErrorStrings) {
    EXPECT_STREQ(getVfsErrorString(VfsError::None), "No error");
    EXPECT_STREQ(getVfsErrorString(VfsError::FileNotFound), "File not found");
    EXPECT_STREQ(getVfsErrorString(VfsError::CorruptArchive), "Corrupt archive");
}

// =============================================================================
// File Hash Entry Tests
// =============================================================================

TEST(FileHashEntryTest, IsValid) {
    FileHashEntry entry;
    EXPECT_FALSE(entry.isValid());
    
    entry.pakIndex = 0;
    entry.searchPathIndex = 0;
    EXPECT_TRUE(entry.isValid());
}

// =============================================================================
// Archive Stats Tests
// =============================================================================

TEST(ArchiveStatsTest, CompressionRatio) {
    ArchiveStats stats;
    stats.totalSize = 1000;
    stats.compressedSize = 500;
    
    EXPECT_NEAR(stats.compressionRatio(), 0.5f, 0.001f);
}

TEST(ArchiveStatsTest, SpaceSaved) {
    ArchiveStats stats;
    stats.totalSize = 1000;
    stats.compressedSize = 700;
    
    EXPECT_EQ(stats.spaceSaved(), 300);
}

TEST(ArchiveStatsTest, NoCompression) {
    ArchiveStats stats;
    stats.totalSize = 1000;
    stats.compressedSize = 1200; // Actually expanded
    
    EXPECT_EQ(stats.spaceSaved(), 0);
}
