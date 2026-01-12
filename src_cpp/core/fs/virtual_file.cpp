/**
 * @file virtual_file.cpp
 * @brief Implementation of VirtualFile convenience methods
 */

#include "virtual_file.hpp"
#include <algorithm>

namespace ezquake::fs {

FileResult<std::string> VirtualFile::readLine(size_t maxLength) {
    std::string line;
    line.reserve(256);
    
    Byte ch;
    while (line.size() < maxLength) {
        auto result = read(std::span<Byte>(&ch, 1));
        if (!result) {
            if (result.error() == FileError::EndOfFile && !line.empty()) {
                return line;  // Return what we have
            }
            return unexpected(result.error());
        }
        
        if (*result == 0) {
            // EOF
            if (line.empty()) {
                return unexpected(FileError::EndOfFile);
            }
            break;
        }
        
        if (ch == '\n') {
            break;
        }
        
        if (ch != '\r') {  // Skip CR in CRLF
            line.push_back(static_cast<char>(ch));
        }
    }
    
    return line;
}

FileResult<std::vector<Byte>> VirtualFile::readAll(size_t maxSize) {
    // Get file size
    auto sizeResult = size();
    if (!sizeResult) {
        return unexpected(sizeResult.error());
    }
    
    size_t fileSize = *sizeResult;
    if (maxSize > 0 && fileSize > maxSize) {
        fileSize = maxSize;
    }
    
    // Seek to beginning
    auto seekResult = seek(0, SeekOrigin::Begin);
    if (!seekResult) {
        return unexpected(seekResult.error());
    }
    
    // Allocate buffer
    std::vector<Byte> buffer;
    try {
        buffer.resize(fileSize);
    } catch (const std::bad_alloc&) {
        return unexpected(FileError::OutOfMemory);
    }
    
    // Read entire file
    size_t totalRead = 0;
    while (totalRead < fileSize) {
        auto readResult = read(std::span<Byte>(buffer.data() + totalRead, fileSize - totalRead));
        if (!readResult) {
            if (readResult.error() == FileError::EndOfFile) {
                buffer.resize(totalRead);
                return buffer;
            }
            return unexpected(readResult.error());
        }
        
        if (*readResult == 0) {
            break;
        }
        
        totalRead += *readResult;
    }
    
    buffer.resize(totalRead);
    return buffer;
}

FileResult<std::string> VirtualFile::readAllText(size_t maxSize) {
    auto result = readAll(maxSize);
    if (!result) {
        return unexpected(result.error());
    }
    
    return std::string(reinterpret_cast<const char*>(result->data()), result->size());
}

FileResult<size_t> VirtualFile::writeText(std::string_view text) {
    return write(std::span<const Byte>(
        reinterpret_cast<const Byte*>(text.data()), 
        text.size()
    ));
}

} // namespace ezquake::fs
