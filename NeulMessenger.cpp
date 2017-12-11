/*
 * NeulMessenger.cpp
 *
 *  Created on: Nov 19, 2017
 *      Author: armin
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "NeulMessenger.h"
#include "CoapPacket.h"

#define NMSG_TIMEOUT 30000

NeulMessenger::NeulMessenger(Module& module, const char* ip, uint16_t port) :
		serverIp { }, serverPort(port), imei { }, token { }, tokenLength(0), module(module), messageIdGen(rand()),
				observe(-1), socket(-1) {
	strncpy(serverIp, ip, sizeof(serverIp));
}

bool NeulMessenger::handleReceived(uint8_t* buffer, size_t bufferLen, Coap::Packet& target) {
	if (!Coap::bufferToPacket(target, buffer, bufferLen)) {
		return false;
	}
	if (target.type == Coap::Type::ACK) {
		// TODO should we do something with this ack?
		return true;
	}
	if (target.type != Coap::Type::RESET) {
		int obs = -1;
		bool pathMatch = false;
		int i = 0;
		for (; i < target.optionnum; i++) {
			if (target.options[i].number == Coap::Option::Number::OBSERVE) {
				obs = *(target.options[i].buffer);
			}
			if (target.options[i].number == Coap::Option::Number::URI_PATH) {
				if ((target.options[i].length == 1 && *(target.options[i].buffer) == 't') &&
						(target.options[i + 1].length == 1 && *(target.options[i + 1].buffer) == 'd')) {
					// this is for us!
					pathMatch = true;
				}
			}
		}
		if (pathMatch) {
			if (target.type == Coap::Type::CON) {
				// We need to ack!
				Coap::Packet packet { };
				packet.type = Coap::Type::ACK;
				packet.messageid = target.messageid;
				packet.payloadlen = 0;
				if (obs >= 0) {
					packet.options[0].number = Coap::Option::Number::OBSERVE;
					packet.options[0].length = 1;
					uint8_t obsv = (uint8_t) obs;
					packet.options[0].buffer = &obsv;
					packet.optionnum = 1;
				}
				else {
					packet.optionnum = 0;
				}
				packet.token = target.token;
				packet.tokenlen = target.tokenlen;
				packet.responseCode = Coap::ResponseCode::CONTENT;
				size_t len = Coap::packetToBuffer(packet, packetBuffer, sizeof(packetBuffer));
				if (!module.sendUDP(socket, serverIp, serverPort, packetBuffer, len)) {
					return false;
				}
			}
			if (obs >= 0) {
				// Ack succeeded, we can now save the token and observe value
				observe = obs;
				tokenLength = target.tokenlen;
				memcpy(token, target.token, tokenLength);
			}
		}
	}
	return true;
}

size_t NeulMessenger::createRegisterPacket(uint8_t* buffer, size_t bufferSize) {
	Coap::Packet packet{};
	packet.type = Coap::Type::CON;
	packet.method = Coap::Method::POST;
	packet.messageid = messageIdGen++;
	packet.options[0].number = Coap::Option::Number::URI_PATH;
	packet.options[0].length = 1;
	packet.options[0].buffer = (uint8_t*) "t";
	packet.options[1].number = Coap::Option::Number::URI_PATH;
	packet.options[1].length = 1;
	packet.options[1].buffer = (uint8_t*) "r";
	char buf[20] = "ep=";
	strncpy(buf + 3, imei, sizeof(buf) - 3);
	packet.options[2].number = Coap::Option::Number::URI_QUERY;
	packet.options[2].length = strlen(buf);
	packet.options[2].buffer = (uint8_t*) buf;
	packet.optionnum = 3;
	packet.payloadlen = 0;
	packet.tokenlen = 0;
	return Coap::packetToBuffer(packet, buffer, bufferSize);
}

NeulMessenger::Result NeulMessenger::sendMessage(uint8_t* buffer, size_t len) {
	if (imei[0] == 0) {
		if (!module.getImei(imei)) {
			return Result::ModuleError;
		}
	}
	if (socket < 0) {
		// no socket means no connection. Registration is needed
		observe = -1;
		tokenLength = 0;
		socket = module.openSocket(5683); // local port, can be any
	}
	if (socket < 0) {
		return Result::SocketError;
	}
	if (observe < 0) {
		// Try to register
		size_t messageLen = createRegisterPacket(packetBuffer, sizeof(packetBuffer));

		if (!module.sendUDP(socket, serverIp, serverPort, packetBuffer, messageLen)) {
			module.closeSocket(socket);
			socket = -1;
			return Result::UdpError;
		}

		timestamp_t start = getTimestamp();
		do {
			size_t received = module.receiveUDP(socket, packetBuffer, sizeof(packetBuffer), NMSG_TIMEOUT - (getTimestamp() - start));
			if (received == 0) {
				return Result::ReceiveTimeout;
			}
			Coap::Packet rpacket;
			handleReceived(packetBuffer, received, rpacket);
		}
		while (observe < 0 && (getTimestamp() - start) < NMSG_TIMEOUT);
	}

	if (observe >= 0) {
		Coap::Packet packet{};
		// Already registered, send data
		packet.type = Coap::Type::NON;
		packet.responseCode = Coap::ResponseCode::CONTENT;
		packet.messageid = messageIdGen++;
		packet.options[0].number = Coap::Option::Number::OBSERVE;
		packet.options[0].length = 1;
		packet.options[0].buffer = (uint8_t*)&observe;
		packet.optionnum = 1;
		packet.payload = buffer;
		packet.payloadlen = len;
		packet.token = token;
		packet.tokenlen = tokenLength;
		size_t messageLen = Coap::packetToBuffer(packet, packetBuffer, sizeof(packetBuffer));
		if (!module.sendUDP(socket, serverIp, serverPort, packetBuffer, messageLen)) {
			module.closeSocket(socket);
			socket = -1;
			return Result::UdpError;
		}
	}
	else {
		return Result::ReceiveTimeout;
	}
	return Result::Success;
}

NeulMessenger::~NeulMessenger() {
	if (socket >= 0) {
		module.closeSocket(socket);
	}
}

size_t NeulMessenger::receiveMessage(uint8_t* buffer, size_t len, duration_t timeout) {
	Coap::Packet rpacket;
	timestamp_t start = getTimestamp();
	do {
		size_t received = module.receiveUDP(socket, packetBuffer, sizeof(packetBuffer), timeout - (getTimestamp() - start));
		if (received == 0) {
			return 0;
		}
		if (handleReceived(packetBuffer, received, rpacket) && rpacket.payloadlen > 0) {
			// we have a payload!
			size_t size = len < rpacket.payloadlen ? len : rpacket.payloadlen;
			memcpy(buffer, rpacket.payload, size);
			return size;
		}

	}
	while (getTimestamp() - start < timeout);
	return 0;
}
