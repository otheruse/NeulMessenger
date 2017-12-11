#include "Arduino.h"
#include "NeulMessenger.h"
#include "Boudica120Module.h"
#include "MDM9206Module.h"
#include "string.h"

#if defined(ARDUINO_AVR_LEONARDO)
#include <ArduinoSTL.h>
#define DEBUG_STREAM Serial
#define MODEM_STREAM Serial1
#define powerPin 7
#elif defined(ARDUINO_STM32L4_OTHERUSE1)
#define DEBUG_STREAM Serial
#define MODEM_STREAM Serial2
#define powerPin 25
#elif defined (ARDUINO_NUCLEO_L053R8)
#define DEBUG_STREAM Serial2
#define MODEM_STREAM Serial1
#define powerPin 7
#elif defined(ARDUINO_OTHERUSE_L433RC_NBIOT)
#define DEBUG_STREAM Serial
#define MODEM_STREAM Serial3
#define powerPin 22
#elif defined(ARDUINO_NUCLEO_L476RG)
#define DEBUG_STREAM Serial
#define MODEM_STREAM Serial1
#define powerPin 22
#endif

const int commands = 2;
const char* find[commands] = { "reboot", "echo" };

ArduinoNStream stream(MODEM_STREAM);
MDM9206Module module(stream);
//NeulMessenger messenger(module, "37.139.31.129", 5683);
NeulMessenger messenger(module, "172.16.14.22", 5683);
uint8_t rbuf[100];


timestamp_t startTime;

bool sendCommandExpectResponse(const char* command, const char* response, uint32_t maxWait) {
	stream.flush();
	DEBUG_STREAM.println(command);
	MODEM_STREAM.println(command);
	return stream.find(maxWait, {response}) == 0;
}

void setup() {
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);
	DEBUG_STREAM.begin(115200);
	MODEM_STREAM.begin(115200);
	// TODO reset/turn on module

//	sendCommandExpectResponse("AT+CFUN=1,1", "RDY", 10000);
	DEBUG_STREAM.println("Waiting for modem to become ready...");
	delay(3000);
/*
 */
//	sendCommandExpectResponse("AT+CGDCONT=1,\"IP\",\"iot.test.telekom\"", "OK", 3000);
	sendCommandExpectResponse("AT+CGDCONT=1,\"IP\",\"oceanconnect.t-mobile.nl\"", "OK", 3000);
	sendCommandExpectResponse("AT+QCFG=\"iotopmode\",1,1","OK", 3000);
	sendCommandExpectResponse("AT+QCFG=\"nwscanseq\",030201,1","OK", 3000);
	sendCommandExpectResponse("AT+QCFG=\"nbsibscramble\",1","OK", 3000);
	sendCommandExpectResponse("AT+CGREG=1","OK", 3000);
	sendCommandExpectResponse("AT+CGATT=1","OK", 60000);
/**/
	sendCommandExpectResponse("AT+CGPADDR=1","OK", 3000);
	char imei[20];
	module.getImei(imei);
	DEBUG_STREAM.print("IMEI: ");
	DEBUG_STREAM.println(imei);
	char message[] = "Neul message from BG96";
	messenger.sendMessage((uint8_t*)message, strlen(message));
	size_t bytes = messenger.receiveMessage(rbuf, sizeof(rbuf), 2000);
	DEBUG_STREAM.println("\r\n");
	DEBUG_STREAM.print(bytes, 10);
	DEBUG_STREAM.println(" bytes received");
	DEBUG_STREAM.println("\r\n\r\nStarting serial pass-through");
	startTime = getTimestamp();
	// Clear user input buffer
	while (DEBUG_STREAM.available()) {
		DEBUG_STREAM.read();
	}
}

bool checkMatch(char c, const char* string, size_t& idx) {
	if (c == string[idx]) {
		idx++;
		if (idx == strlen(string)) {
			idx = 0;
			return true;
		}
	}
	else {
		idx = 0;
	}
	return false;
}

void loop()
{
	timestamp_t elapsed = getTimestamp() - startTime;
	if (elapsed > 60000) {
		startTime = getTimestamp();
		const char* testMessage = "Periodic test message";
		messenger.sendMessage((uint8_t*)testMessage, strlen(testMessage));
		size_t bytes = messenger.receiveMessage(rbuf, sizeof(rbuf), 2000);
		DEBUG_STREAM.println("\r\n");
		DEBUG_STREAM.print(bytes, 10);
		DEBUG_STREAM.println(" bytes received");
	}
	static bool echo = false;
	static size_t index[commands] = { };
	while (DEBUG_STREAM.available())
	{
		uint8_t c = DEBUG_STREAM.read();
		for (int i = 0; i < commands; ++i) {
			if (checkMatch(c, find[i], index[i])) {
				switch (i) {
				case 0:
					NVIC_SystemReset();
					break;
				case 1:
					echo = !echo;
					break;
				default:
					break;
				}
			}
		}
		if (echo) {
			DEBUG_STREAM.write(c);
		}
		MODEM_STREAM.write(c);
	}

	while (MODEM_STREAM.available())
	{
		DEBUG_STREAM.write(MODEM_STREAM.read());
	}
}
