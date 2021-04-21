#pragma once

#include "backend/backend.hpp"
#include "backend/stream.hpp"
#include "data/ans_table.hpp"
#include "ints.hpp"
#include "settings.hpp"

#define CONTROL_RESET_STATE 1
#define CONTROL_ENCODE 2
#define CONTROL_FLUSH 4

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)                                  \
	(byte & 0x80 ? '1' : '0'), (byte & 0x40 ? '1' : '0'),     \
		(byte & 0x20 ? '1' : '0'), (byte & 0x10 ? '1' : '0'), \
		(byte & 0x08 ? '1' : '0'), (byte & 0x04 ? '1' : '0'), \
		(byte & 0x02 ? '1' : '0'), (byte & 0x01 ? '1' : '0')

namespace ANS
{
	constexpr static const u8 all_bits_remaining = (sizeof(state_t) << 3);

	using DecompressResult = bool;

	struct State
	{
		state_t x		= 0;
		state_t partial = 0;
		u8 partial_bits = 0;
		u32 meta_offset = 0;
	};

	struct Meta
	{
		u32 channels;
		u32 current_channel;
		u32 message_pad;
		state_t control_state[CHANNEL_COUNT];
		u32 offset;
		u32 dead_bits;

		// TODO Currenly seems broken
		bool operator==(const Meta other)
		{
			if(this->channels != other.channels) return false;
			for(u32 i = 0; i < this->channels; i++)
			{
				if(this->control_state[i] != other.control_state[i])
					return false;
			}
			return this->message_pad == other.message_pad
				   && this->control_state == other.control_state
				   && this->offset == other.offset
				   && this->dead_bits == other.dead_bits;
		}

		bool operator!=(const Meta other) { return !(*this == other); }
	};

	void compress(backend::stream<message_t>& message,
				  backend::stream<state_t>& out,
				  backend::stream<Meta>& meta,
				  u32 padding,
				  u8& control);

	DecompressResult decompress(backend::stream<state_t>& out,
								backend::stream<Meta>& meta,
								backend::stream<message_t>& message);
} // namespace ANS
