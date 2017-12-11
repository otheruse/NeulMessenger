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
#define powerPin 7
#endif

ArduinoNStream stream(MODEM_STREAM);
Boudica120Module module(stream);
NeulMessenger messenger(module, "172.16.14.22", 5683);
uint8_t rbuf[100];

timestamp_t startTime;

bool sendCommandExpectResponse(const char* command, const char* response, uint32_t maxWait) {
	stream.flush();
	DEBUG_STREAM.println(command);
	MODEM_STREAM.println(command);
	return stream.find(maxWait, { response }) == 0;
}

bool waitForIP() {
	const size_t maxTries = 10;
	DEBUG_STREAM.println("Waiting for ip address...");
	size_t tries = 0;
	char buffer[25];
	int ip[4];
	do {
		if (sendCommandExpectResponse("AT+CGPADDR=1", "+CGPADDR:1,", 2000)) {
			int n = stream.readLine(buffer, sizeof(buffer), 1000);
			buffer[n - 2] = 0; // remove \r\n
			if (n >= 7) {
				int matches = sscanf(buffer, "%d.%d.%d.%d", ip, ip + 1, ip + 2, ip + 3);
				if (matches == 4) {
					break;
				}
			}
		}
		delaymS(1000);
		tries++;
	}
	while (tries < maxTries);
	if (tries >= maxTries) {
		return false;
	}
	DEBUG_STREAM.print("Got ip address: ");
	DEBUG_STREAM.println(buffer);
	return true;
}


void setup() {
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);
	// Turn the nb-iot module on
	pinMode(powerPin, OUTPUT);
	digitalWrite(powerPin, HIGH);
	DEBUG_STREAM.begin(115200);
	MODEM_STREAM.begin(9600);
	stream.find(3000, { "OK" }); // Wait for the modem to start responding after turn-on

	if (!sendCommandExpectResponse("AT+CFUN=0", "OK", 3000)) {
		DEBUG_STREAM.println("Error with modem");
		return;
	}

	if (!sendCommandExpectResponse("AT+NCONFIG?", "OK", 3000)) {
		DEBUG_STREAM.println("Error with modem");
		return;
	}

	delaymS(1000);
	sendCommandExpectResponse("AT+NCONFIG=CR_0354_0338_SCRAMBLING,FALSE", "OK", 2000);
	sendCommandExpectResponse("AT+NCONFIG=AUTOCONNECT,FALSE", "OK", 2000);
	sendCommandExpectResponse("AT+NCONFIG=CR_0859_SI_AVOID,FALSE", "OK", 2000);

	delay(2000); // wait a bit

	if (!sendCommandExpectResponse("AT+CFUN=1", "OK", 3000)) {
		if (!sendCommandExpectResponse("AT+NRB", "Neul", 5000)) {
			DEBUG_STREAM.println("Error with modem");
			return;
		}
		delay(2000); // wait a bit
		if (!sendCommandExpectResponse("AT+CFUN=1", "OK", 3000)) {
			DEBUG_STREAM.println("Error with modem");
			return;
		}
	}
	// First connect to the network
	if (!sendCommandExpectResponse("AT+CGDCONT=1,\"IP\", \"oceanconnect.t-mobile.nl\"", "OK", 3000)) {
		DEBUG_STREAM.println("Could not configure APN");
		return;
	}
	if (!sendCommandExpectResponse("AT+COPS=1,2,\"20416\"", "OK", 3000)) {
		DEBUG_STREAM.println("Could not configure APN");
		return;
	}
	stream.printf("AT+CEREG=1\r\n");
	timestamp_t start = getTimestamp();
	const timestamp_t waitTime = 120000;
	char buffer[5];
	do {
		DEBUG_STREAM.println("Wating for network connection...");
		if (stream.find(waitTime - (getTimestamp() - start), { "+CEREG:" }) != 0) {
			DEBUG_STREAM.println("Could not establish connection");
			return;
		}
		stream.readLine(buffer, sizeof(buffer), 100);
		int cegreg = -1;
		cegreg = atoi(buffer);
		if (cegreg == 1 || cegreg == 5) {
			DEBUG_STREAM.println("Connection established");
			// Home network or roaming
			break;
		}
	}
	while (getTimestamp() - start < waitTime);
	waitForIP();
	delaymS(2000);// wait a bit again
	char message[] = "Neul message from BC95";
	sendCommandExpectResponse("AT+NSOCL=0\r\n", "OK", 1000);
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

void loop()
{
	static bool first = true;
	if (first) {
		DEBUG_STREAM.println("In the loop");
		first = false;
	}
	while (DEBUG_STREAM.available())
	{
		uint8_t c = DEBUG_STREAM.read();
		DEBUG_STREAM.write(c);
		MODEM_STREAM.write(c);
	}

	while (MODEM_STREAM.available())
	{
		DEBUG_STREAM.write(MODEM_STREAM.read());
	}
}
