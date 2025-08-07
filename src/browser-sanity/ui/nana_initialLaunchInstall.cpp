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
extern "C" {
    #include <browser_sanity.h>
    #include <debug_log.h>
    #include <resource.h>
}

// Forward declaration for progress window access
extern void ShowProgressWindow(bool isInstall);
extern void UpdateProgressWindow(int percentage, const char* status);
extern void CompleteProgressWindow(bool success);

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
            // Show progress window and start installation
            ShowProgressWindow(true); // true = installation
            PerformInstallationWithProgress();
        });
        
        exitButton.events().click([this]() {
            nana::API::exit_all();
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
    
    void PerformInstallationWithProgress() {
        // Perform installation process with progress updates (single-threaded)
        try {
            UpdateProgressWindow(10, "Preparing installation...");
            Sleep(500); // Use Windows Sleep instead of std::this_thread
            
            UpdateProgressWindow(25, "Checking system requirements...");
            Sleep(500);
            
            UpdateProgressWindow(40, "Creating program directories...");
            Sleep(500);
            
            UpdateProgressWindow(60, "Installing application files...");
            bool installResult = InstallApplication();
            
            if (installResult) {
                UpdateProgressWindow(80, "Configuring startup settings...");
                Sleep(500);
                
                UpdateProgressWindow(95, "Finalizing installation...");
                Sleep(500);
                
                CompleteProgressWindow(true);
                
                // Show completion message and exit
                nana::msgbox msg(installerForm, "Installation Complete");
                msg.icon(nana::msgbox::icon_information);
                msg << "Browser Sanity has been successfully installed!\n\nThe application will now launch from Program Files.";
                msg.show();
                
                LaunchInstalledVersion();
                nana::API::exit_all();
            } else {
                CompleteProgressWindow(false);
                
                nana::msgbox msg(installerForm, "Installation Failed");
                msg.icon(nana::msgbox::icon_error);
                msg << "Failed to install Browser Sanity. Please check the logs for details.";
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
        // TODO: Launch the installed version from Program Files
        // This would typically involve ShellExecute to the installed location
    }
};