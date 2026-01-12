/*
 * ezQuake C++ Port - C Compatibility Bridge Implementation
 * 
 * Copyright (C) 1996-1997 Id Software, Inc.
 * Copyright (C) 2024 ezQuake Team
 */

#include "c_bridge.h"
#include "core/math/vec3.hpp"
#include "core/math/matrix.hpp"
#include "core/math/mathlib.hpp"
#include "core/text/string_utils.hpp"
#include "core/memory/allocator.hpp"
#include "core/console/console.hpp"
#include <cstring>
#include <cstdarg>

using namespace ezquake;
using namespace ezquake::math;

//=============================================================================
// Global Console Instance
//
// This is the global Console instance used by legacy C code.
// The Console class itself is NOT a singleton - this instance lives here
// in the C compatibility layer, where global state belongs.
//=============================================================================

namespace {
    // The global console instance for C compatibility
    Console g_console;
}

// Accessor for other modules that need the default console
namespace ezquake {
    Console& getDefaultConsole() {
        return g_console;
    }
}

//=============================================================================
// Vector Operations
//=============================================================================

extern "C" {

float VectorNormalize_CPP(float* v) {
    Vec3 vec(v);
    float len = vec.normalize();
    vec.copyTo(v);
    return len;
}

void VectorNormalizeFast_CPP(float* v) {
    Vec3 vec(v);
    vec.normalizeFast();
    vec.copyTo(v);
}

float VectorLength_CPP(const float* v) {
    return Vec3(v).length();
}

float DotProduct_CPP(const float* a, const float* b) {
    return Vec3(a).dot(Vec3(b));
}

void CrossProduct_CPP(const float* a, const float* b, float* out) {
    Vec3 result = Vec3(a).cross(Vec3(b));
    result.copyTo(out);
}

void AngleVectors_CPP(const float* angles, float* forward, float* right, float* up) {
    Vec3 f, r, u;
    Vec3(angles).toDirections(f, r, u);
    if (forward) f.copyTo(forward);
    if (right) r.copyTo(right);
    if (up) u.copyTo(up);
}

void RotatePointAroundVector_CPP(float* dst, const float* dir, const float* point, float degrees) {
    Vec3 result = rotatePointAroundVector(Vec3(dir), Vec3(point), degrees);
    result.copyTo(dst);
}

void ProjectPointOnPlane_CPP(float* dst, const float* p, const float* normal) {
    Vec3 result = projectPointOnPlane(Vec3(p), Vec3(normal));
    result.copyTo(dst);
}

void PerpendicularVector_CPP(float* dst, const float* src) {
    Vec3 result = perpendicularVector(Vec3(src));
    result.copyTo(dst);
}

void MakeNormalVectors_CPP(const float* forward, float* right, float* up) {
    Vec3 r, u;
    makeNormalVectors(Vec3(forward), r, u);
    r.copyTo(right);
    u.copyTo(up);
}

int BoxOnPlaneSide_CPP(const float* emins, const float* emaxs,
                        const float* normal, float dist, int type, int signbits) {
    Plane plane;
    plane.normal = Vec3(normal);
    plane.dist = dist;
    plane.type = static_cast<Byte>(type);
    plane.signbits = static_cast<Byte>(signbits);
    
    return boxOnPlaneSide(Vec3(emins), Vec3(emaxs), plane);
}

//=============================================================================
// Matrix Operations
//=============================================================================

void Matrix3x3_CreateRotate_CPP(float out[3][3], float angle, const float* v) {
    Matrix3x3 m = Matrix3x3::fromAxisAngle(angle, Vec3(v));
    std::memcpy(out, m.data(), sizeof(float) * 9);
}

void Matrix3x3_MultiplyByVector_CPP(float* out, const float in[3][3], const float* v) {
    Matrix3x3 m(in);
    Vec3 result = m * Vec3(v);
    result.copyTo(out);
}

void R_ConcatRotations_CPP(const float in1[3][3], const float in2[3][3], float out[3][3]) {
    Matrix3x3 result = Matrix3x3(in1) * Matrix3x3(in2);
    std::memcpy(out, result.data(), sizeof(float) * 9);
}

//=============================================================================
// String Operations
//=============================================================================

int Q_atoi_CPP(const char* str) {
    return text::parseInt(str);
}

float Q_atof_CPP(const char* str) {
    return text::parseFloat(str);
}

// Thread-local buffer for Q_ftos return value
static thread_local char g_ftosBuffer[64];

const char* Q_ftos_CPP(float value) {
    String s = text::formatFloat(value);
    std::strncpy(g_ftosBuffer, s.c_str(), sizeof(g_ftosBuffer) - 1);
    g_ftosBuffer[sizeof(g_ftosBuffer) - 1] = '\0';
    return g_ftosBuffer;
}

int Q_strcasecmp_CPP(const char* s1, const char* s2) {
    return text::compareIgnoreCase(s1, s2);
}

const char* Q_strcasestr_CPP(const char* haystack, const char* needle) {
    auto pos = text::findIgnoreCase(haystack, needle);
    if (pos == StringView::npos) return nullptr;
    return haystack + pos;
}

int Q_glob_match_CPP(const char* pattern, const char* text) {
    return text::globMatch(pattern, text) ? 1 : 0;
}

unsigned int Com_HashKey_CPP(const char* name) {
    return text::hash(name);
}

//=============================================================================
// Memory Operations
//=============================================================================

void* Q_malloc_CPP(size_t size) {
    return memory::allocate(size);
}

void* Q_calloc_CPP(size_t n, size_t size) {
    return memory::allocateZeroed(n, size);
}

void* Q_realloc_CPP(void* p, size_t newsize) {
    return memory::reallocate(p, newsize);
}

void Q_free_CPP(void* ptr) {
    memory::deallocate(ptr);
}

char* Q_strdup_CPP(const char* src) {
    return memory::duplicateString(src);
}

//=============================================================================
// Console Operations
//=============================================================================

void Con_Print_CPP(const char* text) {
    if (text) {
        g_console.print(text);
    }
}

void Con_Printf_CPP(const char* format, ...) {
    if (!format) return;
    
    char buffer[4096];
    va_list args;
    va_start(args, format);
    std::vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    g_console.print(buffer);
}

void Con_SafePrintf_CPP(const char* format, ...) {
    if (!format) return;
    
    char buffer[4096];
    va_list args;
    va_start(args, format);
    std::vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    g_console.safePrint(buffer);
}

void Con_Clear_CPP(void) {
    g_console.clear();
}

void Con_Toggle_CPP(void) {
    g_console.toggle();
}

void Con_Init_CPP(void) {
    g_console.init();
}

void Con_Shutdown_CPP(void) {
    g_console.shutdown();
}

void Con_CheckResize_CPP(int width) {
    g_console.checkResize(static_cast<size_t>(width));
}

void Con_ClearNotify_CPP(void) {
    g_console.clearNotify();
}

} // extern "C"

//=============================================================================
// C++ Global Console Functions (namespace ezquake)
//
// These are the namespace-scoped global functions declared in console.hpp
//=============================================================================

namespace ezquake {

void Con_Print(const char* text) {
    if (text) {
        g_console.print(text);
    }
}

void Con_Printf(const char* format, ...) {
    if (!format) return;
    
    char buffer[4096];
    va_list args;
    va_start(args, format);
    std::vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    g_console.print(buffer);
}

void Con_SafePrintf(const char* format, ...) {
    if (!format) return;
    
    char buffer[4096];
    va_list args;
    va_start(args, format);
    std::vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    g_console.safePrint(buffer);
}

void Con_Clear() {
    g_console.clear();
}

void Con_Toggle() {
    g_console.toggle();
}

void Con_Init() {
    g_console.init();
}

void Con_Shutdown() {
    g_console.shutdown();
}

void Con_CheckResize() {
    // Would need video width here
    // g_console.checkResize(vid.width >> 3);
}

void Con_ClearNotify() {
    g_console.clearNotify();
}

} // namespace ezquake
