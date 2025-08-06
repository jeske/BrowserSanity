# Browser Sanity Project Structure

## Overview
Browser Sanity is a Windows application that ensures browser choice preferences are respected by intercepting Microsoft Edge launches and redirecting them to the user's preferred browser.

## Directory Structure

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

## Build System
The project uses a Makefile-based build system with tdm-gcc as the compiler. The main build targets are:

1. `msedge-redirect.exe` - The lightweight Edge replacement
2. `BrowserSanity.exe` - The main application with UI and management features

## Development Workflow
1. Edit source files in the `src/` directory
2. Run `build.bat` to compile the project
3. Output executables will be placed in the `build/` directory