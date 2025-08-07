/**
 * @file action_install.cpp
 * @brief Installation action implementation
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
    WCHAR wszTargetPath[MAX_PATH];
    WCHAR wszDescription[MAX_PATH];
    WCHAR wszIconPath[MAX_PATH];
    
    DebugLogInfo("Creating shortcut: %s -> %s", shortcutPath, targetPath);
    
    // Convert strings to wide characters
    MultiByteToWideChar(CP_ACP, 0, shortcutPath, -1, wszShortcutPath, MAX_PATH);
    MultiByteToWideChar(CP_ACP, 0, targetPath, -1, wszTargetPath, MAX_PATH);
    MultiByteToWideChar(CP_ACP, 0, description, -1, wszDescription, MAX_PATH);
    if (iconPath) {
        MultiByteToWideChar(CP_ACP, 0, iconPath, -1, wszIconPath, MAX_PATH);
    }
    
    // Initialize COM
    CoInitialize(NULL);
    
    // Create the IShellLink interface
    if (FAILED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                               IID_IShellLink, (void**)&psl))) {
        DebugLogError("Failed to create IShellLink interface");
        CoUninitialize();
        return FALSE;
    }
    
    // Set the path to the target
    psl->SetPath(wszTargetPath);
    
    // Set the description
    psl->SetDescription(wszDescription);
    
    // Set the icon
    if (iconPath) {
        psl->SetIconLocation(wszIconPath, iconIndex);
    }
    
    // Query for the IPersistFile interface
    if (FAILED(psl->QueryInterface(IID_IPersistFile, (void**)&ppf))) {
        DebugLogError("Failed to query IPersistFile interface");
        psl->Release();
        CoUninitialize();
        return FALSE;
    }
    
    // Save the shortcut
    if (FAILED(ppf->Save(wszShortcutPath, TRUE))) {
        DebugLogError("Failed to save shortcut to: %s", shortcutPath);
        ppf->Release();
        psl->Release();
        CoUninitialize();
        return FALSE;
    }
    
    // Release the interfaces
    ppf->Release();
    psl->Release();
    
    // Uninitialize COM
    CoUninitialize();
    
    DebugLogInfo("Shortcut created successfully: %s", shortcutPath);
    return TRUE;
}

/**
 * @brief Installs the application
 *
 * @return TRUE if successful, FALSE otherwise
 */
BOOL InstallApplication() {
    WCHAR exePath[MAX_PATH];
    WCHAR installDir[MAX_PATH];
    WCHAR installPath[MAX_PATH];
    WCHAR desktopPath[MAX_PATH];
    WCHAR startMenuPath[MAX_PATH];
    WCHAR shortcutPath[MAX_PATH];
    char exePathA[MAX_PATH];
    char installDirA[MAX_PATH];
    char installPathA[MAX_PATH];
    char desktopPathA[MAX_PATH];
    char startMenuPathA[MAX_PATH];
    char shortcutPathA[MAX_PATH];
    AppConfig config;
    
    DebugLogInfo("=== Starting Browser Sanity Installation ===");
    
    // Get the path of the current executable
    if (GetModuleFileName(NULL, exePath, MAX_PATH) == 0) {
        DebugLogError("Failed to get current executable path");
        return FALSE;
    }
    WideCharToMultiByte(CP_ACP, 0, exePath, -1, exePathA, MAX_PATH, NULL, NULL);
    DebugLogInfo("Current executable path: %s", exePathA);
    
    // Create the installation directory
    char* programFiles = getenv("ProgramFiles(x86)");
    if (!programFiles) programFiles = getenv("ProgramFiles");
    if (sprintf_s(installDirA, MAX_PATH, "%s\\Browser Sanity", programFiles) != 0) {
        DebugLogError("Failed to format installation directory path");
        return FALSE;
    }
    MultiByteToWideChar(CP_ACP, 0, installDirA, -1, installDir, MAX_PATH);
    DebugLogInfo("Creating installation directory: %s", installDirA);
    if (!CreateDirectory(installDir, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
        DebugLogError("Failed to create installation directory: %s", installDirA);
        return FALSE;
    }
    
    // Create the installation path
    if (sprintf_s(installPathA, MAX_PATH, "%s\\BrowserSanity.exe", installDirA) != 0) {
        DebugLogError("Failed to format installation path");
        return FALSE;
    }
    MultiByteToWideChar(CP_ACP, 0, installPathA, -1, installPath, MAX_PATH);
    DebugLogInfo("Installation path: %s", installPathA);
    
    // Copy the executable to the installation directory
    DebugLogInfo("Copying executable from %s to %s", exePathA, installPathA);
    if (!CopyFile(exePath, installPath, FALSE)) {
        DebugLogError("Failed to copy executable to installation directory");
        return FALSE;
    }
    DebugLogInfo("Executable copied successfully");
    
    // Get the desktop path
    SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, desktopPath);
    WideCharToMultiByte(CP_ACP, 0, desktopPath, -1, desktopPathA, MAX_PATH, NULL, NULL);
    DebugLogInfo("Desktop path: %s", desktopPathA);
    
    // Create the desktop shortcut
    if (sprintf_s(shortcutPathA, MAX_PATH, "%s\\Browser Sanity.lnk", desktopPathA) != 0) {
        DebugLogError("Failed to format desktop shortcut path");
    } else if (!CreateShortcut(shortcutPathA, installPathA, "Browser Sanity", installPathA, 0)) {
        DebugLogError("Failed to create desktop shortcut");
        // Continue installation even if shortcut creation fails
    }
    
    // Get the start menu path
    SHGetFolderPath(NULL, CSIDL_PROGRAMS, NULL, 0, startMenuPath);
    WideCharToMultiByte(CP_ACP, 0, startMenuPath, -1, startMenuPathA, MAX_PATH, NULL, NULL);
    DebugLogInfo("Start menu path: %s", startMenuPathA);
    
    // Create the start menu directory
    if (sprintf_s(shortcutPathA, MAX_PATH, "%s\\Browser Sanity", startMenuPathA) != 0) {
        DebugLogError("Failed to format start menu directory path");
    } else {
        MultiByteToWideChar(CP_ACP, 0, shortcutPathA, -1, shortcutPath, MAX_PATH);
        DebugLogInfo("Creating start menu directory: %s", shortcutPathA);
        CreateDirectory(shortcutPath, NULL);
    }
    
    // Create the start menu shortcut
    if (sprintf_s(shortcutPathA, MAX_PATH, "%s\\Browser Sanity\\Browser Sanity.lnk", startMenuPathA) != 0) {
        DebugLogError("Failed to format start menu shortcut path");
    } else if (!CreateShortcut(shortcutPathA, installPathA, "Browser Sanity", installPathA, 0)) {
        DebugLogError("Failed to create start menu shortcut");
        // Continue installation even if shortcut creation fails
    }
    
    // Update the configuration
    DebugLogInfo("Updating application configuration");
    ReadAppConfig(&config);
    strcpy(config.installPath, installDirA);
    strcpy(config.version, BROWSER_SANITY_VERSION);
    config.isInstalled = TRUE;
    WriteAppConfig(&config);
    DebugLogInfo("Configuration updated successfully");
    
    // Set to run at startup if enabled
    if (config.runAtStartup) {
        DebugLogInfo("Setting application to run at startup");
        SetRunAtStartup(TRUE);
    }
    
    // Install the redirector
    DebugLogInfo("Installing browser redirector");
    if (InstallRedirector()) {
        DebugLogInfo("Browser redirector installed successfully");
    } else {
        DebugLogError("Failed to install browser redirector");
        // Continue installation even if redirector fails
    }
    
    DebugLogInfo("=== Browser Sanity Installation Complete ===");
    return TRUE;
}

/**
 * @brief Launches the application from the installation directory
 *
 * @return TRUE if successful, FALSE otherwise
 */
BOOL LaunchInstalledApplication() {
    char installPathA[MAX_PATH];
    WCHAR installPath[MAX_PATH];
    AppConfig config;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    
    // Read the configuration
    ReadAppConfig(&config);
    
    if (config.installPath[0] == '\0') {
        return FALSE;
    }
    
    // Create the installation path
    if (sprintf_s(installPathA, MAX_PATH, "%s\\BrowserSanity.exe", config.installPath) != 0) {
        DebugLogError("Failed to format installation path for launch");
        return FALSE;
    }
    MultiByteToWideChar(CP_ACP, 0, installPathA, -1, installPath, MAX_PATH);
    
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
    DebugLogInfo("Running installer action");
    
    // We already have admin privileges, proceed with installation
    DebugLogInfo("Running with administrator privileges");
    
    // Install the application
    if (!InstallApplication()) {
        DebugLogError("Installation failed");
        MessageBox(NULL, L"Failed to install Browser Sanity.", L"Browser Sanity", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    // Show success message
    DebugLogInfo("Installation completed successfully");
    MessageBox(NULL, L"Browser Sanity has been installed successfully.", L"Browser Sanity", MB_OK | MB_ICONINFORMATION);
    
    // Launch the installed application
    DebugLogInfo("Launching installed application");
    LaunchInstalledApplication();
    
    return 0;
}