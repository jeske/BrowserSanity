/**
 * @file main.c
 * @brief Main entry point for the BrowserSanity application - Simplified version
 */

#include "../../include/browser_sanity.h"
#include "../../include/actions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// External functions from ui.c
extern BOOL InitUI(HINSTANCE hInstance, int nCmdShow);

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
            return RunInstallerAction();
            
        case MODE_UNINSTALLER:
            return RunUninstallerAction();
            
        case MODE_WATCHDOG:
            return RunWatchdogAction();
            
        case MODE_NORMAL:
        default:
        {
            // Check system status
            DWORD runningPID = 0;
            BOOL isRunning = IsProcessRunningWithPID(&runningPID);
            BOOL isInstalled = IsComprehensivelyInstalled();
            BOOL isFromInstallDir = IsRunningFromInstallDir();
            
            // If we're the installed version starting normally (not another instance)
            if (isInstalled && isFromInstallDir && !isRunning) {
                // Start watchdog if needed
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
            }
            // If running OR installed (show status dialog)
            else if (isRunning || isInstalled) {
                int result = ShowManualLaunchDialog(NULL, isRunning, isInstalled, runningPID);
                
                switch (result) {
                    case IDYES:
                        // Show settings dialog
                        ShowMainSettingsDialog(NULL);
                        return 0;
                        
                    case IDNO:
                    default:
                        // Exit
                        return 0;
                }
            }
            // Not running and not installed - check if we should offer install
            else {
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
                        return RunInstallerAction();
                    }
                }
                
                // Default: Initialize the UI
                return InitUI(hInstance, nCmdShow) ? 0 : 1;
            }
        }
    }
}