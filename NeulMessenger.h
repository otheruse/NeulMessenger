/*
 * NeulMessenger.h
 *
 *  Created on: Nov 19, 2017
 *      Author: armin
 */

#ifndef NEULMESSENGER_H_
#define NEULMESSENGER_H_
#include "platform.h"
#include <stdint.h>
#include <stddef.h>

#include "ArduinoNStream.h"
#include "Module.h"
#include "CoapPacket.h"
#include "Module.h"

class NeulMessenger {
public:
	enum class Result {
		Success = 0,
		ModuleError,
		SocketError,
		UdpError,
		ReceiveTimeout,
		Nacked,
	};
private:
	char serverIp[16];
	uint16_t serverPort;
	char imei[17];
	uint8_t token[8];
	uint8_t tokenLength = 0;
	Module& module;
	uint16_t messageIdGen;
	uint8_t packetBuffer[MAX_UDP_PACKET_LEN];
	int8_t observe;
	int socket;
	size_t createRegisterPacket(uint8_t* buffer, size_t bufferSize);
	bool handleReceived(uint8_t* buffer, size_t bufferLen, Coap::Packet& target);
public:
	NeulMessenger(Module& module, const char* ip, uint16_t port = 5683);
	Result sendMessage(uint8_t* buffer, size_t len);
	size_t receiveMessage(uint8_t* buffer, size_t len, duration_t timeout);
	virtual ~NeulMessenger();
};

#endif /* NEULMESSENGER_H_ */
