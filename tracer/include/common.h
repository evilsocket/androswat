#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>

#define FATAL(...) fprintf (stderr, __VA_ARGS__); exit(EXIT_FAILURE)

#endif
