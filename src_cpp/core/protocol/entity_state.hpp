/**
 * @file entity_state.hpp
 * @brief Entity state for network protocol
 * 
 * This file defines the NetworkEntityState class for tracking entity baseline
 * and delta-compressed state during network updates.
 * 
 * Original C: entity_state_t in quakedef.h, cl_ents.c entity handling
 */

#pragma once

#include <cstdint>
#include <array>
#include "../math/vec3.hpp"
#include "protocol_types.hpp"

namespace ezquake {

using math::Vec3;

//=============================================================================
// Network Entity State
//=============================================================================

/**
 * @brief Complete state of an entity for network transmission.
 * 
 * This represents the state that can be delta-compressed and transmitted
 * over the network. Named NetworkEntityState to distinguish from client-side
 * EntityState which may have additional fields.
 * 
 * Original: entity_state_t struct
 */
class NetworkEntityState {
public:
    //=========================================================================
    // Constructors
    //=========================================================================
    
    /// Default constructor - zero state
    constexpr NetworkEntityState() noexcept = default;
    
    /// Construct with entity number
    explicit constexpr NetworkEntityState(uint16_t number) noexcept
        : number_(number) {}
    
    //=========================================================================
    // Identity
    //=========================================================================
    
    [[nodiscard]] constexpr uint16_t number() const noexcept { return number_; }
    void setNumber(uint16_t n) noexcept { number_ = n; }
    
    //=========================================================================
    // Position & Orientation
    //=========================================================================
    
    [[nodiscard]] constexpr const Vec3& origin() const noexcept { return origin_; }
    void setOrigin(const Vec3& o) noexcept { origin_ = o; }
    
    [[nodiscard]] constexpr const Vec3& angles() const noexcept { return angles_; }
    void setAngles(const Vec3& a) noexcept { angles_ = a; }
    
    //=========================================================================
    // Model & Appearance
    //=========================================================================
    
    [[nodiscard]] constexpr uint16_t modelIndex() const noexcept { return modelIndex_; }
    void setModelIndex(uint16_t m) noexcept { modelIndex_ = m; }
    
    [[nodiscard]] constexpr uint16_t frame() const noexcept { return frame_; }
    void setFrame(uint16_t f) noexcept { frame_ = f; }
    
    [[nodiscard]] constexpr uint8_t colormap() const noexcept { return colormap_; }
    void setColormap(uint8_t c) noexcept { colormap_ = c; }
    
    [[nodiscard]] constexpr uint8_t skinnum() const noexcept { return skinnum_; }
    void setSkinnum(uint8_t s) noexcept { skinnum_ = s; }
    
    [[nodiscard]] constexpr uint8_t effects() const noexcept { return effects_; }
    void setEffects(uint8_t e) noexcept { effects_ = e; }
    
    //=========================================================================
    // FTE Extensions
    //=========================================================================
    
    [[nodiscard]] constexpr uint8_t alpha() const noexcept { return alpha_; }
    void setAlpha(uint8_t a) noexcept { alpha_ = a; }
    
    [[nodiscard]] constexpr uint8_t scale() const noexcept { return scale_; }
    void setScale(uint8_t s) noexcept { scale_ = s; }
    
    [[nodiscard]] constexpr const std::array<uint8_t, 3>& colormod() const noexcept { return colormod_; }
    void setColormod(const std::array<uint8_t, 3>& c) noexcept { colormod_ = c; }
    
    //=========================================================================
    // Flags
    //=========================================================================
    
    [[nodiscard]] constexpr uint32_t flags() const noexcept { return flags_; }
    void setFlags(uint32_t f) noexcept { flags_ = f; }
    
    [[nodiscard]] constexpr bool isSolid() const noexcept { 
        return (flags_ & EntityFlags::SOLID) != 0; 
    }
    
    //=========================================================================
    // Comparison
    //=========================================================================
    
    /**
     * @brief Compare with another state to determine what needs updating.
     */
    [[nodiscard]] uint32_t diffFlags(const NetworkEntityState& other) const noexcept {
        uint32_t diff = 0;
        
        if (origin_.x() != other.origin_.x()) diff |= EntityFlags::ORIGIN1;
        if (origin_.y() != other.origin_.y()) diff |= EntityFlags::ORIGIN2;
        if (origin_.z() != other.origin_.z()) diff |= EntityFlags::ORIGIN3;
        if (angles_.x() != other.angles_.x()) diff |= EntityFlags::ANGLE1;
        if (angles_.y() != other.angles_.y()) diff |= EntityFlags::ANGLE2;
        if (angles_.z() != other.angles_.z()) diff |= EntityFlags::ANGLE3;
        if (frame_ != other.frame_) diff |= EntityFlags::FRAME;
        if (modelIndex_ != other.modelIndex_) diff |= EntityFlags::MODEL;
        if (colormap_ != other.colormap_) diff |= EntityFlags::COLORMAP;
        if (skinnum_ != other.skinnum_) diff |= EntityFlags::SKIN;
        if (effects_ != other.effects_) diff |= EntityFlags::EFFECTS;
        
        return diff;
    }
    
    /**
     * @brief Copy all values from another state.
     */
    void copyFrom(const NetworkEntityState& other) noexcept {
        *this = other;
    }
    
    /**
     * @brief Reset to default values.
     */
    void clear() noexcept {
        *this = NetworkEntityState{};
    }
    
    /**
     * @brief Check if entity is valid (has a non-zero number).
     */
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return number_ != 0;
    }

private:
    Vec3 origin_{};
    Vec3 angles_{};
    
    uint16_t number_ = 0;
    uint16_t modelIndex_ = 0;
    uint16_t frame_ = 0;
    
    uint8_t colormap_ = 0;
    uint8_t skinnum_ = 0;
    uint8_t effects_ = 0;
    
    // FTE extensions
    uint8_t alpha_ = 255;       ///< 255 = opaque
    uint8_t scale_ = 16;        ///< 16 = 1.0 scale
    std::array<uint8_t, 3> colormod_ = {32, 32, 32}; ///< RGB color mod
    
    uint32_t flags_ = 0;
};

//=============================================================================
// Network Packet Entities
//=============================================================================

/**
 * @brief Collection of entities in a network packet.
 * 
 * Original: packet_entities_t
 */
class NetworkPacketEntities {
public:
    static constexpr size_t MAX_ENTITIES = NetworkLimits::MAX_MVD_PACKET_ENTITIES;
    
    //=========================================================================
    // Access
    //=========================================================================
    
    [[nodiscard]] size_t count() const noexcept { return count_; }
    
    [[nodiscard]] NetworkEntityState& operator[](size_t i) noexcept { return entities_[i]; }
    [[nodiscard]] const NetworkEntityState& operator[](size_t i) const noexcept { return entities_[i]; }
    
    [[nodiscard]] NetworkEntityState* begin() noexcept { return entities_.data(); }
    [[nodiscard]] NetworkEntityState* end() noexcept { return entities_.data() + count_; }
    [[nodiscard]] const NetworkEntityState* begin() const noexcept { return entities_.data(); }
    [[nodiscard]] const NetworkEntityState* end() const noexcept { return entities_.data() + count_; }
    
    //=========================================================================
    // Modification
    //=========================================================================
    
    /**
     * @brief Add an entity to the packet.
     */
    bool add(const NetworkEntityState& state) noexcept {
        if (count_ >= MAX_ENTITIES) {
            return false;
        }
        entities_[count_++] = state;
        return true;
    }
    
    /**
     * @brief Clear all entities.
     */
    void clear() noexcept {
        count_ = 0;
    }
    
    /**
     * @brief Find entity by number.
     */
    [[nodiscard]] NetworkEntityState* find(uint16_t entityNum) noexcept {
        for (size_t i = 0; i < count_; ++i) {
            if (entities_[i].number() == entityNum) {
                return &entities_[i];
            }
        }
        return nullptr;
    }
    
    [[nodiscard]] const NetworkEntityState* find(uint16_t entityNum) const noexcept {
        for (size_t i = 0; i < count_; ++i) {
            if (entities_[i].number() == entityNum) {
                return &entities_[i];
            }
        }
        return nullptr;
    }

private:
    std::array<NetworkEntityState, MAX_ENTITIES> entities_{};
    size_t count_ = 0;
};

//=============================================================================
// Player State (for svc_playerinfo)
//=============================================================================

/**
 * @brief Player state for network updates.
 * 
 * Contains the position, view angles, and movement state of a player.
 * Original: player_state_t
 */
class NetworkPlayerState {
public:
    //=========================================================================
    // Constructors
    //=========================================================================
    
    constexpr NetworkPlayerState() noexcept = default;
    
    //=========================================================================
    // Position & Movement
    //=========================================================================
    
    [[nodiscard]] constexpr const Vec3& origin() const noexcept { return origin_; }
    void setOrigin(const Vec3& o) noexcept { origin_ = o; }
    
    [[nodiscard]] constexpr const Vec3& velocity() const noexcept { return velocity_; }
    void setVelocity(const Vec3& v) noexcept { velocity_ = v; }
    
    [[nodiscard]] constexpr const Vec3& viewAngles() const noexcept { return viewAngles_; }
    void setViewAngles(const Vec3& a) noexcept { viewAngles_ = a; }
    
    [[nodiscard]] constexpr const Vec3& cmdAngles() const noexcept { return cmdAngles_; }
    void setCmdAngles(const Vec3& a) noexcept { cmdAngles_ = a; }
    
    //=========================================================================
    // Model & Appearance
    //=========================================================================
    
    [[nodiscard]] constexpr uint16_t modelIndex() const noexcept { return modelIndex_; }
    void setModelIndex(uint16_t m) noexcept { modelIndex_ = m; }
    
    [[nodiscard]] constexpr uint16_t frame() const noexcept { return frame_; }
    void setFrame(uint16_t f) noexcept { frame_ = f; }
    
    [[nodiscard]] constexpr uint8_t skinnum() const noexcept { return skinnum_; }
    void setSkinnum(uint8_t s) noexcept { skinnum_ = s; }
    
    [[nodiscard]] constexpr uint8_t effects() const noexcept { return effects_; }
    void setEffects(uint8_t e) noexcept { effects_ = e; }
    
    [[nodiscard]] constexpr uint8_t weaponFrame() const noexcept { return weaponFrame_; }
    void setWeaponFrame(uint8_t w) noexcept { weaponFrame_ = w; }
    
    //=========================================================================
    // Movement State
    //=========================================================================
    
    [[nodiscard]] constexpr uint8_t msec() const noexcept { return msec_; }
    void setMsec(uint8_t m) noexcept { msec_ = m; }
    
    [[nodiscard]] constexpr uint32_t flags() const noexcept { return flags_; }
    void setFlags(uint32_t f) noexcept { flags_ = f; }
    
    [[nodiscard]] constexpr bool isDead() const noexcept {
        return (flags_ & PlayerFlags::DEAD) != 0;
    }
    
    [[nodiscard]] constexpr bool isGib() const noexcept {
        return (flags_ & PlayerFlags::GIB) != 0;
    }
    
    [[nodiscard]] constexpr bool onGround() const noexcept {
        return (flags_ & PlayerFlags::ONGROUND) != 0 || 
               (flags_ & PlayerFlags::ONGROUND_FTE) != 0;
    }
    
    [[nodiscard]] constexpr int moveType() const noexcept {
        return PlayerFlags::extractMoveType(flags_);
    }
    
    //=========================================================================
    // User Command
    //=========================================================================
    
    [[nodiscard]] constexpr int16_t forwardMove() const noexcept { return forwardMove_; }
    void setForwardMove(int16_t f) noexcept { forwardMove_ = f; }
    
    [[nodiscard]] constexpr int16_t sideMove() const noexcept { return sideMove_; }
    void setSideMove(int16_t s) noexcept { sideMove_ = s; }
    
    [[nodiscard]] constexpr int16_t upMove() const noexcept { return upMove_; }
    void setUpMove(int16_t u) noexcept { upMove_ = u; }
    
    [[nodiscard]] constexpr uint8_t buttons() const noexcept { return buttons_; }
    void setButtons(uint8_t b) noexcept { buttons_ = b; }
    
    [[nodiscard]] constexpr uint8_t impulse() const noexcept { return impulse_; }
    void setImpulse(uint8_t i) noexcept { impulse_ = i; }

private:
    Vec3 origin_{};
    Vec3 velocity_{};
    Vec3 viewAngles_{};
    Vec3 cmdAngles_{};
    
    uint16_t modelIndex_ = 0;
    uint16_t frame_ = 0;
    
    int16_t forwardMove_ = 0;
    int16_t sideMove_ = 0;
    int16_t upMove_ = 0;
    
    uint32_t flags_ = 0;
    
    uint8_t skinnum_ = 0;
    uint8_t effects_ = 0;
    uint8_t weaponFrame_ = 0;
    uint8_t msec_ = 0;
    uint8_t buttons_ = 0;
    uint8_t impulse_ = 0;
};

//=============================================================================
// Network Entity Baseline Manager
//=============================================================================

/**
 * @brief Manages entity baselines for delta compression.
 */
class NetworkEntityBaselines {
public:
    static constexpr size_t MAX_EDICTS = NetworkLimits::MAX_EDICTS;
    
    /**
     * @brief Get baseline for entity.
     */
    [[nodiscard]] NetworkEntityState& get(uint16_t entityNum) noexcept {
        return baselines_[entityNum % MAX_EDICTS];
    }
    
    [[nodiscard]] const NetworkEntityState& get(uint16_t entityNum) const noexcept {
        return baselines_[entityNum % MAX_EDICTS];
    }
    
    /**
     * @brief Set baseline for entity.
     */
    void set(uint16_t entityNum, const NetworkEntityState& state) noexcept {
        baselines_[entityNum % MAX_EDICTS] = state;
    }
    
    /**
     * @brief Clear all baselines.
     */
    void clear() noexcept {
        for (auto& baseline : baselines_) {
            baseline.clear();
        }
    }
    
private:
    std::array<NetworkEntityState, MAX_EDICTS> baselines_{};
};

} // namespace ezquake
