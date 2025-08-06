/**
 * @file installer.c
 * @brief Implementation of the installer/uninstaller functionality
 */

#include "browser_sanity.h"
#include <shlwapi.h>
#include <shlobj.h>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")

/**
 * @brief Creates a shortcut
 * 
 * @param shortcutPath Path where the shortcut will be created
 * @param targetPath Path to the target executable
 * @param description Description of the shortcut
 * @param iconPath Path to the icon file
 * @param iconIndex Index of the icon in the file
 * @return TRUE if successful, FALSE otherwise
 */
static BOOL CreateShortcut(const char* shortcutPath, const char* targetPath, 
                          const char* description, const char* iconPath, int iconIndex) {
    IShellLink* psl;
    IPersistFile* ppf;
    WCHAR wszShortcutPath[MAX_PATH];
    
    // Initialize COM
    CoInitialize(NULL);
    
    // Create the IShellLink interface
    if (FAILED(CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, 
                               &IID_IShellLink, (void**)&psl))) {
        CoUninitialize();
        return FALSE;
    }
    
    // Set the path to the target
    psl->lpVtbl->SetPath(psl, targetPath);
    
    // Set the description
    psl->lpVtbl->SetDescription(psl, description);
    
    // Set the icon
    if (iconPath) {
        psl->lpVtbl->SetIconLocation(psl, iconPath, iconIndex);
    }
    
    // Query for the IPersistFile interface
    if (FAILED(psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, (void**)&ppf))) {
        psl->lpVtbl->Release(psl);
        CoUninitialize();
        return FALSE;
    }
    
    // Convert the shortcut path to a wide string
    MultiByteToWideChar(CP_ACP, 0, shortcutPath, -1, wszShortcutPath, MAX_PATH);
    
    // Save the shortcut
    if (FAILED(ppf->lpVtbl->Save(ppf, wszShortcutPath, TRUE))) {
        ppf->lpVtbl->Release(ppf);
        psl->lpVtbl->Release(psl);
        CoUninitialize();
        return FALSE;
    }
    
    // Release the interfaces
    ppf->lpVtbl->Release(ppf);
    psl->lpVtbl->Release(psl);
    
    // Uninitialize COM
    CoUninitialize();
    
    return TRUE;
}

/**
 * @brief Installs the application
 * 
 * @return TRUE if successful, FALSE otherwise
 */
BOOL InstallApplication() {
    char exePath[MAX_PATH];
    char installDir[MAX_PATH];
    char installPath[MAX_PATH];
    char desktopPath[MAX_PATH];
    char startMenuPath[MAX_PATH];
    char shortcutPath[MAX_PATH];
    AppConfig config;
    
    // Get the path of the current executable
    if (GetModuleFileName(NULL, exePath, MAX_PATH) == 0) {
        return FALSE;
    }
    
    // Create the installation directory
    sprintf(installDir, "%s\\Browser Sanity", getenv("ProgramFiles(x86)"));
    if (!CreateDirectory(installDir, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
        return FALSE;
    }
    
    // Create the installation path
    sprintf(installPath, "%s\\BrowserSanity.exe", installDir);
    
    // Copy the executable to the installation directory
    if (!CopyFile(exePath, installPath, FALSE)) {
        return FALSE;
    }
    
    // Get the desktop path
    SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, desktopPath);
    
    // Create the desktop shortcut
    sprintf(shortcutPath, "%s\\Browser Sanity.lnk", desktopPath);
    CreateShortcut(shortcutPath, installPath, "Browser Sanity", installPath, 0);
    
    // Get the start menu path
    SHGetFolderPath(NULL, CSIDL_PROGRAMS, NULL, 0, startMenuPath);
    
    // Create the start menu directory
    sprintf(shortcutPath, "%s\\Browser Sanity", startMenuPath);
    CreateDirectory(shortcutPath, NULL);
    
    // Create the start menu shortcut
    sprintf(shortcutPath, "%s\\Browser Sanity\\Browser Sanity.lnk", startMenuPath);
    CreateShortcut(shortcutPath, installPath, "Browser Sanity", installPath, 0);
    
    // Update the configuration
    ReadAppConfig(&config);
    strcpy(config.installPath, installDir);
    strcpy(config.version, BROWSER_SANITY_VERSION);
    config.isInstalled = TRUE;
    WriteAppConfig(&config);
    
    // Set to run at startup if enabled
    if (config.runAtStartup) {
        SetRunAtStartup(TRUE);
    }
    
    // Install the redirector
    InstallRedirector();
    
    return TRUE;
}

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
 * @brief Launches the application from the installation directory
 * 
 * @return TRUE if successful, FALSE otherwise
 */
BOOL LaunchInstalledApplication() {
    char installPath[MAX_PATH];
    AppConfig config;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    
    // Read the configuration
    ReadAppConfig(&config);
    
    if (config.installPath[0] == '\0') {
        return FALSE;
    }
    
    // Create the installation path
    sprintf(installPath, "%s\\BrowserSanity.exe", config.installPath);
    
    // Launch the application
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    
    if (CreateProcess(installPath, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return TRUE;
    }
    
    return FALSE;
}