/**
 * @file action_install.c
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
    
    DebugLogInfo("Creating shortcut: %s -> %s", shortcutPath, targetPath);
    
    // Initialize COM
    CoInitialize(NULL);
    
    // Create the IShellLink interface
    if (FAILED(CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                               &IID_IShellLink, (void**)&psl))) {
        DebugLogError("Failed to create IShellLink interface");
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
        DebugLogError("Failed to query IPersistFile interface");
        psl->lpVtbl->Release(psl);
        CoUninitialize();
        return FALSE;
    }
    
    // Convert the shortcut path to a wide string
    MultiByteToWideChar(CP_ACP, 0, shortcutPath, -1, wszShortcutPath, MAX_PATH);
    
    // Save the shortcut
    if (FAILED(ppf->lpVtbl->Save(ppf, wszShortcutPath, TRUE))) {
        DebugLogError("Failed to save shortcut to: %s", shortcutPath);
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
    
    DebugLogInfo("Shortcut created successfully: %s", shortcutPath);
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
    
    DebugLogInfo("=== Starting Browser Sanity Installation ===");
    
    // Get the path of the current executable
    if (GetModuleFileName(NULL, exePath, MAX_PATH) == 0) {
        DebugLogError("Failed to get current executable path");
        return FALSE;
    }
    DebugLogInfo("Current executable path: %s", exePath);
    
    // Create the installation directory
    if (sprintf_s(installDir, MAX_PATH, "%s\\Browser Sanity", getenv("ProgramFiles(x86)")) != 0) {
        DebugLogError("Failed to format installation directory path");
        return FALSE;
    }
    DebugLogInfo("Creating installation directory: %s", installDir);
    if (!CreateDirectory(installDir, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
        DebugLogError("Failed to create installation directory: %s", installDir);
        return FALSE;
    }
    
    // Create the installation path
    if (sprintf_s(installPath, MAX_PATH, "%s\\BrowserSanity.exe", installDir) != 0) {
        DebugLogError("Failed to format installation path");
        return FALSE;
    }
    DebugLogInfo("Installation path: %s", installPath);
    
    // Copy the executable to the installation directory
    DebugLogInfo("Copying executable from %s to %s", exePath, installPath);
    if (!CopyFile(exePath, installPath, FALSE)) {
        DebugLogError("Failed to copy executable to installation directory");
        return FALSE;
    }
    DebugLogInfo("Executable copied successfully");
    
    // Get the desktop path
    SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, desktopPath);
    DebugLogInfo("Desktop path: %s", desktopPath);
    
    // Create the desktop shortcut
    if (sprintf_s(shortcutPath, MAX_PATH, "%s\\Browser Sanity.lnk", desktopPath) != 0) {
        DebugLogError("Failed to format desktop shortcut path");
    } else if (!CreateShortcut(shortcutPath, installPath, "Browser Sanity", installPath, 0)) {
        DebugLogError("Failed to create desktop shortcut");
        // Continue installation even if shortcut creation fails
    }
    
    // Get the start menu path
    SHGetFolderPath(NULL, CSIDL_PROGRAMS, NULL, 0, startMenuPath);
    DebugLogInfo("Start menu path: %s", startMenuPath);
    
    // Create the start menu directory
    if (sprintf_s(shortcutPath, MAX_PATH, "%s\\Browser Sanity", startMenuPath) != 0) {
        DebugLogError("Failed to format start menu directory path");
    } else {
        DebugLogInfo("Creating start menu directory: %s", shortcutPath);
        CreateDirectory(shortcutPath, NULL);
    }
    
    // Create the start menu shortcut
    if (sprintf_s(shortcutPath, MAX_PATH, "%s\\Browser Sanity\\Browser Sanity.lnk", startMenuPath) != 0) {
        DebugLogError("Failed to format start menu shortcut path");
    } else if (!CreateShortcut(shortcutPath, installPath, "Browser Sanity", installPath, 0)) {
        DebugLogError("Failed to create start menu shortcut");
        // Continue installation even if shortcut creation fails
    }
    
    // Update the configuration
    DebugLogInfo("Updating application configuration");
    ReadAppConfig(&config);
    strcpy(config.installPath, installDir);
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
    if (sprintf_s(installPath, MAX_PATH, "%s\\BrowserSanity.exe", config.installPath) != 0) {
        DebugLogError("Failed to format installation path for launch");
        return FALSE;
    }
    
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
    
    // Check if we need to elevate
    if (!IsUserAnAdmin()) {
        char exePath[MAX_PATH];
        
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
        sei.lpVerb = "runas";
        sei.lpFile = exePath;
        sei.lpParameters = "/install";
        sei.nShow = SW_NORMAL;
        
        if (!ShellExecuteEx(&sei)) {
            DebugLogError("Failed to elevate privileges");
            MessageBox(NULL, "Failed to elevate privileges.", "Browser Sanity", MB_OK | MB_ICONERROR);
            return 1;
        }
        
        DebugLogInfo("Elevation request sent successfully");
        return 0;
    }
    
    DebugLogInfo("Running with administrator privileges");
    
    // Install the application
    if (!InstallApplication()) {
        DebugLogError("Installation failed");
        MessageBox(NULL, "Failed to install Browser Sanity.", "Browser Sanity", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    // Show success message
    DebugLogInfo("Installation completed successfully");
    MessageBox(NULL, "Browser Sanity has been installed successfully.", "Browser Sanity", MB_OK | MB_ICONINFORMATION);
    
    // Launch the installed application
    DebugLogInfo("Launching installed application");
    LaunchInstalledApplication();
    
    return 0;
}