/**
 * @file memory_file.cpp
 * @brief Implementation of MemoryFile
 */

#include "memory_file.hpp"
#include <algorithm>
#include <cstring>

namespace ezquake::fs {

MemoryFile::MemoryFile(std::vector<Byte>&& data, std::string name, bool readOnly)
    : ownedData_(std::move(data))
    , name_(std::move(name))
    , isView_(false)
    , readOnly_(readOnly)
{
}

MemoryFile::MemoryFile(std::span<const Byte> view, std::string name)
    : viewData_(view)
    , name_(std::move(name))
    , isView_(true)
    , readOnly_(true)
{
}

VirtualFilePtr MemoryFile::create(std::string name, size_t reserveSize) {
    std::vector<Byte> data;
    if (reserveSize > 0) {
        data.reserve(reserveSize);
    }
    return VirtualFilePtr(new MemoryFile(std::move(data), std::move(name), false));
}

VirtualFilePtr MemoryFile::fromData(std::span<const Byte> data, std::string name) {
    std::vector<Byte> copy(data.begin(), data.end());
    return VirtualFilePtr(new MemoryFile(std::move(copy), std::move(name), false));
}

VirtualFilePtr MemoryFile::fromVector(std::vector<Byte>&& data, std::string name) {
    return VirtualFilePtr(new MemoryFile(std::move(data), std::move(name), false));
}

VirtualFilePtr MemoryFile::fromString(std::string_view text, std::string name) {
    std::vector<Byte> data(text.size());
    std::memcpy(data.data(), text.data(), text.size());
    return VirtualFilePtr(new MemoryFile(std::move(data), std::move(name), false));
}

VirtualFilePtr MemoryFile::fromView(std::span<const Byte> data, std::string name) {
    return VirtualFilePtr(new MemoryFile(data, std::move(name)));
}

FileResult<size_t> MemoryFile::read(std::span<Byte> buffer) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (closed_) {
        return unexpected(FileError::NotOpen);
    }
    
    const auto& sourceData = isView_ ? viewData_ : std::span<const Byte>(ownedData_);
    
    if (position_ >= sourceData.size()) {
        return unexpected(FileError::EndOfFile);
    }
    
    size_t available = sourceData.size() - position_;
    size_t toRead = std::min(buffer.size(), available);
    
    std::memcpy(buffer.data(), sourceData.data() + position_, toRead);
    position_ += toRead;
    
    return toRead;
}

FileResult<size_t> MemoryFile::write(std::span<const Byte> data) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (closed_) {
        return unexpected(FileError::NotOpen);
    }
    
    if (readOnly_ || isView_) {
        return unexpected(FileError::UnsupportedOperation);
    }
    
    // Ensure capacity
    size_t endPos = position_ + data.size();
    if (endPos > ownedData_.size()) {
        try {
            ownedData_.resize(endPos);
        } catch (const std::bad_alloc&) {
            return unexpected(FileError::OutOfMemory);
        }
    }
    
    std::memcpy(ownedData_.data() + position_, data.data(), data.size());
    position_ += data.size();
    
    return data.size();
}

FileResult<size_t> MemoryFile::seek(int64_t offset, SeekOrigin origin) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (closed_) {
        return unexpected(FileError::NotOpen);
    }
    
    const auto dataSize = static_cast<int64_t>(isView_ ? viewData_.size() : ownedData_.size());
    int64_t newPos;
    
    switch (origin) {
        case SeekOrigin::Begin:
            newPos = offset;
            break;
        case SeekOrigin::Current:
            newPos = static_cast<int64_t>(position_) + offset;
            break;
        case SeekOrigin::End:
            newPos = dataSize + offset;
            break;
    }
    
    if (newPos < 0) {
        return unexpected(FileError::SeekError);
    }
    
    // Allow seeking past end for writable files (will extend on write)
    if (newPos > dataSize && (readOnly_ || isView_)) {
        return unexpected(FileError::SeekError);
    }
    
    position_ = static_cast<size_t>(newPos);
    return position_;
}

FileResult<size_t> MemoryFile::tell() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (closed_) {
        return unexpected(FileError::NotOpen);
    }
    
    return position_;
}

FileResult<size_t> MemoryFile::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (closed_) {
        return unexpected(FileError::NotOpen);
    }
    
    return isView_ ? viewData_.size() : ownedData_.size();
}

FileResult<void> MemoryFile::flush() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (closed_) {
        return unexpected(FileError::NotOpen);
    }
    
    // Memory files don't need flushing
    return {};
}

bool MemoryFile::eof() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (closed_) {
        return true;
    }
    
    size_t dataSize = isView_ ? viewData_.size() : ownedData_.size();
    return position_ >= dataSize;
}

bool MemoryFile::isOpen() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return !closed_;
}

FileFlags MemoryFile::flags() const {
    FileFlags f = FileFlags::None;
    
    if (readOnly_ || isView_) {
        f = f | FileFlags::ReadOnly;
    }
    
    return f;
}

std::string_view MemoryFile::path() const {
    // name_ is const after construction
    return name_;
}

std::span<const Byte> MemoryFile::data() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (isView_) {
        return viewData_;
    }
    return std::span<const Byte>(ownedData_);
}

std::vector<Byte> MemoryFile::detach() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (isView_) {
        return {};  // Can't detach a view
    }
    
    std::vector<Byte> result = std::move(ownedData_);
    ownedData_.clear();
    position_ = 0;
    
    return result;
}

} // namespace ezquake::fs
