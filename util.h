#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>

int string_equal(const char *, const char *);
void wrlev(FILE *, int, const char *, ...);

#define min(a, b) ({typeof(a) __a=(a), __b=(b); __a<__b?__a:__b;})
#define max(a, b) ({typeof(a) __a=(a), __b=(b); __a>__b?__a:__b;})

#endif
