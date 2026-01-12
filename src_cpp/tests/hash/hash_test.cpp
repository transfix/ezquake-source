/**
 * @file hash_test.cpp
 * @brief Unit tests for hash and checksum types
 */

#include <gtest/gtest.h>
#include "../../core/hash/hash_types.hpp"

using namespace ezquake::hash;

// ============================================================================
// HashAlgorithm Tests
// ============================================================================

TEST(HashAlgorithmTest, DigestSizes) {
    EXPECT_EQ(hashDigestSize(HashAlgorithm::CRC16), 2u);
    EXPECT_EQ(hashDigestSize(HashAlgorithm::MD4), 16u);
    EXPECT_EQ(hashDigestSize(HashAlgorithm::SHA1), 20u);
    EXPECT_EQ(hashDigestSize(HashAlgorithm::SHA256), 32u);
}

TEST(HashAlgorithmTest, Names) {
    EXPECT_EQ(hashAlgorithmName(HashAlgorithm::CRC16), "CRC-16");
    EXPECT_EQ(hashAlgorithmName(HashAlgorithm::MD4), "MD4");
    EXPECT_EQ(hashAlgorithmName(HashAlgorithm::SHA1), "SHA-1");
}

TEST(HashAlgorithmTest, IsCryptographic) {
    EXPECT_FALSE(hashIsCryptographic(HashAlgorithm::CRC16));
    EXPECT_FALSE(hashIsCryptographic(HashAlgorithm::MD4));
    EXPECT_FALSE(hashIsCryptographic(HashAlgorithm::SHA1));
    EXPECT_TRUE(hashIsCryptographic(HashAlgorithm::SHA256));
}

// ============================================================================
// Digest Tests
// ============================================================================

TEST(DigestTest, DefaultConstruction) {
    MD4Digest digest;
    EXPECT_TRUE(digest.isZero());
    EXPECT_EQ(digest.size(), 16u);
}

TEST(DigestTest, ToHex) {
    MD4Digest digest;
    digest.bytes[0] = 0xAB;
    digest.bytes[1] = 0xCD;
    digest.bytes[15] = 0xEF;
    
    std::string hex = digest.toHex();
    EXPECT_EQ(hex.substr(0, 4), "abcd");
    EXPECT_EQ(hex.substr(30, 2), "ef");
}

TEST(DigestTest, ToHexUpper) {
    MD4Digest digest;
    digest.bytes[0] = 0xAB;
    
    EXPECT_EQ(digest.toHexUpper().substr(0, 2), "AB");
}

TEST(DigestTest, FromHex) {
    auto result = MD4Digest::fromHex("00112233445566778899aabbccddeeff");
    ASSERT_TRUE(result.has_value());
    
    EXPECT_EQ(result->bytes[0], 0x00u);
    EXPECT_EQ(result->bytes[1], 0x11u);
    EXPECT_EQ(result->bytes[15], 0xFFu);
}

TEST(DigestTest, FromHexInvalid) {
    EXPECT_FALSE(MD4Digest::fromHex("short").has_value());
    EXPECT_FALSE(MD4Digest::fromHex("0011223344556677889zabbccddeeff").has_value());  // Invalid 'z'
}

TEST(DigestTest, Clear) {
    MD4Digest digest;
    digest.bytes[0] = 0xFF;
    
    digest.clear();
    EXPECT_TRUE(digest.isZero());
}

TEST(DigestTest, Equality) {
    MD4Digest a, b;
    EXPECT_EQ(a, b);
    
    a.bytes[0] = 1;
    EXPECT_NE(a, b);
}

TEST(DigestTest, IndexOperator) {
    MD4Digest digest;
    digest.bytes[5] = 42;
    
    EXPECT_EQ(digest[5], 42u);
    EXPECT_EQ(digest[100], 0u);  // Out of bounds returns 0
}

// ============================================================================
// CRC16Context Tests
// ============================================================================

TEST(CRC16ContextTest, Reset) {
    CRC16Context ctx;
    ctx.processByte(0xFF);
    
    EXPECT_NE(ctx.result(), 0xFFFF);
    
    ctx.reset();
    EXPECT_EQ(ctx.result(), 0xFFFF);
}

TEST(CRC16ContextTest, ProcessByte) {
    CRC16Context ctx;
    ctx.processByte('A');
    
    uint16_t result1 = ctx.result();
    
    ctx.reset();
    ctx.processByte('B');
    
    uint16_t result2 = ctx.result();
    
    EXPECT_NE(result1, result2);
}

TEST(CRC16ContextTest, ProcessBlock) {
    CRC16Context ctx;
    std::array<uint8_t, 4> data = {'t', 'e', 's', 't'};
    ctx.processBlock(data);
    
    CRC16Context ctx2;
    for (uint8_t b : data) {
        ctx2.processByte(b);
    }
    
    EXPECT_EQ(ctx.result(), ctx2.result());
}

TEST(CRC16ContextTest, DifferentDataDifferentResult) {
    CRC16Context ctx1, ctx2;
    
    std::array<uint8_t, 4> data1 = {1, 2, 3, 4};
    std::array<uint8_t, 4> data2 = {4, 3, 2, 1};
    
    ctx1.processBlock(data1);
    ctx2.processBlock(data2);
    
    EXPECT_NE(ctx1.result(), ctx2.result());
}

// ============================================================================
// MD4Context Tests
// ============================================================================

TEST(MD4ContextTest, DefaultState) {
    MD4Context ctx;
    EXPECT_FALSE(ctx.isFinalized());
    EXPECT_EQ(ctx.state[0], 0x67452301u);
}

TEST(MD4ContextTest, Reset) {
    MD4Context ctx;
    ctx.state[0] = 0;
    ctx.finalized = true;
    
    ctx.reset();
    
    EXPECT_EQ(ctx.state[0], 0x67452301u);
    EXPECT_FALSE(ctx.isFinalized());
}

TEST(MD4ContextTest, BitCount) {
    MD4Context ctx;
    ctx.count[0] = 1024;
    ctx.count[1] = 1;
    
    uint64_t expected = (1ULL << 32) | 1024;
    EXPECT_EQ(ctx.bitCount(), expected);
}

// ============================================================================
// SHA1Context Tests
// ============================================================================

TEST(SHA1ContextTest, DefaultState) {
    SHA1Context ctx;
    EXPECT_FALSE(ctx.isFinalized());
    EXPECT_EQ(ctx.state[0], 0x67452301u);
    EXPECT_EQ(ctx.state[4], 0xC3D2E1F0u);
}

TEST(SHA1ContextTest, Reset) {
    SHA1Context ctx;
    ctx.finalized = true;
    ctx.state[0] = 0;
    
    ctx.reset();
    
    EXPECT_FALSE(ctx.isFinalized());
    EXPECT_EQ(ctx.state[0], 0x67452301u);
}

// ============================================================================
// HashResult Tests
// ============================================================================

TEST(HashResultTest, FromCRC16) {
    HashResult result(static_cast<uint16_t>(0x1234));
    
    EXPECT_TRUE(result.isValid());
    EXPECT_EQ(result.algorithm, HashAlgorithm::CRC16);
    EXPECT_EQ(result.length, 2u);
    EXPECT_EQ(result.asCRC16(), 0x1234);
}

TEST(HashResultTest, FromMD4) {
    MD4Digest md4;
    md4.bytes[0] = 0xAA;
    md4.bytes[15] = 0xBB;
    
    HashResult result(md4);
    
    EXPECT_EQ(result.algorithm, HashAlgorithm::MD4);
    EXPECT_EQ(result.length, 16u);
    EXPECT_EQ(result.bytes[0], 0xAA);
    EXPECT_EQ(result.bytes[15], 0xBB);
}

TEST(HashResultTest, FromSHA1) {
    SHA1Digest sha1;
    sha1.bytes[0] = 0x12;
    
    HashResult result(sha1);
    
    EXPECT_EQ(result.algorithm, HashAlgorithm::SHA1);
    EXPECT_EQ(result.length, 20u);
}

TEST(HashResultTest, ToHex) {
    HashResult result(static_cast<uint16_t>(0xABCD));
    
    std::string hex = result.toHex();
    EXPECT_EQ(hex, "cdab");  // Little-endian
}

TEST(HashResultTest, Equality) {
    HashResult a(static_cast<uint16_t>(0x1234));
    HashResult b(static_cast<uint16_t>(0x1234));
    HashResult c(static_cast<uint16_t>(0x5678));
    
    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

// ============================================================================
// FileChecksum Tests
// ============================================================================

TEST(FileChecksumTest, Construction) {
    HashResult hash(static_cast<uint16_t>(0x1234));
    FileChecksum checksum("test.bsp", hash, 1024);
    
    EXPECT_TRUE(checksum.isValid());
    EXPECT_EQ(checksum.filename, "test.bsp");
    EXPECT_EQ(checksum.fileSize, 1024u);
}

TEST(FileChecksumTest, Matches) {
    HashResult hash1(static_cast<uint16_t>(0x1234));
    HashResult hash2(static_cast<uint16_t>(0x1234));
    HashResult hash3(static_cast<uint16_t>(0x5678));
    
    FileChecksum checksum("file.dat", hash1);
    
    EXPECT_TRUE(checksum.matches(hash2));
    EXPECT_FALSE(checksum.matches(hash3));
}

// ============================================================================
// KnownChecksum Tests
// ============================================================================

TEST(KnownChecksumTest, Matches) {
    KnownChecksum known{"maps/dm4.bsp", 0x12345678, true};
    
    EXPECT_TRUE(known.matches(0x12345678));
    EXPECT_FALSE(known.matches(0x87654321));
}

// ============================================================================
// ChecksumStatus Tests
// ============================================================================

TEST(ChecksumStatusTest, Names) {
    EXPECT_EQ(checksumStatusName(ChecksumStatus::Unknown), "unknown");
    EXPECT_EQ(checksumStatusName(ChecksumStatus::Verified), "verified");
    EXPECT_EQ(checksumStatusName(ChecksumStatus::Mismatch), "mismatch");
}

// ============================================================================
// ChecksumEntry Tests
// ============================================================================

TEST(ChecksumEntryTest, StatusChecks) {
    ChecksumEntry entry;
    entry.status = ChecksumStatus::Verified;
    
    EXPECT_TRUE(entry.isVerified());
    EXPECT_FALSE(entry.hasMismatch());
    
    entry.status = ChecksumStatus::Mismatch;
    EXPECT_FALSE(entry.isVerified());
    EXPECT_TRUE(entry.hasMismatch());
}

// ============================================================================
// Helper Function Tests
// ============================================================================

TEST(HashHelpersTest, CalculateCRC16Block) {
    std::array<uint8_t, 4> data = {'t', 'e', 's', 't'};
    uint16_t crc = calculateCRC16(data);
    
    // Same data should produce same CRC
    EXPECT_EQ(crc, calculateCRC16(data));
}

TEST(HashHelpersTest, CalculateCRC16String) {
    uint16_t crc = calculateCRC16("test");
    
    EXPECT_NE(crc, 0u);
    EXPECT_EQ(crc, calculateCRC16("test"));
    EXPECT_NE(crc, calculateCRC16("different"));
}

TEST(HashHelpersTest, CompareDigestsConstantTime) {
    MD4Digest a, b;
    EXPECT_TRUE(compareDigestsConstantTime(a, b));
    
    a.bytes[5] = 1;
    EXPECT_FALSE(compareDigestsConstantTime(a, b));
}

TEST(HashHelpersTest, MakeHashResult) {
    auto crcResult = makeHashResult(static_cast<uint16_t>(0x1234));
    EXPECT_EQ(crcResult.algorithm, HashAlgorithm::CRC16);
    
    MD4Digest md4;
    auto md4Result = makeHashResult(md4);
    EXPECT_EQ(md4Result.algorithm, HashAlgorithm::MD4);
    
    SHA1Digest sha1;
    auto sha1Result = makeHashResult(sha1);
    EXPECT_EQ(sha1Result.algorithm, HashAlgorithm::SHA1);
}
