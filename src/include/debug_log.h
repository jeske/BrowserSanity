/**
 * @file debug_log.h
 * @brief Configurable debug logging utility for Browser Sanity
 */

#pragma once

#include <windows.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// Debug levels - simple and clean
typedef enum {
    DEBUG_LEVEL_OFF = 0,   // No logging
    DEBUG_LEVEL_ERROR = 1, // Only errors
    DEBUG_LEVEL_INFO = 2,  // Info and errors (default)
    DEBUG_LEVEL_DRAW = 3   // Drawing/rendering operations (very verbose)
} DebugLevel;

// Global debug level - can be changed at runtime
// Set to DEBUG_LEVEL_INFO by default for development
extern DebugLevel g_debugLevel;

// Debug level names for logging
static const char* DEBUG_LEVEL_NAMES[] = {
    "OFF", "ERROR", "INFO", "DRAW"
};

// Internal logging function with level check
inline void DebugLogWithLevel(DebugLevel level, const char* levelName, const char* format, ...) {
    // Check if this level should be logged
    if (level > g_debugLevel) {
        return;
    }
    
    // Get current timestamp
    SYSTEMTIME st;
    GetLocalTime(&st);
    
    // Open log file for append
    FILE* logFile = fopen("C:\\temp\\browsersanity.log", "a");
    if (logFile) {
        // Write timestamp and level
        fprintf(logFile, "[%02d:%02d:%02d.%03d] [%s] ", 
                st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, levelName);
        
        // Write formatted message
        va_list args;
        va_start(args, format);
        vfprintf(logFile, format, args);
        va_end(args);
        
        fprintf(logFile, "\n");
        fclose(logFile);
    }
}

// Convenience macros for different log levels
#define DebugLogError(format, ...)   DebugLogWithLevel(DEBUG_LEVEL_ERROR, "ERROR", format, ##__VA_ARGS__)
#define DebugLogInfo(format, ...)    DebugLogWithLevel(DEBUG_LEVEL_INFO, "INFO", format, ##__VA_ARGS__)
#define DebugLogDraw(format, ...)    DebugLogWithLevel(DEBUG_LEVEL_DRAW, "DRAW", format, ##__VA_ARGS__)

// Backward compatibility - maps to INFO level
#define DebugLog(format, ...)        DebugLogInfo(format, ##__VA_ARGS__)

// Set debug level at runtime
inline void SetDebugLevel(DebugLevel level) {
    g_debugLevel = level;
    if (level > DEBUG_LEVEL_OFF) {
        DebugLogInfo("Debug level set to %s (%d)",
                     (level < 4) ? DEBUG_LEVEL_NAMES[level] : "UNKNOWN", level);
    }
}

// Get current debug level
inline DebugLevel GetDebugLevel() {
    return g_debugLevel;
}

// Initialize logging (clear the log file and set initial level)
inline void InitializeLogging() {
    // Create temp directory if it doesn't exist
    CreateDirectoryA("C:\\temp", NULL);
    
    // Clear the log file
    FILE* logFile = fopen("C:\\temp\\browsersanity.log", "w");
    if (logFile) {
        fprintf(logFile, "=== Browser Sanity Debug Log Started ===\n");
        fclose(logFile);
    }
    
    // Log the initial debug level
    DebugLogInfo("Logging initialized with debug level: %s (%d)", 
                 DEBUG_LEVEL_NAMES[g_debugLevel], g_debugLevel);
}

// Check if a debug level is enabled (useful for expensive debug operations)
inline int IsDebugLevelEnabled(DebugLevel level) {
    return (level <= g_debugLevel);
}

#ifdef __cplusplus
}
#endif