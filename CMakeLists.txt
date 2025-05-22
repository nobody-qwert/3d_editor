cmake_minimum_required(VERSION 3.10)

# Project name and version
project(3DEditor VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
set(SOURCES
    3Dmath.cpp
    editor.cpp
    glutils.cpp
    glwindow.cpp
    log.cpp
    main.cpp
    nurb.cpp
    shapes.cpp
    texturemanager.cpp
    windowsistem.cpp
    jpeg_wrapper.cpp
    jpeg_compat.c
)

# Add header files
set(HEADERS
    3Dmath.h
    compat.h
    editor.h
    glutils.h
    glwindow.h
    log.h
    nurb.h
    resource.h
    shapes.h
    texturemanager.h
    windowsistem.h
    Glext.h
    jpeglib.h
)

# Add resource files
set(RESOURCES
    Editor.rc
)

# Create executable (Windows GUI application)
add_executable(3DEditor WIN32 ${SOURCES} ${HEADERS} ${RESOURCES})

# Find required packages
find_package(OpenGL REQUIRED)

# Include directories
target_include_directories(3DEditor PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OPENGL_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(3DEditor PRIVATE 
    ${OPENGL_LIBRARIES}
    opengl32
    glu32
    user32
    gdi32
    shell32
    comdlg32
    advapi32
    ole32
    oleaut32
    uuid
    comctl32
)

# Add Windows-specific libraries and settings
if(WIN32)
    # Add preprocessor definitions
    target_compile_definitions(3DEditor PRIVATE
        WIN32
        _WINDOWS
        _CRT_SECURE_NO_WARNINGS
        _CRT_NONSTDC_NO_DEPRECATE
        NOMINMAX
    )
    
    # Set compiler-specific options
    if(MSVC)
        # Visual Studio specific options
        target_compile_options(3DEditor PRIVATE 
            /W3
            /wd4996  # Disable deprecation warnings
            /wd4244  # Disable conversion warnings
            /wd4305  # Disable truncation warnings
        )
        
        # Use the MultiThreaded DLL runtime library
        set_property(TARGET 3DEditor PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
            
        # Link with legacy stdio definitions for compatibility
        target_link_libraries(3DEditor PRIVATE legacy_stdio_definitions)
        
        # Disable SAFESEH for compatibility with old libraries
        set_target_properties(3DEditor PROPERTIES
            LINK_FLAGS "/SAFESEH:NO"
        )
        
        # Don't link with the old JPEG library - we'll use our BMP loader instead
        # The old jpeg.lib is incompatible with modern MSVC
        
    else()
        # GCC/MinGW specific options
        target_compile_options(3DEditor PRIVATE -Wall -Wno-deprecated-declarations)
    endif()
endif()

# Copy required files to build directory
add_custom_command(TARGET 3DEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/bulb.jpg"
        "${CMAKE_CURRENT_SOURCE_DIR}/texture.jpg"
        $<TARGET_FILE_DIR:3DEditor>
)

# Set output directory
set_target_properties(3DEditor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Create textures directory in output
add_custom_command(TARGET 3DEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:3DEditor>/textures
)

# Print build information
message(STATUS "Building 3DEditor for Windows")
message(STATUS "OpenGL libraries: ${OPENGL_LIBRARIES}")
message(STATUS "Note: Using built-in BMP loader instead of old JPEG library")
if(MSVC)
    message(STATUS "Using MSVC compiler with SAFESEH disabled for compatibility")
else()
    message(STATUS "Using ${CMAKE_CXX_COMPILER_ID} compiler")
endif()
