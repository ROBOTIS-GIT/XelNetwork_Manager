/*
 * util.c
 *
 *  Created on: 2017. 2. 13.
 *      Author: baram
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#include "util.h"


#if defined (__WIN32__) || (__WIN64__)



uint32_t millis(void)
{
  double ret;

  LARGE_INTEGER freq, counter;

  QueryPerformanceCounter(&counter);
  QueryPerformanceFrequency(&freq);

  ret = (double)counter.QuadPart / (double)freq.QuadPart * 1000.0;

  return (uint32_t)ret;
}


#endif
