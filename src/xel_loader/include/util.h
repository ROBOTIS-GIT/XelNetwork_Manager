/*
 * util.h
 *
 *  Created on: 2017. 2. 13.
 *      Author: baram
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "def.h"

#if defined (__linux__)
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/serial.h>

#elif defined (__WIN32__) || (__WIN64__)
#include <Windows.h>

#endif


#ifdef __cplusplus
extern "C" {
#endif





uint32_t millis(void);
void delay(uint32_t delay_ms);
void utilUpdateCrc(uint16_t *p_crc_cur, uint8_t data_in);

#ifdef __cplusplus
}
#endif

#endif
