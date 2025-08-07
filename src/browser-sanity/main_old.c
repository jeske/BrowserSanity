/**
 * @file main.c
 * @brief Main entry point for the BrowserSanity application
 */

#include "browser_sanity.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// External functions from ui.c
extern BOOL InitUI(HINSTANCE hInstance, int nCmdShow);

// External functions from watchdog.c
extern BOOL StartWatchdog();
extern BOOL StopWatchdog();
extern BOOL IsWatchdogRunning();

// Function declarations
static int ShowRunningInstanceDialog();
static int RunInstallerMode();
static int RunUninstallerMode();
static int RunWatchdogMode();

/**
 * @brief Shows an enhanced status dialog when Browser Sanity is running or installed
 *
 * @return Exit code
 */
static int ShowRunningInstanceDialog() {
    AppConfig config;
    char message[1024];
    char title[64] = "Browser Sanity Status";
    DWORD runningPID = 0;
    BOOL isRunning = IsProcessRunningWithPID(&runningPID);
    int result;
    
    // Read current configuration
    ReadAppConfig(&config);
    
    // Build enhanced status message with PID information
    if (isRunning && config.isInstalled) {
        sprintf(message,
            "Browser Sanity is currently running and installed.\n\n"
            "Installation Path: %s\n"
            "Status: Active (PID: %lu)\n"
            "Version: %s\n\n"
            "What would you like to do?",
            config.installPath, runningPID, config.version);
    }
    else if (config.isInstalled && !isRunning) {
        sprintf(message,
            "Browser Sanity is installed but not currently running.\n\n"
            "Installation Path: %s\n"
            "Status: Installed (Not Running)\n"
            "Version: %s\n\n"
            "What would you like to do?",
            config.installPath, config.version);
    }
    else if (isRunning && !config.isInstalled) {
        sprintf(message,
            "Browser Sanity is currently running but not properly installed.\n\n"
            "Status: Running (PID: %lu) - Portable Mode\n"
            "Installation Status: Not Installed\n\n"
            "What would you like to do?",
            runningPID);
    }
    else {
        sprintf(message,
            "Browser Sanity status is unclear.\n\n"
            "Running: %s\n"
            "Installed: %s\n\n"
            "What would you like to do?",
            isRunning ? "Yes" : "No",
            config.isInstalled ? "Yes" : "No");
    }
    
    // Show enhanced options dialog with better button labels
    char dialogText[1280];
    sprintf(dialogText, "%s\n\nChoose an action:\n\n"
                       "YES - Show Settings\n"
                       "NO - Exit\n"
                       "CANCEL - %s",
                       message,
                       isRunning ? "Restart Browser Sanity" : "Start Browser Sanity");
    
    result = MessageBox(NULL, dialogText, title,
                       MB_YESNOCANCEL | MB_ICONQUESTION);
    
    switch (result) {
        case IDYES:
            // Show settings - initialize UI
            return InitUI(GetModuleHandle(NULL), SW_NORMAL) ? 0 : 1;
            
        case IDNO:
            // Exit
            return 0;
            
        case IDCANCEL:
            // Restart/Start functionality
            if (isRunning) {
                MessageBox(NULL, "Restart functionality will be implemented soon.",
                          "Browser Sanity", MB_OK | MB_ICONINFORMATION);
            } else if (config.isInstalled) {
                // Launch the installed version
                LaunchInstalledApplication();
            } else {
                // Offer to install
                if (MessageBox(NULL,
                              "Browser Sanity is not installed. Would you like to install it now?",
                              "Browser Sanity", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                    return RunInstallerMode();
                }
            }
            return 0;
            
        default:
            return 0;
    }
}

/**
 * @brief Parses command line arguments
 * 
 * @param lpCmdLine Command line string
 * @param mode Pointer to store the application mode
 * @return TRUE if successful, FALSE otherwise
 */
static BOOL ParseCommandLine(LPSTR lpCmdLine, AppMode* mode) {
    // Default mode
    *mode = MODE_NORMAL;
    
    // Check for command line arguments
    if (lpCmdLine && *lpCmdLine) {
        if (strstr(lpCmdLine, "/install")) {
            *mode = MODE_INSTALLER;
            return TRUE;
        } else if (strstr(lpCmdLine, "/uninstall")) {
            *mode = MODE_UNINSTALLER;
            return TRUE;
        } else if (strstr(lpCmdLine, "/watchdog")) {
            *mode = MODE_WATCHDOG;
            return TRUE;
        }
    }
    
    return TRUE;
}

/**
 * @brief Runs the application in installer mode
 * 
 * @return Exit code
 */
static int RunInstallerMode() {
    // Check if we need to elevate
    if (!IsUserAnAdmin()) {
        char exePath[MAX_PATH];
        
        // Get the path of the current executable
        if (GetModuleFileName(NULL, exePath, MAX_PATH) == 0) {
            return 1;
        }
        
        // Elevate and run with the same parameters
        SHELLEXECUTEINFO sei;
        ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
        sei.cbSize = sizeof(SHELLEXECUTEINFO);
        sei.lpVerb = "runas";
        sei.lpFile = exePath;
        sei.lpParameters = "/install";
        sei.nShow = SW_NORMAL;
        
        if (!ShellExecuteEx(&sei)) {
            MessageBox(NULL, "Failed to elevate privileges.", "Browser Sanity", MB_OK | MB_ICONERROR);
            return 1;
        }
        
        return 0;
    }
    
    // Install the application
    if (!InstallApplication()) {
        MessageBox(NULL, "Failed to install Browser Sanity.", "Browser Sanity", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    // Show success message
    MessageBox(NULL, "Browser Sanity has been installed successfully.", "Browser Sanity", MB_OK | MB_ICONINFORMATION);
    
    // Launch the installed application
    LaunchInstalledApplication();
    
    return 0;
}

/**
 * @brief Runs the application in uninstaller mode
 * 
 * @return Exit code
 */
static int RunUninstallerMode() {
    // Check if we need to elevate
    if (!IsUserAnAdmin()) {
        char exePath[MAX_PATH];
        
        // Get the path of the current executable
        if (GetModuleFileName(NULL, exePath, MAX_PATH) == 0) {
            return 1;
        }
        
        // Elevate and run with the same parameters
        SHELLEXECUTEINFO sei;
        ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
        sei.cbSize = sizeof(SHELLEXECUTEINFO);
        sei.lpVerb = "runas";
        sei.lpFile = exePath;
        sei.lpParameters = "/uninstall";
        sei.nShow = SW_NORMAL;
        
        if (!ShellExecuteEx(&sei)) {
            MessageBox(NULL, "Failed to elevate privileges.", "Browser Sanity", MB_OK | MB_ICONERROR);
            return 1;
        }
        
        return 0;
    }
    
    // Uninstall the application
    if (!UninstallApplication()) {
        MessageBox(NULL, "Failed to uninstall Browser Sanity.", "Browser Sanity", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    // Show success message
    MessageBox(NULL, "Browser Sanity has been uninstalled successfully.", "Browser Sanity", MB_OK | MB_ICONINFORMATION);
    
    return 0;
}

/**
 * @brief Runs the application in watchdog mode
 * 
 * @return Exit code
 */
static int RunWatchdogMode() {
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

/**
 * @brief Main entry point
 * 
 * @param hInstance Instance handle
 * @param hPrevInstance Previous instance handle (always NULL)
 * @param lpCmdLine Command line string
 * @param nCmdShow Show command
 * @return Exit code
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    AppMode mode;
    
    // Parse command line arguments
    if (!ParseCommandLine(lpCmdLine, &mode)) {
        return 1;
    }
    
    // Run in the appropriate mode
    switch (mode) {
        case MODE_INSTALLER:
            return RunInstallerMode();
            
        case MODE_UNINSTALLER:
            return RunUninstallerMode();
            
        case MODE_WATCHDOG:
            return RunWatchdogMode();
            
        case MODE_NORMAL:
        default:
            // STEP 1: Check if another instance is already running
            BOOL isRunning = IsProcessRunning();
            
            // STEP 2: Check if Browser Sanity is installed
            BOOL isInstalled = IsComprehensivelyInstalled();
            
            // STEP 3: Check if we're running from the install directory
            BOOL isFromInstallDir = IsRunningFromInstallDir();
            
            // STEP 4: Determine action based on running/installed status
            if (isRunning || isInstalled) {
                // If we're the installed version starting normally (not another instance)
                if (isInstalled && isFromInstallDir && !isRunning) {
                    // Start watchdog if needed, then run normally
                    AppConfig config;
                    ReadAppConfig(&config);
                    
                    if (config.watchdogEnabled && !IsWatchdogRunning()) {
                        char exePath[MAX_PATH];
                        
                        if (GetModuleFileName(NULL, exePath, MAX_PATH) != 0) {
                            STARTUPINFO si;
                            PROCESS_INFORMATION pi;
                            char commandLine[MAX_PATH + 32];
                            
                            sprintf(commandLine, "\"%s\" /watchdog", exePath);
                            
                            ZeroMemory(&si, sizeof(si));
                            si.cb = sizeof(si);
                            ZeroMemory(&pi, sizeof(pi));
                            
                            if (CreateProcess(NULL, commandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
                                CloseHandle(pi.hProcess);
                                CloseHandle(pi.hThread);
                            }
                        }
                    }
                    
                    // Initialize the UI for normal operation
                    return InitUI(hInstance, nCmdShow) ? 0 : 1;
                } else {
                    // Show enhanced status dialog - either running OR installed (or both)
                    return ShowRunningInstanceDialog();
                }
            }
            else {
                // Not running and not installed - show install dialog
                char exePath[MAX_PATH];
                char* fileName;
                
                if (GetModuleFileName(NULL, exePath, MAX_PATH) == 0) {
                    return 1;
                }
                
                fileName = strrchr(exePath, '\\');
                if (fileName) {
                    fileName++;
                } else {
                    fileName = exePath;
                }
                
                // Only show install dialog for BrowserSanity.exe
                if (_stricmp(fileName, "BrowserSanity.exe") == 0) {
                    if (MessageBox(NULL,
                                  "Browser Sanity is not installed. Would you like to install it now?",
                                  "Browser Sanity", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                        return RunInstallerMode();
                    }
                }
                
                // Default: Initialize the UI
                return InitUI(hInstance, nCmdShow) ? 0 : 1;
            }
    }
}