/**
 * @file NKWindow_Settings.cpp
 * @brief Settings dialog window implementation
 */

#include "../../../include/NKWindow.h"
#include "../../../include/NKWindowManager.h"
#include "../../../include/browser_sanity.h"

/**
 * @brief Settings dialog window class
 * Handles Browser Sanity configuration settings
 */
class NKWindow_Settings : public NKWindow {
public:
    NKWindow_Settings(NKWindowManager& windowManager) : NKWindow(windowManager, "Browser Sanity - Settings", 420, 450) {
        // Initialize settings specific state
        m_browserOption = 0;  // 0=Chrome, 1=Firefox, 2=Edge
        m_runAtStartup = nk_true;
        m_enableRedirect = nk_true;
        strcpy(m_redirectUrl, "https://www.google.com");
        strcpy(m_customPath, "");
        
        // Use theme background color
        SetBackgroundColor(248, 248, 248); // NK_THEME_OS_WINDOW_BG
        
        // Enable auto-resize based on content
        EnableAutoResize(true);
        SetSizeConstraints(350, 300, 550, 600);
    }
    
    virtual ~NKWindow_Settings() = default;
    
    // Settings specific state
    int m_browserOption;
    nk_bool m_runAtStartup;
    nk_bool m_enableRedirect;
    char m_redirectUrl[512];
    char m_customPath[512];
    
    virtual void Render() override {
        struct nk_context* ctx = GetContext();
        
        // Create unique window name using HWND to avoid conflicts
        char windowName[256];
        snprintf(windowName, sizeof(windowName), "Browser Sanity - Settings##%p", (void*)GetHWND());
        
        if (nk_begin(ctx, windowName, nk_rect(10, 10, GetWidth() - 20, GetHeight() - 20),
                     NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
                     NK_WINDOW_TITLE | NK_WINDOW_CLOSABLE)) {
            
            // Header with spacing
            nk_layout_row_dynamic(ctx, 35, 1);
            nk_label(ctx, "Configuration Settings", NK_TEXT_CENTERED);
            
            // Add spacing
            nk_layout_row_dynamic(ctx, 10, 1);
            nk_spacing(ctx, 1);
            
            // Browser selection with proper grouping
            if (nk_group_begin(ctx, "Browser Selection", NK_WINDOW_BORDER)) {
                nk_layout_row_dynamic(ctx, 25, 1);
                nk_label(ctx, "Default Browser:", NK_TEXT_LEFT);
                
                nk_layout_row_dynamic(ctx, 30, 1);
                m_browserOption = nk_option_label(ctx, "Chrome", m_browserOption == 0) ? 0 : m_browserOption;
                nk_layout_row_dynamic(ctx, 30, 1);
                m_browserOption = nk_option_label(ctx, "Firefox", m_browserOption == 1) ? 1 : m_browserOption;
                nk_layout_row_dynamic(ctx, 30, 1);
                m_browserOption = nk_option_label(ctx, "Edge", m_browserOption == 2) ? 2 : m_browserOption;
                
                nk_group_end(ctx);
            }
            
            // Add spacing
            nk_layout_row_dynamic(ctx, 15, 1);
            nk_spacing(ctx, 1);
            
            // Startup options with proper grouping
            if (nk_group_begin(ctx, "Startup Options", NK_WINDOW_BORDER)) {
                nk_layout_row_dynamic(ctx, 30, 1);
                nk_checkbox_label(ctx, "Run at Windows Startup", &m_runAtStartup);
                nk_layout_row_dynamic(ctx, 30, 1);
                nk_checkbox_label(ctx, "Enable Browser Redirect", &m_enableRedirect);
                
                nk_group_end(ctx);
            }
            
            // Add spacing
            nk_layout_row_dynamic(ctx, 15, 1);
            nk_spacing(ctx, 1);
            
            // URL Configuration with proper grouping
            if (nk_group_begin(ctx, "URL Configuration", NK_WINDOW_BORDER)) {
                nk_layout_row_dynamic(ctx, 25, 1);
                nk_label(ctx, "Redirect URL:", NK_TEXT_LEFT);
                nk_layout_row_dynamic(ctx, 30, 1);
                nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, m_redirectUrl,
                                              sizeof(m_redirectUrl), nk_filter_default);
                
                // Add some spacing within group
                nk_layout_row_dynamic(ctx, 10, 1);
                nk_spacing(ctx, 1);
                
                nk_layout_row_dynamic(ctx, 25, 1);
                nk_label(ctx, "Custom Browser Path (optional):", NK_TEXT_LEFT);
                nk_layout_row_dynamic(ctx, 30, 1);
                nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, m_customPath,
                                              sizeof(m_customPath), nk_filter_default);
                
                nk_group_end(ctx);
            }
            
            // Add spacing
            nk_layout_row_dynamic(ctx, 15, 1);
            nk_spacing(ctx, 1);
            
            // Action buttons with better spacing
            nk_layout_row_dynamic(ctx, 40, 3);
            if (nk_button_label(ctx, "Save")) {
                SaveSettings();
            }
            
            if (nk_button_label(ctx, "Reset")) {
                ResetToDefaults();
            }
            
            if (nk_button_label(ctx, "Cancel")) {
                HideWindow();
            }
            
        } else {
            // Window was closed via X button
            HideWindow();
        }
        nk_end(ctx);
    }
    
private:
    void SaveSettings() {
        // TODO: Implement actual settings save
        MessageBoxA(GetHWND(), "Settings saved successfully!", "Browser Sanity", MB_OK | MB_ICONINFORMATION);
    }
    
    void ResetToDefaults() {
        m_browserOption = 0;
        m_runAtStartup = nk_true;
        m_enableRedirect = nk_true;
        strcpy(m_redirectUrl, "https://www.google.com");
        strcpy(m_customPath, "");
    }
};

// Factory function to create settings window
extern "C" NKWindow* CreateSettingsWindow(NKWindowManager& windowManager) {
    return new NKWindow_Settings(windowManager);
}