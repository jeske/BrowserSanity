/**
 * @file main_nana.cpp
 * @brief Browser Sanity main entry point using Nana GUI library
 * 
 * This replaces the complex Nuklear multi-window implementation with a clean,
 * simple Nana-based approach that eliminates all the context management issues.
 * 
 * Windows are now organized in separate files:
 * - nana_initialLaunchInstall.cpp - Installer Window
 * - nana_settings.cpp - Settings Window  
 * - nana_installUninstallProgress.cpp - Install/Uninstall Progress Window
 * - nana_toast.cpp - Toast/Notification Window
 *
 * Enhanced with:
 * - System tray icon for background operation
 * - Single instance detection
 * - Proper application state management
 * - Inter-window communication system
 * - Watchdog functionality in main thread
 * - Proper application shutdown handling
 */

#include <nana/gui.hpp>
#include <nana/gui/msgbox.hpp>
#include <memory>
#include <chrono>
#include <atomic>
#include <mutex>

// Include C functionality
#include <browser_sanity.h>
#include <debug_log.h>
#include <resource.h>

// System tray icon implementation (Windows-specific)
#include <shellapi.h>

// For single instance detection
#define BROWSER_SANITY_MUTEX_NAME L"BrowserSanity_SingleInstance_Mutex"
#define WM_BROWSER_SANITY_NOTIFY (WM_USER + 1)
#define TRAY_ICON_ID 1

// Forward declarations
class NanaInstallerWindow;
class NanaSettingsWindow;
class NanaInstallProgressWindow;
class NanaToastWindow;

// Function declarations
void ShowSettingsWindow();
void ShowToastNotificationCpp(const char* title, const char* message);
void ShowProgressWindow(bool isInstall);
void UpdateProgressWindow(int percentage, const char* status);
void CompleteProgressWindow(bool success);
void ExitApplication();
bool IsRedirectIntact();

// Include individual window implementations
#include "ui/nana_initialLaunchInstall.cpp"
#include "ui/nana_settings.cpp"
#include "ui/nana_installUninstallProgress.cpp"
#include "ui/nana_toast.cpp"

/**
 * @brief Global application state for Nana-based implementation
 */
struct BrowserSanityNanaApp {
    HINSTANCE hInstance;
    std::atomic<bool> running;
    std::atomic<bool> isInstalled;
    std::atomic<bool> isMinimizedToTray;
    std::mutex appMutex;
    HWND mainHwnd;
    HANDLE singleInstanceMutex;
    NOTIFYICONDATA trayIconData;
    
    // Configuration
    AppConfig config;
    
    // Window instances
    std::unique_ptr<NanaInstallerWindow> installerWindow;
    std::unique_ptr<NanaSettingsWindow> settingsWindow;
    std::unique_ptr<NanaInstallProgressWindow> progressWindow;
    std::unique_ptr<NanaToastWindow> toastWindow;
    
    // Watchdog check time
    std::chrono::steady_clock::time_point lastWatchdogCheck;
    
    BrowserSanityNanaApp()
        : hInstance(nullptr)
        , running(true)
        , isInstalled(false)
        , isMinimizedToTray(false)
        , mainHwnd(nullptr)
        , singleInstanceMutex(nullptr)
    {
        // Initialize configuration with defaults
        memset(&config, 0, sizeof(config));
        memset(&trayIconData, 0, sizeof(trayIconData));
        lastWatchdogCheck = std::chrono::steady_clock::now();
    }
    
    ~BrowserSanityNanaApp() {
        // Remove tray icon if it exists
        if (isMinimizedToTray) {
            Shell_NotifyIcon(NIM_DELETE, &trayIconData);
        }
        
        // Release single instance mutex
        if (singleInstanceMutex) {
            ReleaseMutex(singleInstanceMutex);
            CloseHandle(singleInstanceMutex);
            singleInstanceMutex = nullptr;
        }
    }
    
    void LoadConfiguration() {
        ReadAppConfig(&config);
    }
    
    void SaveConfiguration() {
        WriteAppConfig(&config);
    }
    
    bool CheckForExistingInstance() {
        // Try to create a named mutex
        singleInstanceMutex = CreateMutexW(NULL, TRUE, BROWSER_SANITY_MUTEX_NAME);
        
        // Check if mutex already exists
        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            // Another instance is running
            DebugLog("Another instance of Browser Sanity is already running");
            
            // Find the window of the existing instance
            HWND existingWindow = FindWindowW(NULL, L"Browser Sanity - Settings");
            if (existingWindow) {
                // Bring the existing window to the foreground
                if (IsIconic(existingWindow)) {
                    ShowWindow(existingWindow, SW_RESTORE);
                }
                SetForegroundWindow(existingWindow);
                
                // Send a custom message to the existing instance
                PostMessage(existingWindow, WM_BROWSER_SANITY_NOTIFY, 0, 0);
            }
            
            return true;
        }
        
        return false;
    }
    
    void CreateTrayIcon(HWND hwnd) {
        if (!hwnd) return;
        
        mainHwnd = hwnd;
        
        // Set up tray icon data
        trayIconData.cbSize = sizeof(NOTIFYICONDATA);
        trayIconData.hWnd = hwnd;
        trayIconData.uID = TRAY_ICON_ID;
        trayIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        trayIconData.uCallbackMessage = WM_BROWSER_SANITY_NOTIFY;
        
        // Load icon from resources
        trayIconData.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(101)); // Assuming 101 is the resource ID
        if (!trayIconData.hIcon) {
            // Fallback to default application icon
            trayIconData.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        }
        
        // Set tooltip
        wcscpy_s(trayIconData.szTip, L"Browser Sanity");
        
        // Add the icon to the system tray
        Shell_NotifyIcon(NIM_ADD, &trayIconData);
        isMinimizedToTray = true;
    }
    
    void RemoveTrayIcon() {
        if (isMinimizedToTray) {
            Shell_NotifyIcon(NIM_DELETE, &trayIconData);
            isMinimizedToTray = false;
        }
    }
    
    void ShowTrayMenu(int x, int y) {
        HMENU hMenu = CreatePopupMenu();
        if (!hMenu) return;
        
        // Add menu items
        AppendMenuW(hMenu, MF_STRING, 1, L"Open Settings");
        AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
        AppendMenuW(hMenu, MF_STRING, 2, L"Exit");
        
        // Set foreground window to handle menu dismissal properly
        SetForegroundWindow(mainHwnd);
        
        // Show context menu
        UINT selectedItem = TrackPopupMenu(
            hMenu,
            TPM_RETURNCMD | TPM_NONOTIFY | TPM_RIGHTBUTTON,
            x, y,
            0,
            mainHwnd,
            NULL
        );
        
        // Handle menu selection
        switch (selectedItem) {
            case 1: // Open Settings
                if (settingsWindow) {
                    settingsWindow->Show();
                }
                break;
                
            case 2: // Exit
                ExitApplication();
                break;
        }
        
        DestroyMenu(hMenu);
    }
    
    void CheckWatchdog() {
        // Only check every 30 seconds
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastWatchdogCheck).count();
        
        if (elapsed >= 30) {
            lastWatchdogCheck = now;
            
            // Check if redirect is still in place
            if (!IsRedirectIntact()) {
                DebugLog("Watchdog detected tampered redirect");
                
                // Show toast notification
                if (toastWindow) {
                    toastWindow->ShowRedirectTamperedNotification();
                }
            }
        }
    }
};

// Global app instance
BrowserSanityNanaApp g_nanaApp;

// Windows message handler for system tray icon
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_BROWSER_SANITY_NOTIFY:
            // Handle system tray icon messages
            switch (LOWORD(lParam)) {
                case WM_RBUTTONUP:
                case WM_CONTEXTMENU:
                    // Show context menu on right-click
                    POINT pt;
                    GetCursorPos(&pt);
                    g_nanaApp.ShowTrayMenu(pt.x, pt.y);
                    break;
                    
                case WM_LBUTTONDBLCLK:
                    // Show settings window on double-click
                    if (g_nanaApp.settingsWindow) {
                        g_nanaApp.settingsWindow->Show();
                    }
                    break;
            }
            return 0;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// Implementation of exported functions
void ShowSettingsWindow() {
    if (g_nanaApp.settingsWindow) {
        g_nanaApp.settingsWindow->Show();
    }
}

void ShowToastNotificationCpp(const char* title, const char* message) {
    if (g_nanaApp.toastWindow) {
        if (title && message) {
            g_nanaApp.toastWindow->ShowNotification(title, message);
        } else {
            g_nanaApp.toastWindow->ShowRedirectTamperedNotification();
        }
    }
}

void ShowProgressWindow(bool isInstall) {
    if (g_nanaApp.progressWindow) {
        if (isInstall) {
            g_nanaApp.progressWindow->StartInstallation();
        } else {
            g_nanaApp.progressWindow->StartUninstallation();
        }
    }
}

void UpdateProgressWindow(int percentage, const char* status) {
    if (g_nanaApp.progressWindow) {
        g_nanaApp.progressWindow->UpdateProgress(percentage, status ? status : "");
    }
}

void CompleteProgressWindow(bool success) {
    if (g_nanaApp.progressWindow) {
        g_nanaApp.progressWindow->Complete(success);
    }
}

void ExitApplication() {
    g_nanaApp.running = false;
    g_nanaApp.RemoveTrayIcon();
    nana::API::exit_all();
}

bool IsRedirectIntact() {
    // Check if the redirect is still in place
    // This is a placeholder - implement the actual check
    return true;
}

// Main entry point
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Initialize logging
    InitializeLogging();
    DebugLog("=== BROWSER SANITY NANA APPLICATION STARTUP ===");
    
    g_nanaApp.hInstance = hInstance;
    
    try {
        // Check for existing instance
        if (g_nanaApp.CheckForExistingInstance()) {
            DebugLog("Another instance is already running, exiting");
            return 0;
        }
        
        // Create a hidden window for system tray icon messages
        WNDCLASS wc = {0};
        wc.lpfnWndProc = WndProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = L"BrowserSanityTrayClass";
        
        RegisterClass(&wc);
        
        HWND hwnd = CreateWindowW(
            L"BrowserSanityTrayClass",
            L"Browser Sanity Tray",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT,
            NULL, NULL, hInstance, NULL
        );
        
        // Load configuration
        g_nanaApp.LoadConfiguration();
        
        // Determine if we're installed (running from Program Files)
        g_nanaApp.isInstalled = IsComprehensivelyInstalled();
        
        // Create all window instances
        g_nanaApp.settingsWindow = std::make_unique<NanaSettingsWindow>();
        g_nanaApp.progressWindow = std::make_unique<NanaInstallProgressWindow>();
        g_nanaApp.toastWindow = std::make_unique<NanaToastWindow>();
        
        if (g_nanaApp.isInstalled) {
            // Show settings window - we're already installed
            DebugLog("Running from Program Files - showing Settings window");
            g_nanaApp.settingsWindow->Show();
            
            // Create system tray icon
            g_nanaApp.CreateTrayIcon(hwnd);
            
        } else {
            // Show installer window - we're running from elsewhere
            DebugLog("Running from outside Program Files - showing Installer window");
            g_nanaApp.installerWindow = std::make_unique<NanaInstallerWindow>();
            g_nanaApp.installerWindow->Show();
        }
        
        // Start Nana event loop
        nana::exec();
        
        // Clean up
        g_nanaApp.RemoveTrayIcon();
        
        DebugLog("=== BROWSER SANITY NANA APPLICATION SHUTDOWN ===");
        return 0;
        
    } catch (const std::exception& e) {
        DebugLogError("Exception in main: %s", e.what());
        nana::msgbox msg("Browser Sanity - Error");
        msg.icon(nana::msgbox::icon_error);
        msg << "An error occurred: " << e.what();
        msg.show();
        return -1;
    }
}