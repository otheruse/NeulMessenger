/*
 * ArduinoBrook.cpp
 *
 *  Created on: Nov 20, 2017
 *      Author: armin
 */

#include "ArduinoNStream.h"

ArduinoNStream::ArduinoNStream(Stream& stream) : stream(stream), timeout(stream.getTimeout())
{
}

ArduinoNStream::~ArduinoNStream()
{
}

int ArduinoNStream::read() {
	timestamp_t start = getTimestamp();
	while (!stream.available()) {
		if (getTimestamp() - start > timeout) {
			return -1;
		}
	}
	int c = stream.read();
	if (c >=0) {
		Serial.write(c);
	}
	return c;
}

size_t ArduinoNStream::available() {
	return stream.available();
}

size_t ArduinoNStream::write(const uint8_t* buf, size_t nbyte) {
	return stream.write(buf, nbyte);
}

void ArduinoNStream::flush() {
	stream.flush();
	while (stream.available() > 0) {
		stream.read();
	}
}

void ArduinoNStream::setTimeout(uint32_t timeout) {
	this->timeout = timeout;
	stream.setTimeout(timeout);
}
