/*
 * QuectelBoudica120Module.cpp
 *
 *  Created on: Nov 20, 2017
 *      Author: armin
 */
#include <Boudica120Module.h>
#include <stdio.h>
#include "util.h"
#include "platform.h"

Boudica120Module::Boudica120Module(NStream& serial) : serial(serial) {
}

Boudica120Module::~Boudica120Module() {
}

int Boudica120Module::openSocket(uint16_t port) {
	serial.flush();
//	serial.printf("AT+NSOCR=DGRAM,17,%u,1\r\n", port);
	serial.printf("AT+NSOCR=DGRAM,17,5863,1\r\n");
	char buf[30];
	serial.setTimeout(2000);
	serial.readNextNumber(buf, sizeof(buf));
	bool success = serial.find(200, { "OK", "ERROR" }, buf, sizeof(buf)) == 0;
	int retVal = -1;
	if (success) {
		retVal = atoi(buf);
 	}
	return retVal;
}

bool Boudica120Module::closeSocket(int socket) {
	serial.flush();
	serial.printf("AT+NSOCL=%d\r\n", socket);
	return serial.find(1000, { "OK", "ERROR" }) == 0;
}

bool Boudica120Module::sendUDP(int socket, char* ip, uint16_t port, uint8_t* buffer, size_t len) {
	serial.flush();
	serial.printf("AT+NSOST=%d,%s,%d,%d,", socket, ip, port, len);
	for (size_t i = 0; i < len; i++) {
		serial.printf("%02X", buffer[i]);
	}
	serial.write((uint8_t*)"\r\n", 2);
	return serial.find(30000, { "OK", "ERROR" }) == 0;
}

size_t Boudica120Module::receiveUDP(int socket, uint8_t* buffer, size_t len, uint32_t timeout) {
	size_t bindex = 0;
	if (serial.find(timeout, {"+NSONMI:"}) == 0) {
		int rcvsock;
		unsigned int rsize;
		char buf[12];
		serial.readNextNumber(buf, sizeof(buf));
		rcvsock = atoi(buf);
		serial.readNextNumber(buf, sizeof(buf));
		rsize = atoi(buf);
		// read and discard the rest of the line
		serial.readLine(nullptr, 0, 1000);
		if (rcvsock == socket) {
			serial.printf("AT+NSORF=%d,%u\r\n", socket, len);
			// socket,ip,port,len,hex_data,remaining
			int commaCount = 4;
			while (commaCount > 0) {
				int c = serial.read();
				if (c < 0) {
					return bindex;
				}
				if (c == ',') {
					commaCount--;
				}
			}
			buf[2] = 0;
			uint8_t c;
			while (rsize > 0 && bindex < len) {
				if (serial.readBytes((uint8_t*)buf, 2, 1000) != 2) {
					break;
				}
				hexStringToByteArray(buf, &c, 1);
				buffer[bindex++] = c;
				rsize--;
			}
		}
	}
	return bindex;
}

bool Boudica120Module::getImei(char* buffer) {
	serial.flush();
	buffer[0] = 0;
	serial.printf("AT+CGSN=1\r\n");
	if (serial.find(3000, { "+CGSN:" }) != 0) {
		return false;
	}
	serial.setTimeout(100);
	int i;
	for (i = 0; i < 16; i++) {
		int c = serial.read();
		if (c < 0) {
			return false;
		}
		if (c >= '0' && c <= '9') {
			buffer[i] = c;
		}
		else {
			break;
		}
	}
	buffer[i++] = 0;
	serial.find(100, { "OK" });
	return i;
}
