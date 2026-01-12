/*
 * ezQuake C++ Port - C Compatibility Bridge
 * 
 * This module provides C-compatible wrappers around the C++ implementation,
 * allowing the existing C code to gradually migrate to C++ while maintaining
 * full compatibility.
 * 
 * Copyright (C) 1996-1997 Id Software, Inc.
 * Copyright (C) 2024 ezQuake Team
 */

#ifndef EZQUAKE_COMPAT_C_BRIDGE_H
#define EZQUAKE_COMPAT_C_BRIDGE_H

#include <stddef.h>  // For size_t

// This header can be included from C code
#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
// Vector Operations (mathlib.h compatibility)
//=============================================================================

/**
 * @brief C-compatible vec3_t operations using C++ implementation
 * 
 * These functions wrap the C++ Vec3 class methods for use from C code.
 */

/**
 * @brief Normalize a vector in place, returns old length
 */
float VectorNormalize_CPP(float* v);

/**
 * @brief Fast normalize using inverse square root approximation
 */
void VectorNormalizeFast_CPP(float* v);

/**
 * @brief Calculate vector length
 */
float VectorLength_CPP(const float* v);

/**
 * @brief Calculate dot product
 */
float DotProduct_CPP(const float* a, const float* b);

/**
 * @brief Calculate cross product
 */
void CrossProduct_CPP(const float* a, const float* b, float* out);

/**
 * @brief Convert angles to forward/right/up vectors
 */
void AngleVectors_CPP(const float* angles, float* forward, float* right, float* up);

/**
 * @brief Rotate point around vector
 */
void RotatePointAroundVector_CPP(float* dst, const float* dir, const float* point, float degrees);

/**
 * @brief Project point onto plane
 */
void ProjectPointOnPlane_CPP(float* dst, const float* p, const float* normal);

/**
 * @brief Get perpendicular vector
 */
void PerpendicularVector_CPP(float* dst, const float* src);

/**
 * @brief Create orthonormal basis from forward vector
 */
void MakeNormalVectors_CPP(const float* forward, float* right, float* up);

/**
 * @brief Box on plane side test
 */
int BoxOnPlaneSide_CPP(const float* emins, const float* emaxs, 
                        const float* normal, float dist, int type, int signbits);

//=============================================================================
// Matrix Operations
//=============================================================================

/**
 * @brief Create rotation matrix from angle and axis
 */
void Matrix3x3_CreateRotate_CPP(float out[3][3], float angle, const float* v);

/**
 * @brief Multiply matrix by vector
 */
void Matrix3x3_MultiplyByVector_CPP(float* out, const float in[3][3], const float* v);

/**
 * @brief Concatenate rotation matrices
 */
void R_ConcatRotations_CPP(const float in1[3][3], const float in2[3][3], float out[3][3]);

//=============================================================================
// String Operations (q_shared.h compatibility)
//=============================================================================

/**
 * @brief Parse integer from string (handles hex, etc.)
 */
int Q_atoi_CPP(const char* str);

/**
 * @brief Parse float from string
 */
float Q_atof_CPP(const char* str);

/**
 * @brief Format float without trailing zeros
 */
const char* Q_ftos_CPP(float value);

/**
 * @brief Case-insensitive string comparison
 */
int Q_strcasecmp_CPP(const char* s1, const char* s2);

/**
 * @brief Case-insensitive substring search
 */
const char* Q_strcasestr_CPP(const char* haystack, const char* needle);

/**
 * @brief Glob pattern matching
 */
int Q_glob_match_CPP(const char* pattern, const char* text);

/**
 * @brief Compute string hash
 */
unsigned int Com_HashKey_CPP(const char* name);

//=============================================================================
// Memory Operations (zone.h compatibility)
//=============================================================================

/**
 * @brief Allocate memory with tracking
 */
void* Q_malloc_CPP(size_t size);

/**
 * @brief Allocate zeroed memory
 */
void* Q_calloc_CPP(size_t n, size_t size);

/**
 * @brief Reallocate memory
 */
void* Q_realloc_CPP(void* p, size_t newsize);

/**
 * @brief Free memory
 */
void Q_free_CPP(void* ptr);

/**
 * @brief Duplicate string
 */
char* Q_strdup_CPP(const char* src);

//=============================================================================
// Console Operations (console.h compatibility)
//=============================================================================

/**
 * @brief Print text to console
 */
void Con_Print_CPP(const char* text);

/**
 * @brief Print formatted text to console
 */
void Con_Printf_CPP(const char* format, ...);

/**
 * @brief Safe print (screen updates may be disabled)
 */
void Con_SafePrintf_CPP(const char* format, ...);

/**
 * @brief Clear console
 */
void Con_Clear_CPP(void);

/**
 * @brief Toggle console visibility
 */
void Con_Toggle_CPP(void);

/**
 * @brief Initialize console
 */
void Con_Init_CPP(void);

/**
 * @brief Shutdown console
 */
void Con_Shutdown_CPP(void);

/**
 * @brief Check for resize (call on video mode change)
 */
void Con_CheckResize_CPP(int width);

/**
 * @brief Clear notify area
 */
void Con_ClearNotify_CPP(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // EZQUAKE_COMPAT_C_BRIDGE_H
