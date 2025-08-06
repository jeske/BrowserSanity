# Browser Sanity Development Guide

This document provides detailed information for developers working on the Browser Sanity project.

## Project Overview

Browser Sanity is a Windows application that ensures browser choice preferences are respected by intercepting Microsoft Edge launches and redirecting them to the user's preferred browser.

The project consists of two main components:

1. **msedge.exe replacement**: A lightweight executable that redirects to your default browser
2. **BrowserSanity.exe**: A management application with settings UI, installer/uninstaller, and watchdog functionality

## Project Structure

```
/
├── build/                  # Build output directory
├── src/                    # Source code
│   ├── msedge-redirect/    # msedge.exe replacement code
│   └── browser-sanity/     # BrowserSanity.exe application code
│       ├── installer/      # Installer/uninstaller functionality
│       ├── ui/             # User interface components
│       └── watchdog/       # Background monitoring functionality
├── include/                # Header files
├── res/                    # Resource files (icons, etc.)
├── tools/                  # Build tools and scripts
└── docs/                   # Documentation
```

## Building the Project

### Prerequisites

- [TDM-GCC](https://jmeubank.github.io/tdm-gcc/) - C/C++ compiler for Windows
- Make (optional, but recommended)

### Build Process

1. Clone the repository
2. Run `build.bat` from the command line
3. The compiled executables will be placed in the `build` directory

The build script will check for the required dependencies and provide instructions if any are missing.

### Build System

The project uses a Makefile-based build system with tdm-gcc as the compiler. The main build targets are:

1. `msedge.exe` - The lightweight Edge replacement
2. `BrowserSanity.exe` - The main application with UI and management features

If Make is not available, the build script will fall back to direct GCC commands.

## Component Details

### msedge-redirect

The msedge-redirect component is a lightweight executable that replaces the original Microsoft Edge executable. When an application attempts to launch Edge, the redirector intercepts the request and launches the user's preferred browser instead.

Key files:
- `include/msedge_redirect.h` - Header file with function declarations
- `src/msedge-redirect/msedge-redirect.c` - Implementation file

Configuration is stored in the Windows Registry under `HKEY_LOCAL_MACHINE\SOFTWARE\BrowserSanity`.

### BrowserSanity Core

The core functionality of the BrowserSanity application handles configuration management, registry interaction, and redirector management.

Key files:
- `include/browser_sanity.h` - Header file with function declarations
- `src/browser-sanity/browser_sanity.c` - Implementation file

### Installer/Uninstaller

The installer/uninstaller component handles the installation and removal of the BrowserSanity application, including creating shortcuts, setting up startup entries, and managing the redirector.

Key files:
- `src/browser-sanity/installer/installer.c` - Implementation file

### User Interface

The UI component provides a graphical interface for configuring the BrowserSanity application.

Key files:
- `src/browser-sanity/ui/ui.c` - Implementation file

### Watchdog

The watchdog component monitors the redirector to ensure it hasn't been tampered with or removed. If it detects any issues, it can automatically repair the redirector or notify the user.

Key files:
- `src/browser-sanity/watchdog/watchdog.c` - Implementation file

### Main Application

The main application ties all the components together and handles command-line arguments for different modes of operation.

Key files:
- `src/browser-sanity/main.c` - Main entry point

## Registry Configuration

The application uses the following registry keys and values:

- `HKEY_LOCAL_MACHINE\SOFTWARE\BrowserSanity`
  - `InstallPath` - Installation directory
  - `Version` - Application version
  - `RunAtStartup` - Whether to run at startup
  - `WatchdogEnabled` - Whether the watchdog is enabled
  - `WatchdogIntervalSeconds` - Watchdog check interval
  - `RedirectEnabled` - Whether redirection is enabled
  - `UseDefaultBrowser` - Whether to use the default browser
  - `CustomBrowserPath` - Path to a custom browser
  - `CustomBrowserArgs` - Custom browser arguments

## Application Modes

The application can run in several modes:

1. **Normal Mode** - Shows the settings UI
2. **Installer Mode** - Runs the installer (triggered by `/install` command-line argument)
3. **Uninstaller Mode** - Runs the uninstaller (triggered by `/uninstall` command-line argument)
4. **Watchdog Mode** - Runs the watchdog (triggered by `/watchdog` command-line argument)

## Contributing

When contributing to the project, please follow these guidelines:

1. Use consistent coding style
2. Add comments for complex functionality
3. Update documentation when making significant changes
4. Test thoroughly before submitting changes

## Testing

To test the application:

1. Build the project
2. Install the application using the UI or `/install` command-line argument
3. Verify that the redirector works by launching applications that would normally open Edge
4. Test the watchdog by temporarily replacing the redirector and verifying it gets repaired
5. Test the uninstaller to ensure it cleans up properly

## Future Improvements

Potential areas for improvement:

1. Add support for more browsers
2. Improve the UI with a modern look and feel
3. Add localization support
4. Create an MSI installer package
5. Add telemetry for tracking redirection statistics (opt-in)