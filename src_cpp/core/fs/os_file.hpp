/**
 * @file os_file.hpp
 * @brief Regular OS file implementation of VirtualFile
 * 
 * Provides file access to regular filesystem files using standard C++ streams.
 */

#pragma once

#include "virtual_file.hpp"
#include <fstream>
#include <mutex>

namespace ezquake::fs {

/**
 * @brief VirtualFile implementation for regular OS files
 * 
 * Uses std::fstream for cross-platform file access.
 * Thread-safe for individual operations (uses mutex).
 */
class OsFile : public VirtualFile {
public:
    /**
     * @brief Open a file
     * @param path Path to the file
     * @param mode Open mode
     * @return File handle, or error
     */
    static FileResult<VirtualFilePtr> open(const std::filesystem::path& path, OpenMode mode);
    
    /**
     * @brief Create a file, failing if it exists
     * @param path Path to the file
     * @return File handle, or error
     */
    static FileResult<VirtualFilePtr> create(const std::filesystem::path& path);
    
    ~OsFile() override;
    
    // VirtualFile interface
    FileResult<size_t> read(std::span<Byte> buffer) override;
    FileResult<size_t> write(std::span<const Byte> data) override;
    FileResult<size_t> seek(int64_t offset, SeekOrigin origin) override;
    FileResult<size_t> tell() const override;
    FileResult<size_t> size() const override;
    FileResult<void> flush() override;
    bool eof() const override;
    bool isOpen() const override;
    FileFlags flags() const override;
    std::string_view path() const override;
    
private:
    OsFile(std::fstream&& stream, std::filesystem::path path, OpenMode mode);
    
    static std::ios_base::openmode toStreamMode(OpenMode mode);
    
    mutable std::mutex mutex_;
    std::fstream stream_;
    std::filesystem::path path_;
    OpenMode mode_;
    mutable size_t cachedSize_ = 0;
    mutable bool sizeValid_ = false;
    bool atEof_ = false;
};

} // namespace ezquake::fs
