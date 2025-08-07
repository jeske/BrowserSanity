/**
 * @file dialog_status.c
 * @brief Enhanced status dialog implementation
 */

#include "../../../include/browser_sanity.h"
#include "../../../include/resource.h"
#include <windows.h>
#include <commctrl.h>

#define IDD_STATUS_DIALOG 2000
#define IDC_STATUS_ICON 2001
#define IDC_STATUS_TEXT 2002
#define IDC_SHOW_SETTINGS_BTN 2003
#define IDC_EXIT_BTN 2004
#define IDC_RESTART_BTN 2005

// Global variables for dialog
static char g_statusMessage[1024];
static BOOL g_isRunning = FALSE;
static BOOL g_isInstalled = FALSE;

// Custom window procedure that handles dialog messages properly
static LRESULT CALLBACK DialogWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDYES: // Show Settings button
                    // TODO: Implement communication with running app
                    ExitProcess(0); // For now, just exit
                    return 0;
                case IDNO: // Exit button
                    ExitProcess(0);
                    return 0;
            }
            break;
            
        case WM_CLOSE:
            ExitProcess(0); // Exit the entire process when close button is clicked
            return 0;
            
        case WM_DESTROY:
            ExitProcess(0); // Cleanup and exit
            return 0;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// Dialog procedure
static INT_PTR CALLBACK StatusDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_INITDIALOG:
        {
            // Set the application icon
            HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPLICATION));
            if (!hIcon) {
                hIcon = LoadIcon(NULL, IDI_APPLICATION);
            }
            SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
            SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
            
            // Set icon in the dialog
            SendMessage(GetDlgItem(hDlg, IDC_STATUS_ICON), STM_SETICON, (WPARAM)hIcon, 0);
            
            // Set the status text
            SetDlgItemText(hDlg, IDC_STATUS_TEXT, g_statusMessage);
            
            // Update button text based on status
            if (g_isRunning) {
                SetDlgItemText(hDlg, IDC_RESTART_BTN, "Restart Browser Sanity");
            } else if (g_isInstalled) {
                SetDlgItemText(hDlg, IDC_RESTART_BTN, "Launch Browser Sanity");
            } else {
                SetDlgItemText(hDlg, IDC_RESTART_BTN, "Install Browser Sanity");
            }
            
            // Center the dialog
            RECT rcParent, rcDialog;
            GetWindowRect(GetDesktopWindow(), &rcParent);
            GetWindowRect(hDlg, &rcDialog);
            
            int x = (rcParent.right - rcParent.left - (rcDialog.right - rcDialog.left)) / 2;
            int y = (rcParent.bottom - rcParent.top - (rcDialog.bottom - rcDialog.top)) / 2;
            
            SetWindowPos(hDlg, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
            
            return TRUE;
        }
        
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDC_SHOW_SETTINGS_BTN:
                    EndDialog(hDlg, IDYES);
                    return TRUE;
                    
                case IDC_EXIT_BTN:
                    EndDialog(hDlg, IDNO);
                    return TRUE;
                    
                case IDC_RESTART_BTN:
                    EndDialog(hDlg, IDCANCEL);
                    return TRUE;
                    
                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    ExitProcess(0);
                    return TRUE;
            }
            break;
            
        case WM_CLOSE:
            EndDialog(hDlg, IDCANCEL);
            ExitProcess(0);
            return TRUE;
    }
    
    return FALSE;
}

/**
 * @brief Shows the enhanced status dialog
 * @param hwndParent Parent window handle
 * @param isRunning Whether Browser Sanity is currently running
 * @param isInstalled Whether Browser Sanity is installed
 * @param runningPID Process ID if running
 * @return Dialog result (IDYES = Show Settings, IDNO = Exit, IDCANCEL = Restart/Launch/Install)
 */
int ShowEnhancedStatusDialog(HWND hwndParent, BOOL isRunning, BOOL isInstalled, DWORD runningPID) {
    AppConfig config;
    
    // Save state
    g_isRunning = isRunning;
    g_isInstalled = isInstalled;
    
    // Read current configuration
    ReadAppConfig(&config);
    
    // Build enhanced status message
    if (isRunning && isInstalled) {
        sprintf(g_statusMessage,
            "Browser Sanity is currently running and installed.\n\n"
            "Installation Path: %s\n"
            "Status: Active (PID: %lu)\n"
            "Version: %s\n\n"
            "What would you like to do?",
            config.installPath, runningPID, config.version);
    }
    else if (isInstalled && !isRunning) {
        sprintf(g_statusMessage,
            "Browser Sanity is installed but not currently running.\n\n"
            "Installation Path: %s\n"
            "Status: Installed (Not Running)\n"
            "Version: %s\n\n"
            "What would you like to do?",
            config.installPath, config.version);
    }
    else if (isRunning && !isInstalled) {
        sprintf(g_statusMessage,
            "Browser Sanity is currently running but not properly installed.\n\n"
            "Status: Running (PID: %lu) - Portable Mode\n"
            "Installation Status: Not Installed\n\n"
            "What would you like to do?",
            runningPID);
    }
    else {
        sprintf(g_statusMessage,
            "Browser Sanity status is unclear.\n\n"
            "Running: %s\n"
            "Installed: %s\n\n"
            "What would you like to do?",
            isRunning ? "Yes" : "No",
            isInstalled ? "Yes" : "No");
    }
    
    // Create dialog template in memory (2x wider than normal)
    typedef struct {
        DLGTEMPLATE dlgTemplate;
        WORD menu;
        WORD wclass;
        WCHAR title[32];
        WORD pointSize;
        WCHAR fontName[16];
    } DialogTemplate;
    
    typedef struct {
        DLGITEMTEMPLATE itemTemplate;
        WORD wclass;
        WORD title;
        WORD extraData;
    } ItemTemplate;
    
    // For simplicity, let's use a MessageBox-style approach but with custom buttons
    // We'll create this using Windows' built-in dialog creation
    
    // Register a window class for our dialog
    static BOOL classRegistered = FALSE;
    if (!classRegistered) {
        WNDCLASS wc = {0};
        wc.lpfnWndProc = DialogWndProc;  // Use our custom procedure
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = "BrowserSanityStatusDialog";
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        
        RegisterClass(&wc);
        classRegistered = TRUE;
    }
    
    // Calculate minimum dialog dimensions based on actual control layout
    HDC hdc = GetDC(NULL);
    HFONT hTempFont = CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    HFONT hOldFont = SelectObject(hdc, hTempFont);
    
    // Measure the main text width
    SIZE textSize;
    char mainText[] = "Browser Sanity is currently running and installed.";
    GetTextExtentPoint32(hdc, mainText, strlen(mainText), &textSize);
    
    // Measure longest possible data field
    char samplePath[] = "C:\\Program Files (x86)\\Browser Sanity\\BrowserSanity.exe";
    SIZE pathSize;
    GetTextExtentPoint32(hdc, samplePath, strlen(samplePath), &pathSize);
    
    SelectObject(hdc, hOldFont);
    DeleteObject(hTempFont);
    ReleaseDC(NULL, hdc);
    
    // Calculate width based on layout: icon(32) + gap(20) + content_area + margin(30)
    // Content area needs: label_width(100) + gap(10) + value_width + margin(30)
    int contentWidth = 100 + 10 + max(textSize.cx, pathSize.cx) + 30;
    int totalContentWidth = 32 + 20 + contentWidth;
    
    // Button area needs: button1(120) + gap(10) + button2(80) + margins(40)
    int buttonAreaWidth = 120 + 10 + 80 + 40;
    
    // Use the larger of content width or button width, with minimum
    int width = max(max(totalContentWidth, buttonAreaWidth), 520);
    
    // Make dialog taller to ensure buttons are visible
    // Calculate height: top_margin(15) + main_text(20) + gap(10) + 3_data_rows(3*20) + gap(20) + buttons(30) + bottom_margin(20)
    int height = 15 + 20 + 10 + (3 * 20) + 20 + 30 + 20; // = 195 pixels
    
    int x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
    
    HWND hDlg = CreateWindowEx(
        WS_EX_DLGMODALFRAME,
        "BrowserSanityStatusDialog",
        "Browser Sanity Status",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        x, y, width, height,
        hwndParent, NULL, GetModuleHandle(NULL), NULL
    );
    
    if (!hDlg) return IDCANCEL;
    
    // Load and display the actual Browser Sanity application icon
    HICON hAppIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APP_ICON));
    if (!hAppIcon) {
        // Fallback to system icon if app icon fails to load
        hAppIcon = LoadIcon(NULL, IDI_APPLICATION);
    }
    
    HWND hIcon = CreateWindow("STATIC", NULL, WS_CHILD | WS_VISIBLE | SS_ICON,
        20, 20, 32, 32, hDlg, NULL, GetModuleHandle(NULL), NULL);
    SendMessage(hIcon, STM_SETICON, (WPARAM)hAppIcon, 0);
    SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hAppIcon);
    SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hAppIcon);
    
    // Create main message text with better positioning
    HWND hMainText = CreateWindow("STATIC", "Browser Sanity is currently running and installed.",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        70, 20, width - 100, 20, hDlg, NULL, GetModuleHandle(NULL), NULL);
    
    // Create status details with aligned layout
    HWND hInstallPath = CreateWindow("STATIC", "Installation Path:", WS_CHILD | WS_VISIBLE | SS_LEFT,
        70, 45, 100, 15, hDlg, NULL, GetModuleHandle(NULL), NULL);
    HWND hInstallValue = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE | SS_LEFT,
        180, 45, width - 210, 15, hDlg, NULL, GetModuleHandle(NULL), NULL);
        
    HWND hStatusLabel = CreateWindow("STATIC", "Status:", WS_CHILD | WS_VISIBLE | SS_LEFT,
        70, 62, 100, 15, hDlg, NULL, GetModuleHandle(NULL), NULL);
    HWND hStatusValue = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE | SS_LEFT,
        180, 62, width - 210, 15, hDlg, NULL, GetModuleHandle(NULL), NULL);
        
    HWND hVersionLabel = CreateWindow("STATIC", "Version:", WS_CHILD | WS_VISIBLE | SS_LEFT,
        70, 79, 100, 15, hDlg, NULL, GetModuleHandle(NULL), NULL);
    HWND hVersionValue = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE | SS_LEFT,
        180, 79, width - 210, 15, hDlg, NULL, GetModuleHandle(NULL), NULL);
    
    // Create only the buttons we need (removed Restart button)
    HWND hShowSettings = CreateWindow("BUTTON", "Show Settings", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        width - 210, height - 45, 120, 25, hDlg, (HMENU)IDYES, GetModuleHandle(NULL), NULL);
        
    HWND hExit = CreateWindow("BUTTON", "Exit", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        width - 85, height - 45, 70, 25, hDlg, (HMENU)IDNO, GetModuleHandle(NULL), NULL);
    
    // Set fonts - different sizes for headers vs content
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
    SendMessage(hInstallPath, WM_SETFONT, (WPARAM)hLabelFont, TRUE);
    SendMessage(hStatusLabel, WM_SETFONT, (WPARAM)hLabelFont, TRUE);
    SendMessage(hVersionLabel, WM_SETFONT, (WPARAM)hLabelFont, TRUE);
    SendMessage(hInstallValue, WM_SETFONT, (WPARAM)hValueFont, TRUE);
    SendMessage(hStatusValue, WM_SETFONT, (WPARAM)hValueFont, TRUE);
    SendMessage(hVersionValue, WM_SETFONT, (WPARAM)hValueFont, TRUE);
    SendMessage(hShowSettings, WM_SETFONT, (WPARAM)hValueFont, TRUE);
    SendMessage(hExit, WM_SETFONT, (WPARAM)hValueFont, TRUE);
    
    // Fill in the actual status data
    ReadAppConfig(&config);
    SetWindowText(hInstallValue, config.installPath);
    
    char statusText[100];
    if (g_isRunning) {
        DWORD pid = 0;
        IsProcessRunningWithPID(&pid);
        sprintf(statusText, "Active (PID: %lu)", pid);
    } else {
        strcpy(statusText, "Installed (Not Running)");
    }
    SetWindowText(hStatusValue, statusText);
    SetWindowText(hVersionValue, config.version);
    
    // Modal dialog loop - DefWindowProc handles close/drag/activate automatically
    int result = IDCANCEL;
    BOOL active = TRUE;
    MSG msg;
    
    if (hwndParent) EnableWindow(hwndParent, FALSE);
    
    while (active) {
        if (GetMessage(&msg, NULL, 0, 0)) {
            // Handle messages for our dialog window and children
            if (msg.hwnd == hDlg || IsChild(hDlg, msg.hwnd)) {
                // The window procedure now handles all button clicks and close events directly
                // so we just need to dispatch messages normally
            }
            
            if (active) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        } else {
            // GetMessage returned 0 (WM_QUIT) or -1 (error)
            result = IDNO;
            active = FALSE;
        }
    }
    
    if (hwndParent) {
        EnableWindow(hwndParent, TRUE);
        SetForegroundWindow(hwndParent);
    }
    
    // Cleanup fonts
    if (hMainFont) DeleteObject(hMainFont);
    if (hLabelFont) DeleteObject(hLabelFont);
    if (hValueFont) DeleteObject(hValueFont);
    
    DestroyWindow(hDlg);
    
    // Handle the results - Exit button or close should exit this process
    if (result == IDNO || result == IDCANCEL) {
        ExitProcess(0);  // Exit this process immediately
    }
    
    return result;
}