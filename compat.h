#ifndef __COMPAT_H
#define __COMPAT_H

// Compatibility header for building the 3D Editor on modern systems
#include <stddef.h> // For NULL

// Allow for loop variables to be visible outside the loop scope
// This is needed for older C++ code that relies on this behavior
// #define for if(0); else for // Commented out due to C1189 error with MSVC

// Handle deprecated iostream.h
#ifdef _MSC_VER
#pragma warning(disable: 4996) // Disable deprecation warnings
#endif

// Handle glaux.h dependency
#ifdef _WIN32
// Check if we're using a modern compiler that might not have glaux.h
#if defined(_MSC_VER) && _MSC_VER >= 1400
// Define minimal AUX_RGBImageRec structure needed by the project
#ifndef __GLAUX_H__
#define __GLAUX_H__
typedef struct _AUX_RGBImageRec {
    int sizeX, sizeY;
    unsigned char *data;
} AUX_RGBImageRec;

// Declare the auxDIBImageLoad function that's used in the project
AUX_RGBImageRec *auxDIBImageLoad(const char *filename);
#endif // __GLAUX_H__

// Implementation of auxDIBImageLoad for modern systems
// This is a simplified version that just returns NULL
// You may need to implement proper BMP loading if this is used
inline AUX_RGBImageRec *auxDIBImageLoad(const char *filename) {
    // Return NULL to indicate failure
    // The project should fall back to JPEG loading
    return NULL;
}
#endif // _MSC_VER >= 1400
#endif // _WIN32

#endif // __COMPAT_H
