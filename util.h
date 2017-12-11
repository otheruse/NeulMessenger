/*
 * util.h
 *
 *  Created on: Nov 29, 2017
 *      Author: armin
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <stdint.h>
#include <stddef.h>
#if defined(__cplusplus)
extern "C" {
#endif
uint8_t characterToNibble(char c);
size_t hexStringToByteArray(const char *hexString, uint8_t* byteArray, size_t byteArrayLength);

#if defined(__cplusplus)
}
#endif

#endif /* UTIL_H_ */
