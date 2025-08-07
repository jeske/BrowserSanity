/**
 * @file nana_toast.cpp
 * @brief Toast Window - For watchdog notifications
 * 
 * Per original spec: "presenting a toast notification that our msedge redirect 
 * has been removed, asking the user if they want to "repair the redirect" or 
 * "open settings""
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

class NanaToastWindow {
private:
    nana::form toastForm;
    nana::place layout;
    
    // UI Controls
    nana::label messageLabel;
    nana::button repairButton;
    nana::button settingsButton;
    nana::button dismissButton;
    
public:
    NanaToastWindow() 
        : toastForm(nana::API::make_center(400, 200))
        , layout(toastForm)
        , messageLabel(toastForm)
        , repairButton(toastForm)
        , settingsButton(toastForm)
        , dismissButton(toastForm)
    {
        InitializeWindow();
        SetupLayout();
        SetupEventHandlers();
    }
    
    void Show() { toastForm.show(); }
    void Hide() { toastForm.hide(); }
    
    void ShowRedirectTamperedNotification() {
        messageLabel.caption("Browser Sanity redirect has been removed or tampered with!");
        Show();
    }

private:
    void InitializeWindow() {
        toastForm.caption("Browser Sanity - Alert");
        
        messageLabel.caption("Browser Sanity notification");
        messageLabel.text_align(nana::align::center);
        
        repairButton.caption("Repair Redirect");
        settingsButton.caption("Open Settings");
        dismissButton.caption("Dismiss");
    }
    
    void SetupLayout() {
        layout.div(
            "vert margin=15 gap=10"
            "<message weight=80>"
            "<buttons weight=40 gap=10>"
        );
        
        layout.field("message") << messageLabel;
        layout.field("buttons") << repairButton << settingsButton << dismissButton;
        
        layout.collocate();
    }
    
    void SetupEventHandlers() {
        repairButton.events().click([this]() {
            if (RepairRedirect()) {
                nana::msgbox msg(toastForm, "Repair Complete");
                msg.icon(nana::msgbox::icon_information);
                msg << "Browser redirect has been repaired successfully!";
                msg.show();
                Hide();
            } else {
                nana::msgbox msg(toastForm, "Repair Failed");
                msg.icon(nana::msgbox::icon_error);
                msg << "Failed to repair browser redirect.";
                msg.show();
            }
        });
        
        settingsButton.events().click([this]() {
            // TODO: Open settings window
            Hide();
        });
        
        dismissButton.events().click([this]() {
            Hide();
        });
        
        toastForm.events().unload([this](const nana::arg_unload& arg) {
            Hide();
        });
    }
    
    bool RepairRedirect() {
        return InstallApplication();
    }
};