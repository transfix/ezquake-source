/**
 * @file shader_test.cpp
 * @brief Tests for shader and program types
 */

#include <gtest/gtest.h>
#include "../../core/shader/shader_types.hpp"

using namespace ezquake::shader;

// =============================================================================
// Limits Tests
// =============================================================================

TEST(ShaderLimitsTest, Constants) {
    EXPECT_GE(limits::MAX_PROGRAMS, 32);
    EXPECT_GE(limits::MAX_UNIFORMS, 128);
    EXPECT_GE(limits::MAX_ATTRIBUTES, 16);
    EXPECT_GE(limits::MAX_SUB_PROGRAMS, 4);
    EXPECT_GE(limits::MAX_UNIFORM_NAME_LEN, 64);
}

// =============================================================================
// ShaderType Tests
// =============================================================================

TEST(ShaderTypeTest, EnumValues) {
    EXPECT_EQ(static_cast<int>(ShaderType::Vertex), 0);
    EXPECT_EQ(static_cast<int>(ShaderType::Fragment), 1);
    EXPECT_EQ(static_cast<int>(ShaderType::Geometry), 2);
    EXPECT_EQ(static_cast<int>(ShaderType::Compute), 3);
}

TEST(ShaderTypeTest, Names) {
    EXPECT_STREQ(getShaderTypeName(ShaderType::Vertex), "vertex");
    EXPECT_STREQ(getShaderTypeName(ShaderType::Fragment), "fragment");
    EXPECT_STREQ(getShaderTypeName(ShaderType::Geometry), "geometry");
    EXPECT_STREQ(getShaderTypeName(ShaderType::Compute), "compute");
    EXPECT_STREQ(getShaderTypeName(ShaderType::TessControl), "tess_control");
    EXPECT_STREQ(getShaderTypeName(ShaderType::TessEval), "tess_eval");
}

TEST(ShaderTypeTest, Extensions) {
    EXPECT_STREQ(getShaderExtension(ShaderType::Vertex), ".vert");
    EXPECT_STREQ(getShaderExtension(ShaderType::Fragment), ".frag");
    EXPECT_STREQ(getShaderExtension(ShaderType::Geometry), ".geom");
    EXPECT_STREQ(getShaderExtension(ShaderType::Compute), ".comp");
}

// =============================================================================
// ProgramId Tests
// =============================================================================

TEST(ProgramIdTest, EnumValues) {
    EXPECT_EQ(static_cast<int>(ProgramId::None), 0);
    EXPECT_EQ(static_cast<int>(ProgramId::AliasModel), 1);
    EXPECT_GT(static_cast<int>(ProgramId::Count), 20);
}

TEST(ProgramIdTest, Names) {
    EXPECT_STREQ(getProgramName(ProgramId::None), "none");
    EXPECT_STREQ(getProgramName(ProgramId::AliasModel), "aliasmodel");
    EXPECT_STREQ(getProgramName(ProgramId::BrushModel), "brushmodel");
    EXPECT_STREQ(getProgramName(ProgramId::Sprite3D), "sprite3d");
    EXPECT_STREQ(getProgramName(ProgramId::HudImages), "hud_images");
    EXPECT_STREQ(getProgramName(ProgramId::PostProcess), "post_process");
}

TEST(ProgramIdTest, IsGlcProgram) {
    EXPECT_FALSE(isGlcProgram(ProgramId::None));
    EXPECT_FALSE(isGlcProgram(ProgramId::AliasModel));
    EXPECT_FALSE(isGlcProgram(ProgramId::BrushModel));
    
    EXPECT_TRUE(isGlcProgram(ProgramId::PostProcessGlc));
    EXPECT_TRUE(isGlcProgram(ProgramId::SkyGlc));
    EXPECT_TRUE(isGlcProgram(ProgramId::TurbGlc));
    EXPECT_TRUE(isGlcProgram(ProgramId::AliasModelStdGlc));
    EXPECT_TRUE(isGlcProgram(ProgramId::WorldTexturedGlc));
}

TEST(ProgramIdTest, IsComputeProgram) {
    EXPECT_FALSE(isComputeProgram(ProgramId::None));
    EXPECT_FALSE(isComputeProgram(ProgramId::AliasModel));
    EXPECT_FALSE(isComputeProgram(ProgramId::PostProcessGlc));
    
    EXPECT_TRUE(isComputeProgram(ProgramId::LightmapCompute));
}

// =============================================================================
// UniformId Tests
// =============================================================================

TEST(UniformIdTest, EnumValues) {
    EXPECT_EQ(static_cast<int>(UniformId::AliasModelDrawMode), 0);
    EXPECT_GT(static_cast<int>(UniformId::Count), 50);
}

TEST(UniformIdTest, Names) {
    EXPECT_STREQ(getUniformName(UniformId::AliasModelDrawMode), "u_drawMode");
    EXPECT_STREQ(getUniformName(UniformId::BrushModelSampler), "u_sampler");
    EXPECT_STREQ(getUniformName(UniformId::SimpleColor), "u_color");
}

// =============================================================================
// AttributeId Tests
// =============================================================================

TEST(AttributeIdTest, EnumValues) {
    EXPECT_EQ(static_cast<int>(AttributeId::AliasModelStdGlcFlags), 0);
    EXPECT_EQ(static_cast<int>(AttributeId::Count), 7);
}

TEST(AttributeIdTest, Names) {
    EXPECT_STREQ(getAttributeName(AttributeId::AliasModelStdGlcFlags), "a_flags");
    EXPECT_STREQ(getAttributeName(AttributeId::WorldDrawflatStyle), "a_style");
    EXPECT_STREQ(getAttributeName(AttributeId::WorldTexturedDetailCoord), "a_detailCoord");
}

// =============================================================================
// UniformType Tests
// =============================================================================

TEST(UniformTypeTest, ElementCounts) {
    EXPECT_EQ(getUniformElementCount(UniformType::Int1), 1);
    EXPECT_EQ(getUniformElementCount(UniformType::Float1), 1);
    EXPECT_EQ(getUniformElementCount(UniformType::Float2), 2);
    EXPECT_EQ(getUniformElementCount(UniformType::Float3), 3);
    EXPECT_EQ(getUniformElementCount(UniformType::Float4), 4);
    EXPECT_EQ(getUniformElementCount(UniformType::Mat3), 9);
    EXPECT_EQ(getUniformElementCount(UniformType::Mat4), 16);
    EXPECT_EQ(getUniformElementCount(UniformType::Sampler2D), 1);
}

// =============================================================================
// UniformValue Tests
// =============================================================================

TEST(UniformValueTest, DefaultConstruction) {
    UniformValue uv;
    EXPECT_EQ(uv.type, UniformType::Float1);
}

TEST(UniformValueTest, FromInt) {
    auto uv = UniformValue::fromInt(42);
    EXPECT_EQ(uv.type, UniformType::Int1);
    EXPECT_EQ(uv.asInt(), 42);
}

TEST(UniformValueTest, FromFloat) {
    auto uv = UniformValue::fromFloat(3.14f);
    EXPECT_EQ(uv.type, UniformType::Float1);
    EXPECT_FLOAT_EQ(uv.asFloat(), 3.14f);
}

TEST(UniformValueTest, FromVec2) {
    auto uv = UniformValue::fromVec2(1.0f, 2.0f);
    EXPECT_EQ(uv.type, UniformType::Float2);
    EXPECT_FLOAT_EQ(uv.data.f[0], 1.0f);
    EXPECT_FLOAT_EQ(uv.data.f[1], 2.0f);
}

TEST(UniformValueTest, FromVec3) {
    auto uv = UniformValue::fromVec3(1.0f, 2.0f, 3.0f);
    EXPECT_EQ(uv.type, UniformType::Float3);
    EXPECT_FLOAT_EQ(uv.data.f[0], 1.0f);
    EXPECT_FLOAT_EQ(uv.data.f[1], 2.0f);
    EXPECT_FLOAT_EQ(uv.data.f[2], 3.0f);
}

TEST(UniformValueTest, FromVec4) {
    auto uv = UniformValue::fromVec4(1.0f, 2.0f, 3.0f, 4.0f);
    EXPECT_EQ(uv.type, UniformType::Float4);
    EXPECT_FLOAT_EQ(uv.data.f[0], 1.0f);
    EXPECT_FLOAT_EQ(uv.data.f[1], 2.0f);
    EXPECT_FLOAT_EQ(uv.data.f[2], 3.0f);
    EXPECT_FLOAT_EQ(uv.data.f[3], 4.0f);
}

// =============================================================================
// ShaderSource Tests
// =============================================================================

TEST(ShaderSourceTest, DefaultConstruction) {
    ShaderSource src;
    EXPECT_EQ(src.type, ShaderType::Vertex);
    EXPECT_TRUE(src.filename.empty());
    EXPECT_TRUE(src.source.empty());
    EXPECT_FALSE(src.isEmbedded);
}

TEST(ShaderSourceTest, IsValid) {
    ShaderSource src;
    EXPECT_FALSE(src.isValid());
    
    src.filename = "test.vert";
    EXPECT_TRUE(src.isValid());
    
    ShaderSource src2;
    src2.source = "void main() {}";
    EXPECT_TRUE(src2.isValid());
}

TEST(ShaderSourceTest, HasSource) {
    ShaderSource src;
    EXPECT_FALSE(src.hasSource());
    
    src.source = "void main() {}";
    EXPECT_TRUE(src.hasSource());
}

// =============================================================================
// ProgramDefinition Tests
// =============================================================================

TEST(ProgramDefinitionTest, HasShader) {
    ProgramDefinition def;
    def.id = ProgramId::AliasModel;
    def.name = "aliasmodel";
    
    EXPECT_FALSE(def.hasShader(ShaderType::Vertex));
    EXPECT_FALSE(def.hasShader(ShaderType::Fragment));
    
    def.shaders[static_cast<size_t>(ShaderType::Vertex)].filename = "alias.vert";
    EXPECT_TRUE(def.hasShader(ShaderType::Vertex));
}

// =============================================================================
// ProgramState Tests
// =============================================================================

TEST(ProgramStateTest, DefaultConstruction) {
    ProgramState state;
    EXPECT_EQ(state.id, ProgramId::None);
    EXPECT_EQ(state.glHandle, 0u);
    EXPECT_EQ(state.options, 0u);
    EXPECT_FALSE(state.isCompiled);
    EXPECT_FALSE(state.needsRecompile);
}

TEST(ProgramStateTest, IsReady) {
    ProgramState state;
    EXPECT_FALSE(state.isReady());
    
    state.isCompiled = true;
    EXPECT_FALSE(state.isReady());  // Still no handle
    
    state.glHandle = 1;
    EXPECT_TRUE(state.isReady());
}

TEST(ProgramStateTest, UniformLocations) {
    ProgramState state;
    
    // All uniforms should be unset (-1)
    EXPECT_FALSE(state.hasUniform(UniformId::AliasModelDrawMode));
    EXPECT_EQ(state.getUniformLocation(UniformId::AliasModelDrawMode), -1);
    
    // Set a uniform location
    state.uniformLocations[static_cast<size_t>(UniformId::AliasModelDrawMode)] = 5;
    EXPECT_TRUE(state.hasUniform(UniformId::AliasModelDrawMode));
    EXPECT_EQ(state.getUniformLocation(UniformId::AliasModelDrawMode), 5);
}

TEST(ProgramStateTest, AttributeLocations) {
    ProgramState state;
    
    EXPECT_EQ(state.getAttributeLocation(AttributeId::WorldDrawflatStyle), -1);
    
    state.attributeLocations[static_cast<size_t>(AttributeId::WorldDrawflatStyle)] = 3;
    EXPECT_EQ(state.getAttributeLocation(AttributeId::WorldDrawflatStyle), 3);
}

TEST(ProgramStateTest, Invalidate) {
    ProgramState state;
    state.id = ProgramId::AliasModel;
    state.glHandle = 1;
    state.isCompiled = true;
    state.uniformLocations[0] = 5;
    
    state.invalidate();
    
    EXPECT_EQ(state.glHandle, 0u);
    EXPECT_FALSE(state.isCompiled);
    EXPECT_TRUE(state.needsRecompile);
    EXPECT_EQ(state.uniformLocations[0], -1);
}

// =============================================================================
// ShaderManager Tests
// =============================================================================

TEST(ShaderManagerTest, DefaultConstruction) {
    ShaderManager manager;
    EXPECT_EQ(manager.currentProgram, ProgramId::None);
    EXPECT_EQ(manager.programSwitchCount, 0u);
    EXPECT_EQ(manager.uniformSetCount, 0u);
}

TEST(ShaderManagerTest, GetProgram) {
    ShaderManager manager;
    
    auto& prog = manager.getProgram(ProgramId::AliasModel);
    prog.id = ProgramId::AliasModel;
    prog.glHandle = 42;
    
    const auto& constProg = manager.getProgram(ProgramId::AliasModel);
    EXPECT_EQ(constProg.glHandle, 42u);
}

TEST(ShaderManagerTest, IsProgramReady) {
    ShaderManager manager;
    EXPECT_FALSE(manager.isProgramReady(ProgramId::AliasModel));
    
    auto& prog = manager.getProgram(ProgramId::AliasModel);
    prog.glHandle = 1;
    prog.isCompiled = true;
    
    EXPECT_TRUE(manager.isProgramReady(ProgramId::AliasModel));
}

TEST(ShaderManagerTest, CurrentProgram) {
    ShaderManager manager;
    EXPECT_FALSE(manager.isCurrentProgram(ProgramId::AliasModel));
    
    manager.currentProgram = ProgramId::AliasModel;
    EXPECT_TRUE(manager.isCurrentProgram(ProgramId::AliasModel));
    EXPECT_FALSE(manager.isCurrentProgram(ProgramId::BrushModel));
}

TEST(ShaderManagerTest, ResetStats) {
    ShaderManager manager;
    manager.programSwitchCount = 100;
    manager.uniformSetCount = 500;
    
    manager.resetStats();
    
    EXPECT_EQ(manager.programSwitchCount, 0u);
    EXPECT_EQ(manager.uniformSetCount, 0u);
}

TEST(ShaderManagerTest, InvalidateAll) {
    ShaderManager manager;
    
    auto& prog = manager.getProgram(ProgramId::AliasModel);
    prog.glHandle = 42;
    prog.isCompiled = true;
    manager.currentProgram = ProgramId::AliasModel;
    
    manager.invalidateAll();
    
    EXPECT_EQ(manager.currentProgram, ProgramId::None);
    EXPECT_FALSE(prog.isCompiled);
    EXPECT_EQ(prog.glHandle, 0u);
}

// =============================================================================
// ComputeDispatchInfo Tests
// =============================================================================

TEST(ComputeDispatchInfoTest, DefaultConstruction) {
    ComputeDispatchInfo info;
    EXPECT_EQ(info.groupsX, 1u);
    EXPECT_EQ(info.groupsY, 1u);
    EXPECT_EQ(info.groupsZ, 1u);
}

TEST(ComputeDispatchInfoTest, TotalGroups) {
    ComputeDispatchInfo info;
    info.groupsX = 4;
    info.groupsY = 8;
    info.groupsZ = 2;
    EXPECT_EQ(info.totalGroups(), 64u);
}

TEST(ComputeDispatchInfoTest, Barriers) {
    ComputeDispatchInfo info;
    EXPECT_FALSE(info.hasBarrier(MemoryBarrierId::ImageAccess));
    
    info.setBarrier(MemoryBarrierId::ImageAccess);
    EXPECT_TRUE(info.hasBarrier(MemoryBarrierId::ImageAccess));
    EXPECT_FALSE(info.hasBarrier(MemoryBarrierId::TextureAccess));
}

TEST(ComputeDispatchInfoTest, CreateDispatch2D) {
    // 1024x768 texture, 8x8 workgroups
    auto info = createDispatch2D(1024, 768, 8, 8);
    EXPECT_EQ(info.groupsX, 128u);
    EXPECT_EQ(info.groupsY, 96u);
    EXPECT_EQ(info.groupsZ, 1u);
    
    // Non-aligned size
    auto info2 = createDispatch2D(100, 100, 8, 8);
    EXPECT_EQ(info2.groupsX, 13u);  // ceil(100/8)
    EXPECT_EQ(info2.groupsY, 13u);
}

TEST(ComputeDispatchInfoTest, CreateDispatch1D) {
    auto info = createDispatch1D(1000, 64);
    EXPECT_EQ(info.groupsX, 16u);  // ceil(1000/64)
    EXPECT_EQ(info.groupsY, 1u);
    EXPECT_EQ(info.groupsZ, 1u);
}

// =============================================================================
// FramebufferId Tests
// =============================================================================

TEST(FramebufferIdTest, EnumValues) {
    EXPECT_EQ(static_cast<int>(FramebufferId::None), 0);
    EXPECT_EQ(static_cast<int>(FramebufferId::Standard), 1);
    EXPECT_GT(static_cast<int>(FramebufferId::Count), 8);
}

TEST(FramebufferIdTest, Names) {
    EXPECT_STREQ(getFramebufferName(FramebufferId::None), "none");
    EXPECT_STREQ(getFramebufferName(FramebufferId::Standard), "std");
    EXPECT_STREQ(getFramebufferName(FramebufferId::StandardMS), "std-ms");
    EXPECT_STREQ(getFramebufferName(FramebufferId::Hud), "hud");
}

TEST(FramebufferIdTest, NeedsDepthBuffer) {
    EXPECT_FALSE(needsDepthBuffer(FramebufferId::None));
    EXPECT_TRUE(needsDepthBuffer(FramebufferId::Standard));
    EXPECT_TRUE(needsDepthBuffer(FramebufferId::StandardMS));
    EXPECT_FALSE(needsDepthBuffer(FramebufferId::Hud));
    EXPECT_FALSE(needsDepthBuffer(FramebufferId::StandardBlit));
}

TEST(FramebufferIdTest, IsMultisampled) {
    EXPECT_FALSE(isMultisampled(FramebufferId::None));
    EXPECT_FALSE(isMultisampled(FramebufferId::Standard));
    EXPECT_TRUE(isMultisampled(FramebufferId::StandardMS));
    EXPECT_FALSE(isMultisampled(FramebufferId::Hud));
    EXPECT_TRUE(isMultisampled(FramebufferId::HudMS));
}

// =============================================================================
// FramebufferTextureId Tests
// =============================================================================

TEST(FramebufferTextureIdTest, EnumValues) {
    EXPECT_EQ(static_cast<int>(FramebufferTextureId::Standard), 0);
    EXPECT_EQ(static_cast<int>(FramebufferTextureId::Depth), 1);
    EXPECT_EQ(static_cast<int>(FramebufferTextureId::Bloom), 2);
    EXPECT_EQ(static_cast<int>(FramebufferTextureId::WorldNormals), 3);
}

TEST(FramebufferTextureIdTest, Names) {
    EXPECT_STREQ(getFramebufferTextureName(FramebufferTextureId::Standard), "std");
    EXPECT_STREQ(getFramebufferTextureName(FramebufferTextureId::Depth), "depth");
    EXPECT_STREQ(getFramebufferTextureName(FramebufferTextureId::Bloom), "bloom");
    EXPECT_STREQ(getFramebufferTextureName(FramebufferTextureId::WorldNormals), "normals");
}

// =============================================================================
// FramebufferState Tests
// =============================================================================

TEST(FramebufferStateTest, DefaultConstruction) {
    FramebufferState state;
    EXPECT_EQ(state.id, FramebufferId::None);
    EXPECT_EQ(state.glHandle, 0u);
    EXPECT_EQ(state.width, 0);
    EXPECT_EQ(state.height, 0);
}

TEST(FramebufferStateTest, IsValid) {
    FramebufferState state;
    EXPECT_FALSE(state.isValid());
    
    state.glHandle = 1;
    EXPECT_FALSE(state.isValid());  // No dimensions
    
    state.width = 1024;
    state.height = 768;
    EXPECT_TRUE(state.isValid());
}

TEST(FramebufferStateTest, IsComplete) {
    FramebufferState state;
    EXPECT_FALSE(state.isComplete());
    
    state.status = 0x8CD5;  // GL_FRAMEBUFFER_COMPLETE
    EXPECT_TRUE(state.isComplete());
}

TEST(FramebufferStateTest, Textures) {
    FramebufferState state;
    EXPECT_FALSE(state.hasTexture(FramebufferTextureId::Standard));
    EXPECT_EQ(state.getTexture(FramebufferTextureId::Standard), 0u);
    
    state.textures[static_cast<size_t>(FramebufferTextureId::Standard)] = 42;
    EXPECT_TRUE(state.hasTexture(FramebufferTextureId::Standard));
    EXPECT_EQ(state.getTexture(FramebufferTextureId::Standard), 42u);
}
