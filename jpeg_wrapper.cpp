#include <stdio.h>

// Define __iob_func for Visual Studio 2015 and later
#if defined(_MSC_VER) && _MSC_VER >= 1900
extern "C" {
    // This is a simplified implementation that just returns NULL
    // It's only meant to satisfy the linker, not to be functional
    FILE * __cdecl __iob_func(void)
    {
        return NULL;
    }
}
#endif
