/**
 * @file browser_sanity.h
 * @brief Header file for the BrowserSanity application
 */

#ifndef BROWSER_SANITY_H
#define BROWSER_SANITY_H

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <shlobj.h>
#include <shellapi.h>
#include "msedge_redirect.h"

/**
 * @brief Registry keys and values used by BrowserSanity
 */
#define BROWSER_SANITY_REG_KEY "SOFTWARE\\BrowserSanity"
#define INSTALL_PATH_VALUE "InstallPath"
#define INSTALL_VERSION_VALUE "Version"
#define RUN_AT_STARTUP_VALUE "RunAtStartup"
#define WATCHDOG_ENABLED_VALUE "WatchdogEnabled"
#define WATCHDOG_INTERVAL_VALUE "WatchdogIntervalSeconds"

/**
 * @brief Default values
 */
#define DEFAULT_WATCHDOG_INTERVAL 30
#define BROWSER_SANITY_VERSION "1.0.0"

/**
 * @brief Application modes
 */
typedef enum {
    MODE_NORMAL,       // Normal UI mode
    MODE_INSTALLER,    // Running as installer
    MODE_UNINSTALLER,  // Running as uninstaller
    MODE_WATCHDOG      // Running as watchdog
} AppMode;

/**
 * @brief Application configuration
 */
typedef struct {
    BOOL isInstalled;
    BOOL runAtStartup;
    BOOL watchdogEnabled;
    DWORD watchdogInterval;
    char installPath[MAX_PATH];
    char version[32];
    RedirectConfig redirectConfig;
} AppConfig;

/**
 * @brief Reads the application configuration from the registry
 * 
 * @param config Pointer to an AppConfig structure to fill
 * @return TRUE if successful, FALSE otherwise
 */
BOOL ReadAppConfig(AppConfig* config);

/**
 * @brief Writes the application configuration to the registry
 * 
 * @param config Pointer to an AppConfig structure to write
 * @return TRUE if successful, FALSE otherwise
 */
BOOL WriteAppConfig(const AppConfig* config);

/**
 * @brief Checks if the application is running from the installation directory
 * 
 * @return TRUE if running from installation directory, FALSE otherwise
 */
BOOL IsRunningFromInstallDir();

/**
 * @brief Checks if the msedge.exe redirector is installed
 * 
 * @return TRUE if installed, FALSE otherwise
 */
BOOL IsRedirectorInstalled();

/**
 * @brief Installs the msedge.exe redirector
 * 
 * @return TRUE if successful, FALSE otherwise
 */
BOOL InstallRedirector();

/**
 * @brief Uninstalls the msedge.exe redirector
 * 
 * @return TRUE if successful, FALSE otherwise
 */
BOOL UninstallRedirector();

/**
 * @brief Sets the application to run at startup
 * 
 * @param enable TRUE to enable, FALSE to disable
 * @return TRUE if successful, FALSE otherwise
 */
BOOL SetRunAtStartup(BOOL enable);

/**
 * @brief Installs the application
 * 
 * @return TRUE if successful, FALSE otherwise
 */
BOOL InstallApplication();

/**
 * @brief Uninstalls the application
 * 
 * @return TRUE if successful, FALSE otherwise
 */
BOOL UninstallApplication();

/**
 * @brief Checks if the redirector is intact
 * 
 * @return TRUE if intact, FALSE if tampered with or missing
 */
BOOL IsRedirectorIntact();

/**
 * @brief Shows a toast notification
 *
 * @param title The notification title
 * @param message The notification message
 * @param actions Array of action strings
 * @param actionCount Number of actions
 * @return TRUE if successful, FALSE otherwise
 */
BOOL ShowToastNotification(const char* title, const char* message, const char** actions, int actionCount);

/**
 * @brief Extracts the embedded msedge.exe binary from resources and writes it to disk
 *
 * @param targetPath Path where the binary should be written
 * @return TRUE if successful, FALSE otherwise
 */
BOOL ExtractMsedgeBinary(const char* targetPath);

/**
 * @brief Launches the installed application (for testing)
 * Implemented in installer.c
 *
 * @return TRUE if successful, FALSE otherwise
 */
BOOL LaunchInstalledApplication();

/**
 * @brief Sets the watchdog service enabled/disabled state
 * Implemented in watchdog.c
 *
 * @param enabled TRUE to enable, FALSE to disable
 * @return TRUE if successful, FALSE otherwise
 */
BOOL SetWatchdogEnabled(BOOL enabled);

/**
 * @brief Checks if Browser Sanity is currently running (excluding this process)
 *
 * @return TRUE if another instance is running, FALSE otherwise
 */
BOOL IsProcessRunning();

/**
 * @brief Comprehensive installation check (registry, path, redirector)
 *
 * @return TRUE if Browser Sanity is properly installed, FALSE otherwise
 */
BOOL IsComprehensivelyInstalled();

#endif /* BROWSER_SANITY_H */