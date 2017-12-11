/*
 * MDM9206Module.cpp
 *
 *  Created on: Nov 28, 2017
 *      Author: armin
 */

#include "MDM9206Module.h"

MDM9206Module::MDM9206Module(NStream& serial) : serial(serial), sockInUse{} {
}

MDM9206Module::~MDM9206Module() {
}

int MDM9206Module::openSocket(uint16_t port) {
	serial.flush();
	int socket = -1;
	for (int i = 0; i < 11 ; i++) {
		if (!sockInUse[i]) {
			socket = i;
			sockInUse[i] = true;
			break;
		}
	}
	if (socket < 0) {
		return -1;
	}
	serial.printf("AT+QIOPEN=1,%d,\"UDP SERVICE\",\"127.0.0.1\",0,%d,0\r\n", socket, port);
	bool success = serial.find(2000, { "OK", "ERROR" }) == 0;
	delaymS(10); // we need to wait a bit for the socket to become ready, otherwise "send message" wil fail
	return success ? socket : -1;
}

bool MDM9206Module::closeSocket(int socket) {
	serial.flush();
	serial.printf("AT+QICLOSE=%d\r\n", socket);
	sockInUse[socket] = false;
	return serial.find(1000, { "OK", "ERROR" }) == 0;
}

bool MDM9206Module::sendUDP(int socket, char* ip, uint16_t port, uint8_t* buffer, size_t len) {
	serial.flush();
	serial.printf("AT+QISEND=%d,%u,\"%s\",%d\r\n", socket, len, ip, port);
	if (serial.find(1000, { "> ", "ERROR" }) != 0) {
		return false;
	}
	serial.write(buffer, len);
	bool success = (serial.find(1000, { "OK", "ERROR" }) == 0);
	if (success) {
		serial.printf("AT+QISEND=%d,0\r\n", socket);
		serial.find(1000, { "OK", "ERROR" });
	}
	return success;
}

size_t MDM9206Module::receiveUDP(int socket, uint8_t* buffer, size_t len, uint32_t timeout) {
	timestamp_t start = getTimestamp();
	bool found = false;
	while (getTimestamp() - start < timeout) {
		// +QIURC: "recv",
		if (serial.find(1000, {"+QIURC: \"recv\","}) == 0) {
			char buf[10]{};
			serial.readLine(buf, sizeof(buf), timeout - (getTimestamp() - start));
			int sock = atoi(buf);
			Serial.print("Message receive indicator found on socket ");
			Serial.println(sock, 10);
			if (sock == socket) {
				found = true;
				break;
			}
		}
	}
	if (found) {
		serial.printf("AT+QIRD=%d\r\n", socket);
		serial.find(1000,{"+QIRD: "});
		serial.readLine((char*)buffer, len, 1000);// the response
		int size = atoi((char*)buffer);
		// Ignore ip address and port for now
		serial.readBytes(buffer, size, 1000);
		return size;
	}
	return 0;
}

bool MDM9206Module::getImei(char* buffer) {
	serial.flush();
	buffer[0] = 0;
	serial.printf("AT+CGSN\r\n");
	serial.setTimeout(1000);
	size_t i = serial.readNextNumber(buffer, 17);
	serial.find(1000, {"OK", "ERROR"});
	return i;

}
