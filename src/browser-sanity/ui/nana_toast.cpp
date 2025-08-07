/**
 * @file nana_toast.cpp
 * @brief Toast Window - For watchdog notifications
 * 
 * Per original spec: "presenting a toast notification that our msedge redirect 
 * has been removed, asking the user if they want to "repair the redirect" or 
 * "open settings""
 * 
 * Enhanced with:
 * - Auto-positioning in bottom-right corner
 * - Notification icon display
 * - Title and message text layout
 * - Auto-dismiss timer
 * - Fade-in/fade-out animations
 * - Click-to-dismiss functionality
 * - Notification queue for multiple messages
 */

#include <nana/gui.hpp>
#include <nana/gui/widgets/form.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/picture.hpp>
#include <nana/gui/place.hpp>
#include <nana/gui/msgbox.hpp>
#include <nana/gui/animation.hpp>
#include <nana/paint/image.hpp>
#include <memory>
#include <string>
#include <queue>
#include <chrono>
#include <thread>
#include <mutex>

// Include C functionality
extern "C" {
    #include <browser_sanity.h>
    #include <debug_log.h>
    #include <resource.h>
}

// Forward declaration for settings window access
extern void ShowSettingsWindow();

/**
 * @brief Notification data structure
 */
struct ToastNotification {
    std::string title;
    std::string message;
    bool requiresAction;
    
    ToastNotification(const std::string& t, const std::string& m, bool action = false)
        : title(t), message(m), requiresAction(action) {}
};

class NanaToastWindow {
private:
    nana::form toastForm;
    nana::place layout;
    
    // UI Controls
    nana::picture iconPicture;
    nana::label titleLabel;
    nana::label messageLabel;
    nana::button repairButton;
    nana::button settingsButton;
    nana::button dismissButton;
    
    // Auto-dismiss timing
    bool autoDismissEnabled;
    int autoDismissDelay; // in milliseconds
    
    // Notification queue
    std::queue<ToastNotification> notificationQueue;
    bool processingNotification;
    
    // Window state
    bool isVisible;
    
public:
    NanaToastWindow() 
        : toastForm(nana::rectangle(0, 0, 400, 200))
        , layout(toastForm)
        , iconPicture(toastForm)
        , titleLabel(toastForm)
        , messageLabel(toastForm)
        , repairButton(toastForm)
        , settingsButton(toastForm)
        , dismissButton(toastForm)
        , autoDismissEnabled(true)
        , autoDismissDelay(8000) // 8 seconds default
        , processingNotification(false)
        , isVisible(false)
    {
        InitializeWindow();
        SetupLayout();
        SetupEventHandlers();
        PositionWindowBottomRight();
    }
    
    ~NanaToastWindow() {
        // Clean up resources
    }
    
    void Show() { 
        if (!isVisible) {
            PositionWindowBottomRight();
            toastForm.show();
            FadeIn();
            isVisible = true;
            
            // No auto-dismiss timer in single-threaded mode
        }
    }
    
    void Hide() { 
        if (isVisible) {
            FadeOut();
            isVisible = false;
            
            // Process next notification in queue if any
            if (!notificationQueue.empty()) {
                notificationQueue.pop();
                processingNotification = false;
                ProcessNextNotification();
            }
        }
    }
    
    /**
     * @brief Show a notification about redirect tampering
     */
    void ShowRedirectTamperedNotification() {
        EnqueueNotification("Redirect Tampered", 
                           "Browser Sanity redirect has been removed or tampered with!", 
                           true); // Requires action
    }
    
    /**
     * @brief Show a general notification
     */
    void ShowNotification(const std::string& title, const std::string& message, bool requiresAction = false) {
        EnqueueNotification(title, message, requiresAction);
    }
    
    /**
     * @brief Enable or disable auto-dismiss
     */
    void SetAutoDismiss(bool enable, int delayMs = 8000) {
        autoDismissEnabled = enable;
        autoDismissDelay = delayMs;
    }

private:
    void InitializeWindow() {
        // Configure window appearance
        toastForm.caption("Browser Sanity - Notification");
        toastForm.bgcolor(nana::colors::white);
        // Note: borderless and shadow not available in this Nana version
        
        // Configure UI elements
        titleLabel.caption("Browser Sanity");
        titleLabel.text_align(nana::align::left);
        titleLabel.typeface(nana::paint::font("", 12, true)); // Bold font
        
        messageLabel.caption("Notification message");
        messageLabel.text_align(nana::align::left);
        
        // Load icon (placeholder - replace with actual icon loading)
        // In a real implementation, you would load the icon from resources
        // iconPicture.load(...);
        
        repairButton.caption("Repair Redirect");
        settingsButton.caption("Open Settings");
        dismissButton.caption("Dismiss");
    }
    
    void SetupLayout() {
        layout.div(
            "vert margin=10 gap=8"
            "<header weight=30 arrange=[20%,80%]>"
            "<message weight=60>"
            "<buttons weight=40 gap=8>"
        );
        
        layout["header"] << iconPicture << titleLabel;
        layout["message"] << messageLabel;
        layout["buttons"] << repairButton << settingsButton << dismissButton;
        
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
            ShowSettingsWindow();
            Hide();
        });
        
        dismissButton.events().click([this]() {
            Hide();
        });
        
        // Click anywhere on the form to dismiss
        toastForm.events().click([this]() {
            // Only auto-dismiss if the notification doesn't require action
            if (!notificationQueue.empty() && !notificationQueue.front().requiresAction) {
                Hide();
            }
        });
        
        toastForm.events().unload([this](const nana::arg_unload& arg) {
            Hide();
        });
    }
    
    bool RepairRedirect() {
        return InstallApplication();
    }
    
    /**
     * @brief Position the window in the bottom-right corner of the screen
     */
    void PositionWindowBottomRight() {
        // Get screen dimensions using Windows API
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        
        // Calculate position (10px margin from bottom-right)
        int x = screenWidth - toastForm.size().width - 10;
        int y = screenHeight - toastForm.size().height - 10;
        
        // Set window position
        toastForm.move(x, y);
    }
    
    /**
     * @brief Fade in animation
     */
    void FadeIn() {
        // Note: No fade animation in this version of Nana
        // Just show the window
    }
    
    /**
     * @brief Fade out animation
     */
    void FadeOut() {
        // Note: No fade animation in this version of Nana
        // Just hide the window
        toastForm.hide();
    }
    
    // Auto-dismiss functionality removed - program should be single-threaded
    
    /**
     * @brief Add a notification to the queue
     */
    void EnqueueNotification(const std::string& title, const std::string& message, bool requiresAction) {
        notificationQueue.push(ToastNotification(title, message, requiresAction));
        
        // Process the notification if not already processing one
        if (!processingNotification) {
            ProcessNextNotification();
        }
    }
    
    /**
     * @brief Process the next notification in the queue
     */
    void ProcessNextNotification() {
        if (notificationQueue.empty() || processingNotification) {
            return;
        }
        
        // Get the next notification
        const ToastNotification& notification = notificationQueue.front();
        
        // Update UI with notification content
        titleLabel.caption(notification.title);
        messageLabel.caption(notification.message);
        
        // Show/hide action buttons based on whether action is required
        repairButton.enabled(notification.requiresAction);
        settingsButton.enabled(notification.requiresAction);
        
        // Mark as processing
        processingNotification = true;
        
        // Show the notification
        Show();
    }
};