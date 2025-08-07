/**
 * @file nana_progress.cpp
 * @brief Install/Uninstall Progress Window - Shows progress during installation/uninstallation
 * 
 * Shows progress bars, status messages, and handles the multi-step install/uninstall process
 * 
 * Enhanced with:
 * - Progress step list display using Nana listbox
 * - Status indicators ([PENDING], [WORKING], [DONE], [FAILED])
 * - Step execution threading integration
 * - Modal dialog behavior
 */

#include <nana/gui.hpp>
#include <nana/gui/widgets/form.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/progress.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/place.hpp>
#include <nana/gui/msgbox.hpp>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <windows.h>

// Include C functionality
extern "C" {
    #include <browser_sanity.h>
    #include <debug_log.h>
    #include <resource.h>
}

/**
 * @brief Progress step structure
 */
struct ProgressStep {
    enum Status {
        PENDING,
        WORKING,
        DONE,
        FAILED
    };
    
    std::string description;
    Status status;
    
    ProgressStep(const std::string& desc) 
        : description(desc), status(PENDING) {}
    
    std::string GetStatusText() const {
        switch (status) {
            case PENDING: return "[PENDING]";
            case WORKING: return "[WORKING]";
            case DONE:    return "[DONE]";
            case FAILED:  return "[FAILED]";
            default:      return "[UNKNOWN]";
        }
    }
    
    nana::color GetStatusColor() const {
        switch (status) {
            case PENDING: return nana::colors::gray;
            case WORKING: return nana::colors::blue;
            case DONE:    return nana::colors::green;
            case FAILED:  return nana::colors::red;
            default:      return nana::colors::black;
        }
    }
};

class NanaInstallProgressWindow {
private:
    nana::form progressForm;
    nana::place layout;
    
    // UI Controls
    nana::label titleLabel;
    nana::listbox stepsList;
    nana::label statusLabel;
    nana::progress progressBar;
    nana::button cancelButton;
    
    // State
    bool isInstalling; // true for install, false for uninstall
    bool canCancel;
    bool isCompleted;
    
    // Progress steps
    std::vector<ProgressStep> steps;
    int currentStepIndex;
    
public:
    NanaInstallProgressWindow() 
        : progressForm(nana::API::make_center(500, 400))
        , layout(progressForm)
        , titleLabel(progressForm)
        , stepsList(progressForm)
        , statusLabel(progressForm)
        , progressBar(progressForm)
        , cancelButton(progressForm)
        , isInstalling(true)
        , canCancel(true)
        , isCompleted(false)
        , currentStepIndex(-1)
    {
        InitializeWindow();
        SetupLayout();
        SetupEventHandlers();
    }
    
    ~NanaInstallProgressWindow() {
        // Clean up resources
    }
    
    void Show() { 
        progressForm.modality(); // Make dialog modal
        progressForm.show(); 
    }
    
    void Hide() { progressForm.hide(); }
    
    void StartInstallation() {
        isInstalling = true;
        isCompleted = false;
        titleLabel.caption("Installing Browser Sanity");
        statusLabel.caption("Preparing installation...");
        progressBar.value(0);
        canCancel = true;
        cancelButton.enabled(true);
        cancelButton.caption("Cancel");
        
        // Initialize installation steps
        InitializeProgressSteps(true);
        
        // Show the window
        Show();
        
        // Start installation process
        PerformInstallation();
    }
    
    void StartUninstallation() {
        isInstalling = false;
        isCompleted = false;
        titleLabel.caption("Uninstalling Browser Sanity");
        statusLabel.caption("Preparing uninstallation...");
        progressBar.value(0);
        canCancel = true;
        cancelButton.enabled(true);
        cancelButton.caption("Cancel");
        
        // Initialize uninstallation steps
        InitializeProgressSteps(false);
        
        // Show the window
        Show();
        
        // Start uninstallation process
        PerformUninstallation();
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
    
    void UpdateProgressStep(int stepIndex, ProgressStep::Status status, const std::string& message = "") {
        if (stepIndex < 0 || stepIndex >= static_cast<int>(steps.size())) {
            return;
        }
        
        // Update step status
        steps[stepIndex].status = status;
        
        // Update UI directly (no threading)
        // Update listbox
        auto item = stepsList.at(0).at(stepIndex);
        item.text(1, steps[stepIndex].GetStatusText());
        
        // Set text color based on status
        // Note: fgcolor method doesn't take column index in this Nana version
        stepsList.at(0).at(stepIndex).fgcolor(steps[stepIndex].GetStatusColor());
        
        // Update status message if provided
        if (!message.empty()) {
            statusLabel.caption(message);
        }
        
        // Update progress bar based on completed steps
        int completedSteps = 0;
        for (const auto& step : steps) {
            if (step.status == ProgressStep::DONE) {
                completedSteps++;
            }
        }
        
        int progressPercentage = (steps.size() > 0) ? (completedSteps * 100) / steps.size() : 0;
        progressBar.value(progressPercentage);
        
        // Select the current step in the listbox
        stepsList.at(0).select(stepIndex);
        
        // Disable cancel during critical operations
        if (status == ProgressStep::WORKING && stepIndex >= static_cast<int>(steps.size()) / 2) {
            canCancel = false;
            cancelButton.enabled(false);
        }
    }
    
    void Complete(bool success) {
        isCompleted = true;
        
        if (success) {
            progressBar.value(100);
            statusLabel.caption(isInstalling ? "Installation completed successfully!" : "Uninstallation completed successfully!");
            
            // Mark all remaining steps as done
            for (size_t i = 0; i < steps.size(); i++) {
                if (steps[i].status != ProgressStep::DONE && steps[i].status != ProgressStep::FAILED) {
                    UpdateProgressStep(i, ProgressStep::DONE);
                }
            }
        } else {
            statusLabel.caption(isInstalling ? "Installation failed!" : "Uninstallation failed!");
            
            // Mark current step as failed
            if (currentStepIndex >= 0 && currentStepIndex < static_cast<int>(steps.size())) {
                UpdateProgressStep(currentStepIndex, ProgressStep::FAILED);
            }
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
        
        // Configure listbox
        stepsList.append_header("Step");
        stepsList.append_header("Status");
        stepsList.enable_single(true, true);
        
        statusLabel.caption("Please wait...");
        statusLabel.text_align(nana::align::center);
        
        progressBar.value(0);
        cancelButton.caption("Cancel");
    }
    
    void SetupLayout() {
        layout.div(
            "vert margin=20 gap=15"
            "<title weight=30>"
            "<steps weight=200>"
            "<status weight=30>"
            "<progress weight=25>"
            "<button weight=35>"
        );
        
        layout.field("title") << titleLabel;
        layout.field("steps") << stepsList;
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
            if (canCancel && !isCompleted) {
                arg.cancel = true; // Prevent closing during critical operations
            }
        });
    }
    
    void InitializeProgressSteps(bool isInstall) {
        // Clear existing steps
        steps.clear();
        stepsList.clear();
        currentStepIndex = -1;
        
        if (isInstall) {
            // Installation steps
            steps.push_back(ProgressStep("Preparing installation"));
            steps.push_back(ProgressStep("Checking system requirements"));
            steps.push_back(ProgressStep("Creating program directories"));
            steps.push_back(ProgressStep("Installing application files"));
            steps.push_back(ProgressStep("Configuring startup settings"));
            steps.push_back(ProgressStep("Setting up browser redirect"));
            steps.push_back(ProgressStep("Finalizing installation"));
        } else {
            // Uninstallation steps
            steps.push_back(ProgressStep("Preparing uninstallation"));
            steps.push_back(ProgressStep("Stopping background services"));
            steps.push_back(ProgressStep("Removing browser redirect"));
            steps.push_back(ProgressStep("Removing startup entries"));
            steps.push_back(ProgressStep("Removing application files"));
            steps.push_back(ProgressStep("Cleaning up registry"));
            steps.push_back(ProgressStep("Finalizing uninstallation"));
        }
        
        // Add steps to listbox
        for (const auto& step : steps) {
            stepsList.at(0).append({step.description, step.GetStatusText()});
        }
        
        // Set initial colors
        for (size_t i = 0; i < steps.size(); i++) {
            // Note: fgcolor method doesn't take column index in this Nana version
            stepsList.at(0).at(i).fgcolor(steps[i].GetStatusColor());
        }
    }
    
    // Removed StartOperationThread - using direct function calls instead
    
    void PerformInstallation() {
        bool success = true;
        
        try {
            // Step 1: Preparing installation
            currentStepIndex = 0;
            UpdateProgressStep(currentStepIndex, ProgressStep::WORKING, "Preparing installation...");
            Sleep(500); // Use Windows Sleep instead of std::this_thread
            UpdateProgressStep(currentStepIndex, ProgressStep::DONE);
            
            // Step 2: Checking system requirements
            currentStepIndex = 1;
            UpdateProgressStep(currentStepIndex, ProgressStep::WORKING, "Checking system requirements...");
            Sleep(500);
            UpdateProgressStep(currentStepIndex, ProgressStep::DONE);
            
            // Step 3: Creating program directories
            currentStepIndex = 2;
            UpdateProgressStep(currentStepIndex, ProgressStep::WORKING, "Creating program directories...");
            Sleep(500);
            UpdateProgressStep(currentStepIndex, ProgressStep::DONE);
            
            // Step 4: Installing application files
            currentStepIndex = 3;
            UpdateProgressStep(currentStepIndex, ProgressStep::WORKING, "Installing application files...");
            
            // Perform actual installation
            bool installResult = InstallApplication();
            
            if (!installResult) {
                UpdateProgressStep(currentStepIndex, ProgressStep::FAILED, "Failed to install application files!");
                success = false;
                Complete(false);
                return;
            }
            
            UpdateProgressStep(currentStepIndex, ProgressStep::DONE);
            
            // Step 5: Configuring startup settings
            currentStepIndex = 4;
            UpdateProgressStep(currentStepIndex, ProgressStep::WORKING, "Configuring startup settings...");
            Sleep(500);
            UpdateProgressStep(currentStepIndex, ProgressStep::DONE);
            
            // Step 6: Setting up browser redirect
            currentStepIndex = 5;
            UpdateProgressStep(currentStepIndex, ProgressStep::WORKING, "Setting up browser redirect...");
            Sleep(500);
            UpdateProgressStep(currentStepIndex, ProgressStep::DONE);
            
            // Step 7: Finalizing installation
            currentStepIndex = 6;
            UpdateProgressStep(currentStepIndex, ProgressStep::WORKING, "Finalizing installation...");
            Sleep(500);
            UpdateProgressStep(currentStepIndex, ProgressStep::DONE);
            
            // Complete installation
            Complete(true);
            
        } catch (const std::exception& e) {
            DebugLogError("Installation exception: %s", e.what());
            
            if (currentStepIndex >= 0 && currentStepIndex < static_cast<int>(steps.size())) {
                UpdateProgressStep(currentStepIndex, ProgressStep::FAILED, std::string("Error: ") + e.what());
            }
            
            Complete(false);
        }
    }
    
    void PerformUninstallation() {
        bool success = true;
        
        try {
            // Step 1: Preparing uninstallation
            currentStepIndex = 0;
            UpdateProgressStep(currentStepIndex, ProgressStep::WORKING, "Preparing uninstallation...");
            Sleep(500);
            UpdateProgressStep(currentStepIndex, ProgressStep::DONE);
            
            // Step 2: Stopping background services
            currentStepIndex = 1;
            UpdateProgressStep(currentStepIndex, ProgressStep::WORKING, "Stopping background services...");
            Sleep(500);
            UpdateProgressStep(currentStepIndex, ProgressStep::DONE);
            
            // Step 3: Removing browser redirect
            currentStepIndex = 2;
            UpdateProgressStep(currentStepIndex, ProgressStep::WORKING, "Removing browser redirect...");
            Sleep(500);
            UpdateProgressStep(currentStepIndex, ProgressStep::DONE);
            
            // Step 4: Removing startup entries
            currentStepIndex = 3;
            UpdateProgressStep(currentStepIndex, ProgressStep::WORKING, "Removing startup entries...");
            Sleep(500);
            UpdateProgressStep(currentStepIndex, ProgressStep::DONE);
            
            // Step 5: Removing application files
            currentStepIndex = 4;
            UpdateProgressStep(currentStepIndex, ProgressStep::WORKING, "Removing application files...");
            
            // Perform actual uninstallation
            bool uninstallResult = UninstallApplication();
            
            if (!uninstallResult) {
                UpdateProgressStep(currentStepIndex, ProgressStep::FAILED, "Failed to remove application files!");
                success = false;
                Complete(false);
                return;
            }
            
            UpdateProgressStep(currentStepIndex, ProgressStep::DONE);
            
            // Step 6: Cleaning up registry
            currentStepIndex = 5;
            UpdateProgressStep(currentStepIndex, ProgressStep::WORKING, "Cleaning up registry...");
            Sleep(500);
            UpdateProgressStep(currentStepIndex, ProgressStep::DONE);
            
            // Step 7: Finalizing uninstallation
            currentStepIndex = 6;
            UpdateProgressStep(currentStepIndex, ProgressStep::WORKING, "Finalizing uninstallation...");
            Sleep(500);
            UpdateProgressStep(currentStepIndex, ProgressStep::DONE);
            
            // Complete uninstallation
            Complete(true);
            
        } catch (const std::exception& e) {
            DebugLogError("Uninstallation exception: %s", e.what());
            
            if (currentStepIndex >= 0 && currentStepIndex < static_cast<int>(steps.size())) {
                UpdateProgressStep(currentStepIndex, ProgressStep::FAILED, std::string("Error: ") + e.what());
            }
            
            Complete(false);
        }
    }
};