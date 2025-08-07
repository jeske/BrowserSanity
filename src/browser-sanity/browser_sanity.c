/**
 * @file browser_sanity.c
 * @brief Implementation of the BrowserSanity core functionality
 */

#include "browser_sanity.h"
#include <shlwapi.h>
#include <shlobj.h>
#include <tlhelp32.h>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")

/**
 * @brief Reads the application configuration from the registry
 * 
 * @param config Pointer to an AppConfig structure to fill
 * @return TRUE if successful, FALSE otherwise
 */
BOOL ReadAppConfig(AppConfig* config) {
    HKEY hKey;
    DWORD dwType, dwSize;
    DWORD dwValue;
    
    // Set default values
    config->isInstalled = FALSE;
    config->runAtStartup = FALSE;
    config->watchdogEnabled = TRUE;
    config->watchdogInterval = DEFAULT_WATCHDOG_INTERVAL;
    strcpy(config->version, BROWSER_SANITY_VERSION);
    config->installPath[0] = '\0';
    
    // Read redirect configuration
    ReadRedirectConfig(&config->redirectConfig);
    
    // Try to open the registry key
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, BROWSER_SANITY_REG_KEY, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        // Key doesn't exist, use defaults
        return TRUE;
    }
    
    // Read install path
    dwSize = sizeof(config->installPath);
    if (RegQueryValueEx(hKey, INSTALL_PATH_VALUE, NULL, &dwType, (LPBYTE)config->installPath, &dwSize) == ERROR_SUCCESS) {
        if (dwType == REG_SZ && config->installPath[0] != '\0') {
            config->isInstalled = TRUE;
        }
    }
    
    // Read version
    dwSize = sizeof(config->version);
    RegQueryValueEx(hKey, INSTALL_VERSION_VALUE, NULL, &dwType, (LPBYTE)config->version, &dwSize);
    
    // Read run at startup flag
    dwSize = sizeof(DWORD);
    if (RegQueryValueEx(hKey, RUN_AT_STARTUP_VALUE, NULL, &dwType, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS) {
        if (dwType == REG_DWORD) {
            config->runAtStartup = (BOOL)dwValue;
        }
    }
    
    // Read watchdog enabled flag
    dwSize = sizeof(DWORD);
    if (RegQueryValueEx(hKey, WATCHDOG_ENABLED_VALUE, NULL, &dwType, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS) {
        if (dwType == REG_DWORD) {
            config->watchdogEnabled = (BOOL)dwValue;
        }
    }
    
    // Read watchdog interval
    dwSize = sizeof(DWORD);
    if (RegQueryValueEx(hKey, WATCHDOG_INTERVAL_VALUE, NULL, &dwType, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS) {
        if (dwType == REG_DWORD) {
            config->watchdogInterval = dwValue;
        }
    }
    
    RegCloseKey(hKey);
    return TRUE;
}

/**
 * @brief Writes the application configuration to the registry
 * 
 * @param config Pointer to an AppConfig structure to write
 * @return TRUE if successful, FALSE otherwise
 */
BOOL WriteAppConfig(const AppConfig* config) {
    HKEY hKey;
    DWORD dwDisposition;
    DWORD dwValue;
    
    // Create or open the registry key
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, BROWSER_SANITY_REG_KEY, 0, NULL, 
                      REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisposition) != ERROR_SUCCESS) {
        return FALSE;
    }
    
    // Write install path
    RegSetValueEx(hKey, INSTALL_PATH_VALUE, 0, REG_SZ, (LPBYTE)config->installPath, strlen(config->installPath) + 1);
    
    // Write version
    RegSetValueEx(hKey, INSTALL_VERSION_VALUE, 0, REG_SZ, (LPBYTE)config->version, strlen(config->version) + 1);
    
    // Write run at startup flag
    dwValue = config->runAtStartup;
    RegSetValueEx(hKey, RUN_AT_STARTUP_VALUE, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));
    
    // Write watchdog enabled flag
    dwValue = config->watchdogEnabled;
    RegSetValueEx(hKey, WATCHDOG_ENABLED_VALUE, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));
    
    // Write watchdog interval
    RegSetValueEx(hKey, WATCHDOG_INTERVAL_VALUE, 0, REG_DWORD, (LPBYTE)&config->watchdogInterval, sizeof(DWORD));
    
    // Write redirect configuration
    dwValue = config->redirectConfig.redirectEnabled;
    RegSetValueEx(hKey, REDIRECT_ENABLED_VALUE, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));
    
    dwValue = config->redirectConfig.useDefaultBrowser;
    RegSetValueEx(hKey, USE_DEFAULT_BROWSER_VALUE, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));
    
    RegSetValueEx(hKey, CUSTOM_BROWSER_PATH_VALUE, 0, REG_SZ, 
                 (LPBYTE)config->redirectConfig.customBrowserPath, 
                 strlen(config->redirectConfig.customBrowserPath) + 1);
    
    RegSetValueEx(hKey, CUSTOM_BROWSER_ARGS_VALUE, 0, REG_SZ, 
                 (LPBYTE)config->redirectConfig.customBrowserArgs, 
                 strlen(config->redirectConfig.customBrowserArgs) + 1);
    
    RegCloseKey(hKey);
    return TRUE;
}

/**
 * @brief Checks if the application is running from the installation directory
 * 
 * @return TRUE if running from installation directory, FALSE otherwise
 */
BOOL IsRunningFromInstallDir() {
    char exePath[MAX_PATH];
    char installPath[MAX_PATH];
    AppConfig config;
    
    // Get the path of the current executable
    if (GetModuleFileName(NULL, exePath, MAX_PATH) == 0) {
        return FALSE;
    }
    
    // Read the installation path from the registry
    ReadAppConfig(&config);
    
    if (config.installPath[0] == '\0') {
        return FALSE;
    }
    
    // Combine the installation path with the executable name
    strcpy(installPath, config.installPath);
    strcat(installPath, "\\BrowserSanity.exe");
    
    // Compare the paths
    return (_stricmp(exePath, installPath) == 0);
}

/**
 * @brief Checks if the msedge.exe redirector is installed
 * 
 * @return TRUE if installed, FALSE otherwise
 */
BOOL IsRedirectorInstalled() {
    char edgePath[MAX_PATH];
    DWORD attributes;
    
    // Get the path to the Edge executable
    sprintf(edgePath, "%s\\Microsoft\\Edge\\Application\\msedge.exe", getenv("ProgramFiles(x86)"));
    
    // Check if the file exists
    attributes = GetFileAttributes(edgePath);
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        return FALSE;
    }
    
    // TODO: Add more checks to verify it's our redirector
    
    return TRUE;
}

/**
 * @brief Installs the msedge.exe redirector
 *
 * @return TRUE if successful, FALSE otherwise
 */
BOOL InstallRedirector() {
    char edgePath[MAX_PATH];
    char backupPath[MAX_PATH];
    
    // Get the path to the Edge executable
    sprintf(edgePath, "%s\\Microsoft\\Edge\\Application\\msedge.exe", getenv("ProgramFiles(x86)"));
    
    // Create backup path
    sprintf(backupPath, "%s\\Microsoft\\Edge\\Application\\msedge.exe.original", getenv("ProgramFiles(x86)"));
    
    // Check if the backup already exists
    if (GetFileAttributes(backupPath) == INVALID_FILE_ATTRIBUTES) {
        // Create a backup of the original msedge.exe
        if (!CopyFile(edgePath, backupPath, TRUE)) {
            return FALSE;
        }
    }
    
    // Extract our redirector from embedded resources to the Edge location
    if (!ExtractMsedgeBinary(edgePath)) {
        return FALSE;
    }
    
    return TRUE;
}

/**
 * @brief Uninstalls the msedge.exe redirector
 * 
 * @return TRUE if successful, FALSE otherwise
 */
BOOL UninstallRedirector() {
    char edgePath[MAX_PATH];
    char backupPath[MAX_PATH];
    
    // Get the path to the Edge executable
    sprintf(edgePath, "%s\\Microsoft\\Edge\\Application\\msedge.exe", getenv("ProgramFiles(x86)"));
    
    // Create backup path
    sprintf(backupPath, "%s\\Microsoft\\Edge\\Application\\msedge.exe.original", getenv("ProgramFiles(x86)"));
    
    // Check if the backup exists
    if (GetFileAttributes(backupPath) != INVALID_FILE_ATTRIBUTES) {
        // Restore the original msedge.exe
        if (!CopyFile(backupPath, edgePath, FALSE)) {
            return FALSE;
        }
        
        // Delete the backup
        DeleteFile(backupPath);
    }
    
    return TRUE;
}

/**
 * @brief Sets the application to run at startup
 * 
 * @param enable TRUE to enable, FALSE to disable
 * @return TRUE if successful, FALSE otherwise
 */
BOOL SetRunAtStartup(BOOL enable) {
    HKEY hKey;
    char exePath[MAX_PATH];
    
    // Get the path of the current executable
    if (GetModuleFileName(NULL, exePath, MAX_PATH) == 0) {
        return FALSE;
    }
    
    // Open the Run registry key
    if (RegOpenKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 
                    0, KEY_WRITE, &hKey) != ERROR_SUCCESS) {
        return FALSE;
    }
    
    if (enable) {
        // Add the application to the Run key
        RegSetValueEx(hKey, "BrowserSanity", 0, REG_SZ, (LPBYTE)exePath, strlen(exePath) + 1);
    } else {
        // Remove the application from the Run key
        RegDeleteValue(hKey, "BrowserSanity");
    }
    
    RegCloseKey(hKey);
    
    // Update the configuration
    AppConfig config;
    ReadAppConfig(&config);
    config.runAtStartup = enable;
    WriteAppConfig(&config);
    
    return TRUE;
}

/**
 * @brief Checks if Browser Sanity is currently running (excluding this process)
 *
 * @param outPID Pointer to store the PID of running process (optional, can be NULL)
 * @return TRUE if another instance is running, FALSE otherwise
 */
BOOL IsProcessRunning() {
    return IsProcessRunningWithPID(NULL);
}

/**
 * @brief Checks if Browser Sanity is currently running and gets PID
 *
 * @param outPID Pointer to store the PID of running process (optional, can be NULL)
 * @return TRUE if another instance is running, FALSE otherwise
 */
BOOL IsProcessRunningWithPID(DWORD* outPID) {
    HANDLE hSnapshot;
    PROCESSENTRY32 pe32;
    DWORD currentPID = GetCurrentProcessId();
    char currentExeName[MAX_PATH];
    BOOL found = FALSE;
    
    if (outPID) {
        *outPID = 0;
    }
    
    // Get the current executable name
    if (GetModuleFileName(NULL, currentExeName, MAX_PATH) == 0) {
        return FALSE;
    }
    
    // Extract just the filename
    char* fileName = strrchr(currentExeName, '\\');
    if (fileName) {
        fileName++;
    } else {
        fileName = currentExeName;
    }
    
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return FALSE;
    }
    
    pe32.dwSize = sizeof(PROCESSENTRY32);
    
    if (Process32First(hSnapshot, &pe32)) {
        do {
            // Skip our own process
            if (pe32.th32ProcessID == currentPID) {
                continue;
            }
            
            // Check if this is BrowserSanity.exe
            if (_stricmp(pe32.szExeFile, fileName) == 0) {
                found = TRUE;
                if (outPID) {
                    *outPID = pe32.th32ProcessID;
                }
                break;
            }
            
        } while (Process32Next(hSnapshot, &pe32));
    }
    
    CloseHandle(hSnapshot);
    return found;
}

/**
 * @brief Comprehensive installation check
 *
 * @return TRUE if Browser Sanity is properly installed, FALSE otherwise
 */
BOOL IsComprehensivelyInstalled() {
    AppConfig config;
    char installPath[MAX_PATH];
    char exePath[MAX_PATH];
    
    // 1. Registry check
    ReadAppConfig(&config);
    if (!config.isInstalled || config.installPath[0] == '\0') {
        return FALSE;
    }
    
    // 2. Path check - verify the executable exists in the install directory
    sprintf(installPath, "%s\\BrowserSanity.exe", config.installPath);
    if (GetFileAttributes(installPath) == INVALID_FILE_ATTRIBUTES) {
        return FALSE;
    }
    
    // 3. Redirector check - verify msedge.exe redirector is installed
    if (!IsRedirectorInstalled()) {
        return FALSE;
    }
    
    return TRUE;
}

/**
 * @brief Checks if the redirector is intact
 * 
 * @return TRUE if intact, FALSE if tampered with or missing
 */
BOOL IsRedirectorIntact() {
    // For now, just check if it's installed
    return IsRedirectorInstalled();
    
    // TODO: Add more checks to verify it's our redirector and hasn't been tampered with
}

/**
 * @brief Shows a toast notification
 * 
 * @param title The notification title
 * @param message The notification message
 * @param actions Array of action strings
 * @param actionCount Number of actions
 * @return TRUE if successful, FALSE otherwise
 */
BOOL ShowToastNotification(const char* title, const char* message, const char** actions, int actionCount) {
    // TODO: Implement toast notifications using Windows API
    
    // For now, just show a message box
    char fullMessage[1024];
    sprintf(fullMessage, "%s\n\n%s", title, message);
    MessageBox(NULL, fullMessage, "Browser Sanity", MB_OK | MB_ICONINFORMATION);
    
    return TRUE;
}