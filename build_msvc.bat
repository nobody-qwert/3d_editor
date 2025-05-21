@echo off
echo Building 3D Editor with MSVC...

REM Create build directory if it doesn't exist
if not exist "build_msvc" mkdir build_msvc

REM Navigate to build directory
cd build_msvc

REM Generate build files with CMake
echo Generating build files with CMake...
cmake .. -G "Visual Studio 17 2022" -A Win32

REM Build the project
echo Building the project...
cmake --build . --config Release

REM Check if build was successful
if %ERRORLEVEL% EQU 0 (
    echo Build successful!
    echo Executable is located at: build_msvc\bin\Release\3DEditor.exe
) else (
    echo Build failed with error code: %ERRORLEVEL%
)

REM Return to original directory
cd ..

pause
