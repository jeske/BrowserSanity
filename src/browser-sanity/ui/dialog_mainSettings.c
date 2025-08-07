/**
 * @file dialog_mainSettings.c
 * @brief Main settings configuration dialog for Browser Sanity
 */

#include "../../../include/browser_sanity.h"
#include "../../../include/resource.h"
#include <windows.h>
#include <commctrl.h>

// Control IDs for the settings dialog
#define IDC_BROWSER_COMBO        3001
#define IDC_STARTUP_CHECK        3002
#define IDC_EXCEPTION_LIST       3003
#define IDC_ADD_EXCEPTION        3004
#define IDC_REMOVE_EXCEPTION     3005
#define IDC_OK_BTN              3006
#define IDC_CANCEL_BTN          3007
#define IDC_APPLY_BTN           3008

// Global settings dialog state
static AppConfig g_tempConfig;
static BOOL g_settingsChanged = FALSE;

// Main settings dialog window procedure
static LRESULT CALLBACK MainSettingsWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDC_OK_BTN:
                    // Save settings and close
                    if (g_settingsChanged) {
                        WriteAppConfig(&g_tempConfig);
                    }
                    DestroyWindow(hWnd);
                    return 0;
                    
                case IDC_CANCEL_BTN:
                    // Close without saving
                    DestroyWindow(hWnd);
                    return 0;
                    
                case IDC_APPLY_BTN:
                    // Apply settings without closing
                    if (g_settingsChanged) {
                        WriteAppConfig(&g_tempConfig);
                        g_settingsChanged = FALSE;
                        EnableWindow(GetDlgItem(hWnd, IDC_APPLY_BTN), FALSE);
                    }
                    return 0;
                    
                case IDC_BROWSER_COMBO:
                    if (HIWORD(wParam) == CBN_SELCHANGE) {
                        // Browser selection changed
                        g_settingsChanged = TRUE;
                        EnableWindow(GetDlgItem(hWnd, IDC_APPLY_BTN), TRUE);
                    }
                    return 0;
                    
                case IDC_STARTUP_CHECK:
                    // Startup checkbox toggled
                    g_settingsChanged = TRUE;
                    EnableWindow(GetDlgItem(hWnd, IDC_APPLY_BTN), TRUE);
                    return 0;
                    
                case IDC_ADD_EXCEPTION:
                    // TODO: Show dialog to add exception
                    return 0;
                    
                case IDC_REMOVE_EXCEPTION:
                    // TODO: Remove selected exception
                    return 0;
            }
            break;
            
        case WM_CLOSE:
            DestroyWindow(hWnd);
            return 0;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/**
 * @brief Shows the main settings configuration dialog
 * @param hwndParent Parent window handle
 * @return Dialog result (IDOK if settings were saved, IDCANCEL if cancelled)
 */
int ShowMainSettingsDialog(HWND hwndParent) {
    // Load current configuration
    ReadAppConfig(&g_tempConfig);
    g_settingsChanged = FALSE;
    
    // Register window class
    static BOOL classRegistered = FALSE;
    if (!classRegistered) {
        WNDCLASS wc = {0};
        wc.lpfnWndProc = MainSettingsWndProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = "BrowserSanityMainSettings";
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APP_ICON));
        
        RegisterClass(&wc);
        classRegistered = TRUE;
    }
    
    // Calculate dialog dimensions
    int width = 600;
    int height = 500;
    int x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
    
    // Create settings dialog window
    HWND hDlg = CreateWindowEx(
        WS_EX_DLGMODALFRAME,
        "BrowserSanityMainSettings",
        "Browser Sanity Settings",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        x, y, width, height,
        hwndParent, NULL, GetModuleHandle(NULL), NULL
    );
    
    if (!hDlg) return IDCANCEL;
    
    // Set application icon
    HICON hAppIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APP_ICON));
    if (!hAppIcon) hAppIcon = LoadIcon(NULL, IDI_APPLICATION);
    SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hAppIcon);
    SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hAppIcon);
    
    // Create main settings sections
    
    // Browser Selection Group
    HWND hBrowserGroup = CreateWindow("BUTTON", "Default Browser", 
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        20, 20, width - 60, 80, hDlg, NULL, GetModuleHandle(NULL), NULL);
        
    HWND hBrowserLabel = CreateWindow("STATIC", "Redirect Edge to:", 
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        40, 50, 100, 20, hDlg, NULL, GetModuleHandle(NULL), NULL);
        
    HWND hBrowserCombo = CreateWindow("COMBOBOX", "", 
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        150, 48, 200, 200, hDlg, (HMENU)IDC_BROWSER_COMBO, GetModuleHandle(NULL), NULL);
    
    // Populate browser combo
    SendMessage(hBrowserCombo, CB_ADDSTRING, 0, (LPARAM)"Google Chrome");
    SendMessage(hBrowserCombo, CB_ADDSTRING, 0, (LPARAM)"Mozilla Firefox");
    SendMessage(hBrowserCombo, CB_ADDSTRING, 0, (LPARAM)"Brave Browser");
    SendMessage(hBrowserCombo, CB_ADDSTRING, 0, (LPARAM)"Opera");
    SendMessage(hBrowserCombo, CB_SETCURSEL, 0, 0); // Default to Chrome
    
    // Startup Options Group
    HWND hStartupGroup = CreateWindow("BUTTON", "Startup Options", 
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        20, 120, width - 60, 80, hDlg, NULL, GetModuleHandle(NULL), NULL);
        
    HWND hStartupCheck = CreateWindow("BUTTON", "Start Browser Sanity with Windows", 
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        40, 150, 250, 20, hDlg, (HMENU)IDC_STARTUP_CHECK, GetModuleHandle(NULL), NULL);
    
    // Exception List Group
    HWND hExceptionGroup = CreateWindow("BUTTON", "Site Exceptions", 
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        20, 220, width - 60, 200, hDlg, NULL, GetModuleHandle(NULL), NULL);
        
    HWND hExceptionLabel = CreateWindow("STATIC", "Sites that should open in Edge:", 
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        40, 250, 200, 20, hDlg, NULL, GetModuleHandle(NULL), NULL);
        
    HWND hExceptionList = CreateWindow("LISTBOX", "", 
        WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_STANDARD,
        40, 275, 350, 100, hDlg, (HMENU)IDC_EXCEPTION_LIST, GetModuleHandle(NULL), NULL);
        
    HWND hAddException = CreateWindow("BUTTON", "Add...", 
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        410, 275, 60, 25, hDlg, (HMENU)IDC_ADD_EXCEPTION, GetModuleHandle(NULL), NULL);
        
    HWND hRemoveException = CreateWindow("BUTTON", "Remove", 
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        410, 305, 60, 25, hDlg, (HMENU)IDC_REMOVE_EXCEPTION, GetModuleHandle(NULL), NULL);
    
    // Dialog buttons
    HWND hOkBtn = CreateWindow("BUTTON", "OK", 
        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        width - 250, height - 50, 70, 25, hDlg, (HMENU)IDC_OK_BTN, GetModuleHandle(NULL), NULL);
        
    HWND hCancelBtn = CreateWindow("BUTTON", "Cancel", 
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        width - 170, height - 50, 70, 25, hDlg, (HMENU)IDC_CANCEL_BTN, GetModuleHandle(NULL), NULL);
        
    HWND hApplyBtn = CreateWindow("BUTTON", "Apply", 
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        width - 90, height - 50, 70, 25, hDlg, (HMENU)IDC_APPLY_BTN, GetModuleHandle(NULL), NULL);
    
    // Initially disable Apply button
    EnableWindow(hApplyBtn, FALSE);
    
    // Set fonts for all controls
    HFONT hFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    
    // Apply font manually to key controls
    SendMessage(hBrowserLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hBrowserCombo, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hStartupCheck, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hExceptionLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hExceptionList, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hAddException, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hRemoveException, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hOkBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hCancelBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hApplyBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    // Modal message loop
    MSG msg;
    if (hwndParent) EnableWindow(hwndParent, FALSE);
    
    int result = IDCANCEL;
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_QUIT) {
            result = IDOK;
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    if (hwndParent) {
        EnableWindow(hwndParent, TRUE);
        SetForegroundWindow(hwndParent);
    }
    
    // Cleanup
    if (hFont) DeleteObject(hFont);
    
    return result;
}