/**
 * @file actions.h
 * @brief Action function declarations
 */

#ifndef ACTIONS_H
#define ACTIONS_H

#include <windows.h>

/**
 * @brief Runs the application in installer mode
 * @return Exit code
 */
int RunInstallerAction();

/**
 * @brief Runs the application in uninstaller mode
 * @return Exit code
 */
int RunUninstallerAction();

/**
 * @brief Runs the application in watchdog mode
 * @return Exit code
 */
int RunWatchdogAction();

/**
 * @brief Shows the enhanced status dialog
 * @param hwndParent Parent window handle
 * @param isRunning Whether Browser Sanity is currently running
 * @param isInstalled Whether Browser Sanity is installed
 * @param runningPID Process ID if running
 * @return Dialog result (IDYES = Show Settings, IDNO = Exit, IDCANCEL = Restart/Launch/Install)
 */
int ShowEnhancedStatusDialog(HWND hwndParent, BOOL isRunning, BOOL isInstalled, DWORD runningPID);

#endif // ACTIONS_H