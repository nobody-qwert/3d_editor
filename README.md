# 3D Editor

This is a 3D editor application originally built around 20 years ago. It has been updated to build on modern Windows systems using CMake.

## Prerequisites

To build this project, you need one of the following:

### Option 1: Visual Studio (MSVC)
- Visual Studio 2022 Community Edition (free)
- Desktop development with C++ workload
- CMake (included with Visual Studio or can be installed separately)

### Option 2: MinGW
- MinGW-w64 with GCC
- CMake

## Installing Dependencies

For convenience, an `install_dependencies.bat` script is provided to help you install the necessary dependencies:

1. Right-click on `install_dependencies.bat` and select "Run as administrator"
2. Choose your preferred compiler option:
   - Option 1: Visual Studio Community Edition (recommended)
   - Option 2: MinGW-w64
3. Follow the instructions provided by the script

The script will open the download pages for the selected compiler and CMake.

## Building the Project

### Using Visual Studio (MSVC)

1. Make sure Visual Studio with C++ development tools is installed
2. Run the `build_msvc.bat` script
3. The executable will be created at `build_msvc\bin\Release\3DEditor.exe`

### Using MinGW

1. Make sure MinGW is installed and in your PATH
2. Run the `build_mingw.bat` script
3. The executable will be created at `build_mingw\bin\3DEditor.exe`

## Manual Build

If you prefer to build manually:

### With Visual Studio:
```
mkdir build_msvc
cd build_msvc
cmake .. -G "Visual Studio 17 2022" -A Win32
cmake --build . --config Release
```

### With MinGW:
```
mkdir build_mingw
cd build_mingw
cmake .. -G "MinGW Makefiles"
cmake --build .
```

## Project Structure

- `3Dmath.cpp/h`: 3D math operations
- `editor.cpp/h`: Main editor functionality
- `glutils.cpp/h`: OpenGL utilities
- `glwindow.cpp/h`: OpenGL window management
- `log.cpp/h`: Logging functionality
- `main.cpp`: Application entry point
- `nurb.cpp/h`: NURB surface implementation
- `shapes.cpp/h`: 3D shape definitions
- `texturemanager.cpp/h`: Texture management
- `windowsistem.cpp/h`: Windows system integration
- `compat.h`: Compatibility layer for modern compilers

## Dependencies

- OpenGL
- JPEG library (included as jpeg.lib)
- Windows libraries

## Compatibility Notes

This project was originally built around 20 years ago and has been updated to work with modern compilers. The following compatibility changes have been made:

1. Added a compatibility header (`compat.h`) to handle deprecated functions and headers
2. Updated OpenGL includes to use modern paths
3. Created a CMake build system that works with both MSVC and MinGW
4. Added workarounds for deprecated libraries like glaux.h

If you encounter any issues during compilation, please check the error messages and make sure you have installed all the required dependencies.
