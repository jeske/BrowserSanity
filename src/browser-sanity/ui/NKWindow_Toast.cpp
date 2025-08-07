/**
 * @file NKWindow_Toast.cpp
 * @brief Toast notification dialog window implementation
 */

#include "../../../include/NKWindow.h"
#include "../../../include/NKWindowManager.h"
#include "../../../include/browser_sanity.h"

/**
 * @brief Toast notification dialog window class
 * Handles temporary notification messages and alerts
 */
class NKWindow_Toast : public NKWindow {
public:
    NKWindow_Toast(NKWindowManager& windowManager) : NKWindow(windowManager, "Browser Sanity - Toast", 350, 200) {
        // Initialize toast specific state
        strcpy(m_message, "Browser redirect successful!");
        strcpy(m_title, "Success");
        m_autoHideTimer = 0;
        m_showDetails = nk_false;
        
        // Use theme background color
        SetBackgroundColor(248, 248, 248); // NK_THEME_OS_WINDOW_BG
        
        // Enable auto-resize for toast
        EnableAutoResize(true);
        SetSizeConstraints(300, 150, 500, 400);
    }
    
    virtual ~NKWindow_Toast() = default;
    
    // Toast specific state
    char m_message[512];
    char m_title[128];
    DWORD m_autoHideTimer;
    nk_bool m_showDetails;
    
    // Toast types
    enum ToastType {
        TOAST_INFO,
        TOAST_SUCCESS,
        TOAST_WARNING,
        TOAST_ERROR
    } m_toastType = TOAST_SUCCESS;
    
    void SetMessage(const char* title, const char* message, ToastType type = TOAST_INFO) {
        strncpy(m_title, title, sizeof(m_title) - 1);
        strncpy(m_message, message, sizeof(m_message) - 1);
        m_toastType = type;
        
        // Set background color based on type
        switch (type) {
            case TOAST_SUCCESS:
            case TOAST_WARNING:
            case TOAST_ERROR:
            default:
                // Use consistent light theme background for all toast types
                SetBackgroundColor(248, 248, 248); // NK_THEME_OS_WINDOW_BG
                break;
        }
    }
    
    virtual void Render() override {
        struct nk_context* ctx = GetContext();
        
        // Create unique window name using HWND to avoid conflicts
        char windowName[256];
        snprintf(windowName, sizeof(windowName), "Browser Sanity - Notification##%p", (void*)GetHWND());
        
        if (nk_begin(ctx, windowName, nk_rect(10, 10, GetWidth() - 20, GetHeight() - 20),
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
                // TODO: Show details or settings window
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
                nk_layout_row_dynamic(ctx, 60, 1);
                nk_label_wrap(ctx, "This notification was triggered by a browser redirect event. Check settings for configuration options.");
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