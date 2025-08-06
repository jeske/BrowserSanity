@echo off
setlocal enabledelayedexpansion

echo Browser Sanity Build Script
echo ===========================
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
pause
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
echo Building Browser Sanity solution...
echo.

REM Create build directory if it doesn't exist
if not exist build mkdir build

REM Build the solution using MSBuild
msbuild BrowserSanity.sln /p:Configuration=Release /p:Platform=Win32 /m
if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b 1
)

echo.
echo Build completed successfully!
echo.
echo Output files:
echo  * build\Release\msedge.exe
echo  * build\Release\BrowserSanity.exe
echo.

exit /b 0