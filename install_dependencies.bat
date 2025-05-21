@echo off
echo Installing dependencies for 3D Editor...

REM Check if running as administrator
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo This script requires administrator privileges.
    echo Please right-click on the script and select "Run as administrator".
    pause
    exit /b 1
)

echo.
echo This script will help you install the necessary dependencies for building the 3D Editor.
echo.
echo Options:
echo 1. Install Visual Studio Community Edition (recommended)
echo 2. Install MinGW-w64
echo 3. Exit
echo.

set /p choice="Enter your choice (1-3): "

if "%choice%"=="1" (
    echo.
    echo Opening Visual Studio Community Edition download page...
    start https://visualstudio.microsoft.com/vs/community/
    echo.
    echo When installing Visual Studio, make sure to select the "Desktop development with C++" workload.
    echo This will install the necessary C++ compiler and tools.
    echo.
) else if "%choice%"=="2" (
    echo.
    echo Opening MinGW-w64 download page...
    start https://sourceforge.net/projects/mingw-w64/files/
    echo.
    echo When installing MinGW-w64, select the following options:
    echo - Version: latest
    echo - Architecture: x86_64
    echo - Threads: win32
    echo - Exception: seh
    echo.
    echo After installation, add the MinGW bin directory to your PATH environment variable.
    echo.
) else if "%choice%"=="3" (
    echo Exiting...
    exit /b 0
) else (
    echo Invalid choice. Exiting...
    exit /b 1
)

echo.
echo You will also need to install CMake if you don't have it already.
echo Opening CMake download page...
start https://cmake.org/download/

echo.
echo After installing the dependencies, you can build the project using:
echo - build_msvc.bat (if you installed Visual Studio)
echo - build_mingw.bat (if you installed MinGW-w64)
echo.

pause
