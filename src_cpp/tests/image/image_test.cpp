/**
 * @file image_test.cpp
 * @brief Unit tests for image, screenshot, and skin types
 */

#include <gtest/gtest.h>
#include "../../core/image/image_types.hpp"

using namespace ezquake::image;

// ============================================================================
// ImageFormat Tests
// ============================================================================

TEST(ImageFormatTest, Extensions) {
    EXPECT_EQ(imageFormatExtension(ImageFormat::PCX), ".pcx");
    EXPECT_EQ(imageFormatExtension(ImageFormat::TGA), ".tga");
    EXPECT_EQ(imageFormatExtension(ImageFormat::JPEG), ".jpg");
    EXPECT_EQ(imageFormatExtension(ImageFormat::PNG), ".png");
}

TEST(ImageFormatTest, MimeTypes) {
    EXPECT_EQ(imageFormatMimeType(ImageFormat::JPEG), "image/jpeg");
    EXPECT_EQ(imageFormatMimeType(ImageFormat::PNG), "image/png");
}

TEST(ImageFormatTest, Names) {
    EXPECT_EQ(imageFormatName(ImageFormat::PCX), "PCX");
    EXPECT_EQ(imageFormatName(ImageFormat::PNG), "PNG");
}

TEST(ImageFormatTest, SupportsAlpha) {
    EXPECT_TRUE(imageFormatSupportsAlpha(ImageFormat::PNG));
    EXPECT_TRUE(imageFormatSupportsAlpha(ImageFormat::TGA));
    EXPECT_FALSE(imageFormatSupportsAlpha(ImageFormat::JPEG));
    EXPECT_FALSE(imageFormatSupportsAlpha(ImageFormat::PCX));
}

TEST(ImageFormatTest, IsLossy) {
    EXPECT_TRUE(imageFormatIsLossy(ImageFormat::JPEG));
    EXPECT_FALSE(imageFormatIsLossy(ImageFormat::PNG));
    EXPECT_FALSE(imageFormatIsLossy(ImageFormat::TGA));
}

TEST(ImageFormatTest, Parse) {
    EXPECT_EQ(parseImageFormat("png"), ImageFormat::PNG);
    EXPECT_EQ(parseImageFormat(".png"), ImageFormat::PNG);
    EXPECT_EQ(parseImageFormat("jpg"), ImageFormat::JPEG);
    EXPECT_EQ(parseImageFormat("jpeg"), ImageFormat::JPEG);
    EXPECT_FALSE(parseImageFormat("xyz").has_value());
}

// ============================================================================
// ScreenshotTarget Tests
// ============================================================================

TEST(ScreenshotTargetTest, DefaultConstruction) {
    ScreenshotTarget target;
    EXPECT_FALSE(target.isValid());
    EXPECT_FALSE(target.hasBuffer());
}

TEST(ScreenshotTargetTest, ParameterizedConstruction) {
    ScreenshotTarget target("test.png", 1920, 1080, ImageFormat::PNG);
    EXPECT_TRUE(target.isValid());
    EXPECT_EQ(target.width, 1920u);
    EXPECT_EQ(target.height, 1080u);
    EXPECT_EQ(target.format, ImageFormat::PNG);
}

TEST(ScreenshotTargetTest, AllocateBuffer) {
    ScreenshotTarget target("test.png", 100, 100);
    EXPECT_FALSE(target.hasBuffer());
    
    EXPECT_TRUE(target.allocateBuffer());
    EXPECT_TRUE(target.hasBuffer());
    EXPECT_EQ(target.buffer.size(), 100u * 100u * 4u);
    EXPECT_TRUE(target.freeMemory);
}

TEST(ScreenshotTargetTest, UseExternalBuffer) {
    std::vector<uint8_t> external(1000);
    ScreenshotTarget target("test.png", 10, 25);
    
    target.useBuffer(external);
    EXPECT_TRUE(target.hasBuffer());
    EXPECT_FALSE(target.freeMemory);
    EXPECT_EQ(target.buffer.size(), 1000u);
}

TEST(ScreenshotTargetTest, PixelCount) {
    ScreenshotTarget target("test.png", 640, 480);
    EXPECT_EQ(target.pixelCount(), 640u * 480u);
    EXPECT_EQ(target.byteSize(), 640u * 480u * 4u);
}

// ============================================================================
// ScreenshotRequest Tests
// ============================================================================

TEST(ScreenshotRequestTest, StateChecks) {
    ScreenshotRequest request;
    EXPECT_FALSE(request.isPending());
    EXPECT_FALSE(request.isComplete());
    EXPECT_FALSE(request.hasFailed());
    
    request.state = ScreenshotState::Pending;
    EXPECT_TRUE(request.isPending());
    
    request.state = ScreenshotState::Complete;
    EXPECT_TRUE(request.isComplete());
    
    request.state = ScreenshotState::Failed;
    EXPECT_TRUE(request.hasFailed());
}

// ============================================================================
// ScreenshotQueue Tests
// ============================================================================

TEST(ScreenshotQueueTest, EnqueueDequeue) {
    ScreenshotQueue queue;
    EXPECT_TRUE(queue.isEmpty());
    
    ScreenshotRequest request;
    request.target = ScreenshotTarget("test.png", 100, 100, ImageFormat::PNG);
    
    EXPECT_TRUE(queue.enqueue(request));
    EXPECT_FALSE(queue.isEmpty());
    EXPECT_EQ(queue.size(), 1u);
}

TEST(ScreenshotQueueTest, MaxSize) {
    ScreenshotQueue queue;
    queue.maxQueueSize = 3;
    
    for (int i = 0; i < 3; ++i) {
        ScreenshotRequest req;
        EXPECT_TRUE(queue.enqueue(req));
    }
    
    EXPECT_TRUE(queue.isFull());
    
    ScreenshotRequest overflow;
    EXPECT_FALSE(queue.enqueue(overflow));
}

TEST(ScreenshotQueueTest, NextPending) {
    ScreenshotQueue queue;
    
    ScreenshotRequest req1;
    req1.target.filename = "first.png";
    queue.enqueue(req1);
    
    auto* pending = queue.nextPending();
    ASSERT_NE(pending, nullptr);
    EXPECT_EQ(pending->target.filename, "first.png");
    
    pending->state = ScreenshotState::Complete;
    pending = queue.nextPending();
    EXPECT_EQ(pending, nullptr);
}

TEST(ScreenshotQueueTest, RemoveCompleted) {
    ScreenshotQueue queue;
    
    for (int i = 0; i < 5; ++i) {
        ScreenshotRequest req;
        queue.enqueue(req);
    }
    
    queue.requests[0].state = ScreenshotState::Complete;
    queue.requests[2].state = ScreenshotState::Failed;
    queue.requests[4].state = ScreenshotState::Complete;
    
    queue.removeCompleted();
    
    EXPECT_EQ(queue.size(), 2u);
    EXPECT_EQ(queue.completedCount, 2u);
    EXPECT_EQ(queue.failedCount, 1u);
}

// ============================================================================
// SkinTextureSlot Tests
// ============================================================================

TEST(SkinTextureSlotTest, Names) {
    EXPECT_EQ(skinTextureSlotName(SkinTextureSlot::Base), "$skin-base");
    EXPECT_EQ(skinTextureSlotName(SkinTextureSlot::Fullbright), "$skin-fb");
    EXPECT_EQ(skinTextureSlotName(SkinTextureSlot::Dead), "$skin-dead");
}

TEST(SkinTextureSlotTest, IsTeammate) {
    EXPECT_FALSE(skinSlotIsTeammate(SkinTextureSlot::Base));
    EXPECT_TRUE(skinSlotIsTeammate(SkinTextureSlot::BaseTeammate));
    EXPECT_FALSE(skinSlotIsTeammate(SkinTextureSlot::Dead));
    EXPECT_TRUE(skinSlotIsTeammate(SkinTextureSlot::DeadTeammate));
}

// ============================================================================
// SkinTextureRef Tests
// ============================================================================

TEST(SkinTextureRefTest, Validity) {
    SkinTextureRef ref;
    EXPECT_FALSE(ref.isValid());
    EXPECT_FALSE(static_cast<bool>(ref));
    
    ref.id = 42;
    EXPECT_TRUE(ref.isValid());
    EXPECT_TRUE(static_cast<bool>(ref));
}

TEST(SkinTextureRefTest, Equality) {
    SkinTextureRef a{100};
    SkinTextureRef b{100};
    SkinTextureRef c{200};
    
    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

// ============================================================================
// Skin Tests
// ============================================================================

TEST(SkinTest, DefaultConstruction) {
    Skin skin;
    EXPECT_FALSE(skin.isLoaded());
    EXPECT_FALSE(skin.failedLoad);
    EXPECT_EQ(skin.bpp, 1);
}

TEST(SkinTest, SetGetName) {
    Skin skin;
    skin.setName("base");
    EXPECT_EQ(skin.getName(), "base");
}

TEST(SkinTest, NameTruncation) {
    Skin skin;
    skin.setName("this_is_a_very_long_skin_name");
    EXPECT_LE(skin.getName().size(), static_cast<size_t>(image_limits::MAX_SKIN_NAME - 1));
}

TEST(SkinTest, Is32Bit) {
    Skin skin;
    EXPECT_FALSE(skin.is32Bit());
    
    skin.bpp = 4;
    EXPECT_TRUE(skin.is32Bit());
}

TEST(SkinTest, TextureManagement) {
    Skin skin;
    EXPECT_FALSE(skin.hasTexture(SkinTextureSlot::Base));
    
    skin.setTexture(SkinTextureSlot::Base, SkinTextureRef{42});
    EXPECT_TRUE(skin.hasTexture(SkinTextureSlot::Base));
    EXPECT_EQ(skin.getTexture(SkinTextureSlot::Base).id, 42u);
    
    skin.clearTextures();
    EXPECT_FALSE(skin.hasTexture(SkinTextureSlot::Base));
}

TEST(SkinTest, Reset) {
    Skin skin("test");
    skin.failedLoad = true;
    skin.width = 100;
    skin.setTexture(SkinTextureSlot::Base, SkinTextureRef{1});
    skin.cachedData.resize(100);
    
    skin.reset();
    
    EXPECT_EQ(skin.getName(), "");
    EXPECT_FALSE(skin.failedLoad);
    EXPECT_EQ(skin.width, 0);
    EXPECT_FALSE(skin.hasTexture(SkinTextureSlot::Base));
    EXPECT_TRUE(skin.cachedData.empty());
}

// ============================================================================
// PlayerSkin Tests
// ============================================================================

TEST(PlayerSkinTest, DefaultConstruction) {
    PlayerSkin skin;
    EXPECT_FALSE(skin.hasBase());
    EXPECT_FALSE(skin.hasFullbright());
    EXPECT_FALSE(skin.hasDead());
}

TEST(PlayerSkinTest, Clear) {
    PlayerSkin skin;
    skin.base = SkinTextureRef{1};
    skin.fullbright = SkinTextureRef{2};
    skin.owned[0] = true;
    
    skin.clear();
    
    EXPECT_FALSE(skin.hasBase());
    EXPECT_FALSE(skin.hasFullbright());
    EXPECT_FALSE(skin.owned[0]);
}

// ============================================================================
// SkinCache Tests
// ============================================================================

TEST(SkinCacheTest, FindAllocate) {
    SkinCache cache;
    EXPECT_TRUE(cache.isEmpty());
    
    EXPECT_EQ(cache.find("test"), nullptr);
    
    auto* skin = cache.allocate("test");
    ASSERT_NE(skin, nullptr);
    EXPECT_EQ(cache.size(), 1u);
    
    auto* found = cache.find("test");
    EXPECT_EQ(found, skin);
}

TEST(SkinCacheTest, FindOrAllocate) {
    SkinCache cache;
    
    auto* first = cache.findOrAllocate("skin1");
    auto* second = cache.findOrAllocate("skin1");
    
    EXPECT_EQ(first, second);
    EXPECT_EQ(cache.size(), 1u);
}

TEST(SkinCacheTest, Clear) {
    SkinCache cache;
    cache.allocate("skin1");
    cache.allocate("skin2");
    
    cache.clear();
    EXPECT_TRUE(cache.isEmpty());
}

// ============================================================================
// ImageData Tests
// ============================================================================

TEST(ImageDataTest, DefaultConstruction) {
    ImageData img;
    EXPECT_FALSE(img.isValid());
    EXPECT_EQ(img.width, 0);
    EXPECT_EQ(img.height, 0);
}

TEST(ImageDataTest, SizeConstruction) {
    ImageData img(100, 50, 4);
    EXPECT_TRUE(img.isValid());
    EXPECT_EQ(img.width, 100);
    EXPECT_EQ(img.height, 50);
    EXPECT_EQ(img.channels, 4);
    EXPECT_EQ(img.byteSize(), 100u * 50u * 4u);
}

TEST(ImageDataTest, RowAccess) {
    ImageData img(10, 5, 4);
    
    auto* row0 = img.row(0);
    auto* row4 = img.row(4);
    
    ASSERT_NE(row0, nullptr);
    ASSERT_NE(row4, nullptr);
    EXPECT_EQ(row4 - row0, 4 * 10 * 4);
    
    EXPECT_EQ(img.row(-1), nullptr);
    EXPECT_EQ(img.row(5), nullptr);
}

TEST(ImageDataTest, Fill) {
    ImageData img(2, 2, 4);
    img.fill(255, 0, 0, 128);
    
    EXPECT_EQ(img.pixels[0], 255u); // R
    EXPECT_EQ(img.pixels[1], 0u);   // G
    EXPECT_EQ(img.pixels[2], 0u);   // B
    EXPECT_EQ(img.pixels[3], 128u); // A
}

TEST(ImageDataTest, FlipVertical) {
    ImageData img(2, 3, 1); // Grayscale for simplicity
    img.pixels = {1, 2, 3, 4, 5, 6}; // 3 rows of 2
    
    img.flipVertical();
    
    EXPECT_EQ(img.pixels[0], 5u);
    EXPECT_EQ(img.pixels[1], 6u);
    EXPECT_EQ(img.pixels[4], 1u);
    EXPECT_EQ(img.pixels[5], 2u);
}

TEST(ImageDataTest, Clear) {
    ImageData img(100, 100);
    EXPECT_TRUE(img.isValid());
    
    img.clear();
    EXPECT_FALSE(img.isValid());
}

// ============================================================================
// Helper Function Tests
// ============================================================================

TEST(ImageHelpersTest, GenerateFilename) {
    auto name = generateScreenshotFilename("shot", ImageFormat::PNG, 42);
    EXPECT_EQ(name, "shot_0042.png");
    
    name = generateScreenshotFilename("capture", ImageFormat::JPEG, -1);
    EXPECT_EQ(name, "capture.jpg");
}

TEST(ImageHelpersTest, CreateDefaultPlayerSkins) {
    auto skins = createDefaultPlayerSkins();
    EXPECT_EQ(skins.size(), static_cast<size_t>(image_limits::MAX_CLIENTS));
}

TEST(ImageHelpersTest, CalculateImageBytes) {
    EXPECT_EQ(calculateImageBytes(100, 100, 4), 40000u);
    EXPECT_EQ(calculateImageBytes(640, 480, 3), 921600u);
}
