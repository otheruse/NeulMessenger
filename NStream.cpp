/*
 * Brook.cpp
 *
 *  Created on: Nov 20, 2017
 *      Author: armin
 */
#include "NStream.h"

#include <stdio.h>
#include "tfp_printf.h"

size_t NStream::readBytes(uint8_t *buffer, size_t count, uint32_t timeout) {
	setTimeout(timeout);
	for (size_t i = 0; i < count; i++) {
		int c = read();
		if (c < 0) {
			return i;
		}
		buffer[i] = c;
	}
	return count;

}

int NStream::find(uint32_t timeout, std::initializer_list<const char*> args, char* buffer, size_t bufferSize) {
	setTimeout(timeout);
	int indices[args.size()];
	for (size_t i = 0; i < args.size(); i++) {
		indices[i] = 0;
	}
	size_t responseIndex = 0;
	timestamp_t start = getTimestamp();
	size_t found = args.size();
	while (found >= args.size() && (timeout == 0 || (getTimestamp() - start) < timeout)) {
		int c = read();
		if (c < 0) {
			// Timout occurred
			break;
		}
		if (responseIndex < bufferSize) {
			buffer[responseIndex++] = c;
		}
		int i = 0;
		for (auto response : args) {
			if (response[indices[i]] == c) {
				indices[i]++;
			}
			else if (response[0] == c) {
				indices[i] = 1;
			}
			else {
				indices[i] = 0;
			}
			if (response[indices[i]] == '\0') {
				// Complete string found
				found = i;
				break;
			}
			i++;
		}
	}
	// 0 terminate the response buffer
	if (responseIndex < bufferSize) {
		buffer[responseIndex] = 0;
	}
	return found;
}

void NStream::putChar(void* ptr, char c) {
	((NStream*) ptr)->write((uint8_t*) &c, 1);
}

void NStream::printf(const char* format, ...) {
	va_list ap;

	va_start(ap, format);

	tfp_format(this, &putChar, format, ap);
	va_end(ap);
}

size_t NStream::readNextNumber(char* buffer, size_t bufferSize) {
	size_t i = 0;
	while (i < bufferSize - 1) {
		int c = read();
		if (c < 0) {
			// timeout
			break;
		}
		if (c >= '0' && c <= '9') {
			buffer[i++] = c;
		}
		else {
			// discard leading non-number characters
			if (i != 0) {
				// we're done
				buffer [i] = 0;
				break;
			}
		}
	}
	buffer[i] = 0; // zero terminated string
	return i;
}

int NStream::readLine(char* buffer, size_t bufferSize, uint32_t timeout) {
	const char* lineEnd = "\r\n";
	setTimeout(timeout);
	int index = 0;
	size_t responseIndex = 0;
	timestamp_t start = getTimestamp();
	while ((timeout == 0 || (getTimestamp() - start) < timeout)) {
		int c = read();
		if (c < 0) {
			// Timout occurred
			break;
		}
		if (responseIndex < bufferSize) {
			buffer[responseIndex++] = c;
		}
		int i = 0;
		if (lineEnd[index] == c) {
			index++;
		}
		else if (lineEnd[0] == c) {
			index = 1;
		}
		else {
			index = 0;
		}
		if (lineEnd[index] == '\0') {
			// Complete string found
			break;
		}
		i++;
	}
	// 0 terminate the response buffer
	if (responseIndex < bufferSize) {
		buffer[responseIndex] = 0;
	}
	return responseIndex;

}
