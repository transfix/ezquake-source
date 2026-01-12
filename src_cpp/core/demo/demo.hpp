/**
 * @file demo.hpp
 * @brief Unified header for the demo subsystem
 * 
 * Includes all demo-related headers for convenient access.
 * 
 * @code
 * #include "core/demo/demo.hpp"
 * 
 * ezquake::DemoPlayer player;
 * if (player.start("demos/match.mvd")) {
 *     while (player.isPlaying()) {
 *         auto result = player.readMessage();
 *         // Process message...
 *     }
 * }
 * @endcode
 */

#pragma once

#include "demo_types.hpp"
#include "demo_recorder.hpp"
#include "demo_player.hpp"
