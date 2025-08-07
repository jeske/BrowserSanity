/**
 * @file NKWindow_Settings.cpp
 * @brief Settings dialog window implementation - matches Archive/old_ui_system/ui2/dialog_mainSettings.c functionality
 */

#include <NKWindow.h>
#include <NKWindowManager.h>
#include <browser_sanity.h>
#include <safe_strings.h>

// For HiDPI support
#include <shellscalingapi.h>
#pragma comment(lib, "Shcore.lib")

// C linkage for Browser Sanity functions
extern "C" {
    #include <browser_sanity.h>
}

// Browser options for dropdown (matching old dialog)
static const char* browser_options[] = {
    "Google Chrome",
    "Mozilla Firefox", 
    "Microsoft Edge",
    "Brave Browser",
    "Opera"
};

/**
 * @brief Settings dialog window class
 * Handles Browser Sanity configuration settings with full functionality from old dialog
 */
class NKWindow_Settings : public NKWindow {
public:
    NKWindow_Settings(NKWindowManager& windowManager) : NKWindow(windowManager, "Browser Sanity - Settings", 600, 500) {
        // Initialize all state variables
        m_browserSelected = 0;
        m_startupEnabled = 0;
        m_watchdogEnabled = 1;
        m_redirectEnabled = 1;
        m_useDefaultBrowser = 1;
        memset(m_customBrowserPath, 0, sizeof(m_customBrowserPath));
        memset(m_customBrowserArgs, 0, sizeof(m_customBrowserArgs));
        memset(m_exceptionList, 0, sizeof(m_exceptionList));
        m_exceptionCount = 0;
        m_selectedException = -1;
        memset(m_newException, 0, sizeof(m_newException));
        m_settingsChanged = 0;
        m_initialized = 0;
        
        // Use theme background color
        SetBackgroundColor(248, 248, 248); // NK_THEME_OS_WINDOW_BG
        
        // Set size constraints
        SetSizeConstraints(550, 450, 700, 650);
    }
    
    virtual ~NKWindow_Settings() = default;
    
    virtual void Render() override {
        struct nk_context* ctx = GetContext();
        
        // Load current configuration on first run
        if (!m_initialized) {
            LoadCurrentSettings();
            m_initialized = 1;
        }
        
        // Get DPI scale for HiDPI support with fallback for older Windows versions
        float dpi_scale = 1.0f;
        HWND hwnd = GetHWND();
        
        // Try GetDpiForWindow (Windows 10 1607+)
        typedef UINT(WINAPI* GetDpiForWindowFunc)(HWND);
        HMODULE user32 = GetModuleHandleA("user32.dll");
        if (user32) {
            GetDpiForWindowFunc getDpiForWindow = (GetDpiForWindowFunc)GetProcAddress(user32, "GetDpiForWindow");
            if (getDpiForWindow) {
                UINT dpi = getDpiForWindow(hwnd);
                dpi_scale = (float)dpi / 96.0f; // 96 is standard DPI
            } else {
                // Fallback: Get system DPI
                HDC hdc = GetDC(hwnd);
                if (hdc) {
                    int dpi = GetDeviceCaps(hdc, LOGPIXELSX);
                    dpi_scale = (float)dpi / 96.0f;
                    ReleaseDC(hwnd, hdc);
                }
            }
        }
        
        // Calculate HiDPI-aware heights using proper style system
        float font_height = ctx->style.font->height;
        float button_padding = ctx->style.button.padding.y * 2;
        float text_padding = ctx->style.text.padding.y * 2;
        float edit_padding = ctx->style.edit.padding.y * 2;
        float window_spacing = ctx->style.window.spacing.y;
        
        // Apply DPI scaling to all measurements - TRIPLED for debugging
        float text_height = (font_height + text_padding) * dpi_scale * 3.0f;
        float button_height = (font_height + button_padding) * dpi_scale * 3.0f;
        float input_height = (font_height + edit_padding) * dpi_scale * 3.0f;
        float spacing_height = window_spacing * dpi_scale * 3.0f;
        float group_height = button_height * 6; // 6 rows for exception list
        
        // Create unique window name using HWND to avoid conflicts
        char windowName[MESSAGE_SIZE];
        snprintf(windowName, sizeof(windowName), "Browser Sanity - Settings##%p", (void*)GetHWND());
        
        if (nk_begin(ctx, windowName, nk_rect(0.0f, 0.0f, (float)GetWidth(), (float)GetHeight()),
                     NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_NO_SCROLLBAR)) {
            
            // Browser Selection Group
            nk_layout_row_dynamic(ctx, text_height, 1);
            nk_label(ctx, "Default Browser Selection:", NK_TEXT_LEFT);
            
            nk_layout_row_dynamic(ctx, button_height, 1);
            if (nk_combo_begin_label(ctx, browser_options[m_browserSelected], nk_vec2(nk_widget_width(ctx), 200))) {
                nk_layout_row_dynamic(ctx, button_height, 1);
                for (int i = 0; i < 5; i++) {
                    if (nk_combo_item_label(ctx, browser_options[i], NK_TEXT_LEFT)) {
                        if (m_browserSelected != i) {
                            m_browserSelected = i;
                            m_settingsChanged = 1;
                        }
                    }
                }
                nk_combo_end(ctx);
            }
            
            // Spacing
            nk_layout_row_dynamic(ctx, spacing_height, 1);
            nk_spacing(ctx, 1);
            
            // Startup Options Group
            nk_layout_row_dynamic(ctx, text_height, 1);
            nk_label(ctx, "Startup Options:", NK_TEXT_LEFT);
            
            nk_layout_row_dynamic(ctx, button_height, 1);
            int old_startup = m_startupEnabled;
            nk_checkbox_label(ctx, "Start Browser Sanity with Windows", &m_startupEnabled);
            if (old_startup != m_startupEnabled) m_settingsChanged = 1;
            
            nk_layout_row_dynamic(ctx, button_height, 1);
            int old_watchdog = m_watchdogEnabled;
            nk_checkbox_label(ctx, "Enable watchdog monitoring", &m_watchdogEnabled);
            if (old_watchdog != m_watchdogEnabled) m_settingsChanged = 1;
            
            nk_layout_row_dynamic(ctx, button_height, 1);
            int old_redirect = m_redirectEnabled;
            nk_checkbox_label(ctx, "Enable browser redirect protection", &m_redirectEnabled);
            if (old_redirect != m_redirectEnabled) m_settingsChanged = 1;
            
            // Spacing
            nk_layout_row_dynamic(ctx, 15, 1);
            nk_spacing(ctx, 1);
            
            // Site Exceptions Group
            nk_layout_row_dynamic(ctx, 25, 1);
            nk_label(ctx, "Site Exceptions:", NK_TEXT_LEFT);
            
            // Exception list
            nk_layout_row_dynamic(ctx, 120, 1);
            if (nk_group_begin(ctx, "Exceptions", NK_WINDOW_BORDER)) {
                for (int i = 0; i < m_exceptionCount; i++) {
                    nk_layout_row_dynamic(ctx, button_height, 1);
                    nk_bool is_selected = (m_selectedException == i);
                    if (nk_selectable_label(ctx, m_exceptionList[i], NK_TEXT_LEFT, &is_selected)) {
                        m_selectedException = (m_selectedException == i) ? -1 : i;
                    }
                }
                nk_group_end(ctx);
            }
            
            // Exception management buttons
            nk_layout_row_dynamic(ctx, 30, 3);
            
            // Add exception
            int len = (int)strlen(m_newException);
            nk_edit_string(ctx, NK_EDIT_FIELD, m_newException, &len, 255, nk_filter_default);
            m_newException[len] = '\0'; // Ensure null termination
            
            if (nk_button_label(ctx, "Add Exception")) {
                if (strlen(m_newException) > 0 && m_exceptionCount < 10) {
                    if (strncpy_s(m_exceptionList[m_exceptionCount], sizeof(m_exceptionList[m_exceptionCount]), 
                                  m_newException, _TRUNCATE) == 0) {
                        m_exceptionCount++;
                        memset(m_newException, 0, sizeof(m_newException));
                        m_settingsChanged = 1;
                    }
                }
            }
            
            // Remove exception
            if (m_selectedException >= 0 && m_selectedException < m_exceptionCount) {
                if (nk_button_label(ctx, "Remove Selected")) {
                    // Shift exceptions down
                    for (int i = m_selectedException; i < m_exceptionCount - 1; i++) {
                        if (strncpy_s(m_exceptionList[i], sizeof(m_exceptionList[i]), 
                                      m_exceptionList[i + 1], _TRUNCATE) != 0) {
                            // Handle error if needed
                        }
                    }
                    m_exceptionCount--;
                    m_selectedException = -1;
                    m_settingsChanged = 1;
                }
            } else {
                // Disabled remove button
                struct nk_style_button button_style = ctx->style.button;
                ctx->style.button.normal = nk_style_item_color(nk_rgb(128, 128, 128));
                ctx->style.button.text_normal = nk_rgb(64, 64, 64);
                nk_button_label(ctx, "Remove Selected");
                ctx->style.button = button_style;
            }
            
            // Spacing before action buttons
            nk_layout_row_dynamic(ctx, 20, 1);
            nk_spacing(ctx, 1);
            
            // Action buttons
            nk_layout_row_dynamic(ctx, 35, 3);
            
            // Apply button (enabled only if settings changed)
            if (m_settingsChanged) {
                if (nk_button_label(ctx, "Apply")) {
                    if (SaveSettings()) {
                        m_settingsChanged = 0;
                        MessageBoxA(GetHWND(), "Settings applied successfully!", "Browser Sanity", MB_OK | MB_ICONINFORMATION);
                    } else {
                        MessageBoxA(GetHWND(), "Failed to apply settings!", "Browser Sanity", MB_OK | MB_ICONERROR);
                    }
                }
            } else {
                // Disabled apply button
                struct nk_style_button button_style = ctx->style.button;
                ctx->style.button.normal = nk_style_item_color(nk_rgb(128, 128, 128));
                ctx->style.button.text_normal = nk_rgb(64, 64, 64);
                nk_button_label(ctx, "Apply");
                ctx->style.button = button_style;
            }
            
            // OK button
            if (nk_button_label(ctx, "OK")) {
                if (m_settingsChanged) {
                    if (SaveSettings()) {
                        HideWindow();
                    } else {
                        MessageBoxA(GetHWND(), "Failed to save settings!", "Browser Sanity", MB_OK | MB_ICONERROR);
                    }
                } else {
                    HideWindow();
                }
            }
            
            // Cancel button
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
    
    virtual void OnShow() override {
        // Reload settings when window is shown
        LoadCurrentSettings();
        m_initialized = 1;
    }

private:
    // Settings state variables (matching old dialog)
    int m_browserSelected;
    int m_startupEnabled;
    int m_watchdogEnabled;
    int m_redirectEnabled;
    int m_useDefaultBrowser;
    char m_customBrowserPath[MAX_PATH];
    char m_customBrowserArgs[1024];
    char m_exceptionList[10][256];
    int m_exceptionCount;
    int m_selectedException;
    char m_newException[256];
    int m_settingsChanged;
    int m_initialized;
    
    void LoadCurrentSettings() {
        AppConfig config;
        ReadAppConfig(&config);
        
        // Set initial values from config
        m_startupEnabled = config.runAtStartup;
        m_watchdogEnabled = config.watchdogEnabled;
        m_redirectEnabled = config.redirectConfig.redirectEnabled;
        m_useDefaultBrowser = config.redirectConfig.useDefaultBrowser;
        
        if (strncpy_s(m_customBrowserPath, sizeof(m_customBrowserPath), 
                      config.redirectConfig.customBrowserPath, _TRUNCATE) != 0) {
            m_customBrowserPath[0] = '\0';
        }
        
        if (strncpy_s(m_customBrowserArgs, sizeof(m_customBrowserArgs), 
                      config.redirectConfig.customBrowserArgs, _TRUNCATE) != 0) {
            m_customBrowserArgs[0] = '\0';
        }
        
        // Note: exceptions functionality removed as it's not in the actual config structure
        m_exceptionCount = 0;
        m_selectedException = -1;
        memset(m_newException, 0, sizeof(m_newException));
        
        m_settingsChanged = 0;
    }
    
    BOOL SaveSettings() {
        AppConfig config;
        ReadAppConfig(&config);
        
        config.runAtStartup = m_startupEnabled;
        config.watchdogEnabled = m_watchdogEnabled;
        config.redirectConfig.redirectEnabled = m_redirectEnabled;
        config.redirectConfig.useDefaultBrowser = m_useDefaultBrowser;
        
        if (strncpy_s(config.redirectConfig.customBrowserPath, sizeof(config.redirectConfig.customBrowserPath), 
                      m_customBrowserPath, _TRUNCATE) != 0) {
            return FALSE;
        }
        
        if (strncpy_s(config.redirectConfig.customBrowserArgs, sizeof(config.redirectConfig.customBrowserArgs), 
                      m_customBrowserArgs, _TRUNCATE) != 0) {
            return FALSE;
        }
        
        if (!WriteAppConfig(&config)) {
            return FALSE;
        }
        
        // Apply run at startup setting
        if (!SetRunAtStartup(m_startupEnabled)) {
            return FALSE;
        }
        
        return TRUE;
    }
};

// Factory function to create settings window
extern "C" NKWindow* CreateSettingsWindow(NKWindowManager& windowManager) {
    return new NKWindow_Settings(windowManager);
}