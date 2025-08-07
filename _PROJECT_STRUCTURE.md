# Browser Sanity Project Structure

AI: you should keep this file up to date. If you make changes to something that should be listed here, please edit this file to list it here.

## Overview

Browser Sanity is a Windows application that ensures browser choice preferences are respected by intercepting Microsoft Edge launches and redirecting them to the user's preferred browser.

## Directory Structure

```
/
├── Archive/                # Archived/old code (installer/ contents moved here)
├── src/                    # Source code
│   ├── msedge-redirect/    # msedge.exe replacement code
│   └── browser-sanity/     # BrowserSanity.exe application code
│       ├── actions/        # Action implementations (install, uninstall, resources, watchdog)
│       ├── ui/             # User interface dialog components
│       └── watchdog/       # Background monitoring functionality
├── include/                # Header files
├── res/                    # Resource files (icons, embedded binaries)
├── docs/                   # Documentation
└── *.vcxproj, *.sln       # Visual Studio project files
```

## Architecture & Naming Conventions

### Actions Directory (`src/browser-sanity/actions/`)
Modular action implementations following the pattern `action_{name}.c`:
- `action_install.c` - Application installation logic
- `action_uninstall.c` - Application removal logic  
- `action_resources.c` - Resource extraction (msedge binary)
- `action_watchdog.c` - Background monitoring service

### UI Directory (`src/browser-sanity/ui/`)
**UI Framework Migration**: The project has migrated from manual Windows API dialogs to **Nuklear** immediate mode GUI framework for automatic DPI scaling and simplified UI code.

**Nuklear Integration**:
- **Header Location**: [`include/nuklear.h`](include/nuklear.h) (single-header library)
- **File Size**: ~640K lines (too large to read directly, use external documentation)
- **GitHub Source**: https://github.com/Immediate-Mode-UI/Nuklear
- **Documentation**: https://immediate-mode-ui.github.io/Nuklear/
- **Benefits**: Automatic DPI scaling, immediate mode rendering, much simpler code vs manual Windows API

**Legacy Dialog Files** (archived in `Archive/old_ui_system/`):
- `dialog_manualLaunch.c` - Main status dialog when app run directly
- `dialog_manualLaunchInstall.c` - Installation prompt dialog (when app not found)
- `dialog_mainSettings.c` - Configuration settings dialog
- `dialog_installanduninstallprogress.c` - Unified install/uninstall progress tracking

### Development Principles
- **Single Responsibility**: Each dialog/action handles one specific purpose
- **Consistent Naming**: Files named by their primary function using descriptive terms
- **Modular Design**: Actions can be called independently, dialogs are self-contained
- **Unified Progress**: Install/uninstall operations share common progress tracking to avoid duplication
- **Immediate Mode UI**: Nuklear provides automatic DPI scaling and eliminates complex Windows API dialog management

## Build System
The project uses Visual Studio project files (`.vcxproj`/`.sln`) with MSVC compiler. Build targets:

1. `msedge-redirect.exe` - The lightweight Edge replacement
2. `BrowserSanity.exe` - The main application with UI and management features

## Development Workflow
1. Edit source files in the `src/` directory using organized action/ui structure
2. Run `build.bat` or build through Visual Studio to compile the project
3. Test dialog functionality and action implementations
4. Archive old code in `Archive/` directory rather than deletion for reference