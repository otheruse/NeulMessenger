/*
 * Brook.h
 *
 *  Created on: Nov 20, 2017
 *      Author: armin
 */

#ifndef NSTREAM_H_
#define NSTREAM_H_
#include "platform.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <initializer_list>

class NStream {
	static void putChar(void* ptr, char c);
public:
	NStream() {
	}
	// Pure virtual
	virtual int read() = 0;
	virtual size_t available() = 0;
	virtual size_t write(const uint8_t* buf, size_t nbyte) = 0;
	virtual void flush() = 0;
	virtual void setTimeout(uint32_t timeout) = 0;
	// Implemented
	virtual size_t readBytes(uint8_t *buffer, size_t bufferSize, uint32_t timeout);
	int find(uint32_t timeout, std::initializer_list<const char*> args, char* buffer = nullptr, size_t bufferSize = 0);
	int readLine(char* buffer, size_t len, uint32_t timeout);
	size_t readNextNumber(char* buffer, size_t bufferSize);
	void printf(const char* format, ...);
	virtual ~NStream() {
	}
};

#endif /* NSTREAM_H_ */
