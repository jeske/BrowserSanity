/**
 * @file dialog_installanduninstallprogress.c
 * @brief Unified install/uninstall progress dialog
 * 
 * DESIGN NOTE: This dialog handles both install and uninstall operations using
 * a unified progress tracking system. The same sequence of steps applies to both:
 * - File operations (copy/delete)
 * - Registry operations 
 * - Shortcut creation/removal
 * - Service start/stop
 * - Process launch/termination
 * 
 * The dialog uses the same UI and progress list, but with different handlers
 * and operation types (install vs uninstall) to avoid code duplication.
 */

#include "../../../include/browser_sanity.h"
#include "../../../include/resource.h"
#include <windows.h>

// Progress step structure
typedef struct {
    char description[128];
    BOOL completed;
    BOOL inProgress;
    BOOL failed;
} ProgressStep;

// Dialog data structure
typedef struct {
    BOOL isInstall;          // TRUE for install, FALSE for uninstall
    int currentStep;
    int totalSteps;
    ProgressStep* steps;
    HWND hListBox;
    HWND hProgressBar;
    HWND hStatusText;
    HWND hCancelButton;
    HWND hCloseButton;
} ProgressDialogData;

// Progress dialog window procedure
static LRESULT CALLBACK ProgressWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    ProgressDialogData* pData = (ProgressDialogData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    
    switch (uMsg) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDCANCEL: // Cancel button
                    // TODO: Implement cancellation logic
                    ExitProcess(0);
                    return 0;
                case IDOK: // Close button (shown when complete)
                    ExitProcess(0);
                    return 0;
            }
            break;
            
        case WM_CLOSE:
            ExitProcess(0);
            return 0;
            
        case WM_DESTROY:
            if (pData && pData->steps) {
                free(pData->steps);
            }
            ExitProcess(0);
            return 0;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/**
 * @brief Updates the progress display for a specific step
 * @param pData Dialog data structure
 * @param stepIndex Index of step to update
 */
static void UpdateProgressStep(ProgressDialogData* pData, int stepIndex) {
    if (!pData || stepIndex >= pData->totalSteps) return;
    
    ProgressStep* step = &pData->steps[stepIndex];
    char statusLine[256];
    
    if (step->failed) {
        sprintf(statusLine, "[FAILED] %s", step->description);
    } else if (step->completed) {
        sprintf(statusLine, "[DONE] %s", step->description);
    } else if (step->inProgress) {
        sprintf(statusLine, "[WORKING] %s", step->description);
    } else {
        sprintf(statusLine, "[PENDING] %s", step->description);
    }
    
    // Update listbox item
    SendMessage(pData->hListBox, LB_DELETESTRING, stepIndex, 0);
    SendMessage(pData->hListBox, LB_INSERTSTRING, stepIndex, (LPARAM)statusLine);
    
    // Update progress bar
    int progress = (pData->currentStep * 100) / pData->totalSteps;
    SendMessage(pData->hProgressBar, PBM_SETPOS, progress, 0);
    
    // Update status text
    if (step->inProgress) {
        SetWindowText(pData->hStatusText, step->description);
    }
}

/**
 * @brief Initializes progress steps for install or uninstall
 * @param isInstall TRUE for install, FALSE for uninstall
 * @param steps Output array of progress steps
 * @return Number of steps
 */
static int InitializeProgressSteps(BOOL isInstall, ProgressStep** steps) {
    const int stepCount = 7;
    *steps = (ProgressStep*)calloc(stepCount, sizeof(ProgressStep));
    
    if (isInstall) {
        strcpy((*steps)[0].description, "Extracting Microsoft Edge binary");
        strcpy((*steps)[1].description, "Creating installation directory");
        strcpy((*steps)[2].description, "Copying application files");
        strcpy((*steps)[3].description, "Creating desktop shortcut");
        strcpy((*steps)[4].description, "Updating registry settings");
        strcpy((*steps)[5].description, "Starting Browser Sanity service");
        strcpy((*steps)[6].description, "Launching Browser Sanity");
    } else {
        strcpy((*steps)[0].description, "Stopping Browser Sanity service");
        strcpy((*steps)[1].description, "Removing registry settings");
        strcpy((*steps)[2].description, "Removing desktop shortcut");
        strcpy((*steps)[3].description, "Removing application files");
        strcpy((*steps)[4].description, "Cleaning installation directory");
        strcpy((*steps)[5].description, "Removing extracted binaries");
        strcpy((*steps)[6].description, "Cleanup complete");
    }
    
    return stepCount;
}

/**
 * @brief Shows the install/uninstall progress dialog
 * @param hwndParent Parent window handle
 * @param isInstall TRUE for install, FALSE for uninstall
 * @return Dialog result
 */
int ShowInstallUninstallProgressDialog(HWND hwndParent, BOOL isInstall) {
    // Register window class
    static BOOL classRegistered = FALSE;
    if (!classRegistered) {
        WNDCLASS wc = {0};
        wc.lpfnWndProc = ProgressWndProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = "BrowserSanityProgress";
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APP_ICON));
        
        RegisterClass(&wc);
        classRegistered = TRUE;
    }
    
    // Calculate dialog dimensions
    int width = 500;
    int height = 400;
    int x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
    
    // Create dialog window
    char title[64];
    sprintf(title, "Browser Sanity %s", isInstall ? "Installation" : "Uninstall");
    
    HWND hDlg = CreateWindowEx(
        WS_EX_DLGMODALFRAME,
        "BrowserSanityProgress",
        title,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        x, y, width, height,
        hwndParent, NULL, GetModuleHandle(NULL), NULL
    );
    
    if (!hDlg) return IDCANCEL;
    
    // Initialize dialog data
    ProgressDialogData* pData = (ProgressDialogData*)calloc(1, sizeof(ProgressDialogData));
    pData->isInstall = isInstall;
    pData->totalSteps = InitializeProgressSteps(isInstall, &pData->steps);
    pData->currentStep = 0;
    
    SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)pData);
    
    // Get client area
    RECT clientRect;
    GetClientRect(hDlg, &clientRect);
    int clientWidth = clientRect.right - clientRect.left;
    int clientHeight = clientRect.bottom - clientRect.top;
    
    // Load application icon
    HICON hAppIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APP_ICON));
    if (!hAppIcon) hAppIcon = LoadIcon(NULL, IDI_APPLICATION);
    SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hAppIcon);
    SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hAppIcon);
    
    // Create progress list
    pData->hListBox = CreateWindow("LISTBOX", NULL,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
        20, 20, clientWidth - 40, clientHeight - 120,
        hDlg, NULL, GetModuleHandle(NULL), NULL);
    
    // Create progress bar
    pData->hProgressBar = CreateWindow("msctls_progress32", NULL,
        WS_CHILD | WS_VISIBLE,
        20, clientHeight - 80, clientWidth - 40, 20,
        hDlg, NULL, GetModuleHandle(NULL), NULL);
    SendMessage(pData->hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    
    // Create status text
    pData->hStatusText = CreateWindow("STATIC", "Ready to begin...",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, clientHeight - 55, clientWidth - 40, 15,
        hDlg, NULL, GetModuleHandle(NULL), NULL);
    
    // Create buttons
    pData->hCancelButton = CreateWindow("BUTTON", "Cancel", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        clientWidth - 160, clientHeight - 35, 70, 25, hDlg, (HMENU)IDCANCEL, GetModuleHandle(NULL), NULL);
        
    pData->hCloseButton = CreateWindow("BUTTON", "Close", WS_CHILD | BS_PUSHBUTTON,
        clientWidth - 80, clientHeight - 35, 70, 25, hDlg, (HMENU)IDOK, GetModuleHandle(NULL), NULL);
    
    // Initialize progress display
    for (int i = 0; i < pData->totalSteps; i++) {
        UpdateProgressStep(pData, i);
    }
    
    // Set fonts
    HFONT hFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
        
    SendMessage(pData->hListBox, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(pData->hStatusText, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(pData->hCancelButton, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(pData->hCloseButton, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    // TODO: Start the actual install/uninstall process in a separate thread
    // For now, just simulate progress completion
    SetWindowText(pData->hStatusText, isInstall ? "Installation ready to begin" : "Uninstall ready to begin");
    
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
    if (hFont) DeleteObject(hFont);
    
    DestroyWindow(hDlg);
    
    return IDOK;
}