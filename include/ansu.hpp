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

namespace ANS
{
	struct State
	{
		state_t x		= 0;
		state_t partial = 0;
		u8 partial_bits = 0;
		u32 meta_offset = 0;
	};

	struct Meta
	{
		state_t control_state;
		state_t partial;
		u32 offset;
		u32 dead_bits;
		u32 dead_bytes;

		bool operator==(const Meta other)
		{
			return this->control_state == other.control_state
				   && this->partial == other.partial
				   && this->offset == other.offset
				   && this->dead_bits == other.dead_bits
				   && this->dead_bytes == this->dead_bytes;
		}

		bool operator!=(const Meta other) { return !(*this == other); }
	};

	void compress(hls::stream<message_t>& message,
				  hls::stream<state_t>& out,
				  hls::stream<Meta>& meta,
				  u32 dropBytes,
				  u8& control);

	void decompress(hls::stream<state_t>& out,
					hls::stream<Meta>& meta,
					hls::stream<message_t>& message);
} // namespace ANS
