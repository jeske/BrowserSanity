/**
 * @file nana_progress.cpp
 * @brief Install/Uninstall Progress Window - Shows progress during installation/uninstallation
 * 
 * Shows progress bars, status messages, and handles the multi-step install/uninstall process
 */

#include <nana/gui.hpp>
#include <nana/gui/widgets/form.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/progress.hpp>
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

class NanaInstallProgressWindow {
private:
    nana::form progressForm;
    nana::place layout;
    
    // UI Controls
    nana::label titleLabel;
    nana::label statusLabel;
    nana::progress progressBar;
    nana::button cancelButton;
    
    // State
    bool isInstalling; // true for install, false for uninstall
    bool canCancel;
    
public:
    NanaInstallProgressWindow() 
        : progressForm(nana::API::make_center(400, 200))
        , layout(progressForm)
        , titleLabel(progressForm)
        , statusLabel(progressForm)
        , progressBar(progressForm)
        , cancelButton(progressForm)
        , isInstalling(true)
        , canCancel(true)
    {
        InitializeWindow();
        SetupLayout();
        SetupEventHandlers();
    }
    
    void Show() { progressForm.show(); }
    void Hide() { progressForm.hide(); }
    
    void StartInstallation() {
        isInstalling = true;
        titleLabel.caption("Installing Browser Sanity");
        statusLabel.caption("Preparing installation...");
        progressBar.value(0);
        canCancel = true;
        cancelButton.enabled(true);
        Show();
    }
    
    void StartUninstallation() {
        isInstalling = false;
        titleLabel.caption("Uninstalling Browser Sanity");
        statusLabel.caption("Preparing uninstallation...");
        progressBar.value(0);
        canCancel = true;
        cancelButton.enabled(true);
        Show();
    }
    
    void UpdateProgress(int percentage, const std::string& status) {
        progressBar.value(percentage);
        statusLabel.caption(status);
        
        // Disable cancel during critical operations
        if (percentage > 80) {
            canCancel = false;
            cancelButton.enabled(false);
        }
    }
    
    void Complete(bool success) {
        if (success) {
            progressBar.value(100);
            statusLabel.caption(isInstalling ? "Installation completed successfully!" : "Uninstallation completed successfully!");
        } else {
            statusLabel.caption(isInstalling ? "Installation failed!" : "Uninstallation failed!");
        }
        
        cancelButton.caption("Close");
        cancelButton.enabled(true);
        canCancel = false;
    }

private:
    void InitializeWindow() {
        progressForm.caption("Browser Sanity");
        
        titleLabel.caption("Processing...");
        titleLabel.text_align(nana::align::center);
        
        statusLabel.caption("Please wait...");
        statusLabel.text_align(nana::align::center);
        
        progressBar.value(0);
        cancelButton.caption("Cancel");
    }
    
    void SetupLayout() {
        layout.div(
            "vert margin=20 gap=15"
            "<title weight=30>"
            "<status weight=30>"
            "<progress weight=25>"
            "<button weight=35>"
        );
        
        layout.field("title") << titleLabel;
        layout.field("status") << statusLabel;
        layout.field("progress") << progressBar;
        layout.field("button") << cancelButton;
        
        layout.collocate();
    }
    
    void SetupEventHandlers() {
        cancelButton.events().click([this]() {
            if (canCancel) {
                nana::msgbox msg(progressForm, "Cancel Operation");
                msg.icon(nana::msgbox::icon_question);
                msg << "Are you sure you want to cancel the " << (isInstalling ? "installation" : "uninstallation") << "?";
                if (msg.show() == nana::msgbox::pick_yes) {
                    // TODO: Implement cancellation logic
                    Hide();
                }
            } else {
                Hide();
            }
        });
        
        progressForm.events().unload([this](const nana::arg_unload& arg) {
            if (canCancel) {
                arg.cancel = true; // Prevent closing during critical operations
            }
        });
    }
};