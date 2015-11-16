#ifndef _long_integer_h
#define _long_integer_h 1

#include <limits.h>

#if	!defined(__STRICT_ANSI__) && defined(__GNUC__)
typedef unsigned long long long_integer;
const long_integer long_integer_max = ULONG_LONG_MAX - 1;
const long_integer not_a_long_integer = ULONG_LONG_MAX;
#else
typedef unsigned long long_integer;
const long_integer long_integer_max = ULONG_MAX - 1;
const long_integer not_a_long_integer = ULONG_MAX;
#endif

#endif
