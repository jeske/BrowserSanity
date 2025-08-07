/**
 * @file action_uninstall.c
 * @brief Uninstallation action implementation
 */

#include "../../../include/browser_sanity.h"
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
    char installDir[MAX_PATH];
    char desktopPath[MAX_PATH];
    char startMenuPath[MAX_PATH];
    char shortcutPath[MAX_PATH];
    AppConfig config;
    
    // Read the configuration
    ReadAppConfig(&config);
    
    if (config.installPath[0] == '\0') {
        return FALSE;
    }
    
    // Uninstall the redirector
    UninstallRedirector();
    
    // Remove from startup
    SetRunAtStartup(FALSE);
    
    // Get the desktop path
    SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, desktopPath);
    
    // Remove the desktop shortcut
    sprintf(shortcutPath, "%s\\Browser Sanity.lnk", desktopPath);
    DeleteFile(shortcutPath);
    
    // Get the start menu path
    SHGetFolderPath(NULL, CSIDL_PROGRAMS, NULL, 0, startMenuPath);
    
    // Remove the start menu shortcut
    sprintf(shortcutPath, "%s\\Browser Sanity\\Browser Sanity.lnk", startMenuPath);
    DeleteFile(shortcutPath);
    
    // Remove the start menu directory
    sprintf(shortcutPath, "%s\\Browser Sanity", startMenuPath);
    RemoveDirectory(shortcutPath);
    
    // Delete the registry key
    RegDeleteKey(HKEY_LOCAL_MACHINE, BROWSER_SANITY_REG_KEY);
    
    // Copy the executable to the temp directory for self-deletion
    char tempPath[MAX_PATH];
    char tempExe[MAX_PATH];
    char installPath[MAX_PATH];
    char command[MAX_PATH * 2];
    
    GetTempPath(MAX_PATH, tempPath);
    sprintf(tempExe, "%s\\BrowserSanity_uninstall.exe", tempPath);
    sprintf(installPath, "%s\\BrowserSanity.exe", config.installPath);
    
    // Copy the executable to the temp directory
    CopyFile(installPath, tempExe, FALSE);
    
    // Create a batch file to delete the installation directory
    char batchPath[MAX_PATH];
    FILE* batch;
    
    sprintf(batchPath, "%s\\BrowserSanity_uninstall.bat", tempPath);
    batch = fopen(batchPath, "w");
    if (batch) {
        fprintf(batch, "@echo off\n");
        fprintf(batch, "timeout /t 1 /nobreak > nul\n");
        fprintf(batch, "del \"%s\"\n", installPath);
        fprintf(batch, "rmdir \"%s\"\n", config.installPath);
        fprintf(batch, "del \"%s\"\n", tempExe);
        fprintf(batch, "del \"%s\"\n", batchPath);
        fclose(batch);
        
        // Execute the batch file
        sprintf(command, "start \"\" \"%s\"", batchPath);
        system(command);
    }
    
    return TRUE;
}

/**
 * @brief Runs the application in uninstaller mode
 *
 * @return Exit code
 */
int RunUninstallerAction() {
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