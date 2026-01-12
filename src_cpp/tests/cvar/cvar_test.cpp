/*
 * ezQuake C++ Port - Cvar System Unit Tests
 * 
 * Copyright (C) 2024 ezQuake Team
 */

#include <gtest/gtest.h>
#include "core/cvar/cvar_registry.hpp"

using namespace ezquake;

//=============================================================================
// CvarColor Tests
//=============================================================================

TEST(CvarColorTest, DefaultConstruction) {
    CvarColor color;
    EXPECT_EQ(color.r, 255);
    EXPECT_EQ(color.g, 255);
    EXPECT_EQ(color.b, 255);
    EXPECT_EQ(color.a, 255);
}

TEST(CvarColorTest, ComponentConstruction) {
    CvarColor color(128, 64, 32, 200);
    EXPECT_EQ(color.r, 128);
    EXPECT_EQ(color.g, 64);
    EXPECT_EQ(color.b, 32);
    EXPECT_EQ(color.a, 200);
}

TEST(CvarColorTest, FromStringRGB) {
    auto color = CvarColor::fromString("128 64 32");
    EXPECT_EQ(color.r, 128);
    EXPECT_EQ(color.g, 64);
    EXPECT_EQ(color.b, 32);
    EXPECT_EQ(color.a, 255);
}

TEST(CvarColorTest, FromStringRGBA) {
    auto color = CvarColor::fromString("128 64 32 200");
    EXPECT_EQ(color.r, 128);
    EXPECT_EQ(color.g, 64);
    EXPECT_EQ(color.b, 32);
    EXPECT_EQ(color.a, 200);
}

TEST(CvarColorTest, FromTrackerString) {
    auto color = CvarColor::fromTrackerString("900");
    EXPECT_EQ(color.r, 255); // 9 -> 255
    EXPECT_EQ(color.g, 0);   // 0 -> 0
    EXPECT_EQ(color.b, 0);   // 0 -> 0
}

TEST(CvarColorTest, ToString) {
    CvarColor color(128, 64, 32);
    EXPECT_EQ(color.toString(), "128 64 32");
    
    CvarColor colorWithAlpha(128, 64, 32, 200);
    EXPECT_EQ(colorWithAlpha.toString(), "128 64 32 200");
}

TEST(CvarColorTest, Equality) {
    CvarColor a(100, 150, 200);
    CvarColor b(100, 150, 200);
    CvarColor c(100, 150, 201);
    
    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

//=============================================================================
// Cvar Flags Tests
//=============================================================================

TEST(CvarFlagsTest, BitwiseOr) {
    CvarFlags flags = CvarFlags::Archive | CvarFlags::UserInfo;
    EXPECT_TRUE(hasFlag(flags, CvarFlags::Archive));
    EXPECT_TRUE(hasFlag(flags, CvarFlags::UserInfo));
    EXPECT_FALSE(hasFlag(flags, CvarFlags::ReadOnly));
}

TEST(CvarFlagsTest, BitwiseAnd) {
    CvarFlags flags = CvarFlags::Archive | CvarFlags::UserInfo | CvarFlags::ReadOnly;
    CvarFlags filtered = flags & CvarFlags::ReadOnly;
    EXPECT_TRUE(hasFlag(filtered, CvarFlags::ReadOnly));
    EXPECT_FALSE(hasFlag(filtered, CvarFlags::Archive));
}

TEST(CvarFlagsTest, BitwiseNot) {
    CvarFlags flags = CvarFlags::Archive | CvarFlags::UserInfo;
    flags &= ~CvarFlags::Archive;
    EXPECT_FALSE(hasFlag(flags, CvarFlags::Archive));
    EXPECT_TRUE(hasFlag(flags, CvarFlags::UserInfo));
}

TEST(CvarFlagsTest, CompoundAssignment) {
    CvarFlags flags = CvarFlags::None;
    flags |= CvarFlags::Archive;
    EXPECT_TRUE(hasFlag(flags, CvarFlags::Archive));
    
    flags |= CvarFlags::ReadOnly;
    EXPECT_TRUE(hasFlag(flags, CvarFlags::ReadOnly));
    
    flags &= ~CvarFlags::Archive;
    EXPECT_FALSE(hasFlag(flags, CvarFlags::Archive));
    EXPECT_TRUE(hasFlag(flags, CvarFlags::ReadOnly));
}

//=============================================================================
// Cvar Basic Tests
//=============================================================================

TEST(CvarTest, Construction) {
    Cvar cvar("test_cvar", "42");
    
    EXPECT_EQ(cvar.name(), "test_cvar");
    EXPECT_EQ(cvar.string(), "42");
    EXPECT_EQ(cvar.defaultValue(), "42");
    EXPECT_FLOAT_EQ(cvar.value(), 42.0f);
    EXPECT_EQ(cvar.integer(), 42);
    EXPECT_TRUE(cvar.boolean());
}

TEST(CvarTest, ConstructionWithFlags) {
    Cvar cvar("test_cvar", "1.5", CvarFlags::Archive | CvarFlags::UserInfo);
    
    EXPECT_TRUE(cvar.hasFlag(CvarFlags::Archive));
    EXPECT_TRUE(cvar.hasFlag(CvarFlags::UserInfo));
    EXPECT_FALSE(cvar.hasFlag(CvarFlags::ReadOnly));
}

TEST(CvarTest, StringValue) {
    Cvar cvar("name", "hello world");
    
    EXPECT_EQ(cvar.string(), "hello world");
    EXPECT_FLOAT_EQ(cvar.value(), 0.0f);
    EXPECT_EQ(cvar.integer(), 0);
    EXPECT_FALSE(cvar.boolean());
}

TEST(CvarTest, FloatValue) {
    Cvar cvar("sensitivity", "3.5");
    
    EXPECT_EQ(cvar.string(), "3.5");
    EXPECT_FLOAT_EQ(cvar.value(), 3.5f);
    EXPECT_EQ(cvar.integer(), 3);
}

TEST(CvarTest, NegativeValue) {
    Cvar cvar("offset", "-10.5");
    
    EXPECT_FLOAT_EQ(cvar.value(), -10.5f);
    EXPECT_EQ(cvar.integer(), -10);
}

TEST(CvarTest, BooleanFalse) {
    Cvar cvar("disabled", "0");
    
    EXPECT_FALSE(cvar.boolean());
    EXPECT_EQ(cvar.integer(), 0);
}

//=============================================================================
// Cvar Set Tests
//=============================================================================

TEST(CvarTest, SetString) {
    Cvar cvar("test", "initial");
    
    bool result = cvar.set("new value");
    
    EXPECT_TRUE(result);
    EXPECT_EQ(cvar.string(), "new value");
    EXPECT_TRUE(cvar.isModified());
}

TEST(CvarTest, SetValue) {
    Cvar cvar("test", "0");
    
    bool result = cvar.setValue(3.14159f);
    
    EXPECT_TRUE(result);
    EXPECT_NEAR(cvar.value(), 3.14159f, 0.0001f);
}

TEST(CvarTest, SetInteger) {
    Cvar cvar("test", "0");
    
    bool result = cvar.setInteger(42);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(cvar.integer(), 42);
    EXPECT_EQ(cvar.string(), "42");
}

TEST(CvarTest, SetBoolean) {
    Cvar cvar("test", "0");
    
    cvar.setBoolean(true);
    EXPECT_TRUE(cvar.boolean());
    EXPECT_EQ(cvar.string(), "1");
    
    cvar.setBoolean(false);
    EXPECT_FALSE(cvar.boolean());
    EXPECT_EQ(cvar.string(), "0");
}

TEST(CvarTest, SetReadOnlyFails) {
    Cvar cvar("test", "initial", CvarFlags::ReadOnly);
    
    bool result = cvar.set("new value");
    
    EXPECT_FALSE(result);
    EXPECT_EQ(cvar.string(), "initial");
}

TEST(CvarTest, ForceSetReadOnly) {
    Cvar cvar("test", "initial", CvarFlags::ReadOnly);
    
    cvar.forceSet("forced value");
    
    EXPECT_EQ(cvar.string(), "forced value");
}

TEST(CvarTest, Reset) {
    Cvar cvar("test", "default");
    cvar.set("changed");
    
    cvar.reset();
    
    EXPECT_EQ(cvar.string(), "default");
}

TEST(CvarTest, ResetNoResetFlag) {
    Cvar cvar("test", "default", CvarFlags::NoReset);
    cvar.set("changed");
    
    cvar.reset();
    
    EXPECT_EQ(cvar.string(), "changed"); // Should not reset
}

TEST(CvarTest, ClearModified) {
    Cvar cvar("test", "initial");
    cvar.set("new");
    EXPECT_TRUE(cvar.isModified());
    
    cvar.clearModified();
    EXPECT_FALSE(cvar.isModified());
}

//=============================================================================
// Cvar Callback Tests
//=============================================================================

TEST(CvarTest, CallbackOnSet) {
    bool callbackCalled = false;
    String receivedValue;
    
    Cvar cvar("test", "initial", CvarFlags::None, 
        [&](Cvar& var, StringView newValue, bool& cancel) {
            callbackCalled = true;
            receivedValue = String(newValue);
        });
    
    cvar.set("new value");
    
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(receivedValue, "new value");
}

TEST(CvarTest, CallbackCancel) {
    Cvar cvar("test", "initial", CvarFlags::None,
        [](Cvar& var, StringView newValue, bool& cancel) {
            // Reject any value that starts with "bad"
            if (newValue.substr(0, 3) == "bad") {
                cancel = true;
            }
        });
    
    bool result = cvar.set("good value");
    EXPECT_TRUE(result);
    EXPECT_EQ(cvar.string(), "good value");
    
    result = cvar.set("bad value");
    EXPECT_FALSE(result);
    EXPECT_EQ(cvar.string(), "good value"); // Unchanged
}

TEST(CvarTest, SetIgnoreCallback) {
    bool callbackCalled = false;
    
    Cvar cvar("test", "initial", CvarFlags::None,
        [&](Cvar&, StringView, bool&) {
            callbackCalled = true;
        });
    
    cvar.setIgnoreCallback("new value");
    
    EXPECT_FALSE(callbackCalled);
    EXPECT_EQ(cvar.string(), "new value");
}

TEST(CvarTest, TriggerCallback) {
    int callCount = 0;
    
    Cvar cvar("test", "initial", CvarFlags::None,
        [&](Cvar&, StringView, bool&) {
            callCount++;
        });
    
    cvar.triggerCallback();
    
    EXPECT_EQ(callCount, 1);
}

//=============================================================================
// Cvar Ruleset Tests
//=============================================================================

TEST(CvarTest, RulesetMin) {
    Cvar cvar("test", "50");
    cvar.setRulesetMin(10.0f);
    
    // Value above min - allowed
    EXPECT_TRUE(cvar.setValue(20.0f));
    EXPECT_FLOAT_EQ(cvar.value(), 20.0f);
    
    // Value below min - rejected
    EXPECT_FALSE(cvar.setValue(5.0f));
    EXPECT_FLOAT_EQ(cvar.value(), 20.0f); // Unchanged
}

TEST(CvarTest, RulesetMax) {
    Cvar cvar("test", "50");
    cvar.setRulesetMax(100.0f);
    
    // Value below max - allowed
    EXPECT_TRUE(cvar.setValue(80.0f));
    EXPECT_FLOAT_EQ(cvar.value(), 80.0f);
    
    // Value above max - rejected
    EXPECT_FALSE(cvar.setValue(150.0f));
    EXPECT_FLOAT_EQ(cvar.value(), 80.0f); // Unchanged
}

TEST(CvarTest, RulesetLock) {
    Cvar cvar("test", "50");
    cvar.setRulesetLock(50.0f);
    
    // Only exact value allowed
    EXPECT_FALSE(cvar.setValue(49.0f));
    EXPECT_FALSE(cvar.setValue(51.0f));
    EXPECT_TRUE(cvar.setValue(50.0f));
}

//=============================================================================
// Cvar Latched Tests
//=============================================================================

TEST(CvarTest, LatchedSet) {
    Cvar cvar("test", "initial", CvarFlags::LatchGfx);
    
    cvar.set("new value");
    
    // Value should be latched, not immediately applied
    EXPECT_EQ(cvar.string(), "initial");
    EXPECT_TRUE(cvar.latchedString().has_value());
    EXPECT_EQ(cvar.latchedString().value(), "new value");
    EXPECT_TRUE(cvar.isModified());
}

TEST(CvarTest, ApplyLatched) {
    Cvar cvar("test", "initial", CvarFlags::LatchGfx);
    
    cvar.set("new value");
    EXPECT_EQ(cvar.string(), "initial");
    
    cvar.applyLatched();
    
    EXPECT_EQ(cvar.string(), "new value");
    EXPECT_FALSE(cvar.latchedString().has_value());
}

//=============================================================================
// Cvar Auto Value Tests
//=============================================================================

TEST(CvarTest, AutoSet) {
    Cvar cvar("test", "default");
    
    cvar.autoSet("auto value");
    
    EXPECT_TRUE(cvar.autoString().has_value());
    EXPECT_EQ(cvar.autoString().value(), "auto value");
    EXPECT_EQ(cvar.string(), "default"); // Main value unchanged
}

TEST(CvarTest, AutoReset) {
    Cvar cvar("test", "default");
    cvar.autoSet("auto value");
    
    cvar.autoReset();
    
    EXPECT_FALSE(cvar.autoString().has_value());
}

//=============================================================================
// Cvar Flag Manipulation Tests
//=============================================================================

TEST(CvarTest, AddFlags) {
    Cvar cvar("test", "value");
    EXPECT_FALSE(cvar.hasFlag(CvarFlags::Archive));
    
    cvar.addFlags(CvarFlags::Archive);
    EXPECT_TRUE(cvar.hasFlag(CvarFlags::Archive));
}

TEST(CvarTest, RemoveFlags) {
    Cvar cvar("test", "value", CvarFlags::Archive | CvarFlags::UserInfo);
    
    cvar.removeFlags(CvarFlags::Archive);
    
    EXPECT_FALSE(cvar.hasFlag(CvarFlags::Archive));
    EXPECT_TRUE(cvar.hasFlag(CvarFlags::UserInfo));
}

//=============================================================================
// Cvar Conversion Operators
//=============================================================================

TEST(CvarTest, ConversionOperators) {
    Cvar cvar("test", "42");
    
    Float f = static_cast<Float>(cvar);
    EXPECT_FLOAT_EQ(f, 42.0f);
    
    Int32 i = static_cast<Int32>(cvar);
    EXPECT_EQ(i, 42);
    
    Bool b = static_cast<Bool>(cvar);
    EXPECT_TRUE(b);
}

//=============================================================================
// CvarRegistry Tests (Dependency Injection - No Singletons!)
//=============================================================================

class CvarRegistryTest : public ::testing::Test {
protected:
    // Each test gets its own registry - no global state!
    CvarRegistry registry_;
};

TEST_F(CvarRegistryTest, CreateAndFind) {
    auto& cvar = registry_.create("test_cvar", "42");
    
    auto* found = registry_.find("test_cvar");
    
    EXPECT_NE(found, nullptr);
    EXPECT_EQ(&cvar, found);
    EXPECT_EQ(found->string(), "42");
}

TEST_F(CvarRegistryTest, FindNotExists) {
    auto* found = registry_.find("nonexistent");
    EXPECT_EQ(found, nullptr);
}

TEST_F(CvarRegistryTest, Exists) {
    registry_.create("test_cvar", "42");
    
    EXPECT_TRUE(registry_.exists("test_cvar"));
    EXPECT_FALSE(registry_.exists("nonexistent"));
}

TEST_F(CvarRegistryTest, Value) {
    registry_.create("test_float", "3.14");
    
    EXPECT_NEAR(registry_.value("test_float"), 3.14f, 0.01f);
    EXPECT_FLOAT_EQ(registry_.value("nonexistent"), 0.0f);
}

TEST_F(CvarRegistryTest, String) {
    registry_.create("test_string", "hello");
    
    EXPECT_EQ(registry_.string("test_string"), "hello");
    EXPECT_EQ(registry_.string("nonexistent"), "");
}

TEST_F(CvarRegistryTest, Count) {
    EXPECT_EQ(registry_.count(), 0u);
    
    registry_.create("cvar1", "1");
    EXPECT_EQ(registry_.count(), 1u);
    
    registry_.create("cvar2", "2");
    EXPECT_EQ(registry_.count(), 2u);
}

TEST_F(CvarRegistryTest, Remove) {
    registry_.create("user_cvar", "value", CvarFlags::UserCreated);
    EXPECT_TRUE(registry_.exists("user_cvar"));
    
    bool removed = registry_.remove("user_cvar");
    
    EXPECT_TRUE(removed);
    EXPECT_FALSE(registry_.exists("user_cvar"));
}

TEST_F(CvarRegistryTest, RemoveNonUserCreatedFails) {
    registry_.create("system_cvar", "value");
    
    bool removed = registry_.remove("system_cvar");
    
    EXPECT_FALSE(removed);
    EXPECT_TRUE(registry_.exists("system_cvar"));
}

TEST_F(CvarRegistryTest, Complete) {
    registry_.create("sensitivity", "3");
    registry_.create("serverinfo", "");
    registry_.create("other", "");
    
    auto completions = registry_.complete("se");
    
    EXPECT_EQ(completions.size(), 2u);
    EXPECT_TRUE(std::find(completions.begin(), completions.end(), "sensitivity") != completions.end());
    EXPECT_TRUE(std::find(completions.begin(), completions.end(), "serverinfo") != completions.end());
}

TEST_F(CvarRegistryTest, FindMatching) {
    registry_.create("cl_sensitivity", "3");
    registry_.create("cl_speed", "400");
    registry_.create("sv_maxclients", "8");
    
    auto matching = registry_.findMatching("cl_*");
    
    EXPECT_EQ(matching.size(), 2u);
}

TEST_F(CvarRegistryTest, FindWithFlags) {
    registry_.create("archived1", "1", CvarFlags::Archive);
    registry_.create("archived2", "2", CvarFlags::Archive);
    registry_.create("readonly", "3", CvarFlags::ReadOnly);
    
    auto archived = registry_.findWithFlags(CvarFlags::Archive);
    
    EXPECT_EQ(archived.size(), 2u);
}

TEST_F(CvarRegistryTest, ForEach) {
    registry_.create("cvar1", "1");
    registry_.create("cvar2", "2");
    registry_.create("cvar3", "3");
    
    int total = 0;
    registry_.forEach([&](Cvar& cvar) {
        total += cvar.integer();
    });
    
    EXPECT_EQ(total, 6); // 1 + 2 + 3
}

TEST_F(CvarRegistryTest, ClearModifiedFlags) {
    auto& cvar1 = registry_.create("cvar1", "1");
    auto& cvar2 = registry_.create("cvar2", "2");
    
    cvar1.set("changed");
    cvar2.set("changed");
    EXPECT_TRUE(cvar1.isModified());
    EXPECT_TRUE(cvar2.isModified());
    
    registry_.clearModifiedFlags();
    
    EXPECT_FALSE(cvar1.isModified());
    EXPECT_FALSE(cvar2.isModified());
}

TEST_F(CvarRegistryTest, AnyModified) {
    auto& cvar1 = registry_.create("cvar1", "1");
    registry_.create("cvar2", "2");
    
    cvar1.clearModified();
    EXPECT_FALSE(registry_.anyModified());
    
    cvar1.set("changed");
    EXPECT_TRUE(registry_.anyModified());
}

TEST_F(CvarRegistryTest, ApplyAllLatched) {
    auto& cvar1 = registry_.create("cvar1", "1", CvarFlags::LatchGfx);
    auto& cvar2 = registry_.create("cvar2", "2", CvarFlags::LatchSound);
    
    cvar1.set("new1");
    cvar2.set("new2");
    EXPECT_EQ(cvar1.string(), "1");
    EXPECT_EQ(cvar2.string(), "2");
    
    registry_.applyAllLatched();
    
    EXPECT_EQ(cvar1.string(), "new1");
    EXPECT_EQ(cvar2.string(), "new2");
}

TEST_F(CvarRegistryTest, ResetAll) {
    auto& cvar1 = registry_.create("cvar1", "default1");
    auto& cvar2 = registry_.create("cvar2", "default2");
    
    cvar1.set("changed1");
    cvar2.set("changed2");
    
    registry_.resetAll();
    
    EXPECT_EQ(cvar1.string(), "default1");
    EXPECT_EQ(cvar2.string(), "default2");
}

TEST_F(CvarRegistryTest, Groups) {
    registry_.setCurrentGroup("video");
    registry_.create("vid_width", "1920");
    registry_.create("vid_height", "1080");
    
    registry_.setCurrentGroup("sound");
    registry_.create("s_volume", "0.7");
    
    registry_.resetCurrentGroup();
    
    auto groups = registry_.getGroups();
    EXPECT_EQ(groups.size(), 2u);
    
    auto videoVars = registry_.getCvarsInGroup("video");
    EXPECT_EQ(videoVars.size(), 2u);
}

//=============================================================================
// TypedCvar Tests (with dependency injection)
//=============================================================================

TEST_F(CvarRegistryTest, TypedCvarFloat) {
    // Register the cvar in the registry first
    registry_.create("sensitivity", "3.0");
    auto* cvar = registry_.find("sensitivity");
    ASSERT_NE(cvar, nullptr);
    
    EXPECT_NEAR(cvar->value(), 3.0f, 0.001f);
    
    cvar->setValue(5.0f);
    EXPECT_NEAR(cvar->value(), 5.0f, 0.001f);
}

TEST_F(CvarRegistryTest, TypedCvarInt) {
    registry_.create("maxclients", "8");
    auto* cvar = registry_.find("maxclients");
    ASSERT_NE(cvar, nullptr);
    
    EXPECT_EQ(cvar->integer(), 8);
    
    cvar->setInteger(16);
    EXPECT_EQ(cvar->integer(), 16);
}

TEST_F(CvarRegistryTest, TypedCvarBool) {
    registry_.create("fullscreen", "0");
    auto* cvar = registry_.find("fullscreen");
    ASSERT_NE(cvar, nullptr);
    
    EXPECT_FALSE(cvar->boolean());
    
    cvar->setBoolean(true);
    EXPECT_TRUE(cvar->boolean());
}

TEST_F(CvarRegistryTest, TypedCvarString) {
    registry_.create("name", "player");
    auto* cvar = registry_.find("name");
    ASSERT_NE(cvar, nullptr);
    
    EXPECT_EQ(cvar->string(), "player");
    
    cvar->set("newname");
    EXPECT_EQ(cvar->string(), "newname");
}
