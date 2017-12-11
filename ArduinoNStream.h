/*
 * ArduinoBrook.h
 *
 *  Created on: Nov 20, 2017
 *      Author: armin
 */

#ifndef ARDUINONSTREAM_H_
#define ARDUINONSTREAM_H_
#include "Arduino.h"

#include "NStream.h"

class ArduinoNStream : public NStream {
	Stream& stream;
	uint32_t timeout;
public:
	ArduinoNStream(Stream& stream);
	virtual ~ArduinoNStream();
	virtual int read() override;
	virtual size_t available() override;
	virtual size_t write(const uint8_t* buf, size_t nbyte) override;
	virtual void flush() override;
	virtual void setTimeout(uint32_t timeout) override;
};

#endif /* ARDUINONSTREAM_H_ */
