/**
 * @file action_uninstall.cpp
 * @brief Uninstallation action implementation
 */

#include <browser_sanity.h>
#include <debug_log.h>
#include <safe_strings.h>
#include <windows.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <shlobj.h>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")

/**
 * @brief Uninstalls the application
 *
 * @return TRUE if successful, FALSE otherwise
 */
BOOL UninstallApplication() {
    WCHAR desktopPath[MAX_PATH];
    WCHAR startMenuPath[MAX_PATH];
    WCHAR shortcutPath[MAX_PATH];
    char desktopPathA[MAX_PATH];
    char startMenuPathA[MAX_PATH];
    char shortcutPathA[MAX_PATH];
    AppConfig config;
    
    DebugLogInfo("=== Starting Browser Sanity Uninstallation ===");
    
    // Read the configuration
    ReadAppConfig(&config);
    
    if (config.installPath[0] == '\0') {
        DebugLogError("No installation path found in configuration");
        return FALSE;
    }
    
    DebugLogInfo("Installation path: %s", config.installPath);
    
    // Uninstall the redirector
    DebugLogInfo("Uninstalling browser redirector");
    if (UninstallRedirector()) {
        DebugLogInfo("Browser redirector uninstalled successfully");
    } else {
        DebugLogError("Failed to uninstall browser redirector");
    }
    
    // Remove from startup
    DebugLogInfo("Removing from startup");
    SetRunAtStartup(FALSE);
    
    // Get the desktop path
    SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, desktopPath);
    WideCharToMultiByte(CP_ACP, 0, desktopPath, -1, desktopPathA, MAX_PATH, NULL, NULL);
    DebugLogInfo("Desktop path: %s", desktopPathA);
    
    // Remove the desktop shortcut
    if (sprintf_s(shortcutPathA, MAX_PATH, "%s\\Browser Sanity.lnk", desktopPathA) != 0) {
        DebugLogError("Failed to format desktop shortcut path");
    } else {
        MultiByteToWideChar(CP_ACP, 0, shortcutPathA, -1, shortcutPath, MAX_PATH);
        DebugLogInfo("Removing desktop shortcut: %s", shortcutPathA);
        if (DeleteFile(shortcutPath)) {
            DebugLogInfo("Desktop shortcut removed successfully");
        } else {
            DebugLogInfo("Desktop shortcut not found or already removed");
        }
    }
    
    // Get the start menu path
    SHGetFolderPath(NULL, CSIDL_PROGRAMS, NULL, 0, startMenuPath);
    WideCharToMultiByte(CP_ACP, 0, startMenuPath, -1, startMenuPathA, MAX_PATH, NULL, NULL);
    DebugLogInfo("Start menu path: %s", startMenuPathA);
    
    // Remove the start menu shortcut
    if (sprintf_s(shortcutPathA, MAX_PATH, "%s\\Browser Sanity\\Browser Sanity.lnk", startMenuPathA) != 0) {
        DebugLogError("Failed to format start menu shortcut path");
    } else {
        MultiByteToWideChar(CP_ACP, 0, shortcutPathA, -1, shortcutPath, MAX_PATH);
        DebugLogInfo("Removing start menu shortcut: %s", shortcutPathA);
        if (DeleteFile(shortcutPath)) {
            DebugLogInfo("Start menu shortcut removed successfully");
        } else {
            DebugLogInfo("Start menu shortcut not found or already removed");
        }
    }
    
    // Remove the start menu directory
    if (sprintf_s(shortcutPathA, MAX_PATH, "%s\\Browser Sanity", startMenuPathA) != 0) {
        DebugLogError("Failed to format start menu directory path");
    } else {
        MultiByteToWideChar(CP_ACP, 0, shortcutPathA, -1, shortcutPath, MAX_PATH);
        DebugLogInfo("Removing start menu directory: %s", shortcutPathA);
        if (RemoveDirectory(shortcutPath)) {
            DebugLogInfo("Start menu directory removed successfully");
        } else {
            DebugLogInfo("Start menu directory not found or already removed");
        }
    }
    
    // Delete the registry key
    WCHAR regKeyW[256];
    MultiByteToWideChar(CP_ACP, 0, BROWSER_SANITY_REG_KEY, -1, regKeyW, 256);
    DebugLogInfo("Removing registry key: %s", BROWSER_SANITY_REG_KEY);
    if (RegDeleteKey(HKEY_LOCAL_MACHINE, regKeyW) == ERROR_SUCCESS) {
        DebugLogInfo("Registry key removed successfully");
    } else {
        DebugLogInfo("Registry key not found or already removed");
    }
    
    // Copy the executable to the temp directory for self-deletion
    WCHAR tempPath[MAX_PATH];
    WCHAR tempExe[MAX_PATH];
    WCHAR installPath[MAX_PATH];
    char tempPathA[MAX_PATH];
    char tempExeA[MAX_PATH];
    char installPathA[MAX_PATH];
    char command[COMMAND_LINE_SIZE];
    
    GetTempPath(MAX_PATH, tempPath);
    WideCharToMultiByte(CP_ACP, 0, tempPath, -1, tempPathA, MAX_PATH, NULL, NULL);
    if (sprintf_s(tempExeA, MAX_PATH, "%s\\BrowserSanity_uninstall.exe", tempPathA) != 0) {
        DebugLogError("Failed to format temp executable path");
        return FALSE;
    }
    if (sprintf_s(installPathA, MAX_PATH, "%s\\BrowserSanity.exe", config.installPath) != 0) {
        DebugLogError("Failed to format install path");
        return FALSE;
    }
    MultiByteToWideChar(CP_ACP, 0, tempExeA, -1, tempExe, MAX_PATH);
    MultiByteToWideChar(CP_ACP, 0, installPathA, -1, installPath, MAX_PATH);
    
    DebugLogInfo("Setting up self-deletion process");
    DebugLogInfo("Temp path: %s", tempPathA);
    DebugLogInfo("Install path: %s", installPathA);
    
    // Copy the executable to the temp directory
    DebugLogInfo("Copying executable to temp directory for self-deletion");
    if (CopyFile(installPath, tempExe, FALSE)) {
        DebugLogInfo("Executable copied to temp directory successfully");
    } else {
        DebugLogError("Failed to copy executable to temp directory");
    }
    
    // Create a batch file to delete the installation directory
    char batchPath[MAX_PATH];
    FILE* batch;
    
    if (sprintf_s(batchPath, MAX_PATH, "%s\\BrowserSanity_uninstall.bat", tempPathA) != 0) {
        DebugLogError("Failed to format batch file path");
        return FALSE;
    }
    
    DebugLogInfo("Creating cleanup batch file: %s", batchPath);
    batch = fopen(batchPath, "w");
    if (batch) {
        fprintf(batch, "@echo off\n");
        fprintf(batch, "timeout /t 1 /nobreak > nul\n");
        fprintf(batch, "del \"%s\"\n", installPathA);
        fprintf(batch, "rmdir \"%s\"\n", config.installPath);
        fprintf(batch, "del \"%s\"\n", tempExeA);
        fprintf(batch, "del \"%s\"\n", batchPath);
        fclose(batch);
        
        // Execute the batch file
        if (sprintf_s(command, COMMAND_LINE_SIZE, "start \"\" \"%s\"", batchPath) != 0) {
            DebugLogError("Failed to format batch command");
            return FALSE;
        }
        DebugLogInfo("Executing cleanup batch file: %s", command);
        system(command);
        DebugLogInfo("Cleanup batch file started");
    } else {
        DebugLogError("Failed to create cleanup batch file");
    }
    
    DebugLogInfo("=== Browser Sanity Uninstallation Complete ===");
    return TRUE;
}

/**
 * @brief Runs the application in uninstaller mode
 *
 * @return Exit code
 */
int RunUninstallerAction() {
    DebugLogInfo("Running uninstaller action");
    
    // Check if we need to elevate
    if (!IsUserAnAdmin()) {
        WCHAR exePath[MAX_PATH];
        
        DebugLogInfo("Elevation required - requesting administrator privileges");
        
        // Get the path of the current executable
        if (GetModuleFileName(NULL, exePath, MAX_PATH) == 0) {
            DebugLogError("Failed to get executable path for elevation");
            return 1;
        }
        
        // Elevate and run with the same parameters
        SHELLEXECUTEINFO sei;
        ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
        sei.cbSize = sizeof(SHELLEXECUTEINFO);
        sei.lpVerb = L"runas";
        sei.lpFile = exePath;
        sei.lpParameters = L"/uninstall";
        sei.nShow = SW_NORMAL;
        
        if (!ShellExecuteEx(&sei)) {
            DebugLogError("Failed to elevate privileges");
            MessageBox(NULL, L"Failed to elevate privileges.", L"Browser Sanity", MB_OK | MB_ICONERROR);
            return 1;
        }
        
        DebugLogInfo("Elevation request sent successfully");
        return 0;
    }
    
    DebugLogInfo("Running with administrator privileges");
    
    // Uninstall the application
    if (!UninstallApplication()) {
        DebugLogError("Uninstallation failed");
        MessageBox(NULL, L"Failed to uninstall Browser Sanity.", L"Browser Sanity", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    // Show success message
    DebugLogInfo("Uninstallation completed successfully");
    MessageBox(NULL, L"Browser Sanity has been uninstalled successfully.", L"Browser Sanity", MB_OK | MB_ICONINFORMATION);
    
    return 0;
}