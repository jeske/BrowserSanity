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
#include <nana/gui/place.hpp>
#include <nana/gui/msgbox.hpp>
#include <memory>
#include <string>

// Include C functionality
extern "C" {
    #include <browser_sanity.h>
    #include <debug_log.h>
    #include <resource.h>
}

class NanaInstallerWindow {
private:
    nana::form installerForm;
    nana::place layout;
    
    // UI Controls
    nana::label titleLabel;
    nana::label descriptionLabel;
    nana::label purposeLabel;
    nana::button githubLinkButton;
    nana::label versionLabel;
    nana::button updateButton;
    nana::button installButton;
    nana::button exitButton;
    
    // State
    bool updateAvailable;
    std::string currentVersion;
    std::string latestVersion;
    
public:
    NanaInstallerWindow() 
        : installerForm(nana::API::make_center(500, 400))
        , layout(installerForm)
        , titleLabel(installerForm)
        , descriptionLabel(installerForm)
        , purposeLabel(installerForm)
        , githubLinkButton(installerForm)
        , versionLabel(installerForm)
        , updateButton(installerForm)
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
    
    void Show() { installerForm.show(); }
    void Hide() { installerForm.hide(); }

private:
    void InitializeWindow() {
        installerForm.caption("Browser Sanity - Installer");
        
        titleLabel.caption("Browser Sanity");
        titleLabel.text_align(nana::align::center);
        
        descriptionLabel.caption("End the tyranny of applications that ignore your default browser settings!");
        descriptionLabel.text_align(nana::align::center);
        
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
        installButton.caption("Install Browser Sanity");
        exitButton.caption("Exit");
        
        // Initially hide update button
        updateButton.enabled(false);
    }
    
    void SetupLayout() {
        layout.div(
            "vert margin=20 gap=15"
            "<title weight=40>"
            "<description weight=60>"
            "<purpose weight=120>"
            "<github weight=35>"
            "<version_info weight=35>"
            "<buttons weight=40>"
        );
        
        layout.field("title") << titleLabel;
        layout.field("description") << descriptionLabel;
        layout.field("purpose") << purposeLabel;
        layout.field("github") << githubLinkButton;
        layout.field("version_info") << versionLabel << updateButton;
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
            if (PerformInstallation()) {
                nana::msgbox msg(installerForm, "Installation Complete");
                msg.icon(nana::msgbox::icon_information);
                msg << "Browser Sanity has been successfully installed!\n\nThe application will now launch from Program Files.";
                msg.show();
                
                // Launch installed version and exit
                LaunchInstalledVersion();
                nana::API::exit_all();
            } else {
                nana::msgbox msg(installerForm, "Installation Failed");
                msg.icon(nana::msgbox::icon_error);
                msg << "Failed to install Browser Sanity. Please check the logs for details.";
                msg.show();
            }
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
    
    bool PerformInstallation() {
        // Call the C installation function
        return InstallApplication();
    }
    
    void LaunchInstalledVersion() {
        // TODO: Launch the installed version from Program Files
        // This would typically involve ShellExecute to the installed location
    }
};