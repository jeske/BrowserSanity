/**
 * @file ui.c
 * @brief Implementation of the user interface
 */

#include "browser_sanity.h"
#include <commctrl.h>
#include <windowsx.h>
#include <shellapi.h>

#pragma comment(lib, "comctl32.lib")

// Window class name
#define WINDOW_CLASS_NAME "BrowserSanityWindow"

// Window dimensions
#define WINDOW_WIDTH 550
#define WINDOW_HEIGHT 450

// Control IDs
#define IDC_DESCRIPTION_TEXT 1000
#define IDC_STATUS_LABEL 1001
#define IDC_REDIRECT_CHECKBOX 1002
#define IDC_DEFAULT_BROWSER_RADIO 1003
#define IDC_CUSTOM_BROWSER_RADIO 1004
#define IDC_CUSTOM_BROWSER_PATH 1005
#define IDC_BROWSE_BUTTON 1006
#define IDC_CUSTOM_ARGS_LABEL 1007
#define IDC_CUSTOM_ARGS_EDIT 1008
#define IDC_STARTUP_CHECKBOX 1009
#define IDC_WATCHDOG_CHECKBOX 1010
#define IDC_INSTALL_BUTTON 1011
#define IDC_UNINSTALL_BUTTON 1012
#define IDC_APPLY_BUTTON 1013
#define IDC_GITHUB_LINK 1014
#define IDC_SHOW_SETTINGS_BUTTON 1015
#define IDC_LAUNCH_BUTTON 1016

// Global variables
static HINSTANCE g_hInstance = NULL;
static HWND g_hMainWindow = NULL;
static AppConfig g_config;

// Forward declarations
static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void CreateControls(HWND hwnd);
static void UpdateControls(HWND hwnd);
static void SaveSettings(HWND hwnd);
static void BrowseForBrowser(HWND hwnd);
static void InstallApp(HWND hwnd);
static void UninstallApp(HWND hwnd);
static int ShowAlreadyInstalledDialog(HWND hwnd);

/**
 * @brief Initializes the UI
 * 
 * @param hInstance Instance handle
 * @param nCmdShow Show command
 * @return TRUE if successful, FALSE otherwise
 */
BOOL InitUI(HINSTANCE hInstance, int nCmdShow) {
    WNDCLASSEX wc;
    HWND hwnd;
    MSG msg;
    
    // Save the instance handle
    g_hInstance = hInstance;
    
    // Initialize common controls
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icc);
    
    // Register the window class
    ZeroMemory(&wc, sizeof(WNDCLASSEX));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = WINDOW_CLASS_NAME;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    
    if (!RegisterClassEx(&wc)) {
        return FALSE;
    }
    
    // Read the configuration
    ReadAppConfig(&g_config);
    
    // Read the configuration - the logic for when to show this UI is now handled in main.c
    // This function only shows the settings UI when explicitly called
    
    // Create the window
    hwnd = CreateWindowEx(
        0,
        WINDOW_CLASS_NAME,
        g_config.isInstalled ? "Browser Sanity - Settings" : "Browser Sanity - Installation",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, NULL, hInstance, NULL
    );
    
    if (!hwnd) {
        return FALSE;
    }
    
    // Save the window handle
    g_hMainWindow = hwnd;
    
    // Create the controls
    CreateControls(hwnd);
    
    // Update the controls
    UpdateControls(hwnd);
    
    // Show the window
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    
    // Message loop
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return TRUE;
}

/**
 * @brief Window procedure
 */
static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            return 0;
            
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDC_REDIRECT_CHECKBOX:
                    if (HIWORD(wParam) == BN_CLICKED) {
                        BOOL checked = Button_GetCheck((HWND)lParam);
                        EnableWindow(GetDlgItem(hwnd, IDC_DEFAULT_BROWSER_RADIO), checked);
                        EnableWindow(GetDlgItem(hwnd, IDC_CUSTOM_BROWSER_RADIO), checked);
                        EnableWindow(GetDlgItem(hwnd, IDC_CUSTOM_BROWSER_PATH), 
                                    checked && Button_GetCheck(GetDlgItem(hwnd, IDC_CUSTOM_BROWSER_RADIO)));
                        EnableWindow(GetDlgItem(hwnd, IDC_BROWSE_BUTTON), 
                                    checked && Button_GetCheck(GetDlgItem(hwnd, IDC_CUSTOM_BROWSER_RADIO)));
                        EnableWindow(GetDlgItem(hwnd, IDC_CUSTOM_ARGS_LABEL), 
                                    checked && Button_GetCheck(GetDlgItem(hwnd, IDC_CUSTOM_BROWSER_RADIO)));
                        EnableWindow(GetDlgItem(hwnd, IDC_CUSTOM_ARGS_EDIT), 
                                    checked && Button_GetCheck(GetDlgItem(hwnd, IDC_CUSTOM_BROWSER_RADIO)));
                    }
                    return 0;
                    
                case IDC_DEFAULT_BROWSER_RADIO:
                case IDC_CUSTOM_BROWSER_RADIO:
                    if (HIWORD(wParam) == BN_CLICKED) {
                        BOOL useCustom = Button_GetCheck(GetDlgItem(hwnd, IDC_CUSTOM_BROWSER_RADIO));
                        EnableWindow(GetDlgItem(hwnd, IDC_CUSTOM_BROWSER_PATH), useCustom);
                        EnableWindow(GetDlgItem(hwnd, IDC_BROWSE_BUTTON), useCustom);
                        EnableWindow(GetDlgItem(hwnd, IDC_CUSTOM_ARGS_LABEL), useCustom);
                        EnableWindow(GetDlgItem(hwnd, IDC_CUSTOM_ARGS_EDIT), useCustom);
                    }
                    return 0;
                    
                case IDC_BROWSE_BUTTON:
                    if (HIWORD(wParam) == BN_CLICKED) {
                        BrowseForBrowser(hwnd);
                    }
                    return 0;
                    
                case IDC_APPLY_BUTTON:
                    if (HIWORD(wParam) == BN_CLICKED) {
                        SaveSettings(hwnd);
                    }
                    return 0;
                    
                case IDC_INSTALL_BUTTON:
                    if (HIWORD(wParam) == BN_CLICKED) {
                        InstallApp(hwnd);
                    }
                    return 0;
                    
                case IDC_UNINSTALL_BUTTON:
                    if (HIWORD(wParam) == BN_CLICKED) {
                        UninstallApp(hwnd);
                    }
                    return 0;
                    
                case IDC_GITHUB_LINK:
                    if (HIWORD(wParam) == STN_CLICKED) {
                        ShellExecute(NULL, "open", "https://github.com/yourusername/browser-sanity", NULL, NULL, SW_SHOWNORMAL);
                    }
                    return 0;
            }
            break;
            
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/**
 * @brief Creates the controls
 */
static void CreateControls(HWND hwnd) {
    HWND hControl;
    HFONT hFont;
    RECT rcClient;
    int y = 20;
    
    // Get the client area
    GetClientRect(hwnd, &rcClient);
    
    // Create a font
    hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                      OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                      DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    
    // Create the rich description text (only if not installed)
    if (!g_config.isInstalled) {
        hControl = CreateWindow(
            "STATIC",
            "Browser Sanity prevents applications from bypassing your default browser choice by "
            "redirecting Microsoft Edge launches to your preferred browser.\r\n\r\n"
            "• Lightweight msedge.exe replacement\r\n"
            "• Respects your default browser settings\r\n"
            "• Background monitoring and protection\r\n"
            "• Easy installation and configuration",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            20, y, rcClient.right - 40, 80,
            hwnd, (HMENU)IDC_DESCRIPTION_TEXT, g_hInstance, NULL
        );
        SendMessage(hControl, WM_SETFONT, (WPARAM)hFont, TRUE);
        y += 90;
    }
    
    // Create the status label
    hControl = CreateWindow(
        "STATIC", "Status: Not installed",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, y, rcClient.right - 40, 20,
        hwnd, (HMENU)IDC_STATUS_LABEL, g_hInstance, NULL
    );
    SendMessage(hControl, WM_SETFONT, (WPARAM)hFont, TRUE);
    y += 30;
    
    // Create the redirect checkbox
    hControl = CreateWindow(
        "BUTTON", "Enable msedge.exe redirection",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        20, y, rcClient.right - 40, 20,
        hwnd, (HMENU)IDC_REDIRECT_CHECKBOX, g_hInstance, NULL
    );
    SendMessage(hControl, WM_SETFONT, (WPARAM)hFont, TRUE);
    y += 30;
    
    // Create the default browser radio button
    hControl = CreateWindow(
        "BUTTON", "Use default browser",
        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
        40, y, rcClient.right - 60, 20,
        hwnd, (HMENU)IDC_DEFAULT_BROWSER_RADIO, g_hInstance, NULL
    );
    SendMessage(hControl, WM_SETFONT, (WPARAM)hFont, TRUE);
    y += 25;
    
    // Create the custom browser radio button
    hControl = CreateWindow(
        "BUTTON", "Use custom browser",
        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
        40, y, rcClient.right - 60, 20,
        hwnd, (HMENU)IDC_CUSTOM_BROWSER_RADIO, g_hInstance, NULL
    );
    SendMessage(hControl, WM_SETFONT, (WPARAM)hFont, TRUE);
    y += 25;
    
    // Create the custom browser path edit
    hControl = CreateWindow(
        "EDIT", "",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        60, y, rcClient.right - 140, 20,
        hwnd, (HMENU)IDC_CUSTOM_BROWSER_PATH, g_hInstance, NULL
    );
    SendMessage(hControl, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    // Create the browse button
    hControl = CreateWindow(
        "BUTTON", "Browse...",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        rcClient.right - 100, y, 60, 20,
        hwnd, (HMENU)IDC_BROWSE_BUTTON, g_hInstance, NULL
    );
    SendMessage(hControl, WM_SETFONT, (WPARAM)hFont, TRUE);
    y += 30;
    
    // Create the custom args label
    hControl = CreateWindow(
        "STATIC", "Custom arguments:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        60, y, 120, 20,
        hwnd, (HMENU)IDC_CUSTOM_ARGS_LABEL, g_hInstance, NULL
    );
    SendMessage(hControl, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    // Create the custom args edit
    hControl = CreateWindow(
        "EDIT", "",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        180, y, rcClient.right - 200, 20,
        hwnd, (HMENU)IDC_CUSTOM_ARGS_EDIT, g_hInstance, NULL
    );
    SendMessage(hControl, WM_SETFONT, (WPARAM)hFont, TRUE);
    y += 40;
    
    // Create the startup checkbox
    hControl = CreateWindow(
        "BUTTON", "Run at Windows startup",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        20, y, rcClient.right - 40, 20,
        hwnd, (HMENU)IDC_STARTUP_CHECKBOX, g_hInstance, NULL
    );
    SendMessage(hControl, WM_SETFONT, (WPARAM)hFont, TRUE);
    y += 25;
    
    // Create the watchdog checkbox
    hControl = CreateWindow(
        "BUTTON", "Enable watchdog (monitors and repairs the redirector if tampered with)",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        20, y, rcClient.right - 40, 20,
        hwnd, (HMENU)IDC_WATCHDOG_CHECKBOX, g_hInstance, NULL
    );
    SendMessage(hControl, WM_SETFONT, (WPARAM)hFont, TRUE);
    y += 40;
    
    // Create the GitHub link
    hControl = CreateWindow(
        "STATIC", "Visit GitHub Repository",
        WS_CHILD | WS_VISIBLE | SS_NOTIFY | SS_CENTER,
        20, y, rcClient.right - 40, 20,
        hwnd, (HMENU)IDC_GITHUB_LINK, g_hInstance, NULL
    );
    SendMessage(hControl, WM_SETFONT, (WPARAM)hFont, TRUE);
    y += 40;
    
    // Create the install button
    hControl = CreateWindow(
        "BUTTON", "Install",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        rcClient.right / 2 - 150, y, 90, 30,
        hwnd, (HMENU)IDC_INSTALL_BUTTON, g_hInstance, NULL
    );
    SendMessage(hControl, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    // Create the uninstall button
    hControl = CreateWindow(
        "BUTTON", "Uninstall",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        rcClient.right / 2 - 45, y, 90, 30,
        hwnd, (HMENU)IDC_UNINSTALL_BUTTON, g_hInstance, NULL
    );
    SendMessage(hControl, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    // Create the apply button
    hControl = CreateWindow(
        "BUTTON", "Apply Settings",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        rcClient.right / 2 + 60, y, 90, 30,
        hwnd, (HMENU)IDC_APPLY_BUTTON, g_hInstance, NULL
    );
    SendMessage(hControl, WM_SETFONT, (WPARAM)hFont, TRUE);
}

/**
 * @brief Updates the controls based on the configuration
 */
static void UpdateControls(HWND hwnd) {
    char statusText[256];
    
    // Update the status label
    if (g_config.isInstalled) {
        sprintf(statusText, "Status: Installed (Version %s)", g_config.version);
        SetWindowText(GetDlgItem(hwnd, IDC_STATUS_LABEL), statusText);
        
        // Update the install/uninstall buttons
        EnableWindow(GetDlgItem(hwnd, IDC_INSTALL_BUTTON), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_UNINSTALL_BUTTON), TRUE);
    } else {
        SetWindowText(GetDlgItem(hwnd, IDC_STATUS_LABEL), "Status: Not installed");
        
        // Update the install/uninstall buttons
        EnableWindow(GetDlgItem(hwnd, IDC_INSTALL_BUTTON), TRUE);
        EnableWindow(GetDlgItem(hwnd, IDC_UNINSTALL_BUTTON), FALSE);
    }
    
    // Update the redirect checkbox
    Button_SetCheck(GetDlgItem(hwnd, IDC_REDIRECT_CHECKBOX), g_config.redirectConfig.redirectEnabled);
    
    // Update the browser radio buttons
    Button_SetCheck(GetDlgItem(hwnd, IDC_DEFAULT_BROWSER_RADIO), g_config.redirectConfig.useDefaultBrowser);
    Button_SetCheck(GetDlgItem(hwnd, IDC_CUSTOM_BROWSER_RADIO), !g_config.redirectConfig.useDefaultBrowser);
    
    // Update the custom browser path
    SetWindowText(GetDlgItem(hwnd, IDC_CUSTOM_BROWSER_PATH), g_config.redirectConfig.customBrowserPath);
    
    // Update the custom args
    SetWindowText(GetDlgItem(hwnd, IDC_CUSTOM_ARGS_EDIT), g_config.redirectConfig.customBrowserArgs);
    
    // Update the startup checkbox
    Button_SetCheck(GetDlgItem(hwnd, IDC_STARTUP_CHECKBOX), g_config.runAtStartup);
    
    // Update the watchdog checkbox
    Button_SetCheck(GetDlgItem(hwnd, IDC_WATCHDOG_CHECKBOX), g_config.watchdogEnabled);
    
    // Enable/disable controls based on the redirect checkbox
    BOOL redirectEnabled = Button_GetCheck(GetDlgItem(hwnd, IDC_REDIRECT_CHECKBOX));
    EnableWindow(GetDlgItem(hwnd, IDC_DEFAULT_BROWSER_RADIO), redirectEnabled);
    EnableWindow(GetDlgItem(hwnd, IDC_CUSTOM_BROWSER_RADIO), redirectEnabled);
    
    // Enable/disable controls based on the custom browser radio
    BOOL useCustom = Button_GetCheck(GetDlgItem(hwnd, IDC_CUSTOM_BROWSER_RADIO));
    EnableWindow(GetDlgItem(hwnd, IDC_CUSTOM_BROWSER_PATH), redirectEnabled && useCustom);
    EnableWindow(GetDlgItem(hwnd, IDC_BROWSE_BUTTON), redirectEnabled && useCustom);
    EnableWindow(GetDlgItem(hwnd, IDC_CUSTOM_ARGS_LABEL), redirectEnabled && useCustom);
    EnableWindow(GetDlgItem(hwnd, IDC_CUSTOM_ARGS_EDIT), redirectEnabled && useCustom);
}

/**
 * @brief Saves the settings from the controls
 */
static void SaveSettings(HWND hwnd) {
    char buffer[MAX_PATH];
    
    // Get the redirect checkbox
    g_config.redirectConfig.redirectEnabled = Button_GetCheck(GetDlgItem(hwnd, IDC_REDIRECT_CHECKBOX));
    
    // Get the browser radio buttons
    g_config.redirectConfig.useDefaultBrowser = Button_GetCheck(GetDlgItem(hwnd, IDC_DEFAULT_BROWSER_RADIO));
    
    // Get the custom browser path
    GetWindowText(GetDlgItem(hwnd, IDC_CUSTOM_BROWSER_PATH), buffer, MAX_PATH);
    strcpy(g_config.redirectConfig.customBrowserPath, buffer);
    
    // Get the custom args
    GetWindowText(GetDlgItem(hwnd, IDC_CUSTOM_ARGS_EDIT), buffer, MAX_PATH);
    strcpy(g_config.redirectConfig.customBrowserArgs, buffer);
    
    // Get the startup checkbox
    BOOL runAtStartup = Button_GetCheck(GetDlgItem(hwnd, IDC_STARTUP_CHECKBOX));
    if (runAtStartup != g_config.runAtStartup) {
        SetRunAtStartup(runAtStartup);
        g_config.runAtStartup = runAtStartup;
    }
    
    // Get the watchdog checkbox
    BOOL watchdogEnabled = Button_GetCheck(GetDlgItem(hwnd, IDC_WATCHDOG_CHECKBOX));
    if (watchdogEnabled != g_config.watchdogEnabled) {
        SetWatchdogEnabled(watchdogEnabled);
        g_config.watchdogEnabled = watchdogEnabled;
    }
    
    // Write the configuration
    WriteAppConfig(&g_config);
    
    // Show a message
    MessageBox(hwnd, "Settings saved successfully.", "Browser Sanity", MB_OK | MB_ICONINFORMATION);
}

/**
 * @brief Browses for a browser executable
 */
static void BrowseForBrowser(HWND hwnd) {
    char buffer[MAX_PATH] = {0};
    OPENFILENAME ofn;
    
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "Executable Files (*.exe)\0*.exe\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = buffer;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "exe";
    
    if (GetOpenFileName(&ofn)) {
        SetWindowText(GetDlgItem(hwnd, IDC_CUSTOM_BROWSER_PATH), buffer);
    }
}

/**
 * @brief Installs the application
 */
static void InstallApp(HWND hwnd) {
    // Save the settings first
    SaveSettings(hwnd);
    
    // Check if we're already running from the installation directory
    if (IsRunningFromInstallDir()) {
        MessageBox(hwnd, "The application is already installed.", "Browser Sanity", MB_OK | MB_ICONINFORMATION);
        return;
    }
    
    // Ask for confirmation
    if (MessageBox(hwnd, "This will install Browser Sanity on your system. Continue?", 
                  "Browser Sanity", MB_YESNO | MB_ICONQUESTION) != IDYES) {
        return;
    }
    
    // Install the application
    if (InstallApplication()) {
        MessageBox(hwnd, "Browser Sanity has been installed successfully.", 
                  "Browser Sanity", MB_OK | MB_ICONINFORMATION);
        
        // Launch the installed application
        LaunchInstalledApplication();
        
        // Close this instance
        PostMessage(hwnd, WM_CLOSE, 0, 0);
    } else {
        MessageBox(hwnd, "Failed to install Browser Sanity.", 
                  "Browser Sanity", MB_OK | MB_ICONERROR);
    }
}

/**
 * @brief Uninstalls the application
 */
static void UninstallApp(HWND hwnd) {
    // Ask for confirmation
    if (MessageBox(hwnd, "This will uninstall Browser Sanity from your system. Continue?", 
                  "Browser Sanity", MB_YESNO | MB_ICONQUESTION) != IDYES) {
        return;
    }
    
    // Uninstall the application
    if (UninstallApplication()) {
        MessageBox(hwnd, "Browser Sanity has been uninstalled successfully.", 
                  "Browser Sanity", MB_OK | MB_ICONINFORMATION);
        
        // Close this instance
        PostMessage(hwnd, WM_CLOSE, 0, 0);
    } else {
        MessageBox(hwnd, "Failed to uninstall Browser Sanity.",
                  "Browser Sanity", MB_OK | MB_ICONERROR);
    }
}

/**
 * @brief Shows the "already installed" dialog with appropriate options
 */
static int ShowAlreadyInstalledDialog(HWND hwnd) {
    char message[512];
    BOOL isRunning = IsProcessRunning();
    
    sprintf(message,
        "Browser Sanity is already installed on this system.\r\n\r\n"
        "What would you like to do?\r\n\r\n"
        "• Show Settings - Open the configuration window\r\n"
        "• Launch - Start Browser Sanity %s\r\n"
        "• Uninstall - Remove Browser Sanity from your system\r\n"
        "• Cancel - Exit without changes",
        isRunning ? "(already running)" : ""
    );
    
    // Use a custom message box with multiple buttons
    int result = MessageBox(hwnd, message, "Browser Sanity - Already Installed",
                           MB_ICONINFORMATION | MB_YESNOCANCEL);
    
    switch (result) {
        case IDYES:
            // Create a second dialog to choose between Launch and Settings
            if (isRunning) {
                return IDOK; // Show settings if already running
            } else {
                int choice = MessageBox(hwnd,
                    "Choose an action:\r\n\r\n"
                    "Yes - Launch Browser Sanity\r\n"
                    "No - Show Settings\r\n"
                    "Cancel - Exit",
                    "Browser Sanity", MB_YESNOCANCEL | MB_ICONQUESTION);
                
                if (choice == IDYES) {
                    return IDYES; // Launch
                } else if (choice == IDNO) {
                    return IDOK; // Show Settings
                } else {
                    return IDCANCEL; // Cancel
                }
            }
        case IDNO:
            return IDNO; // Uninstall
        case IDCANCEL:
        default:
            return IDCANCEL; // Cancel
    }
}