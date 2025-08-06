# Browser Sanity

A Windows application that ensures browser choice preferences are respected by intercepting Microsoft Edge launches and redirecting them to the user's preferred browser.

## Problem Statement

Many Windows applications do not respect the default browser settings and directly launch Microsoft Edge. Browser Sanity solves this problem by replacing the Edge executable with a lightweight redirector that launches your preferred browser instead.

## Components

1. **msedge.exe replacement**: A lightweight executable that redirects to your default browser
2. **BrowserSanity.exe**: A management application that:
   - Provides a settings UI
   - Installs/uninstalls the redirector
   - Monitors the redirector to ensure it stays in place
   - Runs at startup (optional)

## Building from Source

### Prerequisites

- [TDM-GCC](https://jmeubank.github.io/tdm-gcc/) - C/C++ compiler for Windows
- Make (optional, but recommended)

### Build Instructions

1. Clone this repository
2. Run `build.bat` from the command line
3. The compiled executables will be placed in the `build` directory

## Installation

### Automatic Installation

1. Run `BrowserSanity.exe`
2. Click the "Install" button
3. Grant administrator privileges when prompted

### Manual Installation

1. Build the project as described above
2. Copy `build/msedge.exe` to `C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe`
   (Make sure to backup the original file first)
3. Copy `build/BrowserSanity.exe` to a location of your choice

## Usage

After installation, any application that attempts to launch Microsoft Edge will be redirected to your default browser automatically.

To configure Browser Sanity:
1. Run `BrowserSanity.exe` from the Start Menu or desktop shortcut
2. Use the settings UI to:
   - Enable/disable the redirector
   - Configure startup options
   - Uninstall the application

## License

This project is open source and available under the MIT License.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.