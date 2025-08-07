/**
 * @file msedge_redirect_lib.c
 * @brief Redirect functionality library (without WinMain)
 */

#include <msedge_redirect.h>
#include <safe_strings.h>
#include <debug_log.h>
#include <shlwapi.h>
#include <shlobj.h>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")

/**
 * @brief Reads the configuration from the registry
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
    if (sprintf_s(szCommand, MAX_PATH, "SOFTWARE\\Classes\\%s\\shell\\open\\command", szProgId) < 0) {
        DebugLogError("Failed to format registry command path");
        return FALSE;
    }
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
    if (szCommand[0] == '"') {
        pExe = strchr(szCommand + 1, '"');
        if (pExe) {
            *pExe = '\0';
            if (strncpy_s(browserPath, bufferSize, szCommand + 1, _TRUNCATE) != 0) {
                DebugLogError("Failed to copy browser path from quoted command");
                return FALSE;
            }
            return TRUE;
        }
    } else {
        pExe = strchr(szCommand, ' ');
        if (pExe) {
            *pExe = '\0';
            if (strncpy_s(browserPath, bufferSize, szCommand, _TRUNCATE) != 0) {
                DebugLogError("Failed to copy browser path from command with space");
                return FALSE;
            }
            return TRUE;
        } else {
            if (strncpy_s(browserPath, bufferSize, szCommand, _TRUNCATE) != 0) {
                DebugLogError("Failed to copy browser path from command");
                return FALSE;
            }
            return TRUE;
        }
    }
    
    return FALSE;
}

/**
 * @brief Launches the browser with the specified URL
 */
BOOL LaunchBrowser(const RedirectConfig* config, LPSTR commandLine) {
    char browserPath[MAX_PATH];
    char fullCommandLine[COMMAND_LINE_SIZE];
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
            if (sprintf_s(fullCommandLine, COMMAND_LINE_SIZE, "rundll32.exe url.dll,FileProtocolHandler %s", commandLine) < 0) {
                DebugLogError("Failed to format rundll32 command line");
                return FALSE;
            }
            
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
            if (strncpy_s(browserPath, sizeof(browserPath), config->customBrowserPath, _TRUNCATE) != 0) {
                DebugLogError("Failed to copy custom browser path");
                return FALSE;
            }
        }
    }
    
    // Build the command line
    if (config->customBrowserArgs[0] != '\0' && !config->useDefaultBrowser) {
        // Use custom arguments
        if (sprintf_s(fullCommandLine, COMMAND_LINE_SIZE, "\"%s\" %s %s", browserPath, config->customBrowserArgs, commandLine) < 0) {
            DebugLogError("Failed to format custom browser command line with args");
            return FALSE;
        }
    } else {
        // Use default arguments
        if (sprintf_s(fullCommandLine, COMMAND_LINE_SIZE, "\"%s\" %s", browserPath, commandLine) < 0) {
            DebugLogError("Failed to format browser command line");
            return FALSE;
        }
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