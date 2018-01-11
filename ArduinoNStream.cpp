/*
 * ArduinoBrook.cpp
 *
 *  Created on: Nov 20, 2017
 *      Author: armin
 */

#include "ArduinoNStream.h"

ArduinoNStream::ArduinoNStream(Stream& stream, Stream* echoStream, bool echoWrite) : stream(stream), _echoStream(echoStream), timeout(stream.getTimeout()), echoWrite(echoWrite)
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
	if (_echoStream!= nullptr && c >=0) {
		_echoStream->write(c);
	}
	return c;
}

size_t ArduinoNStream::available() {
	return stream.available();
}

size_t ArduinoNStream::write(const uint8_t* buf, size_t nbyte) {
	if (_echoStream != nullptr && echoWrite) {
		_echoStream->write((const char*)buf,  nbyte);
	}
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
