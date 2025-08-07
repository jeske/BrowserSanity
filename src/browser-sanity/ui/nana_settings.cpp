/**
 * @file nana_settings.cpp
 * @brief Settings Window - Shows when already installed
 * 
 * Per original spec: "allowing the user to install the redirect and display 
 * whether the redirect is in place, control whether this application is set 
 * to launch on windows startup, or "uninstall""
 */

#include <nana/gui.hpp>
#include <nana/gui/widgets/form.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/checkbox.hpp>
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

class NanaSettingsWindow {
private:
    nana::form settingsForm;
    nana::place layout;
    
    // UI Controls
    nana::label titleLabel;
    nana::label redirectStatusLabel;
    nana::label redirectStatusValue;
    nana::button installRedirectButton;
    nana::button removeRedirectButton;
    nana::checkbox startupCheck;
    nana::button uninstallButton;
    nana::button exitButton;
    
    // State
    bool redirectInstalled;
    bool startupEnabled;
    
public:
    NanaSettingsWindow() 
        : settingsForm(nana::API::make_center(450, 350))
        , layout(settingsForm)
        , titleLabel(settingsForm)
        , redirectStatusLabel(settingsForm)
        , redirectStatusValue(settingsForm)
        , installRedirectButton(settingsForm)
        , removeRedirectButton(settingsForm)
        , startupCheck(settingsForm)
        , uninstallButton(settingsForm)
        , exitButton(settingsForm)
        , redirectInstalled(false)
        , startupEnabled(false)
    {
        InitializeWindow();
        SetupLayout();
        SetupEventHandlers();
        RefreshStatus();
    }
    
    void Show() { settingsForm.show(); }
    void Hide() { settingsForm.hide(); }

private:
    void InitializeWindow() {
        settingsForm.caption("Browser Sanity - Settings");
        
        titleLabel.caption("Browser Sanity Settings");
        titleLabel.text_align(nana::align::center);
        
        redirectStatusLabel.caption("Redirect Status:");
        redirectStatusValue.caption("Checking...");
        
        installRedirectButton.caption("Install Redirect");
        removeRedirectButton.caption("Remove Redirect");
        startupCheck.caption("Start with Windows");
        uninstallButton.caption("Uninstall Browser Sanity");
        exitButton.caption("Exit");
    }
    
    void SetupLayout() {
        layout.div(
            "vert margin=20 gap=15"
            "<title weight=40>"
            "<status weight=60 gap=10>"
            "<redirect_buttons weight=40 gap=10>"
            "<startup weight=35>"
            "<actions weight=40 gap=10>"
        );
        
        layout.field("title") << titleLabel;
        layout.field("status") << redirectStatusLabel << redirectStatusValue;
        layout.field("redirect_buttons") << installRedirectButton << removeRedirectButton;
        layout.field("startup") << startupCheck;
        layout.field("actions") << uninstallButton << exitButton;
        
        layout.collocate();
    }
    
    void SetupEventHandlers() {
        installRedirectButton.events().click([this]() {
            if (InstallRedirect()) {
                nana::msgbox msg(settingsForm, "Redirect Installed");
                msg.icon(nana::msgbox::icon_information);
                msg << "Browser redirect has been successfully installed!";
                msg.show();
                RefreshStatus();
            } else {
                nana::msgbox msg(settingsForm, "Installation Failed");
                msg.icon(nana::msgbox::icon_error);
                msg << "Failed to install browser redirect.";
                msg.show();
            }
        });
        
        removeRedirectButton.events().click([this]() {
            if (RemoveRedirect()) {
                nana::msgbox msg(settingsForm, "Redirect Removed");
                msg.icon(nana::msgbox::icon_information);
                msg << "Browser redirect has been removed.";
                msg.show();
                RefreshStatus();
            } else {
                nana::msgbox msg(settingsForm, "Removal Failed");
                msg.icon(nana::msgbox::icon_error);
                msg << "Failed to remove browser redirect.";
                msg.show();
            }
        });
        
        startupCheck.events().checked([this](const nana::arg_checkbox& arg) {
            SetRunAtStartup(startupCheck.checked());
        });
        
        uninstallButton.events().click([this]() {
            nana::msgbox msg(settingsForm, "Confirm Uninstall");
            msg.icon(nana::msgbox::icon_question);
            msg << "Are you sure you want to uninstall Browser Sanity?\n\nThis will remove the redirect and all program files.";
            if (msg.show() == nana::msgbox::pick_yes) {
                if (PerformUninstallation()) {
                    nana::msgbox farewell(settingsForm, "Uninstall Complete");
                    farewell.icon(nana::msgbox::icon_information);
                    farewell << "Browser Sanity has been successfully uninstalled.\n\nThank you for using Browser Sanity!";
                    farewell.show();
                    nana::API::exit_all();
                } else {
                    nana::msgbox error(settingsForm, "Uninstall Failed");
                    error.icon(nana::msgbox::icon_error);
                    error << "Failed to uninstall Browser Sanity completely.";
                    error.show();
                }
            }
        });
        
        exitButton.events().click([this]() {
            nana::API::exit_all();
        });
        
        settingsForm.events().unload([this](const nana::arg_unload& arg) {
            nana::API::exit_all();
        });
    }
    
    void RefreshStatus() {
        // Check redirect status
        redirectInstalled = IsComprehensivelyInstalled();
        
        if (redirectInstalled) {
            redirectStatusValue.caption("Installed and Active");
            installRedirectButton.enabled(false);
            removeRedirectButton.enabled(true);
        } else {
            redirectStatusValue.caption("Not Installed");
            installRedirectButton.enabled(true);
            removeRedirectButton.enabled(false);
        }
        
        // Check startup status
        AppConfig config;
        ReadAppConfig(&config);
        startupEnabled = config.runAtStartup;
        startupCheck.check(startupEnabled);
    }
    
    bool InstallRedirect() {
        // TODO: Implement redirect installation
        return InstallApplication();
    }
    
    bool RemoveRedirect() {
        // TODO: Implement redirect removal
        return UninstallApplication();
    }
    
    bool PerformUninstallation() {
        return UninstallApplication();
    }
};