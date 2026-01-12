/**
 * @file virtual_file.hpp
 * @brief Virtual file system abstraction for ezQuake
 * 
 * Provides a polymorphic interface for file access that can be backed by:
 * - Regular OS files
 * - Memory buffers
 * - PAK archives
 * - PK3/ZIP archives
 * - Network streams
 */

#pragma once

#include "../types.hpp"
#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <memory>
#include <optional>
#include <variant>
#include <filesystem>

namespace ezquake::fs {

/**
 * @brief Error codes for filesystem operations
 */
enum class FileError {
    None = 0,
    NotFound,           // File does not exist
    AccessDenied,       // Permission denied
    AlreadyExists,      // File already exists (when creating exclusively)
    InvalidPath,        // Path is malformed
    IsDirectory,        // Expected file, got directory
    NotDirectory,       // Expected directory, got file
    ReadError,          // I/O error during read
    WriteError,         // I/O error during write
    SeekError,          // Invalid seek position
    EndOfFile,          // Attempted to read past end
    NotOpen,            // File handle not open
    UnsupportedOperation, // Operation not supported for this file type
    ArchiveError,       // Error in archive (PAK/PK3) format
    CompressionError,   // Decompression failed
    OutOfMemory,        // Memory allocation failed
    Unknown             // Unspecified error
};

/**
 * @brief Convert FileError to human-readable string
 */
constexpr std::string_view fileErrorToString(FileError error) noexcept {
    switch (error) {
        case FileError::None:              return "No error";
        case FileError::NotFound:          return "File not found";
        case FileError::AccessDenied:      return "Access denied";
        case FileError::AlreadyExists:     return "File already exists";
        case FileError::InvalidPath:       return "Invalid path";
        case FileError::IsDirectory:       return "Is a directory";
        case FileError::NotDirectory:      return "Not a directory";
        case FileError::ReadError:         return "Read error";
        case FileError::WriteError:        return "Write error";
        case FileError::SeekError:         return "Seek error";
        case FileError::EndOfFile:         return "End of file";
        case FileError::NotOpen:           return "File not open";
        case FileError::UnsupportedOperation: return "Unsupported operation";
        case FileError::ArchiveError:      return "Archive format error";
        case FileError::CompressionError:  return "Decompression error";
        case FileError::OutOfMemory:       return "Out of memory";
        case FileError::Unknown:           return "Unknown error";
    }
    return "Unknown error";
}

/**
 * @brief Unexpected error wrapper (like std::unexpected for C++23)
 */
template<typename E>
class Unexpected {
public:
    explicit constexpr Unexpected(E error) : error_(error) {}
    constexpr E error() const { return error_; }
private:
    E error_;
};

/**
 * @brief Result type for filesystem operations (C++20 compatible std::expected alternative)
 */
template<typename T, typename E = FileError>
class Expected {
public:
    // Success constructors
    Expected(const T& value) : data_(value) {}
    Expected(T&& value) : data_(std::move(value)) {}
    
    // Error constructor
    Expected(Unexpected<E> error) : data_(error.error()) {}
    
    // Check if has value
    bool has_value() const { return std::holds_alternative<T>(data_); }
    explicit operator bool() const { return has_value(); }
    
    // Access value
    T& value() { return std::get<T>(data_); }
    const T& value() const { return std::get<T>(data_); }
    
    T& operator*() { return value(); }
    const T& operator*() const { return value(); }
    
    T* operator->() { return &value(); }
    const T* operator->() const { return &value(); }
    
    // Access error
    E error() const { return std::get<E>(data_); }
    
private:
    std::variant<T, E> data_;
};

/**
 * @brief Specialization for void results
 */
template<typename E>
class Expected<void, E> {
public:
    Expected() : hasValue_(true) {}
    Expected(Unexpected<E> error) : error_(error.error()), hasValue_(false) {}
    
    bool has_value() const { return hasValue_; }
    explicit operator bool() const { return hasValue_; }
    
    E error() const { return error_; }
    
private:
    E error_ = E{};
    bool hasValue_ = false;
};

/**
 * @brief Helper to create unexpected error
 */
template<typename E>
Unexpected<E> unexpected(E error) {
    return Unexpected<E>(error);
}

/**
 * @brief Result type for filesystem operations
 */
template<typename T>
using FileResult = Expected<T, FileError>;

/**
 * @brief Seek origin for file positioning
 */
enum class SeekOrigin {
    Begin,      // From start of file
    Current,    // From current position
    End         // From end of file
};

/**
 * @brief File open modes
 */
enum class OpenMode {
    Read,           // Open for reading only
    Write,          // Open for writing (truncate if exists)
    Append,         // Open for appending
    ReadWrite,      // Open for reading and writing
    Create          // Create new file (fail if exists)
};

/**
 * @brief Flags for file attributes
 */
enum class FileFlags : uint32_t {
    None            = 0,
    CopyProtected   = 1 << 0,   // File is in a PAK (can't be easily copied)
    Compressed      = 1 << 1,   // File is compressed
    ReadOnly        = 1 << 2,   // File cannot be written
    InArchive       = 1 << 3,   // File is inside an archive
    Temporary       = 1 << 4,   // Temporary file
    NoSeek          = 1 << 5    // Seeking is not efficient (e.g., network)
};

constexpr FileFlags operator|(FileFlags a, FileFlags b) noexcept {
    return static_cast<FileFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

constexpr FileFlags operator&(FileFlags a, FileFlags b) noexcept {
    return static_cast<FileFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

constexpr bool hasFlag(FileFlags flags, FileFlags flag) noexcept {
    return (static_cast<uint32_t>(flags) & static_cast<uint32_t>(flag)) != 0;
}

/**
 * @brief Abstract base class for virtual file access
 * 
 * All file-like objects (OS files, PAK entries, memory buffers, network streams)
 * implement this interface for uniform access.
 */
class VirtualFile {
public:
    virtual ~VirtualFile() = default;
    
    // Disable copy (files are unique handles)
    VirtualFile(const VirtualFile&) = delete;
    VirtualFile& operator=(const VirtualFile&) = delete;
    
    // Allow move
    VirtualFile(VirtualFile&&) = default;
    VirtualFile& operator=(VirtualFile&&) = default;
    
    /**
     * @brief Read bytes from the file
     * @param buffer Destination buffer
     * @return Number of bytes actually read, or error
     */
    virtual FileResult<size_t> read(std::span<Byte> buffer) = 0;
    
    /**
     * @brief Write bytes to the file
     * @param data Source data
     * @return Number of bytes actually written, or error
     */
    virtual FileResult<size_t> write(std::span<const Byte> data) = 0;
    
    /**
     * @brief Seek to a position in the file
     * @param offset Offset from origin
     * @param origin Reference point for offset
     * @return New position, or error
     */
    virtual FileResult<size_t> seek(int64_t offset, SeekOrigin origin) = 0;
    
    /**
     * @brief Get current position in the file
     * @return Current position, or error
     */
    virtual FileResult<size_t> tell() const = 0;
    
    /**
     * @brief Get total size of the file
     * @return File size in bytes, or error
     */
    virtual FileResult<size_t> size() const = 0;
    
    /**
     * @brief Flush any buffered writes to storage
     * @return Success or error
     */
    virtual FileResult<void> flush() = 0;
    
    /**
     * @brief Check if end of file has been reached
     */
    virtual bool eof() const = 0;
    
    /**
     * @brief Check if file is open and valid
     */
    virtual bool isOpen() const = 0;
    
    /**
     * @brief Get file attribute flags
     */
    virtual FileFlags flags() const = 0;
    
    /**
     * @brief Get the file path (if known)
     */
    virtual std::string_view path() const = 0;
    
    // Convenience methods
    
    /**
     * @brief Read a single line (up to newline or EOF)
     * @param maxLength Maximum line length
     * @return Line without trailing newline, or error
     */
    FileResult<std::string> readLine(size_t maxLength = 4096);
    
    /**
     * @brief Read entire file into a vector
     * @param maxSize Maximum size to read (0 = unlimited)
     * @return File contents, or error
     */
    FileResult<std::vector<Byte>> readAll(size_t maxSize = 0);
    
    /**
     * @brief Read entire file as a string
     * @param maxSize Maximum size to read (0 = unlimited)
     * @return File contents as string, or error
     */
    FileResult<std::string> readAllText(size_t maxSize = 0);
    
    /**
     * @brief Write a string to the file
     * @param text String to write
     * @return Number of bytes written, or error
     */
    FileResult<size_t> writeText(std::string_view text);
    
    /**
     * @brief Check if file is copy protected (in a PAK)
     */
    bool isCopyProtected() const { return hasFlag(flags(), FileFlags::CopyProtected); }
    
    /**
     * @brief Check if file supports efficient seeking
     */
    bool canSeek() const { return !hasFlag(flags(), FileFlags::NoSeek); }
    
    /**
     * @brief Check if file is read-only
     */
    bool isReadOnly() const { return hasFlag(flags(), FileFlags::ReadOnly); }
    
protected:
    VirtualFile() = default;
};

/**
 * @brief Unique pointer to a virtual file
 */
using VirtualFilePtr = std::unique_ptr<VirtualFile>;

/**
 * @brief Information about a located file
 */
struct FileLocation {
    std::filesystem::path fullPath;     // Full path to the file or containing archive
    std::string inArchivePath;          // Path within archive (empty if not in archive)
    size_t fileSize = 0;                // File size in bytes
    FileFlags flags = FileFlags::None;  // File attributes
    
    bool isInArchive() const { return !inArchivePath.empty(); }
};

} // namespace ezquake::fs
