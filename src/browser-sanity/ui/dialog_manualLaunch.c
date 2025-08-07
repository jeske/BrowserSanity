/**
 * @file dialog_manualLaunch.c
 * @brief Manual launch dialog - shown when BrowserSanity.exe is run directly
 */

#include "../../../include/browser_sanity.h"
#include "../../../include/resource.h"
#include <windows.h>

// Manual launch dialog window procedure
static LRESULT CALLBACK ManualLaunchWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDYES: // Show Settings button
                    // TODO: Launch settings dialog and then exit
                    ExitProcess(0);
                    return 0;
                case IDCANCEL: // Uninstall button
                    // TODO: Launch uninstall process and then exit
                    ExitProcess(0);
                    return 0;
                case IDNO: // Exit button
                    ExitProcess(0);
                    return 0;
            }
            break;
            
        case WM_CLOSE:
            ExitProcess(0);
            return 0;
            
        case WM_DESTROY:
            ExitProcess(0);
            return 0;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/**
 * @brief Shows the manual launch dialog when BrowserSanity.exe is run directly
 * @param hwndParent Parent window handle
 * @param isRunning Whether Browser Sanity is currently running
 * @param isInstalled Whether Browser Sanity is installed
 * @param runningPID Process ID if running
 * @return Dialog result (IDYES = Show Settings, IDNO = Exit)
 */
int ShowManualLaunchDialog(HWND hwndParent, BOOL isRunning, BOOL isInstalled, DWORD runningPID) {
    AppConfig config;
    ReadAppConfig(&config);
    
    // Register window class
    static BOOL classRegistered = FALSE;
    if (!classRegistered) {
        WNDCLASS wc = {0};
        wc.lpfnWndProc = ManualLaunchWndProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = "BrowserSanityManualLaunch";
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APP_ICON));
        
        RegisterClass(&wc);
        classRegistered = TRUE;
    }
    
    // Calculate dialog dimensions
    int width = 520;
    int height = 280;
    int x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
    
    // Create dialog window
    HWND hDlg = CreateWindowEx(
        WS_EX_DLGMODALFRAME,
        "BrowserSanityManualLaunch",
        "Browser Sanity Status",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        x, y, width, height,
        hwndParent, NULL, GetModuleHandle(NULL), NULL
    );
    
    if (!hDlg) return IDNO;
    
    // Load application icon
    HICON hAppIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APP_ICON));
    if (!hAppIcon) hAppIcon = LoadIcon(NULL, IDI_APPLICATION);
    
    // Create icon control
    HWND hIcon = CreateWindow("STATIC", NULL, WS_CHILD | WS_VISIBLE | SS_ICON,
        20, 20, 32, 32, hDlg, NULL, GetModuleHandle(NULL), NULL);
    SendMessage(hIcon, STM_SETICON, (WPARAM)hAppIcon, 0);
    SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hAppIcon);
    SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hAppIcon);
    
    // Create main message
    char mainMessage[256];
    if (isRunning && isInstalled) {
        strcpy(mainMessage, "Browser Sanity is currently running and installed.");
    } else if (isInstalled && !isRunning) {
        strcpy(mainMessage, "Browser Sanity is installed but not currently running.");
    } else if (isRunning && !isInstalled) {
        strcpy(mainMessage, "Browser Sanity is running in portable mode.");
    } else {
        strcpy(mainMessage, "Browser Sanity status is unclear.");
    }
    
    HWND hMainText = CreateWindow("STATIC", mainMessage,
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        70, 20, width - 100, 20, hDlg, NULL, GetModuleHandle(NULL), NULL);
    
    // Create status labels and values
    HWND hInstallLabel = CreateWindow("STATIC", "Installation Path:", WS_CHILD | WS_VISIBLE | SS_LEFT,
        70, 50, 120, 15, hDlg, NULL, GetModuleHandle(NULL), NULL);
    HWND hInstallValue = CreateWindow("STATIC", config.installPath, WS_CHILD | WS_VISIBLE | SS_LEFT,
        200, 50, width - 230, 15, hDlg, NULL, GetModuleHandle(NULL), NULL);
        
    HWND hStatusLabel = CreateWindow("STATIC", "Status:", WS_CHILD | WS_VISIBLE | SS_LEFT,
        70, 70, 120, 15, hDlg, NULL, GetModuleHandle(NULL), NULL);
    
    char statusText[100];
    if (isRunning) {
        sprintf(statusText, "Active (PID: %lu)", runningPID);
    } else {
        strcpy(statusText, "Not Running");
    }
    HWND hStatusValue = CreateWindow("STATIC", statusText, WS_CHILD | WS_VISIBLE | SS_LEFT,
        200, 70, width - 230, 15, hDlg, NULL, GetModuleHandle(NULL), NULL);
        
    HWND hVersionLabel = CreateWindow("STATIC", "Version:", WS_CHILD | WS_VISIBLE | SS_LEFT,
        70, 90, 120, 15, hDlg, NULL, GetModuleHandle(NULL), NULL);
    HWND hVersionValue = CreateWindow("STATIC", config.version, WS_CHILD | WS_VISIBLE | SS_LEFT,
        200, 90, width - 230, 15, hDlg, NULL, GetModuleHandle(NULL), NULL);
    
    // Get actual client area dimensions
    RECT clientRect;
    GetClientRect(hDlg, &clientRect);
    int clientWidth = clientRect.right - clientRect.left;
    int clientHeight = clientRect.bottom - clientRect.top;
    
    // Create buttons (positioned from right to left in lower right of client area)
    HWND hExit = CreateWindow("BUTTON", "Exit", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        clientWidth - 80, clientHeight - 40, 70, 25, hDlg, (HMENU)IDNO, GetModuleHandle(NULL), NULL);
        
    HWND hUninstall = CreateWindow("BUTTON", "Uninstall", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        clientWidth - 160, clientHeight - 40, 75, 25, hDlg, (HMENU)IDCANCEL, GetModuleHandle(NULL), NULL);
        
    HWND hShowSettings = CreateWindow("BUTTON", "Show Settings", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        clientWidth - 260, clientHeight - 40, 95, 25, hDlg, (HMENU)IDYES, GetModuleHandle(NULL), NULL);
    
    // Set fonts
    HFONT hMainFont = CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
        
    HFONT hLabelFont = CreateFont(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
        
    HFONT hValueFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    
    SendMessage(hMainText, WM_SETFONT, (WPARAM)hMainFont, TRUE);
    SendMessage(hInstallLabel, WM_SETFONT, (WPARAM)hLabelFont, TRUE);
    SendMessage(hStatusLabel, WM_SETFONT, (WPARAM)hLabelFont, TRUE);
    SendMessage(hVersionLabel, WM_SETFONT, (WPARAM)hLabelFont, TRUE);
    SendMessage(hInstallValue, WM_SETFONT, (WPARAM)hValueFont, TRUE);
    SendMessage(hStatusValue, WM_SETFONT, (WPARAM)hValueFont, TRUE);
    SendMessage(hVersionValue, WM_SETFONT, (WPARAM)hValueFont, TRUE);
    SendMessage(hShowSettings, WM_SETFONT, (WPARAM)hValueFont, TRUE);
    SendMessage(hUninstall, WM_SETFONT, (WPARAM)hValueFont, TRUE);
    SendMessage(hExit, WM_SETFONT, (WPARAM)hValueFont, TRUE);
    
    // Modal message loop
    MSG msg;
    if (hwndParent) EnableWindow(hwndParent, FALSE);
    
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    if (hwndParent) {
        EnableWindow(hwndParent, TRUE);
        SetForegroundWindow(hwndParent);
    }
    
    // Cleanup
    if (hMainFont) DeleteObject(hMainFont);
    if (hLabelFont) DeleteObject(hLabelFont);
    if (hValueFont) DeleteObject(hValueFont);
    
    DestroyWindow(hDlg);
    
    return IDNO; // This will never be reached due to ExitProcess calls
}