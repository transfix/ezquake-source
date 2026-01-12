/**
 * @file temp_entity_test.cpp
 * @brief Tests for temporary entity subsystem
 */

#include <gtest/gtest.h>
#include "core/entity/entity.hpp"
#include <cmath>

using namespace ezquake;

//=============================================================================
// TempEntityType Tests
//=============================================================================

TEST(TempEntityTypeTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(TempEntityType::Spike), 0);
    EXPECT_EQ(static_cast<uint8_t>(TempEntityType::SuperSpike), 1);
    EXPECT_EQ(static_cast<uint8_t>(TempEntityType::Gunshot), 2);
    EXPECT_EQ(static_cast<uint8_t>(TempEntityType::Explosion), 3);
    EXPECT_EQ(static_cast<uint8_t>(TempEntityType::TarExplosion), 4);
    EXPECT_EQ(static_cast<uint8_t>(TempEntityType::Lightning1), 5);
    EXPECT_EQ(static_cast<uint8_t>(TempEntityType::Lightning2), 6);
    EXPECT_EQ(static_cast<uint8_t>(TempEntityType::WizSpike), 7);
    EXPECT_EQ(static_cast<uint8_t>(TempEntityType::KnightSpike), 8);
    EXPECT_EQ(static_cast<uint8_t>(TempEntityType::Lightning3), 9);
    EXPECT_EQ(static_cast<uint8_t>(TempEntityType::LavaSplash), 10);
    EXPECT_EQ(static_cast<uint8_t>(TempEntityType::Teleport), 11);
    EXPECT_EQ(static_cast<uint8_t>(TempEntityType::Blood), 12);
    EXPECT_EQ(static_cast<uint8_t>(TempEntityType::LightningBlood), 13);
}

TEST(TempEntityTypeTest, Names) {
    EXPECT_EQ(tempEntityTypeName(TempEntityType::Spike), "spike");
    EXPECT_EQ(tempEntityTypeName(TempEntityType::Explosion), "explosion");
    EXPECT_EQ(tempEntityTypeName(TempEntityType::Lightning1), "lightning1");
    EXPECT_EQ(tempEntityTypeName(TempEntityType::Teleport), "teleport");
    EXPECT_EQ(tempEntityTypeName(TempEntityType::Blood), "blood");
}

TEST(TempEntityTypeTest, IsLightning) {
    EXPECT_TRUE(isLightningType(TempEntityType::Lightning1));
    EXPECT_TRUE(isLightningType(TempEntityType::Lightning2));
    EXPECT_TRUE(isLightningType(TempEntityType::Lightning3));
    EXPECT_FALSE(isLightningType(TempEntityType::Spike));
    EXPECT_FALSE(isLightningType(TempEntityType::Explosion));
}

TEST(TempEntityTypeTest, IsPositionOnly) {
    EXPECT_TRUE(isPositionOnlyType(TempEntityType::Spike));
    EXPECT_TRUE(isPositionOnlyType(TempEntityType::Explosion));
    EXPECT_TRUE(isPositionOnlyType(TempEntityType::Teleport));
    EXPECT_FALSE(isPositionOnlyType(TempEntityType::Lightning1));
    EXPECT_FALSE(isPositionOnlyType(TempEntityType::Gunshot));
    EXPECT_FALSE(isPositionOnlyType(TempEntityType::Blood));
}

//=============================================================================
// TrailType Tests
//=============================================================================

TEST(TrailTypeTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(TrailType::Rocket), 0);
    EXPECT_EQ(static_cast<uint8_t>(TrailType::Grenade), 1);
    EXPECT_EQ(static_cast<uint8_t>(TrailType::Rail), 8);
    EXPECT_EQ(static_cast<uint8_t>(TrailType::Bleeding2), 13);
}

TEST(TrailTypeTest, Names) {
    EXPECT_EQ(trailTypeName(TrailType::Rocket), "rocket");
    EXPECT_EQ(trailTypeName(TrailType::Grenade), "grenade");
    EXPECT_EQ(trailTypeName(TrailType::Blood), "blood");
    EXPECT_EQ(trailTypeName(TrailType::Rail), "rail");
}

//=============================================================================
// ExplosionType Tests
//=============================================================================

TEST(ExplosionTypeTest, Values) {
    EXPECT_EQ(static_cast<uint8_t>(ExplosionType::Normal), 0);
    EXPECT_EQ(static_cast<uint8_t>(ExplosionType::Teleport), 2);
    EXPECT_EQ(static_cast<uint8_t>(ExplosionType::Blob), 6);
    EXPECT_EQ(static_cast<uint8_t>(ExplosionType::None), 10);
}

TEST(ExplosionTypeTest, Names) {
    EXPECT_EQ(explosionTypeName(ExplosionType::Normal), "normal");
    EXPECT_EQ(explosionTypeName(ExplosionType::Blob), "blob");
    EXPECT_EQ(explosionTypeName(ExplosionType::Detpack), "detpack");
    EXPECT_EQ(explosionTypeName(ExplosionType::None), "none");
}

//=============================================================================
// Constants Tests
//=============================================================================

TEST(TempEntityConstantsTest, Values) {
    EXPECT_EQ(temp_entity::MAX_TEMP_ENTITIES, 32u);
    EXPECT_EQ(temp_entity::MAX_BEAMS, 32u);
    EXPECT_EQ(temp_entity::MAX_EXPLOSIONS, 32u);
    EXPECT_EQ(temp_entity::MAX_LIGHTNING_BEAMS, 10u);
    EXPECT_FLOAT_EQ(temp_entity::BEAM_DURATION, 0.2f);
    EXPECT_FLOAT_EQ(temp_entity::BEAM_SEGMENT_LENGTH, 30.0f);
}

TEST(TempEntityConstantsTest, BloodColors) {
    EXPECT_EQ(temp_entity::DEFAULT_LG_BLOOD_COLOR, 225);
    EXPECT_EQ(temp_entity::DEFAULT_RL_BLOOD_SMALL, 225);
    EXPECT_EQ(temp_entity::DEFAULT_RL_BLOOD_BIG, 73);
    EXPECT_EQ(temp_entity::DEFAULT_SG_BLOOD_COLOR, 73);
}

//=============================================================================
// SavedTempEntity Tests
//=============================================================================

TEST(SavedTempEntityTest, DefaultState) {
    SavedTempEntity entity;
    EXPECT_EQ(entity.position[0], 0.0f);
    EXPECT_EQ(entity.time, 0.0f);
    EXPECT_EQ(entity.type, TempEntityType::Spike);
}

TEST(SavedTempEntityTest, IsExpired) {
    SavedTempEntity entity;
    entity.time = 10.0f;
    
    EXPECT_FALSE(entity.isExpired(10.5f));  // 0.5s ago
    EXPECT_FALSE(entity.isExpired(11.5f));  // 1.5s ago
    EXPECT_TRUE(entity.isExpired(12.5f));   // 2.5s ago (default 2s max age)
    EXPECT_TRUE(entity.isExpired(11.5f, 1.0f)); // Custom 1s max age
}

//=============================================================================
// TempEntityHistory Tests
//=============================================================================

class TempEntityHistoryTest : public ::testing::Test {
protected:
    TempEntityHistory history_;
};

TEST_F(TempEntityHistoryTest, DefaultState) {
    EXPECT_TRUE(history_.empty());
    EXPECT_EQ(history_.size(), 0u);
    EXPECT_EQ(history_.writeIndex(), 0u);
}

TEST_F(TempEntityHistoryTest, AddEntity) {
    history_.add(Vec3{100, 200, 300}, 1.0f, TempEntityType::Explosion);
    
    EXPECT_FALSE(history_.empty());
    EXPECT_EQ(history_.size(), 1u);
    EXPECT_EQ(history_.writeIndex(), 1u);
    
    const auto& entity = history_.at(0);
    EXPECT_EQ(entity.position[0], 100.0f);
    EXPECT_EQ(entity.time, 1.0f);
    EXPECT_EQ(entity.type, TempEntityType::Explosion);
}

TEST_F(TempEntityHistoryTest, RingBuffer) {
    // Fill beyond max
    for (size_t i = 0; i < TempEntityHistory::MAX_ENTITIES + 5; ++i) {
        history_.add(Vec3{static_cast<float>(i), 0, 0}, static_cast<float>(i), TempEntityType::Spike);
    }
    
    EXPECT_EQ(history_.size(), TempEntityHistory::MAX_ENTITIES);
    EXPECT_EQ(history_.writeIndex(), 5u);  // Wrapped around
}

TEST_F(TempEntityHistoryTest, Clear) {
    history_.add(Vec3{0, 0, 0}, 1.0f, TempEntityType::Blood);
    history_.clear();
    
    EXPECT_TRUE(history_.empty());
    EXPECT_EQ(history_.size(), 0u);
}

//=============================================================================
// ExplosionState Tests
//=============================================================================

class ExplosionStateTest : public ::testing::Test {
protected:
    ExplosionState explosion_;
};

TEST_F(ExplosionStateTest, DefaultState) {
    EXPECT_EQ(explosion_.origin[0], 0.0f);
    EXPECT_EQ(explosion_.startTime, 0.0f);
    EXPECT_FALSE(explosion_.active);
}

TEST_F(ExplosionStateTest, CurrentFrame) {
    explosion_.start(Vec3{0, 0, 0}, 10.0f);
    
    EXPECT_EQ(explosion_.currentFrame(10.0f), 0);   // t=0
    EXPECT_EQ(explosion_.currentFrame(10.1f), 1);   // t=0.1s at 10fps
    EXPECT_EQ(explosion_.currentFrame(10.5f), 5);   // t=0.5s at 10fps
}

TEST_F(ExplosionStateTest, IsComplete) {
    explosion_.start(Vec3{0, 0, 0}, 10.0f);
    
    EXPECT_FALSE(explosion_.isComplete(10.0f));
    EXPECT_FALSE(explosion_.isComplete(10.5f));
    EXPECT_TRUE(explosion_.isComplete(10.7f));  // Frame 7 >= 6
}

TEST_F(ExplosionStateTest, Start) {
    explosion_.start(Vec3{100, 200, 300}, 5.0f);
    
    EXPECT_EQ(explosion_.origin[0], 100.0f);
    EXPECT_EQ(explosion_.origin[1], 200.0f);
    EXPECT_EQ(explosion_.origin[2], 300.0f);
    EXPECT_EQ(explosion_.startTime, 5.0f);
    EXPECT_TRUE(explosion_.active);
}

TEST_F(ExplosionStateTest, Clear) {
    explosion_.start(Vec3{100, 200, 300}, 5.0f);
    explosion_.clear();
    
    EXPECT_EQ(explosion_.origin[0], 0.0f);
    EXPECT_EQ(explosion_.startTime, 0.0f);
    EXPECT_FALSE(explosion_.active);
}

//=============================================================================
// ExplosionPool Tests
//=============================================================================

class ExplosionPoolTest : public ::testing::Test {
protected:
    ExplosionPool pool_;
};

TEST_F(ExplosionPoolTest, DefaultState) {
    EXPECT_EQ(pool_.activeCount(), 0u);
}

TEST_F(ExplosionPoolTest, Allocate) {
    ExplosionState* exp = pool_.allocate();
    ASSERT_NE(exp, nullptr);
    
    exp->start(Vec3{0, 0, 0}, 1.0f);
    EXPECT_EQ(pool_.activeCount(), 1u);
}

TEST_F(ExplosionPoolTest, AllocateMultiple) {
    for (int i = 0; i < 5; ++i) {
        ExplosionState* exp = pool_.allocate();
        ASSERT_NE(exp, nullptr);
        exp->start(Vec3{static_cast<float>(i), 0, 0}, static_cast<float>(i));
    }
    
    EXPECT_EQ(pool_.activeCount(), 5u);
}

TEST_F(ExplosionPoolTest, Update) {
    ExplosionState* exp = pool_.allocate();
    exp->start(Vec3{0, 0, 0}, 1.0f);
    
    EXPECT_EQ(pool_.activeCount(), 1u);
    
    pool_.update(2.0f);  // Should complete and clear
    
    EXPECT_EQ(pool_.activeCount(), 0u);
}

TEST_F(ExplosionPoolTest, ForEachActive) {
    ExplosionState* exp1 = pool_.allocate();
    exp1->start(Vec3{1, 0, 0}, 1.0f);
    
    ExplosionState* exp2 = pool_.allocate();
    exp2->start(Vec3{2, 0, 0}, 2.0f);
    
    int count = 0;
    float sum = 0;
    pool_.forEachActive([&](const ExplosionState& e) {
        ++count;
        sum += e.origin[0];
    });
    
    EXPECT_EQ(count, 2);
    EXPECT_FLOAT_EQ(sum, 3.0f);
}

TEST_F(ExplosionPoolTest, Clear) {
    ExplosionState* exp = pool_.allocate();
    exp->start(Vec3{0, 0, 0}, 1.0f);
    
    pool_.clear();
    
    EXPECT_EQ(pool_.activeCount(), 0u);
}

//=============================================================================
// BeamState Tests
//=============================================================================

class BeamStateTest : public ::testing::Test {
protected:
    BeamState beam_;
};

TEST_F(BeamStateTest, DefaultState) {
    EXPECT_EQ(beam_.entity, 0);
    EXPECT_EQ(beam_.modelType, BeamModelType::Bolt1);
    EXPECT_EQ(beam_.endTime, 0.0f);
    EXPECT_FALSE(beam_.isActive(0.0f));
}

TEST_F(BeamStateTest, IsActive) {
    beam_.endTime = 10.0f;
    
    EXPECT_TRUE(beam_.isActive(9.0f));
    EXPECT_TRUE(beam_.isActive(9.9f));
    EXPECT_FALSE(beam_.isActive(10.0f));
    EXPECT_FALSE(beam_.isActive(11.0f));
}

TEST_F(BeamStateTest, Length) {
    beam_.start = Vec3{0, 0, 0};
    beam_.end = Vec3{300, 400, 0};  // 3-4-5 triangle scaled by 100
    
    EXPECT_FLOAT_EQ(beam_.length(), 500.0f);
}

TEST_F(BeamStateTest, Direction) {
    beam_.start = Vec3{0, 0, 0};
    beam_.end = Vec3{100, 0, 0};
    
    Vec3 dir = beam_.direction();
    EXPECT_NEAR(dir[0], 1.0f, 0.001f);
    EXPECT_NEAR(dir[1], 0.0f, 0.001f);
    EXPECT_NEAR(dir[2], 0.0f, 0.001f);
}

TEST_F(BeamStateTest, AnglesPointingUp) {
    beam_.start = Vec3{0, 0, 0};
    beam_.end = Vec3{0, 0, 100};
    
    auto [pitch, yaw] = beam_.angles();
    EXPECT_NEAR(pitch, 90.0f, 0.1f);
}

TEST_F(BeamStateTest, AnglesPointingDown) {
    beam_.start = Vec3{0, 0, 100};
    beam_.end = Vec3{0, 0, 0};
    
    auto [pitch, yaw] = beam_.angles();
    EXPECT_NEAR(pitch, 270.0f, 0.1f);
}

TEST_F(BeamStateTest, AnglesPointingForward) {
    beam_.start = Vec3{0, 0, 0};
    beam_.end = Vec3{100, 0, 0};
    
    auto [pitch, yaw] = beam_.angles();
    EXPECT_NEAR(pitch, 0.0f, 0.1f);
    EXPECT_NEAR(yaw, 0.0f, 0.1f);
}

TEST_F(BeamStateTest, Update) {
    beam_.update(5, BeamModelType::Bolt2, Vec3{0, 0, 0}, Vec3{100, 0, 0}, 10.0f);
    
    EXPECT_EQ(beam_.entity, 5);
    EXPECT_EQ(beam_.modelType, BeamModelType::Bolt2);
    EXPECT_EQ(beam_.start[0], 0.0f);
    EXPECT_EQ(beam_.end[0], 100.0f);
    EXPECT_FLOAT_EQ(beam_.endTime, 10.2f);  // 10.0 + BEAM_DURATION
}

TEST_F(BeamStateTest, Clear) {
    beam_.update(5, BeamModelType::Bolt2, Vec3{0, 0, 0}, Vec3{100, 0, 0}, 10.0f);
    beam_.clear();
    
    EXPECT_EQ(beam_.entity, 0);
    EXPECT_EQ(beam_.endTime, 0.0f);
    EXPECT_FALSE(beam_.isActive(10.0f));
}

//=============================================================================
// BeamPool Tests
//=============================================================================

class BeamPoolTest : public ::testing::Test {
protected:
    BeamPool pool_;
};

TEST_F(BeamPoolTest, DefaultState) {
    EXPECT_EQ(pool_.activeCount(0.0f), 0u);
}

TEST_F(BeamPoolTest, UpdateBeam) {
    bool success = pool_.updateBeam(1, BeamModelType::Bolt1, 
                                     Vec3{0, 0, 0}, Vec3{100, 0, 0}, 10.0f);
    
    EXPECT_TRUE(success);
    EXPECT_EQ(pool_.activeCount(10.0f), 1u);
}

TEST_F(BeamPoolTest, UpdateSameEntity) {
    pool_.updateBeam(1, BeamModelType::Bolt1, Vec3{0, 0, 0}, Vec3{100, 0, 0}, 10.0f);
    pool_.updateBeam(1, BeamModelType::Bolt1, Vec3{0, 0, 0}, Vec3{200, 0, 0}, 10.1f);
    
    // Should still be just 1 beam (updated in place)
    EXPECT_EQ(pool_.activeCount(10.1f), 1u);
}

TEST_F(BeamPoolTest, MultipleEntities) {
    pool_.updateBeam(1, BeamModelType::Bolt1, Vec3{0, 0, 0}, Vec3{100, 0, 0}, 10.0f);
    pool_.updateBeam(2, BeamModelType::Bolt2, Vec3{0, 0, 0}, Vec3{100, 0, 0}, 10.0f);
    pool_.updateBeam(3, BeamModelType::Bolt3, Vec3{0, 0, 0}, Vec3{100, 0, 0}, 10.0f);
    
    EXPECT_EQ(pool_.activeCount(10.0f), 3u);
}

TEST_F(BeamPoolTest, BeamsExpire) {
    pool_.updateBeam(1, BeamModelType::Bolt1, Vec3{0, 0, 0}, Vec3{100, 0, 0}, 10.0f);
    
    EXPECT_EQ(pool_.activeCount(10.0f), 1u);
    EXPECT_EQ(pool_.activeCount(10.3f), 0u);  // Expired after BEAM_DURATION
}

TEST_F(BeamPoolTest, GetPlayerBeamEnd) {
    pool_.updateBeam(1, BeamModelType::Bolt1, Vec3{0, 0, 0}, Vec3{500, 0, 0}, 10.0f);
    
    auto beamEnd = pool_.getPlayerBeamEnd(0, 10.0f);  // Player 0 = entity 1
    ASSERT_TRUE(beamEnd.has_value());
    EXPECT_EQ((*beamEnd)[0], 500.0f);
}

TEST_F(BeamPoolTest, GetPlayerBeamEndNoBeam) {
    auto beamEnd = pool_.getPlayerBeamEnd(0, 10.0f);
    EXPECT_FALSE(beamEnd.has_value());
}

TEST_F(BeamPoolTest, ForEachActive) {
    pool_.updateBeam(1, BeamModelType::Bolt1, Vec3{1, 0, 0}, Vec3{100, 0, 0}, 10.0f);
    pool_.updateBeam(2, BeamModelType::Bolt2, Vec3{2, 0, 0}, Vec3{100, 0, 0}, 10.0f);
    
    int count = 0;
    pool_.forEachActive(10.0f, [&](const BeamState& b) {
        ++count;
    });
    
    EXPECT_EQ(count, 2);
}

TEST_F(BeamPoolTest, Clear) {
    pool_.updateBeam(1, BeamModelType::Bolt1, Vec3{0, 0, 0}, Vec3{100, 0, 0}, 10.0f);
    pool_.clear();
    
    EXPECT_EQ(pool_.activeCount(10.0f), 0u);
}

//=============================================================================
// RailColor Tests
//=============================================================================

TEST(RailColorTest, ToPalette) {
    EXPECT_EQ(railColorToPalette(RailColor::White), 6);
    EXPECT_EQ(railColorToPalette(RailColor::Blue), 208);
    EXPECT_EQ(railColorToPalette(RailColor::Green), 180);
    EXPECT_EQ(railColorToPalette(RailColor::LightBlue), 35);
    EXPECT_EQ(railColorToPalette(RailColor::Red), 224);
    EXPECT_EQ(railColorToPalette(RailColor::Magenta), 133);
    EXPECT_EQ(railColorToPalette(RailColor::Yellow), 192);
}

//=============================================================================
// RailTrailInfo Tests
//=============================================================================

TEST(RailTrailInfoTest, DecodeValid) {
    // Entity -1: -1 & 7 = 7, (-1 >> 3) & 63 = 63
    auto info = RailTrailInfo::decode(-1);
    ASSERT_TRUE(info.has_value());
    EXPECT_EQ(info->playerNum, 63);
    EXPECT_EQ(info->color, RailColor::White2);
    
    // Entity -512: -512 & 7 = 0, (-512 >> 3) & 63 = 0
    info = RailTrailInfo::decode(-512);
    ASSERT_TRUE(info.has_value());
    EXPECT_EQ(info->playerNum, 0);
    EXPECT_EQ(info->color, RailColor::White);
}

TEST(RailTrailInfoTest, DecodeColorBits) {
    // Entity -8 encodes color 0
    auto info = RailTrailInfo::decode(-8);
    ASSERT_TRUE(info.has_value());
    EXPECT_EQ(info->color, RailColor::White);
    
    // Entity -9 encodes color 7
    info = RailTrailInfo::decode(-9);
    ASSERT_TRUE(info.has_value());
    EXPECT_EQ(info->color, RailColor::White2);
}

TEST(RailTrailInfoTest, DecodeInvalid) {
    EXPECT_FALSE(RailTrailInfo::decode(0).has_value());
    EXPECT_FALSE(RailTrailInfo::decode(1).has_value());
    EXPECT_FALSE(RailTrailInfo::decode(-513).has_value());
}

TEST(RailTrailInfoTest, PaletteIndex) {
    RailTrailInfo info;
    info.color = RailColor::Blue;
    EXPECT_EQ(info.paletteIndex(), 208);
}

//=============================================================================
// InstagibTrailType Tests
//=============================================================================

TEST(InstagibTrailTypeTest, ToTrailType) {
    EXPECT_EQ(instagibToTrailType(InstagibTrailType::Grenade), TrailType::Grenade);
    EXPECT_EQ(instagibToTrailType(InstagibTrailType::Rocket), TrailType::Rocket);
    EXPECT_EQ(instagibToTrailType(InstagibTrailType::Rail), TrailType::Rail);
    EXPECT_FALSE(instagibToTrailType(InstagibTrailType::None).has_value());
    EXPECT_FALSE(instagibToTrailType(InstagibTrailType::ClassicRail).has_value());
    EXPECT_FALSE(instagibToTrailType(InstagibTrailType::QmbRail).has_value());
}

//=============================================================================
// SparkParams Tests
//=============================================================================

TEST(SparkParamsTest, WizSpike) {
    auto params = SparkParams::wizSpike(Vec3{100, 200, 300});
    
    EXPECT_EQ(params.position[0], 100.0f);
    EXPECT_EQ(params.color[0], 0);
    EXPECT_EQ(params.color[1], 124);
    EXPECT_EQ(params.color[2], 0);
    EXPECT_EQ(params.count, 20);
    EXPECT_FLOAT_EQ(params.size, 90.0f);
}

TEST(SparkParamsTest, KnightSpike) {
    auto params = SparkParams::knightSpike(Vec3{0, 0, 0});
    
    EXPECT_EQ(params.color[0], 255);
    EXPECT_EQ(params.color[1], 77);
    EXPECT_EQ(params.color[2], 0);
    EXPECT_EQ(params.count, 20);
    EXPECT_FLOAT_EQ(params.size, 150.0f);
}

TEST(SparkParamsTest, Lightning) {
    std::array<uint8_t, 3> color = {255, 255, 0};
    auto params = SparkParams::lightning(Vec3{0, 0, 0}, 2.0f, 100.0f, color);
    
    EXPECT_EQ(params.count, 6);  // 3 * 2.0
    EXPECT_FLOAT_EQ(params.size, 100.0f);
    EXPECT_FLOAT_EQ(params.duration, 0.25f);
}

//=============================================================================
// TempEntityState Tests
//=============================================================================

class TempEntityStateTest : public ::testing::Test {
protected:
    TempEntityState state_;
};

TEST_F(TempEntityStateTest, DefaultState) {
    EXPECT_EQ(state_.beams.activeCount(0.0f), 0u);
    EXPECT_EQ(state_.explosions.activeCount(), 0u);
    EXPECT_TRUE(state_.history.empty());
    EXPECT_FALSE(state_.playerBeamUpdated);
}

TEST_F(TempEntityStateTest, Clear) {
    state_.beams.updateBeam(1, BeamModelType::Bolt1, Vec3{0, 0, 0}, Vec3{100, 0, 0}, 10.0f);
    auto exp = state_.explosions.allocate();
    exp->start(Vec3{0, 0, 0}, 10.0f);
    state_.history.add(Vec3{0, 0, 0}, 10.0f, TempEntityType::Explosion);
    state_.playerBeamUpdated = true;
    
    state_.clear();
    
    EXPECT_EQ(state_.beams.activeCount(10.0f), 0u);
    EXPECT_EQ(state_.explosions.activeCount(), 0u);
    EXPECT_TRUE(state_.history.empty());
    EXPECT_FALSE(state_.playerBeamUpdated);
}

TEST_F(TempEntityStateTest, Update) {
    auto exp = state_.explosions.allocate();
    exp->start(Vec3{0, 0, 0}, 10.0f);
    
    EXPECT_EQ(state_.explosions.activeCount(), 1u);
    
    state_.update(11.0f);  // After explosion completes
    
    EXPECT_EQ(state_.explosions.activeCount(), 0u);
}
