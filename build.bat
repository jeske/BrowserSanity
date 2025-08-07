@echo off
setlocal enabledelayedexpansion

echo Browser Sanity Build Script (Nana Version)
echo ==========================================
echo Using MSBuild for compilation
echo.

REM Check if msbuild.exe is already in PATH
msbuild /version >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo Found MSBuild in PATH
    goto :build_solution
)

REM Check if Visual Studio is installed and set up environment
echo MSBuild not found in PATH. Checking for Visual Studio installation...

REM Check for Visual Studio Community 2022
set "VCVARSALL=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
if exist "%VCVARSALL%" (
    goto :setup_msvc_env
)

REM Check for Visual Studio Professional 2022
set "VCVARSALL=C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat"
if exist "%VCVARSALL%" (
    goto :setup_msvc_env
)

REM Check for Visual Studio Enterprise 2022
set "VCVARSALL=C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat"
if exist "%VCVARSALL%" (
    goto :setup_msvc_env
)

REM Check for Build Tools 2022
set "VCVARSALL=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
if exist "%VCVARSALL%" (
    goto :setup_msvc_env
)

REM Check for Visual Studio 2019 Community
set "VCVARSALL=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat"
if exist "%VCVARSALL%" (
    goto :setup_msvc_env
)

REM Check for Visual Studio 2019 Professional
set "VCVARSALL=C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat"
if exist "%VCVARSALL%" (
    goto :setup_msvc_env
)

REM Check for Visual Studio 2019 Enterprise
set "VCVARSALL=C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat"
if exist "%VCVARSALL%" (
    goto :setup_msvc_env
)

REM Check for Build Tools 2019
set "VCVARSALL=C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
if exist "%VCVARSALL%" (
    goto :setup_msvc_env
)

echo No Visual Studio installation found.
echo.
echo To install Visual Studio with C++ support, run one of:
echo   winget install Microsoft.VisualStudio.2022.Community
echo   winget install Microsoft.VisualStudio.2022.BuildTools
echo.
echo Or download from: https://visualstudio.microsoft.com/downloads/
echo.
echo Make sure to install the "Desktop development with C++" workload
echo.
exit /b 1

:setup_msvc_env
echo Found Visual Studio at: %VCVARSALL%

REM Detect architecture
if "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
    set "ARCH=x64"
) else if "%PROCESSOR_ARCHITEW6432%"=="AMD64" (
    set "ARCH=x64"
) else (
    set "ARCH=x86"
)

echo Setting up MSVC environment for %ARCH%...
call "%VCVARSALL%" %ARCH%
if %ERRORLEVEL% neq 0 (
    echo Failed to set up MSVC environment
    exit /b 1
)

echo MSVC environment configured successfully

:build_solution
echo.
echo Terminating any running BrowserSanity processes...
taskkill /F /IM BrowserSanity.exe /T 3 >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo BrowserSanity.exe processes terminated
) else (
    echo No BrowserSanity.exe processes found
)
echo.
echo Building Browser Sanity solution (Nana version)...
echo.

REM Create build directory if it doesn't exist
if not exist build mkdir build

REM Build the Nana-based solution using MSBuild
echo Building BrowserSanity_Nana.vcxproj...
msbuild BrowserSanity_Nana.vcxproj /p:Configuration=Release /p:Platform=Win32 /m
if %ERRORLEVEL% neq 0 (
    echo Build failed!
    echo.
    echo Common issues:
    echo  - Missing C++ build tools
    echo  - Missing Windows SDK
    echo  - Nana library compilation errors
    echo.
    echo Try installing:
    echo  - Visual Studio 2022 with "Desktop development with C++" workload
    echo  - Windows 10/11 SDK
    echo.
    exit /b 1
)

echo.
echo Build completed successfully!
echo.
echo Output files:
echo  * build-Release\msedge.exe
echo  * build-Release\BrowserSanity.exe (Nana version)
echo.
echo The new Nana-based version should have:
echo  - No multi-window context issues
echo  - Cleaner, more maintainable code
echo  - Better DPI scaling
echo  - More reliable event handling
echo.

REM Automatically run the application for development
echo.
echo Starting BrowserSanity.exe automatically...
start "" "build-Release\BrowserSanity.exe"

exit /b 0