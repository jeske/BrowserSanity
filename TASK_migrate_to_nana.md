# Nana GUI Migration Task List

## Overview
Complete migration from Nuklear immediate mode GUI to Nana retained mode GUI framework.

**Migration Status**: In Progress - Basic MainLaunch window working, need to complete all dialogs and functionality

**Why Nana?** After 14+ hours fighting Nuklear's multi-window context management issues, assertion failures, and crashes, Nana provides a clean, stable, modern C++ GUI solution with automatic resource management.

---

## File-by-File Migration Tasks

### 1. MainLaunch Window → NanaMainLaunchWindow (COMPLETED ✅)
**Functionality**: Primary application interface with status and controls

- [x] Create NanaMainLaunchWindow class in main_nana.cpp
- [x] Implement window layout using Nana place manager
- [x] Port status display logic (running/installed/portable mode detection)
- [x] Convert installation path display to Nana labels
- [x] Convert version display to Nana labels
- [x] Convert PID display to Nana labels
- [x] Implement "Settings" button action (placeholder)
- [x] Implement "Show Notification" button action (placeholder)
- [x] Implement "Install/Uninstall" button functionality
- [x] Implement "Exit" button → proper application shutdown
- [x] Handle window close events properly
- [x] Test MainLaunch window rendering and basic functionality

### 2. Settings Window → NanaSettingsWindow
**Functionality**: Main configuration dialog

- [ ] Expand NanaSettingsWindow class with full UI layout
- [ ] Implement browser selection group using Nana group widget
- [ ] Convert browser dropdown to Nana combox (Chrome, Firefox, Edge, etc.)
- [ ] Implement startup options group
- [ ] Convert "Start Browser Sanity with Windows" checkbox
- [ ] Implement site exceptions group
- [ ] Convert exception list to Nana listbox with scroll
- [ ] Implement "Add Exception" button and input dialog
- [ ] Implement "Remove Exception" button functionality
- [ ] Convert OK/Cancel/Apply buttons with proper layout
- [ ] Implement settings change tracking and Apply button enable/disable
- [ ] Port configuration save/load logic integration
- [ ] Test settings dialog rendering and all functionality

### 3. Toast/Notification Window → NanaToastWindow
**Functionality**: Non-intrusive notification display

- [ ] Expand NanaToastWindow class with proper notification UI
- [ ] Implement auto-positioning (bottom-right corner)
- [ ] Add notification icon display
- [ ] Implement title and message text layout
- [ ] Add auto-dismiss timer functionality
- [ ] Implement fade-in/fade-out animations (if supported by Nana)
- [ ] Add click-to-dismiss functionality
- [ ] Implement notification queue for multiple messages
- [ ] Test toast notifications with various message types
- [ ] Integrate with application events (install complete, errors, etc.)

### 4. Progress Dialog → NanaProgressWindow
**Functionality**: Install/uninstall progress tracking

- [ ] Create NanaProgressWindow class
- [ ] Implement progress step list display using Nana listbox
- [ ] Add status indicators ([PENDING], [WORKING], [DONE], [FAILED])
- [ ] Implement Nana progress bar widget
- [ ] Add status text display area
- [ ] Convert Cancel/Close button logic
- [ ] Implement InitializeProgressSteps() for install vs uninstall
- [ ] Implement UpdateProgressStep() for real-time updates
- [ ] Port step execution threading integration
- [ ] Add proper modal dialog behavior
- [ ] Test progress dialog with actual install/uninstall operations

### 5. Installation Info Dialog → NanaInstallationInfoWindow
**Functionality**: Welcome/installation information dialog

- [ ] Create NanaInstallationInfoWindow class
- [ ] Implement application icon display using Nana picture widget
- [ ] Add title text ("Browser Sanity") with proper styling
- [ ] Display version/build info text
- [ ] Implement rich description text with formatting
- [ ] Convert "View on GitHub" button → ShellExecute functionality
- [ ] Add credits text display area
- [ ] Convert Install/Exit buttons with proper focus handling
- [ ] Implement proper modal dialog behavior
- [ ] Test installation info dialog and all actions

---

## Core Infrastructure Tasks

### 6. Application Architecture Integration
- [ ] Update main.cpp to use Nana event loop instead of Windows message loop
- [ ] Integrate C backend functions with C++ Nana frontend
- [ ] Implement proper application state management
- [ ] Add window management system for multiple dialogs
- [ ] Implement inter-window communication system
- [ ] Add proper application shutdown handling

### 7. Configuration System Integration
- [ ] Ensure AppConfig structure compatibility with Nana UI
- [ ] Implement settings persistence with new UI system
- [ ] Add configuration validation and error handling
- [ ] Test configuration save/load with all dialog types
- [ ] Implement configuration change notifications

### 8. Event System and Actions
- [ ] Port all button click handlers to Nana event system
- [ ] Implement proper error handling and user feedback
- [ ] Add confirmation dialogs for destructive actions
- [ ] Implement proper threading for long-running operations
- [ ] Add progress feedback for all operations

### 9. Visual Polish and UX
- [ ] Implement consistent styling across all windows
- [ ] Add proper window icons and branding
- [ ] Implement proper keyboard navigation and shortcuts
- [ ] Add tooltips for complex UI elements
- [ ] Ensure proper DPI scaling (Nana handles automatically)
- [ ] Test accessibility features

### 10. Legacy System Cleanup
- [ ] Archive old Nuklear implementation files to Archive/nuklear_old/
- [ ] Remove Nuklear dependencies from project files
- [ ] Clean up old UI function declarations
- [ ] Remove Windows API dialog dependencies
- [ ] Update project documentation

---

## Priority Order (Most Critical First)

1. **Settings Window Implementation** (core functionality needed)
2. **Toast/Notification System** (user feedback essential)
3. **Progress Dialog** (install/uninstall operations)
4. **Installation Info Dialog** (new user experience)
5. **Application Architecture Integration** (proper main.cpp integration)
6. **Configuration System Integration** (settings persistence)
7. **Event System and Actions** (complete functionality)
8. **Visual Polish and UX** (professional appearance)
9. **Legacy System Cleanup** (code maintenance)

---

## Success Criteria

- [ ] All original dialog functionality preserved and enhanced
- [ ] No Nuklear dependencies remaining
- [ ] No multi-window context management issues
- [ ] All dialogs render correctly and respond to user input
- [ ] Installation/uninstallation workflows function properly
- [ ] Settings save/load maintains compatibility
- [ ] Application startup logic works with new Nana system
- [ ] Clean, maintainable C++ code architecture
- [ ] Proper error handling and user feedback
- [ ] Professional, consistent user interface

---

## Technical Benefits of Nana Migration

- **Stability**: No more assertion failures or context conflicts
- **Modern C++**: RAII, smart pointers, exception safety
- **Automatic Resource Management**: No manual cleanup required
- **DPI Awareness**: Built-in high-DPI support
- **Layout Management**: Powerful place layout system
- **Event Handling**: Clean, type-safe event system
- **Threading**: Proper thread-safe UI updates
- **Maintainability**: Clear class hierarchy and separation of concerns

---

## Architecture Notes

- **Retained Mode**: UI elements persist between frames (vs Nuklear's immediate mode)
- **Class-Based**: Each window is a C++ class with proper encapsulation
- **Event-Driven**: Nana handles all input processing and event dispatching
- **Layout System**: Place layout manager handles complex layouts automatically
- **Resource Management**: Automatic cleanup via RAII and smart pointers
- **Integration**: Clean C++/C boundary with extern "C" functions for compatibility

---

## Current Status