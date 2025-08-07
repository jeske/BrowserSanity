/**
 * @file NKWindow_Toast.cpp
 * @brief Toast notification dialog window implementation
 */

#include <NKWindow.h>
#include <NKWindowManager.h>
#include <browser_sanity.h>
#include <safe_strings.h>

// C linkage for Browser Sanity functions
extern "C" {
    #include <browser_sanity.h>
}

// Forward declarations for inter-window communication functions
extern "C" void ShowSettingsWindow();

/**
 * @brief Toast notification dialog window class
 * Handles temporary notification messages and alerts
 */
class NKWindow_Toast : public NKWindow {
public:
    NKWindow_Toast(NKWindowManager& windowManager) : NKWindow(windowManager, "Browser Sanity - Notification", 350, 200) {
        // Initialize toast specific state
        if (strncpy_s(m_message, sizeof(m_message), "Browser Sanity is working correctly!", _TRUNCATE) != 0) {
            m_message[0] = '\0';
        }
        if (strncpy_s(m_title, sizeof(m_title), "Status", _TRUNCATE) != 0) {
            m_title[0] = '\0';
        }
        if (strncpy_s(m_detailsText, sizeof(m_detailsText), "All systems operational. Browser redirects are functioning normally.", _TRUNCATE) != 0) {
            m_detailsText[0] = '\0';
        }
        m_autoHideTimer = 0;
        m_showDetails = nk_false;
        m_toastType = TOAST_INFO;
        
        // Use theme background color
        SetBackgroundColor(248, 248, 248); // NK_THEME_OS_WINDOW_BG
        
        // Enable auto-resize for toast
        EnableAutoResize(true);
        SetSizeConstraints(300, 150, 500, 400);
    }
    
    virtual ~NKWindow_Toast() = default;
    
    // Toast specific state
    char m_message[MESSAGE_SIZE];
    char m_title[WINDOW_TITLE_SIZE];
    char m_detailsText[DETAILS_SIZE];
    DWORD m_autoHideTimer;
    nk_bool m_showDetails;
    
    // Toast types
    enum ToastType {
        TOAST_INFO,
        TOAST_SUCCESS,
        TOAST_WARNING,
        TOAST_ERROR
    } m_toastType = TOAST_INFO;
    
    void SetMessage(const char* title, const char* message, ToastType type = TOAST_INFO, const char* details = nullptr) {
        if (strncpy_s(m_title, sizeof(m_title), title, _TRUNCATE) != 0) {
            m_title[0] = '\0';
        }
        
        if (strncpy_s(m_message, sizeof(m_message), message, _TRUNCATE) != 0) {
            m_message[0] = '\0';
        }
        
        if (details) {
            if (strncpy_s(m_detailsText, sizeof(m_detailsText), details, _TRUNCATE) != 0) {
                m_detailsText[0] = '\0';
            }
        } else {
            // Generate default details based on type
            switch (type) {
                case TOAST_SUCCESS:
                    if (strncpy_s(m_detailsText, sizeof(m_detailsText), "Operation completed successfully. No further action required.", _TRUNCATE) != 0) {
                        m_detailsText[0] = '\0';
                    }
                    break;
                case TOAST_WARNING:
                    if (strncpy_s(m_detailsText, sizeof(m_detailsText), "Please review the warning and take appropriate action if needed.", _TRUNCATE) != 0) {
                        m_detailsText[0] = '\0';
                    }
                    break;
                case TOAST_ERROR:
                    if (strncpy_s(m_detailsText, sizeof(m_detailsText), "An error occurred. Please check the application status and try again.", _TRUNCATE) != 0) {
                        m_detailsText[0] = '\0';
                    }
                    break;
                default:
                    if (strncpy_s(m_detailsText, sizeof(m_detailsText), "For more information, check the main application window or settings.", _TRUNCATE) != 0) {
                        m_detailsText[0] = '\0';
                    }
                    break;
            }
        }
        
        m_toastType = type;
        
        // Use consistent light theme background for all toast types
        SetBackgroundColor(248, 248, 248); // NK_THEME_OS_WINDOW_BG
        
        // Reset auto-hide timer
        m_autoHideTimer = GetTickCount();
    }
    
    virtual void Render() override {
        struct nk_context* ctx = GetContext();
        
        // Create unique window name using HWND to avoid conflicts
        char windowName[MESSAGE_SIZE];
        snprintf(windowName, sizeof(windowName), "Browser Sanity - Notification##%p", (void*)GetHWND());
        
        if (nk_begin(ctx, windowName, nk_rect(10.0f, 10.0f, (float)(GetWidth() - 20), (float)(GetHeight() - 20)),
                     NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_CLOSABLE)) {
            
            // Add spacing at top
            nk_layout_row_dynamic(ctx, 10, 1);
            nk_spacing(ctx, 1);
            
            // Title with more space
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_label(ctx, m_title, NK_TEXT_CENTERED);
            
            // Add spacing
            nk_layout_row_dynamic(ctx, 10, 1);
            nk_spacing(ctx, 1);
            
            // Message with proper wrapping and more height
            nk_layout_row_dynamic(ctx, 50, 1);
            nk_label_wrap(ctx, m_message);
            
            // Add spacing
            nk_layout_row_dynamic(ctx, 15, 1);
            nk_spacing(ctx, 1);
            
            // Action buttons with better spacing
            nk_layout_row_dynamic(ctx, 35, 3);
            if (nk_button_label(ctx, "OK")) {
                HideWindow();
            }
            
            if (nk_button_label(ctx, "Details")) {
                m_showDetails = !m_showDetails;
                // Resize window when showing/hiding details
                if (m_showDetails) {
                    SetSizeConstraints(300, 200, 500, 500);
                } else {
                    SetSizeConstraints(300, 150, 500, 400);
                }
            }
            
            if (nk_button_label(ctx, "Dismiss")) {
                HideWindow();
            }
            
            // Optional details section
            if (m_showDetails) {
                nk_layout_row_dynamic(ctx, 15, 1);
                nk_spacing(ctx, 1);
                
                nk_layout_row_dynamic(ctx, 25, 1);
                nk_label(ctx, "Additional Information:", NK_TEXT_LEFT);
                nk_layout_row_dynamic(ctx, 80, 1);
                nk_label_wrap(ctx, m_detailsText);
                
                // Add action buttons in details section
                nk_layout_row_dynamic(ctx, 15, 1);
                nk_spacing(ctx, 1);
                
                nk_layout_row_dynamic(ctx, 35, 2);
                if (nk_button_label(ctx, "Open Settings")) {
                    OpenSettingsWindow();
                }
                if (nk_button_label(ctx, "Check Status")) {
                    ShowStatusInfo();
                }
            }
            
        } else {
            // Window was closed via X button
            HideWindow();
        }
        nk_end(ctx);
    }
    
    virtual void OnShow() override {
        // Start auto-hide timer when shown
        m_autoHideTimer = GetTickCount();
    }
    
    virtual void OnCreate() override {
        // Position toast in bottom-right corner of screen
        RECT desktop;
        GetWindowRect(GetDesktopWindow(), &desktop);
        
        int x = desktop.right - GetWidth() - 20;
        int y = desktop.bottom - GetHeight() - 60;
        
        SetWindowPos(GetHWND(), HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
    }
    
    // Check if toast should auto-hide (call this in update loop)
    bool ShouldAutoHide(DWORD timeoutMs = 5000) {
        if (m_autoHideTimer > 0 && IsActive()) {
            return (GetTickCount() - m_autoHideTimer) > timeoutMs;
        }
        return false;
    }

private:
    void OpenSettingsWindow() {
        // Call the global settings function
        ShowSettingsWindow();
    }
    
    void ShowStatusInfo() {
        // Update the toast with current status information
        AppConfig config;
        ReadAppConfig(&config);
        
        char statusDetails[DETAILS_SIZE];
        DWORD pid = 0;
        BOOL isRunning = IsProcessRunningWithPID(&pid);
        BOOL isInstalled = IsComprehensivelyInstalled();
        
        if (isInstalled) {
            if (isRunning) {
                if (sprintf_s(statusDetails, sizeof(statusDetails), "Browser Sanity is installed and running (PID: %lu).\n\n"
                                     "Installation Path: %s\n"
                                     "Version: %s\n"
                                     "Run at Startup: %s\n"
                                     "Redirect Enabled: %s",
                       pid, config.installPath, config.version,
                       config.runAtStartup ? "Yes" : "No",
                       config.redirectConfig.redirectEnabled ? "Yes" : "No") < 0) {
                    if (strncpy_s(statusDetails, sizeof(statusDetails), "Status information unavailable", _TRUNCATE) != 0) {
                        statusDetails[0] = '\0';
                    }
                }
            } else {
                if (sprintf_s(statusDetails, sizeof(statusDetails), "Browser Sanity is installed but not currently running.\n\n"
                                     "Installation Path: %s\n"
                                     "Version: %s\n"
                                     "Run at Startup: %s\n"
                                     "Redirect Enabled: %s",
                       config.installPath, config.version,
                       config.runAtStartup ? "Yes" : "No",
                       config.redirectConfig.redirectEnabled ? "Yes" : "No") < 0) {
                    if (strncpy_s(statusDetails, sizeof(statusDetails), "Status information unavailable", _TRUNCATE) != 0) {
                        statusDetails[0] = '\0';
                    }
                }
            }
        } else {
            if (strncpy_s(statusDetails, sizeof(statusDetails), "Browser Sanity is not currently installed.\n\n"
                                "Use the main window to install Browser Sanity and enable browser redirection.", _TRUNCATE) != 0) {
                statusDetails[0] = '\0';
            }
        }
        
        SetMessage("Current Status", isInstalled ? "Browser Sanity Status Information" : "Not Installed", 
                  isInstalled ? TOAST_INFO : TOAST_WARNING, statusDetails);
        m_showDetails = nk_true;
    }
};

// Factory function to create toast window
extern "C" NKWindow* CreateToastWindow(NKWindowManager& windowManager) {
    return new NKWindow_Toast(windowManager);
}

// Convenience function to show a toast message
extern "C" void ShowToastMessage(NKWindowManager& windowManager, const char* title, const char* message) {
    // This would be called from the main application
    // Implementation would create and show a toast window
    NKWindow_Toast* toast = new NKWindow_Toast(windowManager);
    toast->SetMessage(title, message);
    // TODO: Show the toast window
}