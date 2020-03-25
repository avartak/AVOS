#ifndef CLIB_STRING_H
#define CLIB_STRING_H

#include <stddef.h>

extern void*  memcpy (void*, const void*, size_t);
extern void*  memmove(void*, const void*, size_t);
extern void*  memset (void*, int, size_t);
extern int    memcmp (const void*, const void*, size_t);
extern void*  memchr (const void*, int, size_t);
size_t strlen (const char* string);

#endif
