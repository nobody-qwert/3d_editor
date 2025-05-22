#include <stdio.h>
#include <stdlib.h>

// Define __iob_func for Visual Studio 2015 and later
#if defined(_MSC_VER) && _MSC_VER >= 1900
extern "C" {
    // This is handled by jpeg_compat.c, just declare it here
    FILE * __cdecl __iob_func(void);
}
#endif

// Stub implementations for any missing JPEG functions
extern "C" {
    // If the JPEG library is missing, provide stub implementations
    // These will be overridden if the real JPEG library is linked
    
    void* LoadJPG(const char* filename) {
        // Return NULL to indicate failure
        return NULL;
    }
    
    int LoadToOpenGL_JPG(const char* filename, unsigned int& texID, int minFilter, int magFilter) {
        // Return 0 to indicate failure
        return 0;
    }
}
