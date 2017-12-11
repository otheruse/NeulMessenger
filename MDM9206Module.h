/*
 * MDM9206Module.h
 *
 *  Created on: Nov 28, 2017
 *      Author: armin
 */

#ifndef MDM9206MODULE_H_
#define MDM9206MODULE_H_
#include "Module.h"
#include "NStream.h"

class MDM9206Module: public Module {
	NStream& serial;
	bool sockInUse[11];
public:
	MDM9206Module(NStream& serial);
	virtual ~MDM9206Module();

	virtual int openSocket(uint16_t port) override;
	virtual bool closeSocket(int socket) override;

	virtual bool sendUDP(int socket, char* ip, uint16_t port, uint8_t* buffer, size_t len) override;
	virtual size_t receiveUDP(int socket, uint8_t* buffer, size_t len, uint32_t timeout) override;

	virtual bool getImei(char* buffer) override;
};

#endif /* MDM9206MODULE_H_ */
