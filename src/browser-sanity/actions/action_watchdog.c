/**
 * @file action_watchdog.c
 * @brief Watchdog action implementation
 */

#include "../../../include/browser_sanity.h"
#include <windows.h>

// External functions from watchdog.c
extern BOOL StartWatchdog();
extern BOOL StopWatchdog();
extern BOOL IsWatchdogRunning();

/**
 * @brief Runs the application in watchdog mode
 * 
 * @return Exit code
 */
int RunWatchdogAction() {
    AppConfig config;
    MSG msg;
    
    // Read the configuration
    ReadAppConfig(&config);
    
    // Check if the watchdog is enabled
    if (!config.watchdogEnabled) {
        return 0;
    }
    
    // Start the watchdog
    if (!StartWatchdog()) {
        return 1;
    }
    
    // Message loop
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    // Stop the watchdog
    StopWatchdog();
    
    return 0;
}