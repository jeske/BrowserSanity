/**
 * @file dialog_manualLaunchInstall.c
 * @brief Installation prompt dialog - shown when BrowserSanity.exe is not found/installed
 * 
 * This dialog replaces the standard yes/no installation prompt with a rich information
 * dialog that explains the application's purpose and provides installation options.
 */

#include "../../../include/browser_sanity.h"
#include "../../../include/resource.h"
#include <windows.h>
#include <shellapi.h>

// Installation info dialog window procedure
static LRESULT CALLBACK InstallInfoWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDYES: // Install button
                    // Hide this dialog and start install progress
                    ShowWindow(hWnd, SW_HIDE);
                    ShowInstallUninstallProgressDialog(hWnd, TRUE); // TRUE = install
                    return 0;
                case IDNO: // Cancel button
                    ExitProcess(0);
                    return 0;
                case 1001: // GitHub link clicked
                    ShellExecute(NULL, "open", "https://github.com/jeske/BrowserSanity", NULL, NULL, SW_SHOWNORMAL);
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
 * @brief Shows the installation information dialog when Browser Sanity is not found
 * @param hwndParent Parent window handle
 * @return Dialog result (IDYES = Install, IDNO = Cancel)
 */
int ShowInstallationInfoDialog(HWND hwndParent) {
    // Register window class
    static BOOL classRegistered = FALSE;
    if (!classRegistered) {
        WNDCLASS wc = {0};
        wc.lpfnWndProc = InstallInfoWndProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = "BrowserSanityInstallInfo";
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APP_ICON));
        
        RegisterClass(&wc);
        classRegistered = TRUE;
    }
    
    // Calculate dialog dimensions
    int width = 550;
    int height = 450;
    int x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
    
    // Create dialog window
    HWND hDlg = CreateWindowEx(
        WS_EX_DLGMODALFRAME,
        "BrowserSanityInstallInfo",
        "Browser Sanity - Installation",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        x, y, width, height,
        hwndParent, NULL, GetModuleHandle(NULL), NULL
    );
    
    if (!hDlg) return IDNO;
    
    // Get client area
    RECT clientRect;
    GetClientRect(hDlg, &clientRect);
    int clientWidth = clientRect.right - clientRect.left;
    int clientHeight = clientRect.bottom - clientRect.top;
    
    // Load application icon
    HICON hAppIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APP_ICON));
    if (!hAppIcon) hAppIcon = LoadIcon(NULL, IDI_APPLICATION);
    
    // Create large icon display
    HWND hIcon = CreateWindow("STATIC", NULL, WS_CHILD | WS_VISIBLE | SS_ICON,
        30, 30, 48, 48, hDlg, NULL, GetModuleHandle(NULL), NULL);
    SendMessage(hIcon, STM_SETICON, (WPARAM)hAppIcon, 0);
    SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hAppIcon);
    SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hAppIcon);
    
    // Create title
    HWND hTitle = CreateWindow("STATIC", "Browser Sanity",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        90, 30, clientWidth - 120, 25, hDlg, NULL, GetModuleHandle(NULL), NULL);
    
    // Create version and build info
    char versionText[128];
    sprintf(versionText, "Version %s - Built %s", BROWSER_SANITY_VERSION, __DATE__);
    HWND hVersion = CreateWindow("STATIC", versionText,
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        90, 55, clientWidth - 120, 15, hDlg, NULL, GetModuleHandle(NULL), NULL);
    
    // Create GitHub link (moved above info box)
    HWND hGitHubLink = CreateWindow("BUTTON", "View on GitHub",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        30, 100, 120, 25, hDlg, (HMENU)1001, GetModuleHandle(NULL), NULL);
    
    // Create credits (moved above info box)
    HWND hCredits = CreateWindow("STATIC", "Created by David W. Jeske (with Claude Sonnet 4 in Kilo Code)",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        160, 105, 300, 15, hDlg, NULL, GetModuleHandle(NULL), NULL);
    
    // Create description area (fixed bullet points with plain text)
    const char* description =
        "Browser Sanity ensures your browser choice preferences are respected by intercepting Microsoft Edge launches and redirecting them to your preferred browser.\r\n\r\n"
        "Key Features:\r\n"
        "- Automatically redirects Edge launches to your chosen browser\r\n"
        "- Maintains browser choice freedom on Windows systems\r\n"
        "- Lightweight background monitoring with minimal resource usage\r\n"
        "- Configurable settings and exception handling\r\n"
        "- Respects user preferences without system modifications\r\n\r\n"
        "Browser Sanity runs quietly in the background, only activating when Microsoft Edge attempts to launch, ensuring your preferred browser opens instead.";
    
    HWND hDescription = CreateWindow("EDIT", description,
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL,
        30, 130, clientWidth - 60, clientHeight - 210,
        hDlg, NULL, GetModuleHandle(NULL), NULL);
    
    // Set white background for description
    SetClassLongPtr(hDescription, GCLP_HBRBACKGROUND, (LONG_PTR)GetStockObject(WHITE_BRUSH));
    
    // Create buttons (Exit on left, Install on right)
    HWND hExit = CreateWindow("BUTTON", "Exit",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        30, clientHeight - 45, 70, 30, hDlg, (HMENU)IDNO, GetModuleHandle(NULL), NULL);
        
    HWND hInstall = CreateWindow("BUTTON", "Install Browser Sanity",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_DEFPUSHBUTTON,
        clientWidth - 170, clientHeight - 45, 140, 30, hDlg, (HMENU)IDYES, GetModuleHandle(NULL), NULL);
    
    // Set fonts
    HFONT hTitleFont = CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
        
    HFONT hVersionFont = CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
        
    HFONT hDescFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
        
    HFONT hButtonFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    
    SendMessage(hTitle, WM_SETFONT, (WPARAM)hTitleFont, TRUE);
    SendMessage(hVersion, WM_SETFONT, (WPARAM)hVersionFont, TRUE);
    SendMessage(hDescription, WM_SETFONT, (WPARAM)hDescFont, TRUE);
    SendMessage(hCredits, WM_SETFONT, (WPARAM)hVersionFont, TRUE);
    SendMessage(hGitHubLink, WM_SETFONT, (WPARAM)hButtonFont, TRUE);
    SendMessage(hInstall, WM_SETFONT, (WPARAM)hButtonFont, TRUE);
    SendMessage(hExit, WM_SETFONT, (WPARAM)hButtonFont, TRUE);
    
    // Set white backgrounds for all controls
    SetClassLongPtr(hTitle, GCLP_HBRBACKGROUND, (LONG_PTR)GetStockObject(WHITE_BRUSH));
    SetClassLongPtr(hVersion, GCLP_HBRBACKGROUND, (LONG_PTR)GetStockObject(WHITE_BRUSH));
    SetClassLongPtr(hCredits, GCLP_HBRBACKGROUND, (LONG_PTR)GetStockObject(WHITE_BRUSH));
    
    // Set focus to install button
    SetFocus(hInstall);
    
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
    if (hTitleFont) DeleteObject(hTitleFont);
    if (hVersionFont) DeleteObject(hVersionFont);
    if (hDescFont) DeleteObject(hDescFont);
    if (hButtonFont) DeleteObject(hButtonFont);
    
    DestroyWindow(hDlg);
    
    return IDNO; // This will never be reached due to ExitProcess calls
}