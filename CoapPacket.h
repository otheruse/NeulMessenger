/*
 * CoapPacket.h
 *
 *  Created on: Nov 17, 2017
 *      Author: armin
 */

#ifndef COAPPACKET_H_
#define COAPPACKET_H_

#include <stdint.h>

#define COAP_MAX_OPTION_NUM 5

namespace Coap {
enum class Type {
    CON = 0,
    NON = 1,
    ACK = 2,
    RESET = 3
};

enum class Method : uint8_t {
    GET = 1,
    POST = 2,
    PUT = 3,
    DELETE = 4
};

enum class ResponseCode : uint8_t {
	EMPTY_MESSAGE = 0,
	CREATED = 65,
	DELETED = 66,
	VALID = 67,
	CHANGED = 68,
	CONTENT = 69,
	BAD_REQUEST = 128,
	UNAUTHORIZED = 129,
	BAD_OPTION = 130,
	FORBIDDEN = 131,
	NOT_FOUND = 132,
	METHOD_NOT_ALLOWED = 133,
	PRECONDITION_FAILED = 140,
	REQUEST_ENTITY_TOO_LARGE= 141,
	UNSUPPORTED_CONTENT_FORMAT = 143,
	INTERNAL_SERVER_ERROR = 160,
	NOT_IMPLEMENTED = 161,
	BAD_GATEWAY = 162,
	SERVICE_UNAVALIABLE = 163,
	GATEWAY_TIMEOUT = 164,
	PROXYING_NOT_SUPPORTED = 165
};

enum class ContentType {
    NONE = -1,
    TEXT_PLAIN = 0,
    APPLICATION_LINK_FORMAT = 40,
    APPLICATION_XML = 41,
    APPLICATION_OCTET_STREAM = 42,
    APPLICATION_EXI = 47,
    APPLICATION_JSON = 50
};

struct Option {
	enum class Number {
	    IF_MATCH = 1,
	    URI_HOST = 3,
	    E_TAG = 4,
	    IF_NONE_MATCH = 5,
		OBSERVE = 6,
	    URI_PORT = 7,
	    LOCATION_PATH = 8,
	    URI_PATH = 11,
	    CONTENT_FORMAT = 12,
	    MAX_AGE = 14,
	    URI_QUERY = 15,
	    ACCEPT = 17,
	    LOCATION_QUERY = 20,
	    PROXY_URI = 35,
	    PROXY_SCHEME = 39,
	};
	Number number;
    uint8_t length;
    uint8_t *buffer;
};

struct Packet {
    Type type;
    union {
    	Method method;
    	ResponseCode responseCode;
    	uint8_t code;
    };
    uint8_t *token;
    uint8_t tokenlen;
    uint8_t *payload;
    uint8_t payloadlen;
    uint16_t messageid;

    uint8_t optionnum;
    Option options[COAP_MAX_OPTION_NUM];
};

size_t packetToBuffer(Packet& packet, uint8_t* buffer, size_t bufferSize);
bool bufferToPacket(Packet& packet, uint8_t* buffer, size_t bufferSize);
}

#endif /* COAPPACKET_H_ */
