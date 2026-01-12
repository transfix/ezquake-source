/**
 * @file filesystem.hpp
 * @brief File system manager for ezQuake
 * 
 * Provides the main interface for file access with support for:
 * - Multiple search paths (game directories)
 * - PAK file support
 * - Home directory and base directory
 * - File location and caching
 */

#pragma once

#include "virtual_file.hpp"
#include <vector>
#include <unordered_map>
#include <shared_mutex>
#include <functional>

namespace ezquake::fs {

/**
 * @brief Type of search path entry
 */
enum class SearchPathType {
    Directory,      // Regular filesystem directory
    Pak,            // Quake PAK file
    Pk3,            // Quake 3 PK3/ZIP file
    Pk4             // Quake 4 PK4 file
};

/**
 * @brief A search path entry
 */
struct SearchPath {
    std::filesystem::path path;     // Path to directory or archive
    SearchPathType type = SearchPathType::Directory;
    bool copyProtected = false;     // Files from PAKs are copy protected
    bool isTemporary = false;       // Temporary search path
    int priority = 0;               // Higher priority = searched first
    
    bool operator<(const SearchPath& other) const {
        return priority > other.priority;  // Higher priority first
    }
};

/**
 * @brief Options for file searches
 */
struct SearchOptions {
    bool includePaks = true;        // Search in PAK files
    bool cacheResults = true;       // Cache file locations
    bool homeOnly = false;          // Only search home directory
    bool baseOnly = false;          // Only search base directory
};

/**
 * @brief Relative path types (matching original fs.h)
 */
enum class RelativeTo {
    None,       // File name used as-is (absolute path)
    GameOs,     // Relative to game directory, OS files only
    Base,       // Relative to base/home directory
    Home,       // Relative to home directory only
    Pak,        // Only search in PAK files
    Any         // Search everywhere including PAKs
};

/**
 * @brief File system manager
 * 
 * Manages search paths and provides file access. Unlike the original
 * C implementation, this is designed for dependency injection rather
 * than global state.
 * 
 * Thread-safe for concurrent access.
 */
class FileSystem {
public:
    FileSystem();
    ~FileSystem();
    
    // Not copyable or movable due to mutex
    FileSystem(const FileSystem&) = delete;
    FileSystem& operator=(const FileSystem&) = delete;
    FileSystem(FileSystem&&) = delete;
    FileSystem& operator=(FileSystem&&) = delete;
    
    /**
     * @brief Initialize the file system with base and home directories
     * @param baseDir Base game directory (e.g., /usr/share/quake)
     * @param homeDir User home directory (e.g., ~/.ezquake)
     * @param gameDir Game subdirectory (e.g., "qw")
     */
    void init(const std::filesystem::path& baseDir,
              const std::filesystem::path& homeDir,
              std::string_view gameDir = "qw");
    
    /**
     * @brief Shut down and release all resources
     */
    void shutdown();
    
    // === File Access ===
    
    /**
     * @brief Open a file for reading
     * @param filename Filename to open (relative to search paths)
     * @param relativeTo Where to search for the file
     * @return File handle, or error
     */
    FileResult<VirtualFilePtr> openRead(std::string_view filename,
                                         RelativeTo relativeTo = RelativeTo::Any);
    
    /**
     * @brief Open a file for writing
     * @param filename Filename to open (relative to game directory)
     * @param append If true, append to existing file
     * @return File handle, or error
     */
    FileResult<VirtualFilePtr> openWrite(std::string_view filename,
                                          bool append = false);
    
    /**
     * @brief Load an entire file into memory
     * @param filename Filename to load
     * @param relativeTo Where to search for the file
     * @return File contents, or error
     */
    FileResult<std::vector<Byte>> loadFile(std::string_view filename,
                                            RelativeTo relativeTo = RelativeTo::Any);
    
    /**
     * @brief Load an entire file as text
     * @param filename Filename to load
     * @param relativeTo Where to search for the file
     * @return File contents as string, or error
     */
    FileResult<std::string> loadText(std::string_view filename,
                                      RelativeTo relativeTo = RelativeTo::Any);
    
    /**
     * @brief Save data to a file
     * @param filename Filename to save
     * @param data Data to write
     * @return Number of bytes written, or error
     */
    FileResult<size_t> saveFile(std::string_view filename,
                                 std::span<const Byte> data);
    
    /**
     * @brief Save text to a file
     * @param filename Filename to save
     * @param text Text to write
     * @return Number of bytes written, or error
     */
    FileResult<size_t> saveText(std::string_view filename,
                                 std::string_view text);
    
    // === File Location ===
    
    /**
     * @brief Locate a file without opening it
     * @param filename Filename to find
     * @param relativeTo Where to search
     * @return File location info, or nullopt if not found
     */
    std::optional<FileLocation> locate(std::string_view filename,
                                        RelativeTo relativeTo = RelativeTo::Any);
    
    /**
     * @brief Check if a file exists
     * @param filename Filename to check
     * @param relativeTo Where to search
     */
    bool exists(std::string_view filename,
                RelativeTo relativeTo = RelativeTo::Any);
    
    /**
     * @brief Get the full path to a file
     * @param filename Filename to resolve
     * @param relativeTo Where to search
     * @return Full path, or empty if not found
     */
    std::filesystem::path fullPath(std::string_view filename,
                                    RelativeTo relativeTo = RelativeTo::Any);
    
    // === Search Paths ===
    
    /**
     * @brief Add a search path
     * @param path Path to add
     * @param type Type of search path
     * @param priority Priority (higher = searched first)
     */
    void addSearchPath(const std::filesystem::path& path,
                        SearchPathType type = SearchPathType::Directory,
                        int priority = 0);
    
    /**
     * @brief Remove a search path
     * @param path Path to remove
     */
    void removeSearchPath(const std::filesystem::path& path);
    
    /**
     * @brief Clear all search paths
     */
    void clearSearchPaths();
    
    /**
     * @brief Get the current search paths
     */
    std::vector<SearchPath> searchPaths() const;
    
    // === Directory Properties ===
    
    /**
     * @brief Get the base directory
     */
    const std::filesystem::path& baseDir() const { return baseDir_; }
    
    /**
     * @brief Get the home directory
     */
    const std::filesystem::path& homeDir() const { return homeDir_; }
    
    /**
     * @brief Get the current game directory
     */
    const std::filesystem::path& gameDir() const { return gameDir_; }
    
    /**
     * @brief Get the game directory name (e.g., "qw")
     */
    std::string_view gameDirName() const { return gameDirName_; }
    
    // === Cache Management ===
    
    /**
     * @brief Enable or disable file location caching
     */
    void setCacheEnabled(bool enabled);
    
    /**
     * @brief Check if caching is enabled
     */
    bool isCacheEnabled() const;
    
    /**
     * @brief Clear the file location cache
     */
    void flushCache();
    
    /**
     * @brief Notify that the filesystem has changed
     * 
     * Called when files are added/removed to invalidate caches.
     */
    void notifyChanged();
    
    // === File Enumeration ===
    
    /**
     * @brief Enumerate files matching a pattern
     * @param pattern Glob pattern (e.g., "*.cfg")
     * @param callback Function to call for each file
     * @param relativeTo Where to search
     */
    void enumerate(std::string_view pattern,
                   std::function<void(const std::filesystem::path&)> callback,
                   RelativeTo relativeTo = RelativeTo::Any);
    
    // === Utilities ===
    
    /**
     * @brief Check if a filename is unsafe (contains .. or absolute paths)
     */
    static bool isUnsafeFilename(std::string_view filename);
    
    /**
     * @brief Sanitize a filename for safe access
     * @param filename Filename to sanitize
     * @return Sanitized filename, or empty if completely invalid
     */
    static std::string sanitizeFilename(std::string_view filename);
    
    /**
     * @brief Get the file extension (lowercase, without dot)
     */
    static std::string getExtension(std::string_view filename);
    
    /**
     * @brief Create all directories in a path
     */
    static FileResult<void> createPath(const std::filesystem::path& path);
    
private:
    // Search for a file in all search paths
    std::optional<FileLocation> searchFile(std::string_view filename,
                                            const SearchOptions& options);
    
    // Build full path for a relative file
    std::filesystem::path buildPath(std::string_view filename,
                                     RelativeTo relativeTo);
    
    mutable std::shared_mutex mutex_;
    
    std::filesystem::path baseDir_;
    std::filesystem::path homeDir_;
    std::filesystem::path gameDir_;     // Full path to game directory
    std::string gameDirName_;           // Just the game directory name
    
    std::vector<SearchPath> searchPaths_;
    
    // File location cache
    bool cacheEnabled_ = true;
    mutable std::unordered_map<std::string, FileLocation> locationCache_;
    
    bool initialized_ = false;
};

} // namespace ezquake::fs
