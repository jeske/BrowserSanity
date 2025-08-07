/**
 * @file main.cpp
 * @brief PROVEN WORKING PATTERN: Per-Window Processing with Shared Nuklear Context
 *
 * CRITICAL SUCCESS PATTERN: Individual Window Processing Cycles
 *
 * This file demonstrates the proven working architecture that solves multi-window
 * content targeting in Nuklear applications. The key breakthrough is processing
 * each window individually with immediate command processing.
 *
 * STRING ENCODING NOTICE:
 * This application uses ANSI 8-bit strings throughout for simplicity and compatibility
 * with the MultiByte character set configuration. All Windows API calls use the 'A'
 * versions (CreateWindowExA, SetWindowTextA, etc.) to match this encoding.
 *
 * TODO: Switch to double-byte Windows-standard Unicode (UTF-16) when implementing
 *       international language translations. This will require:
 *       - Changing project CharacterSet to Unicode in BrowserSanity.vcxproj
 *       - Converting all string literals to L"wide strings"
 *       - Using 'W' versions of Windows API calls (CreateWindowExW, etc.)
 *       - Updating all char* parameters to wchar_t* throughout the codebase
 *
 * KEY SUCCESS PRINCIPLES IMPLEMENTED:
 * 1. Process ALL pending Windows messages before calling UpdateAll()
 * 2. Single shared nk_context for all windows (Nuklear's intended design)
 * 3. Per-window processing: Render() → ProcessCommands() → nk_clear() → Next Window
 * 4. Immediate draw command processing prevents command mixing between windows
 * 5. WM_PAINT messages have lower priority and get starved in tight loops
 *
 * PER-WINDOW PROCESSING FLOW:
 * For each window individually:
 *   window->Render() → nk_foreach(commands) → ProcessDrawCommandForWindow() → nk_clear()
 * Then: InvalidateRect() → [Message Loop] → WM_PAINT → BitBlt
 *
 * This eliminates the "all windows showing same content" problem by ensuring
 * each window's commands are processed immediately to its own backing store
 * before the next window generates its commands.
 *
 * TODO: Implement repaint optimization using Nuklear's widget return values
 *       and activity detection to avoid unnecessary redraws. See:
 *       STRATEGIES_FOR_AI/Nuklear_OptimizingRepainting.md for patterns like
 *       nk_item_is_any_active() and widget return value tracking.
 *
 * This pattern has been tested and proven to work reliably for multi-window
 * Nuklear applications with proper input handling and unique window content.
 */

#include <NKWindow.h>
#include <NKWindowManager.h>
#include <browser_sanity.h>
#include <main.h>
#include <debug_log.h>
#include <resource.h>
#include <memory>

// Global application state definition
BrowserSanityApp g_app;

// Forward declarations for window factory functions
extern "C" NKWindow* CreateMainLaunchWindow(NKWindowManager& windowManager);
extern "C" NKWindow* CreateSettingsWindow(NKWindowManager& windowManager);
extern "C" NKWindow* CreateToastWindow(NKWindowManager& windowManager);

// Window class names (ANSI strings for MultiByte character set)
#define MAIN_WINDOW_CLASS "BrowserSanityMainWindow"
#define SETTINGS_WINDOW_CLASS "BrowserSanitySettingsWindow"
#define TOAST_WINDOW_CLASS "BrowserSanityToastWindow"

// Register window classes
bool RegisterWindowClasses(HINSTANCE hInstance) {
    // Load the application icon
    HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
    if (!hIcon) {
        // Fallback to default application icon if our icon fails to load
        hIcon = LoadIcon(NULL, IDI_APPLICATION);
    }
    
    // Main window class (using ANSI version for MultiByte character set)
    WNDCLASSEXA main_wc = {0};
    main_wc.cbSize = sizeof(WNDCLASSEXA);
    main_wc.style = CS_HREDRAW | CS_VREDRAW;
    main_wc.lpfnWndProc = NKWindowProc;
    main_wc.hInstance = hInstance;
    main_wc.hIcon = hIcon;
    main_wc.hIconSm = hIcon;
    main_wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    main_wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    main_wc.lpszClassName = MAIN_WINDOW_CLASS;
    
    if (!RegisterClassExA(&main_wc)) {
        return false;
    }
    
    // Settings window class (inherits icon from main_wc)
    WNDCLASSEXA settings_wc = main_wc;
    settings_wc.lpszClassName = SETTINGS_WINDOW_CLASS;
    
    if (!RegisterClassExA(&settings_wc)) {
        return false;
    }
    
    // Toast window class (inherits icon from main_wc)
    WNDCLASSEXA toast_wc = main_wc;
    toast_wc.lpszClassName = TOAST_WINDOW_CLASS;
    
    if (!RegisterClassExA(&toast_wc)) {
        return false;
    }
    
    return true;
}

// Initialize application windows
bool InitializeWindows(NKWindowManager& windowManager) {
    // Create window instances using factory functions
    g_app.mainWindow = std::unique_ptr<NKWindow>(CreateMainLaunchWindow(windowManager));
    g_app.settingsWindow = std::unique_ptr<NKWindow>(CreateSettingsWindow(windowManager));
    g_app.toastWindow = std::unique_ptr<NKWindow>(CreateToastWindow(windowManager));
    
    if (!g_app.mainWindow || !g_app.settingsWindow || !g_app.toastWindow) {
        return false;
    }
    
    // Create OS windows
    if (!g_app.mainWindow->CreateOSWindow(g_app.hInstance, NKWindowProc, MAIN_WINDOW_CLASS, 100, 100)) {
        return false;
    }
    
    if (!g_app.settingsWindow->CreateOSWindow(g_app.hInstance, NKWindowProc, SETTINGS_WINDOW_CLASS, 650, 150)) {
        return false;
    }
    
    if (!g_app.toastWindow->CreateOSWindow(g_app.hInstance, NKWindowProc, TOAST_WINDOW_CLASS, 400, 400)) {
        return false;
    }
    
    // Register windows with the manager
    windowManager.RegisterWindow(g_app.mainWindow.get());
    windowManager.RegisterWindow(g_app.settingsWindow.get());
    windowManager.RegisterWindow(g_app.toastWindow.get());
    
    return true;
}

// Show initial windows
void ShowInitialWindows(NKWindowManager& windowManager, int nCmdShow) {
    // Show only the main install/uninstall dialog at startup
    g_app.mainWindow->ShowWindow(nCmdShow);
    
    // Set the main window as the active window for keyboard input
    windowManager.SetActiveWindow(g_app.mainWindow.get());
    
    // Keep settings and toast windows hidden initially
    // They will be shown when requested by user actions
}

// Main message loop with C++ window management
// CRITICAL LESSON LEARNED: Immediate mode GUIs with Windows message loops
void RunMessageLoop(NKWindowManager& windowManager) {
    MSG msg = {0};
    
    g_app.running = true;
    while (g_app.running) {
        // CRITICAL: Process ALL pending Windows messages before updating Nuklear
        //
        // LESSON LEARNED: The original tight loop with PeekMessage/UpdateAll caused
        // blank windows because WM_PAINT messages were being starved. Windows assigns
        // different priorities to messages, and WM_PAINT has lower priority than
        // input messages. In a tight loop calling UpdateAll() immediately after
        // PeekMessage(), the low-priority WM_PAINT messages never got processed,
        // resulting in blank windows until user interaction forced message processing.
        //
        // SOLUTION: Process ALL pending messages in the queue before calling UpdateAll().
        // This ensures WM_PAINT messages get their chance to execute the render pipeline:
        // Nuklear generates draw commands → InvalidateRect called → WM_PAINT processed → BitBlt
        BOOL hasMessages = TRUE;
        while (hasMessages && g_app.running) {
            hasMessages = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
            
            if (hasMessages) {
                // Message available - check if it's WM_QUIT
                if (msg.message == WM_QUIT) {
                    g_app.running = false;
                    break;
                }
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        
        if (!g_app.running) break;
        
        // Only update Nuklear after ALL Windows messages are processed
        // This ensures the render pipeline works correctly:
        // 1. UpdateAll() calls nk_begin/nk_end for each window
        // 2. Nuklear generates draw commands
        // 3. InvalidateRect() is called to trigger WM_PAINT
        // 4. Next loop iteration processes WM_PAINT messages
        // 5. WM_PAINT handler calls BitBlt to copy from memory DC to window DC
        windowManager.UpdateAll();
        
        // Small sleep to prevent excessive CPU usage
        Sleep(16); // ~60 FPS
    }
}

// Cleanup application resources
void CleanupApplication(NKWindowManager& windowManager) {
    // Cleanup all windows
    windowManager.CleanupAll();
    
    // Reset unique_ptrs
    g_app.mainWindow.reset();
    g_app.settingsWindow.reset();
    g_app.toastWindow.reset();
}

// Main entry point
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Initialize logging first
    InitializeLogging();
    DebugLog("=== APPLICATION STARTUP ===");
    
    g_app.hInstance = hInstance;
    
    // Create window manager instance
    NKWindowManager windowManager;
    windowManager.Initialize();
    DebugLog("Window manager initialized");
    
    // Register window classes
    if (!RegisterWindowClasses(hInstance)) {
        MessageBoxA(NULL, "Failed to register window classes!", "Error", MB_OK | MB_ICONERROR);
        return -1;
    }
    
    // Initialize windows
    if (!InitializeWindows(windowManager)) {
        MessageBoxA(NULL, "Failed to initialize windows!", "Error", MB_OK | MB_ICONERROR);
        return -1;
    }
    
    // Show initial windows
    DebugLog("Showing initial windows");
    ShowInitialWindows(windowManager, nCmdShow);
    
    // Run main message loop
    DebugLog("Starting message loop");
    RunMessageLoop(windowManager);
    
    // Cleanup
    CleanupApplication(windowManager);
    
    return 0;
}

// Export functions for inter-window communication
extern "C" {
    // Function to show settings window from main window
    void ShowSettingsWindow() {
        if (g_app.settingsWindow) {
            g_app.settingsWindow->ShowWindow();
        }
    }
    
    // Function to show toast notification (C++ version)
    void ShowToastNotificationCpp(const char* title, const char* message) {
        if (g_app.toastWindow) {
            // TODO: Set toast message and show
            g_app.toastWindow->ShowWindow();
        }
    }
    
    // Function to exit application
    void ExitApplication() {
        g_app.running = false;
    }
}