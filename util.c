/*
 * util.cpp
 *
 *  Created on: Nov 29, 2017
 *      Author: armin
 */


#include "util.h"

uint8_t characterToNibble(char c) {
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return 255;
}


size_t hexStringToByteArray(const char *hexString, uint8_t* byteArray, size_t byteArrayLength) {
	size_t i = 0;
	uint8_t b = 0;
	while (hexString[i] != 0 && (i / 2 < byteArrayLength)) {
		uint8_t nibble = characterToNibble(hexString[i]);
		if (nibble == 255) {
			return (i + 1) / 2;
		}
		if (!(i & 1)) {
			// even nibble
			b = nibble << 4;
		} else {
			// odd nibble
			b |= nibble;
			byteArray[i / 2] = b;
		}
		i++;
	}
	return (i + 1) / 2;
}
