# Browser Sanity User Guide

## Overview

Browser Sanity is a Windows application that ensures your browser choice preferences are respected by intercepting Microsoft Edge launches and redirecting them to your preferred browser.

## Installation

### Automatic Installation

1. Download the latest release of Browser Sanity from the [GitHub repository](https://github.com/yourusername/browser-sanity/releases)
2. Run `BrowserSanity.exe`
3. Click the "Install" button
4. Grant administrator privileges when prompted

The installer will:
- Create a directory in Program Files (x86)
- Copy the necessary files
- Create desktop and start menu shortcuts
- Set up the redirector
- Configure startup settings (if enabled)

### Manual Installation

If you prefer to install manually:

1. Download the latest release
2. Extract the files to a location of your choice
3. Run `BrowserSanity.exe` with administrator privileges
4. Use the settings UI to configure the application

## Using Browser Sanity

### Main Interface

The main interface provides the following options:

- **Enable msedge.exe redirection**: Turn the redirection on or off
- **Use default browser**: Redirect to your system's default browser
- **Use custom browser**: Redirect to a specific browser of your choice
- **Run at Windows startup**: Start Browser Sanity automatically when Windows starts
- **Enable watchdog**: Monitor and repair the redirector if it gets tampered with

### Configuration Options

#### Default Browser Redirection

When "Use default browser" is selected, Browser Sanity will redirect all Microsoft Edge launches to whatever browser is set as your system default.

#### Custom Browser Redirection

When "Use custom browser" is selected, you can:
1. Specify the path to your preferred browser executable
2. Add custom command-line arguments

This is useful for:
- Launching a browser with specific settings
- Using a portable browser that isn't registered as a system browser
- Launching a specific browser profile

### Watchdog Functionality

The watchdog monitors the redirector to ensure it hasn't been tampered with or removed. If it detects any issues, it will:

1. Show a notification
2. Offer to repair the redirector
3. Provide an option to open settings

By default, the watchdog checks every 30 seconds.

## Uninstallation

### From the UI

1. Run `BrowserSanity.exe`
2. Click the "Uninstall" button
3. Confirm the uninstallation
4. Grant administrator privileges when prompted

### From Control Panel

Browser Sanity can also be uninstalled from the Windows Control Panel:

1. Open Control Panel
2. Go to "Programs and Features" or "Apps & features"
3. Find "Browser Sanity" in the list
4. Click "Uninstall"

## Troubleshooting

### Redirection Not Working

If the redirection is not working:

1. Check if Browser Sanity is installed and running
2. Verify that redirection is enabled in the settings
3. Make sure the watchdog is enabled
4. Try reinstalling the redirector by clicking "Apply Settings"

### Edge Updates Removing the Redirector

Microsoft Edge updates may replace the redirector. If this happens:

1. Browser Sanity's watchdog should detect this and offer to repair it
2. If not, open Browser Sanity and click "Apply Settings" to reinstall the redirector

### Elevation Issues

If you encounter permission errors:

1. Make sure you're running Browser Sanity with administrator privileges
2. Check if your antivirus is blocking the application
3. Try reinstalling the application

## Frequently Asked Questions

### Is Browser Sanity safe to use?

Yes, Browser Sanity is open source and does not collect any personal data. It simply redirects browser launches and doesn't modify any system files beyond the Edge executable.

### Will this break Microsoft Edge updates?

No, when Edge updates, it will likely replace our redirector with the new Edge executable. The watchdog will detect this and offer to repair the redirector.

### Can I still use Edge if I need to?

If you need to use Edge for specific purposes, you can temporarily disable the redirection in the Browser Sanity settings.

### Does this work with all applications?

Browser Sanity works with most applications that attempt to launch Edge directly. However, some applications may use different methods to launch Edge that cannot be intercepted.

### Will this affect other browsers?

No, Browser Sanity only affects Microsoft Edge. Other browsers will continue to work normally.

## Support and Feedback

If you encounter any issues or have suggestions for improvement, please:

1. Check the [GitHub repository](https://github.com/yourusername/browser-sanity) for known issues
2. Submit a new issue if your problem isn't already reported
3. Consider contributing to the project if you have development skills