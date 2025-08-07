/**
 * @file msedge_redirect.h
 * @brief Header file for the msedge.exe replacement functionality
 */

#ifndef MSEDGE_REDIRECT_H
#define MSEDGE_REDIRECT_H

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Registry keys and values used by the redirector
 */
#define BROWSER_SANITY_REG_KEY "SOFTWARE\\BrowserSanity"
#define REDIRECT_ENABLED_VALUE "RedirectEnabled"
#define CUSTOM_BROWSER_PATH_VALUE "CustomBrowserPath"
#define CUSTOM_BROWSER_ARGS_VALUE "CustomBrowserArgs"
#define USE_DEFAULT_BROWSER_VALUE "UseDefaultBrowser"

/**
 * @brief Configuration structure for the redirector
 */
typedef struct {
    BOOL redirectEnabled;
    BOOL useDefaultBrowser;
    char customBrowserPath[MAX_PATH];
    char customBrowserArgs[1024];
} RedirectConfig;

/**
 * @brief Reads the configuration from the registry
 * 
 * @param config Pointer to a RedirectConfig structure to fill
 * @return TRUE if successful, FALSE otherwise
 */
BOOL ReadRedirectConfig(RedirectConfig* config);

/**
 * @brief Gets the path to the default browser
 * 
 * @param browserPath Buffer to store the path
 * @param bufferSize Size of the buffer
 * @return TRUE if successful, FALSE otherwise
 */
BOOL GetDefaultBrowserPath(char* browserPath, DWORD bufferSize);

/**
 * @brief Launches the browser with the specified URL
 * 
 * @param config The redirect configuration
 * @param commandLine The command line arguments (usually the URL)
 * @return TRUE if successful, FALSE otherwise
 */
BOOL LaunchBrowser(const RedirectConfig* config, LPSTR commandLine);

#endif /* MSEDGE_REDIRECT_H */