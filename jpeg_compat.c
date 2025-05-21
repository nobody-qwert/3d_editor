#include <stdio.h>

#if defined(_MSC_VER) && _MSC_VER >= 1900
FILE _iob[] = { *stdin, *stdout, *stderr };

FILE *__cdecl __iob_func(void)
{
    return _iob;
}
#endif
