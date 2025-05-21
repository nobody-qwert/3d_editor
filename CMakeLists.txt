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
)

# Add header files
set(HEADERS
    3Dmath.h
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
target_include_directories(3DEditor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link libraries
target_link_libraries(3DEditor PRIVATE 
    ${OPENGL_LIBRARIES}
    ${CMAKE_CURRENT_SOURCE_DIR}/jpeg.lib
)

# Add Windows-specific libraries
if(WIN32)
    target_link_libraries(3DEditor PRIVATE
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
        odbc32
        odbccp32
        comctl32
    )
    
# Add legacy stdio definitions for older libraries like jpeg.lib
if(MSVC)
    target_link_libraries(3DEditor PRIVATE legacy_stdio_definitions)
    # Add preprocessor definition to handle __iob symbol
    target_compile_definitions(3DEditor PRIVATE _CRT_SECURE_NO_WARNINGS)
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

# Add compile definitions
target_compile_definitions(3DEditor PRIVATE
    WIN32
    _WINDOWS
)

# Set compiler-specific options
if(MSVC)
    # Visual Studio specific options
    target_compile_options(3DEditor PRIVATE /W3)
    
    # Disable SAFESEH to work with older libraries like jpeg.lib and force linking despite unresolved externals
    set_target_properties(3DEditor PROPERTIES LINK_FLAGS "/SAFESEH:NO /NODEFAULTLIB:LIBC.lib /DEFAULTLIB:legacy_stdio_definitions.lib /FORCE:UNRESOLVED")
    
    # Use the MultiThreaded DLL runtime library
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /MD")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MD")
else()
    # GCC/MinGW specific options
    target_compile_options(3DEditor PRIVATE -Wall)
endif()
