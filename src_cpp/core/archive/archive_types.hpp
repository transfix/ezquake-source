/**
 * @file archive_types.hpp
 * @brief Archive and Virtual Filesystem type definitions
 *
 * Converted from:
 * - wad.h - WAD file types
 * - vfs.h - Virtual filesystem types (pak, zip, search paths)
 */

#pragma once

#include <array>
#include <cstdint>
#include <string_view>
#include <optional>

namespace ezquake::archive {

// =============================================================================
// Archive Constants
// =============================================================================

namespace archive_limits {
    /// Maximum path length
    inline constexpr std::size_t MAX_QPATH = 64;
    
    /// Maximum OS path length
    inline constexpr std::size_t MAX_OSPATH = 256;
    
    /// WAD magic numbers
    inline constexpr uint32_t WAD2_MAGIC = 0x32444157; // "WAD2"
    inline constexpr uint32_t WAD3_MAGIC = 0x33444157; // "WAD3"
    
    /// PAK magic
    inline constexpr uint32_t PAK_MAGIC = 0x4B434150;  // "PACK"
    
    /// Maximum WAD lumps
    inline constexpr std::size_t MAX_WAD_LUMPS = 4096;
    
    /// Maximum pack files
    inline constexpr std::size_t MAX_PACK_FILES = 16384;
}

// =============================================================================
// WAD Compression Types
// =============================================================================

/**
 * @brief WAD compression method
 */
enum class WadCompression : uint8_t {
    None = 0,
    LZSS = 1
};

// =============================================================================
// WAD Lump Types
// =============================================================================

/**
 * @brief WAD lump type identifiers
 *
 * From wad.h: TYP_* defines
 */
enum class WadLumpType : uint8_t {
    None = 0,       // No type
    Label = 1,      // Label only
    
    // Lumpy types (64+)
    Palette = 64,   // Color palette
    QTex = 65,      // Quake texture
    QPic = 66,      // Quake picture
    Sound = 67,     // Sound data
    MipTex = 68     // Mipmap texture
};

/**
 * @brief Get display name for lump type
 */
[[nodiscard]] constexpr const char* getLumpTypeName(WadLumpType type) noexcept {
    switch (type) {
        case WadLumpType::None:    return "None";
        case WadLumpType::Label:   return "Label";
        case WadLumpType::Palette: return "Palette";
        case WadLumpType::QTex:    return "QTex";
        case WadLumpType::QPic:    return "QPic";
        case WadLumpType::Sound:   return "Sound";
        case WadLumpType::MipTex:  return "MipTex";
        default:                   return "Unknown";
    }
}

/**
 * @brief Check if type is a lumpy type (64+)
 */
[[nodiscard]] constexpr bool isLumpyType(WadLumpType type) noexcept {
    return static_cast<uint8_t>(type) >= 64;
}

// =============================================================================
// WAD Lump Info
// =============================================================================

/**
 * @brief WAD file lump/entry information
 */
struct WadLump {
    int32_t filePos = 0;        // Position in WAD file
    int32_t diskSize = 0;       // Size on disk (compressed)
    int32_t size = 0;           // Uncompressed size
    WadLumpType type = WadLumpType::None;
    WadCompression compression = WadCompression::None;
    std::array<char, 16> name{};    // Lump name (null-terminated)
    
    /// Get name as string_view
    [[nodiscard]] std::string_view getName() const noexcept {
        return std::string_view(name.data());
    }
    
    /// Check if compressed
    [[nodiscard]] constexpr bool isCompressed() const noexcept {
        return compression != WadCompression::None;
    }
    
    /// Get compression ratio
    [[nodiscard]] constexpr float compressionRatio() const noexcept {
        if (size == 0) return 1.0f;
        return static_cast<float>(diskSize) / static_cast<float>(size);
    }
};

// =============================================================================
// WAD Header
// =============================================================================

/**
 * @brief WAD file header
 */
struct WadHeader {
    uint32_t magic = 0;         // "WAD2" or "WAD3"
    int32_t numLumps = 0;       // Number of lumps
    int32_t tableOffset = 0;    // Offset to lump table
    
    /// Check if valid WAD2 file
    [[nodiscard]] constexpr bool isWad2() const noexcept {
        return magic == archive_limits::WAD2_MAGIC;
    }
    
    /// Check if valid WAD3 file
    [[nodiscard]] constexpr bool isWad3() const noexcept {
        return magic == archive_limits::WAD3_MAGIC;
    }
    
    /// Check if any valid WAD
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return isWad2() || isWad3();
    }
    
    /// Get version number (2 or 3)
    [[nodiscard]] constexpr int version() const noexcept {
        if (isWad2()) return 2;
        if (isWad3()) return 3;
        return 0;
    }
};

// =============================================================================
// QPic Structure
// =============================================================================

/**
 * @brief Quake picture header
 *
 * From wad.h: qpic_t
 */
struct QPicHeader {
    int32_t width = 0;
    int32_t height = 0;
    
    /// Get total pixels
    [[nodiscard]] constexpr int32_t totalPixels() const noexcept {
        return width * height;
    }
    
    /// Get data size in bytes (8-bit indexed)
    [[nodiscard]] constexpr std::size_t dataSize() const noexcept {
        return static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
    }
    
    /// Check if valid dimensions
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return width > 0 && height > 0 && width <= 2048 && height <= 2048;
    }
};

// =============================================================================
// Pack File Entry
// =============================================================================

/**
 * @brief Pack file entry (file inside .pak)
 *
 * From vfs.h: packfile_t
 */
struct PackFile {
    std::array<char, archive_limits::MAX_QPATH> name{};
    int32_t filePos = 0;    // Offset in pak file
    int32_t fileLen = 0;    // File length
    
    /// Get name as string_view
    [[nodiscard]] std::string_view getName() const noexcept {
        return std::string_view(name.data());
    }
    
    /// Get end position in pak
    [[nodiscard]] constexpr int32_t endPos() const noexcept {
        return filePos + fileLen;
    }
};

// =============================================================================
// Pack Header
// =============================================================================

/**
 * @brief PAK file header
 */
struct PackHeader {
    uint32_t magic = 0;         // "PACK"
    int32_t tableOffset = 0;    // Offset to file table
    int32_t tableSize = 0;      // Size of file table
    
    /// Check if valid PAK header
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return magic == archive_limits::PAK_MAGIC;
    }
    
    /// Get number of files
    [[nodiscard]] constexpr int32_t numFiles() const noexcept {
        // PackFile entry is 64 bytes (56 name + 4 pos + 4 len)
        return tableSize / 64;
    }
};

// =============================================================================
// File Location
// =============================================================================

/**
 * @brief Located file in virtual filesystem
 *
 * From vfs.h: flocation_t
 */
struct FileLocation {
    int32_t searchPathIndex = -1;   // Index of search path
    int32_t index = 0;              // Index within archive
    std::array<char, archive_limits::MAX_OSPATH> rawName{};
    int32_t offset = 0;             // Offset in file/archive
    int32_t length = 0;             // File length
    
    /// Check if location is valid
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return searchPathIndex >= 0 && length > 0;
    }
    
    /// Check if it's a raw file (not in archive)
    [[nodiscard]] bool isRawFile() const noexcept {
        return rawName[0] != '\0';
    }
    
    /// Get raw name as string_view
    [[nodiscard]] std::string_view getRawName() const noexcept {
        return std::string_view(rawName.data());
    }
};

// =============================================================================
// Search Path Flags
// =============================================================================

namespace SearchPathFlags {
    inline constexpr uint32_t None           = 0;
    inline constexpr uint32_t CopyProtected  = 1u << 0;  // No downloads
    inline constexpr uint32_t Temporary      = 1u << 1;  // Temporary path
    inline constexpr uint32_t Pure           = 1u << 2;  // Pure server path
    inline constexpr uint32_t WriteEnabled   = 1u << 3;  // Can write files
    inline constexpr uint32_t GameDir        = 1u << 4;  // Game directory
    inline constexpr uint32_t BaseDir        = 1u << 5;  // Base game directory
}

// =============================================================================
// Archive Type
// =============================================================================

/**
 * @brief Type of archive/search path
 */
enum class ArchiveType : uint8_t {
    Directory = 0,  // Filesystem directory
    Pak,            // Quake PAK file
    Zip,            // ZIP/PK3 archive
    Wad,            // WAD file
    Gzip,           // Gzip compressed
    DoomWad         // Doom WAD format
};

/**
 * @brief Get display name for archive type
 */
[[nodiscard]] constexpr const char* getArchiveTypeName(ArchiveType type) noexcept {
    switch (type) {
        case ArchiveType::Directory: return "Directory";
        case ArchiveType::Pak:       return "PAK";
        case ArchiveType::Zip:       return "ZIP";
        case ArchiveType::Wad:       return "WAD";
        case ArchiveType::Gzip:      return "GZip";
        case ArchiveType::DoomWad:   return "DoomWAD";
        default:                     return "Unknown";
    }
}

/**
 * @brief Get typical file extension for archive type
 */
[[nodiscard]] constexpr const char* getArchiveExtension(ArchiveType type) noexcept {
    switch (type) {
        case ArchiveType::Pak:  return ".pak";
        case ArchiveType::Zip:  return ".pk3";
        case ArchiveType::Wad:  return ".wad";
        case ArchiveType::Gzip: return ".gz";
        default:                return "";
    }
}

// =============================================================================
// Search Path Info
// =============================================================================

/**
 * @brief Search path information
 */
struct SearchPathInfo {
    ArchiveType type = ArchiveType::Directory;
    uint32_t flags = SearchPathFlags::None;
    int32_t crcCheck = 0;       // CRC for validation
    int32_t crcReply = 0;       // CRC sent to server
    std::array<char, archive_limits::MAX_OSPATH> path{};
    
    /// Get path as string_view
    [[nodiscard]] std::string_view getPath() const noexcept {
        return std::string_view(path.data());
    }
    
    /// Check if copy-protected
    [[nodiscard]] constexpr bool isCopyProtected() const noexcept {
        return (flags & SearchPathFlags::CopyProtected) != 0;
    }
    
    /// Check if temporary
    [[nodiscard]] constexpr bool isTemporary() const noexcept {
        return (flags & SearchPathFlags::Temporary) != 0;
    }
    
    /// Check if writable
    [[nodiscard]] constexpr bool isWritable() const noexcept {
        return (flags & SearchPathFlags::WriteEnabled) != 0;
    }
};

// =============================================================================
// Mipmap Texture
// =============================================================================

/**
 * @brief Mipmap texture header (in WAD)
 */
struct MipTexHeader {
    std::array<char, 16> name{};
    uint32_t width = 0;
    uint32_t height = 0;
    std::array<uint32_t, 4> offsets{};  // Offsets to 4 mip levels
    
    /// Get name as string_view
    [[nodiscard]] std::string_view getName() const noexcept {
        return std::string_view(name.data());
    }
    
    /// Get mip level dimensions
    [[nodiscard]] constexpr uint32_t mipWidth(int level) const noexcept {
        return width >> level;
    }
    
    [[nodiscard]] constexpr uint32_t mipHeight(int level) const noexcept {
        return height >> level;
    }
    
    /// Get mip level size in bytes
    [[nodiscard]] constexpr std::size_t mipSize(int level) const noexcept {
        return static_cast<std::size_t>(mipWidth(level)) * mipHeight(level);
    }
    
    /// Get total size of all mip levels
    [[nodiscard]] constexpr std::size_t totalMipSize() const noexcept {
        std::size_t total = 0;
        for (int i = 0; i < 4; ++i) {
            total += mipSize(i);
        }
        return total;
    }
    
    /// Check if valid texture
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return width > 0 && height > 0 && 
               (width & (width - 1)) == 0 &&  // Power of 2
               (height & (height - 1)) == 0;
    }
};

// =============================================================================
// File Open Mode
// =============================================================================

/**
 * @brief File open mode flags
 */
enum class FileOpenMode : uint8_t {
    Read = 0,       // Read only
    Write,          // Write only (create/truncate)
    Append,         // Append to existing
    ReadWrite       // Read and write
};

/**
 * @brief Get mode string for fopen
 */
[[nodiscard]] constexpr const char* getModeString(FileOpenMode mode) noexcept {
    switch (mode) {
        case FileOpenMode::Read:      return "rb";
        case FileOpenMode::Write:     return "wb";
        case FileOpenMode::Append:    return "ab";
        case FileOpenMode::ReadWrite: return "r+b";
        default:                      return "rb";
    }
}

// =============================================================================
// VFS Error Codes
// =============================================================================

/**
 * @brief Virtual filesystem error codes
 */
enum class VfsError : uint8_t {
    None = 0,
    FileNotFound,
    AccessDenied,
    InvalidPath,
    CorruptArchive,
    CompressionError,
    OutOfMemory,
    EndOfFile,
    Unknown
};

/**
 * @brief Get error description
 */
[[nodiscard]] constexpr const char* getVfsErrorString(VfsError err) noexcept {
    switch (err) {
        case VfsError::None:             return "No error";
        case VfsError::FileNotFound:     return "File not found";
        case VfsError::AccessDenied:     return "Access denied";
        case VfsError::InvalidPath:      return "Invalid path";
        case VfsError::CorruptArchive:   return "Corrupt archive";
        case VfsError::CompressionError: return "Compression error";
        case VfsError::OutOfMemory:      return "Out of memory";
        case VfsError::EndOfFile:        return "End of file";
        default:                         return "Unknown error";
    }
}

// =============================================================================
// File Hash Entry
// =============================================================================

/**
 * @brief Hash table entry for file lookup
 */
struct FileHashEntry {
    uint32_t hash = 0;          // Name hash
    int32_t pakIndex = -1;      // Index in pak file
    int32_t searchPathIndex = -1; // Search path index
    
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return pakIndex >= 0 && searchPathIndex >= 0;
    }
};

// =============================================================================
// Archive Stats
// =============================================================================

/**
 * @brief Archive statistics
 */
struct ArchiveStats {
    std::size_t fileCount = 0;
    std::size_t totalSize = 0;
    std::size_t compressedSize = 0;
    ArchiveType type = ArchiveType::Directory;
    
    /// Get compression ratio
    [[nodiscard]] constexpr float compressionRatio() const noexcept {
        if (totalSize == 0) return 1.0f;
        return static_cast<float>(compressedSize) / static_cast<float>(totalSize);
    }
    
    /// Get space saved by compression
    [[nodiscard]] constexpr std::size_t spaceSaved() const noexcept {
        return totalSize > compressedSize ? totalSize - compressedSize : 0;
    }
};

} // namespace ezquake::archive
