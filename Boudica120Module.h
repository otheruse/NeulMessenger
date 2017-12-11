/*
 * QuectelBC95Module.h
 *
 *  Created on: Nov 20, 2017
 *      Author: armin
 */

#ifndef BC95MODULE_H_
#define BC95MODULE_H_

#include <stdint.h>
#include <stddef.h>
#include "Module.h"
#include "NStream.h"

class Boudica120Module  : public Module {
	NStream& serial;
public:
	Boudica120Module(NStream& serial);
	virtual ~Boudica120Module();

	virtual int openSocket(uint16_t port) override;
	virtual bool closeSocket(int socket) override;

	virtual bool sendUDP(int socket, char* ip, uint16_t port, uint8_t* buffer, size_t len) override;
	virtual size_t receiveUDP(int socket, uint8_t* buffer, size_t len, uint32_t timeout) override;

	virtual bool getImei(char* buffer) override;

};

#endif /* BC95MODULE_H_ */
