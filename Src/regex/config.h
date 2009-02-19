#ifndef TOMBO_CONFIG_FOR_REGEX
#define TOMBO_CONFIG_FOR_REGEX

#define HAVE_STRING_H
#define HAVE_PROTOTYPES
#define NO_ALLOCA
#define STDC_HEADERS

//sys/types.hÇÕë∂ç›ÇπÇ∏
#include <tchar.h>

unsigned long scan_oct(const char *start, int len, int *retlen);
unsigned long scan_hex(const char *start, int len, int *retlen);

#define snprintf _snprintf

void *xmalloc(size_t siz);
void *xrealloc(void *p, size_t siz);
void xfree(void *p);

#include <ctype.h>

#if defined(PLATFORM_HPC) || defined(PLATFORM_PSPC)
#include "ctypeutil.h"
#endif

#if defined(PLATFORM_WIN32)
#include <malloc.h>
#endif
#endif