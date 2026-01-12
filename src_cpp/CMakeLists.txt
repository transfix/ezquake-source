# ezQuake C++ Port - Core Library
# C++20 Modern Implementation

cmake_minimum_required(VERSION 3.22)

project(ezquake_cpp VERSION 1.0.0 LANGUAGES CXX)

# Require C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(EZQUAKE_CPP_BUILD_TESTS "Build unit tests" ON)
option(EZQUAKE_CPP_BUILD_BENCHMARKS "Build performance benchmarks" OFF)
option(EZQUAKE_CPP_ASAN "Enable AddressSanitizer" OFF)
option(EZQUAKE_CPP_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wsign-conversion
        -Wcast-align
        -Wunused
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
        -Wno-unused-parameter
    )
    
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        add_compile_options(-Wduplicated-cond -Wduplicated-branches -Wlogical-op)
    endif()
    
    if(EZQUAKE_CPP_ASAN)
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    endif()
    
    if(EZQUAKE_CPP_UBSAN)
        add_compile_options(-fsanitize=undefined)
        add_link_options(-fsanitize=undefined)
    endif()
    
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4 /permissive-)
endif()

# Core math library (foundation of everything)
add_library(ezquake_math STATIC
    core/math/vec3.cpp
    core/math/matrix.cpp
    core/math/mathlib.cpp
)

target_include_directories(ezquake_math PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
)

# Core utilities library
add_library(ezquake_core STATIC
    core/types.cpp
    core/memory/allocator.cpp
    core/memory/arena.cpp
    core/text/string_utils.cpp
    core/text/utf8.cpp
    core/cvar/cvar_registry.cpp
    core/cmd/cmd_registry.cpp
    core/console/console.cpp
    core/context.cpp
    core/fs/virtual_file.cpp
    core/fs/os_file.cpp
    core/fs/memory_file.cpp
    core/fs/filesystem.cpp
    core/net/net_types.cpp
    core/net/socket.cpp
    core/net/netchan.cpp
    core/common/info_string.cpp
    core/common/token_parser.cpp
    core/common/command_line.cpp
    core/demo/demo_recorder.cpp
    core/demo/demo_player.cpp
)

target_include_directories(ezquake_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
)

target_link_libraries(ezquake_core PUBLIC ezquake_math)

# C compatibility layer (for gradual migration)
add_library(ezquake_c_compat STATIC
    compat/c_bridge.cpp
)

target_include_directories(ezquake_c_compat PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src  # Access to original C headers
)

target_link_libraries(ezquake_c_compat PUBLIC ezquake_core)

# Tests
if(EZQUAKE_CPP_BUILD_TESTS)
    enable_testing()
    
    # Fetch GoogleTest
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    
    # Test executable
    add_executable(ezquake_tests
        tests/math/vec3_test.cpp
        tests/math/matrix_test.cpp
        tests/math/mathlib_test.cpp
        tests/core/types_test.cpp
        tests/core/string_utils_test.cpp
        tests/core/utf8_test.cpp
        tests/core/memory_test.cpp
        tests/cvar/cvar_test.cpp
        tests/cmd/cmd_test.cpp
        tests/console/console_test.cpp
        tests/fs/fs_test.cpp
        tests/net/net_test.cpp
        tests/common/common_test.cpp
        tests/client/client_types_test.cpp
        tests/input/input_test.cpp
        tests/prediction/prediction_test.cpp
        tests/protocol/protocol_test.cpp
        tests/connection/connection_test.cpp
        tests/demo/demo_test.cpp
        tests/view/view_test.cpp
        tests/entity/temp_entity_test.cpp
        tests/server/server_test.cpp
        tests/physics/physics_test.cpp
        tests/net/server_net_test.cpp
        tests/render/model_test.cpp
        tests/render/texture_test.cpp
        tests/render/render_state_test.cpp
        tests/render/particle_test.cpp
        tests/render/lightmap_test.cpp
        tests/ui/hud_test.cpp
        tests/sound/sound_test.cpp
        tests/archive/archive_test.cpp
        tests/render/sprite_test.cpp
        tests/vm/vm_test.cpp
        tests/menu/menu_test.cpp
        tests/config/config_test.cpp
        tests/bsp/bsp_test.cpp
        tests/browser/browser_test.cpp
        tests/effects/effects_test.cpp
        tests/game/ruleset_test.cpp
        tests/game/teamplay_test.cpp
        tests/demo/mvd_test.cpp
        tests/video/video_test.cpp
        tests/stats/fragstats_test.cpp
        tests/draw/draw_test.cpp
        tests/trigger/trigger_test.cpp
        tests/logging/logging_test.cpp
        tests/image/image_test.cpp
        tests/hash/hash_test.cpp
        tests/tracker/tracker_test.cpp
        tests/charset/charset_test.cpp
        tests/shader/shader_test.cpp
        tests/system/system_test.cpp
        tests/corona/corona_test.cpp
        tests/test_main.cpp
    )
    
    target_link_libraries(ezquake_tests PRIVATE
        ezquake_c_compat
        ezquake_core
        ezquake_math
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
    )
    
    target_include_directories(ezquake_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    
    include(GoogleTest)
    gtest_discover_tests(ezquake_tests)
endif()

# Installation
install(TARGETS ezquake_math ezquake_core ezquake_c_compat
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(DIRECTORY core/ DESTINATION include/ezquake
    FILES_MATCHING PATTERN "*.hpp"
)
