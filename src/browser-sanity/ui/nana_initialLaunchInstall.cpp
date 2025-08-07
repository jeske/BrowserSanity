/**
 * @file nana_installer.cpp
 * @brief Installer Window - Shows when run from outside Program Files
 * 
 * Per original spec: "present the user with some information about the application, 
 * it's purpose, a clickable link to the github repository, it will double check it 
 * is the latest version of itself (presenting an "update" button if appropriate) 
 * and also present an "Install" button"
 */

#include <nana/gui.hpp>
#include <nana/gui/widgets/form.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/picture.hpp>
#include <nana/gui/place.hpp>
#include <nana/gui/msgbox.hpp>
#include <nana/paint/image.hpp>
#include <memory>
#include <string>
#include <windows.h>

// Include C functionality
#include <browser_sanity.h>
#include <debug_log.h>
#include <resource.h>

// Include the installer action function
int RunInstallerAction();

// Forward declaration for progress window access
extern void ShowProgressWindow(bool isInstall);
extern void UpdateProgressWindow(int percentage, const char* status);
extern void CompleteProgressWindow(bool success);
extern void ExitApplication();

class NanaInstallerWindow {
private:
    nana::form installerForm;
    nana::place layout;
    
    // UI Controls
    nana::picture appIconPicture;
    nana::label titleLabel;
    nana::label descriptionLabel;
    nana::label purposeLabel;
    nana::button githubLinkButton;
    nana::label versionLabel;
    nana::button updateButton;
    nana::label creditsLabel;
    nana::button installButton;
    nana::button exitButton;
    
    // State
    bool updateAvailable;
    std::string currentVersion;
    std::string latestVersion;
    
public:
    NanaInstallerWindow()
        : installerForm(nana::API::make_center(500, 500))
        , layout(installerForm)
        , appIconPicture(installerForm)
        , titleLabel(installerForm)
        , descriptionLabel(installerForm)
        , purposeLabel(installerForm)
        , githubLinkButton(installerForm)
        , versionLabel(installerForm)
        , updateButton(installerForm)
        , creditsLabel(installerForm)
        , installButton(installerForm)
        , exitButton(installerForm)
        , updateAvailable(false)
        , currentVersion("1.0.0")
        , latestVersion("1.0.0")
    {
        InitializeWindow();
        SetupLayout();
        SetupEventHandlers();
        CheckForUpdates();
        CheckInstallationStatus();
    }
    
    void Show() {
        installerForm.modality(); // Make dialog modal
        installerForm.show();
    }
    void Hide() { installerForm.hide(); }

private:
    void InitializeWindow() {
        installerForm.caption("Browser Sanity - Installer");
        installerForm.bgcolor(nana::colors::white);
        
        // Load application icon
        try {
            // Try to load icon from resources
            // In a real implementation, you would load from resources
            // For now, we'll use a placeholder approach
            nana::paint::image img;
            // img.open("path/to/icon.png");
            // appIconPicture.load(img);
        } catch (const std::exception& e) {
            DebugLogError("Failed to load app icon: %s", e.what());
        }
        
        titleLabel.caption("Browser Sanity");
        titleLabel.text_align(nana::align::center);
        titleLabel.typeface(nana::paint::font("", 16, true)); // Bold, larger font
        
        descriptionLabel.caption("End the tyranny of applications that ignore your default browser settings!");
        descriptionLabel.text_align(nana::align::center);
        descriptionLabel.typeface(nana::paint::font("", 11, true)); // Bold
        
        // Status will be updated in CheckInstallationStatus()
        
        purposeLabel.caption(
            "Browser Sanity replaces msedge.exe to redirect Edge launches to your preferred browser.\n\n"
            "Features:\n"
            "• Redirects Edge to your default browser\n"
            "• Runs silently in the background\n"
            "• Monitors and repairs redirect if tampered with\n"
            "• Easy install/uninstall process"
        );
        
        githubLinkButton.caption("View on GitHub");
        versionLabel.caption("Version: " + currentVersion);
        updateButton.caption("Update Available");
        
        creditsLabel.caption("Created by David Jeske\nPowered by Nana C++ GUI Library");
        creditsLabel.text_align(nana::align::center);
        
        installButton.caption("Install Browser Sanity");
        exitButton.caption("Exit");
        
        // Initially hide update button
        updateButton.enabled(false);
        
        // Set focus to install button
        installButton.focus();
    }
    
    void SetupLayout() {
        layout.div(
            "vert margin=20 gap=15"
            "<header weight=60 arrange=[20%,80%]>"
            "<description weight=40>"
            "<purpose weight=120>"
            "<github weight=35>"
            "<version_info weight=35>"
            "<credits weight=50>"
            "<buttons weight=40>"
        );
        
        layout.field("header") << appIconPicture << titleLabel;
        layout.field("description") << descriptionLabel;
        layout.field("purpose") << purposeLabel;
        layout.field("github") << githubLinkButton;
        layout.field("version_info") << versionLabel << updateButton;
        layout.field("credits") << creditsLabel;
        layout.field("buttons") << installButton << exitButton;
        
        layout.collocate();
    }
    
    void SetupEventHandlers() {
        githubLinkButton.events().click([this]() {
            // Open GitHub repository
            ShellExecuteA(NULL, "open", "https://github.com/jeske/BrowserSanity", NULL, NULL, SW_SHOWNORMAL);
        });
        
        updateButton.events().click([this]() {
            // Handle update process
            nana::msgbox msg(installerForm, "Update");
            msg.icon(nana::msgbox::icon_information);
            msg << "Update functionality not yet implemented.";
            msg.show();
        });
        
        installButton.events().click([this]() {
            // Check if we need to elevate privileges first
            if (!IsUserAnAdmin()) {
                DebugLogInfo("Elevation required - requesting administrator privileges");
                
                WCHAR exePath[MAX_PATH];
                if (GetModuleFileNameW(NULL, exePath, MAX_PATH) == 0) {
                    nana::msgbox msg(installerForm, "Installation Error");
                    msg.icon(nana::msgbox::icon_error);
                    msg << "Failed to get executable path for elevation";
                    msg.show();
                    return;
                }
                
                // Elevate and run with the same parameters
                SHELLEXECUTEINFOW sei;
                ZeroMemory(&sei, sizeof(SHELLEXECUTEINFOW));
                sei.cbSize = sizeof(SHELLEXECUTEINFOW);
                sei.lpVerb = L"runas";
                sei.lpFile = exePath;
                sei.lpParameters = L"/install";
                sei.nShow = SW_NORMAL;
                
                if (!ShellExecuteExW(&sei)) {
                    nana::msgbox msg(installerForm, "Installation Error");
                    msg.icon(nana::msgbox::icon_error);
                    msg << "Failed to elevate privileges. Please run as administrator.";
                    msg.show();
                    return;
                }
                
                // The elevated process will handle the installation
                // We can close this instance
                ExitApplication();
                return;
            }
            
            // We have admin privileges, proceed with installation
            ShowProgressWindow(true); // true = installation
            PerformInstallationWithProgress();
        });
        
        exitButton.events().click([this]() {
            ExitApplication();
        });
        
        installerForm.events().unload([this](const nana::arg_unload& arg) {
            nana::API::exit_all();
        });
    }
    
    void CheckForUpdates() {
        // TODO: Implement version checking
        // For now, assume no updates available
        updateAvailable = false;
        updateButton.enabled(updateAvailable);
    }
    
    void CheckInstallationStatus() {
        // Check if the application is installed in Program Files
        bool isInstalled = IsComprehensivelyInstalled();
        
        // Check if the application is currently running
        DWORD runningPID = 0;
        bool isRunning = IsProcessRunningWithPID(&runningPID);
        
        // Update the purpose label with the status
        std::string statusText = "Current Status:\n";
        statusText += isInstalled ? "• Installed in Program Files: Yes\n" : "• Installed in Program Files: No\n";
        statusText += isRunning ? "• Currently Running: Yes (PID: " + std::to_string(runningPID) + ")\n\n" : "• Currently Running: No\n\n";
        
        statusText += "Browser Sanity replaces msedge.exe to redirect Edge launches to your preferred browser.\n\n"
                     "Features:\n"
                     "• Redirects Edge to your default browser\n"
                     "• Runs silently in the background\n"
                     "• Monitors and repairs redirect if tampered with\n"
                     "• Easy install/uninstall process";
        
        purposeLabel.caption(statusText);
        
        // Update the install button text based on status
        if (isInstalled) {
            installButton.caption("Reinstall Browser Sanity");
        } else {
            installButton.caption("Install Browser Sanity");
        }
    }
    
    void PerformInstallationWithProgress() {
        // Perform installation process with progress updates (single-threaded)
        try {
            // Hook into the actual installation process
            
            // Step 1: Prepare installation
            UpdateProgressWindow(10, "Preparing installation...");
            
            // Step 2: Check system requirements
            UpdateProgressWindow(25, "Checking system requirements...");
            
            // We already checked for admin privileges before showing the progress window
            
            // Step 3: Create program directories
            UpdateProgressWindow(40, "Creating program directories...");
            
            // Step 4: Install application files
            UpdateProgressWindow(60, "Installing application files...");
            DebugLogInfo("Calling RunInstallerAction() from nana_initialLaunchInstall.cpp");
            int installResult = RunInstallerAction();
            DebugLogInfo("RunInstallerAction() returned: %d", installResult);
            
            // Convert the result to a boolean (0 = success)
            bool success = (installResult == 0);
            
            if (success) {
                // Step 5: Configure startup settings
                UpdateProgressWindow(80, "Configuring startup settings...");
                
                // Step 6: Finalize installation
                UpdateProgressWindow(95, "Finalizing installation...");
                
                CompleteProgressWindow(true);
                
                // Show completion message and exit
                nana::msgbox msg(installerForm, "Installation Complete");
                msg.icon(nana::msgbox::icon_information);
                msg << "Browser Sanity has been successfully installed!\n\nThe application will now launch from Program Files.";
                msg.show();
                
                LaunchInstalledApplication();
                ExitApplication();
            } else {
                CompleteProgressWindow(false);
                
                // Get the last error code and message
                DWORD errorCode = GetLastError();
                char errorMessage[1024] = {0};
                FormatMessageA(
                    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    errorCode,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    errorMessage,
                    sizeof(errorMessage),
                    NULL
                );
                
                DebugLogError("Installation failed with error code %lu: %s", errorCode, errorMessage);
                
                nana::msgbox msg(installerForm, "Installation Failed");
                msg.icon(nana::msgbox::icon_error);
                msg << "Failed to install Browser Sanity.\n\nError code: " << errorCode << "\n" << errorMessage << "\n\nPlease check the logs for details.";
                msg.show();
            }
        } catch (const std::exception& e) {
            CompleteProgressWindow(false);
            
            nana::msgbox msg(installerForm, "Installation Error");
            msg.icon(nana::msgbox::icon_error);
            msg << "Installation failed with error: " << e.what();
            msg.show();
        }
    }
    
    void LaunchInstalledVersion() {
        LaunchInstalledApplication();
    }
};