/*
 * ezQuake C++ Port - Frame and Entity Types
 * 
 * Client-side frame and entity state.
 * 
 * Copyright (C) 2024 ezQuake Team
 */

#pragma once

#include "core/types.hpp"
#include "core/math/vec3.hpp"
#include "core/client/client_types.hpp"
#include "core/client/player_state.hpp"
#include <array>
#include <cstdint>
#include <vector>

namespace ezquake {

//=============================================================================
// EntityState - Network state of an entity
//=============================================================================

/**
 * @brief State of an entity as received from the server.
 * 
 * This represents the network-transmitted state of an entity.
 * 
 * Original: entity_state_t
 */
class EntityState {
public:
    constexpr EntityState() noexcept = default;
    
    [[nodiscard]] constexpr int number() const noexcept { return number_; }
    [[nodiscard]] constexpr int flags() const noexcept { return flags_; }
    [[nodiscard]] constexpr const Vec3& origin() const noexcept { return origin_; }
    [[nodiscard]] constexpr const Vec3& angles() const noexcept { return angles_; }
    [[nodiscard]] constexpr int modelIndex() const noexcept { return modelIndex_; }
    [[nodiscard]] constexpr int frame() const noexcept { return frame_; }
    [[nodiscard]] constexpr int colorMap() const noexcept { return colorMap_; }
    [[nodiscard]] constexpr int skinNum() const noexcept { return skinNum_; }
    [[nodiscard]] constexpr int effects() const noexcept { return effects_; }
    
    constexpr void setNumber(int num) noexcept { number_ = num; }
    constexpr void setFlags(int flags) noexcept { flags_ = flags; }
    constexpr void setOrigin(const Vec3& origin) noexcept { origin_ = origin; }
    constexpr void setAngles(const Vec3& angles) noexcept { angles_ = angles; }
    constexpr void setModelIndex(int index) noexcept { modelIndex_ = index; }
    constexpr void setFrame(int frame) noexcept { frame_ = frame; }
    constexpr void setColorMap(int map) noexcept { colorMap_ = map; }
    constexpr void setSkinNum(int skin) noexcept { skinNum_ = skin; }
    constexpr void setEffects(int effects) noexcept { effects_ = effects; }
    
    void clear() noexcept {
        number_ = 0;
        flags_ = 0;
        origin_ = Vec3{};
        angles_ = Vec3{};
        modelIndex_ = 0;
        frame_ = 0;
        colorMap_ = 0;
        skinNum_ = 0;
        effects_ = 0;
    }
    
private:
    int number_ = 0;        // Entity number
    int flags_ = 0;         // Entity flags
    Vec3 origin_;           // Position
    Vec3 angles_;           // Orientation
    int modelIndex_ = 0;    // Model to render
    int frame_ = 0;         // Animation frame
    int colorMap_ = 0;      // Color mapping
    int skinNum_ = 0;       // Skin index
    int effects_ = 0;       // Visual effects (EF_* flags)
};

//=============================================================================
// CEntity - Client-side entity with interpolation
//=============================================================================

/**
 * @brief Client-side entity with interpolation state.
 * 
 * This contains both the current entity state and data needed
 * for smooth interpolation between network updates.
 * 
 * Original: centity_t
 */
class CEntity {
public:
    CEntity() noexcept = default;
    
    // Baseline state (initial state from server)
    [[nodiscard]] const EntityState& baseline() const noexcept { return baseline_; }
    [[nodiscard]] EntityState& baseline() noexcept { return baseline_; }
    void setBaseline(const EntityState& state) noexcept { baseline_ = state; }
    
    // Current state (latest from server)
    [[nodiscard]] const EntityState& current() const noexcept { return current_; }
    [[nodiscard]] EntityState& current() noexcept { return current_; }
    void setCurrent(const EntityState& state) noexcept { current_ = state; }
    
    // Interpolated lerp origin
    [[nodiscard]] const Vec3& lerpOrigin() const noexcept { return lerpOrigin_; }
    void setLerpOrigin(const Vec3& origin) noexcept { lerpOrigin_ = origin; }
    
    // Previous frame state for interpolation
    [[nodiscard]] const Vec3& oldOrigin() const noexcept { return oldOrigin_; }
    [[nodiscard]] const Vec3& oldAngles() const noexcept { return oldAngles_; }
    [[nodiscard]] int oldFrame() const noexcept { return oldFrame_; }
    
    void setOldOrigin(const Vec3& origin) noexcept { oldOrigin_ = origin; }
    void setOldAngles(const Vec3& angles) noexcept { oldAngles_ = angles; }
    void setOldFrame(int frame) noexcept { oldFrame_ = frame; }
    
    // Sequence tracking
    [[nodiscard]] int sequence() const noexcept { return sequence_; }
    [[nodiscard]] int oldSequence() const noexcept { return oldSequence_; }
    void setSequence(int seq) noexcept { sequence_ = seq; }
    void setOldSequence(int seq) noexcept { oldSequence_ = seq; }
    
    // Lerp timing
    [[nodiscard]] double startLerp() const noexcept { return startLerp_; }
    [[nodiscard]] double deltaLerp() const noexcept { return deltaLerp_; }
    [[nodiscard]] double frameTime() const noexcept { return frameTime_; }
    
    void setStartLerp(double time) noexcept { startLerp_ = time; }
    void setDeltaLerp(double delta) noexcept { deltaLerp_ = delta; }
    void setFrameTime(double time) noexcept { frameTime_ = time; }
    
    // VWep state (player entities only)
    [[nodiscard]] int oldVwIndex() const noexcept { return oldVwIndex_; }
    [[nodiscard]] int oldVwFrame() const noexcept { return oldVwFrame_; }
    void setOldVwIndex(int index) noexcept { oldVwIndex_ = index; }
    void setOldVwFrame(int frame) noexcept { oldVwFrame_ = frame; }
    
    // Velocity (hack for prediction)
    [[nodiscard]] const Vec3& velocity() const noexcept { return velocity_; }
    void setVelocity(const Vec3& vel) noexcept { velocity_ = vel; }
    
    // Contents (for BSP collision)
    [[nodiscard]] int contents() const noexcept { return contents_; }
    void setContents(int contents) noexcept { contents_ = contents; }
    
    // Trail tracking
    [[nodiscard]] int trailNumber() const noexcept { return trailNumber_; }
    [[nodiscard]] float particleEmitTime() const noexcept { return particleEmitTime_; }
    [[nodiscard]] int coronaId() const noexcept { return coronaId_; }
    
    void setTrailNumber(int num) noexcept { trailNumber_ = num; }
    void setParticleEmitTime(float time) noexcept { particleEmitTime_ = time; }
    void setCoronaId(int id) noexcept { coronaId_ = id; }
    
    // Calculate interpolated state
    void interpolate(double time) noexcept {
        if (deltaLerp_ <= 0.0) {
            lerpOrigin_ = current_.origin();
            return;
        }
        
        double frac = (time - startLerp_) / deltaLerp_;
        if (frac < 0.0) frac = 0.0;
        if (frac > 1.0) frac = 1.0;
        
        // Linear interpolation
        lerpOrigin_ = oldOrigin_ + (current_.origin() - oldOrigin_) * static_cast<float>(frac);
    }
    
    void clear() noexcept {
        baseline_.clear();
        current_.clear();
        lerpOrigin_ = Vec3{};
        velocity_ = Vec3{};
        oldOrigin_ = Vec3{};
        oldAngles_ = Vec3{};
        oldFrame_ = 0;
        sequence_ = 0;
        oldSequence_ = 0;
        startLerp_ = 0.0;
        deltaLerp_ = 0.0;
        frameTime_ = 0.0;
        oldVwIndex_ = 0;
        oldVwFrame_ = 0;
        contents_ = 0;
        particleEmitTime_ = 0.0f;
        trailNumber_ = 0;
        coronaId_ = 0;
    }
    
private:
    EntityState baseline_;      // Initial state from server
    EntityState current_;       // Latest state from server
    
    Vec3 lerpOrigin_;           // Interpolated position
    Vec3 velocity_;             // Hack for prediction
    
    Vec3 oldOrigin_;            // Previous origin
    Vec3 oldAngles_;            // Previous angles
    int oldFrame_ = 0;          // Previous frame
    
    int sequence_ = 0;          // Update sequence
    int oldSequence_ = 0;       // Previous update sequence
    
    double startLerp_ = 0.0;    // Lerp start time
    double deltaLerp_ = 0.0;    // Lerp duration
    double frameTime_ = 0.0;    // Frame time
    
    int oldVwIndex_ = 0;        // Previous VWep index
    int oldVwFrame_ = 0;        // Previous VWep frame
    
    int contents_ = 0;          // BSP contents
    
    float particleEmitTime_ = 0.0f;  // Particle emission time
    int trailNumber_ = 0;            // Trail tracking number
    int coronaId_ = 0;               // Corona effect ID
};

//=============================================================================
// PacketEntities - Entity states received in a packet
//=============================================================================

/**
 * @brief Collection of entity states from a network packet.
 * 
 * Original: packet_entities_t
 */
class PacketEntities {
public:
    static constexpr int MAX_PACKET_ENTITIES = 512;
    
    PacketEntities() noexcept = default;
    
    [[nodiscard]] std::size_t count() const noexcept { return entities_.size(); }
    [[nodiscard]] bool empty() const noexcept { return entities_.empty(); }
    
    void add(const EntityState& entity) {
        if (entities_.size() < MAX_PACKET_ENTITIES) {
            entities_.push_back(entity);
        }
    }
    
    [[nodiscard]] const EntityState& operator[](std::size_t index) const noexcept {
        return entities_[index];
    }
    [[nodiscard]] EntityState& operator[](std::size_t index) noexcept {
        return entities_[index];
    }
    
    // Find entity by number
    [[nodiscard]] const EntityState* find(int number) const noexcept {
        for (const auto& ent : entities_) {
            if (ent.number() == number) {
                return &ent;
            }
        }
        return nullptr;
    }
    
    void clear() { entities_.clear(); }
    void reserve(std::size_t count) { entities_.reserve(count); }
    
    // Iterators
    auto begin() noexcept { return entities_.begin(); }
    auto end() noexcept { return entities_.end(); }
    auto begin() const noexcept { return entities_.begin(); }
    auto end() const noexcept { return entities_.end(); }
    
private:
    std::vector<EntityState> entities_;
};

//=============================================================================
// Frame - A single client frame
//=============================================================================

/**
 * @brief A single client frame containing all state at a point in time.
 * 
 * Contains the user command that generated this frame, timing info,
 * and all player states received from the server.
 * 
 * Original: frame_t
 */
class Frame {
public:
    Frame() noexcept = default;
    
    // The command that generated this frame
    [[nodiscard]] const UserCommand& command() const noexcept { return command_; }
    [[nodiscard]] UserCommand& command() noexcept { return command_; }
    void setCommand(const UserCommand& cmd) noexcept { command_ = cmd; }
    
    // Timing
    [[nodiscard]] double sentTime() const noexcept { return sentTime_; }
    [[nodiscard]] double receivedTime() const noexcept { return receivedTime_; }
    void setSentTime(double time) noexcept { sentTime_ = time; }
    void setReceivedTime(double time) noexcept { receivedTime_ = time; }
    
    // Size tracking
    [[nodiscard]] int sentSize() const noexcept { return sentSize_; }
    [[nodiscard]] int receivedSize() const noexcept { return receivedSize_; }
    void setSentSize(int size) noexcept { sentSize_ = size; }
    void setReceivedSize(int size) noexcept { receivedSize_ = size; }
    
    // Delta sequence
    [[nodiscard]] int deltaSequence() const noexcept { return deltaSequence_; }
    void setDeltaSequence(int seq) noexcept { deltaSequence_ = seq; }
    
    // Sequence when received
    [[nodiscard]] int seqWhenReceived() const noexcept { return seqWhenReceived_; }
    void setSeqWhenReceived(int seq) noexcept { seqWhenReceived_ = seq; }
    
    // Validity
    [[nodiscard]] bool isValid() const noexcept { return !invalid_; }
    [[nodiscard]] bool isInvalid() const noexcept { return invalid_; }
    void setInvalid(bool invalid) noexcept { invalid_ = invalid; }
    
    // QWD demo flag
    [[nodiscard]] bool inQwd() const noexcept { return inQwd_; }
    void setInQwd(bool qwd) noexcept { inQwd_ = qwd; }
    
    // Player states
    [[nodiscard]] const PlayerState& playerState(int index) const noexcept {
        return playerStates_[static_cast<std::size_t>(index)];
    }
    [[nodiscard]] PlayerState& playerState(int index) noexcept {
        return playerStates_[static_cast<std::size_t>(index)];
    }
    
    // Packet entities
    [[nodiscard]] const PacketEntities& packetEntities() const noexcept { return packetEntities_; }
    [[nodiscard]] PacketEntities& packetEntities() noexcept { return packetEntities_; }
    
    // Calculate round-trip time
    [[nodiscard]] double rtt() const noexcept {
        if (receivedTime_ <= 0.0) return -1.0;
        return receivedTime_ - sentTime_;
    }
    
    void clear() noexcept {
        command_.clear();
        sentTime_ = 0.0;
        receivedTime_ = -1.0;
        sentSize_ = 0;
        receivedSize_ = 0;
        deltaSequence_ = -1;
        seqWhenReceived_ = 0;
        invalid_ = false;
        inQwd_ = false;
        for (auto& ps : playerStates_) {
            ps.clear();
        }
        packetEntities_.clear();
    }
    
private:
    UserCommand command_;                // Command that generated this frame
    double sentTime_ = 0.0;              // Time command was sent
    double receivedTime_ = -1.0;         // Time response received, or -1
    int sentSize_ = 0;                   // Size of sent packet
    int receivedSize_ = 0;               // Size of received packet
    int deltaSequence_ = -1;             // Sequence to delta from, -1 = full
    int seqWhenReceived_ = 0;            // Server sequence when received
    bool invalid_ = false;               // True if delta was invalid
    bool inQwd_ = false;                 // True if from QWD demo
    
    std::array<PlayerState, client::MAX_CLIENTS> playerStates_;
    PacketEntities packetEntities_;
};

//=============================================================================
// FrameBuffer - Ring buffer of frames
//=============================================================================

/**
 * @brief Ring buffer of client frames for prediction.
 * 
 * Stores UPDATE_BACKUP frames for client-side prediction.
 */
class FrameBuffer {
public:
    static constexpr int BUFFER_SIZE = client::UPDATE_BACKUP;
    static constexpr int BUFFER_MASK = BUFFER_SIZE - 1;
    
    FrameBuffer() noexcept = default;
    
    // Access frame by sequence number
    [[nodiscard]] const Frame& operator[](int sequence) const noexcept {
        return frames_[static_cast<std::size_t>(sequence & BUFFER_MASK)];
    }
    [[nodiscard]] Frame& operator[](int sequence) noexcept {
        return frames_[static_cast<std::size_t>(sequence & BUFFER_MASK)];
    }
    
    // Get frame at absolute index
    [[nodiscard]] const Frame& at(std::size_t index) const noexcept {
        return frames_[index & BUFFER_MASK];
    }
    [[nodiscard]] Frame& at(std::size_t index) noexcept {
        return frames_[index & BUFFER_MASK];
    }
    
    void clear() noexcept {
        for (auto& frame : frames_) {
            frame.clear();
        }
    }
    
    [[nodiscard]] constexpr std::size_t size() const noexcept { return BUFFER_SIZE; }
    
private:
    std::array<Frame, BUFFER_SIZE> frames_;
};

} // namespace ezquake
