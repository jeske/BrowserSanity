# Project Spec : Browser Sanity

The reason for this project, is that MANY applications on windows DO NOT respect the default internet settings. So when one setups Chrome as their default browser, there are still MANY things that launch msedge.exe. We want to end this TYRRANY! (joke)

Our program will have two parts:

A standalone lightweight compiled windows program that is a drop in replacement for msedge.exe. This program will be copied to C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe, REPLACING the msedge.exe executable, so that any attempt to directly launch edge, will instead directly launch whatever we want. The default behavior will be to launch the system configured default Internet Browser. An alternate behavior (triggered by registry key) will launch whatever the user specifies.. This will be used, for example, to launch chrome with specific command line arguments, instead of the defaults. (There is a prototype of this redirect in msedge-redirect.c)

The second part of the program BrowserSanity.exe, will have a few functions:

1. Act as an "installer" of itself... if run from outside program files, it will present the user with some information about the application, it's purpose, a clickable link to the github repository, it will double check it is the latest version of itself (presenting an "update" button if appropriate) and also present an "Install" button.. Clicking "Install" will (a) ask for elevated permissions, create a directory in program files (x86) Browser Sanity, copy itself there, set itself up to run at user startup, setup a start-menu shortcut, setup a desktop shortcut, relaunch itself (unelevated) from the program files location, then quit
2. Act an a settings ui, allowing the user to install the redirect and display whether the redirect is in place, control whether this application is set to launch on windows startup, or "uninstall" (trigger an uninstall process, more below)
3. act as an uninstaller of iself... if the user chooses to uninstall the program from program files, obviously it can't delete itself while it's still runnning, so it will first (a) resture the normal msedge exe if it needs to, copy BrowserSanity.exe to the system temp directory (like c:\temp) trigger the launch of that copy of the exe with an uninstall paramater, then shut itself down). The other copy of BrowserSanity.exe that runs with /uninstall will then (a) ask for elevated permissions.... check that the BrowserSanity msedge.exe redirect is removed, and (if possible) that real msedge.exe is restored, remove BrowserSanity's registry entries, remove BrowserSanity's program files, and bid the user a happy friendly farewell with a dialog. 
4. Act as a "background watchdog" every 30 seconds checking to see if our msedge.exe redirect has been tampered with, and if so, presenting a toast notification that our msedge redirect has been removed, asking the user if they want to "repair the redirect" or "open settings". 

# Tools and background

- We are using tdm-gcc to compile this project.
- We would like to use the SIMPLEST possible program structure and build system to organize the project and get it to do what we need. We also want it to be "self assisting" in that, if a user downloads the github source and runs the "build.bat" script, it should check for the install of any dependencies and print to the console the web links and instructions to install whatever is necessary.