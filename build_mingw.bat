@echo off
echo Building 3D Editor with MinGW...

REM Create build directory if it doesn't exist
if not exist "build_mingw" mkdir build_mingw

REM Navigate to build directory
cd build_mingw

REM Generate build files with CMake
echo Generating build files with CMake...
cmake .. -G "MinGW Makefiles"

REM Build the project
echo Building the project...
cmake --build .

REM Check if build was successful
if %ERRORLEVEL% EQU 0 (
    echo Build successful!
    echo Executable is located at: build_mingw\bin\3DEditor.exe
) else (
    echo Build failed with error code: %ERRORLEVEL%
)

REM Return to original directory
cd ..

pause
