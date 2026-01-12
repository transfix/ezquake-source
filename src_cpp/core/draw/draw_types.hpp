/**
 * @file draw_types.hpp
 * @brief 2D drawing types for UI, menus, and HUD
 * 
 * This module covers:
 * - Picture/image types (mpic_t equivalent)
 * - Cached picture identifiers
 * - Color types and utilities
 * - Text alignment and rendering
 * - WAD picture definitions
 */

#pragma once

#include <cstdint>
#include <array>
#include <string>
#include <string_view>
#include <optional>
#include <algorithm>

namespace ezquake::draw {

// ============================================================================
// Constants
// ============================================================================

namespace draw_limits {
    constexpr int32_t MAX_CACHED_PICS = 256;
    constexpr int32_t MAX_WADPIC_NAME = 32;
    constexpr int32_t MAX_COLOR_INFO = 256;
    constexpr int32_t GLYPH_COUNT = 256;
}

// ============================================================================
// Forward Declarations
// ============================================================================

struct Picture;
struct ColorInfo;

// ============================================================================
// Color Type
// ============================================================================

/**
 * @brief Color represented as packed 32-bit RGBA
 * 
 * Matches color_t from draw.h (typedef int color_t).
 * Format: 0xAABBGGRR (little endian RGBA)
 */
class Color {
public:
    constexpr Color() noexcept : value_(static_cast<int32_t>(0xFFFFFFFF)) {}  // White, full alpha
    constexpr explicit Color(int32_t packed) noexcept : value_(packed) {}
    constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) noexcept
        : value_(static_cast<int32_t>(r) | 
                 (static_cast<int32_t>(g) << 8) |
                 (static_cast<int32_t>(b) << 16) |
                 (static_cast<int32_t>(a) << 24)) {}
    
    [[nodiscard]] constexpr int32_t packed() const noexcept { return value_; }
    
    [[nodiscard]] constexpr uint8_t r() const noexcept { 
        return static_cast<uint8_t>(value_ & 0xFF); 
    }
    [[nodiscard]] constexpr uint8_t g() const noexcept { 
        return static_cast<uint8_t>((value_ >> 8) & 0xFF); 
    }
    [[nodiscard]] constexpr uint8_t b() const noexcept { 
        return static_cast<uint8_t>((value_ >> 16) & 0xFF); 
    }
    [[nodiscard]] constexpr uint8_t a() const noexcept { 
        return static_cast<uint8_t>((value_ >> 24) & 0xFF); 
    }
    
    // Get as float array [0-1]
    [[nodiscard]] std::array<float, 4> toFloatRGBA() const noexcept {
        return {
            static_cast<float>(r()) / 255.0f,
            static_cast<float>(g()) / 255.0f,
            static_cast<float>(b()) / 255.0f,
            static_cast<float>(a()) / 255.0f
        };
    }
    
    // Get as byte array
    [[nodiscard]] std::array<uint8_t, 4> toByteRGBA() const noexcept {
        return { r(), g(), b(), a() };
    }
    
    // Premultiply alpha
    [[nodiscard]] Color premultiplyAlpha() const noexcept {
        float alpha = static_cast<float>(a()) / 255.0f;
        return Color(
            static_cast<uint8_t>(r() * alpha),
            static_cast<uint8_t>(g() * alpha),
            static_cast<uint8_t>(b() * alpha),
            a()
        );
    }
    
    // Premultiply with specific alpha override
    [[nodiscard]] Color premultiplyAlpha(float alphaOverride) const noexcept {
        float alpha = alphaOverride * (static_cast<float>(a()) / 255.0f);
        return Color(
            static_cast<uint8_t>(r() * alpha),
            static_cast<uint8_t>(g() * alpha),
            static_cast<uint8_t>(b() * alpha),
            static_cast<uint8_t>(alphaOverride * 255.0f)
        );
    }
    
    // Common colors
    static constexpr Color white() noexcept { return Color(255, 255, 255, 255); }
    static constexpr Color black() noexcept { return Color(0, 0, 0, 255); }
    static constexpr Color red() noexcept { return Color(255, 0, 0, 255); }
    static constexpr Color green() noexcept { return Color(0, 255, 0, 255); }
    static constexpr Color blue() noexcept { return Color(0, 0, 255, 255); }
    static constexpr Color yellow() noexcept { return Color(255, 255, 0, 255); }
    static constexpr Color transparent() noexcept { return Color(0, 0, 0, 0); }
    
    constexpr bool operator==(const Color& other) const noexcept {
        return value_ == other.value_;
    }
    constexpr bool operator!=(const Color& other) const noexcept {
        return value_ != other.value_;
    }
    
private:
    int32_t value_;
};

// ============================================================================
// Text Alignment
// ============================================================================

/**
 * @brief Text alignment for string rendering
 * 
 * Matches text_alignment_t from draw.h.
 */
enum class TextAlignment : uint8_t {
    Left = 0,
    Center = 1,
    Right = 2
};

[[nodiscard]] constexpr std::string_view textAlignmentName(TextAlignment align) noexcept {
    switch (align) {
        case TextAlignment::Left: return "left";
        case TextAlignment::Center: return "center";
        case TextAlignment::Right: return "right";
    }
    return "unknown";
}

// ============================================================================
// Color Info for Text Rendering
// ============================================================================

/**
 * @brief Color information for multi-colored text
 * 
 * Matches clrinfo_t from draw.h.
 * Used to specify color changes at specific character indices.
 */
struct ColorInfo {
    Color color;      // The color to apply
    int32_t index;    // Character index where this color starts
    
    constexpr ColorInfo() noexcept : color(Color::white()), index(0) {}
    constexpr ColorInfo(Color c, int32_t i) noexcept : color(c), index(i) {}
    
    [[nodiscard]] constexpr bool operator<(const ColorInfo& other) const noexcept {
        return index < other.index;
    }
};

// ============================================================================
// Picture/Image Types
// ============================================================================

/**
 * @brief Texture reference (opaque handle)
 * 
 * In the original code this references GPU texture objects.
 * Here we use an integer handle that can be mapped to actual textures.
 */
struct TextureRef {
    int32_t index = -1;
    
    constexpr TextureRef() noexcept = default;
    constexpr explicit TextureRef(int32_t idx) noexcept : index(idx) {}
    
    [[nodiscard]] constexpr bool isValid() const noexcept { return index >= 0; }
    [[nodiscard]] constexpr bool operator==(const TextureRef& other) const noexcept {
        return index == other.index;
    }
    [[nodiscard]] constexpr bool operator!=(const TextureRef& other) const noexcept {
        return index != other.index;
    }
};

/**
 * @brief 2D picture for menus, HUD, etc.
 * 
 * Matches mpic_t from draw.h.
 * Contains texture reference and UV coordinates for atlased textures.
 */
struct Picture {
    int32_t width = 0;
    int32_t height = 0;
    TextureRef texture;
    
    // Texture coordinates (for atlas textures)
    float sl = 0.0f;  // Left S coordinate
    float tl = 0.0f;  // Top T coordinate
    float sh = 1.0f;  // Right S coordinate (S high)
    float th = 1.0f;  // Bottom T coordinate (T high)
    
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return width > 0 && height > 0 && texture.isValid();
    }
    
    [[nodiscard]] constexpr float aspectRatio() const noexcept {
        return height > 0 ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;
    }
    
    // Get UV width/height
    [[nodiscard]] constexpr float uvWidth() const noexcept { return sh - sl; }
    [[nodiscard]] constexpr float uvHeight() const noexcept { return th - tl; }
    
    // Get sub-region UV coordinates
    [[nodiscard]] std::array<float, 4> getSubRegionUV(int srcX, int srcY, int srcW, int srcH) const noexcept {
        if (width <= 0 || height <= 0) {
            return { sl, tl, sh, th };
        }
        
        float uvW = uvWidth();
        float uvH = uvHeight();
        float fWidth = static_cast<float>(width);
        float fHeight = static_cast<float>(height);
        
        float newSl = sl + (static_cast<float>(srcX) / fWidth) * uvW;
        float newTl = tl + (static_cast<float>(srcY) / fHeight) * uvH;
        float newSh = sl + (static_cast<float>(srcX + srcW) / fWidth) * uvW;
        float newTh = tl + (static_cast<float>(srcY + srcH) / fHeight) * uvH;
        
        return { newSl, newTl, newSh, newTh };
    }
};

// ============================================================================
// Cached Picture IDs (Menu Graphics)
// ============================================================================

/**
 * @brief Cached picture identifiers for menu/UI graphics
 * 
 * Matches cache_pic_id_t from draw.h.
 */
enum class CachedPicId : uint8_t {
    Pause = 0,
    Loading,
    BoxTopLeft,
    BoxMiddleLeft,
    BoxBottomLeft,
    BoxTopMiddle,
    BoxMiddleMiddle,
    BoxMiddleMiddle2,
    BoxBottomMiddle,
    BoxTopRight,
    BoxMiddleRight,
    BoxBottomRight,
    TitleMain,
    MainMenu,
    MenuDot1,
    MenuDot2,
    MenuDot3,
    MenuDot4,
    MenuDot5,
    MenuDot6,
    TitleSinglePlayer,
    QPlaque,
    SinglePlayerMenu,
    LoadMenu,
    SaveMenu,
    MultiplayerMenu,
    Ranking,
    Complete,
    Inter,
    Finale,
    
    Count  // Total number of cached pics
};

[[nodiscard]] constexpr std::string_view cachedPicPath(CachedPicId id) noexcept {
    switch (id) {
        case CachedPicId::Pause: return "gfx/pause.lmp";
        case CachedPicId::Loading: return "gfx/loading.lmp";
        case CachedPicId::BoxTopLeft: return "gfx/box_tl.lmp";
        case CachedPicId::BoxMiddleLeft: return "gfx/box_ml.lmp";
        case CachedPicId::BoxBottomLeft: return "gfx/box_bl.lmp";
        case CachedPicId::BoxTopMiddle: return "gfx/box_tm.lmp";
        case CachedPicId::BoxMiddleMiddle: return "gfx/box_mm.lmp";
        case CachedPicId::BoxMiddleMiddle2: return "gfx/box_mm2.lmp";
        case CachedPicId::BoxBottomMiddle: return "gfx/box_bm.lmp";
        case CachedPicId::BoxTopRight: return "gfx/box_tr.lmp";
        case CachedPicId::BoxMiddleRight: return "gfx/box_mr.lmp";
        case CachedPicId::BoxBottomRight: return "gfx/box_br.lmp";
        case CachedPicId::TitleMain: return "gfx/ttl_main.lmp";
        case CachedPicId::MainMenu: return "gfx/mainmenu.lmp";
        case CachedPicId::MenuDot1: return "gfx/menudot1.lmp";
        case CachedPicId::MenuDot2: return "gfx/menudot2.lmp";
        case CachedPicId::MenuDot3: return "gfx/menudot3.lmp";
        case CachedPicId::MenuDot4: return "gfx/menudot4.lmp";
        case CachedPicId::MenuDot5: return "gfx/menudot5.lmp";
        case CachedPicId::MenuDot6: return "gfx/menudot6.lmp";
        case CachedPicId::TitleSinglePlayer: return "gfx/ttl_sgl.lmp";
        case CachedPicId::QPlaque: return "gfx/qplaque.lmp";
        case CachedPicId::SinglePlayerMenu: return "gfx/sp_menu.lmp";
        case CachedPicId::LoadMenu: return "gfx/p_load.lmp";
        case CachedPicId::SaveMenu: return "gfx/p_save.lmp";
        case CachedPicId::MultiplayerMenu: return "gfx/p_multi.lmp";
        case CachedPicId::Ranking: return "gfx/ranking.lmp";
        case CachedPicId::Complete: return "gfx/complete.lmp";
        case CachedPicId::Inter: return "gfx/inter.lmp";
        case CachedPicId::Finale: return "gfx/finale.lmp";
        default: return "";
    }
}

// ============================================================================
// WAD Picture Types
// ============================================================================

/**
 * @brief WAD picture identifiers for status bar and HUD graphics
 * 
 * Matches the WADPIC_* enum from draw.h.
 */
enum class WadPicId : uint8_t {
    // Status indicators
    Ram = 0,
    Net,
    Turtle,
    Disc,
    BackTile,
    
    // Numbers 0-9
    Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    
    // Alt numbers 0-9 (for negative/red)
    ANum0, ANum1, ANum2, ANum3, ANum4, ANum5, ANum6, ANum7, ANum8, ANum9,
    
    // Number symbols
    NumMinus,
    ANumMinus,
    NumColon,
    NumSlash,
    
    // Weapon inventory (not selected)
    InvShotgun,
    InvSuperShotgun,
    InvNailgun,
    InvSuperNailgun,
    InvRocketLauncher,
    InvGrenadeLauncher,
    InvLightning,
    
    // Weapon inventory (selected)
    Inv2Shotgun,
    Inv2SuperShotgun,
    Inv2Nailgun,
    Inv2SuperNailgun,
    Inv2RocketLauncher,
    Inv2GrenadeLauncher,
    Inv2Lightning,
    
    // Weapon flash animation frames (1-5 for each weapon)
    InvA1Shotgun, InvA2Shotgun, InvA3Shotgun, InvA4Shotgun, InvA5Shotgun,
    InvA1SuperShotgun, InvA2SuperShotgun, InvA3SuperShotgun, InvA4SuperShotgun, InvA5SuperShotgun,
    InvA1Nailgun, InvA2Nailgun, InvA3Nailgun, InvA4Nailgun, InvA5Nailgun,
    InvA1SuperNailgun, InvA2SuperNailgun, InvA3SuperNailgun, InvA4SuperNailgun, InvA5SuperNailgun,
    InvA1RocketLauncher, InvA2RocketLauncher, InvA3RocketLauncher, InvA4RocketLauncher, InvA5RocketLauncher,
    InvA1GrenadeLauncher, InvA2GrenadeLauncher, InvA3GrenadeLauncher, InvA4GrenadeLauncher, InvA5GrenadeLauncher,
    InvA1Lightning, InvA2Lightning, InvA3Lightning, InvA4Lightning, InvA5Lightning,
    
    // Ammo types
    SbShells,
    SbNails,
    SbRockets,
    SbCells,
    
    // Armor types
    SbArmor1,  // Green
    SbArmor2,  // Yellow
    SbArmor3,  // Red
    
    // Keys
    SbKey1,
    SbKey2,
    
    // Powerups
    SbInvis,
    SbInvuln,
    SbSuit,
    SbQuad,
    
    // Sigils (runes)
    SbSigil1,
    SbSigil2,
    SbSigil3,
    SbSigil4,
    
    // Health faces (progressive damage)
    SbFace1, SbFaceP1,
    SbFace2, SbFaceP2,
    SbFace3, SbFaceP3,
    SbFace4, SbFaceP4,
    SbFace5, SbFaceP5,
    
    // Special faces
    FaceInvis,
    FaceInvul2,
    FaceInv2,
    FaceQuad,
    
    // Status bars
    SbSbar,
    SbIbar,
    SbScorebar,
    
    // IBar ammo counts
    SbIbarAmmo1,
    SbIbarAmmo2,
    SbIbarAmmo3,
    SbIbarAmmo4,
    
    Count  // Total count
};

/**
 * @brief WAD picture definition
 * 
 * Matches wadpic_t from draw.h.
 */
struct WadPicture {
    std::string name;
    Picture* picture = nullptr;  // Pointer to loaded picture
    
    [[nodiscard]] bool isLoaded() const noexcept {
        return picture != nullptr && picture->isValid();
    }
};

// Get WAD picture name for ID
[[nodiscard]] constexpr std::string_view wadPicName(WadPicId id) noexcept {
    switch (id) {
        case WadPicId::Ram: return "ram";
        case WadPicId::Net: return "net";
        case WadPicId::Turtle: return "turtle";
        case WadPicId::Disc: return "disc";
        case WadPicId::BackTile: return "backtile";
        case WadPicId::Num0: return "num_0";
        case WadPicId::Num1: return "num_1";
        case WadPicId::Num2: return "num_2";
        case WadPicId::Num3: return "num_3";
        case WadPicId::Num4: return "num_4";
        case WadPicId::Num5: return "num_5";
        case WadPicId::Num6: return "num_6";
        case WadPicId::Num7: return "num_7";
        case WadPicId::Num8: return "num_8";
        case WadPicId::Num9: return "num_9";
        case WadPicId::ANum0: return "anum_0";
        case WadPicId::ANum1: return "anum_1";
        case WadPicId::ANum2: return "anum_2";
        case WadPicId::ANum3: return "anum_3";
        case WadPicId::ANum4: return "anum_4";
        case WadPicId::ANum5: return "anum_5";
        case WadPicId::ANum6: return "anum_6";
        case WadPicId::ANum7: return "anum_7";
        case WadPicId::ANum8: return "anum_8";
        case WadPicId::ANum9: return "anum_9";
        case WadPicId::NumMinus: return "num_minus";
        case WadPicId::ANumMinus: return "anum_minus";
        case WadPicId::NumColon: return "num_colon";
        case WadPicId::NumSlash: return "num_slash";
        case WadPicId::SbShells: return "sb_shells";
        case WadPicId::SbNails: return "sb_nails";
        case WadPicId::SbRockets: return "sb_rocket";
        case WadPicId::SbCells: return "sb_cells";
        case WadPicId::SbArmor1: return "sb_armor1";
        case WadPicId::SbArmor2: return "sb_armor2";
        case WadPicId::SbArmor3: return "sb_armor3";
        case WadPicId::SbKey1: return "sb_key1";
        case WadPicId::SbKey2: return "sb_key2";
        case WadPicId::SbInvis: return "sb_invis";
        case WadPicId::SbInvuln: return "sb_invuln";
        case WadPicId::SbSuit: return "sb_suit";
        case WadPicId::SbQuad: return "sb_quad";
        case WadPicId::SbSigil1: return "sb_sigil1";
        case WadPicId::SbSigil2: return "sb_sigil2";
        case WadPicId::SbSigil3: return "sb_sigil3";
        case WadPicId::SbSigil4: return "sb_sigil4";
        case WadPicId::SbSbar: return "sbar";
        case WadPicId::SbIbar: return "ibar";
        case WadPicId::SbScorebar: return "scorebar";
        default: return "";
    }
}

// ============================================================================
// Draw State
// ============================================================================

/**
 * @brief Scissor rectangle for clipping
 */
struct ScissorRect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    bool enabled = false;
    
    [[nodiscard]] constexpr bool contains(float px, float py) const noexcept {
        if (!enabled) return true;
        return px >= x && px < x + width && py >= y && py < y + height;
    }
    
    void reset() noexcept {
        x = y = width = height = 0.0f;
        enabled = false;
    }
};

/**
 * @brief Global drawing state
 */
struct DrawState {
    float overallAlpha = 1.0f;      // Global alpha multiplier
    ScissorRect scissor;             // Current scissor rectangle
    bool initialized = false;
    
    // Multiply alpha with overall alpha
    [[nodiscard]] float effectiveAlpha(float alpha) const noexcept {
        return alpha * overallAlpha;
    }
    
    void reset() noexcept {
        overallAlpha = 1.0f;
        scissor.reset();
    }
};

// ============================================================================
// Picture Cache
// ============================================================================

/**
 * @brief Picture cache entry
 */
struct CachedPicture {
    std::string path;
    Picture picture;
    bool loaded = false;
    bool failed = false;  // Mark as failed so we don't retry
    
    [[nodiscard]] bool isValid() const noexcept {
        return loaded && picture.isValid();
    }
};

/**
 * @brief Picture cache manager
 */
class PictureCache {
public:
    void clear() noexcept {
        for (auto& entry : entries_) {
            entry = CachedPicture{};
        }
        count_ = 0;
    }
    
    [[nodiscard]] size_t count() const noexcept { return count_; }
    [[nodiscard]] bool empty() const noexcept { return count_ == 0; }
    [[nodiscard]] bool full() const noexcept { return count_ >= entries_.size(); }
    
    // Find picture by path (returns nullptr if not found)
    [[nodiscard]] Picture* find(std::string_view path) noexcept {
        for (size_t i = 0; i < count_; ++i) {
            if (entries_[i].path == path && entries_[i].loaded) {
                return &entries_[i].picture;
            }
        }
        return nullptr;
    }
    
    // Add new picture to cache (returns nullptr if full)
    Picture* add(std::string_view path) noexcept {
        if (full()) return nullptr;
        
        auto& entry = entries_[count_++];
        entry.path = std::string(path);
        entry.loaded = false;
        entry.failed = false;
        return &entry.picture;
    }
    
    // Mark picture as loaded
    void markLoaded(Picture* pic) noexcept {
        for (size_t i = 0; i < count_; ++i) {
            if (&entries_[i].picture == pic) {
                entries_[i].loaded = true;
                return;
            }
        }
    }
    
    // Mark picture as failed
    void markFailed(Picture* pic) noexcept {
        for (size_t i = 0; i < count_; ++i) {
            if (&entries_[i].picture == pic) {
                entries_[i].failed = true;
                return;
            }
        }
    }
    
private:
    std::array<CachedPicture, draw_limits::MAX_CACHED_PICS> entries_;
    size_t count_ = 0;
};

// ============================================================================
// Bigfont Character Layout
// ============================================================================

/**
 * @brief Big font character coordinates
 * 
 * Used for Draw_GetBigfontSourceCoords() functionality.
 */
struct BigfontCharCoords {
    int sourceX = 0;
    int sourceY = 0;
    
    constexpr BigfontCharCoords() noexcept = default;
    constexpr BigfontCharCoords(int sx, int sy) noexcept : sourceX(sx), sourceY(sy) {}
};

// Get bigfont source coordinates for a character
[[nodiscard]] inline BigfontCharCoords getBigfontSourceCoords(char c, int charWidth, int charHeight) noexcept {
    // Characters are laid out in rows of 16
    int charIndex = static_cast<unsigned char>(c);
    int row = charIndex / 16;
    int col = charIndex % 16;
    
    return BigfontCharCoords(col * charWidth, row * charHeight);
}

// ============================================================================
// Draw Rect (for batching)
// ============================================================================

/**
 * @brief Rectangle for 2D drawing operations
 */
struct DrawRect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    
    constexpr DrawRect() noexcept = default;
    constexpr DrawRect(float x_, float y_, float w_, float h_) noexcept 
        : x(x_), y(y_), width(w_), height(h_) {}
    
    [[nodiscard]] constexpr float right() const noexcept { return x + width; }
    [[nodiscard]] constexpr float bottom() const noexcept { return y + height; }
    
    [[nodiscard]] constexpr bool contains(float px, float py) const noexcept {
        return px >= x && px < right() && py >= y && py < bottom();
    }
    
    [[nodiscard]] constexpr bool overlaps(const DrawRect& other) const noexcept {
        return x < other.right() && right() > other.x &&
               y < other.bottom() && bottom() > other.y;
    }
    
    // Fit a picture into this rect maintaining aspect ratio
    [[nodiscard]] DrawRect fitPicture(const Picture& pic) const noexcept {
        if (!pic.isValid() || width <= 0 || height <= 0) {
            return *this;
        }
        
        float picAspect = pic.aspectRatio();
        float rectAspect = width / height;
        
        DrawRect result;
        if (picAspect > rectAspect) {
            // Picture is wider - fit to width
            result.width = width;
            result.height = width / picAspect;
            result.x = x;
            result.y = y + (height - result.height) * 0.5f;
        } else {
            // Picture is taller - fit to height
            result.height = height;
            result.width = height * picAspect;
            result.x = x + (width - result.width) * 0.5f;
            result.y = y;
        }
        
        return result;
    }
};

// ============================================================================
// Factory Functions
// ============================================================================

/**
 * @brief Create a default draw state
 */
[[nodiscard]] inline DrawState createDefaultDrawState() noexcept {
    DrawState state;
    state.initialized = true;
    return state;
}

/**
 * @brief Create color from byte array
 */
[[nodiscard]] inline Color colorFromRGBA(const uint8_t rgba[4]) noexcept {
    return Color(rgba[0], rgba[1], rgba[2], rgba[3]);
}

/**
 * @brief Create color from float array [0-1]
 */
[[nodiscard]] inline Color colorFromFloatRGBA(const float rgba[4]) noexcept {
    return Color(
        static_cast<uint8_t>(std::clamp(rgba[0], 0.0f, 1.0f) * 255.0f),
        static_cast<uint8_t>(std::clamp(rgba[1], 0.0f, 1.0f) * 255.0f),
        static_cast<uint8_t>(std::clamp(rgba[2], 0.0f, 1.0f) * 255.0f),
        static_cast<uint8_t>(std::clamp(rgba[3], 0.0f, 1.0f) * 255.0f)
    );
}

} // namespace ezquake::draw
