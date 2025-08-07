/**
 * @file main_nana.cpp
 * @brief Browser Sanity main entry point using Nana GUI library
 * 
 * This replaces the complex Nuklear multi-window implementation with a clean,
 * simple Nana-based approach that eliminates all the context management issues.
 * 
 * Windows are now organized in separate files:
 * - nana_installer.cpp - Installer Window
 * - nana_settings.cpp - Settings Window  
 * - nana_progress.cpp - Install/Uninstall Progress Window
 * - nana_toast.cpp - Toast/Notification Window
 */

#include <nana/gui.hpp>
#include <nana/gui/msgbox.hpp>
#include <memory>

// Include C functionality
extern "C" {
    #include <browser_sanity.h>
    #include <debug_log.h>
    #include <resource.h>
}

// Include individual window implementations
#include "ui/nana_installer.cpp"
#include "ui/nana_settings.cpp"
#include "ui/nana_progress.cpp"
#include "ui/nana_toast.cpp"

/**
 * @brief Global application state for Nana-based implementation
 */
struct BrowserSanityNanaApp {
    HINSTANCE hInstance;
    bool running;
    bool isInstalled;
    
    // Window instances
    std::unique_ptr<NanaInstallerWindow> installerWindow;
    std::unique_ptr<NanaSettingsWindow> settingsWindow;
    std::unique_ptr<NanaInstallProgressWindow> progressWindow;
    std::unique_ptr<NanaToastWindow> toastWindow;
    
    BrowserSanityNanaApp() : hInstance(nullptr), running(true), isInstalled(false) {}
};

// Global app instance
BrowserSanityNanaApp g_nanaApp;

/**
 * @brief Main entry point - Determines which window to show based on installation status
 */
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Initialize logging
    InitializeLogging();
    DebugLog("=== BROWSER SANITY NANA APPLICATION STARTUP ===");
    
    g_nanaApp.hInstance = hInstance;
    
    try {
        // Determine if we're installed (running from Program Files)
        g_nanaApp.isInstalled = IsComprehensivelyInstalled();
        
        if (g_nanaApp.isInstalled) {
            // Show settings window - we're already installed
            DebugLog("Running from Program Files - showing Settings window");
            g_nanaApp.settingsWindow = std::make_unique<NanaSettingsWindow>();
            g_nanaApp.progressWindow = std::make_unique<NanaInstallProgressWindow>();
            g_nanaApp.toastWindow = std::make_unique<NanaToastWindow>();
            g_nanaApp.settingsWindow->Show();
            
            // TODO: Start background watchdog thread
            
        } else {
            // Show installer window - we're running from elsewhere
            DebugLog("Running from outside Program Files - showing Installer window");
            g_nanaApp.installerWindow = std::make_unique<NanaInstallerWindow>();
            g_nanaApp.progressWindow = std::make_unique<NanaInstallProgressWindow>();
            g_nanaApp.installerWindow->Show();
        }
        
        // Start Nana event loop
        nana::exec();
        
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

// Export functions for C compatibility
extern "C" {
    void ShowSettingsWindow() {
        if (g_nanaApp.settingsWindow) {
            g_nanaApp.settingsWindow->Show();
        }
    }
    
    void ShowToastNotificationCpp(const char* title, const char* message) {
        if (g_nanaApp.toastWindow) {
            g_nanaApp.toastWindow->ShowRedirectTamperedNotification();
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
        nana::API::exit_all();
    }
}