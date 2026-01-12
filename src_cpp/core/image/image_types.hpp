/**
 * @file image_types.hpp
 * @brief Image, screenshot, and skin type definitions
 * 
 * This module provides types for image handling, screenshot capture,
 * and player skin management.
 * 
 * Replaces:
 * - client.h: image_format_t, scr_sshot_target_t
 * - client.h: skin_t, skin_texture_t
 * - skin.c: player_skin_t
 */

#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <string_view>
#include <array>
#include <vector>
#include <optional>
#include <memory>
#include <span>

namespace ezquake::image {

// ============================================================================
// Constants
// ============================================================================

namespace image_limits {
    constexpr int32_t MAX_FILENAME_LENGTH = 128;
    constexpr int32_t MAX_CACHED_SKINS = 256;
    constexpr int32_t MAX_SKIN_NAME = 16;
    constexpr int32_t SKIN_WIDTH = 320;
    constexpr int32_t SKIN_HEIGHT = 200;
    constexpr int32_t MAX_CLIENTS = 32;  // For player skins
}

// ============================================================================
// Image Format
// ============================================================================

/**
 * @brief Supported image formats
 * 
 * Replaces C: IMAGE_PCX, IMAGE_TGA, IMAGE_JPEG, IMAGE_PNG
 */
enum class ImageFormat : uint8_t {
    PCX = 0,    ///< Zsoft PCX format (legacy)
    TGA = 1,    ///< Targa format
    JPEG = 2,   ///< JPEG format
    PNG = 3     ///< PNG format (preferred)
};

/**
 * @brief Get file extension for format
 */
[[nodiscard]] constexpr std::string_view imageFormatExtension(ImageFormat format) noexcept {
    switch (format) {
        case ImageFormat::PCX:  return ".pcx";
        case ImageFormat::TGA:  return ".tga";
        case ImageFormat::JPEG: return ".jpg";
        case ImageFormat::PNG:  return ".png";
        default: return ".png";
    }
}

/**
 * @brief Get MIME type for format
 */
[[nodiscard]] constexpr std::string_view imageFormatMimeType(ImageFormat format) noexcept {
    switch (format) {
        case ImageFormat::PCX:  return "image/x-pcx";
        case ImageFormat::TGA:  return "image/x-tga";
        case ImageFormat::JPEG: return "image/jpeg";
        case ImageFormat::PNG:  return "image/png";
        default: return "application/octet-stream";
    }
}

/**
 * @brief Get format name
 */
[[nodiscard]] constexpr std::string_view imageFormatName(ImageFormat format) noexcept {
    switch (format) {
        case ImageFormat::PCX:  return "PCX";
        case ImageFormat::TGA:  return "TGA";
        case ImageFormat::JPEG: return "JPEG";
        case ImageFormat::PNG:  return "PNG";
        default: return "Unknown";
    }
}

/**
 * @brief Check if format supports alpha channel
 */
[[nodiscard]] constexpr bool imageFormatSupportsAlpha(ImageFormat format) noexcept {
    switch (format) {
        case ImageFormat::TGA:
        case ImageFormat::PNG:
            return true;
        case ImageFormat::PCX:
        case ImageFormat::JPEG:
        default:
            return false;
    }
}

/**
 * @brief Check if format is lossy
 */
[[nodiscard]] constexpr bool imageFormatIsLossy(ImageFormat format) noexcept {
    return format == ImageFormat::JPEG;
}

// ============================================================================
// Screenshot Target
// ============================================================================

/**
 * @brief Screenshot capture target
 * 
 * Replaces C: scr_sshot_target_t
 * Contains information about where to write a screenshot and its properties.
 */
struct ScreenshotTarget {
    std::string filename;           ///< Output filename
    std::span<uint8_t> buffer;      ///< Pixel buffer (external or internal)
    std::vector<uint8_t> ownedBuffer; ///< Owned buffer if we allocated it
    bool freeMemory = false;        ///< Whether we own the buffer
    bool movieCapture = false;      ///< Part of movie capture sequence
    size_t width = 0;               ///< Image width in pixels
    size_t height = 0;              ///< Image height in pixels
    ImageFormat format = ImageFormat::PNG;
    
    ScreenshotTarget() = default;
    
    ScreenshotTarget(std::string_view file, size_t w, size_t h, ImageFormat fmt = ImageFormat::PNG)
        : filename(file), width(w), height(h), format(fmt) {}
    
    /**
     * @brief Allocate internal buffer
     */
    bool allocateBuffer() {
        size_t size = width * height * 4; // RGBA
        if (size == 0) return false;
        
        ownedBuffer.resize(size);
        buffer = ownedBuffer;
        freeMemory = true;
        return true;
    }
    
    /**
     * @brief Use external buffer
     */
    void useBuffer(std::span<uint8_t> externalBuffer) noexcept {
        buffer = externalBuffer;
        ownedBuffer.clear();
        freeMemory = false;
    }
    
    [[nodiscard]] bool isValid() const noexcept {
        return width > 0 && height > 0 && !filename.empty();
    }
    
    [[nodiscard]] bool hasBuffer() const noexcept {
        return !buffer.empty();
    }
    
    [[nodiscard]] size_t pixelCount() const noexcept {
        return width * height;
    }
    
    [[nodiscard]] size_t byteSize() const noexcept {
        return width * height * 4;
    }
};

// ============================================================================
// Screenshot Queue
// ============================================================================

/**
 * @brief Screenshot capture state
 */
enum class ScreenshotState : uint8_t {
    Idle,           ///< No capture in progress
    Pending,        ///< Capture requested but not started
    Capturing,      ///< Currently capturing
    Writing,        ///< Writing to file
    Complete,       ///< Capture complete
    Failed          ///< Capture failed
};

/**
 * @brief Screenshot request in queue
 */
struct ScreenshotRequest {
    ScreenshotTarget target;
    ScreenshotState state = ScreenshotState::Idle;
    int frameDelay = 0;             ///< Frames to wait before capture
    double requestTime = 0.0;       ///< Time request was made
    std::string errorMessage;       ///< Error if failed
    
    [[nodiscard]] bool isPending() const noexcept {
        return state == ScreenshotState::Pending;
    }
    
    [[nodiscard]] bool isComplete() const noexcept {
        return state == ScreenshotState::Complete;
    }
    
    [[nodiscard]] bool hasFailed() const noexcept {
        return state == ScreenshotState::Failed;
    }
};

/**
 * @brief Screenshot queue manager
 */
struct ScreenshotQueue {
    std::vector<ScreenshotRequest> requests;
    size_t maxQueueSize = 10;
    size_t completedCount = 0;
    size_t failedCount = 0;
    
    [[nodiscard]] bool isEmpty() const noexcept {
        return requests.empty();
    }
    
    [[nodiscard]] size_t size() const noexcept {
        return requests.size();
    }
    
    [[nodiscard]] bool isFull() const noexcept {
        return requests.size() >= maxQueueSize;
    }
    
    bool enqueue(ScreenshotRequest request) {
        if (isFull()) return false;
        request.state = ScreenshotState::Pending;
        requests.push_back(std::move(request));
        return true;
    }
    
    [[nodiscard]] ScreenshotRequest* nextPending() noexcept {
        for (auto& req : requests) {
            if (req.isPending()) return &req;
        }
        return nullptr;
    }
    
    void removeCompleted() {
        requests.erase(
            std::remove_if(requests.begin(), requests.end(),
                [this](const ScreenshotRequest& r) {
                    if (r.isComplete()) { ++completedCount; return true; }
                    if (r.hasFailed()) { ++failedCount; return true; }
                    return false;
                }),
            requests.end()
        );
    }
    
    void clear() {
        requests.clear();
    }
};

// ============================================================================
// Skin Texture Types
// ============================================================================

/**
 * @brief Skin texture slot types
 * 
 * Replaces C: skin_texture_t enum
 */
enum class SkinTextureSlot : uint8_t {
    Base = 0,           ///< Standard skin
    BaseTeammate = 1,   ///< Teammate variation
    Fullbright = 2,     ///< Fullbright texture
    Dead = 3,           ///< Dead/corpse skin
    DeadTeammate = 4,   ///< Dead teammate variation
    Count = 5           ///< Number of texture slots
};

constexpr size_t SKIN_TEXTURE_COUNT = static_cast<size_t>(SkinTextureSlot::Count);

/**
 * @brief Get texture slot name
 */
[[nodiscard]] constexpr std::string_view skinTextureSlotName(SkinTextureSlot slot) noexcept {
    switch (slot) {
        case SkinTextureSlot::Base:         return "$skin-base";
        case SkinTextureSlot::BaseTeammate: return "$skin-base-tm";
        case SkinTextureSlot::Fullbright:   return "$skin-fb";
        case SkinTextureSlot::Dead:         return "$skin-dead";
        case SkinTextureSlot::DeadTeammate: return "$skin-dead-tm";
        default: return "$skin-unknown";
    }
}

/**
 * @brief Check if slot is for teammates
 */
[[nodiscard]] constexpr bool skinSlotIsTeammate(SkinTextureSlot slot) noexcept {
    return slot == SkinTextureSlot::BaseTeammate || 
           slot == SkinTextureSlot::DeadTeammate;
}

// ============================================================================
// Skin Types
// ============================================================================

/**
 * @brief Opaque texture reference for skin textures
 */
struct SkinTextureRef {
    uint32_t id = 0;
    
    [[nodiscard]] bool isValid() const noexcept { return id != 0; }
    [[nodiscard]] explicit operator bool() const noexcept { return isValid(); }
    
    bool operator==(const SkinTextureRef& other) const noexcept = default;
};

/**
 * @brief Player skin data
 * 
 * Replaces C: skin_t
 */
struct Skin {
    std::array<char, image_limits::MAX_SKIN_NAME> name{};
    bool failedLoad = false;        ///< Skin failed to load
    bool warned = false;            ///< Warning already printed
    int32_t width = 0;              ///< Skin width in pixels
    int32_t height = 0;             ///< Skin height in pixels
    int32_t bpp = 1;                ///< Bytes per pixel (1=PCX, 4=32bit)
    std::array<SkinTextureRef, SKIN_TEXTURE_COUNT> textures{};
    std::vector<uint8_t> cachedData; ///< Raw skin pixel data
    
    Skin() = default;
    
    explicit Skin(std::string_view skinName) {
        setName(skinName);
    }
    
    void setName(std::string_view skinName) {
        size_t len = std::min(skinName.size(), static_cast<size_t>(image_limits::MAX_SKIN_NAME - 1));
        std::copy_n(skinName.begin(), len, name.begin());
        name[len] = '\0';
    }
    
    [[nodiscard]] std::string_view getName() const noexcept {
        return name.data();
    }
    
    [[nodiscard]] bool isLoaded() const noexcept {
        return !failedLoad && (hasCachedData() || hasTexture(SkinTextureSlot::Base));
    }
    
    [[nodiscard]] bool is32Bit() const noexcept {
        return bpp == 4;
    }
    
    [[nodiscard]] bool hasCachedData() const noexcept {
        return !cachedData.empty();
    }
    
    [[nodiscard]] bool hasTexture(SkinTextureSlot slot) const noexcept {
        auto index = static_cast<size_t>(slot);
        return index < SKIN_TEXTURE_COUNT && textures[index].isValid();
    }
    
    [[nodiscard]] SkinTextureRef getTexture(SkinTextureSlot slot) const noexcept {
        auto index = static_cast<size_t>(slot);
        return index < SKIN_TEXTURE_COUNT ? textures[index] : SkinTextureRef{};
    }
    
    void setTexture(SkinTextureSlot slot, SkinTextureRef ref) noexcept {
        auto index = static_cast<size_t>(slot);
        if (index < SKIN_TEXTURE_COUNT) {
            textures[index] = ref;
        }
    }
    
    void clearTextures() noexcept {
        textures.fill(SkinTextureRef{});
    }
    
    void reset() {
        name.fill('\0');
        failedLoad = false;
        warned = false;
        width = 0;
        height = 0;
        bpp = 1;
        clearTextures();
        cachedData.clear();
    }
};

/**
 * @brief Player skin assignment
 * 
 * Replaces C: player_skin_t
 * Maps a player to their current skin textures
 */
struct PlayerSkin {
    SkinTextureRef base;        ///< Standard skin texture
    SkinTextureRef fullbright;  ///< Fullbright layer
    SkinTextureRef dead;        ///< Dead/corpse texture
    std::array<bool, 3> owned{}; ///< Whether textures are owned
    
    [[nodiscard]] bool hasBase() const noexcept { return base.isValid(); }
    [[nodiscard]] bool hasFullbright() const noexcept { return fullbright.isValid(); }
    [[nodiscard]] bool hasDead() const noexcept { return dead.isValid(); }
    
    void clear() noexcept {
        base = SkinTextureRef{};
        fullbright = SkinTextureRef{};
        dead = SkinTextureRef{};
        owned.fill(false);
    }
};

// ============================================================================
// Skin Cache
// ============================================================================

/**
 * @brief Skin cache manager
 */
struct SkinCache {
    std::array<Skin, image_limits::MAX_CACHED_SKINS> skins{};
    size_t count = 0;
    bool needsPreache = true;
    
    [[nodiscard]] Skin* find(std::string_view name) noexcept {
        for (size_t i = 0; i < count; ++i) {
            if (skins[i].getName() == name) {
                return &skins[i];
            }
        }
        return nullptr;
    }
    
    [[nodiscard]] const Skin* find(std::string_view name) const noexcept {
        for (size_t i = 0; i < count; ++i) {
            if (skins[i].getName() == name) {
                return &skins[i];
            }
        }
        return nullptr;
    }
    
    Skin* allocate(std::string_view name) {
        if (count >= skins.size()) return nullptr;
        
        auto& skin = skins[count++];
        skin.reset();
        skin.setName(name);
        return &skin;
    }
    
    Skin* findOrAllocate(std::string_view name) {
        if (auto* existing = find(name)) {
            return existing;
        }
        return allocate(name);
    }
    
    void clear() {
        for (size_t i = 0; i < count; ++i) {
            skins[i].reset();
        }
        count = 0;
    }
    
    [[nodiscard]] size_t size() const noexcept { return count; }
    [[nodiscard]] bool isEmpty() const noexcept { return count == 0; }
    [[nodiscard]] bool isFull() const noexcept { return count >= skins.size(); }
};

// ============================================================================
// Image Data
// ============================================================================

/**
 * @brief Raw image data
 */
struct ImageData {
    std::vector<uint8_t> pixels;
    int32_t width = 0;
    int32_t height = 0;
    int32_t channels = 4;  ///< 1=grayscale, 3=RGB, 4=RGBA
    ImageFormat sourceFormat = ImageFormat::PNG;
    
    ImageData() = default;
    
    ImageData(int32_t w, int32_t h, int32_t ch = 4)
        : width(w), height(h), channels(ch) {
        pixels.resize(static_cast<size_t>(w) * static_cast<size_t>(h) * static_cast<size_t>(ch));
    }
    
    [[nodiscard]] bool isValid() const noexcept {
        return width > 0 && height > 0 && !pixels.empty();
    }
    
    [[nodiscard]] size_t byteSize() const noexcept {
        return static_cast<size_t>(width) * static_cast<size_t>(height) * static_cast<size_t>(channels);
    }
    
    [[nodiscard]] size_t pixelCount() const noexcept {
        return static_cast<size_t>(width) * static_cast<size_t>(height);
    }
    
    [[nodiscard]] size_t rowStride() const noexcept {
        return static_cast<size_t>(width) * static_cast<size_t>(channels);
    }
    
    [[nodiscard]] uint8_t* row(int32_t y) noexcept {
        if (y < 0 || y >= height) return nullptr;
        return pixels.data() + static_cast<size_t>(y) * rowStride();
    }
    
    [[nodiscard]] const uint8_t* row(int32_t y) const noexcept {
        if (y < 0 || y >= height) return nullptr;
        return pixels.data() + static_cast<size_t>(y) * rowStride();
    }
    
    void clear() {
        pixels.clear();
        width = 0;
        height = 0;
    }
    
    /**
     * @brief Fill with solid color
     */
    void fill(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
        if (channels < 3) return;
        
        for (size_t i = 0; i < pixels.size(); i += static_cast<size_t>(channels)) {
            pixels[i] = r;
            pixels[i + 1] = g;
            pixels[i + 2] = b;
            if (channels >= 4) pixels[i + 3] = a;
        }
    }
    
    /**
     * @brief Flip image vertically (for OpenGL texture uploads)
     */
    void flipVertical() {
        if (!isValid()) return;
        
        size_t stride = rowStride();
        std::vector<uint8_t> tempRow(stride);
        
        for (int32_t y = 0; y < height / 2; ++y) {
            uint8_t* top = row(y);
            uint8_t* bottom = row(height - 1 - y);
            
            std::copy_n(top, stride, tempRow.begin());
            std::copy_n(bottom, stride, top);
            std::copy_n(tempRow.begin(), stride, bottom);
        }
    }
};

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * @brief Parse image format from extension
 */
[[nodiscard]] inline std::optional<ImageFormat> parseImageFormat(std::string_view extension) noexcept {
    if (extension.empty()) return std::nullopt;
    
    // Skip leading dot if present
    if (extension[0] == '.') {
        extension = extension.substr(1);
    }
    
    if (extension == "pcx") return ImageFormat::PCX;
    if (extension == "tga") return ImageFormat::TGA;
    if (extension == "jpg" || extension == "jpeg") return ImageFormat::JPEG;
    if (extension == "png") return ImageFormat::PNG;
    
    return std::nullopt;
}

/**
 * @brief Generate screenshot filename with auto-numbering
 */
[[nodiscard]] inline std::string generateScreenshotFilename(
    std::string_view baseName,
    ImageFormat format,
    int32_t number = -1
) {
    std::string result(baseName);
    
    if (number >= 0) {
        result += "_";
        if (number < 1000) result += "0";
        if (number < 100) result += "0";
        if (number < 10) result += "0";
        result += std::to_string(number);
    }
    
    result += imageFormatExtension(format);
    return result;
}

/**
 * @brief Create default player skins array
 */
[[nodiscard]] inline std::array<PlayerSkin, image_limits::MAX_CLIENTS> createDefaultPlayerSkins() {
    std::array<PlayerSkin, image_limits::MAX_CLIENTS> skins{};
    return skins;
}

/**
 * @brief Calculate bytes needed for image
 */
[[nodiscard]] constexpr size_t calculateImageBytes(
    size_t width,
    size_t height,
    int32_t channels = 4
) noexcept {
    return width * height * static_cast<size_t>(channels);
}

} // namespace ezquake::image
