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
            // Check if we're running from the installation directory
            if (IsRunningFromInstallDir()) {
                // Start the watchdog if it's not already running
                AppConfig config;
                ReadAppConfig(&config);
                
                if (config.watchdogEnabled && !IsWatchdogRunning()) {
                    // Launch the watchdog process
                    char exePath[MAX_PATH];
                    
                    // Get the path of the current executable
                    if (GetModuleFileName(NULL, exePath, MAX_PATH) == 0) {
                        return 1;
                    }
                    
                    // Launch the watchdog process
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
            } else {
                // Check if we're being run directly (not from the installer)
                // If so, show the installer UI
                char exePath[MAX_PATH];
                char* fileName;
                
                // Get the path of the current executable
                if (GetModuleFileName(NULL, exePath, MAX_PATH) == 0) {
                    return 1;
                }
                
                // Get the file name
                fileName = strrchr(exePath, '\\');
                if (fileName) {
                    fileName++;
                } else {
                    fileName = exePath;
                }
                
                // Check if we're being run as BrowserSanity.exe
                if (stricmp(fileName, "BrowserSanity.exe") == 0) {
                    // Show a message about installation
                    if (MessageBox(NULL, 
                                  "Browser Sanity is not installed. Would you like to install it now?", 
                                  "Browser Sanity", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                        return RunInstallerMode();
                    }
                }
            }
            
            // Initialize the UI
            return InitUI(hInstance, nCmdShow) ? 0 : 1;
    }
}