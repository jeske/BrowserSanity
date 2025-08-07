/**
 * @file NKWindow_MainLaunch.cpp
 * @brief Main launch dialog window implementation
 */

#include "../../../include/NKWindow.h"
#include "../../../include/NKWindowManager.h"
#include "../../../include/browser_sanity.h"
#include "../../../include/debug_log.h"

/**
 * @brief Main launch dialog window class
 * Handles the primary Browser Sanity interface with status display and action buttons
 */
class NKWindow_MainLaunch : public NKWindow {
public:
    NKWindow_MainLaunch(NKWindowManager& windowManager) : NKWindow(windowManager, "Browser Sanity - Main Control", 480, 500) {
        // Initialize main launch specific state
        m_browserRunning = false;
        m_isInstalled = true;
        m_runningPID = 0;
        strcpy(m_statusMessage, "Ready");
        
        // Use theme background color
        SetBackgroundColor(248, 248, 248); // NK_THEME_OS_WINDOW_BG
        
        // Enable auto-resize based on content
        EnableAutoResize(true);
        SetSizeConstraints(400, 350, 600, 700);
    }
    
    virtual ~NKWindow_MainLaunch() = default;
    
    // Main launch dialog specific state
    nk_bool m_browserRunning;
    nk_bool m_isInstalled;
    DWORD m_runningPID;
    char m_statusMessage[256];
    
    // UI state
    int m_selectedBrowser = 0;  // 0=Chrome, 1=Firefox, 2=Edge
    nk_bool m_runAtStartup = nk_true;
    char m_redirectUrl[512] = "https://www.google.com";
    
    virtual void Render() override {
        DebugLog("MainLaunch: Starting render for HWND %p", (void*)GetHWND());
        struct nk_context* ctx = GetContext();
        
        // Create unique window name using HWND to avoid conflicts
        char windowName[256];
        snprintf(windowName, sizeof(windowName), "Browser Sanity - Main Control##%p", (void*)GetHWND());
        
        if (nk_begin(ctx, windowName, nk_rect(10, 10, GetWidth() - 20, GetHeight() - 20),
                     NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_TITLE)) {
            DebugLog("MainLaunch: nk_begin successful, building UI");
            
            // Header with spacing
            nk_layout_row_dynamic(ctx, 35, 1);
            nk_label(ctx, "Browser Sanity Control Panel", NK_TEXT_CENTERED);
            
            // Add spacing
            nk_layout_row_dynamic(ctx, 10, 1);
            nk_spacing(ctx, 1);
            
            // Status section with proper grouping
            if (nk_group_begin(ctx, "Status", NK_WINDOW_BORDER)) {
                nk_layout_row_dynamic(ctx, 25, 1);
                nk_label(ctx, "Status:", NK_TEXT_LEFT);
                
                nk_layout_row_dynamic(ctx, 30, 2);
                nk_label(ctx, m_statusMessage, NK_TEXT_LEFT);
                
                if (nk_button_label(ctx, "Refresh Status")) {
                    RefreshStatus();
                }
                
                nk_layout_row_dynamic(ctx, 30, 1);
                nk_checkbox_label(ctx, "Browser Running", &m_browserRunning);
                
                nk_group_end(ctx);
            }
            
            // Add spacing
            nk_layout_row_dynamic(ctx, 10, 1);
            nk_spacing(ctx, 1);
            
            // Action buttons with spacing
            nk_layout_row_dynamic(ctx, 40, 2);
            if (nk_button_label(ctx, "Show Settings")) {
                // TODO: Show settings window
                strcpy(m_statusMessage, "Settings clicked");
            }
            
            if (nk_button_label(ctx, "Show Toast")) {
                // TODO: Show toast window
                strcpy(m_statusMessage, "Toast clicked");
            }
            
            // Add spacing
            nk_layout_row_dynamic(ctx, 15, 1);
            nk_spacing(ctx, 1);
            
            // Browser selection with proper grouping
            if (nk_group_begin(ctx, "Browser Selection", NK_WINDOW_BORDER)) {
                nk_layout_row_dynamic(ctx, 25, 1);
                nk_label(ctx, "Default Browser:", NK_TEXT_LEFT);
                
                nk_layout_row_dynamic(ctx, 30, 1);
                m_selectedBrowser = nk_option_label(ctx, "Chrome", m_selectedBrowser == 0) ? 0 : m_selectedBrowser;
                nk_layout_row_dynamic(ctx, 30, 1);
                m_selectedBrowser = nk_option_label(ctx, "Firefox", m_selectedBrowser == 1) ? 1 : m_selectedBrowser;
                nk_layout_row_dynamic(ctx, 30, 1);
                m_selectedBrowser = nk_option_label(ctx, "Edge", m_selectedBrowser == 2) ? 2 : m_selectedBrowser;
                
                nk_group_end(ctx);
            }
            
            // Add spacing
            nk_layout_row_dynamic(ctx, 15, 1);
            nk_spacing(ctx, 1);
            
            // Configuration section
            if (nk_group_begin(ctx, "Configuration", NK_WINDOW_BORDER)) {
                nk_layout_row_dynamic(ctx, 30, 1);
                nk_checkbox_label(ctx, "Run at Startup", &m_runAtStartup);
                
                nk_layout_row_dynamic(ctx, 25, 1);
                nk_label(ctx, "Redirect URL:", NK_TEXT_LEFT);
                nk_layout_row_dynamic(ctx, 30, 1);
                nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, m_redirectUrl,
                                              sizeof(m_redirectUrl), nk_filter_default);
                
                nk_group_end(ctx);
            }
            
            // Add spacing
            nk_layout_row_dynamic(ctx, 15, 1);
            nk_spacing(ctx, 1);
            
            // Control buttons with better spacing
            nk_layout_row_dynamic(ctx, 40, 3);
            if (nk_button_label(ctx, "Install")) {
                strcpy(m_statusMessage, "Installing...");
            }
            
            if (nk_button_label(ctx, "Uninstall")) {
                strcpy(m_statusMessage, "Uninstalling...");
            }
            
            if (nk_button_label(ctx, "Exit")) {
                PostMessage(GetHWND(), WM_CLOSE, 0, 0);
            }
        }
        nk_end(ctx);
        DebugLog("MainLaunch: Render complete for HWND %p", (void*)GetHWND());
    }
    
    virtual void OnCreate() override {
        RefreshStatus();
    }
    
private:
    void RefreshStatus() {
        // TODO: Implement actual status checking
        if (m_isInstalled) {
            if (m_browserRunning) {
                sprintf(m_statusMessage, "Running (PID: %lu)", m_runningPID);
            } else {
                strcpy(m_statusMessage, "Installed, not running");
            }
        } else {
            strcpy(m_statusMessage, "Not installed");
        }
    }
};

// Factory function to create main launch window
extern "C" NKWindow* CreateMainLaunchWindow(NKWindowManager& windowManager) {
    return new NKWindow_MainLaunch(windowManager);
}