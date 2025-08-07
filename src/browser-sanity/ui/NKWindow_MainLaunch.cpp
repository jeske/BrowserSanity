/**
 * @file NKWindow_MainLaunch.cpp
 * @brief Main launch dialog window implementation
 */

#include <NKWindow.h>
#include <NKWindowManager.h>
#include <browser_sanity.h>
#include <safe_strings.h>
#include <debug_log.h>

// C linkage for Browser Sanity functions
extern "C" {
    #include <browser_sanity.h>
    // Forward declaration for full installation function
    BOOL InstallApplication();
    BOOL UninstallApplication();
}

// Forward declarations for inter-window communication functions
extern "C" void ShowSettingsWindow();
extern "C" void ShowToastNotificationCpp(const char* title, const char* message);
extern "C" void ExitApplication();

/**
 * @brief Main launch dialog window class
 * Handles the primary Browser Sanity interface with status display and action buttons
 */
class NKWindow_MainLaunch : public NKWindow {
public:
    NKWindow_MainLaunch(NKWindowManager& windowManager) : NKWindow(windowManager, "Browser Sanity - Main Control", 480, 500) {
        // Initialize main launch specific state
        m_browserRunning = false;
        m_isInstalled = false;
        m_runningPID = 0;
        if (strncpy_s(m_statusMessage, sizeof(m_statusMessage), "Checking status...", _TRUNCATE) != 0) {
            m_statusMessage[0] = '\0';
        }
        
        // Initialize configuration
        memset(&m_config, 0, sizeof(m_config));
        
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
    char m_statusMessage[MESSAGE_SIZE];
    AppConfig m_config;
    
    // UI state
    int m_selectedBrowser = 0;  // 0=Chrome, 1=Firefox, 2=Edge
    nk_bool m_runAtStartup = nk_true;
    char m_redirectUrl[URL_SIZE] = "https://www.google.com";
    
    virtual void Render() override {
        DebugLog("MainLaunch: Starting render for HWND %p", (void*)GetHWND());
        struct nk_context* ctx = GetContext();
        
        // Create unique window name using HWND to avoid conflicts
        char windowName[MESSAGE_SIZE];
        snprintf(windowName, sizeof(windowName), "Browser Sanity - Main Control##%p", (void*)GetHWND());
        
        if (nk_begin(ctx, windowName, nk_rect(10.0f, 10.0f, (float)(GetWidth() - 20), (float)(GetHeight() - 20)),
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
                ShowSettingsWindow();
                if (strncpy_s(m_statusMessage, sizeof(m_statusMessage), "Settings window opened", _TRUNCATE) != 0) {
                    m_statusMessage[0] = '\0';
                }
            }
            
            if (nk_button_label(ctx, "Show Toast")) {
                ShowToastNotificationCpp("Test Notification", "This is a test toast notification from Browser Sanity!");
                if (strncpy_s(m_statusMessage, sizeof(m_statusMessage), "Toast notification shown", _TRUNCATE) != 0) {
                    m_statusMessage[0] = '\0';
                }
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
                if (PerformInstallation()) {
                    if (strncpy_s(m_statusMessage, sizeof(m_statusMessage), "Installation successful!", _TRUNCATE) != 0) {
                        m_statusMessage[0] = '\0';
                    }
                    RefreshStatus();
                } else {
                    if (strncpy_s(m_statusMessage, sizeof(m_statusMessage), "Installation failed!", _TRUNCATE) != 0) {
                        m_statusMessage[0] = '\0';
                    }
                }
            }
            
            if (nk_button_label(ctx, "Uninstall")) {
                if (PerformUninstallation()) {
                    if (strncpy_s(m_statusMessage, sizeof(m_statusMessage), "Uninstallation successful!", _TRUNCATE) != 0) {
                        m_statusMessage[0] = '\0';
                    }
                    RefreshStatus();
                } else {
                    if (strncpy_s(m_statusMessage, sizeof(m_statusMessage), "Uninstallation failed!", _TRUNCATE) != 0) {
                        m_statusMessage[0] = '\0';
                    }
                }
            }
            
            if (nk_button_label(ctx, "Exit")) {
                ExitApplication();
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
        // Read current configuration from registry
        ReadAppConfig(&m_config);
        
        // Check installation status
        m_isInstalled = IsComprehensivelyInstalled();
        
        // Check if process is running
        m_browserRunning = IsProcessRunningWithPID(&m_runningPID);
        
        // Update UI state based on configuration
        m_runAtStartup = m_config.runAtStartup;
        if (m_config.redirectConfig.customBrowserPath[0] != '\0') {
            if (strncpy_s(m_redirectUrl, sizeof(m_redirectUrl), m_config.redirectConfig.customBrowserPath, _TRUNCATE) != 0) {
                m_redirectUrl[0] = '\0';
            }
        }
        
        // Update status message
        if (m_isInstalled) {
            if (m_browserRunning) {
                if (sprintf_s(m_statusMessage, sizeof(m_statusMessage), "Running (PID: %lu)", m_runningPID) < 0) {
                    if (strncpy_s(m_statusMessage, sizeof(m_statusMessage), "Running", _TRUNCATE) != 0) {
                        m_statusMessage[0] = '\0';
                    }
                }
            } else {
                if (strncpy_s(m_statusMessage, sizeof(m_statusMessage), "Installed, not running", _TRUNCATE) != 0) {
                    m_statusMessage[0] = '\0';
                }
            }
        } else {
            if (strncpy_s(m_statusMessage, sizeof(m_statusMessage), "Not installed", _TRUNCATE) != 0) {
                m_statusMessage[0] = '\0';
            }
        }
    }
    
    BOOL PerformInstallation() {
        DebugLogInfo("Starting Browser Sanity installation process");
        
        // Call the full installation function from action_install.c
        if (InstallApplication()) {
            DebugLogInfo("Installation completed successfully");
            
            // Update UI configuration from the installed config
            AppConfig config;
            ReadAppConfig(&config);
            config.runAtStartup = m_runAtStartup;
            WriteAppConfig(&config);
            
            // Set run at startup if requested
            if (m_runAtStartup) {
                DebugLogInfo("Setting application to run at startup");
                SetRunAtStartup(TRUE);
            }
            
            return TRUE;
        } else {
            DebugLogError("Installation failed");
            return FALSE;
        }
    }
    
    BOOL PerformUninstallation() {
        DebugLogInfo("Starting Browser Sanity uninstallation process");
        
        // Call the full uninstallation function
        if (UninstallApplication()) {
            DebugLogInfo("Uninstallation completed successfully");
            
            // Remove from startup
            SetRunAtStartup(FALSE);
            
            // Clear configuration
            AppConfig config;
            memset(&config, 0, sizeof(config));
            WriteAppConfig(&config);
            
            return TRUE;
        } else {
            DebugLogError("Uninstallation failed");
            return FALSE;
        }
    }
};

// Factory function to create main launch window
extern "C" NKWindow* CreateMainLaunchWindow(NKWindowManager& windowManager) {
    return new NKWindow_MainLaunch(windowManager);
}