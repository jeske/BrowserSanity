/**
 * @file safe_strings.h
 * @brief Safe string handling constants and patterns for Browser Sanity
 * 
 * This file implements Pattern 1 from SafeStringHandling.md:
 * Named Size Constants with consistent naming pattern.
 * 
 * NAMING CONVENTION:
 * - Use existing constants like MAX_PATH where appropriate
 * - Only create new constants for specific purposes not covered by existing ones
 * 
 * USAGE PATTERN:
 * char file_path[MAX_PATH];
 * sprintf_s(file_path, MAX_PATH, "C:\\Program Files\\%s", name);
 * 
 * NEVER write an un-length-checked string into a fixed buffer!
 */

#pragma once

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// PURPOSE-BASED BUFFER SIZES (only where MAX_PATH isn't appropriate)
// =============================================================================

// Registry and system identifiers
#define PROG_ID_SIZE            64              // Program ID strings
#define VERSION_SIZE            32              // Version strings

// User interface text
#define MESSAGE_SIZE            512             // UI messages, status text
#define WINDOW_TITLE_SIZE       256             // Window titles
#define DETAILS_SIZE            1024            // Detailed information text

// URLs and web content
#define URL_SIZE                512             // URLs and web addresses
#define ARGS_SIZE               1024            // Command line arguments

// Command execution
#define COMMAND_LINE_SIZE       4096            // Full command line strings

// General purpose text buffers
#define TEXT_BUFFER_SIZE        1024            // General text operations

// =============================================================================
// SAFE STRING OPERATION MACROS
// =============================================================================

// Safe sprintf with automatic size detection
#define SAFE_SPRINTF(buffer, format, ...) \
    sprintf_s(buffer, sizeof(buffer), format, ##__VA_ARGS__)

// Safe strncpy with automatic size detection and truncation
#define SAFE_STRNCPY(dest, src) \
    strncpy_s(dest, sizeof(dest), src, _TRUNCATE)

// Safe strcat with automatic size detection
#define SAFE_STRCAT(dest, src) \
    strcat_s(dest, sizeof(dest), src)

// Error checking versions
#define SAFE_SPRINTF_CHECK(buffer, format, ...) \
    do { \
        errno_t _result = sprintf_s(buffer, sizeof(buffer), format, ##__VA_ARGS__); \
        if (_result != 0) { \
            DebugLogError("sprintf_s failed for " #buffer); \
        } \
    } while(0)

#define SAFE_STRNCPY_CHECK(dest, src) \
    do { \
        errno_t _result = strncpy_s(dest, sizeof(dest), src, _TRUNCATE); \
        if (_result != 0) { \
            DebugLogError("strncpy_s failed for " #dest); \
        } \
    } while(0)

#ifdef __cplusplus
}
#endif