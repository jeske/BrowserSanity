#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Tiny msedge.exe replacement that redirects to Chrome
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    char chromePath[MAX_PATH];
    char commandLine[4096];
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    
    printf("MSEdge Redirector...\r\n");
    // Try to find Chrome in common locations
    const char* chromePaths[] = {
        "C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe",
        "C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe",
        NULL
    };
    
    // Check AppData path dynamically
    char userChrome[MAX_PATH];
    if (GetEnvironmentVariable("LOCALAPPDATA", userChrome, MAX_PATH)) {
        strcat(userChrome, "\\Google\\Chrome\\Application\\chrome.exe");
    }
    
    // Find Chrome executable
    chromePath[0] = '\0';
    
    // Check user Chrome first
    if (GetFileAttributes(userChrome) != INVALID_FILE_ATTRIBUTES) {
        strcpy(chromePath, userChrome);
    } else {
        // Check system Chrome installations
        for (int i = 0; chromePaths[i] != NULL; i++) {
            if (GetFileAttributes(chromePaths[i]) != INVALID_FILE_ATTRIBUTES) {
                strcpy(chromePath, chromePaths[i]);
                break;
            }
        }
    }
    
    // Build command line
    if (strlen(chromePath) > 0) {
        snprintf(commandLine, sizeof(commandLine), "\"%s\" %s", chromePath, lpCmdLine);
    } else {
        // Fallback: use default browser
        snprintf(commandLine, sizeof(commandLine), "rundll32.exe url.dll,FileProtocolHandler %s", lpCmdLine);
    }
    
    // Initialize process structures
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    
    // Launch Chrome
    if (CreateProcess(NULL, commandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        // Don't wait - just launch and exit
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return 0;
    } else {
        // If Chrome launch failed, try default browser association
        ShellExecute(NULL, "open", lpCmdLine, NULL, NULL, SW_SHOWNORMAL);
        return 1;
    }
}