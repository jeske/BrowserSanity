# Browser Sanity UI Specification

## Overview

Browser Sanity is a Windows application that ensures browser choice preferences are respected by intercepting Microsoft Edge launches and redirecting them to the user's preferred browser. The UI is built using the Nana C++ GUI library, which provides a clean, modern interface with automatic resource management and proper DPI scaling.

## UI Architecture

The application follows a multi-window architecture with each window implemented as a separate C++ class. The main application manages these windows through a central state management system.

### Core Components

1. **Main Application (main_nana.cpp)**
   - Manages application lifecycle
   - Handles window creation and destruction
   - Provides inter-window communication
   - Implements system tray icon and single instance detection
   - Manages configuration persistence
   - Implements watchdog functionality

2. **Common Styling (nana_common_style.hpp)**
   - Provides consistent styling across all windows
   - Defines color scheme, typography, and spacing
   - Implements helper functions for styling UI elements

## Window Types

### 1. Main Launch Window (NanaInstallerWindow)

**Purpose**: Shown when the application is run from outside Program Files, serves as the installation entry point.

**UI Elements**:
- Application icon
- Title ("Browser Sanity")
- Description text explaining the application's purpose
- Features list with bullet points
- "View on GitHub" button
- Version information with update button (when updates are available)
- Credits text
- "Install" button (primary action)
- "Exit" button

**Behavior**:
- Modal dialog (blocks interaction with other windows)
- Clicking "Install" shows the progress dialog
- Clicking "View on GitHub" opens the repository in the default browser
- Clicking "Exit" closes the application

### 2. Settings Window (NanaSettingsWindow)

**Purpose**: Main interface when the application is already installed, allows configuration of application settings.

**UI Elements**:
- Status section showing redirect status
- Install/Remove redirect buttons
- "Start with Windows" checkbox
- Uninstall button
- Exit button

**Behavior**:
- Shows when the application is launched from Program Files
- Can be accessed from the system tray icon
- Clicking "Uninstall" shows a confirmation dialog before proceeding

### 3. Progress Dialog (NanaInstallProgressWindow)

**Purpose**: Shows progress during installation or uninstallation operations.

**UI Elements**:
- Title indicating operation type (install/uninstall)
- Step list showing individual operation steps
- Status indicators ([PENDING], [WORKING], [DONE], [FAILED])
- Progress bar showing overall completion
- Status text showing current operation
- Cancel/Close button

**Behavior**:
- Modal dialog (blocks interaction with other windows)
- Shows detailed progress of multi-step operations
- Updates in real-time as steps are completed
- Cancel button is disabled during critical operations
- Changes to "Close" button when operation completes

### 4. Toast Notification Window (NanaToastWindow)

**Purpose**: Shows non-intrusive notifications, particularly when the redirect has been tampered with.

**UI Elements**:
- Notification icon
- Title text
- Message text
- Action buttons (e.g., "Repair Redirect", "Open Settings")
- Dismiss button

**Behavior**:
- Appears in the bottom-right corner of the screen
- Fades in and out with animations
- Auto-dismisses after a timeout (for non-critical notifications)
- Can be dismissed by clicking anywhere on the notification
- Queues multiple notifications if they arrive simultaneously

## System Tray Integration

**Purpose**: Allows the application to run in the background and be easily accessed.

**UI Elements**:
- System tray icon showing application status
- Context menu with options:
  - Open Settings
  - Exit

**Behavior**:
- Right-click shows context menu
- Double-click opens Settings window
- Provides persistent access to the application when minimized

## Visual Design

### Color Scheme
- **Primary Color**: Blue (#0078D7)
- **Secondary Color**: Windows 10 blue (#0078D7)
- **Accent Color**: Amber (#FFBA00)
- **Success Color**: Green (#5CB85C)
- **Warning Color**: Orange (#F0AD4E)
- **Danger Color**: Red (#D9534F)
- **Info Color**: Light blue (#5BC0DE)
- **Background Color**: White (#FFFFFF)
- **Text Color**: Dark gray (#333333)

### Typography
- **Title**: 16pt, bold
- **Subtitle**: 14pt, bold
- **Normal Text**: 12pt, regular
- **Small Text**: 10pt, regular

### Layout
- Consistent margins (20px) and spacing (15px)
- Responsive layouts that adapt to window resizing
- Proper alignment and grouping of related elements

## Interaction Patterns

### Window Management
- Modal dialogs for operations requiring user attention
- Non-modal windows for general UI
- Proper focus handling for keyboard navigation

### Notifications
- Toast notifications for background events
- Message boxes for critical information
- Status text for operation feedback

### Input Handling
- Button clicks for primary actions
- Checkboxes for boolean settings
- Text fields for string input
- Proper validation and error handling

## Accessibility Considerations

- Proper keyboard navigation support
- High-contrast visual elements
- Text scaling for readability
- Tooltips for complex UI elements

## Future Enhancements

- Browser selection dropdown in Settings window
- Site exceptions management
- More detailed configuration options
- Enhanced visual styling and animations
- Improved error handling and user feedback