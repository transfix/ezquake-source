/**
 * @file sound_types.hpp
 * @brief Sound system type definitions
 *
 * Converted from:
 * - qsound.h - Sound types and structures
 * - cdaudio.h - CD audio types
 */

#pragma once

#include <array>
#include <cstdint>
#include <string_view>
#include <optional>
#include "core/math/vec3.hpp"

namespace ezquake::sound {

// =============================================================================
// Sound Constants
// =============================================================================

namespace sound_limits {
    /// Maximum sound channels
    inline constexpr std::size_t MAX_CHANNELS = 512;
    
    /// Maximum dynamic (entity) channels
    inline constexpr std::size_t MAX_DYNAMIC_CHANNELS = 32;
    
    /// Maximum path length for sound names
    inline constexpr std::size_t MAX_SOUND_PATH = 64;
    
    /// Number of ambient sounds
    inline constexpr std::size_t NUM_AMBIENTS = 4;
    
    /// Ambient sound channels start
    inline constexpr std::size_t AMBIENT_CHANNEL_START = MAX_DYNAMIC_CHANNELS;
    
    /// Static sound channels start
    inline constexpr std::size_t STATIC_CHANNEL_START = 
        MAX_DYNAMIC_CHANNELS + NUM_AMBIENTS;
    
    /// Maximum volume value
    inline constexpr int MAX_VOLUME = 255;
    
    /// Default sample rates
    inline constexpr unsigned int RATE_11K = 11025;
    inline constexpr unsigned int RATE_22K = 22050;
    inline constexpr unsigned int RATE_44K = 44100;
    inline constexpr unsigned int RATE_48K = 48000;
}

// =============================================================================
// Sound Format
// =============================================================================

/**
 * @brief Audio sample format specification
 */
struct SoundFormat {
    unsigned int sampleRate = 44100;    // Samples per second
    unsigned int sampleWidth = 2;       // Bytes per sample (1=8bit, 2=16bit)
    unsigned int channels = 2;          // 1=mono, 2=stereo
    
    /// Get bytes per second
    [[nodiscard]] constexpr unsigned int bytesPerSecond() const noexcept {
        return sampleRate * sampleWidth * channels;
    }
    
    /// Get bytes per sample (including all channels)
    [[nodiscard]] constexpr unsigned int bytesPerSample() const noexcept {
        return sampleWidth * channels;
    }
    
    /// Check if stereo
    [[nodiscard]] constexpr bool isStereo() const noexcept {
        return channels >= 2;
    }
    
    /// Check if 16-bit
    [[nodiscard]] constexpr bool is16Bit() const noexcept {
        return sampleWidth >= 2;
    }
    
    constexpr bool operator==(const SoundFormat& other) const noexcept = default;
};

// =============================================================================
// Sound Effect Reference
// =============================================================================

/**
 * @brief Sound effect reference
 *
 * From qsound.h: sfx_t
 */
struct SoundEffect {
    std::array<char, sound_limits::MAX_SOUND_PATH> name{};
    void* buffer = nullptr;     // Loaded sound data
    
    /// Check if sound is loaded
    [[nodiscard]] constexpr bool isLoaded() const noexcept {
        return buffer != nullptr;
    }
    
    /// Get sound name as string_view
    [[nodiscard]] std::string_view getName() const noexcept {
        return std::string_view(name.data());
    }
};

// =============================================================================
// Sound Cache
// =============================================================================

/**
 * @brief Cached sound data
 *
 * From qsound.h: sfxcache_t
 */
struct SoundCache {
    SoundFormat format{};
    unsigned int totalLength = 0;   // Total samples
    int loopStart = -1;             // Loop start sample (-1 = no loop)
    
    /// Check if sound loops
    [[nodiscard]] constexpr bool loops() const noexcept {
        return loopStart >= 0;
    }
    
    /// Get duration in seconds
    [[nodiscard]] constexpr float duration() const noexcept {
        if (format.sampleRate == 0) return 0.0f;
        return static_cast<float>(totalLength) / static_cast<float>(format.sampleRate);
    }
    
    /// Get data size in bytes
    [[nodiscard]] constexpr std::size_t dataSize() const noexcept {
        return static_cast<std::size_t>(totalLength) * format.bytesPerSample();
    }
};

// =============================================================================
// WAV File Info
// =============================================================================

/**
 * @brief WAV file header information
 *
 * From qsound.h: wavinfo_t
 */
struct WavInfo {
    int sampleRate = 0;
    int sampleWidth = 0;    // Bytes per sample
    int channels = 0;
    int loopStart = -1;     // Loop point (-1 = no loop)
    int samples = 0;        // Total samples
    int dataOffset = 0;     // Byte offset to data chunk
    
    /// Convert to SoundFormat
    [[nodiscard]] constexpr SoundFormat toFormat() const noexcept {
        return SoundFormat{
            static_cast<unsigned int>(sampleRate),
            static_cast<unsigned int>(sampleWidth),
            static_cast<unsigned int>(channels)
        };
    }
    
    /// Check if valid
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return sampleRate > 0 && sampleWidth > 0 && channels > 0 && samples > 0;
    }
};

// =============================================================================
// Sound Channel
// =============================================================================

/**
 * @brief Active sound channel
 *
 * From qsound.h: channel_t
 */
struct SoundChannel {
    using Vec3 = math::Vec3;
    
    const SoundEffect* sfx = nullptr;   // Sound being played
    int leftVolume = 0;                 // Left channel volume (0-255)
    int rightVolume = 0;                // Right channel volume (0-255)
    int endTime = 0;                    // End time in paint samples
    int position = 0;                   // Current sample position
    int looping = -1;                   // Loop point (-1 = no loop)
    int entityNum = 0;                  // Entity playing sound
    int entityChannel = 0;              // Entity channel slot
    Vec3 origin{};                      // Sound origin
    float distMult = 1.0f;              // Distance attenuation multiplier
    int masterVolume = 0;               // Master volume (0-255)
    
    /// Check if channel is active
    [[nodiscard]] constexpr bool isActive() const noexcept {
        return sfx != nullptr;
    }
    
    /// Check if channel is looping
    [[nodiscard]] constexpr bool isLooping() const noexcept {
        return looping >= 0;
    }
    
    /// Check if sound has finished
    [[nodiscard]] constexpr bool isFinished(int currentTime) const noexcept {
        if (isLooping()) return false;
        return currentTime >= endTime;
    }
    
    /// Reset channel
    constexpr void reset() noexcept {
        sfx = nullptr;
        leftVolume = 0;
        rightVolume = 0;
        endTime = 0;
        position = 0;
        looping = -1;
        entityNum = 0;
        entityChannel = 0;
        origin = Vec3{};
        distMult = 1.0f;
        masterVolume = 0;
    }
    
    /// Get combined volume (average of left/right)
    [[nodiscard]] constexpr int averageVolume() const noexcept {
        return (leftVolume + rightVolume) / 2;
    }
};

// =============================================================================
// Sound Hardware State
// =============================================================================

/**
 * @brief Sound hardware/driver state
 *
 * From qsound.h: soundhw_t
 */
struct SoundHardware {
    unsigned int numChannels = 2;       // Output channels (1=mono, 2=stereo)
    unsigned int samples = 0;           // Total samples in buffer
    unsigned int samplePos = 0;         // Current write position
    unsigned int sampleBits = 16;       // Bits per sample
    unsigned int sampleRate = 44100;    // Sample rate (Hz)
    unsigned long bytesSent = 0;        // Bytes sent to driver
    uint8_t* buffer = nullptr;          // DMA buffer
    
    // Mixing state
    int paintedTime = 0;                // Last painted sample time
    int oldSamplePos = 0;               // Previous sample position
    int numWraps = 0;                   // Buffer wrap count
    int queueLength = 0;                // Queued samples
    
    /// Get buffer size in bytes
    [[nodiscard]] constexpr std::size_t bufferSize() const noexcept {
        return static_cast<std::size_t>(samples) * (sampleBits / 8) * numChannels;
    }
    
    /// Get format specification
    [[nodiscard]] constexpr SoundFormat getFormat() const noexcept {
        return SoundFormat{sampleRate, sampleBits / 8, numChannels};
    }
    
    /// Check if stereo output
    [[nodiscard]] constexpr bool isStereo() const noexcept {
        return numChannels >= 2;
    }
};

// =============================================================================
// Sound Attenuation
// =============================================================================

/**
 * @brief Sound attenuation presets
 */
enum class SoundAttenuation : uint8_t {
    None = 0,       // No attenuation (global sound)
    Normal = 1,     // Normal attenuation
    Idle = 2,       // Idle attenuation (quieter)
    Static = 3      // Static sound attenuation
};

/**
 * @brief Get attenuation multiplier
 */
[[nodiscard]] constexpr float getAttenuationValue(SoundAttenuation atten) noexcept {
    switch (atten) {
        case SoundAttenuation::None:   return 0.0f;
        case SoundAttenuation::Normal: return 1.0f;
        case SoundAttenuation::Idle:   return 2.0f;
        case SoundAttenuation::Static: return 3.0f;
        default:                       return 1.0f;
    }
}

// =============================================================================
// Entity Channel
// =============================================================================

/**
 * @brief Entity sound channel slots
 */
enum class EntityChannel : int8_t {
    Auto = 0,       // Auto-pick channel
    Weapon = 1,     // Weapon sounds
    Voice = 2,      // Voice/pain sounds
    Item = 3,       // Item pickup sounds
    Body = 4        // Body sounds
};

// =============================================================================
// Ambient Sound Types
// =============================================================================

/**
 * @brief Ambient sound types
 */
enum class AmbientType : uint8_t {
    Water = 0,
    Sky = 1,
    Slime = 2,
    Lava = 3
};

/**
 * @brief Get ambient sound index
 */
[[nodiscard]] constexpr std::size_t getAmbientIndex(AmbientType type) noexcept {
    return sound_limits::AMBIENT_CHANNEL_START + static_cast<std::size_t>(type);
}

// =============================================================================
// Volume Settings
// =============================================================================

/**
 * @brief Volume levels for different sound types
 */
struct VolumeSettings {
    float master = 1.0f;        // Master volume (0-1)
    float effects = 1.0f;       // Sound effects
    float music = 1.0f;         // Music/BGM
    float ambient = 1.0f;       // Ambient sounds
    float voice = 1.0f;         // VOIP volume
    bool swapStereo = false;    // Swap left/right channels
    
    /// Apply master volume to an effect volume
    [[nodiscard]] constexpr float effectiveEffects() const noexcept {
        return master * effects;
    }
    
    /// Apply master volume to music volume
    [[nodiscard]] constexpr float effectiveMusic() const noexcept {
        return master * music;
    }
    
    /// Convert to 0-255 range
    [[nodiscard]] constexpr int toIntVolume(float vol) const noexcept {
        float clamped = vol < 0.0f ? 0.0f : (vol > 1.0f ? 1.0f : vol);
        return static_cast<int>(clamped * sound_limits::MAX_VOLUME);
    }
};

// =============================================================================
// CD Audio State
// =============================================================================

/**
 * @brief CD audio playback state
 */
struct CDAudioState {
    uint8_t currentTrack = 0;   // Currently playing track
    bool playing = false;       // Is music playing
    bool looping = false;       // Is track looping
    bool paused = false;        // Is playback paused
    bool wasPlaying = false;    // Was playing before pause
    bool enabled = true;        // CD audio enabled
    bool valid = false;         // CD audio system initialized
    
    /// Check if audio is available
    [[nodiscard]] constexpr bool isAvailable() const noexcept {
        return valid && enabled;
    }
    
    /// Check if actually playing (not paused)
    [[nodiscard]] constexpr bool isActivelyPlaying() const noexcept {
        return playing && !paused;
    }
    
    /// Stop playback
    constexpr void stop() noexcept {
        playing = false;
        looping = false;
        paused = false;
        currentTrack = 0;
    }
};

// =============================================================================
// Sound Listener
// =============================================================================

/**
 * @brief 3D sound listener state
 */
struct SoundListener {
    using Vec3 = math::Vec3;
    
    Vec3 origin{};      // Listener position
    Vec3 forward{};     // Forward direction
    Vec3 right{};       // Right direction
    Vec3 up{};          // Up direction
    
    /// Calculate distance to sound origin
    [[nodiscard]] float distanceTo(const Vec3& soundOrigin) const noexcept {
        Vec3 diff = soundOrigin - origin;
        return diff.length();
    }
    
    /// Calculate direction to sound (normalized)
    [[nodiscard]] Vec3 directionTo(const Vec3& soundOrigin) const noexcept {
        Vec3 diff = soundOrigin - origin;
        float len = diff.length();
        if (len < 0.001f) return Vec3{};
        return diff / len;
    }
    
    /// Calculate stereo panning (-1 = left, 0 = center, 1 = right)
    [[nodiscard]] float calculatePan(const Vec3& soundOrigin) const noexcept {
        Vec3 dir = directionTo(soundOrigin);
        return dir.dot(right);
    }
};

// =============================================================================
// Resample Style
// =============================================================================

/**
 * @brief Audio resampling method
 */
enum class ResampleStyle : uint8_t {
    None = 0,           // No resampling (truncate)
    Linear = 1,         // Linear interpolation
    Cubic = 2           // Cubic interpolation
};

// =============================================================================
// Sound Play Request
// =============================================================================

/**
 * @brief Parameters for playing a sound
 */
struct SoundPlayParams {
    using Vec3 = math::Vec3;
    
    const SoundEffect* sfx = nullptr;
    Vec3 origin{};
    int entityNum = 0;
    EntityChannel entityChannel = EntityChannel::Auto;
    float volume = 1.0f;
    SoundAttenuation attenuation = SoundAttenuation::Normal;
    
    /// Create for entity sound
    [[nodiscard]] static SoundPlayParams forEntity(
        const SoundEffect* sound, int entNum, EntityChannel channel,
        const Vec3& pos, float vol = 1.0f, 
        SoundAttenuation atten = SoundAttenuation::Normal) noexcept 
    {
        return SoundPlayParams{sound, pos, entNum, channel, vol, atten};
    }
    
    /// Create for static sound
    [[nodiscard]] static SoundPlayParams forStatic(
        const SoundEffect* sound, const Vec3& pos, 
        float vol = 1.0f, SoundAttenuation atten = SoundAttenuation::Static) noexcept 
    {
        return SoundPlayParams{sound, pos, 0, EntityChannel::Auto, vol, atten};
    }
    
    /// Create for local sound (no attenuation)
    [[nodiscard]] static SoundPlayParams forLocal(
        const SoundEffect* sound, float vol = 1.0f) noexcept 
    {
        return SoundPlayParams{sound, Vec3{}, 0, EntityChannel::Auto, vol, 
                               SoundAttenuation::None};
    }
};

// =============================================================================
// Sound Channel Pool
// =============================================================================

/**
 * @brief Pool of sound channels
 */
struct SoundChannelPool {
    static constexpr std::size_t MAX_CHANNELS = sound_limits::MAX_CHANNELS;
    
    std::array<SoundChannel, MAX_CHANNELS> channels{};
    std::size_t totalChannels = 0;
    
    /// Find free dynamic channel
    [[nodiscard]] std::optional<std::size_t> findFreeDynamic() const noexcept {
        for (std::size_t i = 0; i < sound_limits::MAX_DYNAMIC_CHANNELS; ++i) {
            if (!channels[i].isActive()) {
                return i;
            }
        }
        return std::nullopt;
    }
    
    /// Find channel by entity
    [[nodiscard]] std::optional<std::size_t> findByEntity(
        int entityNum, int entityChannel) const noexcept 
    {
        for (std::size_t i = 0; i < sound_limits::MAX_DYNAMIC_CHANNELS; ++i) {
            if (channels[i].entityNum == entityNum && 
                channels[i].entityChannel == entityChannel) {
                return i;
            }
        }
        return std::nullopt;
    }
    
    /// Count active channels
    [[nodiscard]] std::size_t countActive() const noexcept {
        std::size_t count = 0;
        for (const auto& ch : channels) {
            if (ch.isActive()) ++count;
        }
        return count;
    }
    
    /// Stop all sounds
    void stopAll() noexcept {
        for (auto& ch : channels) {
            ch.reset();
        }
    }
};

} // namespace ezquake::sound
