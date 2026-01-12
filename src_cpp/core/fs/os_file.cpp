/**
 * @file os_file.cpp
 * @brief Implementation of OsFile
 */

#include "os_file.hpp"
#include <algorithm>

namespace ezquake::fs {

OsFile::OsFile(std::fstream&& stream, std::filesystem::path path, OpenMode mode)
    : stream_(std::move(stream))
    , path_(std::move(path))
    , mode_(mode)
{
}

OsFile::~OsFile() {
    if (stream_.is_open()) {
        stream_.close();
    }
}

FileResult<VirtualFilePtr> OsFile::open(const std::filesystem::path& path, OpenMode mode) {
    // Check if file exists for read modes
    if (mode == OpenMode::Read || mode == OpenMode::ReadWrite) {
        std::error_code ec;
        if (!std::filesystem::exists(path, ec)) {
            return unexpected(FileError::NotFound);
        }
        if (std::filesystem::is_directory(path, ec)) {
            return unexpected(FileError::IsDirectory);
        }
    }
    
    auto streamMode = toStreamMode(mode);
    std::fstream stream(path, streamMode);
    
    if (!stream.is_open()) {
        // Try to determine specific error
        std::error_code ec;
        if (!std::filesystem::exists(path, ec)) {
            return unexpected(FileError::NotFound);
        }
        return unexpected(FileError::AccessDenied);
    }
    
    return VirtualFilePtr(new OsFile(std::move(stream), path, mode));
}

FileResult<VirtualFilePtr> OsFile::create(const std::filesystem::path& path) {
    // Check if file already exists
    std::error_code ec;
    if (std::filesystem::exists(path, ec)) {
        return unexpected(FileError::AlreadyExists);
    }
    
    // Create parent directories if needed
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path(), ec);
        if (ec) {
            return unexpected(FileError::AccessDenied);
        }
    }
    
    std::fstream stream(path, std::ios::out | std::ios::binary);
    if (!stream.is_open()) {
        return unexpected(FileError::AccessDenied);
    }
    
    return VirtualFilePtr(new OsFile(std::move(stream), path, OpenMode::Create));
}

std::ios_base::openmode OsFile::toStreamMode(OpenMode mode) {
    switch (mode) {
        case OpenMode::Read:
            return std::ios::in | std::ios::binary;
        case OpenMode::Write:
            return std::ios::out | std::ios::binary | std::ios::trunc;
        case OpenMode::Append:
            return std::ios::out | std::ios::binary | std::ios::app;
        case OpenMode::ReadWrite:
            return std::ios::in | std::ios::out | std::ios::binary;
        case OpenMode::Create:
            return std::ios::out | std::ios::binary;
    }
    return std::ios::in | std::ios::binary;
}

FileResult<size_t> OsFile::read(std::span<Byte> buffer) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!stream_.is_open()) {
        return unexpected(FileError::NotOpen);
    }
    
    if (mode_ == OpenMode::Write || mode_ == OpenMode::Append) {
        return unexpected(FileError::UnsupportedOperation);
    }
    
    stream_.read(reinterpret_cast<char*>(buffer.data()), 
                 static_cast<std::streamsize>(buffer.size()));
    
    auto bytesRead = static_cast<size_t>(stream_.gcount());
    
    if (stream_.eof()) {
        atEof_ = true;
        stream_.clear();  // Clear EOF flag for future operations
        if (bytesRead == 0) {
            return unexpected(FileError::EndOfFile);
        }
    } else if (stream_.fail()) {
        stream_.clear();
        return unexpected(FileError::ReadError);
    }
    
    return bytesRead;
}

FileResult<size_t> OsFile::write(std::span<const Byte> data) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!stream_.is_open()) {
        return unexpected(FileError::NotOpen);
    }
    
    if (mode_ == OpenMode::Read) {
        return unexpected(FileError::UnsupportedOperation);
    }
    
    auto posBefore = stream_.tellp();
    stream_.write(reinterpret_cast<const char*>(data.data()),
                  static_cast<std::streamsize>(data.size()));
    
    if (stream_.fail()) {
        stream_.clear();
        return unexpected(FileError::WriteError);
    }
    
    sizeValid_ = false;  // Size may have changed
    
    auto posAfter = stream_.tellp();
    return static_cast<size_t>(posAfter - posBefore);
}

FileResult<size_t> OsFile::seek(int64_t offset, SeekOrigin origin) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!stream_.is_open()) {
        return unexpected(FileError::NotOpen);
    }
    
    std::ios_base::seekdir dir;
    switch (origin) {
        case SeekOrigin::Begin:   dir = std::ios::beg; break;
        case SeekOrigin::Current: dir = std::ios::cur; break;
        case SeekOrigin::End:     dir = std::ios::end; break;
    }
    
    stream_.seekg(offset, dir);
    stream_.seekp(offset, dir);
    
    if (stream_.fail()) {
        stream_.clear();
        return unexpected(FileError::SeekError);
    }
    
    atEof_ = false;
    return static_cast<size_t>(stream_.tellg());
}

FileResult<size_t> OsFile::tell() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!stream_.is_open()) {
        return unexpected(FileError::NotOpen);
    }
    
    auto pos = const_cast<std::fstream&>(stream_).tellg();
    if (pos < 0) {
        return unexpected(FileError::SeekError);
    }
    
    return static_cast<size_t>(pos);
}

FileResult<size_t> OsFile::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (sizeValid_) {
        return cachedSize_;
    }
    
    // Use filesystem::file_size for efficiency
    std::error_code ec;
    auto fileSize = std::filesystem::file_size(path_, ec);
    if (ec) {
        // Fallback: seek to end and back
        auto& mutableStream = const_cast<std::fstream&>(stream_);
        auto currentPos = mutableStream.tellg();
        mutableStream.seekg(0, std::ios::end);
        auto endPos = mutableStream.tellg();
        mutableStream.seekg(currentPos);
        
        if (endPos < 0) {
            return unexpected(FileError::SeekError);
        }
        
        cachedSize_ = static_cast<size_t>(endPos);
    } else {
        cachedSize_ = fileSize;
    }
    
    sizeValid_ = true;
    return cachedSize_;
}

FileResult<void> OsFile::flush() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!stream_.is_open()) {
        return unexpected(FileError::NotOpen);
    }
    
    stream_.flush();
    if (stream_.fail()) {
        stream_.clear();
        return unexpected(FileError::WriteError);
    }
    
    return {};
}

bool OsFile::eof() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return atEof_ || stream_.eof();
}

bool OsFile::isOpen() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stream_.is_open();
}

FileFlags OsFile::flags() const {
    FileFlags f = FileFlags::None;
    
    if (mode_ == OpenMode::Read) {
        f = f | FileFlags::ReadOnly;
    }
    
    return f;
}

std::string_view OsFile::path() const {
    // Note: path_ is const after construction, no lock needed
    // We need to store the string to return a view
    static thread_local std::string pathStr;
    pathStr = path_.string();
    return pathStr;
}

} // namespace ezquake::fs
