/**
 * @file NKWindow_Settings.cpp
 * @brief Settings dialog window implementation
 */

#include <NKWindow.h>
#include <NKWindowManager.h>
#include <browser_sanity.h>
#include <safe_strings.h>

// C linkage for Browser Sanity functions
extern "C" {
    #include <browser_sanity.h>
}

/**
 * @brief Settings dialog window class
 * Handles Browser Sanity configuration settings
 */
class NKWindow_Settings : public NKWindow {
public:
    NKWindow_Settings(NKWindowManager& windowManager) : NKWindow(windowManager, "Browser Sanity - Settings", 420, 450) {
        // Initialize configuration
        memset(&m_config, 0, sizeof(m_config));
        LoadCurrentSettings();
        
        // Use theme background color
        SetBackgroundColor(248, 248, 248); // NK_THEME_OS_WINDOW_BG
        
        // Enable auto-resize based on content
        EnableAutoResize(true);
        SetSizeConstraints(350, 300, 550, 600);
    }
    
    virtual ~NKWindow_Settings() = default;
    
    // Settings specific state
    AppConfig m_config;
    int m_browserOption;
    nk_bool m_runAtStartup;
    nk_bool m_enableRedirect;
    char m_redirectUrl[URL_SIZE];
    char m_customPath[MAX_PATH];
    nk_bool m_settingsChanged;
    
    virtual void Render() override {
        struct nk_context* ctx = GetContext();
        
        // Create unique window name using HWND to avoid conflicts
        char windowName[MESSAGE_SIZE];
        snprintf(windowName, sizeof(windowName), "Browser Sanity - Settings##%p", (void*)GetHWND());
        
        if (nk_begin(ctx, windowName, nk_rect(10.0f, 10.0f, (float)(GetWidth() - 20), (float)(GetHeight() - 20)),
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
                if (SaveSettings()) {
                    m_settingsChanged = nk_false;
                    MessageBoxA(GetHWND(), "Settings saved successfully!", "Browser Sanity", MB_OK | MB_ICONINFORMATION);
                } else {
                    MessageBoxA(GetHWND(), "Failed to save settings!", "Browser Sanity", MB_OK | MB_ICONERROR);
                }
            }
            
            if (nk_button_label(ctx, "Reset")) {
                ResetToDefaults();
                m_settingsChanged = nk_true;
            }
            
            if (nk_button_label(ctx, "Cancel")) {
                if (m_settingsChanged) {
                    int result = MessageBoxA(GetHWND(), "You have unsaved changes. Are you sure you want to cancel?",
                                            "Browser Sanity", MB_YESNO | MB_ICONQUESTION);
                    if (result == IDYES) {
                        LoadCurrentSettings(); // Reload original settings
                        HideWindow();
                    }
                } else {
                    HideWindow();
                }
            }
            
        } else {
            // Window was closed via X button
            HideWindow();
        }
        nk_end(ctx);
    }
    
private:
    void LoadCurrentSettings() {
        // Read current configuration from registry
        ReadAppConfig(&m_config);
        
        // Update UI state from configuration
        m_runAtStartup = m_config.runAtStartup;
        m_enableRedirect = m_config.redirectConfig.redirectEnabled;
        
        // Set browser option based on custom path
        if (m_config.redirectConfig.useDefaultBrowser) {
            m_browserOption = 0; // Default browser
        } else {
            m_browserOption = 1; // Custom browser
        }
        
        // Copy paths
        if (strncpy_s(m_customPath, sizeof(m_customPath), m_config.redirectConfig.customBrowserPath, _TRUNCATE) != 0) {
            m_customPath[0] = '\0';
        }
        if (strncpy_s(m_redirectUrl, sizeof(m_redirectUrl), m_config.redirectConfig.customBrowserArgs, _TRUNCATE) != 0) {
            m_redirectUrl[0] = '\0';
        }
        
        // If no custom args, use default URL
        if (m_redirectUrl[0] == '\0') {
            if (strncpy_s(m_redirectUrl, sizeof(m_redirectUrl), "https://www.google.com", _TRUNCATE) != 0) {
                m_redirectUrl[0] = '\0';
            }
        }
        
        m_settingsChanged = nk_false;
    }
    
    BOOL SaveSettings() {
        // Update configuration from UI state
        m_config.runAtStartup = m_runAtStartup;
        m_config.redirectConfig.redirectEnabled = m_enableRedirect;
        m_config.redirectConfig.useDefaultBrowser = (m_browserOption == 0);
        
        // Copy paths
        if (strncpy_s(m_config.redirectConfig.customBrowserPath, sizeof(m_config.redirectConfig.customBrowserPath), m_customPath, _TRUNCATE) != 0) {
            m_config.redirectConfig.customBrowserPath[0] = '\0';
        }
        if (strncpy_s(m_config.redirectConfig.customBrowserArgs, sizeof(m_config.redirectConfig.customBrowserArgs), m_redirectUrl, _TRUNCATE) != 0) {
            m_config.redirectConfig.customBrowserArgs[0] = '\0';
        }
        
        // Write configuration to registry
        if (!WriteAppConfig(&m_config)) {
            return FALSE;
        }
        
        // Apply run at startup setting
        if (!SetRunAtStartup(m_runAtStartup)) {
            return FALSE;
        }
        
        return TRUE;
    }
    
    void ResetToDefaults() {
        m_browserOption = 0;
        m_runAtStartup = nk_true;
        m_enableRedirect = nk_true;
        if (strncpy_s(m_redirectUrl, sizeof(m_redirectUrl), "https://www.google.com", _TRUNCATE) != 0) {
            m_redirectUrl[0] = '\0';
        }
        m_customPath[0] = '\0';
    }
    
    virtual void OnShow() override {
        // Reload settings when window is shown
        LoadCurrentSettings();
    }
};

// Factory function to create settings window
extern "C" NKWindow* CreateSettingsWindow(NKWindowManager& windowManager) {
    return new NKWindow_Settings(windowManager);
}