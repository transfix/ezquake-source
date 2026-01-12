/**
 * @file connection.hpp
 * @brief Unified header for client connection subsystem
 * 
 * Includes all connection-related types and utilities:
 * - Connection state machine (ConnectionState enum)
 * - Download/upload state tracking
 * - Demo recording/playback state
 * - Server list management
 * 
 * @note This is a convenience header that includes all connection-related
 *       components. For fine-grained control, include specific headers directly.
 */

#pragma once

#include "connection_state.hpp"
#include "server_list.hpp"
