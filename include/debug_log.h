/**
 * @file debug_log.h
 * @brief Simple debug logging utility for Browser Sanity
 */

#pragma once

#include <windows.h>
#include <stdio.h>

// Simple debug logging function
inline void DebugLog(const char* format, ...) {
    // Get current timestamp
    SYSTEMTIME st;
    GetLocalTime(&st);
    
    // Open log file for append
    FILE* logFile = fopen("C:\\temp\\browsersanity.txt", "a");
    if (logFile) {
        // Write timestamp
        fprintf(logFile, "[%02d:%02d:%02d.%03d] ", 
                st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
        
        // Write formatted message
        va_list args;
        va_start(args, format);
        vfprintf(logFile, format, args);
        va_end(args);
        
        fprintf(logFile, "\n");
        fclose(logFile);
    }
}

// Initialize logging (clear the log file)
inline void InitializeLogging() {
    // Create temp directory if it doesn't exist
    CreateDirectoryA("C:\\temp", NULL);
    
    // Clear the log file
    FILE* logFile = fopen("C:\\temp\\browsersanity.txt", "w");
    if (logFile) {
        fprintf(logFile, "=== Browser Sanity Debug Log Started ===\n");
        fclose(logFile);
    }
}