/**
 * @file filesystem.cpp
 * @brief Implementation of FileSystem
 */

#include "filesystem.hpp"
#include "os_file.hpp"
#include "memory_file.hpp"
#include <algorithm>
#include <cctype>
#include <regex>

namespace ezquake::fs {

FileSystem::FileSystem() = default;

FileSystem::~FileSystem() {
    shutdown();
}

void FileSystem::init(const std::filesystem::path& baseDir,
                       const std::filesystem::path& homeDir,
                       std::string_view gameDir) {
    std::unique_lock lock(mutex_);
    
    baseDir_ = baseDir;
    homeDir_ = homeDir;
    gameDirName_ = std::string(gameDir);
    
    // Build full game directory path
    gameDir_ = homeDir_ / gameDirName_;
    
    // Create home directory if it doesn't exist
    std::error_code ec;
    std::filesystem::create_directories(gameDir_, ec);
    
    // Add default search paths
    // Home directory has highest priority (user overrides)
    searchPaths_.push_back({gameDir_, SearchPathType::Directory, false, false, 100});
    
    // Base game directory
    auto basePath = baseDir_ / gameDirName_;
    if (std::filesystem::exists(basePath, ec)) {
        searchPaths_.push_back({basePath, SearchPathType::Directory, false, false, 50});
    }
    
    // ID1 directory (base Quake content)
    auto id1Path = baseDir_ / "id1";
    if (std::filesystem::exists(id1Path, ec)) {
        searchPaths_.push_back({id1Path, SearchPathType::Directory, false, false, 10});
    }
    
    initialized_ = true;
}

void FileSystem::shutdown() {
    std::unique_lock lock(mutex_);
    
    searchPaths_.clear();
    locationCache_.clear();
    initialized_ = false;
}

FileResult<VirtualFilePtr> FileSystem::openRead(std::string_view filename,
                                                  RelativeTo relativeTo) {
    if (isUnsafeFilename(filename)) {
        return unexpected(FileError::InvalidPath);
    }
    
    // Handle absolute paths
    if (relativeTo == RelativeTo::None) {
        return OsFile::open(std::filesystem::path(filename), OpenMode::Read);
    }
    
    // Locate the file
    auto location = locate(filename, relativeTo);
    if (!location) {
        return unexpected(FileError::NotFound);
    }
    
    // TODO: Handle PAK files - for now just open as regular file
    if (location->isInArchive()) {
        // Would need to extract from PAK/PK3 here
        return unexpected(FileError::UnsupportedOperation);
    }
    
    return OsFile::open(location->fullPath, OpenMode::Read);
}

FileResult<VirtualFilePtr> FileSystem::openWrite(std::string_view filename,
                                                   bool append) {
    if (isUnsafeFilename(filename)) {
        return unexpected(FileError::InvalidPath);
    }
    
    std::shared_lock lock(mutex_);
    
    auto fullPath = gameDir_ / filename;
    
    // Create parent directories
    auto parentPath = fullPath.parent_path();
    if (!parentPath.empty()) {
        std::error_code ec;
        std::filesystem::create_directories(parentPath, ec);
        if (ec) {
            return unexpected(FileError::AccessDenied);
        }
    }
    
    return OsFile::open(fullPath, append ? OpenMode::Append : OpenMode::Write);
}

FileResult<std::vector<Byte>> FileSystem::loadFile(std::string_view filename,
                                                     RelativeTo relativeTo) {
    auto fileResult = openRead(filename, relativeTo);
    if (!fileResult) {
        return unexpected(fileResult.error());
    }
    
    return (*fileResult)->readAll();
}

FileResult<std::string> FileSystem::loadText(std::string_view filename,
                                               RelativeTo relativeTo) {
    auto fileResult = openRead(filename, relativeTo);
    if (!fileResult) {
        return unexpected(fileResult.error());
    }
    
    return (*fileResult)->readAllText();
}

FileResult<size_t> FileSystem::saveFile(std::string_view filename,
                                          std::span<const Byte> data) {
    auto fileResult = openWrite(filename);
    if (!fileResult) {
        return unexpected(fileResult.error());
    }
    
    return (*fileResult)->write(data);
}

FileResult<size_t> FileSystem::saveText(std::string_view filename,
                                          std::string_view text) {
    auto fileResult = openWrite(filename);
    if (!fileResult) {
        return unexpected(fileResult.error());
    }
    
    return (*fileResult)->writeText(text);
}

std::optional<FileLocation> FileSystem::locate(std::string_view filename,
                                                 RelativeTo relativeTo) {
    if (isUnsafeFilename(filename)) {
        return std::nullopt;
    }
    
    // Check cache first
    {
        std::shared_lock lock(mutex_);
        if (cacheEnabled_) {
            auto it = locationCache_.find(std::string(filename));
            if (it != locationCache_.end()) {
                return it->second;
            }
        }
    }
    
    SearchOptions options;
    options.includePaks = (relativeTo == RelativeTo::Any || relativeTo == RelativeTo::Pak);
    options.homeOnly = (relativeTo == RelativeTo::Home);
    options.baseOnly = (relativeTo == RelativeTo::Base);
    
    auto result = searchFile(filename, options);
    
    // Cache the result
    if (result && cacheEnabled_) {
        std::unique_lock lock(mutex_);
        locationCache_[std::string(filename)] = *result;
    }
    
    return result;
}

std::optional<FileLocation> FileSystem::searchFile(std::string_view filename,
                                                    const SearchOptions& options) {
    std::shared_lock lock(mutex_);
    
    // Sort search paths by priority
    auto paths = searchPaths_;
    std::sort(paths.begin(), paths.end());
    
    std::error_code ec;
    
    for (const auto& searchPath : paths) {
        // Skip PAKs if not requested
        if (!options.includePaks && searchPath.type != SearchPathType::Directory) {
            continue;
        }
        
        // TODO: Handle PAK/PK3 files
        if (searchPath.type != SearchPathType::Directory) {
            continue;
        }
        
        auto fullPath = searchPath.path / filename;
        
        if (std::filesystem::exists(fullPath, ec) && 
            std::filesystem::is_regular_file(fullPath, ec)) {
            FileLocation loc;
            loc.fullPath = fullPath;
            loc.fileSize = std::filesystem::file_size(fullPath, ec);
            if (searchPath.copyProtected) {
                loc.flags = loc.flags | FileFlags::CopyProtected;
            }
            return loc;
        }
    }
    
    return std::nullopt;
}

bool FileSystem::exists(std::string_view filename, RelativeTo relativeTo) {
    return locate(filename, relativeTo).has_value();
}

std::filesystem::path FileSystem::fullPath(std::string_view filename,
                                            RelativeTo relativeTo) {
    auto loc = locate(filename, relativeTo);
    if (loc) {
        return loc->fullPath;
    }
    return {};
}

void FileSystem::addSearchPath(const std::filesystem::path& path,
                                 SearchPathType type,
                                 int priority) {
    std::unique_lock lock(mutex_);
    
    // Check if already exists
    for (const auto& sp : searchPaths_) {
        if (sp.path == path) {
            return;  // Already added
        }
    }
    
    SearchPath sp;
    sp.path = path;
    sp.type = type;
    sp.priority = priority;
    sp.copyProtected = (type != SearchPathType::Directory);
    
    searchPaths_.push_back(sp);
    
    // Invalidate cache
    locationCache_.clear();
}

void FileSystem::removeSearchPath(const std::filesystem::path& path) {
    std::unique_lock lock(mutex_);
    
    auto it = std::remove_if(searchPaths_.begin(), searchPaths_.end(),
        [&path](const SearchPath& sp) { return sp.path == path; });
    
    if (it != searchPaths_.end()) {
        searchPaths_.erase(it, searchPaths_.end());
        locationCache_.clear();
    }
}

void FileSystem::clearSearchPaths() {
    std::unique_lock lock(mutex_);
    searchPaths_.clear();
    locationCache_.clear();
}

std::vector<SearchPath> FileSystem::searchPaths() const {
    std::shared_lock lock(mutex_);
    return searchPaths_;
}

void FileSystem::setCacheEnabled(bool enabled) {
    std::unique_lock lock(mutex_);
    cacheEnabled_ = enabled;
    if (!enabled) {
        locationCache_.clear();
    }
}

bool FileSystem::isCacheEnabled() const {
    std::shared_lock lock(mutex_);
    return cacheEnabled_;
}

void FileSystem::flushCache() {
    std::unique_lock lock(mutex_);
    locationCache_.clear();
}

void FileSystem::notifyChanged() {
    flushCache();
}

void FileSystem::enumerate(std::string_view pattern,
                            std::function<void(const std::filesystem::path&)> callback,
                            RelativeTo relativeTo) {
    std::shared_lock lock(mutex_);
    
    // Convert glob pattern to regex
    std::string regexPattern;
    for (char c : pattern) {
        switch (c) {
            case '*': regexPattern += ".*"; break;
            case '?': regexPattern += "."; break;
            case '.': regexPattern += "\\."; break;
            default: regexPattern += c; break;
        }
    }
    
    std::regex re(regexPattern, std::regex::icase);
    std::error_code ec;
    
    for (const auto& searchPath : searchPaths_) {
        if (searchPath.type != SearchPathType::Directory) {
            continue;  // TODO: Handle PAK enumeration
        }
        
        if (!std::filesystem::exists(searchPath.path, ec)) {
            continue;
        }
        
        for (const auto& entry : std::filesystem::directory_iterator(searchPath.path, ec)) {
            if (!entry.is_regular_file()) {
                continue;
            }
            
            auto filename = entry.path().filename().string();
            if (std::regex_match(filename, re)) {
                callback(entry.path());
            }
        }
    }
}

bool FileSystem::isUnsafeFilename(std::string_view filename) {
    if (filename.empty()) {
        return true;
    }
    
    // Check for absolute paths
    if (filename[0] == '/' || filename[0] == '\\') {
        return true;
    }
    
    // Windows drive letters
    if (filename.size() >= 2 && filename[1] == ':') {
        return true;
    }
    
    // Check for path traversal
    if (filename.find("..") != std::string_view::npos) {
        return true;
    }
    
    // Check for shell metacharacters
    for (char c : filename) {
        if (c == '|' || c == '&' || c == ';' || c == '$' || 
            c == '`' || c == '\'' || c == '"' || c == '<' || c == '>') {
            return true;
        }
    }
    
    return false;
}

std::string FileSystem::sanitizeFilename(std::string_view filename) {
    if (isUnsafeFilename(filename)) {
        return "";
    }
    
    std::string result;
    result.reserve(filename.size());
    
    for (char c : filename) {
        // Convert backslashes to forward slashes
        if (c == '\\') {
            result += '/';
        }
        // Remove control characters
        else if (c >= 32 && c != 127) {
            result += c;
        }
    }
    
    return result;
}

std::string FileSystem::getExtension(std::string_view filename) {
    auto dotPos = filename.rfind('.');
    if (dotPos == std::string_view::npos || dotPos == filename.size() - 1) {
        return "";
    }
    
    std::string ext(filename.substr(dotPos + 1));
    
    // Convert to lowercase
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    
    return ext;
}

FileResult<void> FileSystem::createPath(const std::filesystem::path& path) {
    std::error_code ec;
    
    auto parentPath = path.parent_path();
    if (!parentPath.empty() && !std::filesystem::exists(parentPath, ec)) {
        std::filesystem::create_directories(parentPath, ec);
        if (ec) {
            return unexpected(FileError::AccessDenied);
        }
    }
    
    return {};
}

std::filesystem::path FileSystem::buildPath(std::string_view filename,
                                             RelativeTo relativeTo) {
    switch (relativeTo) {
        case RelativeTo::None:
            return std::filesystem::path(filename);
        case RelativeTo::GameOs:
        case RelativeTo::Base:
            return baseDir_ / gameDirName_ / filename;
        case RelativeTo::Home:
            return homeDir_ / gameDirName_ / filename;
        case RelativeTo::Any:
        case RelativeTo::Pak:
        default:
            return gameDir_ / filename;
    }
}

} // namespace ezquake::fs
