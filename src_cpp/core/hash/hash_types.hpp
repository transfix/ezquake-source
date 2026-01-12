/**
 * @file hash_types.hpp
 * @brief Hash and checksum type definitions
 * 
 * This module provides types for CRC, MD4, SHA1 and other checksums
 * used for data integrity verification.
 * 
 * Replaces:
 * - crc.h: CRC16 functions
 * - sha1.h: SHA1_CTX
 * - md4.c: MD4_CTX
 */

#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <string_view>
#include <array>
#include <span>
#include <optional>

namespace ezquake::hash {

// ============================================================================
// Constants
// ============================================================================

namespace hash_limits {
    constexpr size_t CRC16_SIZE = 2;
    constexpr size_t MD4_DIGEST_SIZE = 16;
    constexpr size_t SHA1_DIGEST_SIZE = 20;
    constexpr size_t SHA256_DIGEST_SIZE = 32;
    constexpr size_t MAX_DIGEST_SIZE = 64;
}

// ============================================================================
// Hash Algorithm Types
// ============================================================================

/**
 * @brief Supported hash algorithms
 */
enum class HashAlgorithm : uint8_t {
    CRC16,      ///< 16-bit CRC (Quake protocol)
    MD4,        ///< MD4 (legacy Quake)
    SHA1,       ///< SHA-1 (160-bit)
    SHA256,     ///< SHA-256 (256-bit)
    XXHash32,   ///< Fast non-cryptographic hash
    XXHash64    ///< Fast 64-bit non-cryptographic hash
};

/**
 * @brief Get digest size for algorithm
 */
[[nodiscard]] constexpr size_t hashDigestSize(HashAlgorithm algo) noexcept {
    switch (algo) {
        case HashAlgorithm::CRC16:   return hash_limits::CRC16_SIZE;
        case HashAlgorithm::MD4:     return hash_limits::MD4_DIGEST_SIZE;
        case HashAlgorithm::SHA1:    return hash_limits::SHA1_DIGEST_SIZE;
        case HashAlgorithm::SHA256:  return hash_limits::SHA256_DIGEST_SIZE;
        case HashAlgorithm::XXHash32: return 4;
        case HashAlgorithm::XXHash64: return 8;
        default: return 0;
    }
}

/**
 * @brief Get algorithm name
 */
[[nodiscard]] constexpr std::string_view hashAlgorithmName(HashAlgorithm algo) noexcept {
    switch (algo) {
        case HashAlgorithm::CRC16:    return "CRC-16";
        case HashAlgorithm::MD4:      return "MD4";
        case HashAlgorithm::SHA1:     return "SHA-1";
        case HashAlgorithm::SHA256:   return "SHA-256";
        case HashAlgorithm::XXHash32: return "XXHash32";
        case HashAlgorithm::XXHash64: return "XXHash64";
        default: return "Unknown";
    }
}

/**
 * @brief Check if algorithm is cryptographically secure
 * Note: MD4 and SHA1 are considered broken but still used for legacy compatibility
 */
[[nodiscard]] constexpr bool hashIsCryptographic(HashAlgorithm algo) noexcept {
    switch (algo) {
        case HashAlgorithm::SHA256:
            return true;
        default:
            return false;
    }
}

// ============================================================================
// Digest Types
// ============================================================================

/**
 * @brief Fixed-size hash digest
 */
template<size_t N>
struct Digest {
    std::array<uint8_t, N> bytes{};
    
    constexpr Digest() = default;
    
    explicit constexpr Digest(std::span<const uint8_t> data) {
        size_t copyLen = std::min(data.size(), N);
        for (size_t i = 0; i < copyLen; ++i) {
            bytes[i] = data[i];
        }
    }
    
    [[nodiscard]] constexpr size_t size() const noexcept { return N; }
    [[nodiscard]] constexpr const uint8_t* data() const noexcept { return bytes.data(); }
    [[nodiscard]] constexpr uint8_t* data() noexcept { return bytes.data(); }
    
    [[nodiscard]] constexpr uint8_t operator[](size_t i) const noexcept {
        return i < N ? bytes[i] : 0;
    }
    
    [[nodiscard]] constexpr bool isZero() const noexcept {
        for (size_t i = 0; i < N; ++i) {
            if (bytes[i] != 0) return false;
        }
        return true;
    }
    
    constexpr void clear() noexcept {
        bytes.fill(0);
    }
    
    [[nodiscard]] std::string toHex() const {
        static constexpr char hexChars[] = "0123456789abcdef";
        std::string result;
        result.reserve(N * 2);
        for (size_t i = 0; i < N; ++i) {
            result += hexChars[(bytes[i] >> 4) & 0x0F];
            result += hexChars[bytes[i] & 0x0F];
        }
        return result;
    }
    
    [[nodiscard]] std::string toHexUpper() const {
        static constexpr char hexChars[] = "0123456789ABCDEF";
        std::string result;
        result.reserve(N * 2);
        for (size_t i = 0; i < N; ++i) {
            result += hexChars[(bytes[i] >> 4) & 0x0F];
            result += hexChars[bytes[i] & 0x0F];
        }
        return result;
    }
    
    [[nodiscard]] static std::optional<Digest<N>> fromHex(std::string_view hex) {
        if (hex.size() != N * 2) return std::nullopt;
        
        Digest<N> result;
        for (size_t i = 0; i < N; ++i) {
            char hi = hex[i * 2];
            char lo = hex[i * 2 + 1];
            
            uint8_t hiVal, loVal;
            if (hi >= '0' && hi <= '9') hiVal = static_cast<uint8_t>(hi - '0');
            else if (hi >= 'a' && hi <= 'f') hiVal = static_cast<uint8_t>(hi - 'a' + 10);
            else if (hi >= 'A' && hi <= 'F') hiVal = static_cast<uint8_t>(hi - 'A' + 10);
            else return std::nullopt;
            
            if (lo >= '0' && lo <= '9') loVal = static_cast<uint8_t>(lo - '0');
            else if (lo >= 'a' && lo <= 'f') loVal = static_cast<uint8_t>(lo - 'a' + 10);
            else if (lo >= 'A' && lo <= 'F') loVal = static_cast<uint8_t>(lo - 'A' + 10);
            else return std::nullopt;
            
            result.bytes[i] = static_cast<uint8_t>((hiVal << 4) | loVal);
        }
        return result;
    }
    
    constexpr bool operator==(const Digest<N>& other) const noexcept = default;
    constexpr bool operator!=(const Digest<N>& other) const noexcept = default;
};

// Common digest type aliases
using CRC16 = uint16_t;
using MD4Digest = Digest<hash_limits::MD4_DIGEST_SIZE>;
using SHA1Digest = Digest<hash_limits::SHA1_DIGEST_SIZE>;
using SHA256Digest = Digest<hash_limits::SHA256_DIGEST_SIZE>;

// ============================================================================
// CRC16 Context (Quake-style)
// ============================================================================

/**
 * @brief CRC16 calculation context
 * 
 * Replaces C: CRC_Init, CRC_ProcessByte, CRC_Value
 */
struct CRC16Context {
    uint16_t value = 0xFFFF;
    
    constexpr CRC16Context() = default;
    
    /**
     * @brief Reset to initial state
     */
    constexpr void reset() noexcept {
        value = 0xFFFF;
    }
    
    /**
     * @brief Process a single byte
     */
    constexpr void processByte(uint8_t data) noexcept {
        // Standard CRC-16-CCITT polynomial
        value ^= static_cast<uint16_t>(data) << 8;
        for (int i = 0; i < 8; ++i) {
            if (value & 0x8000) {
                value = (value << 1) ^ 0x1021;
            } else {
                value <<= 1;
            }
        }
    }
    
    /**
     * @brief Process a block of data
     */
    constexpr void processBlock(std::span<const uint8_t> data) noexcept {
        for (uint8_t byte : data) {
            processByte(byte);
        }
    }
    
    /**
     * @brief Get final CRC value
     */
    [[nodiscard]] constexpr uint16_t result() const noexcept {
        return value;
    }
};

// ============================================================================
// MD4 Context
// ============================================================================

/**
 * @brief MD4 hash context
 * 
 * Replaces C: MD4_CTX
 */
struct MD4Context {
    std::array<uint32_t, 4> state{0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476};
    std::array<uint32_t, 2> count{0, 0};
    std::array<uint8_t, 64> buffer{};
    bool finalized = false;
    MD4Digest digest{};
    
    MD4Context() = default;
    
    void reset() noexcept {
        state = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476};
        count = {0, 0};
        buffer.fill(0);
        finalized = false;
        digest.clear();
    }
    
    [[nodiscard]] bool isFinalized() const noexcept {
        return finalized;
    }
    
    [[nodiscard]] MD4Digest result() const noexcept {
        return digest;
    }
    
    /**
     * @brief Get current bit count
     */
    [[nodiscard]] uint64_t bitCount() const noexcept {
        return (static_cast<uint64_t>(count[1]) << 32) | count[0];
    }
};

// ============================================================================
// SHA1 Context
// ============================================================================

/**
 * @brief SHA1 hash context
 * 
 * Replaces C: SHA1_CTX
 */
struct SHA1Context {
    std::array<uint32_t, 5> state{0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
    std::array<size_t, 2> count{0, 0};
    std::array<uint8_t, 64> buffer{};
    bool finalized = false;
    SHA1Digest digest{};
    
    SHA1Context() = default;
    
    void reset() noexcept {
        state = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
        count = {0, 0};
        buffer.fill(0);
        finalized = false;
        digest.clear();
    }
    
    [[nodiscard]] bool isFinalized() const noexcept {
        return finalized;
    }
    
    [[nodiscard]] SHA1Digest result() const noexcept {
        return digest;
    }
};

// ============================================================================
// Generic Hash Result
// ============================================================================

/**
 * @brief Generic hash result that can hold any digest type
 */
struct HashResult {
    HashAlgorithm algorithm = HashAlgorithm::CRC16;
    std::array<uint8_t, hash_limits::MAX_DIGEST_SIZE> bytes{};
    size_t length = 0;
    
    HashResult() = default;
    
    explicit HashResult(uint16_t crc)
        : algorithm(HashAlgorithm::CRC16), length(2) {
        bytes[0] = static_cast<uint8_t>(crc & 0xFF);
        bytes[1] = static_cast<uint8_t>((crc >> 8) & 0xFF);
    }
    
    explicit HashResult(const MD4Digest& md4)
        : algorithm(HashAlgorithm::MD4), length(hash_limits::MD4_DIGEST_SIZE) {
        std::copy_n(md4.data(), length, bytes.begin());
    }
    
    explicit HashResult(const SHA1Digest& sha1)
        : algorithm(HashAlgorithm::SHA1), length(hash_limits::SHA1_DIGEST_SIZE) {
        std::copy_n(sha1.data(), length, bytes.begin());
    }
    
    [[nodiscard]] bool isValid() const noexcept {
        return length > 0 && length <= hash_limits::MAX_DIGEST_SIZE;
    }
    
    [[nodiscard]] std::span<const uint8_t> data() const noexcept {
        return {bytes.data(), length};
    }
    
    [[nodiscard]] std::string toHex() const {
        static constexpr char hexChars[] = "0123456789abcdef";
        std::string result;
        result.reserve(length * 2);
        for (size_t i = 0; i < length; ++i) {
            result += hexChars[(bytes[i] >> 4) & 0x0F];
            result += hexChars[bytes[i] & 0x0F];
        }
        return result;
    }
    
    [[nodiscard]] uint16_t asCRC16() const noexcept {
        if (algorithm != HashAlgorithm::CRC16 || length < 2) return 0;
        return static_cast<uint16_t>(bytes[0]) | (static_cast<uint16_t>(bytes[1]) << 8);
    }
    
    bool operator==(const HashResult& other) const noexcept {
        if (algorithm != other.algorithm || length != other.length) return false;
        for (size_t i = 0; i < length; ++i) {
            if (bytes[i] != other.bytes[i]) return false;
        }
        return true;
    }
};

// ============================================================================
// File Checksum
// ============================================================================

/**
 * @brief File checksum with metadata
 */
struct FileChecksum {
    std::string filename;
    HashResult hash;
    size_t fileSize = 0;
    bool verified = false;
    
    FileChecksum() = default;
    
    FileChecksum(std::string_view file, const HashResult& h, size_t size = 0)
        : filename(file), hash(h), fileSize(size) {}
    
    [[nodiscard]] bool isValid() const noexcept {
        return !filename.empty() && hash.isValid();
    }
    
    [[nodiscard]] bool matches(const HashResult& other) const noexcept {
        return hash == other;
    }
};

// ============================================================================
// Model/Map Checksums (Quake-specific)
// ============================================================================

/**
 * @brief Known checksum for integrity verification
 */
struct KnownChecksum {
    const char* name;       ///< Resource name (e.g., "maps/dm4.bsp")
    uint32_t checksum;      ///< Expected CRC/checksum value
    bool required;          ///< Whether mismatch should block
    
    [[nodiscard]] constexpr bool matches(uint32_t value) const noexcept {
        return checksum == value;
    }
};

/**
 * @brief Checksum verification result
 */
enum class ChecksumStatus : uint8_t {
    Unknown,        ///< Checksum not yet computed
    Verified,       ///< Matches expected value
    Mismatch,       ///< Does not match expected
    NotRequired,    ///< No checksum required for this file
    Error           ///< Error computing checksum
};

[[nodiscard]] constexpr std::string_view checksumStatusName(ChecksumStatus status) noexcept {
    switch (status) {
        case ChecksumStatus::Unknown:     return "unknown";
        case ChecksumStatus::Verified:    return "verified";
        case ChecksumStatus::Mismatch:    return "mismatch";
        case ChecksumStatus::NotRequired: return "not required";
        case ChecksumStatus::Error:       return "error";
        default: return "unknown";
    }
}

/**
 * @brief Checksum verification entry
 */
struct ChecksumEntry {
    std::string filename;
    uint32_t computed = 0;
    uint32_t expected = 0;
    ChecksumStatus status = ChecksumStatus::Unknown;
    
    [[nodiscard]] bool isVerified() const noexcept {
        return status == ChecksumStatus::Verified;
    }
    
    [[nodiscard]] bool hasMismatch() const noexcept {
        return status == ChecksumStatus::Mismatch;
    }
};

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * @brief Calculate CRC16 of data block
 */
[[nodiscard]] inline uint16_t calculateCRC16(std::span<const uint8_t> data) noexcept {
    CRC16Context ctx;
    ctx.processBlock(data);
    return ctx.result();
}

/**
 * @brief Calculate CRC16 of string
 */
[[nodiscard]] inline uint16_t calculateCRC16(std::string_view str) noexcept {
    CRC16Context ctx;
    for (char c : str) {
        ctx.processByte(static_cast<uint8_t>(c));
    }
    return ctx.result();
}

/**
 * @brief Compare two digests (constant-time for security)
 */
template<size_t N>
[[nodiscard]] inline bool compareDigestsConstantTime(
    const Digest<N>& a,
    const Digest<N>& b
) noexcept {
    uint8_t result = 0;
    for (size_t i = 0; i < N; ++i) {
        result = static_cast<uint8_t>(result | (a.bytes[i] ^ b.bytes[i]));
    }
    return result == 0;
}

/**
 * @brief Create hash result from CRC16
 */
[[nodiscard]] inline HashResult makeHashResult(uint16_t crc) {
    return HashResult(crc);
}

/**
 * @brief Create hash result from MD4
 */
[[nodiscard]] inline HashResult makeHashResult(const MD4Digest& md4) {
    return HashResult(md4);
}

/**
 * @brief Create hash result from SHA1
 */
[[nodiscard]] inline HashResult makeHashResult(const SHA1Digest& sha1) {
    return HashResult(sha1);
}

} // namespace ezquake::hash
