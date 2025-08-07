/**
 * @file action_install.c
 * @brief Installation action implementation
 */

#include "../../../include/browser_sanity.h"
#include <windows.h>
#include <shellapi.h>
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

/**
 * @brief Runs the application in installer mode
 *
 * @return Exit code
 */
int RunInstallerAction() {
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