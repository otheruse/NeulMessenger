/*
 * platform.h
 *
 *  Created on: Nov 20, 2017
 *      Author: armin
 */

#ifndef PLATFORM_H_
#define PLATFORM_H_
#include "Arduino.h"

#if defined(ARDUINO_ARCH_AVR) && defined (__cplusplus)
#include <ArduinoSTL.h>
#endif

// Used for the packet buffer in the NeulMessenger class
#define MAX_UDP_PACKET_LEN 512

typedef unsigned long timestamp_t;
typedef unsigned long duration_t;

// Timestamp should be in milliseconds
static inline timestamp_t getTimestamp() {
	return millis();
}

// Wait for at least ms milliseconds
static inline void delaymS(duration_t ms) {
	delay(ms);
}

#define PRINTF_LONG_SUPPORT

#endif /* PLATFORM_H_ */
