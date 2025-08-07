# Nuklear UI Migration Task List

## Overview
Complete migration from Windows API dialogs to Nuklear immediate mode GUI framework.

**Migration Status**: In Progress - Nuklear framework integrated, empty window issue needs resolution

---

## File-by-File Migration Tasks

### 1. Archive/old_ui_system/dialog_manualLaunch.c → dialog_manual_launch.c
**Functionality**: Manual launch status dialog (201 lines)

- [ ] Create `dialog_manual_launch.c` following naming convention
- [ ] Implement `RenderManualLaunchDialog()` function signature
- [ ] Port status display logic (running/installed/portable mode detection)
- [ ] Convert installation path display to Nuklear text
- [ ] Convert version display to Nuklear text  
- [ ] Convert PID display to Nuklear text
- [ ] Implement "Show Settings" button action
- [ ] Implement "Uninstall" button → launch progress dialog
- [ ] Implement "Exit" button → ExitProcess(0)
- [ ] Handle window close/destroy events properly
- [ ] Test manual launch dialog rendering and functionality

### 2. Archive/old_ui_system/dialog_mainSettings.c → dialog_main_settings.c  
**Functionality**: Main configuration dialog (244 lines)

- [ ] Create `dialog_main_settings.c` following naming convention
- [ ] Implement `RenderMainSettingsDialog()` function signature
- [ ] Port browser selection group box
- [ ] Convert browser dropdown to Nuklear combo box (Chrome, Firefox, Brave, Opera)
- [ ] Port startup options group box
- [ ] Convert "Start Browser Sanity with Windows" checkbox
- [ ] Port site exceptions group box  
- [ ] Convert exception list to Nuklear listbox
- [ ] Implement "Add Exception" button and dialog
- [ ] Implement "Remove Exception" button functionality
- [ ] Convert OK/Cancel/Apply buttons
- [ ] Implement settings change tracking and Apply button enable/disable
- [ ] Port configuration save/load logic
- [ ] Test main settings dialog rendering and functionality

### 3. Archive/old_ui_system/dialog_installanduninstallprogress.c → dialog_progress.c
**Functionality**: Install/uninstall progress tracking (267 lines)

- [ ] Create `dialog_progress.c` following naming convention  
- [ ] Implement `RenderProgressDialog()` function signature
- [ ] Port ProgressStep structure and data management
- [ ] Convert progress list to Nuklear list with status indicators
- [ ] Implement status text formatting ([PENDING], [WORKING], [DONE], [FAILED])
- [ ] Convert progress bar to Nuklear progress widget
- [ ] Port status text display
- [ ] Convert Cancel/Close button logic
- [ ] Implement `InitializeProgressSteps()` for install vs uninstall
- [ ] Implement `UpdateProgressStep()` for real-time updates
- [ ] Port step execution threading (TODO from original)
- [ ] Test progress dialog rendering and step updates

### 4. Archive/old_ui_system/dialog_manualLaunchInstall.c → dialog_installation_info.c
**Functionality**: Installation information/welcome dialog (231 lines)

- [ ] Create `dialog_installation_info.c` following naming convention
- [ ] Implement `RenderInstallationInfoDialog()` function signature
- [ ] Port application icon display
- [ ] Convert title text ("Browser Sanity") 
- [ ] Convert version/build info text display
- [ ] Port rich description text with bullet points
- [ ] Convert "View on GitHub" button → ShellExecute functionality
- [ ] Convert credits text display
- [ ] Convert Install/Exit buttons
- [ ] Port button focus handling (default Install button)
- [ ] Remove DPI-aware font scaling (Nuklear handles this)
- [ ] Test installation info dialog rendering and actions

### 5. Archive/old_ui_system/dialog_status_old.c → dialog_status.c
**Functionality**: Enhanced status dialog (383 lines)

- [ ] Create `dialog_status.c` following naming convention
- [ ] Implement `RenderStatusDialog()` function signature  
- [ ] Port application icon display
- [ ] Convert status message text display
- [ ] Port installation path label/value display
- [ ] Port status label/value display (Active PID vs Not Running)
- [ ] Port version label/value display
- [ ] Convert "Show Settings" button (disabled if not running)
- [ ] Convert "Exit" button functionality
- [ ] Remove complex Windows API layout calculations
- [ ] Port modal dialog behavior
- [ ] Test status dialog rendering and button actions

### 6. Archive/old_ui_system/UI_Utilities.c → [DEPRECATED - Remove DPI Functions]
**Functionality**: DPI awareness utilities (255 lines) - NOT NEEDED WITH NUKLEAR

- [x] **SKIP MIGRATION** - Nuklear handles DPI automatically
- [ ] Remove dependencies on `GetEffectiveDPI()`, `CreateScaledFont()`, `ScaleForDPI()`
- [ ] Remove dependencies on `MeasureTextWidth()`, `MeasureTextHeight()`, `CalculateButtonSize()`
- [ ] Remove dependencies on `InitializeDPIAwareness()`
- [ ] Update header file to remove UI_Utilities function declarations
- [ ] Clean up any remaining UI_Utilities.c references in project files

### 7. src/browser-sanity/ui/ui.c → [REFACTOR INTO MULTIPLE DIALOGS]
**Functionality**: Monolithic UI with full settings interface (608 lines)

- [ ] Break down `InitUI()` function into separate dialog functions
- [ ] Extract installation UI → move to `dialog_installation_info.c`
- [ ] Extract settings UI → move to `dialog_main_settings.c`  
- [ ] Extract status display → move to `dialog_status.c`
- [ ] Port browser path browsing → convert to Nuklear file dialog
- [ ] Port checkbox state management (redirect, startup, watchdog)
- [ ] Port radio button state management (default vs custom browser)
- [ ] Port text field management (custom browser path, args)
- [ ] Convert Install/Uninstall/Apply buttons
- [ ] Port settings save/load integration
- [ ] Remove Windows API control creation and management
- [ ] Test refactored dialog components

---

## Core Infrastructure Tasks

### 8. Fix Current Nuklear Rendering Issues
- [ ] **CRITICAL**: Debug empty window problem in current Nuklear implementation
- [ ] Investigate minimal GDI backend completeness (`nk_gdi_render()` implementation)
- [ ] Verify Nuklear context initialization in `nuklear_ui.c`
- [ ] Test basic text rendering in Nuklear ("Hello World" test)
- [ ] Verify window message handling integration
- [ ] Ensure proper Nuklear input processing

### 9. UIState Structure Enhancement
- [ ] Expand `UIState` structure in `browser_sanity.h` to support all dialog states
- [ ] Add state fields for each dialog type (manual launch, settings, progress, etc.)
- [ ] Add dialog navigation state management
- [ ] Add shared data between dialogs (configuration, status info)
- [ ] Update all dialog functions to accept UIState parameter

### 10. Dialog Navigation System  
- [ ] Implement dialog switching mechanism in main Nuklear render loop
- [ ] Add dialog enumeration (enum DialogType)
- [ ] Implement dialog stack for modal behavior
- [ ] Add dialog transition functions (open/close/switch)
- [ ] Integrate with existing application logic (main.c)

### 11. Integration and Testing
- [ ] Update `main.c` to use new dialog system instead of old `InitUI()`
- [ ] Remove all old UI function calls from main.c
- [ ] Update function declarations in `browser_sanity.h`
- [ ] Update project file (BrowserSanity.vcxproj) to include all new dialog files  
- [ ] Remove old UI files from compilation
- [ ] Comprehensive testing of all dialog workflows

---

## Priority Order (Most Critical First)

1. **Fix empty window rendering issue** (blocking all progress)
2. **Manual launch dialog** (primary entry point)  
3. **Installation info dialog** (new user experience)
4. **Main settings dialog** (core functionality)
5. **Status dialog** (running application interface)  
6. **Progress dialog** (install/uninstall operations)
7. **UI cleanup and old system removal**

---

## Success Criteria

- [ ] All original dialog functionality preserved
- [ ] No Windows API dialog dependencies remaining  
- [ ] Proper DPI scaling without custom code (Nuklear automatic)
- [ ] All dialogs render correctly and respond to user input
- [ ] Installation/uninstallation workflows function properly
- [ ] Settings save/load maintains compatibility
- [ ] Application startup logic works with new dialog system
- [ ] Clean project structure with proper `dialog_(purpose).c` naming

---

## Notes

- **Nuklear Benefits**: Automatic DPI handling, simpler immediate mode code, no complex Windows API layout
- **Architecture**: Each dialog becomes a single `Render*Dialog()` function in its own file
- **State Management**: All dialog state goes through central `UIState` structure  
- **Rendering**: Single main render loop dispatches to appropriate dialog render function
- **Input**: Nuklear handles all input processing automatically