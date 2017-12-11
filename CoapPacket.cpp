/*
 * CoapPacket.cpp
 *
 *  Created on: Nov 17, 2017
 *      Author: armin
 */

#include <string.h>

#include "CoapPacket.h"

#define COAP_HEADER_SIZE 4

namespace Coap {

static inline void coapOptionDelta(uint32_t v, uint8_t* n) {
	v < 13 ? (*n = (0xFF & v)) : (v <= 0xFF + 13 ? (*n = 13) : (*n = 14));
}

size_t packetToBuffer(Packet& packet, uint8_t* buffer, size_t bufferSize) {
	uint8_t *p = buffer;
	uint16_t running_delta = 0;
	size_t packetSize = 0;

	// make coap packet base header
	*p = 0x01 << 6;
	*p |= ((uint8_t)packet.type & 0x03) << 4;
	*p++ |= (packet.tokenlen & 0x0F);
	*p++ = packet.code;
	*p++ = (packet.messageid >> 8);
	*p++ = (packet.messageid & 0xFF);
	p = buffer + COAP_HEADER_SIZE;
	packetSize += 4;

	// make token
	if (packet.token != nullptr && packet.tokenlen <= 0x0F) {
		memcpy(p, packet.token, packet.tokenlen);
		p += packet.tokenlen;
		packetSize += packet.tokenlen;
	}

	// make option header
	for (int i = 0; i < packet.optionnum; i++) {
		uint32_t optdelta;
		uint8_t len, delta;

		if (packetSize + 5 + packet.options[i].length >= bufferSize) {
			return 0;
		}
		optdelta = (uint8_t)packet.options[i].number - running_delta;
		coapOptionDelta(optdelta, &delta);
		coapOptionDelta((uint32_t )packet.options[i].length, &len);

		*p++ = (0xFF & (delta << 4 | len));
		if (delta == 13) {
			*p++ = (optdelta - 13);
			packetSize++;
		} else if (delta == 14) {
			*p++ = ((optdelta - 269) >> 8);
			*p++ = (0xFF & (optdelta - 269));
			packetSize += 2;
		}
		if (len == 13) {
			*p++ = (packet.options[i].length - 13);
			packetSize++;
		} else if (len == 14) {
			*p++ = (packet.options[i].length >> 8);
			*p++ = (0xFF & (packet.options[i].length - 269));
			packetSize += 2;
		}

		memcpy(p, packet.options[i].buffer, packet.options[i].length);
		p += packet.options[i].length;
		packetSize += packet.options[i].length + 1;
		running_delta = (uint8_t)packet.options[i].number;
	}

	// make payload
	if (packet.payloadlen > 0) {
		if ((packetSize + 1 + packet.payloadlen) >= bufferSize) {
			return 0;
		}
		*p++ = 0xFF;
		memcpy(p, packet.payload, packet.payloadlen);
		packetSize += 1 + packet.payloadlen;
	}
	return packetSize;
}

static int parseOption(Option *option, uint16_t *running_delta, uint8_t **buf, size_t buflen) {
	uint8_t *p = *buf;
	uint8_t headlen = 1;
	uint16_t len, delta;

	if (buflen < headlen)
		return -1;

	delta = (p[0] & 0xF0) >> 4;
	len = p[0] & 0x0F;

	if (delta == 13) {
		headlen++;
		if (buflen < headlen)
			return -1;
		delta = p[1] + 13;
		p++;
	} else if (delta == 14) {
		headlen += 2;
		if (buflen < headlen)
			return -1;
		delta = ((p[1] << 8) | p[2]) + 269;
		p += 2;
	} else if (delta == 15)
		return -1;

	if (len == 13) {
		headlen++;
		if (buflen < headlen)
			return -1;
		len = p[1] + 13;
		p++;
	} else if (len == 14) {
		headlen += 2;
		if (buflen < headlen)
			return -1;
		len = ((p[1] << 8) | p[2]) + 269;
		p += 2;
	} else if (len == 15)
		return -1;

	if ((p + 1 + len) > (*buf + buflen))
		return -1;
	option->number = (Option::Number)(delta + *running_delta);
	option->buffer = p + 1;
	option->length = len;
	*buf = p + 1 + len;
	*running_delta += delta;

	return 0;
}

bool bufferToPacket(Packet& packet, uint8_t* buffer, size_t bufferSize) {

	packet.type = (Coap::Type)((buffer[0] & 0x30) >> 4);
	packet.tokenlen = buffer[0] & 0x0F;
	packet.code = buffer[1];
	packet.messageid = 0xFF00 & (buffer[2] << 8);
	packet.messageid |= 0x00FF & buffer[3];

	if (packet.tokenlen == 0) {
		packet.token = nullptr;

	}
	else if (packet.tokenlen <= 8) {
		packet.token = buffer + 4;
	}
	else {
		return false;
	}

	// parse packet options/payload
	if ((size_t)COAP_HEADER_SIZE + packet.tokenlen < bufferSize) {
		int optionIndex = 0;
		uint16_t delta = 0;
		uint8_t *end = buffer + bufferSize;
		uint8_t *p = buffer + COAP_HEADER_SIZE + packet.tokenlen;
		while (optionIndex < COAP_MAX_OPTION_NUM && *p != 0xFF && p < end) {
			if (0 != parseOption(&packet.options[optionIndex], &delta, &p, end - p)) {
				return false;
			}
			optionIndex++;
		}
		packet.optionnum = optionIndex;

		if (p + 1 < end && *p == 0xFF) {
			packet.payload = p + 1;
			packet.payloadlen = end - (p + 1);
		} else {
			packet.payload = nullptr;
			packet.payloadlen = 0;
		}
	}
	return true;

}
}

