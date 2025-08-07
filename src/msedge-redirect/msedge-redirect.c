/**
 * @file msedge-redirect.c
 * @brief Implementation of the msedge.exe replacement
 */

#include <msedge_redirect.h>
#include <shlwapi.h>
#include <shlobj.h>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")

/**
 * @brief Reads the configuration from the registry
 * 
 * @param config Pointer to a RedirectConfig structure to fill
 * @return TRUE if successful, FALSE otherwise
 */
BOOL ReadRedirectConfig(RedirectConfig* config) {
    HKEY hKey;
    DWORD dwType, dwSize;
    DWORD dwValue;
    
    // Set default values
    config->redirectEnabled = TRUE;
    config->useDefaultBrowser = TRUE;
    config->customBrowserPath[0] = '\0';
    config->customBrowserArgs[0] = '\0';
    
    // Try to open the registry key
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, BROWSER_SANITY_REG_KEY, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        // Key doesn't exist, use defaults
        return TRUE;
    }
    
    // Read redirect enabled flag
    dwSize = sizeof(DWORD);
    if (RegQueryValueEx(hKey, REDIRECT_ENABLED_VALUE, NULL, &dwType, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS) {
        if (dwType == REG_DWORD) {
            config->redirectEnabled = (BOOL)dwValue;
        }
    }
    
    // Read use default browser flag
    dwSize = sizeof(DWORD);
    if (RegQueryValueEx(hKey, USE_DEFAULT_BROWSER_VALUE, NULL, &dwType, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS) {
        if (dwType == REG_DWORD) {
            config->useDefaultBrowser = (BOOL)dwValue;
        }
    }
    
    // Read custom browser path
    dwSize = sizeof(config->customBrowserPath);
    RegQueryValueEx(hKey, CUSTOM_BROWSER_PATH_VALUE, NULL, &dwType, (LPBYTE)config->customBrowserPath, &dwSize);
    
    // Read custom browser arguments
    dwSize = sizeof(config->customBrowserArgs);
    RegQueryValueEx(hKey, CUSTOM_BROWSER_ARGS_VALUE, NULL, &dwType, (LPBYTE)config->customBrowserArgs, &dwSize);
    
    RegCloseKey(hKey);
    return TRUE;
}

/**
 * @brief Gets the path to the default browser
 * 
 * @param browserPath Buffer to store the path
 * @param bufferSize Size of the buffer
 * @return TRUE if successful, FALSE otherwise
 */
BOOL GetDefaultBrowserPath(char* browserPath, DWORD bufferSize) {
    HKEY hKey;
    DWORD dwSize;
    char szProgId[64];
    char szCommand[MAX_PATH];
    char* pExe;
    
    // Get the default browser ProgID
    if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\http\\UserChoice", 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return FALSE;
    }
    
    dwSize = sizeof(szProgId);
    if (RegQueryValueEx(hKey, "ProgId", NULL, NULL, (LPBYTE)szProgId, &dwSize) != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return FALSE;
    }
    RegCloseKey(hKey);
    
    // Get the command for the ProgID
    sprintf(szCommand, "SOFTWARE\\Classes\\%s\\shell\\open\\command", szProgId);
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szCommand, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return FALSE;
    }
    
    dwSize = MAX_PATH;
    if (RegQueryValueEx(hKey, NULL, NULL, NULL, (LPBYTE)szCommand, &dwSize) != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return FALSE;
    }
    RegCloseKey(hKey);
    
    // Extract the executable path from the command
    // The command is typically in the format: "C:\path\to\browser.exe" %1
    if (szCommand[0] == '"') {
        // Find the closing quote
        pExe = strchr(szCommand + 1, '"');
        if (pExe) {
            *pExe = '\0';
            strncpy(browserPath, szCommand + 1, bufferSize);
            return TRUE;
        }
    } else {
        // No quotes, find the first space
        pExe = strchr(szCommand, ' ');
        if (pExe) {
            *pExe = '\0';
            strncpy(browserPath, szCommand, bufferSize);
            return TRUE;
        } else {
            // No space, use the whole string
            strncpy(browserPath, szCommand, bufferSize);
            return TRUE;
        }
    }
    
    return FALSE;
}

/**
 * @brief Launches the browser with the specified URL
 * 
 * @param config The redirect configuration
 * @param commandLine The command line arguments (usually the URL)
 * @return TRUE if successful, FALSE otherwise
 */
BOOL LaunchBrowser(const RedirectConfig* config, LPSTR commandLine) {
    char browserPath[MAX_PATH];
    char fullCommandLine[4096];
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    
    // If redirection is disabled, do nothing
    if (!config->redirectEnabled) {
        return FALSE;
    }
    
    // Determine which browser to use
    if (config->useDefaultBrowser) {
        // Use the default browser
        if (!GetDefaultBrowserPath(browserPath, sizeof(browserPath))) {
            // Fallback to rundll32 method if we can't get the default browser
            sprintf(fullCommandLine, "rundll32.exe url.dll,FileProtocolHandler %s", commandLine);
            
            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            ZeroMemory(&pi, sizeof(pi));
            
            if (CreateProcess(NULL, fullCommandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
                return TRUE;
            }
            
            return FALSE;
        }
    } else {
        // Use the custom browser
        if (config->customBrowserPath[0] == '\0') {
            // No custom browser specified, use default
            if (!GetDefaultBrowserPath(browserPath, sizeof(browserPath))) {
                return FALSE;
            }
        } else {
            strncpy(browserPath, config->customBrowserPath, sizeof(browserPath));
        }
    }
    
    // Build the command line
    if (config->customBrowserArgs[0] != '\0' && !config->useDefaultBrowser) {
        // Use custom arguments
        sprintf(fullCommandLine, "\"%s\" %s %s", browserPath, config->customBrowserArgs, commandLine);
    } else {
        // Use default arguments
        sprintf(fullCommandLine, "\"%s\" %s", browserPath, commandLine);
    }
    
    // Launch the browser
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    
    if (CreateProcess(NULL, fullCommandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return TRUE;
    }
    
    // If direct launch failed, try ShellExecute
    ShellExecute(NULL, "open", commandLine, NULL, NULL, SW_SHOWNORMAL);
    
    return FALSE;
}

/**
 * @brief Main entry point for the application
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    RedirectConfig config;
    
    // Read configuration from registry
    ReadRedirectConfig(&config);
    
    // Launch the browser
    LaunchBrowser(&config, lpCmdLine);
    
    return 0;
}