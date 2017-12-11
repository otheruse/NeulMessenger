/*
 * Module.h
 *
 *  Created on: Nov 19, 2017
 *      Author: armin
 */

#ifndef MODULE_H_
#define MODULE_H_
#include <stdint.h>
#include <stddef.h>

class Module {
public:
	Module() {
	}
	virtual int openSocket(uint16_t port) = 0;
	virtual bool closeSocket(int socket) = 0;

	virtual bool sendUDP(int socket, char* ip, uint16_t port, uint8_t* buffer, size_t len) = 0;
	virtual size_t receiveUDP(int socket, uint8_t* buffer, size_t len, uint32_t timeout) = 0;

	virtual bool getImei(char* buffer) = 0;
	virtual ~Module() {
	}
};

#endif /* MODULE_H_ */
