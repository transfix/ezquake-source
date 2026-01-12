/**
 * @file sound_test.cpp
 * @brief Tests for sound system types
 */

#include <gtest/gtest.h>
#include "core/sound/sound_types.hpp"

using namespace ezquake::sound;
using ezquake::math::Vec3;

// =============================================================================
// Sound Limits Tests
// =============================================================================

TEST(SoundLimitsTest, Constants) {
    EXPECT_EQ(sound_limits::MAX_CHANNELS, 512);
    EXPECT_EQ(sound_limits::MAX_DYNAMIC_CHANNELS, 32);
    EXPECT_EQ(sound_limits::MAX_SOUND_PATH, 64);
    EXPECT_EQ(sound_limits::NUM_AMBIENTS, 4);
    EXPECT_EQ(sound_limits::MAX_VOLUME, 255);
}

TEST(SoundLimitsTest, ChannelRanges) {
    EXPECT_EQ(sound_limits::AMBIENT_CHANNEL_START, 32);
    EXPECT_EQ(sound_limits::STATIC_CHANNEL_START, 36);
}

TEST(SoundLimitsTest, SampleRates) {
    EXPECT_EQ(sound_limits::RATE_11K, 11025);
    EXPECT_EQ(sound_limits::RATE_22K, 22050);
    EXPECT_EQ(sound_limits::RATE_44K, 44100);
    EXPECT_EQ(sound_limits::RATE_48K, 48000);
}

// =============================================================================
// Sound Format Tests
// =============================================================================

TEST(SoundFormatTest, DefaultConstruction) {
    SoundFormat format;
    EXPECT_EQ(format.sampleRate, 44100);
    EXPECT_EQ(format.sampleWidth, 2);
    EXPECT_EQ(format.channels, 2);
}

TEST(SoundFormatTest, BytesPerSecond) {
    SoundFormat format{44100, 2, 2}; // 44.1kHz, 16-bit, stereo
    EXPECT_EQ(format.bytesPerSecond(), 44100 * 2 * 2);
}

TEST(SoundFormatTest, BytesPerSample) {
    SoundFormat mono8{22050, 1, 1};
    EXPECT_EQ(mono8.bytesPerSample(), 1);
    
    SoundFormat stereo16{44100, 2, 2};
    EXPECT_EQ(stereo16.bytesPerSample(), 4);
}

TEST(SoundFormatTest, IsStereo) {
    SoundFormat mono{44100, 2, 1};
    EXPECT_FALSE(mono.isStereo());
    
    SoundFormat stereo{44100, 2, 2};
    EXPECT_TRUE(stereo.isStereo());
}

TEST(SoundFormatTest, Is16Bit) {
    SoundFormat bit8{44100, 1, 2};
    EXPECT_FALSE(bit8.is16Bit());
    
    SoundFormat bit16{44100, 2, 2};
    EXPECT_TRUE(bit16.is16Bit());
}

// =============================================================================
// Sound Effect Tests
// =============================================================================

TEST(SoundEffectTest, DefaultConstruction) {
    SoundEffect sfx;
    EXPECT_EQ(sfx.buffer, nullptr);
    EXPECT_FALSE(sfx.isLoaded());
}

TEST(SoundEffectTest, IsLoaded) {
    SoundEffect sfx;
    sfx.buffer = reinterpret_cast<void*>(0x1234);
    EXPECT_TRUE(sfx.isLoaded());
}

// =============================================================================
// Sound Cache Tests
// =============================================================================

TEST(SoundCacheTest, Loops) {
    SoundCache cache;
    EXPECT_FALSE(cache.loops());
    
    cache.loopStart = 0;
    EXPECT_TRUE(cache.loops());
}

TEST(SoundCacheTest, Duration) {
    SoundCache cache;
    cache.format.sampleRate = 44100;
    cache.totalLength = 44100;
    
    EXPECT_NEAR(cache.duration(), 1.0f, 0.001f);
}

TEST(SoundCacheTest, DataSize) {
    SoundCache cache;
    cache.format = {44100, 2, 2}; // 4 bytes per sample
    cache.totalLength = 1000;
    
    EXPECT_EQ(cache.dataSize(), 4000);
}

// =============================================================================
// WAV Info Tests
// =============================================================================

TEST(WavInfoTest, ToFormat) {
    WavInfo wav{44100, 2, 2, -1, 1000, 44};
    
    auto format = wav.toFormat();
    EXPECT_EQ(format.sampleRate, 44100);
    EXPECT_EQ(format.sampleWidth, 2);
    EXPECT_EQ(format.channels, 2);
}

TEST(WavInfoTest, IsValid) {
    WavInfo valid{44100, 2, 2, -1, 1000, 44};
    EXPECT_TRUE(valid.isValid());
    
    WavInfo invalid{0, 2, 2, -1, 1000, 44};
    EXPECT_FALSE(invalid.isValid());
}

// =============================================================================
// Sound Channel Tests
// =============================================================================

TEST(SoundChannelTest, DefaultConstruction) {
    SoundChannel channel;
    EXPECT_EQ(channel.sfx, nullptr);
    EXPECT_FALSE(channel.isActive());
}

TEST(SoundChannelTest, IsLooping) {
    SoundChannel channel;
    EXPECT_FALSE(channel.isLooping());
    
    channel.looping = 0;
    EXPECT_TRUE(channel.isLooping());
}

TEST(SoundChannelTest, IsFinished) {
    SoundChannel channel;
    channel.endTime = 1000;
    
    EXPECT_FALSE(channel.isFinished(500));
    EXPECT_TRUE(channel.isFinished(1000));
    EXPECT_TRUE(channel.isFinished(1500));
}

TEST(SoundChannelTest, IsFinishedLooping) {
    SoundChannel channel;
    channel.endTime = 1000;
    channel.looping = 0;
    
    EXPECT_FALSE(channel.isFinished(1500)); // Looping never finishes
}

TEST(SoundChannelTest, Reset) {
    SoundChannel channel;
    channel.leftVolume = 100;
    channel.rightVolume = 100;
    channel.masterVolume = 200;
    
    channel.reset();
    EXPECT_EQ(channel.leftVolume, 0);
    EXPECT_EQ(channel.rightVolume, 0);
    EXPECT_EQ(channel.masterVolume, 0);
}

TEST(SoundChannelTest, AverageVolume) {
    SoundChannel channel;
    channel.leftVolume = 100;
    channel.rightVolume = 200;
    
    EXPECT_EQ(channel.averageVolume(), 150);
}

// =============================================================================
// Sound Hardware Tests
// =============================================================================

TEST(SoundHardwareTest, BufferSize) {
    SoundHardware hw;
    hw.samples = 4096;
    hw.sampleBits = 16;
    hw.numChannels = 2;
    
    EXPECT_EQ(hw.bufferSize(), 4096 * 2 * 2);
}

TEST(SoundHardwareTest, GetFormat) {
    SoundHardware hw;
    hw.sampleRate = 44100;
    hw.sampleBits = 16;
    hw.numChannels = 2;
    
    auto format = hw.getFormat();
    EXPECT_EQ(format.sampleRate, 44100);
    EXPECT_EQ(format.sampleWidth, 2);
    EXPECT_EQ(format.channels, 2);
}

TEST(SoundHardwareTest, IsStereo) {
    SoundHardware mono;
    mono.numChannels = 1;
    EXPECT_FALSE(mono.isStereo());
    
    SoundHardware stereo;
    stereo.numChannels = 2;
    EXPECT_TRUE(stereo.isStereo());
}

// =============================================================================
// Sound Attenuation Tests
// =============================================================================

TEST(SoundAttenuationTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(SoundAttenuation::None), 0);
    EXPECT_EQ(static_cast<uint8_t>(SoundAttenuation::Normal), 1);
    EXPECT_EQ(static_cast<uint8_t>(SoundAttenuation::Static), 3);
}

TEST(SoundAttenuationTest, Values) {
    EXPECT_EQ(getAttenuationValue(SoundAttenuation::None), 0.0f);
    EXPECT_EQ(getAttenuationValue(SoundAttenuation::Normal), 1.0f);
    EXPECT_EQ(getAttenuationValue(SoundAttenuation::Static), 3.0f);
}

// =============================================================================
// Entity Channel Tests
// =============================================================================

TEST(EntityChannelTest, EnumValues) {
    EXPECT_EQ(static_cast<int8_t>(EntityChannel::Auto), 0);
    EXPECT_EQ(static_cast<int8_t>(EntityChannel::Weapon), 1);
    EXPECT_EQ(static_cast<int8_t>(EntityChannel::Voice), 2);
}

// =============================================================================
// Ambient Type Tests
// =============================================================================

TEST(AmbientTypeTest, Index) {
    EXPECT_EQ(getAmbientIndex(AmbientType::Water), 32);
    EXPECT_EQ(getAmbientIndex(AmbientType::Sky), 33);
    EXPECT_EQ(getAmbientIndex(AmbientType::Slime), 34);
    EXPECT_EQ(getAmbientIndex(AmbientType::Lava), 35);
}

// =============================================================================
// Volume Settings Tests
// =============================================================================

TEST(VolumeSettingsTest, EffectiveVolumes) {
    VolumeSettings vol;
    vol.master = 0.5f;
    vol.effects = 0.8f;
    vol.music = 0.6f;
    
    EXPECT_NEAR(vol.effectiveEffects(), 0.4f, 0.001f);
    EXPECT_NEAR(vol.effectiveMusic(), 0.3f, 0.001f);
}

TEST(VolumeSettingsTest, ToIntVolume) {
    VolumeSettings vol;
    
    EXPECT_EQ(vol.toIntVolume(0.0f), 0);
    EXPECT_EQ(vol.toIntVolume(1.0f), 255);
    EXPECT_EQ(vol.toIntVolume(0.5f), 127);
}

TEST(VolumeSettingsTest, ClampedVolume) {
    VolumeSettings vol;
    
    EXPECT_EQ(vol.toIntVolume(-0.5f), 0);
    EXPECT_EQ(vol.toIntVolume(2.0f), 255);
}

// =============================================================================
// CD Audio State Tests
// =============================================================================

TEST(CDAudioStateTest, DefaultState) {
    CDAudioState cd;
    EXPECT_FALSE(cd.playing);
    EXPECT_FALSE(cd.looping);
    EXPECT_TRUE(cd.enabled);
    EXPECT_FALSE(cd.valid);
}

TEST(CDAudioStateTest, IsAvailable) {
    CDAudioState cd;
    EXPECT_FALSE(cd.isAvailable());
    
    cd.valid = true;
    EXPECT_TRUE(cd.isAvailable());
    
    cd.enabled = false;
    EXPECT_FALSE(cd.isAvailable());
}

TEST(CDAudioStateTest, IsActivelyPlaying) {
    CDAudioState cd;
    cd.playing = true;
    EXPECT_TRUE(cd.isActivelyPlaying());
    
    cd.paused = true;
    EXPECT_FALSE(cd.isActivelyPlaying());
}

TEST(CDAudioStateTest, Stop) {
    CDAudioState cd;
    cd.playing = true;
    cd.looping = true;
    cd.currentTrack = 5;
    
    cd.stop();
    EXPECT_FALSE(cd.playing);
    EXPECT_FALSE(cd.looping);
    EXPECT_EQ(cd.currentTrack, 0);
}

// =============================================================================
// Sound Listener Tests
// =============================================================================

TEST(SoundListenerTest, DistanceTo) {
    SoundListener listener;
    listener.origin = {0, 0, 0};
    
    float dist = listener.distanceTo({3, 4, 0});
    EXPECT_NEAR(dist, 5.0f, 0.001f);
}

TEST(SoundListenerTest, DirectionTo) {
    SoundListener listener;
    listener.origin = {0, 0, 0};
    
    auto dir = listener.directionTo({10, 0, 0});
    EXPECT_NEAR(dir.x(), 1.0f, 0.001f);
    EXPECT_NEAR(dir.y(), 0.0f, 0.001f);
    EXPECT_NEAR(dir.z(), 0.0f, 0.001f);
}

TEST(SoundListenerTest, CalculatePan) {
    SoundListener listener;
    listener.origin = {0, 0, 0};
    listener.right = {1, 0, 0};
    
    // Sound to the right
    float pan = listener.calculatePan({10, 0, 0});
    EXPECT_NEAR(pan, 1.0f, 0.001f);
    
    // Sound to the left
    pan = listener.calculatePan({-10, 0, 0});
    EXPECT_NEAR(pan, -1.0f, 0.001f);
}

// =============================================================================
// Sound Play Params Tests
// =============================================================================

TEST(SoundPlayParamsTest, ForLocal) {
    auto params = SoundPlayParams::forLocal(nullptr, 0.5f);
    
    EXPECT_EQ(params.volume, 0.5f);
    EXPECT_EQ(params.attenuation, SoundAttenuation::None);
    EXPECT_EQ(params.entityNum, 0);
}

TEST(SoundPlayParamsTest, ForStatic) {
    Vec3 origin{100, 200, 50};
    auto params = SoundPlayParams::forStatic(nullptr, origin, 0.8f);
    
    EXPECT_EQ(params.volume, 0.8f);
    EXPECT_EQ(params.attenuation, SoundAttenuation::Static);
    EXPECT_EQ(params.origin.x(), 100);
}

TEST(SoundPlayParamsTest, ForEntity) {
    Vec3 origin{100, 200, 50};
    auto params = SoundPlayParams::forEntity(nullptr, 42, EntityChannel::Weapon, 
                                              origin, 1.0f);
    
    EXPECT_EQ(params.entityNum, 42);
    EXPECT_EQ(params.entityChannel, EntityChannel::Weapon);
    EXPECT_EQ(params.attenuation, SoundAttenuation::Normal);
}

// =============================================================================
// Sound Channel Pool Tests
// =============================================================================

TEST(SoundChannelPoolTest, FindFreeDynamic) {
    SoundChannelPool pool;
    
    auto slot = pool.findFreeDynamic();
    EXPECT_TRUE(slot.has_value());
    EXPECT_EQ(slot.value(), 0);
}

TEST(SoundChannelPoolTest, FindFreeDynamicAllUsed) {
    SoundChannelPool pool;
    SoundEffect sfx;
    sfx.buffer = reinterpret_cast<void*>(1);
    
    for (std::size_t i = 0; i < sound_limits::MAX_DYNAMIC_CHANNELS; ++i) {
        pool.channels[i].sfx = &sfx;
    }
    
    auto slot = pool.findFreeDynamic();
    EXPECT_FALSE(slot.has_value());
}

TEST(SoundChannelPoolTest, FindByEntity) {
    SoundChannelPool pool;
    pool.channels[5].entityNum = 42;
    pool.channels[5].entityChannel = 2;
    
    auto slot = pool.findByEntity(42, 2);
    EXPECT_TRUE(slot.has_value());
    EXPECT_EQ(slot.value(), 5);
}

TEST(SoundChannelPoolTest, CountActive) {
    SoundChannelPool pool;
    SoundEffect sfx;
    sfx.buffer = reinterpret_cast<void*>(1);
    
    pool.channels[0].sfx = &sfx;
    pool.channels[5].sfx = &sfx;
    pool.channels[10].sfx = &sfx;
    
    EXPECT_EQ(pool.countActive(), 3);
}

TEST(SoundChannelPoolTest, StopAll) {
    SoundChannelPool pool;
    SoundEffect sfx;
    sfx.buffer = reinterpret_cast<void*>(1);
    
    pool.channels[0].sfx = &sfx;
    pool.channels[5].sfx = &sfx;
    
    pool.stopAll();
    EXPECT_EQ(pool.countActive(), 0);
}

// =============================================================================
// Resample Style Tests
// =============================================================================

TEST(ResampleStyleTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(ResampleStyle::None), 0);
    EXPECT_EQ(static_cast<uint8_t>(ResampleStyle::Linear), 1);
    EXPECT_EQ(static_cast<uint8_t>(ResampleStyle::Cubic), 2);
}
