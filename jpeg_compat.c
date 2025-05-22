#include <stdio.h>

#if defined(_MSC_VER) && _MSC_VER >= 1900
// For Visual Studio 2015 and later, provide compatibility for old JPEG library
// that expects the old __iob symbol

// Declare external variables that will be initialized at runtime
extern FILE* __iob_stdin;
extern FILE* __iob_stdout;
extern FILE* __iob_stderr;

FILE* __iob_stdin = NULL;
FILE* __iob_stdout = NULL;
FILE* __iob_stderr = NULL;

FILE *__cdecl __iob_func(void)
{
    // Initialize on first call
    if (__iob_stdin == NULL) {
        __iob_stdin = stdin;
        __iob_stdout = stdout;
        __iob_stderr = stderr;
    }
    
    // Return pointer to stdin (the JPEG library typically accesses __iob[0])
    return __iob_stdin;
}
#endif
