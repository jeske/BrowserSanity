# Browser Sanity Project Structure

AI: you should keep this file up to date. If you make changes to something that should be listed here, please edit this file to list it here.

## Overview

Browser Sanity is a Windows application that ensures browser choice preferences are respected by intercepting Microsoft Edge launches and redirecting them to the user's preferred browser.

## Directory Structure

```
/
├── Archive/                    # Archived/old code
│   └── old_nuklear_ui_files/   # Previous Nuklear UI implementation (archived)
│       ├── NuklearCPP/         # C++ Nuklear wrapper classes
│       ├── ui/                 # Legacy Nuklear UI components
│       └── *.cpp               # Old implementation files
├── src/                        # Source code
│   ├── msedge-redirect/        # msedge.exe replacement code
│   └── browser-sanity/         # BrowserSanity.exe application code
│       ├── actions/            # Action implementations (install, uninstall, resources, watchdog)
│       ├── ui/                 # NEW: Nana UI window implementations
│       │   ├── nana_initialLaunchInstall.cpp  # Installer window (info, GitHub, install)
│       │   ├── nana_settings.cpp   # Settings window (redirect status, uninstall)
│       │   ├── nana_installUninstallProgress.cpp   # Install/uninstall progress window
│       │   ├── nana_toast.cpp      # Toast notification window
│       │   └── nana_common_style.hpp  # Common styling for all UI components
│       ├── watchdog/           # Background monitoring functionality
│       ├── main_nana.cpp       # NEW: Nana-based main entry point
│       └── main.cpp            # Original main entry point
├── deps/                       # Dependencies
│   └── nana/                   # Nana GUI library (git submodule)
├── include/                    # Header files
├── res/                        # Resource files (icons, embedded binaries)
├── docs/                       # Documentation
│   ├── DEVELOPMENT.md          # Development guidelines
│   ├── USER_GUIDE.md           # User documentation
│   ├── SafeStringHandling_Implementation.md # Safe string handling details
│   └── history/                # Historical project documents
│       └── Initial-Project-Spec.md # Original project specification
├── STRATEGIES_FOR_AI/          # AI development guidance documents
├── BrowserSanity_Nana.vcxproj  # NEW: Nana-based Visual Studio project
├── BrowserSanity.vcxproj       # Original Nuklear-based project
└── *.sln, build_nana.bat      # Build files
```

## Architecture & Naming Conventions

### UI Framework Migration: Nuklear → Nana (COMPLETED)

**MIGRATION COMPLETED**: The project has successfully migrated from **Nuklear** immediate mode GUI to **Nana** retained mode GUI framework, resolving all multi-window context management issues.

**Previous Nuklear Issues (RESOLVED)**:
- ✅ Complex multi-window context management eliminated
- ✅ Assertion failures with multiple windows resolved
- ✅ 14+ hours of troubleshooting issues eliminated
- ✅ Immediate mode complexity for multi-window applications resolved

**New Nana Implementation** (`src/browser-sanity/main_nana.cpp` + individual window files):
- **Framework**: Nana C++ GUI library (retained mode)
- **Benefits**: Automatic resource management, place layout system, no context conflicts
- **Architecture**: Clean window classes with RAII and smart pointers
- **Organization**: Individual files for each window type (modular design)

### Window Architecture (Nana Implementation)

Based on original project specification, the application provides these windows:

1. **nana_initialLaunchInstall.cpp** - `NanaInitialLaunchInstallWindow`
   - Shows when run from outside Program Files
   - Application information and purpose description
   - Clickable GitHub link (https://github.com/jeske/BrowserSanity)
   - Version checking with update button capability
   - Install button for setup process

2. **nana_settings.cpp** - `NanaSettingsWindow`
   - Shows when already installed in Program Files
   - Redirect installation status display
   - Install/Remove redirect buttons
   - Startup options control ("Start with Windows")
   - Uninstall functionality with confirmation

3. **nana_installUninstallProgress.cpp** - `NanaInstallUninstallProgressWindow`
   - Shows during install/uninstall operations
   - Progress bar with percentage completion
   - Dynamic status messages for each step
   - Cancellation support (when safe to cancel)
   - Success/failure reporting with appropriate messaging

4. **nana_toast.cpp** - `NanaToastWindow`
   - For watchdog notifications when redirect is tampered
   - Alert message display
   - "Repair Redirect" and "Open Settings" action buttons
   - Dismissible notifications

### Actions Directory (`src/browser-sanity/actions/`)
Modular action implementations following the pattern `action_{name}.c`:

- `action_install.c` - Application installation logic
  - Handles the complete installation process of Browser Sanity
  - Creates installation directory in Program Files
  - Copies executable to installation location
  - Creates desktop and start menu shortcuts
  - Updates application configuration in registry
  - Installs the msedge.exe redirector
  - Handles privilege elevation for administrative tasks
  - Launches the installed application after successful installation

- `action_uninstall.c` - Application removal logic
  - Performs complete uninstallation of Browser Sanity
  - Removes the msedge.exe redirector
  - Disables startup configuration
  - Removes desktop and start menu shortcuts
  - Deletes registry entries
  - Uses a self-deletion mechanism for removing installation files
  - Handles privilege elevation for administrative tasks

- `action_resources.c` - Resource extraction utilities
  - Extracts embedded msedge.exe binary from application resources
  - Handles resource loading and writing to disk
  - Manages Windows resource APIs for binary extraction

- `action_watchdog.c` - Background monitoring service
  - Implements the watchdog mode of the application
  - Reads configuration to determine if watchdog is enabled
  - Starts the watchdog monitoring service
  - Maintains a Windows message loop for the watchdog process

### Watchdog Implementation (`src/browser-sanity/watchdog/`)

- `watchdog.c` - Core watchdog functionality
  - Implements a background thread that monitors the msedge.exe redirector
  - Periodically checks if the redirector has been tampered with or removed
  - Shows toast notifications when tampering is detected
  - Automatically repairs the redirector when necessary
  - Provides functions to start, stop, and configure the watchdog service

### Development Principles
- **Single Responsibility**: Each window handles one specific purpose
- **Modular Design**: Windows are in separate files, actions can be called independently
- **Consistent Naming**: Files named by their primary function using descriptive terms
- **Retained Mode UI**: Nana provides automatic DPI scaling and eliminates complex context management
- **RAII Resource Management**: Modern C++ patterns with automatic cleanup

### Nana Integration Details
- **Library Location**: Nana source integrated as git submodule in `deps/nana/`
- **Submodule Setup**: Configured in `.gitmodules` file for proper version control
- **Build Integration**: All Nana source files included in `BrowserSanity_Nana.vcxproj`
- **Layout System**: Uses Nana's place layout manager for automatic UI arrangement
- **Event Handling**: Lambda-based event handlers with proper resource management
- **No External Dependencies**: Self-contained build with no additional DLL requirements

## Build System
The project uses Visual Studio project files (`.vcxproj`/`.sln`) with MSVC compiler. Build targets:

1. `msedge-redirect.exe` - The lightweight Edge replacement
2. `BrowserSanity.exe` - The main application with Nana UI and management features

**Build Commands**:
- `build_nana.bat` - Builds the new Nana-based version
- `build.bat` - Builds the original Nuklear-based version (legacy)

## Development Workflow
1. Edit source files in the `src/` directory using organized action/ui structure
2. Run `build_nana.bat` to compile the Nana-based project
3. Test window functionality and action implementations
4. Archive old code in `Archive/` directory rather than deletion for reference

## Project History
The original project specification can be found in `docs/history/Initial-Project-Spec.md`. This document outlines the initial goals of the project:

1. Create a lightweight program that replaces msedge.exe to redirect to the user's preferred browser
2. Develop a main application (BrowserSanity.exe) that serves as:
   - An installer
   - A settings UI
   - An uninstaller
   - A background watchdog

The project initially used Nuklear for the UI, but due to issues with Nuklear's multi-window management (which took 14+ hours of troubleshooting), the decision was made to migrate to Nana for a cleaner, more maintainable codebase.

## Migration Status: COMPLETE ✅
- ✅ Nuklear → Nana migration completed
- ✅ Multi-window context issues resolved
- ✅ Individual window files created and organized
- ✅ Build system updated and functional
- ✅ Original Nuklear code archived for reference
- ✅ Clean, maintainable codebase established