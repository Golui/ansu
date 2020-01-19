#pragma once

#include "ans_table.hpp"
#include "ints.hpp"
#include "settings.hpp"
#ifdef NO_VIVADO
#	include "binstream.hpp"
#else
#	include <hls_stream.h>
#endif

#define CONTROL_RESET_STATE 0b1
#define CONTROL_ENCODE 0b10
#define CONTROL_FLUSH 0b100

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)                                  \
	(byte & 0x80 ? '1' : '0'), (byte & 0x40 ? '1' : '0'),     \
		(byte & 0x20 ? '1' : '0'), (byte & 0x10 ? '1' : '0'), \
		(byte & 0x08 ? '1' : '0'), (byte & 0x04 ? '1' : '0'), \
		(byte & 0x02 ? '1' : '0'), (byte & 0x01 ? '1' : '0')

struct ANSMeta
{
	state_t control_state;
	state_t partial;
	u32 offset;
	u8 dead_bits;

	bool operator==(const ANSMeta other)
	{
		return this->control_state == other.control_state
			   && this->partial == other.partial && this->offset == other.offset
			   && this->dead_bits == other.dead_bits;
	}

	bool operator!=(const ANSMeta other) { return !(*this == other); }
};

void encode_stream(hls::stream<message_t>& message,
				   hls::stream<state_t>& out,
				   hls::stream<ANSMeta>& meta,
				   u8& control);
