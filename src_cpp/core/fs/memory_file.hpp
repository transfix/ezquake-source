/**
 * @file memory_file.hpp
 * @brief In-memory file implementation of VirtualFile
 * 
 * Provides file-like access to memory buffers. Useful for:
 * - Decompressed PAK/PK3 data
 * - Network received data
 * - Temporary files
 * - Unit testing
 */

#pragma once

#include "virtual_file.hpp"
#include <vector>
#include <mutex>

namespace ezquake::fs {

/**
 * @brief VirtualFile implementation backed by a memory buffer
 * 
 * Can be created from:
 * - Empty buffer (for writing)
 * - Existing data (copy or move)
 * - Span (read-only view)
 * 
 * Thread-safe for individual operations.
 */
class MemoryFile : public VirtualFile {
public:
    /**
     * @brief Create an empty memory file for writing
     * @param name Optional name for the file
     * @param reserveSize Initial capacity to reserve
     */
    static VirtualFilePtr create(std::string name = "", size_t reserveSize = 0);
    
    /**
     * @brief Create a memory file from existing data (copy)
     * @param data Data to copy into the file
     * @param name Optional name for the file
     */
    static VirtualFilePtr fromData(std::span<const Byte> data, std::string name = "");
    
    /**
     * @brief Create a memory file from existing data (move)
     * @param data Data to move into the file
     * @param name Optional name for the file
     */
    static VirtualFilePtr fromVector(std::vector<Byte>&& data, std::string name = "");
    
    /**
     * @brief Create a memory file from a string (copy)
     * @param text String to copy into the file
     * @param name Optional name for the file
     */
    static VirtualFilePtr fromString(std::string_view text, std::string name = "");
    
    /**
     * @brief Create a read-only memory file viewing external data
     * @param data Data to view (must outlive the file)
     * @param name Optional name for the file
     * 
     * WARNING: The caller must ensure the data remains valid for the
     * lifetime of the MemoryFile!
     */
    static VirtualFilePtr fromView(std::span<const Byte> data, std::string name = "");
    
    ~MemoryFile() override = default;
    
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
    
    /**
     * @brief Get read-only access to the underlying buffer
     * @return Span of the buffer contents
     */
    std::span<const Byte> data() const;
    
    /**
     * @brief Detach and return the underlying buffer
     * 
     * After calling this, the file becomes empty.
     * Only works for owned buffers (not views).
     * 
     * @return The buffer, or empty if this is a view
     */
    std::vector<Byte> detach();
    
private:
    MemoryFile(std::vector<Byte>&& data, std::string name, bool readOnly);
    MemoryFile(std::span<const Byte> view, std::string name);
    
    mutable std::mutex mutex_;
    std::vector<Byte> ownedData_;       // Owned data (if not a view)
    std::span<const Byte> viewData_;    // View of external data
    std::string name_;
    size_t position_ = 0;
    bool isView_ = false;               // True if using viewData_
    bool readOnly_ = false;
    bool closed_ = false;
};

} // namespace ezquake::fs
