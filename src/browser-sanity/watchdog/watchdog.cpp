/**
 * @file watchdog.cpp
 * @brief Implementation of the watchdog functionality
 */

#include <browser_sanity.h>
#include <process.h>

// Global variables
static BOOL g_watchdogRunning = FALSE;
static HANDLE g_watchdogThread = NULL;

/**
 * @brief Watchdog thread function
 * 
 * @param param Thread parameter (unused)
 * @return Thread exit code
 */
static unsigned __stdcall WatchdogThreadProc(void* param) {
    AppConfig config;
    BOOL redirectorIntact;
    const char* actions[] = {"Repair", "Open Settings"};
    
    while (g_watchdogRunning) {
        // Read the configuration
        ReadAppConfig(&config);
        
        // Check if the redirector is intact
        redirectorIntact = IsRedirectorIntact();
        
        if (!redirectorIntact) {
            // Show a notification
            ShowToastNotification(
                "Browser Sanity Alert",
                "The msedge.exe redirector has been tampered with or removed. "
                "Would you like to repair it?",
                actions,
                2
            );
            
            // TODO: Handle the user's response to the notification
            // For now, just try to repair it automatically
            InstallRedirector();
        }
        
        // Sleep for the configured interval
        Sleep(config.watchdogInterval * 1000);
    }
    
    return 0;
}

/**
 * @brief Starts the watchdog
 * 
 * @return TRUE if successful, FALSE otherwise
 */
BOOL StartWatchdog() {
    AppConfig config;
    
    // Read the configuration
    ReadAppConfig(&config);
    
    // Check if the watchdog is enabled
    if (!config.watchdogEnabled) {
        return FALSE;
    }
    
    // Check if the watchdog is already running
    if (g_watchdogRunning) {
        return TRUE;
    }
    
    // Set the running flag
    g_watchdogRunning = TRUE;
    
    // Create the watchdog thread
    g_watchdogThread = (HANDLE)_beginthreadex(NULL, 0, WatchdogThreadProc, NULL, 0, NULL);
    if (g_watchdogThread == NULL) {
        g_watchdogRunning = FALSE;
        return FALSE;
    }
    
    return TRUE;
}

/**
 * @brief Stops the watchdog
 * 
 * @return TRUE if successful, FALSE otherwise
 */
BOOL StopWatchdog() {
    // Check if the watchdog is running
    if (!g_watchdogRunning) {
        return TRUE;
    }
    
    // Clear the running flag
    g_watchdogRunning = FALSE;
    
    // Wait for the thread to exit
    if (g_watchdogThread != NULL) {
        WaitForSingleObject(g_watchdogThread, INFINITE);
        CloseHandle(g_watchdogThread);
        g_watchdogThread = NULL;
    }
    
    return TRUE;
}

/**
 * @brief Checks if the watchdog is running
 * 
 * @return TRUE if running, FALSE otherwise
 */
BOOL IsWatchdogRunning() {
    return g_watchdogRunning;
}

/**
 * @brief Sets the watchdog enabled state
 * 
 * @param enable TRUE to enable, FALSE to disable
 * @return TRUE if successful, FALSE otherwise
 */
BOOL SetWatchdogEnabled(BOOL enable) {
    AppConfig config;
    
    // Read the configuration
    ReadAppConfig(&config);
    
    // Update the configuration
    config.watchdogEnabled = enable;
    WriteAppConfig(&config);
    
    // Start or stop the watchdog
    if (enable) {
        return StartWatchdog();
    } else {
        return StopWatchdog();
    }
}

/**
 * @brief Sets the watchdog interval
 * 
 * @param intervalSeconds Interval in seconds
 * @return TRUE if successful, FALSE otherwise
 */
BOOL SetWatchdogInterval(DWORD intervalSeconds) {
    AppConfig config;
    
    // Read the configuration
    ReadAppConfig(&config);
    
    // Update the configuration
    config.watchdogInterval = intervalSeconds;
    WriteAppConfig(&config);
    
    return TRUE;
}