/**
 * @file debug_log.c
 * @brief Debug logging implementation - defines global variables
 */

#include <debug_log.h>

// Global debug level variable definition
// Set to DEBUG_LEVEL_INFO by default for development
DebugLevel g_debugLevel = DEBUG_LEVEL_INFO;